#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

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

class WebCLKernelHandler; // used by wrapFunctionCall

/// Performs transformations that need to be done by the memory access
/// validation algorithm. Doesn't decide what transformations should
/// be done, but only offers services to cache and emit basic
/// modifications. It's up to the various transformation passes to
/// call the services in correct order.
///
/// \see WebCLConsumer
class WebCLTransformer : public WebCLReporter
{
public:

    WebCLTransformer(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    ~WebCLTransformer();
  
    /// Applies all AST transformations. All cached modifications are
    /// flushed to the rewriter.
    ///
    /// \return Whether errors were detected. Transformed code
    /// shouldn't be output if there were errors.
    bool rewrite();

    /// Create address space structure. The structure contains all
    /// relocated variables of the given address space as fields.
    void createAddressSpaceTypedef(
        AddressSpaceInfo &as, const std::string &name, const std::string &alignment);
    /// Create private address space structure.
    /// \see createAddressSpaceTypedef
    void createPrivateAddressSpaceTypedef(AddressSpaceInfo &as);
    /// Create local address space structure.
    /// \see createAddressSpaceTypedef
    void createLocalAddressSpaceTypedef(AddressSpaceInfo &as);
    /// Create constant address space structure.
    /// \see createAddressSpaceTypedef
    void createConstantAddressSpaceTypedef(AddressSpaceInfo &as);

    /// Replace a reference to a relocated variable with a reference
    /// to the corresponding address space structure field.
    void replaceWithRelocated(clang::DeclRefExpr *use, clang::VarDecl *decl);
    /// Remove a variable declaration if it's not needed anymore. For
    /// example, the variable could have been relocated to an address
    /// space structure and initializations don't depend on it.
    void removeRelocated(clang::VarDecl *decl);

    /// Create a limits structure that contains the begin and end
    /// addresses of each memory object (in the given address space)
    /// that was passed to a kernel. The limits structure contains
    /// also the begin and end addresses of corresponding static
    /// address space structure.
    void createAddressSpaceLimitsTypedef(
        AddressSpaceLimits &limits, const std::string &name);
    /// Create limits structure for globals. It contains only dynamic
    /// limits.
    void createGlobalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    /// Create limits structure for constants. It may contain both
    /// dynamic and static limits.
    void createConstantAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);
    /// Create limits structure for locals. It may contain both
    /// dynamic and static limits.
    void createLocalAddressSpaceLimitsTypedef(AddressSpaceLimits &asLimits);

    // Amends the main allocation structure with a field that contains
    // limits of all disjoint memory areas. This is done for address
    // spaces that may contain dynamic limits, i.e. the private
    // address space is excluded.
    void createAddressSpaceLimitsField(const std::string &type, const std::string &name);
    /// Amends the main allocation structure with a field that
    /// contains enough room for the largest pointer access in the
    /// given address space.
    void createAddressSpaceNullField(
        const std::string &name, unsigned addressSpace);
    /// Creates the main allocation structure type that holds
    /// information about all disjoint memory areas as well as
    /// fallback areas (address space specific null pointers).
    void createProgramAllocationsTypedef(
        AddressSpaceLimits &globalLimits, AddressSpaceLimits &constantLimits,
        AddressSpaceLimits &localLimits, AddressSpaceInfo &privateAs);

    /// Creates the instance that holds all relocated constant
    /// variables.
    void createConstantAddressSpaceAllocation(AddressSpaceInfo &as);
    /// Creates an instance that holds all relocated local variables.
    ///
    /// FUTURE: Each kernel could have a different kind of allocation
    ///         structure.
    void createLocalAddressSpaceAllocation(clang::FunctionDecl *kernelFunc);
  
    /// Creates an initializer for the address space specific limits
    /// of the main allocation structure. Called for address spaces
    /// that may contain dynamic limits, i.e. the private address
    /// space is excluded.
    void createAddressSpaceLimitsInitializer(
        std::ostream &out, clang::FunctionDecl *kernel, AddressSpaceLimits &limits);
    /// Creates an allocation with initialization for the instance of
    /// the main allocation structure.
    void createProgramAllocationsAllocation(
        clang::FunctionDecl *kernelFunc, AddressSpaceLimits &globalLimits,
        AddressSpaceLimits &constantLimits, AddressSpaceLimits &localLimits,
        AddressSpaceInfo &privateAs);

    /// Creates and initializes a variable declaration that holds
    /// enough space for the largest memory access in the given
    /// address space.
    ///
    /// Used for constants and locals, because those address spaces may
    /// contain both static and dynamic limits.
    void createAddressSpaceNullAllocation(std::ostream &out, unsigned addressSpace);
    /// Creates and initializes a fallback area variable for constant
    /// memory accesses.
    void createConstantAddressSpaceNullAllocation();
    /// Creates a fallback area variable for local memory accesses.
    void createLocalAddressSpaceNullAllocation(clang::FunctionDecl *kernel);
    /// Initializes a fallback area for the given address space. Emits
    /// code to bail out from the kernel if existing memory areas
    /// aren't big enough to hold the largest memory access in that
    /// address space.
    ///
    /// Used for globals and privates:
    /// - It's impossible to allocate a separate fallback area
    ///   variable for globals (there are only dynamic limits).
    /// - It's shouldn't be necessary to allocate a separate fallback
    ///   area variable for privates (there are only static limits).
    void initializeAddressSpaceNull(clang::FunctionDecl *kernel, AddressSpaceLimits &limits);

    /// \brief Zero a single local memory range.
    void createLocalRangeZeroing(std::ostream &out, const std::string &arguments);
    /// \brief Zero all local memory ranges.
    void createLocalAreaZeroing(clang::FunctionDecl *kernelFunc,
                                AddressSpaceLimits &localLimits);

    /// Replaces memory access with a checked access. If the access
    /// doesn't fall within limits of any given disjoint memory areas,
    /// the fallback area (null pointer) is accessed instead.
    void addMemoryAccessCheck(clang::Expr *access, unsigned size, AddressSpaceLimits &limits);

    /// Adds an initialization row to start of function if relocated
    /// variable was a function argument.
    ///
    /// void foo(int arg)
    /// {
    ///     bar(&arg);
    /// }
    /// ->
    /// void foo(_WclProgramAllocations *_wcl_allocs, int arg)
    /// {
    ///     _wcl_allocs->pa._wcl_arg = arg;
    ///     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    ///     bar(&_wcl_allocs->pa._wcl_arg);
    /// }
    void addRelocationInitializerFromFunctionArg(clang::ParmVarDecl *parmDecl);

    /// Adds initialization for relocated private variable after
    /// original variable initialization.
    ///
    /// int foo = 1;
    /// ->
    /// int foo = 1;
    /// _wcl_allocs->pa._wcl_foo = foo;
    ///
    /// The driver compiler should optimize unused variables (original
    /// foo) away afterwards.
    void addRelocationInitializer(clang::VarDecl *decl);

    /// Defines a macro that tells what is the largest memory access
    /// in the given address space. The macro defines this size as
    /// bytes, not as bits.
    void addMinimumRequiredContinuousAreaLimit(unsigned addressSpace,
                                               unsigned minWidthInBits);

    /// Moves declarations to module prologue.
    ///
    /// Expects that declarations are in one of following formats:
    /// typedef <type> <name>; // any typedef
    /// struct <name> { ... }; // structure definition
    /// struct <name>;         // forward declaration of a structure
    ///
    /// Normalization passes have already taken care that unsupported
    /// forms aren't present:
    /// struct { ... } var; // nameles declaration or instantiation with declaration
    void moveToModulePrologue(clang::NamedDecl *decl);

    // Applies already added transformation to the source.
    void flushQueuedTransformations();
  
    /// Inform about a variable that is accessed indirectly with a
    /// pointer. The variable declaration will be moved to a
    /// corresponding address space structure so that indirect
    /// accesses can be checked.
    void addRelocatedVariable(clang::DeclStmt *stmt, clang::VarDecl *decl);

