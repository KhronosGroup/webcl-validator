#include "transformer.hpp"

#include "clang/Frontend/CompilerInstance.h"

WebCLTransformer::WebCLTransformer(clang::CompilerInstance &instance,
                                   clang::Sema &sema)
    : WebCLReporter(instance)
    , clang::TreeTransform<WebCLTransformer>(sema)
{
}

WebCLTransformer::~WebCLTransformer()
{
}

bool WebCLTransformer::AlwaysRebuild()
{
    return true;
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    clang::Expr *base = expr->getBase();
    if (!base) {
        error(expr->getLocStart(), "Invalid indexed array.");
        return;
    }
    clang::Expr *index = expr->getIdx();
    if (!index) {
        error(expr->getLocStart(), "Invalid array index.");
        return;
    }

    clang::ExprResult baseClone = getDerived().TransformExpr(base);
    if (baseClone.isInvalid()) {
        error(base->getLocStart(), "Can't clone indexed array.");
        return;
    }
    clang::ExprResult indexClone = getDerived().TransformExpr(index);
    if (indexClone.isInvalid()) {
        error(index->getLocStart(), "Can't clone array index.");
        return;
    }

    clang::IntegerLiteral *boundLiteral = clang::IntegerLiteral::Create(
        SemaRef.Context, bound, SemaRef.Context.getSizeType(), clang::SourceLocation());
    if (!boundLiteral) {
        error(index->getLocStart(), "Can't create bound for array index.");
        return;
    }
    clang::ExprResult boundResult = TransformIntegerLiteral(boundLiteral);
    if (boundResult.isInvalid()) {
        error(index->getLocStart(), "Can't transform bound of array index.");
        return;
    }
    clang::ExprResult clampedIndex = getDerived().RebuildBinaryOperator(
        clang::SourceLocation(), clang::BO_Rem, indexClone.get(), boundResult.get());
    if (clampedIndex.isInvalid()) {
        error(index->getLocStart(), "Can't create check for array index bound.");
        return;
    }

    // i[array] -> array[i % bound]
    expr->setLHS(baseClone.get());
    expr->setRHS(clampedIndex.get());
}
