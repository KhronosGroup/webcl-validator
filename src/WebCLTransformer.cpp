/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include "WebCLDebug.hpp"
#include "WebCLHeader.hpp"
#include "WebCLTransformer.hpp"
#include "WebCLVisitor.hpp"
#include "WebCLPass.hpp"
#include "WebCLTypes.hpp"
#include "general.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

WebCLTransformer::WebCLTransformer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : WebCLReporter(instance)
    , wclRewriter_(instance, rewriter)
    , cfg_()
{
}

WebCLTransformer::~WebCLTransformer()
{
    for (FunctionPrologueMap::iterator i = kernelPrologues_.begin();
         i != kernelPrologues_.end(); ++i) {
        std::stringstream *out = i->second;
        delete out;
    }

    for (FunctionPrologueMap::iterator i = functionPrologues_.begin();
         i != functionPrologues_.end(); ++i) {
        std::stringstream *out = i->second;
        delete out;
    }
}

bool WebCLTransformer::rewrite()
{
    bool status = true;

    // do all replacements stored in refactoring first ()
    flushQueuedTransformations();

    status = status && rewritePrologue();
  
    std::set<const clang::FunctionDecl*> kernelOrFunction;
    for (FunctionPrologueMap::iterator iter = kernelPrologues_.begin();
         iter != kernelPrologues_.end(); iter++) {
      kernelOrFunction.insert(iter->first);
    }

    for (FunctionPrologueMap::iterator iter = functionPrologues_.begin();
       iter != functionPrologues_.end(); iter++) {
      kernelOrFunction.insert(iter->first);
    }
  
    for (std::set<const clang::FunctionDecl*>::iterator iter = kernelOrFunction.begin();
         iter != kernelOrFunction.end(); iter++) {

      const clang::FunctionDecl *func = *iter;
      clang::Stmt *body = func->getBody();
      if (!body) {
        error(func->getLocStart(), "Function has no body.");
        return false;
      }

      std::stringstream prologue;
      if (kernelPrologues_.count(func) > 0) {
          prologue << functionPrologue(kernelPrologues_, func).str();
      }
      if (functionPrologues_.count(func) > 0) {
          prologue << functionPrologue(functionPrologues_, func).str();
      }

      clang::SourceLocation loc = body->getLocStart();
      clang::SourceRange range(loc, loc);
      std::string orig = wclRewriter_.getTransformedText(range) + "\n";
      wclRewriter_.replaceText(range, orig + prologue.str());
    }

    flushQueuedTransformations();

    return status;
}

void WebCLTransformer::createJsonHeader(std::set<clang::FunctionDecl*> &kernels)
{
    WebCLHeader header(instance_, cfg_);
    header.emitHeader(jsonPrologue_, kernels);
}

std::stringstream& WebCLTransformer::functionPrologue(
    FunctionPrologueMap &prologues, const clang::FunctionDecl *kernel)
{
    if (!prologues.count(kernel)) {
        std::stringstream *out = new std::stringstream();
        if (!out) {
            fatal("Internal error. Can't create stream for function prologue.");
            return modulePrologue_;
        }
        prologues[kernel] = out;
    }
    return *prologues[kernel];
}

std::string WebCLTransformer::addressSpaceInfoAsStruct(AddressSpaceInfo &as)
{
  std::stringstream retVal;
  retVal << "{\n";
  for (AddressSpaceInfo::iterator declIter = as.begin();
       declIter != as.end(); ++declIter) {
    retVal << cfg_.indentation_;
    emitVarDeclToStruct(retVal, (*declIter));
    retVal << ";\n";
  }
  retVal << "}";
  return retVal.str();
}

std::string WebCLTransformer::addressSpaceInitializer(AddressSpaceInfo &as) {
  std::stringstream retVal;
  retVal << "{ ";
  std::string comma = "";
  for (AddressSpaceInfo::iterator declIter = as.begin();
       declIter != as.end(); ++declIter) {
    retVal << comma;
    emitVariableInitialization(retVal, (*declIter));
    comma = ", ";
  }
  retVal << " }";

  DEBUG( std::cerr << "Created address space initializer: " << retVal.str() << "\n"; );
  return retVal.str();
}

std::string WebCLTransformer::addressSpaceLimitsAsStruct(AddressSpaceLimits &asLimits)
{
    std::stringstream retVal;
    retVal << "{\n";
  
    // if address space has static allocations
    if (asLimits.hasStaticallyAllocatedLimits()) {
        std::stringstream prefix;
        prefix << cfg_.indentation_ << "__";

        switch (asLimits.getAddressSpace()) {
        case clang::LangAS::opencl_constant:
            prefix << cfg_.constantAddressSpace_ << " "
                   << cfg_.constantRecordType_ << " *";
            retVal << prefix.str() << " "
                   << cfg_.constantMinField_ << ";\n";
            retVal << prefix.str() << " "
                   << cfg_.constantMaxField_ << ";\n";
            break;

        case clang::LangAS::opencl_local:
            prefix << cfg_.localAddressSpace_ << " "
                   << cfg_.localRecordType_ << " *";
            retVal << prefix.str() << " "
                   << cfg_.localMinField_ << ";\n";
            retVal << prefix.str() << " "
                   << cfg_.localMaxField_ << ";\n";
            break;

        default:
            break;
        }
    }
  
    for (AddressSpaceLimits::LimitList::iterator declIter = asLimits.getDynamicLimits().begin();
         declIter != asLimits.getDynamicLimits().end(); ++declIter) {
        clang::ParmVarDecl *decl = *declIter;

        retVal << cfg_.indentation_;
        emitVarDeclToStruct(retVal, decl, cfg_.getNameOfLimitField(decl, false));
        retVal << ";\n";

        retVal << cfg_.indentation_;
        emitVarDeclToStruct(retVal, decl, cfg_.getNameOfLimitField(decl, true));
        retVal << ";\n";
    }
    retVal << "}";
    return retVal.str();
}

std::string WebCLTransformer::addressSpaceLimitsInitializer(
    clang::FunctionDecl *kernelFunc, AddressSpaceLimits &asLimits)
{
    std::stringstream retVal;
    retVal << "{ ";
    std::string comma = "";

    // if address space has static allocations
    if (asLimits.hasStaticallyAllocatedLimits()) {
        std::string name = "&(&";
        
        switch (asLimits.getAddressSpace()) {
        case clang::LangAS::opencl_constant:
            name += cfg_.constantRecordName_ + ")";
            retVal << name << "[0], " << name << "[1]";
            comma = ",";
            break;

        case clang::LangAS::opencl_local:
            name += cfg_.localRecordName_ + ")";
            retVal << name << "[0], " << name << "[1]";
            comma = ",";
            break;

        case clang::LangAS::opencl_global:
            assert(false && "Global address space can't have static allocations.");
            break;

        default:
            assert((asLimits.getAddressSpace() == 0) &&
                   "Expected private address space.");
            break;
        }
    }

    for (AddressSpaceLimits::LimitList::iterator declIter = asLimits.getDynamicLimits().begin();
         declIter != asLimits.getDynamicLimits().end(); ++declIter) {

        retVal << comma;
        clang::ParmVarDecl *decl = *declIter;
        clang::FunctionDecl *func = llvm::dyn_cast<clang::FunctionDecl>(decl->getParentFunctionOrMethod());
        assert(func && "Parameter doesn't have a parent function.");
    
        if (func == kernelFunc) {
            const std::string name = decl->getName();
            retVal << "&" << name
                   << "[0], "
                   << "&" << name
                   << "[" << cfg_.getNameOfSizeParameter(decl) << "]";
        } else {
            retVal << "0, 0";
        }
    
        comma = ",";
    }
    retVal << " }";
    return retVal.str();
}

void WebCLTransformer::createAddressSpaceLimitsNullInitializer(
    std::ostream &out, unsigned addressSpace)
{
  switch (addressSpace) {
    case clang::LangAS::opencl_constant:
    case clang::LangAS::opencl_local:
      out << cfg_.getNameOfAddressSpaceNull(addressSpace);
      break;
    default:
      out << "0";
  }
}

void WebCLTransformer::createAddressSpaceTypedef(
    AddressSpaceInfo &as, const std::string &name, const std::string &alignment)
{
    if (!as.empty()) {
        modulePrologue_ << "typedef struct "
                        << addressSpaceInfoAsStruct(as)
                        << " __attribute__ ((aligned (" << alignment << "))) " << name << ";\n\n";
    }
}

void WebCLTransformer::createPrivateAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.privateRecordType_, cfg_.getNameOfAlignMacro("private"));
}

void WebCLTransformer::createLocalAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.localRecordType_, cfg_.getNameOfAlignMacro("local"));
}

void WebCLTransformer::createConstantAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.constantRecordType_, cfg_.getNameOfAlignMacro("constant"));
}

void WebCLTransformer::createAddressSpaceLimitsTypedef(
    AddressSpaceLimits &limits, const std::string &name)
{
    modulePrologue_ << "typedef struct "
                    << addressSpaceLimitsAsStruct(limits)
                    << " " << name << ";\n\n";
}

void WebCLTransformer::createGlobalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits)
{
    createAddressSpaceLimitsTypedef(asLimits, cfg_.globalLimitsType_);
}

void WebCLTransformer::createConstantAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits)
{
    createAddressSpaceLimitsTypedef(asLimits, cfg_.constantLimitsType_);
}

void WebCLTransformer::createLocalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits)
{
    createAddressSpaceLimitsTypedef(asLimits, cfg_.localLimitsType_);
}

void WebCLTransformer::createAddressSpaceLimitsField(
    const std::string &type, const std::string &name)
{
    modulePrologue_ << cfg_.indentation_ << type << " " << name << ";\n";
}

void WebCLTransformer::createAddressSpaceNullField(
    const std::string &name, unsigned addressSpace)
{
    modulePrologue_ << cfg_.indentation_
                    << "__" << cfg_.getNameOfAddressSpace(addressSpace)
                    << " " << cfg_.nullType_ << " *" << name << ";\n";
}

void WebCLTransformer::createProgramAllocationsTypedef(
    AddressSpaceLimits &globalLimits, AddressSpaceLimits &constantLimits,
    AddressSpaceLimits &localLimits, AddressSpaceInfo &privateAs)
{
    modulePrologue_ << "typedef struct {\n";
    if (!globalLimits.empty()) {
        createAddressSpaceLimitsField(cfg_.globalLimitsType_, cfg_.globalLimitsField_);
        createAddressSpaceNullField(cfg_.globalNullField_, globalLimits.getAddressSpace());
    }
    if (!constantLimits.empty()) {
        createAddressSpaceLimitsField(cfg_.constantLimitsType_, cfg_.constantLimitsField_);
        createAddressSpaceNullField(cfg_.constantNullField_, constantLimits.getAddressSpace());
    }
    if (!localLimits.empty()) {
        createAddressSpaceLimitsField(cfg_.localLimitsType_, cfg_.localLimitsField_);
        createAddressSpaceNullField(cfg_.localNullField_, localLimits.getAddressSpace());
    }
    if (!privateAs.empty()) {
        modulePrologue_ << cfg_.indentation_ << cfg_.privateRecordType_<< " " << cfg_.privatesField_ << ";\n";
        createAddressSpaceNullField(cfg_.privateNullField_, 0);
    }
    modulePrologue_ << "} " << cfg_.addressSpaceRecordType_ << ";\n\n";
}

void WebCLTransformer::createConstantAddressSpaceAllocation(AddressSpaceInfo &as)
{
    if (!as.empty()) {
        modulePrologue_ << "__" << cfg_.constantAddressSpace_ << " "
                        << cfg_.constantRecordType_ << " "
                        << cfg_.constantRecordName_ << " = "
                        << addressSpaceInitializer(as) << ";\n\n";
    }
}

void WebCLTransformer::createLocalAddressSpaceAllocation(clang::FunctionDecl *kernelFunc)
{
    std::ostream &out = functionPrologue(kernelPrologues_, kernelFunc);

    out << "\n" << cfg_.indentation_ << "__" << cfg_.localAddressSpace_ << " "
        << cfg_.localRecordType_ << " " << cfg_.localRecordName_ << ";\n";
}

void WebCLTransformer::createAddressSpaceLimitsInitializer(
    std::ostream &out, clang::FunctionDecl *kernel, AddressSpaceLimits &limits)
{
    out << cfg_.getIndentation(2) << addressSpaceLimitsInitializer(kernel, limits);
    out << ",\n" << cfg_.getIndentation(2);
    createAddressSpaceLimitsNullInitializer(out, limits.getAddressSpace());
}

void WebCLTransformer::createProgramAllocationsAllocation(
    clang::FunctionDecl *kernelFunc, AddressSpaceLimits &globalLimits,
    AddressSpaceLimits &constantLimits, AddressSpaceLimits &localLimits,
    AddressSpaceInfo &privateAs)
{
    std::ostream &out = functionPrologue(kernelPrologues_, kernelFunc);

    out << "\n" << cfg_.indentation_
        << cfg_.addressSpaceRecordType_ << " " << cfg_.programRecordName_ << " = {\n";

    bool hasPrev = false;

    if (!globalLimits.empty()) {
        createAddressSpaceLimitsInitializer(out, kernelFunc, globalLimits);
        hasPrev = true;
    }

    if (!constantLimits.empty()) {
        if (hasPrev) {
            out << ",\n";
        }
        createAddressSpaceLimitsInitializer(out, kernelFunc, constantLimits);
        hasPrev = true;
    }

    if (!localLimits.empty()) {
        if (hasPrev) {
            out << ",\n";
        }
        createAddressSpaceLimitsInitializer(out, kernelFunc, localLimits);
        hasPrev = true;
    }

    if (!privateAs.empty()) {
      if (hasPrev) {
            out << ",\n";
      }
      // we pretty much cannot initialize this in the start since if e.g. variables
      // are used to initialize private variables, we cannot move initialization to start of function
      // since value might be different in that phase.
      out << cfg_.getIndentation(2) << "{ },\n";
      out << cfg_.getIndentation(2) << "0\n";
    }

    out << "\n" << cfg_.indentation_ << "};\n";
    out << cfg_.indentation_ << cfg_.addressSpaceRecordType_ << " *"
        << cfg_.addressSpaceRecordName_ << " = &" << cfg_.programRecordName_ << ";\n";
}

void WebCLTransformer::createAddressSpaceNullAllocation(
    std::ostream &out, unsigned addressSpace)
{
    const bool isLocal = (addressSpace == clang::LangAS::opencl_local);

    if (isLocal)
        out << cfg_.indentation_;

    out << "__" << cfg_.getNameOfAddressSpace(addressSpace) << " "
        << cfg_.nullType_ << " " << cfg_.getNameOfAddressSpaceNull(addressSpace)
        << "[" << cfg_.getNameOfSizeMacro(addressSpace) << "]";

    if (!isLocal)
        out << " = { 0 }";

    out << ";\n";
}

void WebCLTransformer::createConstantAddressSpaceNullAllocation()
{
    createAddressSpaceNullAllocation(modulePrologue_, clang::LangAS::opencl_constant);
}

void WebCLTransformer::createLocalAddressSpaceNullAllocation(clang::FunctionDecl *kernel)
{
    std::ostream &out = functionPrologue(kernelPrologues_, kernel);
    createAddressSpaceNullAllocation(out, clang::LangAS::opencl_local);
}

