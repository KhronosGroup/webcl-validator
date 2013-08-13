#ifndef WEBCLVALIDATOR_WEBCLVISITOR
#define WEBCLVALIDATOR_WEBCLVISITOR

#include "WebCLBuiltins.hpp"
#include "WebCLReporter.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/RecursiveASTVisitor.h"

namespace clang {
    class TranslationUnitDecl;
}

/// \brief Common base for all AST visitors.
///
/// There are two kinds of visitors:
/// - Visitors that traverse the AST before transformations. They
///   check whether a valid OpenCL AST is also a valid WebCL AST.
/// - Visitors that find nodes to be transformed. When they are sure
///   that some AST node needs to be transformed, they call
///   WebCLTransformer services to do the actual transformations.
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
    /// \see clang::RecursiveASTVisitor::VisitDeclRefExpr
    bool VisitDeclRefExpr(clang::DeclRefExpr *decl);
    /// \see clang::RecursiveASTVisitor::VisitForStmt
    bool VisitForStmt(clang::ForStmt *stmt);
  
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
    virtual bool handleDeclRefExpr(clang::DeclRefExpr *expr);
    virtual bool handleForStmt(clang::ForStmt *stmt);
};

/// \brief Complains about WebCL limitations in OpenCL C code.
class WebCLRestrictor : public WebCLVisitor
{
public:

    explicit WebCLRestrictor(clang::CompilerInstance &instance);
    virtual ~WebCLRestrictor();

    /// \see WebCLVisitor::handleFunctionDecl
    virtual bool handleFunctionDecl(clang::FunctionDecl *decl);

    /// \see WebCLVisitor::handleParmVar
    virtual bool handleParmVarDecl(clang::ParmVarDecl *decl);

private:

    void checkStructureParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
    void check3dImageParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
};

/// \brief Collects all the information about the source code required for transformations.
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

  /// Collects lists of functions, whose signature must be changed
  ///
  /// - Collects kernels and all pointer arguments
  /// - Collects all other functions, which will requires wcl_alloc
  ///   argument to be added
  ///
  /// \see WebCLVisitor::handleFunctionDecl
  virtual bool handleFunctionDecl(clang::FunctionDecl *decl);

  /// Collect calls and later on this instance will know if call is made to
  /// builtin or internal helper.
  ///
  /// - If internal helper need to add wcl_alloc arg to first parameter
  /// - If not internal helper do not touch
  /// - FUTURE: if in unsafe builtin list, need to fix to call
  ///           safe implementation with all possible limits in arguments
  ///           and create corresponding safe implementation to start of code
  ///
  /// \see WebCLVisitor::handleCallExpr
  virtual bool handleCallExpr(clang::CallExpr *expr);
  
  /// Collects all typedef declarations to move them to start of source
  ///
  /// - Typedefs needs to be moved up to be able to use them in our
  ///   address space structures.
  ///
  /// \see WebCLVisitor::handleTypedefDecl
  virtual bool handleTypedefDecl(clang::TypedefDecl *decl);
  
  
  /// Go through all decl references to see if they need to be fixed
  virtual bool handleDeclRefExpr(clang::DeclRefExpr *expr);

  virtual bool handleForStmt(clang::ForStmt *stmt);

  /// This was handled in earlier passes, but not sure if necessary
  /// the VarDecl handler seeme to catch these too.
  /// TODO: remove if not needed
  virtual bool handleDeclStmt(clang::DeclStmt *expr) {
    info(expr->getLocStart(), "DectStmt! what to do... why??");
    return true;
  };
  
  /// Accessors for data collected by analyser
  typedef std::set<clang::FunctionDecl*> FunctionDeclSet;
  typedef std::set<clang::CallExpr*> CallExprSet;
  typedef std::set<clang::VarDecl*> VarDeclSet;
  typedef std::set<clang::DeclRefExpr*> DeclRefExprSet;
  typedef std::vector<clang::TypedefDecl*> TypedefList;

  /// Memory accesses and corresponding declarations, this will change
  /// if separate dependence analysis is added to resolve which limits
  /// each memory access should respect.
  typedef std::map<clang::Expr*, clang::VarDecl*> MemoryAccessMap;
  
  FunctionDeclSet& getKernelFunctions()  { return kernelFunctions_; };
  FunctionDeclSet& getHelperFunctions()  { return helperFunctions_; };
  CallExprSet&     getInternalCalls()     { return internalCalls_; };
  CallExprSet&     getBuiltinCalls()      { return builtinCalls_; };
  VarDeclSet&      getConstantVariables() { return constantVariables_; };
  VarDeclSet&      getLocalVariables()    { return localVariables_; };
  VarDeclSet&      getPrivateVariables()  { return privateVariables_; };
  DeclRefExprSet&  getVariableUses()      { return variableUses_; };
  MemoryAccessMap& getPointerAceesses()   { return pointerAccesses_; };
  TypedefList&     getTypedefs()          { return typedefList_; };

  bool hasAddressReferences(clang::VarDecl *decl) {
    return declarationsWithAddressOfAccess_.count(decl) > 0;
  };

  bool isInsideForStmt(clang::VarDecl *decl) {
    return declarationsMadeInForStatements_.count(decl) > 0;
  };
  
private:

  bool isPrivate(clang::VarDecl *decl) const;
  void collectVariable(clang::VarDecl *decl);

  clang::FunctionDecl *contextFunction_;
  
  FunctionDeclSet kernelFunctions_;
  FunctionDeclSet helperFunctions_;
  CallExprSet     internalCalls_;
  CallExprSet     builtinCalls_;
  VarDeclSet      constantVariables_;
  VarDeclSet      localVariables_;
  VarDeclSet      privateVariables_;
  /// set of variables, which has been accessed with &-operator
  VarDeclSet      declarationsWithAddressOfAccess_;
  /// set of variables, which has been declared in for( <declaration> ; ; )
  VarDeclSet      declarationsMadeInForStatements_;
  /// all uses of variable declarations
  DeclRefExprSet  variableUses_;
  MemoryAccessMap pointerAccesses_;
  TypedefList     typedefList_;
  // all unsupported and unsafe builtins
  WebCLBuiltins   builtins_;
};


/// \brief Common base for transforming visitors.
class WebCLTransformingVisitor : public WebCLVisitor
                               , public WebCLTransformerClient
{
public:

    explicit WebCLTransformingVisitor(clang::CompilerInstance &instance);
    virtual ~WebCLTransformingVisitor();
};

#endif // WEBCLVALIDATOR_WEBCLVISITOR
