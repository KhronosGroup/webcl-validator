#include "transformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    clang::CompilerInstance &instance,
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
    : WebCLReporter(instance), expr_(expr), bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::rewrite(clang::Rewriter &rewriter)
{
    clang::Expr *base = expr_->getBase();
    if (!base) {
        error(expr_->getLocStart(), "Invalid indexed array.");
        return false;
    }
    clang::Expr *index = expr_->getIdx();
    if (!index) {
        error(expr_->getLocStart(), "Invalid array index.");
        return false;
    }
    clang::Expr *plainIndex = index->IgnoreParens();
    if (!plainIndex) {
        error(index->getLocStart(), "Can't remove parentheses from array index.");
        return false;
    }

    const std::string replacement =
        rewriter.getRewrittenText(base->getSourceRange()) + "[(" +
        rewriter.getRewrittenText(plainIndex->getSourceRange()) + ") % " +
        bound_.toString(10, false) + "UL]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

WebCLTransformer::WebCLTransformer(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
{
}

WebCLTransformer::~WebCLTransformer()
{
    for (ExprTransformations::iterator i = exprTransformations_.begin();
         i != exprTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        delete transformation;
    }
}

bool WebCLTransformer::rewrite(clang::Rewriter &rewriter)
{
    bool status = true;

    for (ExprTransformations::iterator i = exprTransformations_.begin();
         i != exprTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(rewriter);
    }

    return status;
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    WebCLTransformation *transformation =
        new WebCLConstantArraySubscriptTransformation(instance_, expr, bound);

    if (!transformation) {
        error(expr->getLocStart(), "Internal error. Can't create array index check.");
        return;
    }

    const std::pair<ExprTransformations::iterator, bool> status =
        exprTransformations_.insert(
            ExprTransformations::value_type(expr, transformation));

    if (!status.second) {
        error(expr->getLocStart(), "Array index check has been already created.");
        return;
    }
}

WebCLTransformerClient::WebCLTransformerClient()
    : transformer_(0)
{
}

WebCLTransformerClient::~WebCLTransformerClient()
{
}

WebCLTransformer& WebCLTransformerClient::getTransformer()
{
    assert(transformer_);
    return *transformer_;
}

void WebCLTransformerClient::setTransformer(WebCLTransformer &transformer)
{
    transformer_ = &transformer;
}
