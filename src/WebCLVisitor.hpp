#ifndef WEBCLVALIDATOR_WEBCLVISITOR
#define WEBCLVALIDATOR_WEBCLVISITOR

#include "WebCLReporter.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/RecursiveASTVisitor.h"

namespace clang {
    class TranslationUnitDecl;
}

/// \brief Complains about WebCL limitations in OpenCL C code.
class WebCLRestrictor : public WebCLReporter
                      , public clang::RecursiveASTVisitor<WebCLRestrictor>
{
public:

    explicit WebCLRestrictor(clang::CompilerInstance &instance);
    ~WebCLRestrictor();

    /// \see RecursiveASTVisitor::VisitFunctionDecl
    bool VisitFunctionDecl(clang::FunctionDecl *decl);

    /// \see RecursiveASTVisitor::VisitParmVar
    bool VisitParmVarDecl(clang::ParmVarDecl *decl);

private:

    void checkStructureParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
    void check3dImageParameter(
        clang::FunctionDecl *decl,
        clang::SourceLocation typeLocation, const clang::Type *type);
};

/// \brief Finds variables that need to be relocated into address
/// space records.
class WebCLRelocator : public WebCLReporter
                     , public WebCLTransformerClient
                     , public clang::RecursiveASTVisitor<WebCLRelocator>
{
public:

    explicit WebCLRelocator(clang::CompilerInstance &instance);
    ~WebCLRelocator();

    /// \see clang::RecursiveASTVisitor::VisitDeclStmt
    bool VisitDeclStmt(clang::DeclStmt *stmt);
    /// \see clang::RecursiveASTVisitor::VisitVarDecl
    bool VisitVarDecl(clang::VarDecl *decl);
    /// \see clang::RecursiveASTVisitor::VisitUnaryOperator
    bool VisitUnaryOperator(clang::UnaryOperator *expr);

private:

    clang::VarDecl *getRelocatedVariable(clang::Expr *expr);

    clang::DeclStmt *current_;
    typedef std::pair<clang::VarDecl*, clang::DeclStmt*> RelocationCandidate;
    typedef std::map<clang::VarDecl*, clang::DeclStmt*> RelocationCandidates;
    RelocationCandidates relocationCandidates_;
};

/// Finds function parameter lists that need to be extended. Also
/// finds function calls and augments argument lists when necessary.
class WebCLParameterizer : public WebCLReporter
                         , public WebCLTransformerClient
                         , public clang::RecursiveASTVisitor<WebCLParameterizer>
{
public:

    explicit WebCLParameterizer(clang::CompilerInstance &instance);
    ~WebCLParameterizer();

    /// Checks whether parameter lists need to be extended:
    ///
    /// - Add array size parameters for kernel array parameters.
    /// - Add address space record parameters for functions that need
    ///   to do pointer and index checking.
    ///
    /// \see clang::RecursiveASTVisitor::VisitFunctionDecl
    bool VisitFunctionDecl(clang::FunctionDecl *decl);

    /// Checks whether argument lists need to be extended:
    ///
    /// - Add address space argument for functions that need to do
    ///  pointer and index checking.
    ///
    /// \see clang::RecursiveASTVisitor::VisitCallExpr
    bool VisitCallExpr(clang::CallExpr *expr);

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
class WebCLAccessor : public WebCLReporter
                    , public WebCLTransformerClient
                    , public clang::RecursiveASTVisitor<WebCLAccessor>
{
public:

    WebCLAccessor(clang::CompilerInstance &instance);
    ~WebCLAccessor();

    /// \see clang::RecursiveASTVisitor::VisitArraySubscriptExpr
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *expr);

    /// \see clang::RecursiveASTVisitor::VisitUnaryOperator
    bool VisitUnaryOperator(clang::UnaryOperator *expr);

private:

    bool getIndexedArraySize(const clang::Expr *base, llvm::APSInt &size);

    bool getArrayIndexValue(const clang::Expr *index, llvm::APSInt &value);

    bool isPointerCheckNeeded(const clang::Expr *expr);
};

#endif // WEBCLVALIDATOR_WEBCLVISITOR
