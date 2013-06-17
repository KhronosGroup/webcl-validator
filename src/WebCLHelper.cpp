#include "WebCLHelper.hpp"

#include "clang/AST/Expr.h"

WebCLHelper::WebCLHelper()
{
}

WebCLHelper::~WebCLHelper()
{
}

clang::QualType WebCLHelper::getPointeeType(clang::Expr *expr)
{
    const clang::Type *type = expr->getType().getTypePtrOrNull();
    if (!type)
        return clang::QualType();
    return type->getPointeeType();
}

clang::Expr *WebCLHelper::pruneExpression(clang::Expr *expr)
{
    expr = expr->IgnoreImpCasts();
    if (!expr)
        return NULL;

    expr = expr->IgnoreParens();
    if (!expr)
        return NULL;

    return expr;
}

clang::ValueDecl *WebCLHelper::pruneValue(clang::Expr *expr)
{
    expr = pruneExpression(expr);
    if (!expr)
        return NULL;

    clang::DeclRefExpr *refExpr = llvm::dyn_cast<clang::DeclRefExpr>(expr);
    if (!refExpr)
        return NULL;

    return refExpr->getDecl();
}
