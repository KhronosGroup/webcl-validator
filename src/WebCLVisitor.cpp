#include "WebCLVisitor.hpp"
#include "WebCLDebug.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"

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
  if (!isFromMainFile(decl->getLocStart()))
    return true;

  switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_local:
      info(decl->getLocStart(), "Local variable declaration. Collect!");
      collectVariable(decl);
      break;
    case clang::LangAS::opencl_constant:
      info(decl->getLocStart(), "Constant variable declaration. Collect!");
      collectVariable(decl);
      break;
    default:
      assert(isPrivate(decl));
      if (decl->isFunctionOrMethodVarDecl()) {
        info(decl->getLocStart(), "Private variable declaration. Collect!");
        collectVariable(decl);
      } else {
        info(decl->getLocStart(), "Function parameter... skip for now.");
      }
  }
  
  return true;

#if 0
  
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
    DEBUG( std::cerr << "######## Woot:" << decl->getType().getAddressSpace() << "\n"; );
  } else {
    info(decl->getLocStart(), "Filter these, so that only globals are left. And maybe some function arguments.");
  }
  
  return true;

#endif
}

bool WebCLAnalyser::handleMemberExpr(clang::MemberExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;
  
  if (expr->isArrow()) {
    info(expr->getLocStart(), "Pointer access!");
    clang::VarDecl *declaration = NULL;
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getBase())) {
      DEBUG( std::cerr << "Jackpot found decl:\n"; );
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      declaration = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
    
    pointerAccesses_[expr] = declaration;
  }
  return true;
}

bool WebCLAnalyser::handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;

  // handle only arrow accesses
  if (expr->isArrow()) {
    info(expr->getLocStart(), "Pointer access!");
    clang::VarDecl *declaration = NULL;
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getBase())) {
      DEBUG( std::cerr << "Jackpot found decl:\n"; );
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      declaration = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
    pointerAccesses_[expr] = declaration;
  }
  return true;
}

bool WebCLAnalyser::handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr)
{
  if (!isFromMainFile(expr->getLocStart())) return true;
  info(expr->getLocStart(), "Pointer access!");

  clang::VarDecl *declaration = NULL;

  // in case if we are abse to trace actual base declaration we can optimize more in future
  if (clang::ImplicitCastExpr *implicitCast = llvm::dyn_cast<clang::ImplicitCastExpr>(expr->getBase())) {
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(implicitCast->getSubExpr())) {
      DEBUG( std::cerr << "Jackpot found decl:\n"; );
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      declaration = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
  }
  
  pointerAccesses_[expr] = declaration;

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
    clang::VarDecl *declaration = NULL;
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getSubExpr())) {
      DEBUG( std::cerr << "Jackpot found decl:\n"; );
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      declaration = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
    // std::cerr << "expr type:" << addr->getType().getAsString()
    //          << " address number space: " << addr->getType().getAddressSpace()
    //          <<  "\n\n";
    pointerAccesses_[expr] = declaration;
    
  } else if(expr->getOpcode() == clang::UO_AddrOf) {
    info(expr->getLocStart(), "Address of something, might require some handling.");
    if (clang::DeclRefExpr *declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr->getSubExpr())) {
      DEBUG( std::cerr << "Found decl for address of operator:\n"; );
      info(declRef->getDecl()->getLocStart(), "-- -- Corresponding decl.");
      clang::VarDecl *decl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl());
      assert(decl);
      declarationsWithAddressOfAccess_.insert(decl);
      // Add variable to corresponding address space record if its
      // address is required.
      collectVariable(decl);
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

  clang::FunctionDecl *callee = expr->getDirectCallee();
  if (!callee) {
      error(expr->getLocStart(), "Function call is not direct.");
      return true;
  }

  if (helperFunctions_.count(callee) > 0) {
    DEBUG( std::cerr << "Looks like it is call to internal function!\n"; );
    internalCalls_.insert(expr);
  } else {
    DEBUG( std::cerr << "Looks like it is call to builtin!\n"; );

    const std::string name = callee->getNameInfo().getAsString();
    if (builtins_.isUnsupported(name)) {
      error(expr->getLocStart(), "WebCL doesn't support %0.") << name;
      return true;
    } else if (builtins_.isUnsafe(name)) {
      error(expr->getLocStart(), "Builtin argument check is required.");
    } else if (hasUnsafeParameters(callee)) {
      error(expr->getLocStart(), "Unsafe builtin not recognized.");
    }

    builtinCalls_.insert(expr);
  }
  
  return true;
}

bool WebCLAnalyser::handleTypedefDecl(clang::TypedefDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
  info(decl->getLocStart(), "Found typedef, add to list to move to start of source.");
  typedefList_.push_back(decl);
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

bool WebCLAnalyser::hasUnsafeParameters(clang::FunctionDecl *decl)
{
    for (unsigned int i = 0; i < decl->getNumParams(); ++i) {
        const clang::ParmVarDecl *param = decl->getParamDecl(i);
        if (param->getType().getTypePtr()->isPointerType())
            return true;
    }
    return false;
}

bool WebCLAnalyser::isPrivate(clang::VarDecl *decl) const
{
    return decl->getType().getAddressSpace() == 0;
}

void WebCLAnalyser::collectVariable(clang::VarDecl *decl)
{
    switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_local:
        localVariables_.insert(decl);
        return;
    case clang::LangAS::opencl_constant:
        constantVariables_.insert(decl);
        return;
    default:
        if (isPrivate(decl))
            privateVariables_.insert(decl);
    }
}
