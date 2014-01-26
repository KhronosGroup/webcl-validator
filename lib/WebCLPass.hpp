#ifndef WEBCLVALIDATOR_WEBCLPASS
#define WEBCLVALIDATOR_WEBCLPASS

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

#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"

#include <map>
#include <set>

namespace clang {
    class ASTContext;
    class Expr;
    class VarDecl;
    class CallExpr;
}

class WebCLAnalyser;
class WebCLTransformer;

/// Abstract base class for different passes of validation
/// stage. Derived classes perform transformations that depend on
/// results of the analysis pass.
class WebCLPass : public WebCLReporter
{
public:

    WebCLPass(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
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
///         validation. What we have to do here is finding the correct position
///         where type declarations end and address space type declarations
///         can be added.
///
/// \see WebCLMatcherAction
/// \see WebCLTool
class WebCLInputNormaliser : public WebCLPass
{
public:

    WebCLInputNormaliser(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
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

    WebCLHelperFunctionHandler(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
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
  
    WebCLAddressSpaceHandler(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
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
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer,
        WebCLAddressSpaceHandler &addressSpaceHandler);
    virtual ~WebCLKernelHandler();

    /// - Analyzes kernel parameters to see what kind of memory area
    ///   limits are needed. Creates limit types. Adds size parameter
    ///   for each memory object parameter.
    /// - Allocates and initializes address space structures and
    ///   limits.
    /// - Exports kernel signatures in JSON format.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

    /// \return Memory area limits for the address space of given
    /// expression.
    AddressSpaceLimits& getLimits(clang::Expr *access, clang::VarDecl *decl);

    /// \return Memory area limits for the address space of given
    /// expression when it is being dereference
    AddressSpaceLimits& getDerefLimits(clang::Expr *access);

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
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser, WebCLTransformer &transformer,
        WebCLKernelHandler &kernelHandler);
    virtual ~WebCLMemoryAccessHandler();

    /// - Generates checks for pointer accesses.
    /// - Generates information about largest memory accesses.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

private:

    /// Contains information about address space limits.
    WebCLKernelHandler &kernelHandler_;
};

/// Generates memory access checks and disallows calls to undeclared functions.
class WebCLFunctionCallHandler : public WebCLPass
{
public:
    WebCLFunctionCallHandler(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser,
	WebCLTransformer &transformer,
        WebCLKernelHandler &kernelHandler);
    virtual ~WebCLFunctionCallHandler();

    /// - Generates checks for pointer accesses.
    /// - Generates information about largest memory accesses.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

private:
    /// Contains information about address space limits.
    WebCLKernelHandler &kernelHandler_;

    void handle(clang::CallExpr *callExpr, bool builtin, unsigned& fnCounter);
};

/// Checks that image2d_t and sampler_t can only originate from function arguments
class WebCLImageSamplerSafetyHandler : public WebCLPass
{
public:
    WebCLImageSamplerSafetyHandler(
        clang::CompilerInstance &instance,
        WebCLAnalyser &analyser,
	WebCLTransformer &transformer,
        WebCLKernelHandler &kernelHandler);
    virtual ~WebCLImageSamplerSafetyHandler();

    /// - Goes through all function calls and checks that their image2d_t
    /// arguments refer to function arguments
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

private:
    class TypeAccessChecker;
    class TypeAccessCheckerImage2d;
    class TypeAccessCheckerSampler;

    typedef std::map<std::string, TypeAccessChecker*> TypeAccessCheckerMap;

    WebCLKernelHandler &kernelHandler_;
    TypeAccessCheckerMap checkedTypes_;
};

#endif // WEBCLVALIDATOR_WEBCLPASS
