#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "WebCLConfiguration.hpp"
#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"
#include "WebCLRewriter.hpp"

#include <map>
#include <set>
#include <utility>
#include <sstream>

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
  
    void initializeAddressSpaceNull(clang::FunctionDecl *kernel, AddressSpaceLimits &limits);

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
  
    void moveToModulePrologue(clang::NamedDecl *decl);
  
    // applies already added transformation to source
    void flushQueuedTransformations();
  
    // TODO: remove methods which requires storing any model state.
    //       only allowed state here is how to represent multiple changes.
  
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
  
    WebCLRewriter wclRewriter_;
  
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
  
    std::set<std::string> usedTypeNames_;
  
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


    /// \brief Inserts module prologue to start of module.
    bool rewritePrologue();
    /// \brief Inserts kernel prologue to start of module.
    bool rewriteKernelPrologue(const clang::FunctionDecl *kernel);

    /// \brief Writes variable declaration as a struct field declaration to stream.
    void emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl);
    /// \brief Writes variable declaration as a struct field declaration to stream.
    void emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl,
                             const std::string &name);

    /// \brief Returns macro call for given addr, type and limits to check.
    /// e.g. _WCL_ADDR_global_1(__global int *, addr, _wcl_allocs->gl.array_min, _wcl_allocs->gl.array_max, _wcl_allocs->gn)
    std::string getClampMacroCall(std::string addr, std::string type, AddressSpaceLimits &limits);
  
    /// \brief Writes bytestream generated from general.cl to stream.
    void emitGeneralCode(std::ostream &out);
  
    /// \brief Goes through set of all generated _WCL_ADDR_* calls and write corresponding #define's to stream
    void emitLimitMacros(std::ostream &out);
  
    /// \brief Returns macro define for clamping address to certain limits for each address space and limit count
    /// e.g. #define _WCL_ADDR_local_2(type, addr, min1, max1, min2, max2, asnull) (<macro code>)
    std::string getWclAddrCheckMacroDefinition(unsigned addrSpaceNum, unsigned limitCount);

    void emitPrologue(std::ostream &out);

    void emitVariableInitialization(
        std::ostream &out, const clang::VarDecl *decl);

    void emitTypeNullInitialization(
        std::ostream &out, clang::QualType qualType);

    WebCLConfiguration cfg_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMER
