#include "WebCLTransformation.hpp"
#include "WebCLTransformerConfiguration.hpp"

#include "clang/AST/Expr.h"
#include "clang/Rewrite/Core/Rewriter.h"

// WebCLTransformation

WebCLTransformation::WebCLTransformation(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
{
}

WebCLTransformation::~WebCLTransformation()
{
}

// WebCLVariableRelocation

WebCLVariableRelocation::WebCLVariableRelocation(
    clang::CompilerInstance &instance,
    clang::DeclStmt *stmt, clang::VarDecl *decl)
    : WebCLTransformation(instance)
    , stmt_(stmt), decl_(decl)
{
}

WebCLVariableRelocation::~WebCLVariableRelocation()
{
}

bool WebCLVariableRelocation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    clang::SourceRange range = stmt_ ? stmt_->getSourceRange() : decl_->getSourceRange();
    if (!stmt_) {
        range.setEnd(range.getEnd().getLocWithOffset(1));
    }

    static const std::string prologue = "\n#if 0\n";
    static const std::string epilogue = "\n#endif\n";
    const std::string replacement =
        prologue + rewriter.getRewrittenText(range) + epilogue;
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(range, replacement);
}

// WebCLRecordParameterInsertion

WebCLRecordParameterInsertion::WebCLRecordParameterInsertion(
    clang::CompilerInstance &instance, clang::FunctionDecl *decl)
    : WebCLTransformation(instance), decl_(decl)
{
}

WebCLRecordParameterInsertion::~WebCLRecordParameterInsertion()
{
}

bool WebCLRecordParameterInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string parameter =
        cfg.addressSpaceRecordType_ + " *" + cfg.addressSpaceRecordName_;
    // Rewriter returns false on success.
    return !rewriter.InsertTextBefore(
        decl_->getParamDecl(0)->getLocStart(), parameter + ", ");
}

// WebCLRecordArgumentInsertion

WebCLRecordArgumentInsertion::WebCLRecordArgumentInsertion(
    clang::CompilerInstance &instance, clang::CallExpr *expr)
    : WebCLTransformation(instance), expr_(expr)
{
}

WebCLRecordArgumentInsertion::~WebCLRecordArgumentInsertion()
{
}

bool WebCLRecordArgumentInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const unsigned int numArguments = expr_->getNumArgs();

    clang::SourceLocation location = expr_->getRParenLoc();
    if (numArguments) {
        clang::Expr *first = expr_->getArg(0);
        if (!first) {
            error(expr_->getLocStart(), "Invalid first argument.");
            return false;
        }
        location = first->getLocStart();
    }

    const std::string comma = (numArguments > 0) ? ", " : "";
    const std::string argument = cfg.addressSpaceRecordName_ + comma;

    // Rewriter returns false on success.
    return !rewriter.InsertTextBefore(location, argument);
}

// WebCLSizeParameterInsertion

WebCLSizeParameterInsertion::WebCLSizeParameterInsertion(
    clang::CompilerInstance &instance, clang::ParmVarDecl *decl)
    : WebCLTransformation(instance), decl_(decl)
{
}

WebCLSizeParameterInsertion::~WebCLSizeParameterInsertion()
{
}

bool WebCLSizeParameterInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string parameter =
        cfg.sizeParameterType_ + " " + cfg.getNameOfSizeParameter(decl_);

    // Doesn't work as expected:
    //return !rewriter.InsertTextAfter(decl_->getLocEnd(), ", " + parameter);

    // Rewriter returns false on success.
    const std::string replacement =
        rewriter.getRewrittenText(decl_->getSourceRange()) + ", " + parameter;
    return !rewriter.ReplaceText(decl_->getSourceRange(), replacement);
}

// WebCLArraySubscriptTransformation

WebCLArraySubscriptTransformation::WebCLArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr)
    : WebCLTransformation(instance), expr_(expr)
{
}

WebCLArraySubscriptTransformation::~WebCLArraySubscriptTransformation()
{
}

std::string WebCLArraySubscriptTransformation::getBaseAsText(clang::Rewriter &rewriter)
{
    clang::Expr *base = expr_->getBase();
    if (!base) {
        error(expr_->getLocStart(), "Invalid indexed array.");
        return "";
    }

    return rewriter.getRewrittenText(base->getSourceRange());
}

std::string WebCLArraySubscriptTransformation::getIndexAsText(clang::Rewriter &rewriter)
{
    clang::Expr *index = expr_->getIdx();
    if (!index) {
        error(expr_->getLocStart(), "Invalid array index.");
        return "";
    }
    clang::Expr *plainIndex = index->IgnoreParens();
    if (!plainIndex) {
        error(index->getLocStart(), "Can't remove parentheses from array index.");
        return "";
    }

    return rewriter.getRewrittenText(plainIndex->getSourceRange());
}

bool WebCLArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker(expr_->getType()) + "(" +
        cfg.addressSpaceRecordName_ + ", " + base + ", " + index +
        ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLConstantArraySubscriptTransformation

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
    llvm::APInt &bound)
    : WebCLArraySubscriptTransformation(instance, expr)
    , bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "("
        + index + ", " + bound_.toString(10, false) + "UL" +
        ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLKernelArraySubscriptTransformation

WebCLKernelArraySubscriptTransformation::WebCLKernelArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
    const std::string &bound)
    : WebCLArraySubscriptTransformation(instance, expr)
    , bound_(bound)
{
}

WebCLKernelArraySubscriptTransformation::~WebCLKernelArraySubscriptTransformation()
{
}

bool WebCLKernelArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "(" + index + ", " + bound_ + ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLPointerDereferenceTransformation

WebCLPointerDereferenceTransformation::WebCLPointerDereferenceTransformation(
    clang::CompilerInstance &instance, clang::Expr *expr)
    : WebCLTransformation(instance), expr_(expr)
{
}

WebCLPointerDereferenceTransformation::~WebCLPointerDereferenceTransformation()
{
}

bool WebCLPointerDereferenceTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string replacement =
        cfg.getNameOfPointerChecker(getPointeeType(expr_)) + "(" +
        cfg.addressSpaceRecordName_ + ", " + rewriter.getRewrittenText(expr_->getSourceRange()) +
        ")";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}
