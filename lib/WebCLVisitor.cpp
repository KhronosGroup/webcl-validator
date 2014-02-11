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

#include "WebCLVisitor.hpp"
#include "WebCLDebug.hpp"
#include "WebCLTypes.hpp"

#include "clang/AST/Attr.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/OpenCL.h"

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

bool WebCLVisitor::VisitRecordDecl(clang::RecordDecl *decl)
{
  return handleRecordDecl(decl);
}

bool WebCLVisitor::VisitDeclRefExpr(clang::DeclRefExpr *expr)
{
  return handleDeclRefExpr(expr);
}

bool WebCLVisitor::VisitForStmt(clang::ForStmt *stmt)
{
  return handleForStmt(stmt);
}

bool WebCLVisitor::VisitGotoStmt(clang::GotoStmt *stmt)
{
    return handleGotoStmt(stmt);
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

bool WebCLVisitor::handleRecordDecl(clang::RecordDecl *decl)
{
  return true;
}

bool WebCLVisitor::handleDeclRefExpr(clang::DeclRefExpr *expr) {
  return true;
}

bool WebCLVisitor::handleForStmt(clang::ForStmt *stmt) {
  return true;
}

bool WebCLVisitor::handleGotoStmt(clang::GotoStmt *stmt) {
  return true;
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

    const clang::QualType qualType = info->getType();
    const clang::Type *type = qualType.getTypePtrOrNull();
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
    checkUnsupportedBuiltinParameter(function, typeLocation, qualType);
    return true;
}

bool WebCLRestrictor::handleGotoStmt(clang::GotoStmt *stmt)
{
    error(stmt->getLocStart(), "WebCL does not support goto");
    return false;
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
    if (canonical.getAsString() == "image3d_t") {
        error(typeLocation, "WebCL doesn't support 3D images.\n");
    }
}

void WebCLRestrictor::checkUnsupportedBuiltinParameter(
    clang::FunctionDecl *decl,
    clang::SourceLocation typeLocation, const clang::QualType &type)
{
    clang::QualType reduced = WebCLTypes::reduceType(instance_, type);
    std::string typeName = reduced.getAsString();
    if (decl->hasAttr<clang::OpenCLKernelAttr>() &&
        WebCLTypes::unsupportedBuiltinTypes().count(typeName) > 0) {
            error(typeLocation, "Unsupported builtin type %0 used as a kernel parameter.") << typeName;
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
      if (!decl->getInit()) {
          error(decl->getLocStart(), "Constant address space variables must be initialized.");
      }
      collectVariable(decl);
      break;
    case clang::LangAS::opencl_global:
      assert(false && "Global variables must be kernel parameters.");
      break;
    default:
      assert(isPrivate(decl) &&
             "Expected private address space.");
      if (decl->isFunctionOrMethodVarDecl()) {
        info(decl->getLocStart(), "Private variable declaration. Collect!");
        collectVariable(decl);
      } else if (decl->hasGlobalStorage()) {
        error(decl->getLocStart(), "Global scope variables must be in constant address space.");
      } else {
        info(decl->getLocStart(), "Function parameter... skip for now.");
      }
  }
  
  return true;
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
      clang::ValueDecl *valueDecl = declRef->getDecl();
      info(valueDecl->getLocStart(), "-- -- Corresponding decl.");
      clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(valueDecl);
      if (varDecl) {
          declarationsWithAddressOfAccess_.insert(varDecl);
          // Add variable to corresponding address space record if its
          // address is required.
          collectVariable(varDecl);
      } else {
          error(valueDecl->getLocStart(), "Taking of value addresses is not supported.");
      }
    }
  }
  return true;
}

namespace {
    std::map<std::string, std::string> typeShorthands()
    {
        static std::map<std::string, std::string> typeShorthands_;
        if (typeShorthands_.empty()) {
            typeShorthands_["unsigned char"] = "uchar";
            typeShorthands_["unsigned short"] = "ushort";
            typeShorthands_["unsigned int"] = "uint";
            typeShorthands_["unsigned long"] = "ulong";

            typeShorthands_["unsigned char *"] = "uchar *";
            typeShorthands_["unsigned short *"] = "ushort *";
            typeShorthands_["unsigned int *"] = "uint *";
            typeShorthands_["unsigned long *"] = "ulong *";
        }
        return typeShorthands_;
    }
}

WebCLAnalyser::KernelArgInfo::KernelArgInfo(clang::CompilerInstance &instance, clang::ParmVarDecl *decl)
    : decl(decl)
    , name(decl->getName().str())
    , reducedTypeName(WebCLTypes::reduceType(instance, decl->getType()).getAsString())
    , pointerKind(WebCLTypes::NOT_POINTER)
    , imageKind(WebCLTypes::NOT_IMAGE)
{
    if (typeShorthands().count(reducedTypeName)) {
        reducedTypeName = typeShorthands()[reducedTypeName];
    }

    const clang::Type *type = decl->getType().getTypePtr();

    // matches image3d_t as well, but that's handled elsewhere
    imageKind = WebCLTypes::imageKind(type, decl);
    if (imageKind != WebCLTypes::NOT_IMAGE) {
        pointerKind = WebCLTypes::IMAGE_HANDLE;
    } else if (type->isPointerType()) {
        switch (type->getPointeeType().getAddressSpace()) {
        case clang::LangAS::opencl_global:
            pointerKind = WebCLTypes::GLOBAL_POINTER;
            break;
        case clang::LangAS::opencl_constant:
            pointerKind = WebCLTypes::CONSTANT_POINTER;
            break;
        case clang::LangAS::opencl_local:
            pointerKind = WebCLTypes::LOCAL_POINTER;
            break;
        default:
            pointerKind = WebCLTypes::PRIVATE_POINTER;
        }
    }
}

WebCLAnalyser::KernelInfo::KernelInfo(clang::CompilerInstance &instance, clang::FunctionDecl *decl)
    : decl(decl)
    , name(decl->getNameInfo().getAsString())
{
    /// Go through arguments, collect info using the compiler instance
    for (clang::FunctionDecl::param_iterator i = decl->param_begin();
        i != decl->param_end(); ++i) {
            args.push_back(KernelArgInfo(instance, *i));
    }
}

bool WebCLAnalyser::handleFunctionDecl(clang::FunctionDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
  
  if (decl->hasAttr<clang::OpenCLKernelAttr>()) {
    info(decl->getLocStart(), "This is kernel, go through arguments to collect pointers etc.");
    kernelFunctions_.push_back(KernelInfo(instance_, decl));
    kernelSet_.insert(decl);

    // Check image typedefs here, so WebCLAnalyzer reference doesn't need to be passed everywhere
    const std::vector<KernelArgInfo>& kernelArgInfo = kernelFunctions_.back().args;
    for (std::vector<KernelArgInfo>::const_iterator kernelArgInfoIt = kernelArgInfo.begin();
         kernelArgInfoIt != kernelArgInfo.end();
         ++kernelArgInfoIt) {
        if (kernelArgInfoIt->imageKind == WebCLTypes::INVALID_TYPEDEF_ACCESS) {
            error(kernelArgInfoIt->decl->getLocStart(), "Invalid qualifier for typedef type");
        }
    }
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
  
  // make sure call is not kernel call
  if (isKernel(callee) > 0) {
    error(expr->getLocStart(), "Calling kernels is not allowed.");
    return true;
  }

  if (helperFunctions_.count(callee) > 0) {
    DEBUG( std::cerr << "Looks like it is call to internal function!\n"; );
    internalCalls_.insert(expr);
  } else {
    DEBUG( std::cerr << "Looks like it is call to builtin!\n"; );

    // LAUNDRY: better to move this logic to builtins class, this api to call it is really unclear
    const std::string name = callee->getNameInfo().getAsString();
    if (builtins_.isSafe(name)) {
      info(expr->getLocStart(), "Builtin was found from safe list.");
    } else if (builtins_.isUnsupported(name)) {
      error(expr->getLocStart(), "WebCL doesn't support %0.") << name;
      return true;
    } else if (builtins_.isUnsafe(name)) {
      warning(expr->getLocStart(), "Builtin argument check is still incomplete.");
    } else if (hasUnsafeParameters(expr)) {
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

  // check there was no struct decl inside this typdef added for moving.. (would be so much easier to do with matchers)
  if (!typeDeclList_.empty()) {
    // LAUNDRY: it cuold be easier to recognize required structures from code with matchers, 
    //          this was particulary inconvienient to do with visitor.
    clang::TypeDecl *lastType = typeDeclList_.back();
    if (lastType->getLocStart().getRawEncoding() > decl->getLocStart().getRawEncoding() &&
        lastType->getLocEnd().getRawEncoding()   < decl->getLocEnd().getRawEncoding()) {
      clang::TypeDecl *popped = typeDeclList_.back();
      typeDeclList_.pop_back();
      info(popped->getLocStart(), "Struct decl was inside typedef. Removed struct declaration from list of structs to be moved up.");
    }
  }

  typeDeclList_.push_back(decl);
  
  return true;
}

bool WebCLAnalyser::handleRecordDecl(clang::RecordDecl *decl)
{
  if (!isFromMainFile(decl->getLocStart())) return true;
  info(decl->getLocStart(), "Found struct declaration! Added to list for moving to top.");
  // add declaration to be moved to top of the module
  typeDeclList_.push_back(decl);

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

bool WebCLAnalyser::handleForStmt(clang::ForStmt *stmt) {
  
  if (clang::DeclStmt *declStmt = llvm::dyn_cast<clang::DeclStmt>(stmt->getInit())) {
    for (clang::DeclStmt::decl_iterator i = declStmt->decl_begin(); i != declStmt->decl_end(); i++) {
      clang::VarDecl *varDecl = llvm::dyn_cast<clang::VarDecl>(*i);
      assert(varDecl && "Expected variable declaration in for statement init clause.");
      info(varDecl->getLocStart(), "Found variable declaration made inside for statement declaration.");
      declarationsMadeInForStatements_.insert(varDecl);
    }
  }
  return true;
}

WebCLAnalyser::KernelList &WebCLAnalyser::getKernelFunctions()
{
    return kernelFunctions_;
}

WebCLAnalyser::FunctionDeclSet &WebCLAnalyser::getHelperFunctions()
{
    return helperFunctions_;
}

WebCLAnalyser::CallExprSet &WebCLAnalyser::getInternalCalls()
{
    return internalCalls_;
}

WebCLAnalyser::CallExprSet &WebCLAnalyser::getBuiltinCalls()
{
    return builtinCalls_;
}

WebCLAnalyser::VarDeclSet &WebCLAnalyser::getConstantVariables()
{
    return constantVariables_;
}

WebCLAnalyser::VarDeclSet &WebCLAnalyser::getLocalVariables()
{
    return localVariables_;
}

WebCLAnalyser::VarDeclSet &WebCLAnalyser::getPrivateVariables()
{
    return privateVariables_;
}

WebCLAnalyser::DeclRefExprSet &WebCLAnalyser::getVariableUses()
{
    return variableUses_;
}

WebCLAnalyser::MemoryAccessMap &WebCLAnalyser::getPointerAceesses()
{
    return pointerAccesses_;
}

WebCLAnalyser::TypeDeclList &WebCLAnalyser::getTypeDecls()
{
    return typeDeclList_;
}

const WebCLAnalyser::KernelList &WebCLAnalyser::getKernelFunctions() const
{
    return kernelFunctions_;
}

const WebCLAnalyser::FunctionDeclSet &WebCLAnalyser::getHelperFunctions() const
{
    return helperFunctions_;
}

const WebCLAnalyser::CallExprSet &WebCLAnalyser::getInternalCalls() const
{
    return internalCalls_;
}

const WebCLAnalyser::CallExprSet &WebCLAnalyser::getBuiltinCalls() const
{
    return builtinCalls_;
}

const WebCLAnalyser::VarDeclSet &WebCLAnalyser::getConstantVariables() const
{
    return constantVariables_;
}

const WebCLAnalyser::VarDeclSet &WebCLAnalyser::getLocalVariables() const
{
    return localVariables_;
}

const WebCLAnalyser::VarDeclSet &WebCLAnalyser::getPrivateVariables() const
{
    return privateVariables_;
}

const WebCLAnalyser::DeclRefExprSet &WebCLAnalyser::getVariableUses() const
{
    return variableUses_;
}

const WebCLAnalyser::MemoryAccessMap &WebCLAnalyser::getPointerAceesses() const
{
    return pointerAccesses_;
}

const WebCLAnalyser::TypeDeclList &WebCLAnalyser::getTypeDecls() const
{
    return typeDeclList_;
}

bool WebCLAnalyser::hasAddressReferences(clang::VarDecl *decl)
{
    return declarationsWithAddressOfAccess_.count(decl) > 0;
}

bool WebCLAnalyser::isInsideForStmt(clang::VarDecl *decl)
{
    return declarationsMadeInForStatements_.count(decl) > 0;
}

bool WebCLAnalyser::hasUnsafeParameters(clang::CallExpr *callExpr)
{
    clang::FunctionDecl *decl = callExpr->getDirectCallee();

    for (unsigned i = 0; i < callExpr->getNumArgs(); ++i) {
	clang::QualType type = callExpr->getArg(i)->getType();
	if (type.getTypePtr()->isPointerType() &&
	    WebCLTypes::reduceType(instance_, type).getAsString() != "image2d_t") {
	    return true;
	}
    }

    for (unsigned int i = 0; i < decl->getNumParams(); ++i) {
        const clang::ParmVarDecl *param = decl->getParamDecl(i);
        if (param->getType().getTypePtr()->isPointerType() &&
	    WebCLTypes::reduceType(instance_, param->getType()).getAsString() != "image2d_t")
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
