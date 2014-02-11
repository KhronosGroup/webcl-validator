#ifndef WEBCLVALIDATOR_WEBCLVISITOR
#define WEBCLVALIDATOR_WEBCLVISITOR

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

#include "WebCLBuiltins.hpp"
#include "WebCLReporter.hpp"
#include "WebCLTypes.hpp"

#include <map>
#include <set>
#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

namespace clang {
    class TranslationUnitDecl;
}

/// \brief Common base for all AST visitors.
///
/// There are two kinds of visitors:
/// - Visitors that traverse a valid OpenCL AST to check for errors. They check
///   whether the OpenCL AST is also a valid WebCL AST.
/// - Visitors that analyze a valid WebCL AST and collect information
///   for transformation passes.
///
/// This class also replaces Visit-methods of RecursiveASTVisitor with
/// corresponding polymorphic handle-methods so that it would be
/// easier to build visitor hierarchies.
class WebCLVisitor : public WebCLReporter
                   , public clang::RecursiveASTVisitor<WebCLVisitor>
{
public:

    explicit WebCLVisitor(clang::CompilerInstance &instance);
    virtual ~WebCLVisitor();

    /// \see clang::RecursiveASTVisitor::VisitTranslationUnitDecl
    bool VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitFunctionDecl
    bool VisitFunctionDecl(clang::FunctionDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitParmVar
    bool VisitParmVarDecl(clang::ParmVarDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitVarDecl
    bool VisitVarDecl(clang::VarDecl *decl);

    /// \see clang::RecursiveASTVisitor::VisitDeclStmt
    bool VisitDeclStmt(clang::DeclStmt *stmt);

    /// \see clang::RecursiveASTVisitor::VisitArraySubscriptExpr
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr);
    /// \see clang::RecursiveASTVisitor::VisitUnaryOperator
    bool VisitUnaryOperator(clang::UnaryOperator *expr);
    /// \see clang::RecursiveASTVisitor::MemberExpr
    bool VisitMemberExpr(clang::MemberExpr *expr);
    /// \see clang::RecursiveASTVisitor::ExtVectorElementExpr
    bool VisitExtVectorElementExpr(clang::ExtVectorElementExpr *expr);
    /// \see clang::RecursiveASTVisitor::VisitCallExpr
    bool VisitCallExpr(clang::CallExpr *expr);
    /// \see clang::RecursiveASTVisitor::VisitTypedefDecl
    bool VisitTypedefDecl(clang::TypedefDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitTypedefDecl
    bool VisitRecordDecl(clang::RecordDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitDeclRefExpr
    bool VisitDeclRefExpr(clang::DeclRefExpr *decl);
    /// \see clang::RecursiveASTVisitor::VisitForStmt
    bool VisitForStmt(clang::ForStmt *stmt);
    /// \see clang::RecursiveASTVisitor::VisitGotoStmt
    bool VisitGotoStmt(clang::GotoStmt *stmt);
  
protected:

    virtual bool handleTranslationUnitDecl(clang::TranslationUnitDecl *decl);
    virtual bool handleFunctionDecl(clang::FunctionDecl *decl);
    virtual bool handleParmVarDecl(clang::ParmVarDecl *decl);
    virtual bool handleVarDecl(clang::VarDecl *decl);

    virtual bool handleDeclStmt(clang::DeclStmt *stmt);

    virtual bool handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr);
    virtual bool handleUnaryOperator(clang::UnaryOperator *expr);
    virtual bool handleMemberExpr(clang::MemberExpr *expr);
    virtual bool handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr);
    virtual bool handleCallExpr(clang::CallExpr *expr);
    virtual bool handleTypedefDecl(clang::TypedefDecl *decl);
    virtual bool handleRecordDecl(clang::RecordDecl *decl);
    virtual bool handleDeclRefExpr(clang::DeclRefExpr *expr);
    virtual bool handleForStmt(clang::ForStmt *stmt);
    virtual bool handleGotoStmt(clang::GotoStmt *stmt);
};

/// \brief Complains about WebCL limitations in OpenCL C code.
class WebCLRestrictor : public WebCLVisitor
{
public:

    explicit WebCLRestrictor(clang::CompilerInstance &instance);
    virtual ~WebCLRestrictor();

    /// \see WebCLVisitor::handleParmVar
    virtual bool handleParmVarDecl(clang::ParmVarDecl *decl);

    /// \see WebCLVisitor::handleGotoStmt
    virtual bool handleGotoStmt(clang::GotoStmt *stmt);

private:

    /// Checks that structures aren't passed to kernels.
    void checkStructureParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
    /// Checks that 3D images aren't used.
    void check3dImageParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
    /// Checks that unsupported builtin types are not used as parameters
    void checkUnsupportedBuiltinParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::QualType &type);
};

/// \brief Collects all information needed by transformations.
class WebCLAnalyser : public WebCLVisitor
{
public:
  
  explicit WebCLAnalyser(clang::CompilerInstance &instance);
  virtual ~WebCLAnalyser();

  /// Collect all static variable allocations
  ///
  /// \see WebCLVisitor::handleVarDecl
  virtual bool handleVarDecl(clang::VarDecl *decl);

  /// Collect structure pointer -> access
  ///
  /// \see WebCLVisitor::handleMemberExpr
  virtual bool handleMemberExpr(clang::MemberExpr *expr);
  
  /// Collect vector pointer -> access
  ///
  /// \see WebCLVisitor::handleExtVectorElementExpr
  virtual bool handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr);
  
  /// Collect array indexing 
  ///
  /// \see WebCLVisitor::handleArraySubscriptExpr
  virtual bool handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr);
  
  /// Collect * and & operator uses
  ///
  /// \see WebCLVisitor::handleUnaryOperator
  virtual bool handleUnaryOperator(clang::UnaryOperator *expr);

  /// Collect functions whose signatures must be changed.
  ///
  /// - Collects kernels and their pointer parameters so that
  ///   corresponding size parameters can be generated.
  /// - Collects all helper functions that need to perform memory
  ///   access checks so that memory allocation parameter can be
  ///   generated.
  ///
  /// \see WebCLVisitor::handleFunctionDecl
  virtual bool handleFunctionDecl(clang::FunctionDecl *decl);

  /// Collects builtin and internal helper function calls.
  ///
  /// - Internal helper calls may need an extra memory allocation
  ///   argument.
  /// - FUTURE: Unsafe builtins require checks for their pointer
  ///           arguments.
  ///
  /// \see WebCLVisitor::handleCallExpr
  virtual bool handleCallExpr(clang::CallExpr *expr);
  
  /// Collects all typedef declarations.
  ///
  /// - Typedefs need to be moved on top so that they can be used in
  ///   our address space structures.
  ///
  /// \see WebCLVisitor::handleTypedefDecl
  virtual bool handleTypedefDecl(clang::TypedefDecl *decl);
  
  /// Collects all structure declarations.
  ///
  /// - Structure declarations need to be moved on top so that they
  ///   can be used in our address space structures.
  ///
  /// \see WebCLVisitor::handleRecordDecl
  virtual bool handleRecordDecl(clang::RecordDecl *decl);
  
  /// Collects variable references.
  ///
  /// - The reference may need to be modified to point to a relocated
  ///   variable.
  virtual bool handleDeclRefExpr(clang::DeclRefExpr *expr);

  /// FUTURE: Remove once variable declarations in first for clause
  ///         have been normalized.
  virtual bool handleForStmt(clang::ForStmt *stmt);

  /// Collected nodes.
  struct KernelArgInfo {
      /// Not exposed outside the library
      clang::ParmVarDecl *decl;

      /// Parameter name.
      std::string name;
      /// Type name, cleaned of qualifiers and non-standard typedefs
      std::string reducedTypeName;
      /// Is this a pointer arg, and if so, to which address space
      WebCLTypes::PointerKind pointerKind;
      /// Is this an image arg, and if so, with which access qualifiers
      WebCLTypes::ImageKind imageKind;

      KernelArgInfo(clang::CompilerInstance &instance, clang::ParmVarDecl *decl);
  };
  struct KernelInfo {
      /// Not exposed outside the library
      clang::FunctionDecl *decl;

      /// Kernel name
      std::string name;
      /// Kernel arguments
      std::vector<KernelArgInfo> args;

      KernelInfo(clang::CompilerInstance &instance, clang::FunctionDecl *decl);
  };

  typedef std::set<clang::FunctionDecl*> FunctionDeclSet;
  typedef std::vector<KernelInfo> KernelList;
  typedef std::set<clang::CallExpr*> CallExprSet;
  typedef std::set<clang::VarDecl*> VarDeclSet;
  typedef std::set<clang::DeclRefExpr*> DeclRefExprSet;
  typedef std::vector<clang::TypeDecl*> TypeDeclList;

  /// Memory accesses and corresponding declarations, this will change
  /// if separate dependence analysis is added to resolve which limits
  /// each memory access should respect.
  typedef std::map<clang::Expr*, clang::VarDecl*> MemoryAccessMap;
  
  /// Accessors for collected data.
  KernelList &getKernelFunctions();
  FunctionDeclSet &getHelperFunctions();
  CallExprSet &getInternalCalls();
  CallExprSet &getBuiltinCalls();
  VarDeclSet &getConstantVariables();
  VarDeclSet &getLocalVariables();
  VarDeclSet &getPrivateVariables();
  DeclRefExprSet &getVariableUses();
  MemoryAccessMap &getPointerAceesses();
  TypeDeclList &getTypeDecls();

  /// Const versions of the above
  const KernelList &getKernelFunctions() const;
  const FunctionDeclSet &getHelperFunctions() const;
  const CallExprSet &getInternalCalls() const;
  const CallExprSet &getBuiltinCalls() const;
  const VarDeclSet &getConstantVariables() const;
  const VarDeclSet &getLocalVariables() const;
  const VarDeclSet &getPrivateVariables() const;
  const DeclRefExprSet &getVariableUses() const;
  const MemoryAccessMap &getPointerAceesses() const;
  const TypeDeclList &getTypeDecls() const;

  /// \return Whether address of variable is taken.
  bool hasAddressReferences(clang::VarDecl *decl);

  /// \return Whether variable has been declared in first for clause.
  bool isInsideForStmt(clang::VarDecl *decl);

  /// \return Whether a function call passes pointer parameter or if the
  /// function declaration takes pointer parameters.
  bool hasUnsafeParameters(clang::CallExpr *expr);
  
  /// \return True if name is name of a kernel function.
  bool isKernel(clang::FunctionDecl *decl) {
    return kernelSet_.count(decl) > 0;
  }

