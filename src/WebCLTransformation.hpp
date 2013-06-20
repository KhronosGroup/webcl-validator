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

    WebCLTransformation(clang::CompilerInstance &instance);
    virtual ~WebCLTransformation();

    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter) = 0;

protected:

    /// \return Whether a transformation has already removed a range
    /// that contains the location.
    bool hasBeenRemoved(clang::SourceLocation location);

    /// \brief Add text before location.
    bool prepend(clang::SourceLocation begin, const std::string &prologue,
                 clang::Rewriter &rewriter);

    /// \brief Add text after location.
    bool append(clang::SourceLocation end, const std::string &epilogue,
                clang::Rewriter &rewriter);

    /// \brief Replace range with given text.
    bool replace(clang::SourceRange range, const std::string &replacement,
                 clang::Rewriter &rewriter);

    /// \brief Remove range by commenting it out.
    ///
    /// Additionally a replacement text can be given that will be
    /// inserted right next to the region that was commented out. Pass
    /// empty replacement text if you only want to remove code.
    bool remove(clang::SourceRange range, const std::string &replacement,
                clang::Rewriter &rewriter);

    typedef std::vector<clang::SourceRange> Removals;
    static Removals removals_;
};

/// \brief A transformation that may contain other transformations
/// recursively.
class WebCLRecursiveTransformation : public WebCLTransformation
{
public:

    WebCLRecursiveTransformation(clang::CompilerInstance &instance);
    virtual ~WebCLRecursiveTransformation();

    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

    /// \return Whether the transformation can be successfully
    /// represented as a text confined to some range.
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range) = 0;
};

/// \brief Relocate a variable into a corresponding address space
/// record.
class WebCLVariableRelocation : public WebCLTransformation
{
public:

    WebCLVariableRelocation(
        clang::CompilerInstance &instance, WebCLTransformations &transformations,
        clang::DeclStmt *stmt, clang::VarDecl *decl);
    virtual ~WebCLVariableRelocation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    const std::string getTransformedInitializer(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        clang::Expr *expr);
    const std::string initializePrimitive(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        const std::string &prefix, clang::Expr *expr);
    const std::string initializeArray(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        const llvm::APInt &size, const std::string &prefix, clang::Expr *expr);

    WebCLTransformations &transformations_;
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

class WebCLRecordArgumentInsertion : public WebCLRecursiveTransformation
{
public:

    WebCLRecordArgumentInsertion(
        clang::CompilerInstance &instance, clang::CallExpr *expr);
    virtual ~WebCLRecordArgumentInsertion();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

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
class WebCLArraySubscriptTransformation : public WebCLRecursiveTransformation
{
public:

    WebCLArraySubscriptTransformation(
        clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr);
    virtual ~WebCLArraySubscriptTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

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

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

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

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

protected:

    std::string bound_;
};

/// \brief Adds pointer limit check.
class WebCLPointerDereferenceTransformation : public WebCLRecursiveTransformation
                                            , public WebCLHelper
{
public:

    WebCLPointerDereferenceTransformation(
        clang::CompilerInstance &instance, clang::Expr *expr);
    virtual ~WebCLPointerDereferenceTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

protected:

    clang::Expr *expr_;
};

/// \brief Helper for transforming expressions recursively.
class WebCLExpressionTransformation : public WebCLRecursiveTransformation
                                    , public clang::RecursiveASTVisitor<WebCLExpressionTransformation>

{
public:

    explicit WebCLExpressionTransformation(
        clang::CompilerInstance &instance, WebCLTransformations &transformations,
        clang::Expr *expr);
    virtual ~WebCLExpressionTransformation();

    /// \see WebCLRecursiveTransformation::getAsText
    virtual bool getAsText(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter,
        std::string &text, clang::SourceRange &range);

    /// \see clang::RecursiveASTVisitor::VisitExpr
    bool VisitExpr(clang::Expr *expr);

protected:

    WebCLTransformations &transformations_;
    clang::Expr *expr_;
    bool status_;
    std::string text_;

    WebCLTransformerConfiguration *cfg_;
    clang::Rewriter *rewriter_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMATION
