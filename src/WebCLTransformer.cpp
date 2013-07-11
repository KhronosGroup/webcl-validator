#include "WebCLTransformation.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

WebCLTransformer::WebCLTransformer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : WebCLReporter(instance)
    , cfg_()
    , transformations_(instance, rewriter, cfg_)
{
}

WebCLTransformer::~WebCLTransformer()
{
}

bool WebCLTransformer::rewrite()
{
    bool status = true;

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
  return retVal.str();
}

std::string WebCLTransformer::addressSpaceLimitsAsStruct(AddressSpaceLimits &asLimits) {
  std::stringstream retVal;
  retVal << "{\n";
  
  // if address space has static allocations
  if (asLimits.hasStaticallyAllocatedLimits()) {
    switch (asLimits.getAddressSpace()) {
      case clang::LangAS::opencl_constant:
        retVal << cfg_.indentation_ << "__constant WclConstants *wcl_constant_allocations_min;\n";
        retVal << cfg_.indentation_ << "__constant WclConstants *wcl_constant_allocations_max;\n";
        break;
      case clang::LangAS::opencl_local:
        retVal << cfg_.indentation_ << "__local WclLocals *wcl_locals_min;\n";
        retVal << cfg_.indentation_ << "__local WclLocals *wcl_locals_max;\n";
        break;
      default:
        break;
    }
  }
  
  for (AddressSpaceLimits::LimitList::iterator declIter = asLimits.getDynamicLimits().begin();
       declIter != asLimits.getDynamicLimits().end(); ++declIter) {
    clang::ParmVarDecl *decl = *declIter;
    retVal << cfg_.indentation_;
    emitVariable(retVal, decl);
    retVal << "_min" << ";\n";
    retVal << cfg_.indentation_;
    emitVariable(retVal, decl);
    retVal << "_max" << ";\n";
  }
  retVal << "}";
  return retVal.str();
}

std::string WebCLTransformer::addressSpaceLimitsInitializer(
    clang::FunctionDecl *kernelFunc,AddressSpaceLimits &asLimits) {

  std::stringstream retVal;
  retVal << "{ ";
  std::string comma = "";

  // if address space has static allocations
  if (asLimits.hasStaticallyAllocatedLimits()) {
    switch (asLimits.getAddressSpace()) {
      case clang::LangAS::opencl_constant:
        retVal << "&(&wcl_constant_allocations)[0],&(&wcl_constant_allocations)[1]";
        comma = ",";
        break;
      case clang::LangAS::opencl_local:
        retVal << "&(&wcl_locals)[0],&(&wcl_locals)[1]";
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
      retVal << "&" << decl->getNameAsString() << "[0],";
      retVal << "&" << decl->getNameAsString() << "[wcl_" + decl->getNameAsString()  + "_size]";
    } else {
      retVal << "0,0";
    }
    
    comma = ",";
  }
  retVal << " }";
  return retVal.str();
}


void WebCLTransformer::createPrivateAddressSpaceTypedef(AddressSpaceInfo &as) {
  if (!as.empty()) {
    modulePrologue_ << "typedef struct " << addressSpaceInfoAsStruct(as) << " WclPrivates;\n\n";
  }
}

void WebCLTransformer::createLocalAddressSpaceTypedef(AddressSpaceInfo &as) {
  if (!as.empty()) {
    modulePrologue_ << "typedef struct " << addressSpaceInfoAsStruct(as) << " WclLocals;\n\n";
  }
}

void WebCLTransformer::createConstantAddressSpaceTypedef(AddressSpaceInfo &as) {
  if (!as.empty()) {
    modulePrologue_ << "typedef struct " << addressSpaceInfoAsStruct(as) << " WclConstants;\n\n";
  }
}

void WebCLTransformer::createConstantAddressSpaceAllocation(AddressSpaceInfo &as) {
  if (!as.empty()) {
    modulePrologue_ << "__constant WclConstants wcl_constant_allocations = "
                    << addressSpaceInitializer(as) << ";\n\n"; }
}

void WebCLTransformer::createGlobalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits) {
  modulePrologue_ << "typedef struct " << addressSpaceLimitsAsStruct(asLimits)
                  << " WclGlobalLimits;\n\n";
}

void WebCLTransformer::createConstantAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits) {
  modulePrologue_ << "typedef struct " << addressSpaceLimitsAsStruct(asLimits)
                  << " WclConstantLimits;\n\n";
}

void WebCLTransformer::createLocalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits) {
  modulePrologue_ << "typedef struct " << addressSpaceLimitsAsStruct(asLimits)
                  << " WclLocalLimits;\n\n";
}

void WebCLTransformer::createProgramAllocationsTypedef(AddressSpaceLimits &globalLimits,
                                     AddressSpaceLimits &constantLimits,
                                     AddressSpaceLimits &localLimits,
                                     AddressSpaceInfo &privateAs) {
  modulePrologue_ << "typedef struct {\n";
  if (!globalLimits.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclGlobalLimits gl;\n";
  }
  if (!constantLimits.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclConstantLimits cl;\n";
  }
  if (!localLimits.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclLocalLimits ll;\n";
  }
  if (!privateAs.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclPrivates pa;\n";
  }
  modulePrologue_ << " } WclProgramAllocations;\n\n";
}

void WebCLTransformer::createLocalAddressSpaceAllocation(clang::FunctionDecl *kernelFunc) {
  kernelPrologue(kernelFunc) << "\n" << cfg_.indentation_ << "__local WclLocals wcl_locals;\n";
}

void WebCLTransformer::createProgramAllocationsAllocation(clang::FunctionDecl *kernelFunc,
                                        AddressSpaceLimits &globalLimits,
                                        AddressSpaceLimits &constantLimits,
                                        AddressSpaceLimits &localLimits,
                                        AddressSpaceInfo &privateAs) {

  kernelPrologue(kernelFunc) << "\n" << cfg_.indentation_  << "WclProgramAllocations wcl_allocations_allocation = {\n";
  bool hasPrev = false;
  if (!globalLimits.empty()) {
    kernelPrologue(kernelFunc) << cfg_.indentation_ << cfg_.indentation_
                               << addressSpaceLimitsInitializer(kernelFunc, globalLimits);
    hasPrev = true;
  }
  if (!constantLimits.empty()) {
    if (hasPrev) kernelPrologue(kernelFunc) << ",\n";
    kernelPrologue(kernelFunc) << cfg_.indentation_ << cfg_.indentation_
                               << addressSpaceLimitsInitializer(kernelFunc, constantLimits);
    hasPrev = true;
  }
  if (!localLimits.empty()) {
    if (hasPrev) kernelPrologue(kernelFunc) << ",\n";
    kernelPrologue(kernelFunc) << cfg_.indentation_ << cfg_.indentation_
                               << addressSpaceLimitsInitializer(kernelFunc, localLimits);
    hasPrev = true;
  }
  if (!privateAs.empty()) {
    if (hasPrev) kernelPrologue(kernelFunc) << ",\n";
    kernelPrologue(kernelFunc) << cfg_.indentation_ << cfg_.indentation_
                               << addressSpaceInitializer(privateAs) << "\n";
  }
  kernelPrologue(kernelFunc) << cfg_.indentation_ << "};\n";
  kernelPrologue(kernelFunc) << cfg_.indentation_ << "WclProgramAllocations *wcl_allocs = &wcl_allocations_allocation;\n";

}

void WebCLTransformer::createLocalAreaZeroing(clang::FunctionDecl *kernelFunc,
                            AddressSpaceLimits &localLimits) {
  kernelPrologue(kernelFunc)
      << cfg_.indentation_
      << "// TODO: Creating local memory cleanup macro call for every limit range\n";
}

void WebCLTransformer::replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl) {
  clang::Rewriter &rewriter = transformations_.getRewriter();
  std::string relocatedRef = cfg_.getReferenceToRelocatedVariable(decl);
  std::string original = rewriter.getRewrittenText(use->getSourceRange());
  std::cerr << "Replacing: " << original << " with: " << relocatedRef << "\n";
  replaceText(use->getSourceRange(), relocatedRef);
}

std::string WebCLTransformer::getClampMacroCall(std::string addr, std::string type, AddressSpaceLimits &limits) {
  
  std::stringstream retVal;

  unsigned limitCount = limits.count();
  
  retVal << "WCL_ADDR_" << cfg_.getNameOfAddressSpace(limits.getAddressSpace())
         << "_" << limitCount << "(" << type << ", " << addr;
  
  if (limits.hasStaticallyAllocatedLimits()) {
    retVal << ", " << cfg_.getStaticLimitRef(limits.getAddressSpace());
  }

  for (AddressSpaceLimits::LimitList::iterator i = limits.getDynamicLimits().begin();
       i != limits.getDynamicLimits().end(); i++) {
    retVal << ", " << cfg_.getDynamicLimitRef(*i);
  }
  
  retVal << ")";

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
  // NOTE: source end locations are wrong after replacements...
  clang::SourceRange baseRange = clang::SourceRange(base->getLocStart(), base->getLocStart().getLocWithOffset(1));
  const std::string original = rewriter.getRewrittenText(access->getSourceRange());
  const std::string baseStr = rewriter.getRewrittenText(baseRange);

  memAddress << "(" << baseStr << ")";
  std::string indexStr;
  if (index) {
    indexStr = rewriter.getRewrittenText(index->getSourceRange());
    memAddress << "+(" << indexStr << ")";
  }
  
  // trust limits given in parameter or check against all limits
  std::string safeAccessMacro = getClampMacroCall(memAddress.str(), base->getType().getAsString(), limits);
  
  std::stringstream retVal;
  retVal << "(*(" << safeAccessMacro << "))";
  if (!field.empty()) {
    retVal << "." << field;
  }

  std::cerr << "Creating memcheck for: " << original
            << "\n                 base: " << baseStr
            << "\n                index: " << indexStr
            << "\n          replacement: " << retVal.str()
            << "\n----------------------------\n";

  replaceText(access->getSourceRange(), retVal.str());
}

void WebCLTransformer::addRelocationInitializer(clang::ParmVarDecl *parmDecl) {
  const clang::FunctionDecl *parent = llvm::dyn_cast<const clang::FunctionDecl>(parmDecl->getParentFunctionOrMethod());
  // add only once
  if (parameterRelocationInitializations_.count(paramDecl) == 0) {
    functionPrologue(parent) << "\n" << cfg_.getReferenceToRelocatedVariable(parmDecl) << " = " << parmDecl->getNameAsString() << ";\n";
    parameterRelocationInitializations_.insert(paramDecl);
  }
}

void WebCLTransformer::moveToModulePrologue(clang::Decl *decl) {
  clang::Rewriter &rewriter = transformations_.getRewriter();
  const std::string typedefText = rewriter.getRewrittenText(decl->getSourceRange());
  removeText(decl->getSourceRange());
  modulePrologue_ << typedefText << ";\n\n";
}

void WebCLTransformer::flushQueuedTransformations() {
  
  clang::Rewriter &rewriter = transformations_.getRewriter();
  
  // if we replace / remove after insert, rewriter seems to fail
  // transformation applying order 1. removals, 2. replacements, 3. insertions
  for (RemovalContainer::iterator iter = removals_.begin();
       iter != removals_.end(); iter++) {
    rewriter.RemoveText(clang::SourceRange(iter->first, iter->second));
  }
  for (ReplacementContainer::iterator iter = replacements_.begin();
       iter != replacements_.end(); iter++) {
    rewriter.ReplaceText(clang::SourceRange(iter->first.first, iter->first.second), iter->second );
  }
  for (InsertionContainer::iterator iter = inserts_.begin();
       iter != inserts_.end(); iter++) {
    rewriter.InsertText(iter->first, iter->second);
  }
  
  removals_.clear();
  replacements_.clear();
  inserts_.clear();
}

void WebCLTransformer::addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                                             unsigned minWidthInBits) {
    modulePrologue_ << "#define WCL_ADDRESS_SPACE_" << cfg_.getNameOfAddressSpace(addressSpace) << "_MIN (" << minWidthInBits << ")\n";
}

