#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMATION
#define WEBCLVALIDATOR_WEBCLTRANSFORMATION

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"

#include "clang/AST/RecursiveASTVisitor.h"

#include "llvm/ADT/APInt.h"

#include <vector>

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

class WebCLTransformations;
class WebCLTransformerConfiguration;

/// \brief Transforms an AST node by rewriting its contents in the
/// source code domain.
class WebCLTransformation : public WebCLReporter
{
public:

    WebCLTransformation(WebCLTransformations &transformations);
    virtual ~WebCLTransformation();

    virtual bool rewrite() = 0;

protected:

    /// \return Whether a transformation has already removed a range
    /// that contains the location.
    bool hasBeenRemoved(clang::SourceLocation location);

    /// \brief Add text before location.
    bool prepend(clang::SourceLocation begin, const std::string &prologue);

    /// \brief Add text after location.
    bool append(clang::SourceLocation end, const std::string &epilogue);

    /// \brief Replace range with given text.
    bool replace(clang::SourceRange range, const std::string &replacement);

    /// \brief Remove range by commenting it out.
    ///
    /// Additionally a replacement text can be given that will be
    /// inserted right next to the region that was commented out. Pass
    /// empty replacement text if you only want to remove code.
    bool remove(clang::SourceRange range, const std::string &replacement);

    WebCLTransformations &transformations_;
    typedef std::vector<clang::SourceRange> Removals;
    static Removals removals_;
};

/// \brief A transformation that may contain other transformations
/// recursively.
class WebCLRecursiveTransformation : public WebCLTransformation
{
public:

    WebCLRecursiveTransformation(WebCLTransformations &transformations);
    virtual ~WebCLRecursiveTransformation();

    virtual bool rewrite();

    /// \return Whether the transformation can be successfully
    /// represented as a text confined to some range.
    virtual bool getAsText(std::string &text, clang::SourceRange &range) = 0;
};

/// \brief Relocate a variable into a corresponding address space
/// record.
class WebCLVariableRelocation : public WebCLTransformation
{
public:

    WebCLVariableRelocation(
        WebCLTransformations &transformations,
        clang::DeclStmt *stmt, clang::VarDecl *decl);
    virtual ~WebCLVariableRelocation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite();

protected:

    const std::string getTransformedInitializer(clang::Expr *expr);
    const std::string initializePrimitive(const std::string &prefix, clang::Expr *expr);
    const std::string initializeArray(
        const llvm::APInt &size, const std::string &prefix, clang::Expr *expr);

    clang::DeclStmt *stmt_;
    clang::VarDecl *decl_;
};

/// \brief Inserts address space record parameter at the first
/// position in parameter list.
class WebCLRecordParameterInsertion : public WebCLTransformation
{
public:

    WebCLRecordParameterInsertion(
        WebCLTransformations &transformations, clang::FunctionDecl *decl);
    virtual ~WebCLRecordParameterInsertion();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite();

protected:

    clang::FunctionDecl *decl_;
};

class WebCLRecordArgumentInsertion : public WebCLRecursiveTransformation
{
public:

    WebCLRecordArgumentInsertion(
        WebCLTransformations &transformations, clang::CallExpr *expr);
    virtual ~WebCLRecordArgumentInsertion();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

protected:

    clang::CallExpr *expr_;
};

/// \brief Inserts size parameter that is linked to a kernel memory
/// object.
class WebCLSizeParameterInsertion : public WebCLTransformation
{
public:

    WebCLSizeParameterInsertion(
        WebCLTransformations &transformations, clang::ParmVarDecl *decl);
    virtual ~WebCLSizeParameterInsertion();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite();

protected:

    clang::ParmVarDecl *decl_;
};

/// \brief Adds an index check to array subscription when the limits
/// are unknown at compile time.
class WebCLArraySubscriptTransformation : public WebCLRecursiveTransformation
{
public:

    WebCLArraySubscriptTransformation(
        WebCLTransformations &transformations, clang::ArraySubscriptExpr *expr);
    virtual ~WebCLArraySubscriptTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

protected:

    std::string getBaseAsText();
    std::string getIndexAsText();

    clang::ArraySubscriptExpr *expr_;
};

/// \brief Adds an index check to array subscription when the limits
/// are known at compile time.
class WebCLConstantArraySubscriptTransformation : public WebCLArraySubscriptTransformation
{
public:

    WebCLConstantArraySubscriptTransformation(
        WebCLTransformations &transformations,
        clang::ArraySubscriptExpr *expr, llvm::APInt &bound);
    virtual ~WebCLConstantArraySubscriptTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

protected:

    llvm::APInt bound_;
};

/// \brief Adds an index check to kernel parameter array subscription.
class WebCLKernelArraySubscriptTransformation : public WebCLArraySubscriptTransformation
{
public:

    WebCLKernelArraySubscriptTransformation(
        WebCLTransformations &transformations,
        clang::ArraySubscriptExpr *expr, const std::string &bound);
    virtual ~WebCLKernelArraySubscriptTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

protected:

    std::string bound_;
};

/// \brief Adds pointer limit check.
class WebCLPointerDereferenceTransformation : public WebCLRecursiveTransformation
                                            , public WebCLHelper
{
public:

    WebCLPointerDereferenceTransformation(
        WebCLTransformations &transformations, clang::Expr *expr);
    virtual ~WebCLPointerDereferenceTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

protected:

    clang::Expr *expr_;
};

/// \brief Helper for transforming expressions recursively.
class WebCLExpressionTransformation : public WebCLRecursiveTransformation
                                    , public clang::RecursiveASTVisitor<WebCLExpressionTransformation>

{
public:

    explicit WebCLExpressionTransformation(
        WebCLTransformations &transformations, clang::Expr *expr);
    virtual ~WebCLExpressionTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(std::string &text, clang::SourceRange &range);

    /// \see clang::RecursiveASTVisitor::VisitExpr
    bool VisitExpr(clang::Expr *expr);

protected:

    clang::Expr *expr_;
    bool status_;
    std::string text_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMATION
