#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "reporter.hpp"

#include "llvm/ADT/APInt.h"

#include <map>

namespace clang {
    class Expr;
    class Rewriter;
    class ArraySubscriptExpr;
}

/// \brief Transforms an AST node by rewriting its contents in the
/// source code domain.
class WebCLTransformation
{
public:
    virtual bool rewrite(clang::Rewriter &rewriter) = 0;
};

/// \brief Adds an index check to array subscription when the limits
/// are known at compile time.
class WebCLConstantArraySubscriptTransformation : public WebCLTransformation
                                                , public WebCLReporter
{
public:
    WebCLConstantArraySubscriptTransformation(
        clang::CompilerInstance &instance,
        clang::ArraySubscriptExpr *expr, llvm::APInt &bound);
    ~WebCLConstantArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(clang::Rewriter &rewriter);

private:

    clang::ArraySubscriptExpr *expr_;
    llvm::APInt bound_;
};

/// \brief Performs AST node transformations.
class WebCLTransformer : public WebCLTransformation
                       , public WebCLReporter
{
public:

    explicit WebCLTransformer(clang::CompilerInstance &instance);
    ~WebCLTransformer();

    /// Applies all AST transformations.
    ///
    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(clang::Rewriter &rewriter);

    /// Modify array indexing:
    /// array[index] -> array[index % bound]
    void addArrayIndexCheck(clang::ArraySubscriptExpr *expr, llvm::APInt &bound);

private:

    typedef std::map<clang::Expr*, WebCLTransformation*> ExprTransformations;
    ExprTransformations exprTransformations_;
};

/// \brief Mixin class for making AST transformations available after
/// construction.
class WebCLTransformerClient
{
public:

    WebCLTransformerClient();
    ~WebCLTransformerClient();

    WebCLTransformer& getTransformer();
    void setTransformer(WebCLTransformer &transformer);

private:

    WebCLTransformer *transformer_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMER
