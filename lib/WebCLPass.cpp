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

#include "WebCLDebug.hpp"
#include "WebCLPass.hpp"
#include "WebCLVisitor.hpp"
#include "WebCLTransformer.hpp"
#include "WebCLTypes.hpp"
#include "WebCLCommon.hpp"

#include "clang/AST/ASTContext.h"

WebCLPass::WebCLPass(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer)
    : WebCLReporter(instance)
    , analyser_(analyser), transformer_(transformer)
{
}

WebCLPass::~WebCLPass()
{
}

WebCLInputNormaliser::WebCLInputNormaliser(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer)
    : WebCLPass(instance, analyser, transformer)
{
}

WebCLInputNormaliser::~WebCLInputNormaliser()
{
}

void WebCLInputNormaliser::run(clang::ASTContext &context)
{
    WebCLAnalyser::TypeDeclList &typeDecls = analyser_.getTypeDecls();
    for (WebCLAnalyser::TypeDeclList::iterator i = typeDecls.begin();
        i != typeDecls.end(); ++i) {
            transformer_.moveToModulePrologue(*i);
    }
}

WebCLHelperFunctionHandler::WebCLHelperFunctionHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer)
    : WebCLPass(instance, analyser, transformer)
{
}

WebCLHelperFunctionHandler::~WebCLHelperFunctionHandler()
{
}

void WebCLHelperFunctionHandler::run(clang::ASTContext &context)
{
    // Go through all non kernel functions and add allocation
    // structure parameter in front of parameter list.
    WebCLAnalyser::FunctionDeclSet &helperFunctions = analyser_.getHelperFunctions();
    for (WebCLAnalyser::FunctionDeclSet::iterator i = helperFunctions.begin();
        i != helperFunctions.end(); ++i) {
            transformer_.addRecordParameter(*i);
    }

    // Go through all helper function calls and add allocation
    // structure argument in front of argument list.
    WebCLAnalyser::CallExprSet &internalCalls = analyser_.getInternalCalls();
    for (WebCLAnalyser::CallExprSet::iterator i = internalCalls.begin();
        i != internalCalls.end(); ++i) {
            transformer_.addRecordArgument(*i);
    }
}

WebCLAddressSpaceHandler::WebCLAddressSpaceHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer)
    : WebCLPass(instance, analyser, transformer)
    , organizedAddressSpaces_()
    , privates_(), locals_(), constants_()
{
}

WebCLAddressSpaceHandler::~WebCLAddressSpaceHandler()
{
}

void WebCLAddressSpaceHandler::run(clang::ASTContext &context)
{
    WebCLAnalyser::VarDeclSet &privateVars = analyser_.getPrivateVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = privateVars.begin();
        i != privateVars.end(); ++i) {
            // struct types must be also in address space, because there might be
            // tables / ponters inside struct... simpler structs could
            // maybe be left out.
            clang::VarDecl *decl = *i;
            if (decl->getType()->isPointerType() ||
                decl->getType()->isStructureType() ||
                decl->getType()->isArrayType() ||
                analyser_.hasAddressReferences(decl)) {
                    DEBUG(
                        std::cerr << "Adding to AS: "
                        << decl->getDeclName().getAsString() << "\n"; );
                    privates_.insert(decl);
            } else {
                DEBUG(
                    std::cerr << "Skipping: "
                    << decl->getDeclName().getAsString() << "\n"; );
            }
    }

    WebCLAnalyser::VarDeclSet &constantVars = analyser_.getConstantVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = constantVars.begin();
        i != constantVars.end(); ++i) {
            constants_.insert(*i);
    }

    WebCLAnalyser::VarDeclSet &localVars = analyser_.getLocalVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = localVars.begin();
        i != localVars.end(); ++i) {
            locals_.insert(*i);
    }

    // create address space types
    transformer_.createPrivateAddressSpaceTypedef(getPrivateAddressSpace());
    transformer_.createLocalAddressSpaceTypedef(getLocalAddressSpace());
    transformer_.createConstantAddressSpaceTypedef(getConstantAddressSpace());
}

bool WebCLAddressSpaceHandler::hasPrivateAddressSpace()
{
    return privates_.size() > 0;
}

bool WebCLAddressSpaceHandler::hasLocalAddressSpace()
{
    return locals_.size() > 0;
}

bool WebCLAddressSpaceHandler::hasConstantAddressSpace()
{
    return constants_.size() > 0;
}

