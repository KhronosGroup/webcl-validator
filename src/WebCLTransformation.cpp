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

// WebCLVariableRelocation

WebCLVariableRelocation::WebCLVariableRelocation(
    WebCLTransformations &transformations,
    clang::DeclStmt *stmt, clang::VarDecl *decl)
    : WebCLTransformation(transformations)
    , stmt_(stmt), decl_(decl)
{
}

WebCLVariableRelocation::~WebCLVariableRelocation()
{
}

bool WebCLVariableRelocation::rewrite()
{
    clang::SourceRange range = stmt_ ? stmt_->getSourceRange() : decl_->getSourceRange();
    // I have no justification for these magical offsets. However,
    // they are required for correct positioning.
    const int offset = stmt_ ? 1 : 2;
    range.setEnd(range.getEnd().getLocWithOffset(offset));

    std::string replacement;
    if (clang::Expr *init = decl_->getInit()) {
        WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
        clang::ASTContext &context = instance_.getASTContext();
        clang::QualType type = decl_->getType();

        if (!init->isConstantInitializer(context, false)) {
            const std::string prefix =
                cfg.indentation_ +
                cfg.getNameOfAddressSpaceRecord(type) + "." +
                cfg.getNameOfRelocatedVariable(decl_);

            const clang::ConstantArrayType *arrayType = context.getAsConstantArrayType(type);
            if (arrayType) {
                replacement = initializeArray(arrayType->getSize(), prefix, init);
            } else {
                replacement = initializePrimitive(prefix, init);
            }

            if (!replacement.size())
                return false;
        }
    }

    return remove(range, replacement);
}

const std::string WebCLVariableRelocation::getTransformedInitializer(clang::Expr *expr)
{
    std::string text;
    clang::SourceRange range;
    WebCLExpressionTransformation transformation(transformations_, expr);
    transformation.getAsText(text, range);
    return text;
}

const std::string WebCLVariableRelocation::initializePrimitive(
    const std::string &prefix, clang::Expr *expr)
{
    std::ostringstream result;
    result << prefix << " = "
           << getTransformedInitializer(expr)
           << ";";
    return result.str();
}

const std::string WebCLVariableRelocation::initializeArray(
    const llvm::APInt &size, const std::string &prefix, clang::Expr *expr)
{
    std::ostringstream result;

    clang::InitListExpr *initializers =
        llvm::dyn_cast<clang::InitListExpr>(expr);

    if (!initializers) {
        error(expr->getLocStart(), "Invalid array initializer.");
        return result.str();
    }

    const uint64_t numElements = size.getZExtValue();
    const uint64_t numInitializers = initializers->getNumInits();
    for (uint64_t element = 0; element < numElements; ++element) {
        const uint64_t initializer =
            (element < numInitializers) ? element : (numInitializers - 1);
        expr = initializers->getInit(initializer);
        result << prefix << "[" << element << "] = "
               << getTransformedInitializer(expr)
               << ";\n";
    }

    return result.str();
}

// WebCLRecordParameterInsertion

WebCLRecordParameterInsertion::WebCLRecordParameterInsertion(
    WebCLTransformations &transformations, clang::FunctionDecl *decl)
    : WebCLTransformation(transformations), decl_(decl)
{
}

WebCLRecordParameterInsertion::~WebCLRecordParameterInsertion()
{
}

bool WebCLRecordParameterInsertion::rewrite()
{
    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string parameter =
        cfg.addressSpaceRecordType_ + " *" + cfg.addressSpaceRecordName_;
    return prepend(decl_->getParamDecl(0)->getLocStart(), parameter + ", ");
}

// WebCLRecordArgumentInsertion

WebCLRecordArgumentInsertion::WebCLRecordArgumentInsertion(
    WebCLTransformations &transformations, clang::CallExpr *expr)
    : WebCLRecursiveTransformation(transformations), expr_(expr)
{
}

WebCLRecordArgumentInsertion::~WebCLRecordArgumentInsertion()
{
}

bool WebCLRecordArgumentInsertion::getAsText(
    std::string &text, clang::SourceRange &range)
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

    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string comma = (numArguments > 0) ? ", " : "";
    const std::string argument = cfg.addressSpaceRecordName_ + comma;

    clang::Rewriter &rewriter = transformations_.getRewriter();
    clang::SourceRange head(expr_->getLocStart(), location.getLocWithOffset(-1));
    clang::SourceRange tail(location, expr_->getLocEnd());
    text = rewriter.getRewrittenText(head) + argument + rewriter.getRewrittenText(tail);
    range = expr_->getSourceRange();
    return true;
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

// WebCLArraySubscriptTransformation

WebCLArraySubscriptTransformation::WebCLArraySubscriptTransformation(
    WebCLTransformations &transformations, clang::ArraySubscriptExpr *expr)
    : WebCLRecursiveTransformation(transformations), expr_(expr)
{
}

WebCLArraySubscriptTransformation::~WebCLArraySubscriptTransformation()
{
}