void WebCLTransformer::initializeAddressSpaceNull(clang::FunctionDecl *kernel,
                                                  AddressSpaceLimits &limits)
{
  // init null only if there is limits.
  if (limits.empty()) return;
  
  std::ostream &out = functionPrologue(kernelPrologues_, kernel);
  
  std::string nullType =  "__" + cfg_.getNameOfAddressSpace(limits.getAddressSpace()) + " " + cfg_.nullType_ + "*";

  out << cfg_.indentation_
      << cfg_.getNameOfAddressSpaceNullPtrRef(limits.getAddressSpace()) << " = ";
  
  int endParenthesis = 0;
  
  if (limits.hasStaticallyAllocatedLimits()) {
      out << "_WCL_SET_NULL(" << nullType << ", " << cfg_.getNameOfSizeMacro(limits.getAddressSpace()) << ", " << cfg_.getStaticLimitRef(limits.getAddressSpace()) << ", ";
      endParenthesis++;
  }
  
  for(AddressSpaceLimits::LimitList::iterator i = limits.getDynamicLimits().begin();
      i != limits.getDynamicLimits().end(); i++) {
      out << "_WCL_SET_NULL(" << nullType << ", " << cfg_.getNameOfSizeMacro(limits.getAddressSpace()) << "," << cfg_.getDynamicLimitRef(*i) << ", ";
    endParenthesis++;
  }
  
  out << "(" << nullType << ")0";
  
  for (int i = 0; i < endParenthesis; i++) {
    out << ")";
  }
  out << ";\n";
  
  out << cfg_.indentation_
      << "if (" << cfg_.getNameOfAddressSpaceNullPtrRef(limits.getAddressSpace()) << " == (" << nullType << ")0) return; // not enough space to meet the minimum access. Would be great if we could give info about the problem for the user. \n";
}

void WebCLTransformer::createLocalRangeZeroing(
    std::ostream &out, const std::string &arguments)
{
    out << cfg_.indentation_
        << cfg_.localRangeZeroingMacro_ << "(" << arguments << ");\n";
}

void WebCLTransformer::createLocalAreaZeroing(
    clang::FunctionDecl *kernelFunc, AddressSpaceLimits &localLimits)
{
    if (localLimits.empty())
        return;

    std::ostream &out = functionPrologue(kernelPrologues_, kernelFunc);

    out << "\n" << cfg_.indentation_ << "// => Local memory zeroing.\n";

    if (localLimits.hasStaticallyAllocatedLimits()) {
        createLocalRangeZeroing(out, cfg_.getStaticLimitRef(clang::LangAS::opencl_local));
    }

    AddressSpaceLimits::LimitList &dynamicLimits = localLimits.getDynamicLimits();
    for (AddressSpaceLimits::LimitList::iterator i = dynamicLimits.begin();
         i != dynamicLimits.end(); ++i) {
        const clang::ParmVarDecl *decl = *i;
        createLocalRangeZeroing(out, cfg_.getDynamicLimitRef(decl));
    }

    createLocalRangeZeroing(out, cfg_.getNullLimitRef(clang::LangAS::opencl_local));

    out << cfg_.indentation_ << "barrier(CLK_LOCAL_MEM_FENCE);\n";
    out << cfg_.indentation_ << "// <= Local memory zeroing.\n";
}

void WebCLTransformer::replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl)
{
  std::string relocatedRef = cfg_.getReferenceToRelocatedVariable(decl);
  std::string original = wclRewriter_.getOriginalText(use->getSourceRange());
  DEBUG( std::cerr << "Replacing: " << original << " with: " << relocatedRef << "\n"; );
  wclRewriter_.replaceText(use->getSourceRange(), relocatedRef);
}

void WebCLTransformer::removeRelocated(clang::VarDecl *decl)
{
  wclRewriter_.removeText(decl->getSourceRange());
}

std::string WebCLTransformer::getCheckMacroCall(MacroKind kind, std::string addr, std::string type, unsigned size, AddressSpaceLimits &limits)
{
  std::stringstream retVal;

  const unsigned limitCount = limits.count();
  const unsigned addressSpace = limits.getAddressSpace();

  std::string name;
  switch (kind) {
  case MACRO_CLAMP:
      name = cfg_.getNameOfLimitClampMacro(addressSpace, limitCount);
      break;
  case MACRO_CHECK:
      name = cfg_.getNameOfLimitCheckMacro(addressSpace, limitCount);
      break;
  default:
      assert(0);
  }

  retVal << name << "(" << type << ", " << addr << ", " << size;

  if (limits.hasStaticallyAllocatedLimits()) {
    retVal << ", " << cfg_.getStaticLimitRef(addressSpace);
  }

  for (AddressSpaceLimits::LimitList::iterator i = limits.getDynamicLimits().begin();
       i != limits.getDynamicLimits().end(); i++) {
    retVal << ", " << cfg_.getDynamicLimitRef(*i);
  }

  if (kind == MACRO_CLAMP) {
      retVal << ", " << cfg_.getNameOfAddressSpaceNullPtrRef(addressSpace);
  }
  retVal << ")";

  // add macro implementations afterwards
  usedClampMacros_.insert(ClampMacroKey(limits.getAddressSpace(), limitCount));
  
  return retVal.str();
}

namespace {
    struct BaseIndexField {
	clang::Expr *base;
	clang::Expr *index;
	std::string  field;
  
	BaseIndexField(clang::Expr *access) : 
	    base(access), index(NULL) 
	{
	    if (clang::MemberExpr *memberExpr = llvm::dyn_cast<clang::MemberExpr>(access)) {
		base = memberExpr->getBase();
		field = memberExpr->getMemberNameInfo().getName().getAsString();

	    } else if (clang::ExtVectorElementExpr *vecExpr =
		llvm::dyn_cast<clang::ExtVectorElementExpr>(access)) {
		base = vecExpr->getBase();
		field = vecExpr->getAccessor().getName().str();
  
	    } else if (clang::ArraySubscriptExpr *arraySubExpr =
		llvm::dyn_cast<clang::ArraySubscriptExpr>(access)) {
		base = arraySubExpr->getBase();
		index = arraySubExpr->getIdx();
    
	    } else if (clang::UnaryOperator *unary = llvm::dyn_cast<clang::UnaryOperator>(access)) {
		base = unary->getSubExpr();
	    }
	}
	
    };
}

std::string WebCLTransformer::getClampMacroExpression(clang::Expr *access, unsigned size, AddressSpaceLimits &limits)
{
    BaseIndexField     bif(access);
    clang::SourceRange baseRange = clang::SourceRange(bif.base->getLocStart(), bif.base->getLocEnd());
    const std::string  original  = wclRewriter_.getOriginalText(access->getSourceRange());
    const std::string  baseStr   = wclRewriter_.getTransformedText(baseRange);
    std::stringstream  memAddress;

    memAddress << "(" << baseStr << ")";
    std::string indexStr;
    if (bif.index) {
	indexStr = wclRewriter_.getTransformedText(bif.index->getSourceRange());
	memAddress << "+(" << indexStr << ")";
    }
  
    // trust limits given in parameter or check against all limits
    std::string macro = getCheckMacroCall(MACRO_CLAMP, memAddress.str(), bif.base->getType().getAsString(), size, limits);

    std::stringstream retVal;
    retVal << "(*(" << macro  << "))";
    if (!bif.field.empty()) {
	retVal << "." << bif.field;
    }
	
    return retVal.str();
}

void WebCLTransformer::addMemoryAccessCheck(clang::Expr *access, unsigned size, AddressSpaceLimits &limits)
{
  std::string retVal = getClampMacroExpression(access, size, limits);
  
  DEBUG(
    std::cerr << "Creating memcheck for: " << original
              << "\n                 base: " << baseStr
              << "\n                index: " << indexStr
              << "\n          replacement: " << retVal
              << "\n----------------------------\n";
    access->dump();
    std::cerr << "============================\n\n"; );
  
  wclRewriter_.replaceText(access->getSourceRange(), retVal);
  DEBUG( std::cerr << "============================\n\n"; );
}

void WebCLTransformer::addRelocationInitializerFromFunctionArg(clang::ParmVarDecl *parmDecl)
{
  const clang::FunctionDecl *parent = llvm::dyn_cast<const clang::FunctionDecl>(parmDecl->getParentFunctionOrMethod());
  // add only once
  if (parameterRelocationInitializations_.count(parmDecl) == 0) {
      std::ostream &out = functionPrologue(functionPrologues_, parent);
      out << "\n" << cfg_.getReferenceToRelocatedVariable(parmDecl) << " = "
          << parmDecl->getNameAsString() << ";\n";
    parameterRelocationInitializations_.insert(parmDecl);
  }
}

void WebCLTransformer::addRelocationInitializer(clang::VarDecl *decl)
{
  clang::SourceLocation addLoc = wclRewriter_.findLocForNext(decl->getLocEnd(), ';');
  clang::SourceRange replaceRange(addLoc, addLoc);
  std::stringstream inits;
  inits << wclRewriter_.getTransformedText(replaceRange) << ";";

  // we have to assign arrays with memcpy
  if (decl->getType().getTypePtr()->isArrayType()) {
    inits << "_WCL_MEMCPY(" << cfg_.getReferenceToRelocatedVariable(decl) << "," << decl->getNameAsString() << ");";
  } else {
    inits << cfg_.getReferenceToRelocatedVariable(decl) << " = " << decl->getNameAsString() << ";";
  }
  wclRewriter_.replaceText(replaceRange, inits.str());

}

void WebCLTransformer::moveToModulePrologue(clang::NamedDecl *decl)
{
    // set typeName if we should make sure that this declaration name is not used multiple times
    std::string typeName;
    if (clang::RecordDecl* structDecl = llvm::dyn_cast<clang::RecordDecl>(decl)) {
        if (structDecl->isAnonymousStructOrUnion() || structDecl->getNameAsString().empty()) {
          error(structDecl->getLocStart(), "Anonymous structs should have been eliminated in this phase.");
          return;
        }

        // ignore forward declarations
        if (structDecl->getDefinition() == structDecl) {
          typeName = structDecl->getDefinition()->getNameAsString();
        }
    } else {
      typeName = decl->getNameAsString();
    }

    // make sure that identically named types are not collected from separate scopes...
    if (!typeName.empty()) {
      if (usedTypeNames_.count(typeName) > 0) {
        error(decl->getLocStart(), std::string("Identically named types aren't supported: " + typeName).c_str());
        return;
      } else {
        info(decl->getLocStart(), std::string("Adding type " + typeName + " to bookkeeping.").c_str());
        usedTypeNames_.insert(typeName);
      }
    }
  
    const std::string typedefText = wclRewriter_.getOriginalText(decl->getSourceRange());
    wclRewriter_.removeText(decl->getSourceRange());
    modulePrologue_ << typedefText << ";\n\n";
}

void WebCLTransformer::flushQueuedTransformations()
{
    wclRewriter_.applyTransformations();
}

void WebCLTransformer::addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                                             unsigned minWidthInBits)
{
    preModulePrologue_ << "#define " << cfg_.getNameOfSizeMacro(addressSpace) << " ("
                       << "((" << minWidthInBits << " + (CHAR_BIT - 1)) / CHAR_BIT)"
                       << ")\n";

    // get aligment rounded to next power of two
    unsigned minAlignment = 1;
    while (minWidthInBits != 0) {
      minWidthInBits = minWidthInBits>>1;
      minAlignment = minAlignment<<1;
    }
    minAlignment = minAlignment>>1;

    preModulePrologue_ << "#define " << cfg_.getNameOfAlignMacro(addressSpace) << " "
                       << "(" << minAlignment << "/CHAR_BIT)\n";
}