AddressSpaceInfo &WebCLAddressSpaceHandler::getPrivateAddressSpace()
{
    return getOrCreateAddressSpaceInfo(&privates_);
}

AddressSpaceInfo &WebCLAddressSpaceHandler::getLocalAddressSpace()
{
    return getOrCreateAddressSpaceInfo(&locals_);
}

AddressSpaceInfo &WebCLAddressSpaceHandler::getConstantAddressSpace()
{
    return getOrCreateAddressSpaceInfo(&constants_);
}

void WebCLAddressSpaceHandler::emitConstantAddressSpaceAllocation()
{
    // Allocate space for constant address space and pass original
    // constants for initialization.
    transformer_.createConstantAddressSpaceAllocation(getConstantAddressSpace());    
    transformer_.createConstantAddressSpaceNullAllocation();
}

void WebCLAddressSpaceHandler::doRelocations()
{
    DEBUG( std::cerr << "Going through all variable uses: \n"; );

    WebCLAnalyser::DeclRefExprSet &varUses = analyser_.getVariableUses();
    for (WebCLAnalyser::DeclRefExprSet::iterator i = varUses.begin();
        i != varUses.end(); ++i) {

            clang::DeclRefExpr *use = *i;
            clang::VarDecl *decl = llvm::dyn_cast<clang::VarDecl>(use->getDecl());

            if (decl) {
                DEBUG( std::cerr << "Decl:" << use->getDecl()->getNameAsString() << "\n"; );

                // check if declaration has been moved to address space and add
                // transformation if so.
                if (isRelocated(decl)) {
                    // add initialization from function arg if necessary
                    if (clang::ParmVarDecl *parmDecl =
                        llvm::dyn_cast<clang::ParmVarDecl>(decl)) {
                            transformer_.addRelocationInitializerFromFunctionArg(parmDecl);
                    }

                    DEBUG( std::cerr << "--- relocated!\n"; );
                    transformer_.replaceWithRelocated(use, decl);
                }
            }
    }

    // add initializer for relocated private address space declarations
    for (AddressSpaceSet::iterator i = privates_.begin(); i != privates_.end(); ++i) {
        clang::VarDecl *privDecl = *i;
        if (privDecl->hasInit()) {
            if (analyser_.isInsideForStmt(privDecl)) {
                const char *message =
                    "Cannot currently relocate variables declared inside for statement. "
                    "Make sure that you have not taken address of counter variable "
                    "anywhere with & operator.";
                error(privDecl->getLocStart(), message);
            } else {
                transformer_.addRelocationInitializer(privDecl);
            }
        }
    }
}

void WebCLAddressSpaceHandler::removeRelocatedVariables()
{
    // Shouldn't be necessary, it is trivial to compiler to remove
    // these later on (current removing implementation breaks if there
    // is multiple variables in one declaration statement)
    //
    // FUTURE: implement removal also for private relocated variables, which are not
    //         used in initializations
    // removeRelocatedVariables(locals_);
    // removeRelocatedVariables(constants_);
}

bool WebCLAddressSpaceHandler::isRelocated(clang::VarDecl *decl)
{
    switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_global:
        assert(false && "Globals can't be relocated.");
        return false;
    case clang::LangAS::opencl_constant:
        return constants_.count(decl) > 0;
    case clang::LangAS::opencl_local:
        return locals_.count(decl) > 0;
    default:
        return privates_.count(decl) > 0;
    }
}

AddressSpaceInfo& WebCLAddressSpaceHandler::getOrCreateAddressSpaceInfo(AddressSpaceSet *declarations)
{
    // IMPROVEMENT: To optimize padding bytes to minimum
    //              add sorting declarations according to their size
    if (organizedAddressSpaces_.count(declarations) == 0) {
        for (AddressSpaceSet::iterator declIter = declarations->begin();
            declIter != declarations->end(); ++declIter) {
                organizedAddressSpaces_[declarations].push_back(*declIter);
        }
    }
    return organizedAddressSpaces_[declarations];
}

void WebCLAddressSpaceHandler::removeRelocatedVariables(AddressSpaceSet &variables)
{
    for (AddressSpaceSet::iterator i = variables.begin(); i != variables.end(); ++i) {
        clang::VarDecl *decl = (*i);
        // Remove all relocations except function parameters.
        if (!llvm::dyn_cast<clang::ParmVarDecl>(decl) && isRelocated(decl))
            transformer_.removeRelocated(decl);
    }
}

WebCLKernelHandler::WebCLKernelHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer,
    WebCLAddressSpaceHandler &addressSpaceHandler)
    : WebCLPass(instance, analyser, transformer)
    , addressSpaceHandler_(addressSpaceHandler)
    , helperFunctionHandler_(instance, analyser, transformer)
    , globalLimits_(clang::LangAS::opencl_global)
    , constantLimits_(clang::LangAS::opencl_constant)
    , localLimits_(clang::LangAS::opencl_local)
    , privateLimits_(0)
{
}

WebCLKernelHandler::~WebCLKernelHandler()
{
    // FUTURE: free memory from declaration limits table
}

void WebCLKernelHandler::run(clang::ASTContext &context)
{
    globalLimits_.setStaticLimits(false);
    constantLimits_.setStaticLimits(addressSpaceHandler_.hasConstantAddressSpace());
    localLimits_.setStaticLimits(addressSpaceHandler_.hasLocalAddressSpace());
    privateLimits_.setStaticLimits(true);

    // go through dynamic limits in the program and create variables for them
    WebCLAnalyser::KernelList &kernels = analyser_.getKernelFunctions();
    for (WebCLAnalyser::KernelList::iterator i = kernels.begin();
        i != kernels.end(); ++i) {
            for (std::vector<WebCLAnalyser::KernelArgInfo>::const_iterator j = i->args.begin();
                j != i->args.end(); ++j) {
                    const WebCLAnalyser::KernelArgInfo &parm = *j;
                    if (parm.pointerKind != WebCLAnalyser::NOT_POINTER) {

                        transformer_.addSizeParameter(parm.decl);

                        DEBUG(
                            std::cerr << "Adding dynamic limits from kernel:"
                            << func->getDeclName().getAsString()
                            << " arg:" << parm.name << "\n"; );

                        switch (parm.pointerKind) {
                        case WebCLAnalyser::GLOBAL_POINTER:
                            DEBUG( std::cerr << "Global address space!\n"; );
                            globalLimits_.insert(parm.decl);
                            break;
                        case WebCLAnalyser::CONSTANT_POINTER:
                            DEBUG( std::cerr << "Constant address space!\n"; );
                            constantLimits_.insert(parm.decl);
                            break;
                        case WebCLAnalyser::LOCAL_POINTER:
                            DEBUG( std::cerr << "Local address space!\n"; );
                            localLimits_.insert(parm.decl);
                            break;
                        case WebCLAnalyser::IMAGE_HANDLE:
                            DEBUG( std::cerr << "Image or sampler argument!\n"; );
                            break;
                        default:
                            // This used to be just a debug, but WebCLHeader made it a fatal error in the end.
                            // To move WebCLHeader out of the validator library, we must make it fatal here.
                            error(parm.decl->getLocStart(), "Invalid address space.");
                        }
                    }
            }
    }

    // Add typedefs for each limit structure. These are required if
    // static or dynamic allocations are present.
    if (!globalLimits_.empty())
        transformer_.createGlobalAddressSpaceLimitsTypedef(globalLimits_);
    if (!constantLimits_.empty())
        transformer_.createConstantAddressSpaceLimitsTypedef(constantLimits_);
    if (!localLimits_.empty())
        transformer_.createLocalAddressSpaceLimitsTypedef(localLimits_);
    if (hasProgramAllocations()) {
        transformer_.createProgramAllocationsTypedef(
            globalLimits_, constantLimits_, localLimits_,
            addressSpaceHandler_.getPrivateAddressSpace());
    }

    // now that we have all the data about the limit structures, we can actually
    // create the initialization code for each kernel
    for (WebCLAnalyser::KernelList::iterator i = kernels.begin();
        i != kernels.end(); ++i) {

            clang::FunctionDecl *func = i->decl;

            // Create allocation for local address space according to
            // earlier typedef. This is required if there are static
            // allocations but not if there are only dynamic allocations.
            if (addressSpaceHandler_.hasLocalAddressSpace())
                transformer_.createLocalAddressSpaceAllocation(func);

            // Allocate space for local null ptr
            transformer_.createLocalAddressSpaceNullAllocation(func);

            // allocate wcl_allocations_allocation and create the wcl_allocs
            // pointer to it, give all the data it needs to be able to create
            // also static initializator and prevent need for separate private
            // area zeroing...
            if (hasProgramAllocations()) {
                transformer_.createProgramAllocationsAllocation(
                    func, globalLimits_, constantLimits_, localLimits_,
                    addressSpaceHandler_.getPrivateAddressSpace());
            }

            // Initialize null pointers for global and private addres spaces
            transformer_.initializeAddressSpaceNull(func, globalLimits_);
            if (!addressSpaceHandler_.getPrivateAddressSpace().empty()) {
                transformer_.initializeAddressSpaceNull(func, privateLimits_);
            }

            // inject code that does zero initializing for all local memory ranges
            transformer_.createLocalAreaZeroing(func, localLimits_);
    }

    // Fixes all the function signatures and calls of internal helper functions
    // with additional wcl_allocs arg
    if (hasProgramAllocations())
        helperFunctionHandler_.run(context);

    // Now that limits and all new address spaces are created do the replacements
    // so that struct fields are used instead of original variable declarations.
    addressSpaceHandler_.doRelocations();
    addressSpaceHandler_.removeRelocatedVariables();
    addressSpaceHandler_.emitConstantAddressSpaceAllocation();
}

