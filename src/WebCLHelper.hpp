#ifndef WEBCLVALIDATOR_WEBCLHELPER
#define WEBCLVALIDATOR_WEBCLHELPER

#include "clang/AST/Type.h"

namespace clang {
    class Expr;
    class ValueDecl;
}

/// \brief Mixin class for examining AST nodes.
class WebCLHelper
{
public:

    WebCLHelper();
    ~WebCLHelper();

    /// Assume that the expression refers to a pointer and return the
    /// type of the pointed value.
    clang::QualType getPointeeType(clang::Expr *expr);

    /// Remove implicit casts and parentheses.
    clang::Expr *pruneExpression(clang::Expr *expr);

    /// Prune the expression, assume that it refers to value
    /// declaration and finally return the value declaration.
    clang::ValueDecl *pruneValue(clang::Expr *expr);
};

#endif // WEBCLVALIDATOR_WEBCLHELPER
