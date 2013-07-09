#include "WebCLTransformation.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

WebCLTransformer::WebCLTransformer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : WebCLReporter(instance)
    , checkedPointerTypes_()
    , checkedIndexTypes_()
    , relocatedGlobals_()
    , relocatedLocals_()
    , relocatedConstants_()
    , relocatedPrivates_()
    , kernels_()
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
  
    for (KernelPrologueMap::iterator iter = kernelPrologues_.begin();
         iter != kernelPrologues_.end(); iter++) {
      status = status && rewriteKernelPrologue(iter->first);
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
      retVal << "NULL, NULL";
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
  if (!constantLimits.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclconstantLimits cl;\n";
  }
  if (!globalLimits.empty()) {
    modulePrologue_ << cfg_.indentation_ << "WclGlobalLimits gl;\n";
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
  kernelPrologue(kernelFunc) << cfg_.indentation_ << "// TODO: Creating local memory cleanup macro call for every limit range\n";
}

void WebCLTransformer::replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl) {
  std::cerr << "Replacing: " << use->getNameInfo().getName().getAsString()
            << " with: " << cfg_.getReferenceToRelocatedVariable(decl) << "\n";  
}

std::string WebCLTransformer::getClampMacroCall(std::string addr, std::string type, AddressSpaceLimits &limits) {
  std::stringstream retVal;

  retVal << "WCL_ADDR_" << limits.getAddressSpace() << "_" << limits.count() << "(" << type << ", " << addr;

  if (limits.hasStaticallyAllocatedLimits()) {
    retVal << ", " << "implement_static_min" << ", " << "implement_static_max";
  }

  for (AddressSpaceLimits::LimitList::iterator i = limits.getDynamicLimits().begin();
       i != limits.getDynamicLimits().end(); i++) {
    retVal << ", " << "implement_dyn_min" << ", " << "implement_dyn_max";
  }
  
  retVal << ")";
  
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

  std::stringstream memAddress;
  
  clang::Rewriter &rewriter = transformations_.getRewriter();
  const std::string original = rewriter.getRewrittenText(access->getSourceRange());
  const std::string baseStr = rewriter.getRewrittenText(base->getSourceRange());
  
  memAddress << "(" << baseStr << ")";
  if (index) {
    const std::string indexStr = rewriter.getRewrittenText(index->getSourceRange());
    memAddress << "+(" << indexStr << ")";
  }
  
  std::cerr << "Creating memcheck for: " << original << "\n";
  
  // trust limits given in parameter or check against all limits
  std::string type = access->getType().getAsString() + "*";
  std::string safeAccessMacro = getClampMacroCall(memAddress.str(), type, limits);
  
  std::cerr << "(*(" << safeAccessMacro << "))";
  if (!field.empty()) {
    std::cerr << "." << field;
  }
  std::cerr << "\n";

}

void WebCLTransformer::addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                                             unsigned minWidthInBits) {
  std::cerr << "#define WCL_ADDRESS_SPACE_" << addressSpace << "_MIN (" << minWidthInBits << ")\n";
}

void WebCLTransformer::addKernel(clang::FunctionDecl *decl)
{
    kernels_.insert(decl);
}

void WebCLTransformer::addRelocatedVariable(clang::DeclStmt *stmt, clang::VarDecl *decl)
{
    transformations_.addTransformation(
        decl,
        new WebCLVariableRelocation(transformations_, stmt, decl));
    addRelocatedVariable(decl);
}

void WebCLTransformer::addRecordParameter(clang::FunctionDecl *decl)
{
    transformations_.addTransformation(
        decl,
        new WebCLRecordParameterInsertion(
            transformations_, decl));
}

void WebCLTransformer::addRecordArgument(clang::CallExpr *expr)
{
    transformations_.addTransformation(
        expr,
        new WebCLRecordArgumentInsertion(
            transformations_, expr));
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    transformations_.addTransformation(
        decl,
        new WebCLSizeParameterInsertion(
            transformations_, decl));
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    transformations_.addTransformation(
        expr,
        new WebCLConstantArraySubscriptTransformation(
            transformations_, expr, bound));
}

