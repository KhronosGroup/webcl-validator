#ifndef WEBCLVALIDATOR_WEBCLVISITOR
#define WEBCLVALIDATOR_WEBCLVISITOR

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

  /// 
  virtual bool handleDeclStmt(clang::DeclStmt *expr) {
    info(expr->getLocStart(), "DectStmt! what to do... why??");
    return true;
  };
  
  /// Accessors for data collected by analyser
  typedef std::set<clang::FunctionDecl*> FunctionDeclSet;
  typedef std::set<clang::CallExpr*> CallExprSet;
  typedef std::set<clang::VarDecl*> VarDeclSet;
  typedef std::set<clang::DeclRefExpr*> DeclRefExprSet;

  
  FunctionDeclSet& getKernelFunctions()  { return kernelFunctions_; };
  FunctionDeclSet& getHelperFunctions()  { return helperFunctions_; };
  CallExprSet&    getInternalCalls()     { return internalCalls_; };
  CallExprSet&    getBuiltinCalls()      { return builtinCalls_; };
  VarDeclSet&     getConstantVariables() { return constantVariables_; };
  VarDeclSet&     getLocalVariables()    { return localVariables_; };
  VarDeclSet&     getPrivateVariables()  { return privateVariables_; };
  DeclRefExprSet& getVariableUses()      { return variableUses_; };
  
  bool hasAddressReferences(clang::VarDecl *decl) {
    return declarationsWithAddressOfAccess_.count(decl) > 0;
  }
  
private:
  
  clang::FunctionDecl *contextFunction_;
  
  FunctionDeclSet kernelFunctions_;
  FunctionDeclSet helperFunctions_;
  CallExprSet     internalCalls_;
  CallExprSet     builtinCalls_;
  VarDeclSet      constantVariables_;
  VarDeclSet      localVariables_;
  VarDeclSet      privateVariables_;
  // set of variables, which has been accessed with &-operator
  VarDeclSet      declarationsWithAddressOfAccess_;
  DeclRefExprSet  variableUses_;
  
  // TODO: maybe I need to keep track of variable names of declarations to
  //       be able to trace them in expressions.
  
};


/// \brief Common base for transforming visitors.
class WebCLTransformingVisitor : public WebCLVisitor
                               , public WebCLTransformerClient
{
public:

    explicit WebCLTransformingVisitor(clang::CompilerInstance &instance);
    virtual ~WebCLTransformingVisitor();
};

/// \brief Finds variables that need to be relocated into address
/// space records and all the kernel arguments, which contain limits.
class WebCLRelocator : public WebCLTransformingVisitor
                     , public WebCLHelper
{
public:

    explicit WebCLRelocator(clang::CompilerInstance &instance);
    virtual ~WebCLRelocator();

  
    /// \see WebCLVisitor::handleDeclStmt
    virtual bool handleDeclStmt(clang::DeclStmt *stmt);
  
    /// \see WebCLVisitor::handleVarDecl
    virtual bool handleVarDecl(clang::VarDecl *decl);
  
    /// \see WebCLVisitor::handleUnaryOperator
    virtual bool handleUnaryOperator(clang::UnaryOperator *expr);

    /// Collects list of kernel arguments, which require limits
    /// - Later on this information is combined with information
    ///   in WebCLParameterizer to be able to write initialization
    ///   code for kernel.
    /// \see WebCLVisitor::handleFunctionDecl
    virtual bool handleFunctionDecl(clang::FunctionDecl *decl);

private:

    clang::VarDecl *getRelocatedVariable(clang::Expr *expr);

    clang::DeclStmt *current_;
    typedef std::pair<clang::VarDecl*, clang::DeclStmt*> RelocationCandidate;
    typedef std::map<clang::VarDecl*, clang::DeclStmt*> RelocationCandidates;
    RelocationCandidates relocationCandidates_;
};

/// Finds function parameter lists that need to be extended. Also
/// finds function calls and augments argument lists when necessary.
class WebCLParameterizer : public WebCLTransformingVisitor
{
public:

    explicit WebCLParameterizer(clang::CompilerInstance &instance);
    virtual ~WebCLParameterizer();

    /// Checks whether parameter lists need to be extended:
    ///
    /// - Add array size parameters for kernel array parameters.
    /// - Add address space record parameters for functions that need
    ///   to do pointer and index checking.
    ///
    /// \see WebCLVisitor::handleFunctionDecl
    virtual bool handleFunctionDecl(clang::FunctionDecl *decl);

    /// Checks whether argument lists need to be extended:
    ///
    /// - Add address space argument for functions that need to do
    ///  pointer and index checking.
    ///
    /// - TODO: Knows if the called function is builtin, or builtin that
    ///   needs to be replaced with safe implementation.
    ///
    /// - TODO: If builtin is replaced, with safe version, add it to bookkeeping
    ///   to be able to emit safe implementation afterwards.
    ///
    /// \see WebCLVisitor::handleCallExpr
    virtual bool handleCallExpr(clang::CallExpr *expr);

private:

    bool handleFunction(clang::FunctionDecl *decl);

    bool handleKernel(clang::FunctionDecl *decl);

    /// \brief Whether function needs address space record parameter.
    bool isRecordRequired(clang::FunctionDecl *decl);

    /// \brief Whether parameter requires size information.
    ///
    /// This is intended for __global, __local and __constant memory
    /// object parameters.
    bool isSizeRequired(const clang::ParmVarDecl *decl);

    /// \brief Whether address space is __global, __local or
    /// __constant.
    bool isNonPrivateOpenCLAddressSpace(unsigned int addressSpace) const;
};

/// \brief Finds array subscriptions and all other pointer dereferences.
///
/// Handled access types: table[index];, struct_ptr->field;, *(any_pointer);
/// For the transformation all accesses are normalized to (*()) format.
/// i.e. (*(table + (index))) and (*(struct_ptr)).field
class WebCLAccessor : public WebCLTransformingVisitor
{
public:

    explicit WebCLAccessor(clang::CompilerInstance &instance);
    virtual ~WebCLAccessor();

    /// \see WebCLVisitor::handleMemberExpr
    virtual bool handleMemberExpr(clang::MemberExpr *expr);
  
    /// \see WebCLVisitor::handleExtVectorElementExpr
    virtual bool handleExtVectorElementExpr(clang::ExtVectorElementExpr *expr);

    /// \see WebCLVisitor::handleArraySubscriptExpr
    virtual bool handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr);

    /// \see WebCLVisitor::handleUnaryOperator
    virtual bool handleUnaryOperator(clang::UnaryOperator *expr);

private:

    bool getIndexedArraySize(const clang::Expr *base, llvm::APSInt &size);

    bool getArrayIndexValue(const clang::Expr *index, llvm::APSInt &value);

    bool isPointerCheckNeeded(const clang::Expr *expr);
};

#endif // WEBCLVALIDATOR_WEBCLVISITOR