    /// Modify function parameter declarations:
    /// function(a, b) -> function(_wcl_allocs, a, b)
    void addRecordParameter(clang::FunctionDecl *decl);

    /// Modify arguments passed to a function:
    /// call(a, b) -> call(_wcl_allocs, a, b)
    void addRecordArgument(clang::CallExpr *expr);

    /// Modify kernel parameter declarations:
    /// kernel(a, array, b) -> kernel(a, array, array_size, b)
    void addSizeParameter(clang::ParmVarDecl *decl);

    /// Modify a function call to call a function of another name
    void changeFunctionCallee(clang::CallExpr *expr, std::string newName);

    /// Modify a function call to a generated replacement if applicaple. The
    /// replacement function is generated if required. if wrapping was
    /// performed the function returns true. otherwise (ie. an unknown builtin or 
    /// functions without dangerous arguments like image2d_t or sampler_t)
    /// false is returned. 
    /// Functions such as vloadn and vstoren generate a wrapping function that
    /// performs the check of arguments and do nothing if the limits are
    /// exceeded.
    /// Functions with sampler_t argument such as read_imagef and read_imagei or
    /// user-defined functions accepting sampler_t are only checked for their
    /// sampler_t argument. For those functions no wrapper function is
    /// generated: instead, the second argument is deconstructed from the value
    /// of its integer constant expression and then reconstructed by generating
    /// an expression that builds the desired value by using the related macro
    /// definitions and the bitwise or operator.
    bool wrapFunctionCall(std::string wrapperName, clang::CallExpr *expr, WebCLKernelHandler &kernelHandler);

    /// Same, but for variable declarations. For variable declarations no helper functions
    /// are currently generated, so it doesn't use a name argument for that.
    bool wrapVariableDeclaration(clang::VarDecl *varDecl, WebCLKernelHandler &kernelHandler);

    /// Used for implementing function wrappers
    class FunctionCallWrapper;

    enum CheckKind {
	CHECK_CLAMP,
	CHECK_CHECK
    };
    /// \return A macro call that forces the given address to point to
    /// a safe memory area.
    ///
    /// e.g. _WCL_ADDR_global_1(__global int *, addr, _wcl_allocs->gl.array_min, _wcl_allocs->gl.array_max, _wcl_allocs->gn)
    std::string getCheckFunctionCall(CheckKind kind, std::string addr, std::string type, unsigned size, AddressSpaceLimits &limits);

private:

    /// Caches source code replacements.
    WebCLRewriter wclRewriter_;
  
    struct ClampFunctionKey {
        ClampFunctionKey(unsigned = 0, unsigned = 0, std::string = "");

        bool operator<(const ClampFunctionKey& other) const;

        unsigned    aSpaceNum;
        unsigned    limitCount;
        std::string type;
    };

    /// Set of all different clamp macro call types in the program.
    typedef std::set<ClampFunctionKey> RequiredFunctionSet;
    RequiredFunctionSet usedClampFunctions_;

    /// Stream for inserting code at the beginning of each kernel or
    /// helper function.
    typedef std::map< const clang::FunctionDecl*, std::stringstream* > FunctionPrologueMap;
    /// Contains only kernels.
    FunctionPrologueMap kernelPrologues_;
    /// Contains kernels and helper functions.
    FunctionPrologueMap functionPrologues_;
    /// \return Chosen prologue stream for the given function.
    ///
    /// A kernel might have both function and kernel prologue
    /// streams. Kernel prologue comes before function prologue.
    std::stringstream& functionPrologue(FunctionPrologueMap &prologues, const clang::FunctionDecl *func);

    /// Stream for code that needs to be located at the beginning of
    /// the transformed program even before typedefs.
    std::stringstream preModulePrologue_;
    /// Stream for code at the start of the module like typedefs and
    /// address space structures.
    std::stringstream modulePrologue_;
    /// Stream for code after limit functions, eg. for builtin functions/macros
    std::stringstream afterLimitFunctions_;
  
