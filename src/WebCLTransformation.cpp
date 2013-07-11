#include "WebCLTransformation.hpp"
#include "WebCLTransformations.hpp"
#include "WebCLTransformerConfiguration.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <sstream>

// WebCLTransformation

WebCLTransformation::Removals WebCLTransformation::removals_;

WebCLTransformation::WebCLTransformation(WebCLTransformations &transformations)
    : WebCLReporter(transformations.getCompilerInstance())
    , transformations_(transformations)
{
}

WebCLTransformation::~WebCLTransformation()
{
}

bool WebCLTransformation::hasBeenRemoved(clang::SourceLocation location)
{
    // We don't want to transform code that has been commented out by
    // previous transformations.
    for (Removals::iterator i = removals_.begin(); i != removals_.end(); ++i) {
        clang::SourceRange range = (*i);
        if ((range.getBegin() < location) && (location < range.getEnd()))
            return true;
    }
    return false;
}

bool WebCLTransformation::prepend(
    clang::SourceLocation begin, const std::string &prologue)
{
    if (hasBeenRemoved(begin))
        return true;
    // Rewriter returns false on success.
    return !transformations_.getRewriter().InsertTextBefore(begin, prologue);
}

bool WebCLTransformation::append(
    clang::SourceLocation end, const std::string &epilogue)
{
    if (hasBeenRemoved(end))
        return true;
    // Rewriter returns false on success.
    return !transformations_.getRewriter().InsertTextAfter(end, epilogue);
}

bool WebCLTransformation::replace(
    clang::SourceRange range, const std::string &replacement)
{
    if (hasBeenRemoved(range.getBegin()) || hasBeenRemoved(range.getEnd()))
        return true;
    // Rewriter returns false on success.
    return !transformations_.getRewriter().ReplaceText(range, replacement);
}

bool WebCLTransformation::remove(
    clang::SourceRange range, const std::string &replacement)
{
    if (hasBeenRemoved(range.getBegin()) || hasBeenRemoved(range.getEnd()))
        return true;

    removals_.push_back(range);

    static const std::string prologue = "\n#if 0\n";
    static const std::string epilogue = "\n#endif\n";

    bool status = true;
    status = status && prepend(range.getBegin(), prologue);
    status = status && append(range.getEnd(), epilogue + replacement);
    return status;
}

/// WebCLRecursiveTransformation

WebCLRecursiveTransformation::WebCLRecursiveTransformation(WebCLTransformations &transformations)
    : WebCLTransformation(transformations)
{
}

WebCLRecursiveTransformation::~WebCLRecursiveTransformation()
{
}

bool WebCLRecursiveTransformation::rewrite()
{
    std::string text;
    clang::SourceRange range;
    if (!getAsText(text, range))
        return false;
    return replace(range, text);
}


// WebCLSizeParameterInsertion

WebCLSizeParameterInsertion::WebCLSizeParameterInsertion(
    WebCLTransformations &transformations, clang::ParmVarDecl *decl)
    : WebCLTransformation(transformations), decl_(decl)
{
}

WebCLSizeParameterInsertion::~WebCLSizeParameterInsertion()
{
}

bool WebCLSizeParameterInsertion::rewrite()
{
    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string parameter =
        cfg.sizeParameterType_ + " " + cfg.getNameOfSizeParameter(decl_);

    // Doesn't work as expected:
    //return !rewriter.InsertTextAfter(decl_->getLocEnd(), ", " + parameter);

    clang::Rewriter &rewriter = transformations_.getRewriter();
    const std::string replacement =
        rewriter.getRewrittenText(decl_->getSourceRange()) + ", " + parameter;
    return replace(decl_->getSourceRange(), replacement);
}