AddressSpaceLimits& WebCLKernelHandler::getLimits(
    clang::Expr *access, clang::VarDecl *decl)
{
    // IMPROVEMENT: remove if true after better static analysis for limit 
    //              resolving is added
    if (true || decl == NULL) {
        switch(WebCLTypes::getAddressSpace(access)) {
        case clang::LangAS::opencl_global:
            return globalLimits_;
        case clang::LangAS::opencl_constant:
            return constantLimits_;
        case clang::LangAS::opencl_local:
            return localLimits_;
        default:
            return privateLimits_;
        }
    }

    // FUTURE: implement getting specific limits..
    if (declarationLimits_.count(decl) == 0) {
        createDeclarationLimits(decl);
    }
    return *declarationLimits_[decl];
}

AddressSpaceLimits& WebCLKernelHandler::getDerefLimits(
    clang::Expr *access)
{
    switch(access->getType().getTypePtr()->getPointeeType().getAddressSpace()) {
    case clang::LangAS::opencl_global:
        return globalLimits_;
    case clang::LangAS::opencl_constant:
        return constantLimits_;
    case clang::LangAS::opencl_local:
        return localLimits_;
    default:
        return privateLimits_;
    }
    assert(false);
}

bool WebCLKernelHandler::hasProgramAllocations()
{
    return !addressSpaceHandler_.getPrivateAddressSpace().empty() ||
        hasGlobalLimits() || hasConstantLimits() || hasLocalLimits();
}

bool WebCLKernelHandler::hasConstantLimits()
{
    return !constantLimits_.empty();
}

bool WebCLKernelHandler::hasGlobalLimits()
{
    return !globalLimits_.empty();
}

bool WebCLKernelHandler::hasLocalLimits()
{
    return !localLimits_.empty();
}

bool WebCLKernelHandler::hasPrivateLimits()
{
    return !privateLimits_.empty();
}

void WebCLKernelHandler::createDeclarationLimits(clang::VarDecl *decl)
{
    // FUTURE: find out if we can trace single limits for this declaration...
}

WebCLMemoryAccessHandler::WebCLMemoryAccessHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser, WebCLTransformer &transformer,
    WebCLKernelHandler &kernelHandler)
    : WebCLPass(instance, analyser, transformer)
    , kernelHandler_(kernelHandler)
{
}

WebCLMemoryAccessHandler::~WebCLMemoryAccessHandler()
{
}

void WebCLMemoryAccessHandler::run(clang::ASTContext &context)
{
    // go through memory accesses from analyser
    WebCLAnalyser::MemoryAccessMap &pointerAccesses =
        analyser_.getPointerAceesses();

    std::map< unsigned, unsigned > maxAccess;
    // add default 8 bit align
    maxAccess[clang::LangAS::opencl_constant] = 8;
    maxAccess[clang::LangAS::opencl_global] = 8;
    maxAccess[clang::LangAS::opencl_local] = 8;
    maxAccess[0] = 8;

    for (WebCLAnalyser::MemoryAccessMap::iterator i = pointerAccesses.begin();
        i != pointerAccesses.end(); ++i) {

            clang::Expr *access = i->first;
            clang::VarDecl *decl = i->second;

            // update maximum access data
            unsigned addressSpace = WebCLTypes::getAddressSpace(access);
            unsigned accessWidth = context.getTypeSize(access->getType());
            if (maxAccess.count(addressSpace) == 0) {
                maxAccess[addressSpace] = accessWidth;
            }
            unsigned oldVal = maxAccess[addressSpace];
            maxAccess[addressSpace] = oldVal > accessWidth ? oldVal : accessWidth;

            // add memory check generation to transformer
            transformer_.addMemoryAccessCheck(
                access,
                1, // a single value
                kernelHandler_.getLimits(access, decl));
    }

    // add defines for address space specific minimum memory requirements.
    // this is needed to be able to serve all memory accesses in program
    for (std::map<unsigned, unsigned>::iterator i = maxAccess.begin();
        i != maxAccess.end(); i++) {

            transformer_.addMinimumRequiredContinuousAreaLimit(i->first, i->second);
    }
}

