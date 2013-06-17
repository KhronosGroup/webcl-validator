#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMATION
#define WEBCLVALIDATOR_WEBCLTRANSFORMATION

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"

#include "llvm/ADT/APInt.h"

namespace clang {
    class ArraySubscriptExpr;
    class CallExpr;
    class DeclStmt;
    class Expr;
    class FunctionDecl;
    class ParmVarDecl;
    class Rewriter;
    class VarDecl;
}

class WebCLTransformerConfiguration;

/// \brief Transforms an AST node by rewriting its contents in the
/// source code domain.
class WebCLTransformation : public WebCLReporter
{
public:

    WebCLTransformation(clang::CompilerInstance &instance);
    virtual ~WebCLTransformation();

    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter) = 0;
};

/// \brief Relocate a variable into a corresponding address space
/// record.
class WebCLVariableRelocation : public WebCLTransformation
{
public:

    WebCLVariableRelocation(
        clang::CompilerInstance &instance,
        clang::DeclStmt *stmt, clang::VarDecl *decl);
    virtual ~WebCLVariableRelocation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::DeclStmt *stmt_;
    clang::VarDecl *decl_;
};

/// \brief Inserts address space record parameter at the first
/// position in parameter list.
class WebCLRecordParameterInsertion : public WebCLTransformation
{
public:

    WebCLRecordParameterInsertion(
        clang::CompilerInstance &instance, clang::FunctionDecl *decl);
    virtual ~WebCLRecordParameterInsertion();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::FunctionDecl *decl_;
};

class WebCLRecordArgumentInsertion : public WebCLTransformation
{
public:

    WebCLRecordArgumentInsertion(
        clang::CompilerInstance &instance, clang::CallExpr *expr);
    virtual ~WebCLRecordArgumentInsertion();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::CallExpr *expr_;
};

/// \brief Inserts size parameter that is linked to a kernel memory
/// object.
class WebCLSizeParameterInsertion : public WebCLTransformation
{
public:

    WebCLSizeParameterInsertion(
        clang::CompilerInstance &instance, clang::ParmVarDecl *decl);
    virtual ~WebCLSizeParameterInsertion();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::ParmVarDecl *decl_;
};

/// \brief Adds an index check to array subscription when the limits
/// are unknown at compile time.
class WebCLArraySubscriptTransformation : public WebCLTransformation
{
public:

    WebCLArraySubscriptTransformation(
        clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr);
    virtual ~WebCLArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    std::string getBaseAsText(clang::Rewriter &rewriter);
    std::string getIndexAsText(clang::Rewriter &rewriter);

    clang::ArraySubscriptExpr *expr_;
};

/// \brief Adds an index check to array subscription when the limits
/// are known at compile time.
class WebCLConstantArraySubscriptTransformation : public WebCLArraySubscriptTransformation
{
public:

    WebCLConstantArraySubscriptTransformation(
        clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
        llvm::APInt &bound);
    virtual ~WebCLConstantArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    llvm::APInt bound_;
};

/// \brief Adds an index check to kernel parameter array subscription.
class WebCLKernelArraySubscriptTransformation : public WebCLArraySubscriptTransformation
{
public:

    WebCLKernelArraySubscriptTransformation(
        clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
        const std::string &bound);
    virtual ~WebCLKernelArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    std::string bound_;
};

/// \brief Adds pointer limit check.
class WebCLPointerDereferenceTransformation : public WebCLTransformation
                                            , public WebCLHelper
{
public:

    WebCLPointerDereferenceTransformation(
        clang::CompilerInstance &instance, clang::Expr *expr);
    virtual ~WebCLPointerDereferenceTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::Expr *expr_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMATION
