#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"
#include "WebCLTransformations.hpp"
#include "WebCLTransformerConfiguration.hpp"

// Replacement class and applyAllReplacements function
#include "clang/Tooling/Refactoring.h"

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
    class TypedefDecl;
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

    void createAddressSpaceTypedef(AddressSpaceInfo &as, const std::string &name, const std::string &alignment);
    void createPrivateAddressSpaceTypedef(AddressSpaceInfo &as);
    void createLocalAddressSpaceTypedef(AddressSpaceInfo &as);
    void createConstantAddressSpaceTypedef(AddressSpaceInfo &as);

    void replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl);
    void removeRelocated(clang::VarDecl *decl);

    void createAddressSpaceLimitsTypedef(
        AddressSpaceLimits &limits, const std::string &name);
    void createGlobalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    void createConstantAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    void createLocalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    void createAddressSpaceLimitsField(const std::string &type, const std::string &name);
    void createAddressSpaceNullField(
        const std::string &name, unsigned addressSpace);
    void createProgramAllocationsTypedef(
        AddressSpaceLimits &globalLimits, AddressSpaceLimits &constantLimits,
        AddressSpaceLimits &localLimits, AddressSpaceInfo &privateAs);
  
    void createConstantAddressSpaceAllocation(AddressSpaceInfo &as);
    void createLocalAddressSpaceAllocation(clang::FunctionDecl *kernelFunc);
    void createAddressSpaceLimitsInitializer(
        std::ostream &out, clang::FunctionDecl *kernel, AddressSpaceLimits &limits);
    void createAddressSpaceInitializer(std::ostream& out, AddressSpaceInfo &info);

    void createProgramAllocationsAllocation(
        clang::FunctionDecl *kernelFunc, AddressSpaceLimits &globalLimits,
        AddressSpaceLimits &constantLimits, AddressSpaceLimits &localLimits,
        AddressSpaceInfo &privateAs);

    void createAddressSpaceNullAllocation(std::ostream &out, unsigned addressSpace);
    void createConstantAddressSpaceNullAllocation();
    void createLocalAddressSpaceNullAllocation(clang::FunctionDecl *kernel);
    void createGlobalAddressSpaceNullAllocation(clang::FunctionDecl *kernel);

    /// \brief Zero a single local memory range.
    void createLocalRangeZeroing(std::ostream &out, const std::string &arguments);
    /// \brief Zero all local memory ranges.
    void createLocalAreaZeroing(clang::FunctionDecl *kernelFunc,
                                AddressSpaceLimits &localLimits);
  
    void addMemoryAccessCheck(clang::Expr *access, AddressSpaceLimits &limits);

    void addRelocationInitializerFromFunctionArg(clang::ParmVarDecl *parmDecl);
    void addRelocationInitializer(clang::VarDecl *decl);
  
    void addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                               unsigned minWidthInBits);
    void addAddressSpaceNull(std::ostream &out, unsigned addressSpace);
  
    void moveToModulePrologue(clang::TypedefDecl *decl);
  
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
  
    // this is stream for things that needs to be absolutely at first in program even before typedefs
    std::stringstream preModulePrologue_;
    // stream for other initializations at start of module like typedefs and address space structures
    std::stringstream modulePrologue_;
  
    // set to keep track that we are not doing paramRelocationInitializations multiple times
    std::set< clang::ParmVarDecl* > parameterRelocationInitializations_;
    std::set< clang::VarDecl* > privateRelocationInitializations_;
  
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
    void createAddressSpaceNullInitializer(std::ostream &out, unsigned addressSpace);

    // returns address space limits info as structure
    std::string addressSpaceLimitsAsStruct(AddressSpaceLimits &asLimits);

    /// returns initializer for limits e.g.
    /// {
    ///     &((&wcl_constan_allocs)[0]), &((&wcl_constan_allocs)[1]),
    ///     NULL, NULL,
    ///     &const_as_arg[0], &const_as_arg[wcl_const_as_arg_size]
    /// }
    std::string addressSpaceLimitsInitializer(
      clang::FunctionDecl *kernelFunc, AddressSpaceLimits &as);
    void createAddressSpaceLimitsNullInitializer(
        std::ostream &out, unsigned addressSpace);

    /// \brief Looks char by char forward until the position where next requested character is found
    clang::SourceLocation findLocForNext(clang::SourceLocation startLoc, char charToFind);
  
    // NOTE: we should refactor all functionality of
    //       insertText, removeText, ... etc. to separate class
    //       since this is basically extended functionality to rewriter
    //       interface (our own low level modification handling).
    typedef std::pair< int, int >                  ModifiedRange;
    typedef std::map< ModifiedRange, std::string > RangeModifications;
    RangeModifications modifiedRanges_;
    // filtered ranges, which has only the top level modifications and does not
    // include nested ones (top level should already contain nested changes as string)
    typedef std::set<ModifiedRange>             RangeModificationsFilter;
    RangeModificationsFilter filteredModifiedRanges_;
    // if we should regenerate the filtered ranges data
    bool isFilteredRangesDirty_;
  
    // transformation methods, which does not do transformation yet, but waits
    // if there is other transformation done for the same location and merge them first
    void removeText(clang::SourceRange range);
    void replaceText(clang::SourceRange range, std::string text);
    RangeModificationsFilter& filteredModifiedRanges();
  
    /// \brief if for asked source range has been added transformations, return transformed result
    /// e.g. if location is [20, 50] we would find all transformations inside is
    //       [20,22], [25,30], [33,45] and before that all transformations inside those ranges
    //       [34,37] etc..
    //       I have pretty much no good idea how to handle this good enough...
    //       TODO: print source locations and find out...
    std::string getTransformedText(clang::SourceRange range);

    bool checkIdentifiers();
    bool rewritePrologue();
    bool rewriteKernelPrologue(const clang::FunctionDecl *kernel);
    bool rewriteTransformations();

    clang::ParmVarDecl *getDeclarationOfArray(clang::ArraySubscriptExpr *expr);

    void addCheckedType(CheckedTypes &types, clang::QualType type);

    void addRelocatedVariable(clang::VarDecl *decl);

    void emitVariable(std::ostream &out, const clang::VarDecl *decl);
    void emitVariable(std::ostream &out, const clang::VarDecl *decl,
                      const std::string &name);

    void emitPrologueRecords(std::ostream &out);

    void emitGeneralCode(std::ostream &out);
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