std::string WebCLArraySubscriptTransformation::getBaseAsText()
{
    clang::Expr *base = expr_->getBase();
    if (!base) {
        error(expr_->getLocStart(), "Invalid indexed array.");
        return "";
    }

    clang::Rewriter &rewriter = transformations_.getRewriter();
    return rewriter.getRewrittenText(base->getSourceRange());
}

std::string WebCLArraySubscriptTransformation::getIndexAsText()
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

    clang::Rewriter &rewriter = transformations_.getRewriter();
    return rewriter.getRewrittenText(plainIndex->getSourceRange());
}

bool WebCLArraySubscriptTransformation::getAsText(
    std::string &text, clang::SourceRange &range)
{
    const std::string base = getBaseAsText();
    const std::string index = getIndexAsText();

    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker(expr_->getType()) + "(" +
        cfg.addressSpaceRecordName_ + ", " + base + ", " + index +
        ")]";

    text = replacement;
    range = expr_->getSourceRange();
    return true;
}

// WebCLConstantArraySubscriptTransformation

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    WebCLTransformations &transformations,
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
    : WebCLArraySubscriptTransformation(transformations, expr)
    , bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::getAsText(
    std::string &text, clang::SourceRange &range)
{
    const std::string base = getBaseAsText();
    const std::string index = getIndexAsText();

    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "("
        + index + ", " + bound_.toString(10, false) + "UL" +
        ")]";

    text = replacement;
    range = expr_->getSourceRange();
    return true;
}

// WebCLKernelArraySubscriptTransformation

WebCLKernelArraySubscriptTransformation::WebCLKernelArraySubscriptTransformation(
    WebCLTransformations &transformations,
    clang::ArraySubscriptExpr *expr, const std::string &bound)
    : WebCLArraySubscriptTransformation(transformations, expr)
    , bound_(bound)
{
}

WebCLKernelArraySubscriptTransformation::~WebCLKernelArraySubscriptTransformation()
{
}

bool WebCLKernelArraySubscriptTransformation::getAsText(
    std::string &text, clang::SourceRange &range)
{
    const std::string base = getBaseAsText();
    const std::string index = getIndexAsText();

    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "(" + index + ", " + bound_ + ")]";

    text = replacement;
    range = expr_->getSourceRange();
    return true;
}

// WebCLPointerDereferenceTransformation

WebCLPointerDereferenceTransformation::WebCLPointerDereferenceTransformation(
    WebCLTransformations &transformations, clang::Expr *expr)
    : WebCLRecursiveTransformation(transformations), expr_(expr)
{
}

WebCLPointerDereferenceTransformation::~WebCLPointerDereferenceTransformation()
{
}

bool WebCLPointerDereferenceTransformation::getAsText(
    std::string &text, clang::SourceRange &range)

{
    WebCLTransformerConfiguration &cfg = transformations_.getConfiguration();
    clang::Rewriter &rewriter = transformations_.getRewriter();
    const std::string replacement =
        cfg.getNameOfPointerChecker(getPointeeType(expr_)) + "(" +
        cfg.addressSpaceRecordName_ + ", " + rewriter.getRewrittenText(expr_->getSourceRange()) +
        ")";

    text = replacement;
    range = expr_->getSourceRange();
    return true;
}

// WebCLExpressionTransformation

WebCLExpressionTransformation::WebCLExpressionTransformation(
    WebCLTransformations &transformations,
    clang::Expr *expr)
    : WebCLRecursiveTransformation(transformations)
    , expr_(expr), status_(false), text_()
{
}

WebCLExpressionTransformation::~WebCLExpressionTransformation()
{
}

bool WebCLExpressionTransformation::getAsText(
    std::string &text, clang::SourceRange &range)
{
    TraverseStmt(expr_);
    if (!status_)
        return false;

    clang::Rewriter &rewriter = transformations_.getRewriter();
    range = expr_->getSourceRange();
    text = text_.size() ? text_ : rewriter.getRewrittenText(range);
    return true;
}

bool WebCLExpressionTransformation::VisitExpr(clang::Expr *expr)
{
    status_ = true;

    WebCLRecursiveTransformation *transformation = transformations_.getTransformation(expr);
    if (!transformation)
        return true;

    clang::SourceRange range;
    status_ = transformation->getAsText(text_, range);
    if (!status_) {
        error(expr->getLocStart(), "Internal error. Transformation can't be represented as text.");
        return false;
    }

    clang::Rewriter &rewriter = transformations_.getRewriter();
    clang::SourceLocation rangeBegin =
        range.getBegin().getLocWithOffset(-1);
    clang::SourceLocation rangeEnd =
        range.getBegin().getLocWithOffset(rewriter.getRangeSize(range));
    clang::SourceRange head(expr_->getLocStart(), rangeBegin);
    clang::SourceRange tail(rangeEnd, expr_->getLocEnd());
    text_ = rewriter.getRewrittenText(head) + text_ + rewriter.getRewrittenText(tail);
    return false;
}
