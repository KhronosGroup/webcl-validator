#include "WebCLTransformation.hpp"
#include "WebCLTransformer.hpp"
#include "general.h"

#include "WebCLDebug.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"

WebCLTransformer::WebCLTransformer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : WebCLReporter(instance)
    , isFilteredRangesDirty_(false)
    , cfg_()
    , transformations_(instance, rewriter, cfg_)
{
}

WebCLTransformer::~WebCLTransformer()
{
}

bool WebCLTransformer::rewrite()
{
    if (!checkIdentifiers())
        return false;

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
  
    clang::Rewriter &rewriter = transformations_.getRewriter();
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
        prologue << kernelPrologue(func).str();
      }
      if (functionPrologues_.count(func) > 0) {
        prologue << functionPrologue(func).str();
      }
      rewriter.InsertTextAfter(body->getLocStart().getLocWithOffset(1), prologue.str());
    }

    status = status && rewriteTransformations();

    return status;
}

std::string WebCLTransformer::addressSpaceInfoAsStruct(AddressSpaceInfo &as) {
  std::stringstream retVal;
  retVal << "{\n";
  for (AddressSpaceInfo::iterator declIter = as.begin();
       declIter != as.end(); ++declIter) {
    retVal << cfg_.indentation_;
    emitVariable(retVal, (*declIter));
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
    // TODO: check if this works at all for private address space...
    //       i.e. if initialization is value of some other variable..
    //       maybe we just have to output zero initializer and assert
    //       to struct/table initialization.

    emitVariableInitialization(retVal, (*declIter));
    comma = ", ";
  }
  retVal << " }";

  DEBUG( std::cerr << "Created address space initializer: " << retVal.str() << "\n"; );
  return retVal.str();
}

void WebCLTransformer::createAddressSpaceNullInitializer(
    std::ostream &out, unsigned addressSpace)
{
    if (addressSpace != 0)
        return;
    out << "0";
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
        emitVariable(retVal, decl, cfg_.getNameOfLimitField(decl, false));
        retVal << ";\n";

        retVal << cfg_.indentation_;
        emitVariable(retVal, decl, cfg_.getNameOfLimitField(decl, true));
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

        default:
            assert(asLimits.getAddressSpace() == 0);
            break;
        }
    }

    for (AddressSpaceLimits::LimitList::iterator declIter = asLimits.getDynamicLimits().begin();
         declIter != asLimits.getDynamicLimits().end(); ++declIter) {

        retVal << comma;
        clang::ParmVarDecl *decl = *declIter;
        clang::FunctionDecl *func = llvm::dyn_cast<clang::FunctionDecl>(decl->getParentFunctionOrMethod());

        assert(func);
    
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
    out << cfg_.getNameOfAddressSpaceNull(addressSpace);
}

void WebCLTransformer::createAddressSpaceTypedef(
    AddressSpaceInfo &as, const std::string &name)
{
    if (!as.empty()) {
        modulePrologue_ << "typedef struct "
                        << addressSpaceInfoAsStruct(as)
                        << " " << name << ";\n\n";
    }
}

void WebCLTransformer::createPrivateAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.privateRecordType_);
}

void WebCLTransformer::createLocalAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.localRecordType_);
}

void WebCLTransformer::createConstantAddressSpaceTypedef(AddressSpaceInfo &as)
{
    createAddressSpaceTypedef(as, cfg_.constantRecordType_);
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
    kernelPrologue(kernelFunc) << "\n" << cfg_.indentation_
                               << "__" << cfg_.localAddressSpace_ << " "
                               << cfg_.localRecordType_ << " "
                               << cfg_.localRecordName_ << ";\n";
}

void WebCLTransformer::createAddressSpaceLimitsInitializer(
    std::ostream &out, clang::FunctionDecl *kernel, AddressSpaceLimits &limits)
{
    out << cfg_.getIndentation(2) << addressSpaceLimitsInitializer(kernel, limits);
    out << ",\n" << cfg_.getIndentation(2);
    createAddressSpaceLimitsNullInitializer(out, limits.getAddressSpace());
}

void WebCLTransformer::createAddressSpaceInitializer(
    std::ostream& out, AddressSpaceInfo &info)
{
    out << cfg_.getIndentation(2) << addressSpaceInitializer(info);
    out << ",\n" << cfg_.getIndentation(2);
    createAddressSpaceNullInitializer(out, 0);
}

void WebCLTransformer::createProgramAllocationsAllocation(
    clang::FunctionDecl *kernelFunc, AddressSpaceLimits &globalLimits,
    AddressSpaceLimits &constantLimits, AddressSpaceLimits &localLimits,
    AddressSpaceInfo &privateAs)
{
    std::ostream &out = kernelPrologue(kernelFunc);

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
      out << cfg_.indentation_ << "{ }\n";
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
    createAddressSpaceNullAllocation(kernelPrologue(kernel), clang::LangAS::opencl_local);
}

void WebCLTransformer::createGlobalAddressSpaceNullAllocation(clang::FunctionDecl *kernel)
{
    std::ostream &out = kernelPrologue(kernel);
    out << cfg_.indentation_
        << "__" << cfg_.getNameOfAddressSpace(clang::LangAS::opencl_global) << " "
        << cfg_.nullType_ << " *" << cfg_.getNameOfAddressSpaceNull(clang::LangAS::opencl_global)
        << " = 0; // largest global area\n";
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

    std::ostream &out = kernelPrologue(kernelFunc);

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

void WebCLTransformer::replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl) {
  clang::Rewriter &rewriter = transformations_.getRewriter();
  std::string relocatedRef = cfg_.getReferenceToRelocatedVariable(decl);
  std::string original = rewriter.getRewrittenText(use->getSourceRange());
  DEBUG( std::cerr << "Replacing: " << original << " with: " << relocatedRef << "\n"; );
  replaceText(use->getSourceRange(), relocatedRef);
}

void WebCLTransformer::removeRelocated(clang::VarDecl *decl)
{
  removeText(decl->getSourceRange());
}

std::string WebCLTransformer::getClampMacroCall(std::string addr, std::string type, AddressSpaceLimits &limits) {
  
  std::stringstream retVal;

  const unsigned limitCount = limits.count();
  const unsigned addressSpace = limits.getAddressSpace();

  retVal << cfg_.getNameOfLimitCheckMacro(addressSpace, limitCount)
         << "(" << type << ", " << addr;

  if (limits.hasStaticallyAllocatedLimits()) {
    retVal << ", " << cfg_.getStaticLimitRef(addressSpace);
  }

  for (AddressSpaceLimits::LimitList::iterator i = limits.getDynamicLimits().begin();
       i != limits.getDynamicLimits().end(); i++) {
    retVal << ", " << cfg_.getDynamicLimitRef(*i);
  }

  retVal << ", " << cfg_.getNameOfAddressSpaceNullPtrRef(addressSpace) << ")";

  // add macro implementations afterwards
  usedClampMacros_.insert(ClampMacroKey(limits.getAddressSpace(), limitCount));
  
  return retVal.str();
}

void WebCLTransformer::addMemoryAccessCheck(clang::Expr *access, AddressSpaceLimits &limits) {
  
  clang::Expr *base = access;
  clang::Expr *index = NULL;
  std::string field;
  
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

  clang::Rewriter &rewriter = transformations_.getRewriter();

  std::stringstream memAddress;
  clang::SourceRange baseRange = clang::SourceRange(base->getLocStart(), base->getLocEnd());
  const std::string original = rewriter.getRewrittenText(access->getSourceRange());
  const std::string baseStr = getTransformedText(baseRange);

  // TODO: get strings from our rewriter instead of rewriter to get also nested transformations / relocations to work
  memAddress << "(" << baseStr << ")";
  std::string indexStr;
  if (index) {
    indexStr = getTransformedText(index->getSourceRange());
    memAddress << "+(" << indexStr << ")";
  }
  
  // trust limits given in parameter or check against all limits
  std::string safeAccessMacro = getClampMacroCall(memAddress.str(), base->getType().getAsString(), limits);
  
  std::stringstream retVal;
  retVal << "(*(" << safeAccessMacro << "))";
  if (!field.empty()) {
    retVal << "." << field;
  }
  
  DEBUG(
    std::cerr << "Creating memcheck for: " << original
              << "\n                 base: " << baseStr
              << "\n                index: " << indexStr
              << "\n          replacement: " << retVal.str()
              << "\n----------------------------\n";
    access->dump();
    std::cerr << "============================\n\n"; );
  
  replaceText(access->getSourceRange(), retVal.str());
  DEBUG( std::cerr << "============================\n\n"; );
}