void WebCLTransformer::addRecordParameter(clang::FunctionDecl *decl)
{
    std::string parameter = cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;

    if (decl->getNumParams() > 0) {
      clang::SourceLocation addLoc = wclRewriter_.findLocForNext(decl->getLocStart(), '(');
      clang::SourceRange addRange = clang::SourceRange(addLoc, addLoc);
      std::string replacement = wclRewriter_.getOriginalText(addRange) + parameter + ", ";
      wclRewriter_.replaceText(addRange, replacement);
    } else {
      // in case of empty args, we need to replcace content inside parenthesis
      // empty params might be e.g. (void)
      clang::TypeLoc typeLoc = decl->getTypeSourceInfo()->getTypeLoc();
      clang::FunctionTypeLoc *funTypeLoc = llvm::dyn_cast<clang::FunctionTypeLoc>(&typeLoc);
      clang::SourceRange addRange(funTypeLoc->getLParenLoc(), funTypeLoc->getRParenLoc());
      wclRewriter_.replaceText(addRange, "(" + parameter + ")");
    }
}

void WebCLTransformer::addRecordArgument(clang::CallExpr *call)
{
  clang::SourceLocation addLoc = wclRewriter_.findLocForNext(call->getLocStart(), '(');
  std::string allocsArg = cfg_.addressSpaceRecordName_;
  if (call->getNumArgs() > 0) {
    allocsArg += ", ";
  }
  clang::SourceRange addRange = clang::SourceRange(addLoc, addLoc);
  allocsArg = wclRewriter_.getOriginalText(addRange) + allocsArg;
  wclRewriter_.replaceText(addRange, allocsArg);
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    const std::string parameter =
        cfg_.sizeParameterType_ + " " + cfg_.getNameOfSizeParameter(decl);
    const std::string replacement =
        wclRewriter_.getOriginalText(decl->getSourceRange()) + ", " + parameter;
    wclRewriter_.replaceText(
        decl->getSourceRange(),
        replacement);
}

bool WebCLTransformer::rewritePrologue()
{
    std::ostringstream out;
    emitPrologue(out);

    clang::SourceManager &manager = instance_.getSourceManager();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    clang::SourceRange range(start, start);
    std::string origStr = wclRewriter_.getTransformedText(range);
    wclRewriter_.replaceText(range, out.str() + origStr);
    return true;
}

bool WebCLTransformer::rewriteKernelPrologue(const clang::FunctionDecl *kernel)
{
    clang::Stmt *body = kernel->getBody();
    if (!body) {
        error(kernel->getLocStart(), "Kernel has no body.");
        return false;
    }
    clang::SourceRange range(body->getLocStart(), body->getLocStart());
    std::string origStr = wclRewriter_.getTransformedText(range) + "\n";
    wclRewriter_.replaceText(range, origStr + functionPrologue(kernelPrologues_, kernel).str());
    return true;
}

void WebCLTransformer::emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl)
{
    emitVarDeclToStruct(out, decl, cfg_.getNameOfRelocatedVariable(decl));
}

void WebCLTransformer::emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl,
                                    const std::string &name)
{
    clang::QualType type = decl->getType();
    clang::Qualifiers qualifiers = type.getQualifiers();
    qualifiers.removeAddressSpace();
  
    const clang::Type *typePtr = type.getTypePtrOrNull();
 
    std::string variable;
    llvm::raw_string_ostream stream(variable);
    clang::PrintingPolicy policy(instance_.getLangOpts());
  
    // dropping qualifiers from array type is pretty hard... there must be better way to do this
    if (type.getTypePtr()->isArrayType()) {
      clang::QualType unqualArray = instance_.getASTContext().getUnqualifiedArrayType(type, qualifiers);
      clang::QualType fixedType = clang::QualType(unqualArray.getTypePtr(), qualifiers);
      const clang::ConstantArrayType *constArr = llvm::dyn_cast<clang::ConstantArrayType>(fixedType.getTypePtr()->getAsArrayTypeUnsafe());
      assert(constArr && "OpenCL can't have non-constant arrays.");
      out << constArr->getElementType().getAsString() << " " << name << "[" << constArr->getSize().getZExtValue() << "]";

    } else {
      clang::QualType::print(typePtr, qualifiers, stream, policy, name);
      out << stream.str();
    }
}

void WebCLTransformer::emitGeneralCode(std::ostream &out)
{
    const char *buffer = reinterpret_cast<const char*>(general_endlfix_cl);
    size_t length = general_endlfix_cl_len;
    const std::string generalClContents(buffer, length);
    out << "\n" << generalClContents << "\n";
}

void WebCLTransformer::emitLimitMacros(std::ostream &out)
{
    for (RequiredMacroSet::iterator i = usedClampMacros_.begin();
         i != usedClampMacros_.end(); i++) {
      
      out << getWclAddrCheckMacroDefinition(i->first, i->second) << "\n";
    }

    out << "\n";
}

std::string WebCLTransformer::getWclAddrCheckMacroDefinition(unsigned aSpaceNum,
                                                             unsigned limitCount)
{
    std::stringstream retVal;
    std::stringstream retValPostfix;
    std::stringstream limitCheckArgs;
    limitCheckArgs << "type, addr, size";
    for (unsigned i = 0; i < limitCount; i++) {
	limitCheckArgs << ", min" << i << ", max" << i;
    }
    retVal  << "#define " << cfg_.getNameOfLimitCheckMacro(aSpaceNum, limitCount)
	    << "(" << limitCheckArgs.str() << ") \\\n"
	    << cfg_.getIndentation(1) << "( 0\\\n";
  
    // at least one of the limits must match
    for (unsigned i = 0; i < limitCount; i++) {
	retVal << cfg_.getIndentation(i + 1) << "|| "
	       << "( "
	       << "((addr) >= ((type)min" << i << "))"
	       << " && "
	       << "((addr + size - 1) <= " << cfg_.getNameOfLimitMacro() << "(type, max" << i << "))"
	       << " ) \\\n";
    }
  
    retVal << cfg_.getIndentation(limitCount + 1) << retValPostfix.str() << " )\n";

    // define clamping macro in terms of the checking macro
    retVal  << "#define " << cfg_.getNameOfLimitClampMacro(aSpaceNum, limitCount)
	    << "(" << limitCheckArgs.str() << ", asnull) \\\n"
	    << cfg_.getIndentation(1) << "( "
	    << cfg_.getNameOfLimitCheckMacro(aSpaceNum, limitCount) 
	    << "(" << limitCheckArgs.str() << ") ? (addr) : (type)(asnull))\n";
  
    return retVal.str();
}

void WebCLTransformer::emitPrologue(std::ostream &out)
{
    out << jsonPrologue_.str();
    out << preModulePrologue_.str();
    out << modulePrologue_.str();
    emitGeneralCode(out);
    emitLimitMacros(out);
    out << afterLimitMacros_.str();
}

void WebCLTransformer::emitTypeNullInitialization(
    std::ostream &out, clang::QualType qualType)
{
    const clang::Type *type = qualType.getTypePtrOrNull();
    if (type && type->isArrayType()) {
      out << "{ ";
      emitTypeNullInitialization(out, type->getAsArrayTypeUnsafe()->getElementType());
      out << " }";
    } else if (type && type->isStructureType()) {
      out << "{ }";
    } else {
      out << "0";
    }
}

void WebCLTransformer::emitVariableInitialization(
    std::ostream &out, const clang::VarDecl *decl)
{
    const clang::Expr *init =  decl->getInit();
  
    if (!init || !init->isConstantInitializer(instance_.getASTContext(), false)) {
        emitTypeNullInitialization(out, decl->getType());
        return;
    }

    const std::string original = wclRewriter_.getTransformedText(init->getSourceRange());
    out << original;
}

void WebCLTransformer::changeFunctionCallee(clang::CallExpr *expr, 
    std::string newName)
{
    clang::Expr *callee = expr->getCallee();
    wclRewriter_.replaceText(callee->getSourceRange(), newName);
}

namespace {
    typedef std::vector<clang::Expr*> ExprVector;
    typedef std::list<std::pair<std::string, std::string> > FunctionArgumentList;

    struct WrappedFunction {
	std::string returnTypeStr_;
	std::string body_;

	WrappedFunction(std::string returnTypeStr, std::string body);
    };

    WrappedFunction::WrappedFunction(std::string returnTypeStr, std::string body) :
	returnTypeStr_(returnTypeStr), body_(body)
    {
	// nothing
    }


    class BuiltinBase {
    public:
	BuiltinBase();

	virtual ~BuiltinBase();

	virtual std::string getName() const = 0;
	virtual unsigned getNumArgs() const = 0;
	
	virtual WrappedFunction wrapFunction(WebCLTransformer &transformer, clang::CompilerInstance &instance, const ExprVector &arguments, WebCLKernelHandler &kernelHandler) const = 0;

    public:
	/// cannot be copied, this code doesn't exist
	BuiltinBase(const BuiltinBase&);

	/// cannot be copied, this code doesn't exist
	void operator=(const BuiltinBase&);
    };

    class VLoad: public BuiltinBase {
    public:
	VLoad(unsigned width);

	std::string getName() const;
	unsigned getNumArgs() const;

	WrappedFunction wrapFunction(WebCLTransformer &transformer, clang::CompilerInstance &instance, const ExprVector &arguments, WebCLKernelHandler &kernelHandler) const;

    private:
	unsigned	width_;
    };

    class VStore: public BuiltinBase {
    public:
	VStore(unsigned width);

	std::string getName() const;
	unsigned getNumArgs() const;

	WrappedFunction wrapFunction(WebCLTransformer &transformer, clang::CompilerInstance &instance, const ExprVector &arguments, WebCLKernelHandler &kernelHandler) const;

    private:
	unsigned	width_;
    };

    BuiltinBase::BuiltinBase()
    {
	// nothing    
    }

    BuiltinBase::~BuiltinBase()
    {
	// nothing    
    }

    VLoad::VLoad(unsigned width) :
	width_(width)
    {
	// nothing
    }

    std::string VLoad::getName() const
    {
	std::stringstream ss;
	ss << "vload" << width_;
	return ss.str();
    }

    unsigned VLoad::getNumArgs() const
    {
	return 2;
    }

    std::string functionDeclaration(
	std::string returnType,
	std::string name,
	FunctionArgumentList arguments)
    {
	std::stringstream result;

	result << returnType << " " << name << "(";
	for (FunctionArgumentList::const_iterator argIt = arguments.begin();
	     argIt != arguments.end();
	     ++argIt) {
	    if (argIt != arguments.begin()) {
		result << ", ";
	    }
	    result << argIt->first << " " << argIt->second;
	}
	result << ")";

	return result.str();
    }

    std::string wrappedDeclaration(
	clang::CompilerInstance &instance, 
	std::string returnTypeStr,
	const clang::CallExpr *callExpr, 
	std::string name)
    {
	WebCLConfiguration cfg;

	FunctionArgumentList newArguments;
	newArguments.push_back(std::make_pair(cfg.addressSpaceRecordType_ + "*", cfg.addressSpaceRecordName_));
	for (size_t argIdx = 0; argIdx < callExpr->getNumArgs(); ++argIdx) {
	    newArguments.push_back(std::make_pair(
		    WebCLTypes::reduceType(instance, callExpr->getArg(argIdx)->getType()).getAsString(),
		    "arg" + stringify(argIdx)));
	}
	
	return functionDeclaration(returnTypeStr, name, newArguments);
    }

    WrappedFunction VLoad::wrapFunction(WebCLTransformer &transformer, clang::CompilerInstance &instance, const ExprVector &arguments, WebCLKernelHandler &kernelHandler) const
    {
	WebCLConfiguration cfg;

	clang::Expr *pointerArg = arguments[1];
	std::string ptrTypeStr = WebCLTypes::reduceType(instance, pointerArg->getType()).getAsString();
	std::string pointeeTypeStr = WebCLTypes::reduceType(instance, pointerArg->getType().getTypePtr()->getPointeeType()).getAsString();
	
	AddressSpaceLimits &limits = kernelHandler.getDerefLimits(pointerArg);

	std::string indent = cfg.getIndentation(1);
	std::string indent__ = cfg.getIndentation(2);
	std::stringstream body;
	std::string zeroValue;
	if (!WebCLTypes::initialZeroValues().count(pointeeTypeStr)) {
	    transformer.error(arguments[1]->getLocStart(), ("Cannot find default zero initializer for type " + pointeeTypeStr).c_str());
	} else {
	    zeroValue = WebCLTypes::initialZeroValues().find(pointeeTypeStr)->second;
	}
	body
	    << indent << ptrTypeStr << " ptr = arg1 + " << width_ << " * (size_t) arg0;\n"
	    << indent << "if (" << transformer.getCheckMacroCall(WebCLTransformer::MACRO_CHECK, "ptr", ptrTypeStr, width_, limits) << ")\n"
	    << indent__ << "return vload" << width_ << "(0, ptr);\n"
	    << indent << "else\n"
	    << indent__ << "return " << zeroValue << ";\n";

	return WrappedFunction(pointeeTypeStr, body.str());
    }

    VStore::VStore(unsigned width) :
	width_(width)
    {
	// nothing
    }

    std::string VStore::getName() const
    {
	std::stringstream ss;
	ss << "vstore" << width_;
	return ss.str();
    }

    unsigned VStore::getNumArgs() const
    {
	return 3;
    }

    WrappedFunction VStore::wrapFunction(WebCLTransformer &transformer, clang::CompilerInstance &instance, const ExprVector &arguments, WebCLKernelHandler &kernelHandler) const
    {
	WebCLConfiguration cfg;

	clang::Expr *pointerArg = arguments[2];
	std::string ptrTypeStr = WebCLTypes::reduceType(instance, pointerArg->getType()).getAsString();
	std::string pointeeTypeStr = WebCLTypes::reduceType(instance, pointerArg->getType().getTypePtr()->getPointeeType()).getAsString();
	
	AddressSpaceLimits &limits = kernelHandler.getDerefLimits(pointerArg);

	std::string indent = cfg.getIndentation(1);
	std::string indent__ = cfg.getIndentation(2);
	std::stringstream body;
	body
	    << indent << ptrTypeStr << " ptr = arg2 + " << width_ << " * (size_t) arg1;\n"
	    << indent << "if (" << transformer.getCheckMacroCall(WebCLTransformer::MACRO_CHECK, "ptr", ptrTypeStr, width_, limits) << ")\n"
	    << indent__ << "vstore" << width_ << "(arg0, 0, ptr);\n";

	return WrappedFunction(pointeeTypeStr, body.str());
    }
}

bool WebCLTransformer::wrapBuiltinFunction(std::string wrapperName, clang::CallExpr *expr, WebCLKernelHandler &kernelHandler)
{
    const clang::FunctionDecl *callee = expr->getDirectCallee();
    bool wrapped = false;
    
    typedef std::map<std::string, BuiltinBase*> BuiltinBaseMap;

    BuiltinBaseMap handlers;

    BuiltinBase *handler;

    for (UintList::const_iterator widthIt = cfg_.dataWidths_.begin();
	 widthIt != cfg_.dataWidths_.end();
	 ++widthIt) {
	handler = new VLoad(*widthIt); handlers[handler->getName()] = handler;
	handler = new VStore(*widthIt); handlers[handler->getName()] = handler;
    }

    std::string name = callee->getNameInfo().getAsString();

    BuiltinBaseMap::const_iterator handlerIt = handlers.find(name);
    if (handlerIt != handlers.end()) {
	WrappedFunction result =
	    handlerIt->second->wrapFunction(
		*this, instance_,
		ExprVector(expr->getArgs(), expr->getArgs() + expr->getNumArgs()),
		kernelHandler);

	afterLimitMacros_ << wrappedDeclaration(instance_, result.returnTypeStr_, expr, wrapperName) << "\n";

	afterLimitMacros_ << "{\n" << result.body_ << "}\n";

	changeFunctionCallee(expr, wrapperName);
	addRecordArgument(expr);

	wrapped = true;
    }

    for (BuiltinBaseMap::iterator it = handlers.begin();
	 it != handlers.end();
	 ++it) {
	delete it->second;
    }

    return wrapped;
}
