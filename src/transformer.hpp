#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "reporter.hpp"

#include "clang/AST/Type.h"
#include "llvm/ADT/APInt.h"

#include <map>
#include <set>
#include <utility>

namespace clang {
    class ArraySubscriptExpr;
    class Decl;
    class DeclStmt;
    class Expr;
    class ParmVarDecl;
    class Rewriter; 
    class VarDecl;
}

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
};

/// \brief Relocate a variable into a corresponding address space
/// record.
class WebCLVariableRelocation : public WebCLTransformation
{
public:

    WebCLVariableRelocation(
        clang::CompilerInstance &instance,
        clang::DeclStmt *stmt, clang::VarDecl *decl);
    virtual ~WebCLVariableRelocation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

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
class WebCLArraySubscriptTransformation : public WebCLTransformation
{
public:

    WebCLArraySubscriptTransformation(
        clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr);
    virtual ~WebCLArraySubscriptTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

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

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

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

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    std::string bound_;
};

/// \brief Adds pointer limit check.
class WebCLPointerDereferenceTransformation : public WebCLTransformation
{
public:

    WebCLPointerDereferenceTransformation(
        clang::CompilerInstance &instance, clang::Expr *expr);
    virtual ~WebCLPointerDereferenceTransformation();

    /// \see WebCLTransformation::rewrite
    virtual bool rewrite(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

protected:

    clang::Expr *expr_;
};

/// \brief A helper structure that configures strings that occur
/// repeatedly in generated code.
class WebCLTransformerConfiguration
{
public:

    WebCLTransformerConfiguration();
    ~WebCLTransformerConfiguration();

    const std::string getNameOfAddressSpace(clang::QualType type) const;
    const std::string getNameOfType(clang::QualType type) const;
    const std::string getNameOfPointerChecker(clang::QualType type) const;
    const std::string getNameOfIndexChecker(clang::QualType type) const;
    const std::string getNameOfIndexChecker() const;
    const std::string getNameOfSizeParameter(clang::ParmVarDecl *decl) const;
    const std::string getIndentation(unsigned int levels) const;

    const std::string prefix_;
    const std::string pointerSuffix_;
    const std::string indexSuffix_;

    const std::string indentation_;
    const std::string sizeParameterType_;

    const std::string privateAddressSpace_;
    const std::string privateRecordType_;
    const std::string privateField_;
    const std::string privateRecordName;

    const std::string localAddressSpace_;
    const std::string localRecordType_;
    const std::string localField_;
    const std::string localRecordName_;

    const std::string constantAddressSpace_;
    const std::string constantRecordType_;
    const std::string constantField_;
    const std::string constantRecordName_;

    const std::string globalAddressSpace_;
    const std::string globalRecordType_;
    const std::string globalField_;
    const std::string globalRecordName_;

    const std::string addressSpaceRecordType_;
    const std::string addressSpaceRecordName_;
};

/// \brief Performs AST node transformations.
class WebCLTransformer : public WebCLReporter
{
public:

    explicit WebCLTransformer(clang::CompilerInstance &instance);
    ~WebCLTransformer();

    /// Applies all AST transformations.
    bool rewrite(clang::Rewriter &rewriter);

    /// Inform about a variable that is accessed indirectly with a
    /// pointer. The variable declaration will be moved to a
    /// corresponding address space record so that indirect accesses
    /// can be checked.
    void addRelocatedVariable(clang::DeclStmt *stmt, clang::VarDecl *decl);

    /// Modify function parameter declarations:
    /// function(a, b) -> function(record, a, b)
    void addRecordParameter(clang::FunctionDecl *decl);

    /// Modify kernel parameter declarations:
    /// kernel(a, array, b) -> kernel(a, array, array_size, b)
    void addSizeParameter(clang::ParmVarDecl *decl);

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

    typedef std::map<const clang::Decl*, WebCLTransformation*> DeclTransformations;
    typedef std::map<const clang::Expr*, WebCLTransformation*> ExprTransformations;
    // address space, name
    typedef std::pair<const std::string, const std::string> CheckedType;
    typedef std::set<CheckedType> CheckedTypes;
    typedef std::set<const clang::VarDecl*> VariableDeclarations;

    bool rewritePrologue(clang::Rewriter &rewriter);
    bool rewriteTransformations(clang::Rewriter &rewriter);

    clang::ParmVarDecl *getDeclarationOfArray(clang::ArraySubscriptExpr *expr);

    void addCheckedType(CheckedTypes &types, clang::QualType type);
    void addTransformation(const clang::Decl *decl, WebCLTransformation *transformation);
    void addTransformation(const clang::Expr *expr, WebCLTransformation *transformation);
    void addRelocatedVariable(clang::VarDecl *decl);

    void emitVariable(std::ostream& out, const clang::VarDecl *decl);
    void emitAddressSpaceRecord(std::ostream &out, const VariableDeclarations &variables,
                                const std::string &name);
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

    DeclTransformations declTransformations_;
    ExprTransformations exprTransformations_;
    CheckedTypes checkedPointerTypes_;
    CheckedTypes checkedIndexTypes_;
    VariableDeclarations relocatedGlobals_;
    VariableDeclarations relocatedLocals_;
    VariableDeclarations relocatedConstants_;
    VariableDeclarations relocatedPrivates_;

    WebCLTransformerConfiguration cfg_;
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
