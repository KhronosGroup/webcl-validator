#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"
#include "WebCLTransformations.hpp"
#include "WebCLTransformerConfiguration.hpp"

#include <map>
#include <set>
#include <utility>
#include <sstream>

#ifdef DEBUG
#include <iostream>

#endif

namespace clang {
    class ArraySubscriptExpr;
    class CallExpr;
    class Decl;
    class DeclStmt;
    class Expr;
    class ParmVarDecl;
    class Rewriter; 
    class VarDecl;
    class DeclRefExpr;
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

    void createPrivateAddressSpaceTypedef(AddressSpaceInfo &as);
    void createLocalAddressSpaceTypedef(AddressSpaceInfo &as);
    void createConstantAddressSpaceTypedef(AddressSpaceInfo &as);
    void createConstantAddressSpaceAllocation(AddressSpaceInfo &as);
    void replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl);
    void createGlobalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    void createConstantAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    void createLocalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);

    void createProgramAllocationsTypedef(AddressSpaceLimits &globalLimits,
                                         AddressSpaceLimits &constantLimits,
                                         AddressSpaceLimits &localLimits,
                                         AddressSpaceInfo &privateAs);
  
    void createLocalAddressSpaceAllocation(clang::FunctionDecl *kernelFunc);
  
    void createProgramAllocationsAllocation(clang::FunctionDecl *kernelFunc,
                                            AddressSpaceLimits &globalLimits,
                                            AddressSpaceLimits &constantLimits,
                                            AddressSpaceLimits &localLimits,
                                            AddressSpaceInfo &privateAs);
  
    void createLocalAreaZeroing(clang::FunctionDecl *kernelFunc,
                                AddressSpaceLimits &localLimits);
  
  
    void addMemoryAccessCheck(clang::Expr *access, AddressSpaceLimits &limits);

    void addRelocationInitializer(clang::ParmVarDecl *parmDecl);
  
    void addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                               unsigned minWidthInBits);
  
    void moveToModulePrologue(clang::Decl *decl);
  
    // applies already added transformation to source
    void flushQueuedTransformations();
  
    // TODO: remove methods which requires storing any model state.
    //       only allowed state here is how to represent multiple changes.
  
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
    /// call(a, b) -> call(wcl_allocs, a, b)
    void addRecordArgument(clang::CallExpr *expr);

    /// Modify kernel parameter declarations:
    /// kernel(a, array, b) -> kernel(a, array, array_size, b)
    void addSizeParameter(clang::ParmVarDecl *decl);


private:
  
    // Set of all different type clamp macro calls in program.
    // One pair for each address space and limit count <address space num, limit count>
    typedef std::pair< unsigned, unsigned > ClampMacroKey;
    typedef std::map< const clang::FunctionDecl*, std::stringstream* > FunctionPrologueMap;
    typedef std::set<ClampMacroKey> RequiredMacroSet;
    RequiredMacroSet usedClampMacros_;
    FunctionPrologueMap kernelPrologues_;
    FunctionPrologueMap functionPrologues_;
    std::stringstream modulePrologue_;
  
    // set to keep track that we are not doing paramRelocationInitializations multiple times
    std::set< clang::ParmVarDecl* > parameterRelocationInitializations_;
  
    // \brief kernel prologue comes before function prologue... kernel might have also functionPrologue
    std::stringstream& kernelPrologue(const clang::FunctionDecl *kernel) {
      // TODO: cleanup memory...
      if (kernelPrologues_.count(kernel) == 0) {
        kernelPrologues_[kernel] = new std::stringstream();
      }
      return *kernelPrologues_[kernel];
    }

    std::stringstream& functionPrologue(const clang::FunctionDecl *func) {
      // TODO: cleanup memory...
      if (functionPrologues_.count(func) == 0) {
        functionPrologues_[func] = new std::stringstream();
      }
      return *functionPrologues_[func];
    }
  
    std::string getClampMacroCall(std::string addr, std::string type, AddressSpaceLimits &limits);
    std::string getWclAddrCheckMacroDefinition(unsigned addrSpaceNum, unsigned limitCount);
  
    // address space, name
    typedef std::pair<const std::string, const std::string> CheckedType;
    typedef std::set<CheckedType> CheckedTypes;
    typedef std::set<const clang::VarDecl*> VariableDeclarations;
    typedef std::set<const clang::FunctionDecl*> Kernels;

    /// returns address space structure e.g. { float *a; uint b; }
    /// also drops address space qualifiers from original declarations
    std::string addressSpaceInfoAsStruct(AddressSpaceInfo &as);
  
    /// returns initializer for as e.g. { NULL, 5 }
    std::string addressSpaceInitializer(AddressSpaceInfo &as);

    // returns address space limits info as structure
    std::string addressSpaceLimitsAsStruct(AddressSpaceLimits &asLimits);

    /// returns initializer for limits e.g.
    /// {
    ///     &((&wcl_constan_allocs)[0]), &((&wcl_constan_allocs)[1]),
    ///     NULL, NULL,
    ///     &const_as_arg[0], &const_as_arg[wcl_const_as_arg_size]
    /// }
    std::string addressSpaceLimitsInitializer(
      clang::FunctionDecl *kernelFunc,AddressSpaceLimits &as);

    // use vectors to preserve order, when doing macro additions, we need to do inner first
    typedef std::pair< clang::SourceLocation, clang::SourceLocation > LocationPair;
    typedef std::pair< LocationPair, std::string >                    ReplacementPair;
    typedef std::pair< clang::SourceLocation, std::string >           InsertionPair;
    typedef std::vector< LocationPair >                               RemovalContainer;
    typedef std::vector< ReplacementPair >                            ReplacementContainer;
    typedef std::vector< InsertionPair >                              InsertionContainer;
  
    // transformation methods, which does not do transformation yet, but waits
    // if there is other transformation done for the same location and merge them first
    void insertText(clang::SourceLocation loc, std::string text) {
      inserts_.push_back(
          InsertionPair(loc, text));
    };
    void removeText(clang::SourceRange range) {
      removals_.push_back(
          LocationPair(range.getBegin(), range.getEnd()));
    };
    void replaceText(clang::SourceRange range, std::string text) {
      replacements_.push_back(
          ReplacementPair( LocationPair(range.getBegin(), range.getEnd()), text));
    };

    // organize transformations so that remove / replace is done first and insert after
    RemovalContainer     removals_;
    ReplacementContainer replacements_;
    InsertionContainer   inserts_;
  
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
