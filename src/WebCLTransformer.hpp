#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"
#include "WebCLTransformations.hpp"
#include "WebCLTransformerConfiguration.hpp"

#include <map>
#include <set>
#include <utility>

namespace clang {
    class ArraySubscriptExpr;
    class CallExpr;
    class Decl;
    class DeclStmt;
    class Expr;
    class ParmVarDecl;
    class Rewriter; 
    class VarDecl;
}

namespace llvm {
    class APInt;
}

class WebCLTransformation;

/// \brief Performs AST node transformations.
class WebCLTransformer : public WebCLReporter
                       , public WebCLHelper
{
public:

    WebCLTransformer(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    ~WebCLTransformer();

    /// Applies all AST transformations.
    bool rewrite();

    // Inform about a kernels so that kernel prologues can be emitted.
    void addKernel(clang::FunctionDecl *decl);

    /// Inform about a variable that is accessed indirectly with a
    /// pointer. The variable declaration will be moved to a
    /// corresponding address space record so that indirect accesses
    /// can be checked.
    void addRelocatedVariable(clang::DeclStmt *stmt, clang::VarDecl *decl);

    /// Modify function parameter declarations:
    /// function(a, b) -> function(record, a, b)
    void addRecordParameter(clang::FunctionDecl *decl);

    /// Modify arguments passed to a function:
    /// call(a, b) -> call(record, a, b)
    void addRecordArgument(clang::CallExpr *expr);

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

    // address space, name
    typedef std::pair<const std::string, const std::string> CheckedType;
    typedef std::set<CheckedType> CheckedTypes;
    typedef std::set<const clang::VarDecl*> VariableDeclarations;
    typedef std::set<const clang::FunctionDecl*> Kernels;

    bool rewritePrologue();
    bool rewriteKernelPrologue(const clang::FunctionDecl *kernel);
    bool rewriteTransformations();

    clang::ParmVarDecl *getDeclarationOfArray(clang::ArraySubscriptExpr *expr);

    void addCheckedType(CheckedTypes &types, clang::QualType type);

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

    void emitTypeInitialization(
        std::ostream &out, clang::QualType qualType);
    void emitVariableInitialization(
        std::ostream &out, const clang::VarDecl *decl);
    void emitRecordInitialization(
        std::ostream &out, const std::string &type, const std::string &name,
        VariableDeclarations &relocations);
    void emitKernelPrologue(std::ostream &out);

    CheckedTypes checkedPointerTypes_;
    CheckedTypes checkedIndexTypes_;
    VariableDeclarations relocatedGlobals_;
    VariableDeclarations relocatedLocals_;
    VariableDeclarations relocatedConstants_;
    VariableDeclarations relocatedPrivates_;
    Kernels kernels_;

    WebCLTransformerConfiguration cfg_;
    WebCLTransformations transformations_;
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