void WebCLTransformer::addArrayIndexCheck(clang::ArraySubscriptExpr *expr)
{
    if (clang::ParmVarDecl *var = getDeclarationOfArray(expr)) {
        transformations_.addTransformation(
            expr,
            new WebCLKernelArraySubscriptTransformation(
                transformations_, expr, cfg_.getNameOfSizeParameter(var)));
    } else {
        addCheckedType(checkedIndexTypes_, expr->getType());

        transformations_.addTransformation(
            expr,
            new WebCLArraySubscriptTransformation(
                transformations_, expr));
    }
}

void WebCLTransformer::addPointerCheck(clang::Expr *expr)
{
    addCheckedType(checkedPointerTypes_, getPointeeType(expr));

    transformations_.addTransformation(
        expr,
        new WebCLPointerDereferenceTransformation(
            transformations_, expr));
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
/*
    std::ostringstream out;
    emitKernelPrologue(out);
*/
  
    clang::Rewriter &rewriter = transformations_.getRewriter();
    // Rewriter returns false on success.
    return !rewriter.InsertTextAfter(
        body->getLocStart().getLocWithOffset(1), kernelPrologue(kernel).str());
}

bool WebCLTransformer::rewriteTransformations()
{
    bool status = true;

  for (Kernels::iterator i = kernels_.begin(); i != kernels_.end(); ++i) {
        const clang::FunctionDecl *kernel = (*i);
        status = status && rewriteKernelPrologue(kernel);
    }

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

void WebCLTransformer::addRelocatedVariable(clang::VarDecl *decl)
{
    clang::QualType type = decl->getType();
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            relocatedGlobals_.insert(decl);
            return;
        case clang::LangAS::opencl_local:
            relocatedLocals_.insert(decl);
            return;
        case clang::LangAS::opencl_constant:
            relocatedConstants_.insert(decl);
            return;
        default:
            error(decl->getLocStart(), "Unknown address space.");
            return;
        }
    }

    relocatedPrivates_.insert(decl);
}

void WebCLTransformer::emitVariable(std::ostream &out, const clang::VarDecl *decl)
{
    clang::QualType type = decl->getType();
    clang::Qualifiers qualifiers = type.getQualifiers();
    qualifiers.removeAddressSpace();

    std::string variable;
    llvm::raw_string_ostream stream(variable);
    clang::PrintingPolicy policy(instance_.getLangOpts());
    clang::QualType::print(
        type.getTypePtrOrNull(), qualifiers, stream, policy,
        cfg_.getNameOfRelocatedVariable(decl));
    out << stream.str();
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

void WebCLTransformer::emitPrologueRecords(std::ostream &out)
{
    emitAddressSpaceRecord(out, relocatedPrivates_, cfg_.privateRecordType_);
    emitAddressSpaceRecord(out, relocatedLocals_, cfg_.localRecordType_);
    emitAddressSpaceRecord(out, relocatedConstants_, cfg_.constantRecordType_);
    emitAddressSpaceRecord(out, relocatedGlobals_, cfg_.globalRecordType_);

    out << "typedef struct {\n"

        << cfg_.indentation_ << cfg_.privateRecordType_
        << " __" << cfg_.privateAddressSpace_ << " *" << cfg_.privateField_ << ";\n"

        << cfg_.indentation_ << cfg_.localRecordType_
        << " __" << cfg_.localAddressSpace_ << " *" << cfg_.localField_ << ";\n"

        << cfg_.indentation_ << cfg_.constantRecordType_
        << " __" << cfg_.constantAddressSpace_ << " *" << cfg_.constantField_ << ";\n"

        << cfg_.indentation_ << cfg_.globalRecordType_
        << " __" << cfg_.globalAddressSpace_ << " *" << cfg_.globalField_ << ";\n"

        << "} " << cfg_.addressSpaceRecordType_ << ";\n"
        << "\n";
}

void WebCLTransformer::emitLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN(a, b) \\\n"
        << cfg_.indentation_ << "(((a) < (b)) ? (a) : (b))\n"

        << "#define WCL_MAX(a, b) \\\n"
        << cfg_.indentation_ << "(((a) < (b)) ? (b) : (a))\n"

        << "#define WCL_CLAMP(low, value, high) \\\n"
        << cfg_.indentation_ << "WCL_MAX((low), WCL_MIN((value), (high)))\n"
        << "\n"

        << "#define WCL_ADDR(type, ptr, min_ptr, max_ptr) \\\n"
        << cfg_.indentation_ <<  "WCL_CLAMP( ((type)min_ptr), (ptr), (((type)max_ptr)-1) )\n\n";
}

void WebCLTransformer::emitPointerLimitMacros(std::ostream &out)
{
  /*
    out << "#define WCL_MIN_PTR(name, type, field) \\\n"
        << cfg_.indentation_ << "((name type *)(field))\n"

        << "#define WCL_MAX_PTR(name, type, field) \\\n"
        << cfg_.indentation_ << "(WCL_MIN_PTR(name, type, (field) + 1) - 1)\n"
        << "\n";
   */
}

void WebCLTransformer::emitIndexLimitMacros(std::ostream &out)
{
  /*
    out << "#define WCL_MIN_IDX(name, type, field, ptr) \\\n"
        << cfg_.indentation_ << "(size_t)0\n"

        << "#define WCL_MAX_IDX(name, type, field, ptr) \\\n"
        << cfg_.indentation_ << "(size_t)(WCL_MAX_PTR(name, type, field) - ptr)\n"
        << "\n";
   
   */
}

void WebCLTransformer::emitPointerCheckerMacro(std::ostream &out)
{
  /*
    static const std::string functionName =
        cfg_.prefix_ + "_##name##_##type##_" + cfg_.pointerSuffix_;
    static const std::string asParameter =
        cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;
    static const std::string asField =
        cfg_.addressSpaceRecordName_ + "->field";
  
    out << "#define WCL_PTR_CHECKER(name, field, type) \\\n"
        << cfg_.getIndentation(1) << "name type *" << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr);\\\n"
        << cfg_.getIndentation(1) << "name type *" << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr) \\\n"
        << cfg_.getIndentation(1) << "{ \\\n"
        << cfg_.getIndentation(2) << "return WCL_CLAMP( \\\n"
        << cfg_.getIndentation(3) << "WCL_MIN_PTR(name, type, " << asField << "), \\\n"
        << cfg_.getIndentation(3) << "ptr, \\\n"
        << cfg_.getIndentation(3) << "WCL_MAX_PTR(name, type, " << asField << ")); \\\n"
        << cfg_.getIndentation(1) << "}\n"
        << "\n";
   */
}

void WebCLTransformer::emitIndexCheckerMacro(std::ostream &out)
{
  /*
    static const std::string functionName =
        cfg_.prefix_ + "_##name##_##type##_" + cfg_.indexSuffix_;
    static const std::string asParameter =
        cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;
    static const std::string asField =
        cfg_.addressSpaceRecordName_ + "->field";

    out << "#define WCL_IDX_CHECKER(name, field, type) \\\n"
        << cfg_.getIndentation(1) << "size_t " << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr, size_t idx); \\\n"
        << cfg_.getIndentation(1) << "size_t " << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr, size_t idx) \\\n"
        << cfg_.getIndentation(1) << "{ \\\n"
        << cfg_.getIndentation(2) << "return WCL_CLAMP( \\\n"
        << cfg_.getIndentation(3) << "WCL_MIN_IDX(name, type, " << asField << ", ptr), \\\n"
        << cfg_.getIndentation(3) << "idx, \\\n"
        << cfg_.getIndentation(3) << "WCL_MAX_IDX(name, type, " << asField << ", ptr)); \\\n"
        << cfg_.getIndentation(1) << "}\n"
        << "\n";
*/
}

void WebCLTransformer::emitPrologueMacros(std::ostream &out)
{
    emitLimitMacros(out);
    emitPointerLimitMacros(out);
    emitIndexLimitMacros(out);
    emitPointerCheckerMacro(out);
    emitIndexCheckerMacro(out);
}

void WebCLTransformer::emitConstantIndexChecker(std::ostream &out)
{
/*
  out << "size_t " << cfg_.getNameOfIndexChecker() << "(size_t idx, size_t limit);\n";
    out << "size_t " << cfg_.getNameOfIndexChecker() << "(size_t idx, size_t limit)\n"
        << "{\n"
        << cfg_.getIndentation(1) << "return idx % limit;\n"
        << "}\n"
        << "\n";
*/
}

void WebCLTransformer::emitChecker(std::ostream &out, const CheckedType &type,
                                   const std::string &kind)
{
/*
  out << "WCL_" << kind << "_CHECKER("
        << type.first << ", " << type.first << "s, " << type.second
        << ")\n";
*/
 }

void WebCLTransformer::emitCheckers(std::ostream &out, const CheckedTypes &types,
                                    const std::string &kind)
{
/*
  for (CheckedTypes::iterator i = types.begin(); i != types.end(); ++i)
        emitChecker(out, (*i), kind);
    if (types.size())
        out << "\n";
*/
 }

void WebCLTransformer::emitPrologueCheckers(std::ostream &out)
{
/*
    emitConstantIndexChecker(out);
    emitCheckers(out, checkedPointerTypes_, "PTR");
    emitCheckers(out, checkedIndexTypes_, "IDX");
*/
}

void WebCLTransformer::emitPrologue(std::ostream &out)
{
    out << modulePrologue_.str();
/*
    emitPrologueRecords(out);
    emitPrologueMacros(out);
    emitPrologueCheckers(out);
*/
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

void WebCLTransformer::emitRecordInitialization(
    std::ostream &out, const std::string &type, const std::string &name,
    VariableDeclarations &relocations)
{
    out << cfg_.indentation_ << type << " " << name << " = {\n";

    for (VariableDeclarations::iterator i = relocations.begin();
         i != relocations.end(); ++i) {
        if (i != relocations.begin())
            out << ",\n";
        out << cfg_.getIndentation(2);
        const clang::VarDecl *decl = (*i);
        emitVariableInitialization(out, decl);
    }

    out << "\n" << cfg_.indentation_ << "};\n";
}

void WebCLTransformer::emitKernelPrologue(std::ostream &out)
{
    out << "\n";

    std::string privateString = cfg_.privateRecordName_;
    if (relocatedPrivates_.size()) {
        emitRecordInitialization(
            out, cfg_.privateRecordType_, privateString, relocatedPrivates_);
        privateString = "&" + privateString;
    } else {
        privateString = "0";
    }

    std::string localString = cfg_.localRecordName_;
    if (relocatedLocals_.size()) {
        emitRecordInitialization(
            out, cfg_.localRecordType_, localString, relocatedLocals_);
        localString = "&" + localString;
    } else {
        localString = "0";
    }

    std::string constantString = cfg_.constantRecordName_;
    if (relocatedConstants_.size()) {
        emitRecordInitialization(
            out, cfg_.constantRecordType_, constantString, relocatedConstants_);
        constantString = "&" + constantString;
    } else {
        constantString = "0";
    }

    std::string globalString = cfg_.globalRecordName_;
    if (relocatedGlobals_.size()) {
        emitRecordInitialization(
            out, cfg_.globalRecordType_, globalString, relocatedGlobals_);
        globalString = "&" + globalString;
    } else {
        globalString = "0";
    }

    out << cfg_.indentation_
        << cfg_.addressSpaceRecordType_ << " " << cfg_.addressSpaceRecordName_
        << " = { "
        << privateString << ", "
        << localString << ", "
        << constantString << ", "
        << globalString
        << " };\n";
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