void WebCLTransformer::addRecordParameter(clang::FunctionDecl *decl)
{
    std::string parameter = cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;

    if (decl->getNumParams() > 0) {
      clang::SourceLocation addLoc = decl->getParamDecl(0)->getLocStart();
      insertText(addLoc, parameter + ", ");
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
  clang::SourceLocation addLoc = call->getRParenLoc();
  std::string allocsArg = "wcl_allocs";
  if (call->getNumArgs() > 0) {
    addLoc = call->getArg(0)->getLocStart();
    allocsArg = "wcl_allocs, ";
  }

  // cant translate yet, since there might be other transformations coming to same position
  insertText(addLoc, allocsArg);
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    transformations_.addTransformation(
        decl,
        new WebCLSizeParameterInsertion(
            transformations_, decl));
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
  
    flushQueuedTransformations();
  
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
      out << constArr->getElementType().getAsString() << " " << cfg_.getNameOfRelocatedVariable(decl) << "[" << constArr->getSize().getZExtValue() << "]";

    } else {
      clang::QualType::print(typePtr, qualifiers, stream, policy, cfg_.getNameOfRelocatedVariable(decl));
      out << stream.str();
    }
}

void WebCLTransformer::emitAddressSpaceRecord(
    std::ostream &out, const VariableDeclarations &variables, const std::string &name)
{
    if (variables.empty()) return;
    out << "typedef struct {\n";

    for (VariableDeclarations::iterator i = variables.begin(); i != variables.end(); ++i) {
        out << cfg_.indentation_;
        emitVariable(out, (*i));
        out << ";\n";
    }

    out << "} " << name << ";\n"
        << "\n";
}

std::string WebCLTransformer::getWclAddrCheckMacroDefinition(unsigned aSpaceNum,
                                                             unsigned limitCount) {

  std::string asNull = cfg_.getNameOfAddressSpaceNullPtrRef(aSpaceNum);
  std::stringstream retVal;
  std::stringstream retValPostfix;
  retVal  << "#define WCL_ADDR_" << cfg_.getNameOfAddressSpace(aSpaceNum) << "_" << limitCount << "(type, addr";
  for (unsigned i = 0; i < limitCount; i++) {
      retVal << ",min" << i << ",max" << i;
  }
  retVal << ") ";

  for (unsigned i = 0; i < limitCount; i++) {
    retVal << "((addr >= ((type)min" << i << ") && addr <= WCL_LAST(type, max" << i << ")) ? (addr) : (";
    retValPostfix << " ))";
  }
  
  retVal << "((type)" << asNull << ") " << retValPostfix.str();
  
  return retVal.str();
}

void WebCLTransformer::emitLimitMacros(std::ostream &out)
{
    out << "#define WCL_LAST(type, ptr) \\\n"
        << cfg_.indentation_ <<  "(((type)ptr)-1)\n\n";

    for (RequiredMacroSet::iterator i = usedClampMacros_.begin();
         i != usedClampMacros_.end(); i++) {
      
      out << getWclAddrCheckMacroDefinition(i->first, i->second) << "\n";
    }

    out << "\n";
}

void WebCLTransformer::emitPrologue(std::ostream &out)
{
    emitLimitMacros(out);
    out << modulePrologue_.str();
}

void WebCLTransformer::emitTypeInitialization(
    std::ostream &out, clang::QualType qualType)
{
    const clang::Type *type = qualType.getTypePtrOrNull();
    if (type && type->isArrayType()) {
        out << "{ 0 }";
        return;
    }

    out << "0";
}

void WebCLTransformer::emitVariableInitialization(
    std::ostream &out, const clang::VarDecl *decl)
{
    const clang::Expr *init =  decl->getInit();
  
    // init->dump();

    if (!init || !init->isConstantInitializer(instance_.getASTContext(), false)) {
        emitTypeInitialization(out, decl->getType());
        return;
    }

    clang::Rewriter &rewriter = transformations_.getRewriter();
    const std::string original = rewriter.getRewrittenText(init->getSourceRange());
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