private:

  /// \return Whether a variable is stored in private address space.
  bool isPrivate(clang::VarDecl *decl) const;

  /// Save variable into address space specific variable collection.
  void collectVariable(clang::VarDecl *decl);
  
  /// User defined kernels.
  KernelList kernelFunctions_;
  /// Set to check fast if function declaration is a kernel.
  FunctionDeclSet kernelSet_;
  /// User defined functions.
  FunctionDeclSet helperFunctions_;
  /// Calls to user defined functions.
  CallExprSet internalCalls_;
  /// Calls to builtin functions.
  CallExprSet builtinCalls_;
  /// Variables in constant address space.
  VarDeclSet constantVariables_;
  /// Variables in local address space.
  VarDeclSet localVariables_;
  /// Variables in private address space.
  VarDeclSet privateVariables_;
  /// Variables whose address has been taken with the & operator.
  VarDeclSet declarationsWithAddressOfAccess_;
  /// Variables declared in the first for clause.
  VarDeclSet declarationsMadeInForStatements_;
  /// All uses of variable declarations.
  DeclRefExprSet variableUses_;
  /// Field accesses with -> operator.
  MemoryAccessMap pointerAccesses_;
  /// Typedefs and record declarations.
  TypeDeclList typeDeclList_;
  /// All unsupported and unsafe builtins.
  WebCLBuiltins   builtins_;
};

#endif // WEBCLVALIDATOR_WEBCLVISITOR
