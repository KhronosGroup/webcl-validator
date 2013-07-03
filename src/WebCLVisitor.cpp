#include "WebCLVisitor.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"

#include <iostream>

// WebCLVisitor

WebCLVisitor::WebCLVisitor(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , clang::RecursiveASTVisitor<WebCLVisitor>()
{
}

WebCLVisitor::~WebCLVisitor()
{
}

bool WebCLVisitor::VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl)
{
    return handleTranslationUnitDecl(decl);
}

bool WebCLVisitor::VisitFunctionDecl(clang::FunctionDecl *decl)
{
    return handleFunctionDecl(decl);
}

bool WebCLVisitor::VisitParmVarDecl(clang::ParmVarDecl *decl)
{
    return handleParmVarDecl(decl);
}

bool WebCLVisitor::VisitVarDecl(clang::VarDecl *decl)
{
    return handleVarDecl(decl);
}

bool WebCLVisitor::VisitDeclStmt(clang::DeclStmt *stmt)
{
    return handleDeclStmt(stmt);
}

bool WebCLVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
    return handleArraySubscriptExpr(expr);
}

bool WebCLVisitor::VisitUnaryOperator(clang::UnaryOperator *expr)
{
    return handleUnaryOperator(expr);
}
bool WebCLVisitor::VisitMemberExpr(clang::MemberExpr *expr)
{
  return handleMemberExpr(expr);
}

bool WebCLVisitor::VisitExtVectorElementExpr(clang::ExtVectorElementExpr *expr)
{
  return handleExtVectorElementExpr(expr);
}

bool WebCLVisitor::VisitCallExpr(clang::CallExpr *expr)
{
    return handleCallExpr(expr);
}

bool WebCLVisitor::VisitTypedefDecl(clang::TypedefDecl *decl)
{
  return handleTypedefDecl(decl);
}

bool WebCLVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr)
{
  return handleDeclRefExpr(expr);
}

bool WebCLVisitor::handleTranslationUnitDecl(clang::TranslationUnitDecl *decl)
{
    return true;
}

bool WebCLVisitor::handleFunctionDecl(clang::FunctionDecl *decl)
{
    return true;
}

bool WebCLVisitor::handleParmVarDecl(clang::ParmVarDecl *decl)
{
    return true;
}

bool WebCLVisitor::handleVarDecl(clang::VarDecl *decl)
{
    return true;
}

bool WebCLVisitor::handleDeclStmt(clang::DeclStmt *stmt)
{
    return true;
}

bool WebCLVisitor::handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
    return true;
}

bool WebCLVisitor::handleUnaryOperator(clang::UnaryOperator *expr)
{
    return true;
}

bool WebCLVisitor::handleMemberExpr(clang::MemberExpr *expr)
{
  return true;
}

bool WebCLVisitor::handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr)
{
  return true;
}

bool WebCLVisitor::handleCallExpr(clang::CallExpr *expr)
{
    return true;
}

bool WebCLVisitor::handleTypedefDecl(clang::TypedefDecl *decl)
{
  return true;
}

bool WebCLVisitor::handleDeclRefExpr(clang::DeclRefExpr *expr) {
  return true;
}

// WebCLTransformingVisitor

WebCLTransformingVisitor::WebCLTransformingVisitor(clang::CompilerInstance &instance)
    : WebCLVisitor(instance)
    , WebCLTransformerClient()
{
}

WebCLTransformingVisitor::~WebCLTransformingVisitor()
{
}

// WebCLRestrictor

WebCLRestrictor::WebCLRestrictor(clang::CompilerInstance &instance)
    : WebCLVisitor(instance)
{
}

WebCLRestrictor::~WebCLRestrictor()
{
}

bool WebCLRestrictor::handleFunctionDecl(clang::FunctionDecl *decl)
{
    if (decl->hasAttr<clang::OpenCLKernelAttr>()) {
        const clang::DeclarationNameInfo nameInfo = decl->getNameInfo();
        const clang::IdentifierInfo *idInfo = nameInfo.getName().getAsIdentifierInfo();
        if (!idInfo) {
            error(nameInfo.getLoc(),
                  "Invalid kernel name.\n");
        } else if (idInfo->getLength() > 255) {
            error(nameInfo.getLoc(),
                  "WebCL restricts kernel name lengths to 255 characters.\n");
        }
    }

    return true;
}

