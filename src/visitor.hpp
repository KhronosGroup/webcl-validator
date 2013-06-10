#ifndef WEBCLVALIDATOR_WEBCLVISITOR
#define WEBCLVALIDATOR_WEBCLVISITOR

#include "reporter.hpp"
#include "transformer.hpp"

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
