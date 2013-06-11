#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "reporter.hpp"

#include "clang/AST/Type.h"
#include "llvm/ADT/APInt.h"

#include <map>
#include <set>
#include <utility>

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
    virtual ~WebCLTransformation() {};
    virtual bool rewrite(clang::Rewriter &rewriter) = 0;
};

/// \brief A base class for transformations that want to wrap an
/// expression inside a checker function.
class WebCLCheckerTransformation : public WebCLTransformation
                                 , public WebCLReporter
{
public:
    WebCLCheckerTransformation(
        clang::CompilerInstance &instance, const std::string &checker);

protected:

    const std::string checker_;
};


/// \brief Adds an index check to array subscription when the limits
/// are unknown at compile time.
class WebCLArraySubscriptTransformation : public WebCLCheckerTransformation
{
public:

    WebCLArraySubscriptTransformation(
        clang::CompilerInstance &instance, const std::string &checker,
        clang::ArraySubscriptExpr *expr);
    virtual ~WebCLArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(clang::Rewriter &rewriter);

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
        clang::CompilerInstance &instance, const std::string &checker,
        clang::ArraySubscriptExpr *expr, llvm::APInt &bound);
    virtual ~WebCLConstantArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(clang::Rewriter &rewriter);

protected:

    llvm::APInt bound_;
};

/// \brief Adds pointer limit check.
class WebCLPointerDereferenceTransformation : public WebCLCheckerTransformation
{
public:
    WebCLPointerDereferenceTransformation(
        clang::CompilerInstance &instance, const std::string &checker,
        clang::Expr *expr);
    virtual ~WebCLPointerDereferenceTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(clang::Rewriter &rewriter);

protected:

    clang::Expr *expr_;
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

    /// Modify array indexing:
    /// array[index] -> array[unknown_array_size_check(index)]
    void addArrayIndexCheck(clang::ArraySubscriptExpr *expr);

    /// Modify pointer dereferencing:
    // *pointer -> *unknown_pointer_check(pointer)
    void addPointerCheck(clang::Expr *expr);

private:

    typedef std::map<const clang::Expr*, WebCLTransformation*> ExprTransformations;
    // address space, name
    typedef std::pair<const std::string, const std::string> CheckedType;
    typedef std::set<CheckedType> CheckedTypes;

    bool rewritePrologue(clang::Rewriter &rewriter);
    bool rewriteTransformations(clang::Rewriter &rewriter);

    std::string getTypeAsString(clang::QualType type);
    std::string getAddressSpaceOfType(clang::QualType type);
    std::string getNameOfType(clang::QualType type);
    std::string getNameOfChecker(clang::QualType type);

    void addCheckedType(CheckedTypes &types, clang::QualType type);
    void addTransformation(const clang::Expr *expr, WebCLTransformation *transformation);

    void emitAddressSpaceOfType(std::ostream &out, clang::QualType type);
    void emitNameOfType(std::ostream &out, clang::QualType type);

    void emitAddressSpaceRecord(std::ostream &out, const std::string &name);
    void emitPrologueRecords(std::ostream &out);

    void emitLimitMacros(std::ostream &out);
    void emitPointerLimitMacros(std::ostream &out);
    void emitIndexLimitMacros(std::ostream &out);
    void emitPointerCheckerMacro(std::ostream &out);
    void emitIndexCheckerMacro(std::ostream &out);
    void emitPrologueMacros(std::ostream &out);

    void emitConstantIndexChecker(std::ostream &out);
    void emitChecker(std::ostream &out, const CheckedType &type, const std::string &kind);
    void emitCheckers(std::ostream &out, const CheckedTypes &types, const std::string &kind);
    void emitPrologueCheckers(std::ostream &out);

    void emitPrologue(std::ostream &out);

    ExprTransformations exprTransformations_;
    CheckedTypes checkedPointerTypes_;
    CheckedTypes checkedIndexTypes_;
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