bool WebCLRestrictor::handleParmVarDecl(clang::ParmVarDecl *decl)
{ 
    const clang::TypeSourceInfo *info = decl->getTypeSourceInfo();
    if (!info) {
        error(decl->getSourceRange().getBegin(), "Invalid parameter type.\n");
        return true;
    }

    clang::SourceLocation typeLocation = info->getTypeLoc().getBeginLoc();

    const clang::Type *type = info->getType().getTypePtrOrNull();
    if (!info) {
        error(typeLocation, "Invalid parameter type.\n");
        return true;
    }
    const clang::DeclContext *context = decl->getParentFunctionOrMethod();
    if (!context) {
        error(typeLocation, "Invalid parameter context.\n");
        return true;
    }
    clang::FunctionDecl *function = clang::FunctionDecl::castFromDeclContext(context);
    if (!function) {
        error(typeLocation, "Invalid parameter context.\n");
        return true;
    }

    checkStructureParameter(function, typeLocation, type);
    check3dImageParameter(function, typeLocation, type);
    return true;
}

void WebCLRestrictor::checkStructureParameter(
    clang::FunctionDecl *decl,
    clang::SourceLocation typeLocation, const clang::Type *type)
{
    if (decl->hasAttr<clang::OpenCLKernelAttr>() &&
        type->isRecordType()) {
        error(typeLocation, "WebCL doesn't support structures as kernel parameters.\n");
    }
}

void WebCLRestrictor::check3dImageParameter(
    clang::FunctionDecl *decl,
    clang::SourceLocation typeLocation, const clang::Type *type)
{
    if (!isFromMainFile(typeLocation))
        return;

    clang::QualType canonical = type->getCanonicalTypeInternal();
    type = canonical.getTypePtrOrNull();
    if (!type) {
        error(typeLocation, "Invalid canonical type.");
    } else if (type->isPointerType()) {
        clang::QualType pointee = type->getPointeeType();
        if (pointee.getAsString() == "struct image3d_t_")
            error(typeLocation, "WebCL doesn't support 3D images.\n");
    }
}

// WebCLAnalyser

WebCLAnalyser::WebCLAnalyser(clang::CompilerInstance &instance)
: WebCLVisitor(instance)
{
}

WebCLAnalyser::~WebCLAnalyser()
{
}

bool WebCLAnalyser::handleVarDecl(clang::VarDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
 
  switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_local:
      info(decl->getLocStart(), "Local variable declaration. Collect!");
      localVariables_.insert(decl);
      break;
    case clang::LangAS::opencl_constant:
      info(decl->getLocStart(), "Constant variable declaration. Collect!");
      constantVariables_.insert(decl);
      break;
    default:
      assert(decl->getType().getAddressSpace() == 0);
      if (decl->isFunctionOrMethodVarDecl()) {
        info(decl->getLocStart(), "Private variable declaration. Collect!");
        privateVariables_.insert(decl);
      } else {
        info(decl->getLocStart(), "Function parameter... skip for now.");
      }
  }
  
  return true;
  
  if (decl->hasLocalStorage()) {
    if (decl->isFunctionOrMethodVarDecl()) {
      info(decl->getLocStart(), "Private variable declaration. Collect!");
      privateVariables_.insert(decl);
      if (decl->hasInit() && decl->getType().getTypePtr()->isStructureOrClassType() ) {
          // TODO: move this restriction to restrictor...
          info(decl->getInit()->getLocStart(), "Struct got initializer fail for now!");
      }
    } else {
      info(decl->getLocStart(), "Function argument variable! Might be needed so collect these too, but to different bookkeeping than normal local variables.");
    }
  } else if (decl->hasGlobalStorage()) {
    info(decl->getLocStart(), "Global/Local address space variable! Collect to corresponding address space.");
    std::cerr << "######## Woot:" << decl->getType().getAddressSpace() << "\n";
  } else {
    info(decl->getLocStart(), "Filter these, so that only globals are left. And maybe some function arguments.");
  }
  
  return true;
}

bool WebCLAnalyser::handleMemberExpr(clang::MemberExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;

  if (expr->isArrow()) {
    info(expr->getLocStart(), "Pointer access!");
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getBase())) {
      std::cerr << "Jackpot found decl:\n";
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
    }
  }
  return true;
}

bool WebCLAnalyser::handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;

  // handle only arrow accesses
  if (expr->isArrow()) {
    info(expr->getLocStart(), "Pointer access!");
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getBase())) {
      std::cerr << "Jackpot found decl:\n";
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
    }
  }
  return true;
}

bool WebCLAnalyser::handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;
  info(expr->getLocStart(), "Pointer access!");

  clang::Expr *base = expr->getBase();
  clang::Expr *idx = expr->getIdx();
  
  // in case if we are abse to trace actual base declaration we can optimize more in future
  if (clang::ImplicitCastExpr *implicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(base)) {
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(implicitCast->getSubExpr())) {
      std::cerr << "Jackpot found decl:\n";
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
    }
  }
  
   std::cerr << "Base type: " << base->getType().getAsString()
            << " address number space: " << base->getType().getAddressSpace() <<  "\n";

  std::cerr << "Index type: " << idx->getType().getAsString()
            << " address number space: " << idx->getType().getAddressSpace() <<  "\n\n";

  return true;
}

bool WebCLAnalyser::handleUnaryOperator(clang::UnaryOperator *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;

  if (expr->getOpcode() == clang::UO_Deref) {
    clang::Expr *pointer = expr->getSubExpr();
    if (!pointer) {
      error(expr->getLocStart(), "Invalid pointer dereference.\n");
      return false;
    }
    info(expr->getLocStart(), "Pointer access!");
    clang::Expr *addr = expr->getSubExpr();
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getSubExpr())) {
      std::cerr << "Jackpot found decl:\n";
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
    }
    
    std::cerr << "expr type:" << addr->getType().getAsString()
              << " address number space: " << addr->getType().getAddressSpace() <<  "\n\n";
  } else if(expr->getOpcode() == clang::UO_AddrOf) {
    info(expr->getLocStart(), "Address of something, might require some handling.");
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getSubExpr())) {
      std::cerr << "Jackpot found decl:\n";
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      clang::VarDecl *decl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
      assert(decl);
      declarationsWithAddressOfAccess_.insert(decl);
      // also add veriable to address space in this special case
      privateVariables_.insert(decl);
    }
  }
  return true;
}

bool WebCLAnalyser::handleFunctionDecl(clang::FunctionDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
  
  // information for e.g. when running through function arguments to which function they belong
  contextFunction_ = decl;

  if (decl->hasAttr<clang::OpenCLKernelAttr>()) {
    // TODO: go through arguments and collect pointers to bookkeeping
    //       create names for limit struct as:
    //       function__argument_name__min and function__argument_name__max
    //       and organize to lists according to address space of argument
    info(decl->getLocStart(), "This is kernel, go through arguments to collect pointers etc.");
    kernelFunctions_.insert(decl);
  } else {
    info(decl->getLocStart(), "This is prototype/function, add to list to add wcl_allocs arg and add to list to recognize internal functions for call conversion.");
    helperFunctions_.insert(decl);
  }
  return true;
}


bool WebCLAnalyser::handleCallExpr(clang::CallExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;
  info(expr->getLocStart(), "Found call, adding to bookkeeping to decide later if parameterlist needs fixing.");

  if (helperFunctions_.count(expr->getDirectCallee()) > 0) {
    std::cerr << "Looks like it is call to internal function!\n";
    internalCalls_.insert(expr);
  } else {
    std::cerr << "Looks like it is call to builtin!\n";
    builtinCalls_.insert(expr);
  }
  
  return true;
}

bool WebCLAnalyser::handleTypedefDecl(clang::TypedefDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
  info(decl->getLocStart(), "Found typedef, add to list to move to start of source.");
  return true;
}

bool WebCLAnalyser::handleDeclRefExpr(clang::DeclRefExpr *expr) {
  if (!isFromMainFile(expr->getLocStart())) return true;
  info(expr->getLocStart(), "Found variable use!");
  variableUses_.insert(expr);

  // we should be able to get parent map from here:
  // instance_.getASTContext();
  return true;
}


// WebCLRelocator

WebCLRelocator::WebCLRelocator(clang::CompilerInstance &instance)
    : WebCLTransformingVisitor(instance)
    , current_(NULL)
    , relocationCandidates_()
{
}

WebCLRelocator::~WebCLRelocator()
{
}

bool WebCLRelocator::handleDeclStmt(clang::DeclStmt *stmt)
{
    current_ = stmt;
    return true;
}

bool WebCLRelocator::handleVarDecl(clang::VarDecl *decl)
{
  
    if (!decl->isLocalVarDecl()) {
      return true;
    }
  
    const clang::Type *type = decl->getType().getTypePtrOrNull();
    if (!type) {
        error(decl->getLocStart(), "Invalid variable type.");
        return true;
    }

    // In global scope 'current_' is NULL.

  
    if (type->isConstantArrayType()) {
        getTransformer().addRelocatedVariable(current_, decl);
        return true;
    }

    RelocationCandidate candidate(decl, current_);
    relocationCandidates_.insert(candidate);
    return true;
}

bool WebCLRelocator::handleUnaryOperator(clang::UnaryOperator *expr)
{
  // TODO: why would we handle unary operator here...? to see if
  //       argument is passed as address somewhere?
  if (!isFromMainFile(expr->getLocStart()))
        return true;

    if (expr->getOpcode() != clang::UO_AddrOf)
        return true;

    clang::Expr *variable = expr->getSubExpr();
    if (!variable) {
        return true;
    }

    clang::VarDecl *relocated = getRelocatedVariable(variable);
    if (!relocated)
        return true;

//    RelocationCandidates::iterator i = relocationCandidates_.find(relocated);
//    if (i == relocationCandidates_.end()) {
//        error(relocated->getLocStart(), "Haven't seen enclosing statement yet.");
//        return true;
//    }
//
//    clang::DeclStmt *stmt = i->second;
//    getTransformer().addRelocatedVariable(stmt, relocated);
    return true;
}

bool WebCLRelocator::handleFunctionDecl(clang::FunctionDecl *decl)
{
  if (decl->hasAttr<clang::OpenCLKernelAttr>()) {
    // TODO: go through arguments and collect pointers to bookkeeping
    //       create names for limit struct as:
    //       function__argument_name__min and function__argument_name__max
    //       and organize to lists according to address space of argument
    return true;
  }
  return true;
}

clang::VarDecl *WebCLRelocator::getRelocatedVariable(clang::Expr *expr)
{
    clang::ValueDecl *pruned = pruneValue(expr);
    if (!pruned)
        return NULL;

    clang::VarDecl *var = llvm::dyn_cast<clang::VarDecl>(pruned);
    if (!var)
        return NULL;

    return var;
}

// WebCLParameterizer

WebCLParameterizer::WebCLParameterizer(clang::CompilerInstance &instance)
    : WebCLTransformingVisitor(instance)
{
}

WebCLParameterizer::~WebCLParameterizer()
{
}

bool WebCLParameterizer::handleFunctionDecl(clang::FunctionDecl *decl)
{
    if (!isFromMainFile(decl->getLocStart()))
        return true;

    if (decl->hasAttr<clang::OpenCLKernelAttr>())
        return handleKernel(decl);
    return handleFunction(decl);
}

bool WebCLParameterizer::handleCallExpr(clang::CallExpr *expr)
{
    if (!isFromMainFile(expr->getLocStart()))
        return true;

    clang::FunctionDecl *decl = expr->getDirectCallee();
    if (!decl) {
        error(expr->getLocStart(), "Invalid callee.");
        return true;
    }

    if (!isRecordRequired(decl))
        return true;

    getTransformer().addRecordArgument(expr);
    return true;
}

bool WebCLParameterizer::handleFunction(clang::FunctionDecl *decl)
{
    if (!isRecordRequired(decl))
        return true;
    getTransformer().addRecordParameter(decl);
    return true;
}

bool WebCLParameterizer::handleKernel(clang::FunctionDecl *decl)
{
    getTransformer().addKernel(decl);

    std::set<clang::ParmVarDecl*> params;
    for (unsigned int i = 0; i < decl->getNumParams(); ++i) {
        clang::ParmVarDecl *param = decl->getParamDecl(i);
        if (!param) {
            error(decl->getLocation(), "Invalid parameter at position %0.") << i;
        } else if (isSizeRequired(param)) {
            getTransformer().addSizeParameter(param);
        }
    }
    return true;
}

bool WebCLParameterizer::isRecordRequired(clang::FunctionDecl *decl)
{
    // Examine whether the function body needs to perform limit checks.
    return decl->getNumParams() > 0;
}

bool WebCLParameterizer::isSizeRequired(const clang::ParmVarDecl *decl)
{
    const clang::TypeSourceInfo *info = decl->getTypeSourceInfo();
    if (!info) {
        error(decl->getSourceRange().getBegin(), "Invalid parameter type.");
        return false;
    }

    const clang::Type *type = decl->getOriginalType().getTypePtrOrNull();
    if (!type) {
        clang::SourceLocation typeLocation = info->getTypeLoc().getBeginLoc();
        error(typeLocation, "Invalid parameter type.");
        return false;
    }

    if (!type->isPointerType())
        return false;

    return isNonPrivateOpenCLAddressSpace(type->getPointeeType().getAddressSpace());
}

bool WebCLParameterizer::isNonPrivateOpenCLAddressSpace(unsigned int addressSpace) const
{
    static const clang::LangAS::ID spaces[] = {
        clang::LangAS::opencl_global,
        clang::LangAS::opencl_local,
        clang::LangAS::opencl_constant
    };

    for (unsigned int i = 0; i < sizeof(spaces) / sizeof(spaces[0]); ++i) {
        if (addressSpace == spaces[i])
            return true;
    }

    return false;
}

// WebCLAccessor

WebCLAccessor::WebCLAccessor(
    clang::CompilerInstance &instance)
    : WebCLTransformingVisitor(instance)
{
}

WebCLAccessor::~WebCLAccessor()
{
}

#include <iostream>

// TODO: Handle all memory accesses:

// ExtVectorElementExpr 0x7fd220cd57a8 <col:38, col:45> 'float' lvalue vectorcomponent x
// (MemberExpr 0x7fd220cd3b80 <col:45, col:65> 'float [3]' lvalue ->table 0x7fd220cd2850
// (ArraySubscriptExpr 0x7fd220cd3970 <col:5, col:17> '__local float4':'float __local __attribute__((ext_vector_type(4)))' lvalue
// (UnaryOperator 0x7fd220cd5ae8 <col:24, col:66> '__local float4':'float __local __attribute__((ext_vector_type(4)))' lvalue prefix '*'

bool WebCLAccessor::handleMemberExpr(clang::MemberExpr *expr)
{
  if (expr->isArrow()) {
    // std::cerr << "handleMemberExpr:\n";
    // expr->dump();
    // TODO: put this to bookkeeping that it must be fixed
  }
  return true;
}

bool WebCLAccessor::handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr)
{
  // handle only arrow accesses
  if (expr->isArrow()) {
    // std::cerr << "handleExtVectorElementExpr:\n";
    // expr->dump();
    // TODO: put this to bookkeeping that it must be fixed
  }
  return true;
}

bool WebCLAccessor::handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
  //std::cerr << "handleArraySubscriptExpr:\n";
  //const clang::Expr *base = expr->getBase();
  //const clang::Expr *index = expr->getIdx();
  // expr->dump();
  // TODO: put this to bookkeeping that it must be fixed
  return true;
}

bool WebCLAccessor::handleUnaryOperator(clang::UnaryOperator *expr)
{
  if (expr->getOpcode() == clang::UO_Deref) {
    clang::Expr *pointer = expr->getSubExpr();
    if (!pointer) {
      error(expr->getLocStart(), "Invalid pointer dereference.\n");
      return false;
    }
    // std::cerr << "handleUnaryOperator: " << expr->getLocStart() << "\n";
    // expr->dump();
    // TODO: put this to bookkeeping that it must be fixed
  }
  return true;
}

/*
bool WebCLAccessor::handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
    if (!isFromMainFile(expr->getLocStart()))
        return true;

    const clang::Expr *base = expr->getBase();
    const clang::Expr *index = expr->getIdx();

    llvm::APSInt arraySize;
    const bool isArraySizeKnown = getIndexedArraySize(base, arraySize);
    bool isArraySizeValid = false;
    if (isArraySizeKnown) {
        if (arraySize.getActiveBits() >= 63) {
            error(base->getLocStart(), "Invalid array size.\n");
        } else {
            isArraySizeValid = arraySize.isStrictlyPositive();
            if (!isArraySizeValid)
                error(base->getLocStart(), "Array size is not positive.\n");
        }
    }

    llvm::APSInt indexValue;
    const bool isIndexValueKnown = getArrayIndexValue(index, indexValue);
    bool isIndexValueValid = false;
    if (isIndexValueKnown) {
        if (indexValue.getActiveBits() >= 63) {
            error(index->getLocStart(), "Invalid array index.\n");
        } else {
            isIndexValueValid = !indexValue.isNegative();
            if (!isIndexValueValid)
                error(index->getLocStart(), "Array index is too small.\n");
        }
    }

    if (isArraySizeValid && isIndexValueValid) {
        const unsigned int arraySizeWidth = arraySize.getBitWidth();
        const unsigned int indexValueWidth = indexValue.getBitWidth();
        if (arraySizeWidth < indexValueWidth) {
            arraySize = arraySize.zext(indexValueWidth);
        } else if (indexValueWidth < arraySizeWidth) {
            indexValue = indexValue.zext(arraySizeWidth);
        }
        if (indexValue.uge(arraySize))
            error(index->getLocStart(), "Array index is too large.\n");
    }

    if (isArraySizeValid && !isIndexValueValid) {
        getTransformer().addArrayIndexCheck(expr, arraySize);
    } else if (!isArraySizeKnown) {
        getTransformer().addArrayIndexCheck(expr);
    }

    return true;
}

bool WebCLAccessor::handleUnaryOperator(clang::UnaryOperator *expr)
{
    if (!isFromMainFile(expr->getLocStart()))
        return true;

    if (expr->getOpcode() != clang::UO_Deref)
        return true;

    clang::Expr *pointer = expr->getSubExpr();
    if (!pointer) {
        error(expr->getLocStart(), "Invalid pointer dereference.\n");
        return false;
    }

    if (isPointerCheckNeeded(pointer)) {
        getTransformer().addPointerCheck(pointer);
    }

    return true;
}
*/

bool WebCLAccessor::getIndexedArraySize(const clang::Expr *base, llvm::APSInt &size)
{
    const clang::Expr *expr = base->IgnoreImpCasts();
    if (!expr) {
        error(base->getLocStart(), "Invalid array.\n");
        return false;
    }

    const clang::ConstantArrayType *type =
        instance_.getASTContext().getAsConstantArrayType(expr->getType());
    if (!type)
        return false;

    size = type->getSize();
    return true;
}

bool WebCLAccessor::getArrayIndexValue(const clang::Expr *index, llvm::APSInt &value)
{
    // Clang does these checks when checking for integer constant
    // expressions so we'll follow that practice.
    if (index->isTypeDependent() || index->isValueDependent())
        return false;
    // See also Expr::EvaluateAsInt for a more powerful variant of
    // Expr::isIntegerConstantExpr.
    return index->isIntegerConstantExpr(value, instance_.getASTContext());
}

bool WebCLAccessor::isPointerCheckNeeded(const clang::Expr *expr)
{
    const clang::Expr *pointer = expr->IgnoreImpCasts();
    if (!pointer) {
        error(expr->getLocStart(), "Invalid pointer access.\n");
        return false;
    }

    const clang::Type *type = pointer->getType().getTypePtrOrNull();
    if (!type) {
        error(pointer->getLocStart(), "Invalid pointer type.\n");
        return false;
    }

    return type->isPointerType();
}