WebCLFunctionCallHandler::WebCLFunctionCallHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser,
    WebCLTransformer &transformer,
    WebCLKernelHandler &kernelHandler)
    : WebCLPass(instance, analyser, transformer)
    , kernelHandler_(kernelHandler)
{
    // nothing
}

void WebCLFunctionCallHandler::handle(clang::CallExpr *callExpr,
    bool builtin,
    unsigned& fnCounter)
{
    std::string origName = callExpr->getDirectCallee()->getNameInfo().getAsString();
    std::string wrapperName = "_wcl_" + origName + "_" + stringify(fnCounter);

    bool success = transformer_.wrapFunctionCall(wrapperName, callExpr, kernelHandler_);

    if (success) {
        ++fnCounter;
    } else if (builtin && analyser_.hasUnsafeParameters(callExpr)) {
        // error on unknown builtin functions involving pointer arguments
        error((callExpr)->getLocStart(), "Builtin argument check is required.");
    } else {
        // pass other unknown functions through
    }
}

void WebCLFunctionCallHandler::run(clang::ASTContext &context)
{
    WebCLAnalyser::CallExprSet builtinCalls = analyser_.getBuiltinCalls();
    WebCLAnalyser::CallExprSet internalCalls = analyser_.getInternalCalls();

    unsigned fnCounter = 0;

    for (WebCLAnalyser::CallExprSet::const_iterator builtinCallIt = builtinCalls.begin();
        builtinCallIt != builtinCalls.end();
        ++builtinCallIt) {
        handle(*builtinCallIt, true, fnCounter);
    }

    for (WebCLAnalyser::CallExprSet::const_iterator internalCallIt = internalCalls.begin();
        internalCallIt != internalCalls.end();
        ++internalCallIt) {
        handle(*internalCallIt, false, fnCounter);
    }
}

WebCLFunctionCallHandler::~WebCLFunctionCallHandler()
{
    // nothing
}

class WebCLImageSamplerSafetyHandler::TypeAccessChecker {
public:
    TypeAccessChecker();

    virtual bool validAccess(const clang::ValueDecl &valueDecl) const = 0;

    virtual ~TypeAccessChecker();
};

WebCLImageSamplerSafetyHandler::TypeAccessChecker::TypeAccessChecker()
{
    // nothing
}

WebCLImageSamplerSafetyHandler::TypeAccessChecker::~TypeAccessChecker()
{
    // nothing
}

class WebCLImageSamplerSafetyHandler::TypeAccessCheckerImage2d: public WebCLImageSamplerSafetyHandler::TypeAccessChecker {
public:
    TypeAccessCheckerImage2d() {}
    ~TypeAccessCheckerImage2d() {}

    virtual bool validAccess(const clang::ValueDecl &valueDecl) const {
        // Further check that the parameter access qualifier is supported, if present
        const clang::OpenCLImageAccess qualifier = valueDecl.getType().getAccess();
        return (!qualifier || qualifier == clang::CLIA_read_only || qualifier == clang::CLIA_write_only);
    }
};

class WebCLImageSamplerSafetyHandler::TypeAccessCheckerSampler: public WebCLImageSamplerSafetyHandler::TypeAccessChecker {
public:
    TypeAccessCheckerSampler() {}
    ~TypeAccessCheckerSampler() {}

    virtual bool validAccess(const clang::ValueDecl &valueDecl) const {
        return true;
    }
};

WebCLImageSamplerSafetyHandler::WebCLImageSamplerSafetyHandler(
    clang::CompilerInstance &instance,
    WebCLAnalyser &analyser,
    WebCLTransformer &transformer)
    : WebCLPass(instance, analyser, transformer)
{
    checkedTypes_["image2d_t"] = new TypeAccessCheckerImage2d;
    checkedTypes_["sampler_t"] = new TypeAccessCheckerSampler;
}

void WebCLImageSamplerSafetyHandler::run(clang::ASTContext &context)
{
    WebCLAnalyser::CallExprSet calls = analyser_.getBuiltinCalls();
    std::set<clang::DeclRefExpr*> usedAsArgument;

    calls.insert(analyser_.getInternalCalls().begin(), analyser_.getInternalCalls().end());

    for (WebCLAnalyser::CallExprSet::const_iterator callExprIt = calls.begin();
        callExprIt != calls.end();
        ++callExprIt) {
            clang::CallExpr *callExpr = *callExprIt;
            for (unsigned argIdx = 0; argIdx < callExpr->getNumArgs(); ++argIdx) {
                clang::Expr *expr = callExpr->getArg(argIdx);
                clang::QualType type = WebCLTypes::reduceType(instance_, expr->getType());
                for (TypeAccessCheckerMap::const_iterator checkedTypeIt = checkedTypes_.begin();
                     checkedTypeIt != checkedTypes_.end();
                     ++checkedTypeIt) {
                    const std::string& checkedTypeName = checkedTypeIt->first;
                    const TypeAccessChecker* checkedTypeChecker = checkedTypeIt->second;
                    if (type.getAsString() == checkedTypeName) {
                        enum {
                            // The corresponding param has not been found (yet)
                            PARAM_NOT_FOUND,
                            // The parameter was found but the type was wrong
                            TYPE_MISMATCH,
                            // The parameter was found but it had an illegal access qualifier
                            ILLEGAL_ACCESS,
                            // Everything is OK
                            SAFE
                        } safety = PARAM_NOT_FOUND;

                        clang::DeclRefExpr *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr);
                        if (!declRefExpr) {
                            if (clang::ImplicitCastExpr *implicitCastExpr = clang::dyn_cast<clang::ImplicitCastExpr>(expr)) {
                                declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(*implicitCastExpr->child_begin());
                            }
                        }

                        usedAsArgument.insert(declRefExpr);

                        if (declRefExpr) {
                            const clang::ValueDecl *valueDecl = declRefExpr->getDecl();
                            if (clang::isa<clang::ParmVarDecl>(valueDecl)) {
                                clang::QualType paramType = WebCLTypes::reduceType(instance_, valueDecl->getType());
                                if (paramType == type) {
                                    if (checkedTypeChecker->validAccess(*valueDecl)) {
                                        safety = SAFE;
                                    } else {
                                        safety = ILLEGAL_ACCESS;
                                    }
                                } else {
                                    // can this case ever happen?
                                    safety = TYPE_MISMATCH;
                                }
                            }
                        }


                        if (safety == TYPE_MISMATCH) {
                            error(expr->getLocStart(), "%0 must always originate from parameters with its original type") << checkedTypeName;
                        } else if (safety == ILLEGAL_ACCESS) {
                            // now this doesn't quite work with the genericity of TypeAccessChecker, but for the moment
                            // only image2d_t can fail with ILLEGAL_ACCESS
                            error(declRefExpr->getLocStart(), "%0 parameters can only have read_only or write_only access qualifier") << checkedTypeName;
                        } else if (safety == PARAM_NOT_FOUND) {
                            error(expr->getLocStart(), "%0 must always originate from parameters, unchanged") << checkedTypeName;
                        }
                    }
                }
            }
    }

    const WebCLAnalyser::DeclRefExprSet uses = analyser_.getVariableUses();
    for (WebCLAnalyser::DeclRefExprSet::const_iterator useIt = uses.begin();
        useIt != uses.end();
        ++useIt) {
            clang::DeclRefExpr *expr = *useIt;
            clang::QualType type = WebCLTypes::reduceType(instance_, expr->getType());
            for (TypeAccessCheckerMap::const_iterator checkedTypeIt = checkedTypes_.begin();
                 checkedTypeIt != checkedTypes_.end();
                 ++checkedTypeIt) {
                if (type.getAsString() == checkedTypeIt->first) {
                    if (!usedAsArgument.count(expr)) {
                        error((expr)->getLocStart(), "%0 must always be used as a function argument") << checkedTypeIt->first;
                    }
                }
            }
    }
}

WebCLImageSamplerSafetyHandler::~WebCLImageSamplerSafetyHandler()
{
    // nothing
}

