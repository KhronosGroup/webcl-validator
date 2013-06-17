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
    /// \see clang::RecursiveASTVisitor::VisitCallExpr
    bool VisitCallExpr(clang::CallExpr *expr);

protected:

    virtual bool handleTranslationUnitDecl(clang::TranslationUnitDecl *decl);
    virtual bool handleFunctionDecl(clang::FunctionDecl *decl);
    virtual bool handleParmVarDecl(clang::ParmVarDecl *decl);
    virtual bool handleVarDecl(clang::VarDecl *decl);
    virtual bool handleDeclStmt(clang::DeclStmt *stmt);
    virtual bool handleArraySubscriptExpr(clang::ArraySubscriptExpr *expr);
    virtual bool handleUnaryOperator(clang::UnaryOperator *expr);
    virtual bool handleCallExpr(clang::CallExpr *expr);
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

/// \brief Common base for transforming visitors.
class WebCLTransformingVisitor : public WebCLVisitor
                               , public WebCLTransformerClient
{
public:

    explicit WebCLTransformingVisitor(clang::CompilerInstance &instance);
    virtual ~WebCLTransformingVisitor();
};

/// \brief Finds variables that need to be relocated into address
/// space records.
class WebCLRelocator : public WebCLTransformingVisitor
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

/// \brief Finds array subscriptions and pointer dereferences.
class WebCLAccessor : public WebCLTransformingVisitor
{
public:

    explicit WebCLAccessor(clang::CompilerInstance &instance);
    virtual ~WebCLAccessor();

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
