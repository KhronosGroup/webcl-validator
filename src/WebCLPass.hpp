#ifndef WEBCLVALIDATOR_WEBCLPASS
#define WEBCLVALIDATOR_WEBCLPASS

#include "WebCLHelper.hpp"

namespace clang {
    class ASTContext;
}

class WebCLAnalyser;
class WebCLTransformer;

/// Abstract base class for different passes of validation
/// stage. Derived classes perform transformations that depend on
/// results of the analysis pass.
class WebCLPass
{
public:

    WebCLPass(WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLPass();

    /// \brief Apply transformations.
    virtual void run(clang::ASTContext &context) = 0;

protected:

    WebCLAnalyser &analyser_;
    WebCLTransformer &transformer_;
};

/// Perform OpenCL C normalization transformations. Simplifies
/// implementation of memory access validation algorithm, but doesn't
/// itself do contribute to validation.
///
/// FUTURE: This should be refactored to be a separate pass before
///         validation.
///
/// \see WebCLMatcherAction
/// \see WebCLTool
class WebCLInputNormaliser : public WebCLPass
{
public:

    WebCLInputNormaliser(WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLInputNormaliser();

    /// Performs input normalization transformations.
    ///
    /// - Goes through all typedefs and structure definitions and
    ///   moves them to start of module.
    ///
    /// \see WebCLPass
    ///
    /// FUTURE: Move declaration part from each for loop to be before
    ///         for statement.
    virtual void run(clang::ASTContext &context);
};

/// Modifies helper function signatures and calls.
class WebCLHelperFunctionHandler : public WebCLPass
{
public:

    WebCLHelperFunctionHandler(WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLHelperFunctionHandler();
    
    /// - Adds allocation structure parameter to function signatures.
    /// - Adds allocation structure argument to corresponding calls.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);
};

/// Creates address space structures.
class WebCLAddressSpaceHandler : public WebCLPass
{
public:
  
    WebCLAddressSpaceHandler(WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLAddressSpaceHandler();

    /// Constructs initial information about what kind of address
    /// spaces the program has.
    ///
    /// - Handles all the constant address space variables.
    /// - Handles all the local address space variables.
    /// - Collects private address space variables if they are
    ///   accessed through a pointer or if their address is taken with
    ///   the &-operator.
    /// - Injects address space types and constant address space initialization
    ///   to prologue.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

    /// \return Whether private variables need to be relocated.
    bool hasPrivateAddressSpace();
    /// \return Whether local variables need to be relocated.
    bool hasLocalAddressSpace();
    /// \return Whether constant variables need to be relocated.
    bool hasConstantAddressSpace();

    /// \return Relocated private variables in order.
    AddressSpaceInfo &getPrivateAddressSpace();
    /// \return Relocated local variables in order.
    AddressSpaceInfo &getLocalAddressSpace();
    /// \return Relocated constant variables in order.
    AddressSpaceInfo &getConstantAddressSpace();

    /// Must be run after relocations to get correct variable name references to
    /// initializations.
    void emitConstantAddressSpaceAllocation();
  
    /// Replaces direct references to relocated variables with
    /// indirect references to the corresponding variables via address
    /// space structures.
    void doRelocations();

    /// Remove variables that have been relocated into address space structures.
    void removeRelocatedVariables();

    /// \return Whether variable declaration has been moved to address
    /// space structure.
    bool isRelocated(clang::VarDecl *decl);
  
private:

    typedef std::set<clang::VarDecl*> AddressSpaceSet;
    std::map< AddressSpaceSet*, AddressSpaceInfo > organizedAddressSpaces_;

    /// Sorts address space variables.
    ///
    /// If there is need to add padding bytes etc. inside address space
    /// structures, they should be added here. If address space is
    /// completely uninitialized, padding bytes aren't needed at all.
    AddressSpaceInfo& getOrCreateAddressSpaceInfo(AddressSpaceSet *declarations);

    /// Remove variables that have been relocated into a specific
    /// address space record.
    void removeRelocatedVariables(AddressSpaceSet &variables);

    /// Relocated private variables.
    AddressSpaceSet privates_;
    /// Relocated local variables.
    AddressSpaceSet locals_;
    /// Relocated constant varaibles.
    AddressSpaceSet constants_;
};

/// Prepares data structures required for memory access checks.
class WebCLKernelHandler : public WebCLPass
{
public:
  
    WebCLKernelHandler(
        WebCLAnalyser &analyser, WebCLTransformer &transformer,
        WebCLAddressSpaceHandler &addressSpaceHandler);
    virtual ~WebCLKernelHandler();

    /// - Analyzes kernel parameters to see what kind of memory area
    ///   limits are needed. Creates limit types. Adds size parameter
    ///   for each memory object parameter.
    /// - Allocates and initializes address space structures and
    ///   limits.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

    /// \return Memory area limits for the address space of given
    /// expression.
    AddressSpaceLimits& getLimits(clang::Expr *access, clang::VarDecl *decl);

    /// \return Whether any memory areas need to be checked.
    bool hasProgramAllocations();

    /// \return Whether constant memory areas need to be checked.
    bool hasConstantLimits();
    /// \return Whether global memory areas need to be checked.
    bool hasGlobalLimits();
    /// \return Whether local memory areas need to be checked.
    bool hasLocalLimits();
    /// \return Whether private memory areas need to be checked.
    bool hasPrivateLimits();

private:

    /// Provides information about relocated variables.
    WebCLAddressSpaceHandler &addressSpaceHandler_;
    /// Modifier helper function signatures and calls.
    WebCLHelperFunctionHandler helperFunctionHandler_;

    /// Contains dynamic limits for global address space.
    AddressSpaceLimits globalLimits_;
    /// Contains static and dynamic limits for constant address space.
    AddressSpaceLimits constantLimits_;
    /// Constains static and dynamic limits for local address space.
    AddressSpaceLimits localLimits_;
    /// Constains statuc limits for private address space.
    AddressSpaceLimits privateLimits_;

    /// Maps variable to a possibly more restricted set of limits.
    std::map< clang::VarDecl*, AddressSpaceLimits* > declarationLimits_;

    /// FUTURE: Performs a more detailed analysis on whether variable
    /// limit checks can be simplified.
    void createDeclarationLimits(clang::VarDecl *decl);
};

/// Generates memory access checks.
class WebCLMemoryAccessHandler : public WebCLPass
{
public:

    WebCLMemoryAccessHandler(
        WebCLAnalyser &analyser, WebCLTransformer &transformer,
        WebCLKernelHandler &kernelHandler);
    virtual ~WebCLMemoryAccessHandler();

    /// - Generates checks for pointer accesses.
    /// - Generates information about largest memory accesses.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

private:

    WebCLKernelHandler &kernelHandler_;
};

#endif // WEBCLVALIDATOR_WEBCLPASS