/// Adds to function prologue assignment from function argument to relocated variable.
///
/// if reloacted variable was function argument, add initialization row
/// to start of function
/// i.e.
/// void foo(int arg) { bar(&arg); } ->
/// void foo(WclProgramAllocations *wcl_allocs, int arg) {
///     wcl_allocs->pa.foo__arg = arg;
///     bar(&wcl_acllocs->pa.foo__arg); }
///
void WebCLTransformer::addRelocationInitializerFromFunctionArg(clang::ParmVarDecl *parmDecl) {
  const clang::FunctionDecl *parent = llvm::dyn_cast<const clang::FunctionDecl>(parmDecl->getParentFunctionOrMethod());
  // add only once
  if (parameterRelocationInitializations_.count(parmDecl) == 0) {
    functionPrologue(parent) << "\n" << cfg_.getReferenceToRelocatedVariable(parmDecl) << " = " << parmDecl->getNameAsString() << ";\n";
    parameterRelocationInitializations_.insert(parmDecl);
  }
}

/// Adds initialization for relocated private variable after original variable initialization.
///
/// e.g. int foo = 1; ===> int foo = 1; _wcl_allocs->pa.helper_function_name__foo = foo;
/// compiler should afterwards optimize these.
void WebCLTransformer::addRelocationInitializer(clang::VarDecl *decl) {

  clang::SourceLocation addLoc = findLocForNext(decl->getLocEnd(), ';');
  clang::SourceRange replaceRange(addLoc, addLoc);
  std::stringstream inits;
  inits << getTransformedText(replaceRange) << ";";

  // we have to assign arrays with memcpy
  if (decl->getType().getTypePtr()->isArrayType()) {
    inits << "_WCL_MEMCPY(" << cfg_.getReferenceToRelocatedVariable(decl) << "," << decl->getNameAsString() << ");";
  } else {
    inits << cfg_.getReferenceToRelocatedVariable(decl) << " = " << decl->getNameAsString() << ";";
  }
  replaceText(replaceRange, inits.str());

}

void WebCLTransformer::moveToModulePrologue(clang::Decl *decl) {
  clang::Rewriter &rewriter = transformations_.getRewriter();
  const std::string typedefText = rewriter.getRewrittenText(decl->getSourceRange());
  removeText(decl->getSourceRange());
  modulePrologue_ << typedefText << ";\n\n";
}

void WebCLTransformer::flushQueuedTransformations() {
  
  clang::Rewriter &rewriter = transformations_.getRewriter();
  
  clang::tooling::Replacements replacements;
  // go through modifications and replace them to source
  for (RangeModificationsFilter::iterator i = filteredModifiedRanges().begin();
       i != filteredModifiedRanges().end(); i++) {
    
    clang::SourceLocation startLoc =
        clang::SourceLocation::getFromRawEncoding(i->first);
    clang::SourceLocation endLoc =
        clang::SourceLocation::getFromRawEncoding(i->second);
    replacements.insert(clang::tooling::Replacement(rewriter.getSourceMgr(),
                                                    clang::CharSourceRange::getTokenRange(startLoc, endLoc),
                                                    modifiedRanges_[*i]));
  }

  clang::tooling::applyAllReplacements(replacements, rewriter);
}

void WebCLTransformer::addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                                             unsigned minWidthInBits)
{
    modulePrologue_ << "#define " << cfg_.getNameOfSizeMacro(addressSpace) << " ("
                    << "((" << minWidthInBits << " + (CHAR_BIT - 1)) / CHAR_BIT)"
                    << ")\n";
}

void WebCLTransformer::addAddressSpaceNull(std::ostream &out, unsigned addressSpace)
{
    out << "__" << cfg_.getNameOfAddressSpace(addressSpace)
        << " uchar " << cfg_.getNameOfAddressSpaceNull(addressSpace)
        << "[(" << cfg_.getNameOfSizeMacro(addressSpace)
        << " + (sizeof(uchar) - 1)) / sizeof(uchar)] = { '\0' };\n";
}

void WebCLTransformer::addRecordParameter(clang::FunctionDecl *decl)
{
    std::string parameter = cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;

    if (decl->getNumParams() > 0) {
      clang::SourceLocation addLoc = findLocForNext(decl->getLocStart(), '(');
      clang::SourceRange addRange = clang::SourceRange(addLoc, addLoc);
      std::string replacement = transformations_.getRewriter().getRewrittenText(addRange) + parameter + ", ";
      replaceText(addRange, replacement);
    } else {
      // in case of empty args, we need to replcace content inside parenthesis
      // empty params might be e.g. (void)
      clang::TypeLoc typeLoc = decl->getTypeSourceInfo()->getTypeLoc();
      clang::FunctionTypeLoc *funTypeLoc = llvm::dyn_cast<clang::FunctionTypeLoc>(&typeLoc);
      clang::SourceRange addRange(funTypeLoc->getLParenLoc(), funTypeLoc->getRParenLoc());
      replaceText(addRange, "(" + parameter + ")");
    }
}

void WebCLTransformer::addRecordArgument(clang::CallExpr *call)
{
  clang::SourceLocation addLoc = findLocForNext(call->getLocStart(), '(');
  std::string allocsArg = cfg_.addressSpaceRecordName_;
  if (call->getNumArgs() > 0) {
    allocsArg += ", ";
  }
  clang::SourceRange addRange = clang::SourceRange(addLoc, addLoc);
  allocsArg = transformations_.getRewriter().getRewrittenText(addRange) + allocsArg;
  replaceText(addRange, allocsArg);
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    transformations_.addTransformation(
        decl,
        new WebCLSizeParameterInsertion(
            transformations_, decl));
}

// FUTURE: this text editing should be encapsulated to separate class, since it is really providing the
//         low level support for modifying the sources. Also implemenatation can be made whole lot
//         smarter afterwards

clang::SourceLocation WebCLTransformer::findLocForNext(clang::SourceLocation startLoc, char charToFind) {

  const clang::SourceManager &SM = instance_.getSourceManager();
  std::pair<clang::FileID, unsigned> locInfo = SM.getDecomposedLoc(startLoc);
  bool invalidTemp = false;
  clang::StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
  assert(invalidTemp == false);
  const char *tokenBegin = file.data() + locInfo.second;
  
  clang::Lexer lexer(SM.getLocForStartOfFile(locInfo.first), instance_.getLangOpts(),
                     file.begin(), tokenBegin, file.end());
  
  // just raw find for next semicolon and get SourceLocation
  const char* endOfFile = SM.getCharacterData(SM.getLocForEndOfFile(locInfo.first));
  const char* semiColonPos = SM.getCharacterData(startLoc);
  while (*semiColonPos != charToFind && semiColonPos != endOfFile) { semiColonPos++; };
  
  return lexer.getSourceLocation(semiColonPos);
}

void WebCLTransformer::removeText(clang::SourceRange range) {
  isFilteredRangesDirty_ = true;
  clang::Rewriter &rewriter = transformations_.getRewriter();
  DEBUG( std::cerr << "Remove SourceLoc " << range.getBegin().getRawEncoding() << ":" << range.getEnd().getRawEncoding() << " " << rewriter.getRewrittenText(range) << "\n"; );
  replaceText(range, "");
}

void WebCLTransformer::replaceText(clang::SourceRange range, std::string text) {
  isFilteredRangesDirty_ = true;
  clang::Rewriter &rewriter = transformations_.getRewriter();
  int rawStart = range.getBegin().getRawEncoding();
  int rawEnd = range.getEnd().getRawEncoding();
  modifiedRanges_[ModifiedRange(rawStart, rawEnd)] = text;
  DEBUG( std::cerr << "Replace SourceLoc " << rawStart << ":" << rawEnd << " " << rewriter.getRewrittenText(range) << " with: " << text << "\n"; );
}

WebCLTransformer::RangeModificationsFilter& WebCLTransformer::filteredModifiedRanges() {

  // filter nested ranges out from added range modifications
  if (isFilteredRangesDirty_) {
    filteredModifiedRanges_.clear();

    int currentStart = -1;
    int biggestEnd = -1;
    
    // find out only toplevel ranges without nested areas
    for (RangeModifications::iterator i = modifiedRanges_.begin(); i != modifiedRanges_.end(); i++) {
      
      DEBUG( std::cerr << "Range " << i->first.first << ":" << i->first.second << " = " << i->second << "\n"; );
      
      if (i->first.first < biggestEnd) {
        DEBUG( std::cerr << "Skipping: " << "\n"; );
        continue;
      }
      
      if (currentStart != i->first.first) {
        if (currentStart != -1) {
          // This is a big vague because
          // area size might be zero in location ids
          // if current area cannot overlap earlier or if no earlier or it does not overlap => add area
          if (currentStart != biggestEnd ||
              filteredModifiedRanges_.empty() ||
              currentStart != filteredModifiedRanges_.rbegin()->second) {
            DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
            filteredModifiedRanges_.insert(ModifiedRange(currentStart, biggestEnd));
          }
        }
        currentStart = i->first.first;
      }
      
      biggestEnd = i->first.second;
    }
    
    // if last currentStart, biggestEnd is not nested... this is a bit nasty because source location start and end might be
    // the same which is a big ambigious...
    if (currentStart != biggestEnd ||
        filteredModifiedRanges_.empty() ||
        currentStart != filteredModifiedRanges_.rbegin()->second) {
      DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
      filteredModifiedRanges_.insert(ModifiedRange(currentStart, biggestEnd));
    }    
  }
  isFilteredRangesDirty_ = false;
  return filteredModifiedRanges_;
}

std::string WebCLTransformer::getTransformedText(clang::SourceRange range) {

  // TODO: make sure that transformed text gets also insertions... !!!!!!!! but how in case if the same area has insertion
  //       and replacements? (in that case do not replace insertion)
  
  clang::Rewriter &rewriter = transformations_.getRewriter();

  int rawStart = range.getBegin().getRawEncoding();
  int rawEnd = range.getEnd().getRawEncoding();

  RangeModifications::iterator start =
    modifiedRanges_.lower_bound(ModifiedRange(rawStart, rawStart));
  
  RangeModifications::iterator end =
    modifiedRanges_.upper_bound(ModifiedRange(rawEnd, rawEnd));

  std::string retVal;
  
  // if there is exact match, return it
  if (modifiedRanges_.count(ModifiedRange(rawStart, rawEnd))) {
    retVal = modifiedRanges_[ModifiedRange(rawStart, rawEnd)];
  } else if (start == end) {
    // if no matches and start == end get from rewriter
    retVal = rewriter.getRewrittenText(range);
  } else {

    // splits source rawStart and rawEnd to pieces which are read from either modified areas table or from original source

    // TODO: refactor to use filteredModifiedRanges() like flushing does
    // ===== start of filtering nested modified ranges =====
    int currentStart = -1;
    int biggestEnd = -1;
    std::vector<ModifiedRange> filteredRanges;
    
    // find out only toplevel ranges without nested areas
    for (RangeModifications::iterator i = start; i != end; i++) {

      DEBUG( std::cerr << "Range " << i->first.first << ":" << i->first.second << " = " << i->second << "\n"; );
      
      if (i->first.first < biggestEnd) {
        DEBUG( std::cerr << "Skipping: " << "\n"; );
        continue;
      }
      
      if (currentStart != i->first.first) {
        if (currentStart != -1) {
          // This is a big vague because
          // area size might be zero in location ids
          // if current area cannot overlap earlier or if no earlier or it does not overlap => add area
          if (currentStart != biggestEnd ||
              filteredRanges.empty() ||
              currentStart != filteredRanges.back().second) {
            DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
            filteredRanges.push_back(ModifiedRange(currentStart, biggestEnd));
          }
        }
        currentStart = i->first.first;
      }
      
      biggestEnd = i->first.second;
    }

    // if last currentStart, biggestEnd is not nested... this is a bit nasty because source location start and end might be
    // the same which is a big ambigious...
    if (currentStart != biggestEnd ||
        filteredRanges.empty() ||
        currentStart != filteredRanges.back().second) {
      DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
      filteredRanges.push_back(ModifiedRange(currentStart, biggestEnd));
    }
    
    // ===== end of filtering =====
    
    // NOTE: easier way to do following could be to get first the whole range as string and then just replace
    // filtered modified ranges, whose original length and start index is easy to find out
    
    std::stringstream result;
    // concat pieces from original sources and replacements from rawStart to rawEnd
    int current = rawStart;
    bool offsetStartLoc = false;
    
    for (unsigned i = 0; i < filteredRanges.size(); i++) {
      ModifiedRange range = filteredRanges[i];
      if (range.first != current) {
        // get range from rewriter current, range.first
        clang::SourceLocation startLoc = clang::SourceLocation::getFromRawEncoding(current);
        clang::SourceLocation endLoc = clang::SourceLocation::getFromRawEncoding(range.first);
        int endLocSize = rewriter.getRangeSize(clang::SourceRange(endLoc, endLoc));
        int startLocSize = 0;
        if (offsetStartLoc) {
          startLocSize = rewriter.getRangeSize(clang::SourceRange(startLoc, startLoc));
        }
        // get source and exclude start and end tokens in case if they were already included
        // in modified range.
        std::string source = rewriter.getRewrittenText(clang::SourceRange(startLoc, endLoc));
        DEBUG( std::cerr << "Source (" << startLoc.getRawEncoding() << ":" << endLoc.getRawEncoding() << "): "
                         << source.substr(startLocSize, source.length() - startLocSize - endLocSize) << "\n"; );
        
        result << source.substr(startLocSize, source.length() - startLocSize - endLocSize);
      }
      // get range.first, range.second from our bookkeeping
      result << modifiedRanges_[ModifiedRange(range.first, range.second)];
      current = range.second;
      offsetStartLoc = true;
      DEBUG( std::cerr << "Result (" << range.first << ":" << range.second << "): " << modifiedRanges_[ModifiedRange(range.first, range.second)] << "\n"; );
    }
    
    // get rest from sources if we have not yet rendered until end
    if (current != rawEnd) {
      clang::SourceLocation startLoc = clang::SourceLocation::getFromRawEncoding(current);
      clang::SourceLocation endLoc = clang::SourceLocation::getFromRawEncoding(rawEnd);
      assert(offsetStartLoc);
      int startLocSize = rewriter.getRangeSize(clang::SourceRange(startLoc, startLoc));
      std::string source = rewriter.getRewrittenText(clang::SourceRange(startLoc, endLoc));
      result << source.substr(startLocSize);
      DEBUG( std::cerr << "Result (" << startLoc.getRawEncoding() << ":" << endLoc.getRawEncoding() << "): " << source.substr(startLocSize) << "\n"; );
    }

    retVal = result.str();
  }
  
  std::cerr << "Get SourceLoc " << rawStart << ":" << rawEnd << " : " << retVal << "\n";

  return retVal;
}

bool WebCLTransformer::checkIdentifiers()
{
    clang::ASTContext &context = instance_.getASTContext();
    clang::IdentifierTable &table = context.Idents;
    const int numPrefixes = 3;
    const char *prefixes[numPrefixes] = {
        cfg_.typePrefix_.c_str(),
        cfg_.variablePrefix_.c_str(),
        cfg_.macroPrefix_.c_str()
    };
    const int lengths[numPrefixes] = {
        cfg_.typePrefix_.size(),
        cfg_.variablePrefix_.size(),
        cfg_.macroPrefix_.size()
    };

    bool status = true;

    for (clang::IdentifierTable::iterator i = table.begin(); i != table.end(); ++i) {
        clang::IdentifierInfo *identifier = i->getValue();
        const char *name = identifier->getNameStart();

        static const unsigned int maxLength = 255;
        if (identifier->getLength() > maxLength) {
            error("Identifier '%0' exceeds maximum length of %1 characters.") << name << maxLength;
            status = false;
        }

        for (int p = 0; p < numPrefixes ; ++p) {
            const char *prefix = prefixes[p];

            if (!strncmp(prefix, name, lengths[p])) {
                error("Identifier '%0' uses reserved prefix '%1'.") << name << prefix;
                status = false;
            }
        }
    }

    return status;
}

bool WebCLTransformer::rewritePrologue()
{
    clang::Rewriter &rewriter = transformations_.getRewriter();
    clang::SourceManager &manager = rewriter.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    std::ostringstream out;
    emitPrologue(out);
    // Rewriter returns false on success.
    return !rewriter.InsertTextAfter(start, out.str());
}

bool WebCLTransformer::rewriteKernelPrologue(const clang::FunctionDecl *kernel)
{
    clang::Stmt *body = kernel->getBody();
    if (!body) {
        error(kernel->getLocStart(), "Kernel has no body.");
        return false;
    }
  
    clang::Rewriter &rewriter = transformations_.getRewriter();
    // Rewriter returns false on success.
    return !rewriter.InsertTextAfter(
        body->getLocStart().getLocWithOffset(1), kernelPrologue(kernel).str());
}

bool WebCLTransformer::rewriteTransformations()
{
    bool status = true;
  
  
    status = status && transformations_.rewriteTransformations();

    return status;
}

clang::ParmVarDecl *WebCLTransformer::getDeclarationOfArray(clang::ArraySubscriptExpr *expr)
{
    clang::Expr *base = expr->getBase();
    if (!base)
        return NULL;

    clang::ValueDecl *pruned = pruneValue(base);
    if (!pruned)
        return NULL;

    clang::ParmVarDecl *var = llvm::dyn_cast<clang::ParmVarDecl>(pruned);
    if (!var)
        return NULL;

    if (!transformations_.contains(var))
        return NULL;

    return var;
}

void WebCLTransformer::addCheckedType(CheckedTypes &types, clang::QualType type)
{
    const CheckedType checked(cfg_.getNameOfAddressSpace(type), cfg_.getNameOfType(type));
    types.insert(checked);
}

void WebCLTransformer::emitVariable(std::ostream &out, const clang::VarDecl *decl)
{
    emitVariable(out, decl, cfg_.getNameOfRelocatedVariable(decl));
}

void WebCLTransformer::emitVariable(std::ostream &out, const clang::VarDecl *decl,
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
      assert(constArr && "OpenCL cant have non-constant arrays.");
      out << constArr->getElementType().getAsString() << " " << name << "[" << constArr->getSize().getZExtValue() << "]";

    } else {
      clang::QualType::print(typePtr, qualifiers, stream, policy, name);
      out << stream.str();
    }
}

std::string WebCLTransformer::getWclAddrCheckMacroDefinition(unsigned aSpaceNum,
                                                             unsigned limitCount)
{
  std::stringstream retVal;
  std::stringstream retValPostfix;
  retVal  << "#define " << cfg_.getNameOfLimitCheckMacro(aSpaceNum, limitCount)
          << "(type, addr";
  for (unsigned i = 0; i < limitCount; i++) {
      retVal << ", min" << i << ", max" << i;
  }
  retVal << ", asnull) \\\n";
  retVal << cfg_.indentation_ << "( \\\n";

  for (unsigned i = 0; i < limitCount; i++) {
    retVal << cfg_.getIndentation(i + 1) << "( "
           << "( "
           << "((addr) >= ((type)min" << i << "))"
           << " && "
           << "((addr) <= " << cfg_.getNameOfLimitMacro() << "(type, max" << i << "))"
           << " )"
           << " ? (addr) : \\\n";
    retValPostfix << " )";
  }
  
  retVal << cfg_.getIndentation(limitCount + 1)
         << "((type)(asnull))"
         << retValPostfix.str() << " )";
  
  return retVal.str();
}

void WebCLTransformer::emitGeneralCode(std::ostream &out)
{
    const char *buffer = reinterpret_cast<const char*>(general_cl);
    size_t length = general_cl_len;
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

void WebCLTransformer::emitPrologue(std::ostream &out)
{
    out << modulePrologue_.str();
    emitGeneralCode(out);
    emitLimitMacros(out);
}

/// \brief Emits empty initializer for type.
void WebCLTransformer::emitTypeInitialization(
    std::ostream &out, clang::QualType qualType)
{
    const clang::Type *type = qualType.getTypePtrOrNull();
    if (type && type->isArrayType()) {
      out << "{ ";
      emitTypeInitialization(out, type->getAsArrayTypeUnsafe()->getElementType());
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
        emitTypeInitialization(out, decl->getType());
        return;
    }

    const std::string original = getTransformedText(init->getSourceRange());
    out << original;
}

// WebCLTransformerClient

WebCLTransformerClient::WebCLTransformerClient()
    : transformer_(0)
{
}

WebCLTransformerClient::~WebCLTransformerClient()
{
}

WebCLTransformer& WebCLTransformerClient::getTransformer()
{
    assert(transformer_);
    return *transformer_;
}

void WebCLTransformerClient::setTransformer(WebCLTransformer &transformer)
{
    transformer_ = &transformer;
}