    /// Set to ensure that we aren't initializing relocated parameters
    /// multiple times.
    std::set< clang::ParmVarDecl* > parameterRelocationInitializations_;
    /// Set to ensure that we don't have multiple type declarations
    /// with the same name.
    std::set<std::string> usedTypeNames_;

    /// \return Address space structure, e.g. { float *a; uint b; }.
    ///
    /// Also drops address space qualifiers from original variable
    /// declarations.
    std::string addressSpaceInfoAsStruct(AddressSpaceInfo &as);
    /// \return Initializer for address space structure, e.g. { NULL, 5 }.
    std::string addressSpaceInitializer(AddressSpaceInfo &as);

    /// \return Address space limits structure. Contains begin and end
    /// pointer fields for each disjoint memory area in the address
    /// space.
    std::string addressSpaceLimitsAsStruct(AddressSpaceLimits &asLimits);
    /// \return Initializer for address space limits structure.
    std::string addressSpaceLimitsInitializer(
      clang::FunctionDecl *kernelFunc, AddressSpaceLimits &as);
    /// \return Initializer for address space fallback area (null
    /// pointer).
    void createAddressSpaceLimitsNullInitializer(
        std::ostream &out, unsigned addressSpace);

    /// \brief Inserts module prologue to start of module.
    bool rewritePrologue();
    /// \brief Inserts kernel prologue to start of kernel body.
    bool rewriteKernelPrologue(const clang::FunctionDecl *kernel);

    /// Adds generic wrappers from a sequence; reduces code duplication
    void addGenericWrappers(const StringList& list,
        unsigned numArgs,
        unsigned ptrArgIndex);

    /// Writes a variable declaration to a stream in the form it
    /// should be declared inside an address space structure.
    ///
    /// Drops address space qualifiers from declaration and gets
    /// relocated variable name:
    /// __constant int foo[2] = { 1 }
    /// ->
    /// int _wcl_foo[2]
    void emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl);

    /// Writes a variable declaration to a stream in the form it
    /// should be declared inside an address space structure. A unique
    /// name, which doesn't conflict with other address space
    /// structure field names, should be given for the variable.
    void emitVarDeclToStruct(std::ostream &out, const clang::VarDecl *decl,
                             const std::string &name);

    /// \return A full expression (incorporating a macro call from
    /// getClampFunctionCall) call that forces the given address to point to a safe
    /// memory area.
    std::string getClampFunctionExpression(clang::Expr *access, unsigned size, AddressSpaceLimits &limits);

    /// \brief Writes bytestream generated from general.cl to stream.
    void emitGeneralCode(std::ostream &out);
  
    /// \brief Goes through the set of all generated _wcl_addr_* calls
    /// and writes corresponding _wcl_addr_* functions to the stream.
    void emitLimitFunctions(std::ostream &out);
  
    /// \return Macro definitions for checking and clamping addresses in given
    /// address space with given limit count.
    ///
    /// e.g.
    /// #define _WCL_ADDR_CHECK_local_2(type, addr, min1, max1, min2, max2) (<macro code>)
    /// #define _WCL_ADDR_CLAMP_local_2(type, addr, min1, max1, min2, max2, asnull) (<macro code>)
    /// the clamping macro is defined in terms of the checking macro
    std::string getWclAddrCheckFunctionDefinition(ClampFunctionKey clamp);

    /// Write generated code at the beginning of module.
    void emitPrologue(std::ostream &out);

    /// Writes initializer for a variable. Empty (zero) initializer is
    /// written if there is no original initializer, or if the
    /// original initializer isn't a compile time constant.
    void emitVariableInitialization(
        std::ostream &out, const clang::VarDecl *decl);

    /// \brief Emits empty (zero) initializer for a type.
    void emitTypeNullInitialization(
        std::ostream &out, clang::QualType qualType);

    /// Generates recurring names.
    WebCLConfiguration cfg_;

    typedef std::list<FunctionCallWrapper*> FunctionCallWrapperList;

    /// Map from a function name to a wrapping handler
    FunctionCallWrapperList functionWrappers_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMER
