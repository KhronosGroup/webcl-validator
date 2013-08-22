#include "WebCLConsumer.hpp"
#include "WebCLHelper.hpp"

#include "clang/AST/ASTContext.h"

#include "WebCLDebug.hpp"

WebCLConsumer::WebCLConsumer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , analyser_(instance)
    , checkingVisitors_()
    , printer_(instance, rewriter)
    , transformingVisitors_()
    , transformer_(NULL)
{
    // Push in reverse order.
    checkingVisitors_.push_front(&analyser_);
    checkingVisitors_.push_front(&restrictor_);
    // Push in reverse order.
    transformingVisitors_.push_front(&printer_);
 }

WebCLConsumer::~WebCLConsumer()
{
}

/// TODO: refactor when ready to separate file
class InputNormaliser {
public:
  InputNormaliser(WebCLAnalyser &analyser, WebCLTransformer &transformer) {

    // FUTURE: this should be separate pass after macro expansion, but before
    //         instrumentation to prepare code for instrumentation and then code
    //         should be parsed again and fed to instrumentation
    
    // FUTURE: move declaration part from each for loop to be before for statement
    
    // go through all typedefs and structure definitions and move them to start of module
    WebCLAnalyser::TypeDeclList &typeDecls = analyser.getTypeDecls();
    for (WebCLAnalyser::TypeDeclList::iterator i = typeDecls.begin();
         i != typeDecls.end(); ++i) {
      transformer.moveToModulePrologue(*i);
    }

  };
  
  InputNormaliser() {};
  
private:
};

/// TODO: refactor when ready to separate file
class HelperFunctionHandler {
public:
  HelperFunctionHandler(WebCLAnalyser &analyser, WebCLTransformer &transformer) {

    // go thorugh all non kernel functions and add first parameter
    WebCLAnalyser::FunctionDeclSet &helperFunctions = analyser.getHelperFunctions();
    for (WebCLAnalyser::FunctionDeclSet::iterator i = helperFunctions.begin();
         i != helperFunctions.end(); ++i) {
        transformer.addRecordParameter(*i);
    }
    
    // go through all helper call and add wcl_alloc argument to call
    WebCLAnalyser::CallExprSet &internalCalls = analyser.getInternalCalls();
    for (WebCLAnalyser::CallExprSet::iterator i = internalCalls.begin();
         i != internalCalls.end(); ++i) {
      transformer.addRecordArgument(*i);
    }
    
  };
  
  HelperFunctionHandler() {};

private:
};

/// TODO: refactor when ready to separate file
class AddressSpaceHandler {
public:
  
  /// Constructs initial information what kind of address spaces program has
  ///
  /// - handle all the constant address space variables
  /// - handle all the local address space variables
  /// - collect private address space variables if they are accessed
  ///   through pointer or if & reference is taken.
  /// - inject address space types and constant address space initialization
  ///   to prologue
  AddressSpaceHandler(WebCLAnalyser &analyser, WebCLTransformer &transformer) :
    analyser_(analyser), transformer_(transformer) {
  
    WebCLAnalyser::VarDeclSet &privateVars = analyser.getPrivateVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = privateVars.begin();
        i != privateVars.end(); ++i) {
      
      // struct types must be also in address space, because there might be
      // tables / ponters inside struct... simpler structs could
      // maybe be left out.
      clang::VarDecl *decl = *i;
      if (decl->getType()->isPointerType() ||
          decl->getType()->isStructureType() ||
          decl->getType()->isArrayType() ||
          analyser.hasAddressReferences(decl)) {
        
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
    
    WebCLAnalyser::VarDeclSet &constantVars = analyser.getConstantVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = constantVars.begin();
        i != constantVars.end(); ++i) {
      constants_.insert(*i);
    }
    
    WebCLAnalyser::VarDeclSet &localVars = analyser.getLocalVariables();
    for(WebCLAnalyser::VarDeclSet::iterator i = localVars.begin();
        i != localVars.end(); ++i) {
      locals_.insert(*i);
    }
      
    // create address space types
    transformer_.createPrivateAddressSpaceTypedef(getPrivateAddressSpace());
    transformer_.createLocalAddressSpaceTypedef(getLocalAddressSpace());
    transformer_.createConstantAddressSpaceTypedef(getConstantAddressSpace());
    
  };
  
  ~AddressSpaceHandler() {};
  
  bool hasPrivateAddressSpace()  { return privates_.size() > 0; };
  bool hasLocalAddressSpace()    { return locals_.size() > 0; };
  bool hasConstantAddressSpace() { return constants_.size() > 0; };

  AddressSpaceInfo &getPrivateAddressSpace() {
    return getOrCreateAddressSpaceInfo(&privates_);
  };

  AddressSpaceInfo &getLocalAddressSpace() {
    return getOrCreateAddressSpaceInfo(&locals_);
  };

  AddressSpaceInfo &getConstantAddressSpace() {
    return getOrCreateAddressSpaceInfo(&constants_);
  };
  
  /// Must be run after reloactions to get correct variablenamereferences to
  /// initializations
  void emitConstantAddressSpaceAllocation() {
    // allocate space for constant address space and pass original constants
    // for initialzations
    transformer_.createConstantAddressSpaceAllocation(getConstantAddressSpace());    
    transformer_.createConstantAddressSpaceNullAllocation();
  };
  
  /// Fixes DeclRefExpr uses of variables to point to address space.
  ///
  /// If address space initialization code generation works correctly,
  /// we can just remove original declarations after this pass. Since
  /// constant address space creation requires initialization generation
  /// we can use the same code to generate static initializer for private
  /// address space.
  void doRelocations() {
    
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
    for (AddressSpaceSet::iterator privDecl = privates_.begin();
         privDecl != privates_.end(); privDecl++) {

      if ((*privDecl)->hasInit()) {
        // LAUNDRY: change assert to be nicer error message
        assert(analyser_.isInsideForStmt(*privDecl) == false && "Cannot currently relocate variables declared inside for statement. Make sure that you have not taken address of counter variable anywhere with & operator.");
        transformer_.addRelocationInitializer(*privDecl);
      }
    }
  };

  /// \brief Remove variables that have been relocated into any
  /// address space records.
  void removeRelocatedVariables()
  {
    // Shouldn't be necessary, it is trivial to compiler to remove these later on
    // (current removing implementation breaks if there is multiple variables in one declaration statement)
    // FUTURE: implement removal also for private relocated variables, which are not
    //         used in initializations
    // removeRelocatedVariables(locals_);
    // removeRelocatedVariables(constants_);
  };

  /// \brief Returns true if variable declaration is moved to address space struct
  bool isRelocated(clang::VarDecl *decl) {
    switch (decl->getType().getAddressSpace()) {
      case clang::LangAS::opencl_global: assert(false && "Globals can't be relocated.");
      case clang::LangAS::opencl_constant: return constants_.count(decl) > 0;
      case clang::LangAS::opencl_local: return locals_.count(decl) > 0;
      default: return privates_.count(decl) > 0;
    }
  };
  
private:
  typedef std::set<clang::VarDecl*> AddressSpaceSet;
  std::map< AddressSpaceSet*, AddressSpaceInfo > organizedAddressSpaces;

  /// Creates order for the address space
  ///
  /// If there is need to add padding bytes etc. inside address space
  /// then they should be added here. If address space is completelu uninitialized
  /// then paddings are not needed at all.
  AddressSpaceInfo& getOrCreateAddressSpaceInfo(AddressSpaceSet *declarations) {
    
    // TODO: add sorting declarations according to their size
    
    if (organizedAddressSpaces.count(declarations) == 0) {
      for (AddressSpaceSet::iterator declIter = declarations->begin();
           declIter != declarations->end(); ++declIter) {
        organizedAddressSpaces[declarations].push_back(*declIter);
      }
    }
    return organizedAddressSpaces[declarations];
  };

  /// \brief Remove variables that have been relocated into a specific
  /// address space record.
  void removeRelocatedVariables(AddressSpaceSet &variables)
  {
    for (AddressSpaceSet::iterator i = variables.begin(); i != variables.end(); ++i) {
      clang::VarDecl *decl = (*i);
      // Remove all relocations except function parameters.
      if (!llvm::dyn_cast<clang::ParmVarDecl>(decl) && isRelocated(decl))
        transformer_.removeRelocated(decl);
    }
  };

  WebCLAnalyser &analyser_;
  WebCLTransformer &transformer_;
  AddressSpaceSet privates_;
  AddressSpaceSet locals_;
  AddressSpaceSet constants_;
  
};

/// TODO: refactor when ready to separate file
class KernelHandler {
public:
  
  KernelHandler(WebCLAnalyser &analyser,
                WebCLTransformer &transformer,
                AddressSpaceHandler &addressSpaceHandler) :
      globalLimits_(false,
                    clang::LangAS::opencl_global)
    , constantLimits_(addressSpaceHandler.hasConstantAddressSpace(),
                      clang::LangAS::opencl_constant)
    , localLimits_(addressSpaceHandler.hasLocalAddressSpace(),
                   clang::LangAS::opencl_local)
    , privateLimits_(true, 0)
  {
    
    // go through dynamic limits in the program and create variables for them
    WebCLAnalyser::FunctionDeclSet &kernels = analyser.getKernelFunctions();
    for (WebCLAnalyser::FunctionDeclSet::iterator i = kernels.begin();
         i != kernels.end(); ++i) {
      
      clang::FunctionDecl *func = *i;
      for (clang::FunctionDecl::param_iterator parmIter = func->param_begin();
           parmIter != func->param_end(); ++parmIter) {
        
        clang::ParmVarDecl *parm = *parmIter;
        
        if (parm->getType().getTypePtr()->isPointerType()) {
        
          transformer.addSizeParameter(parm);
          
          DEBUG(
            std::cerr << "Adding dynamic limits from kernel:"
                      << func->getDeclName().getAsString()
                      << " arg:" << parm->getDeclName().getAsString() << "\n"; );

          switch (parm->getType().getTypePtr()->getPointeeType().getAddressSpace()) {
            case clang::LangAS::opencl_global:
              DEBUG( std::cerr << "Global address space!\n"; );
              globalLimits_.insert(parm);
              break;
            case clang::LangAS::opencl_constant:
              DEBUG( std::cerr << "Constant address space!\n"; );
              constantLimits_.insert(parm);
              break;
            case clang::LangAS::opencl_local:
              DEBUG( std::cerr << "Local address space!\n"; );
              localLimits_.insert(parm);
              break;
            default:
              DEBUG( std::cerr << "Private address space kernel arg pointer! Vendor compiler should catch this if really invalid and not some typedef of valid opencl type.\n"; );
          }
        }
      }
    }
    
    // Add typedefs for each limit structure. These are required if
    // static or dynamic allocations are present.
    if (!globalLimits_.empty())
        transformer.createGlobalAddressSpaceLimitsTypedef(globalLimits_);
    if (!constantLimits_.empty())
        transformer.createConstantAddressSpaceLimitsTypedef(constantLimits_);
    if (!localLimits_.empty())
        transformer.createLocalAddressSpaceLimitsTypedef(localLimits_);
    if (hasProgramAllocations(addressSpaceHandler)) {
        transformer.createProgramAllocationsTypedef(
            globalLimits_, constantLimits_, localLimits_,
            addressSpaceHandler.getPrivateAddressSpace());
    }
    
    // now that we have all the data about the limit structures, we can actually
    // create the initialization code for each kernel
    for (WebCLAnalyser::FunctionDeclSet::iterator i = kernels.begin();
         i != kernels.end(); ++i) {
      
      clang::FunctionDecl *func = *i;

      // Create allocation for local address space according to
      // earlier typedef. This is required if there are static
      // allocations but not if there are only dynamic allocations.
      if (addressSpaceHandler.hasLocalAddressSpace())
          transformer.createLocalAddressSpaceAllocation(func);

      // Allocate space for local null ptr
      transformer.createLocalAddressSpaceNullAllocation(func);
      
      // allocate wcl_allocations_allocation and create the wcl_allocs
      // pointer to it, give all the data it needs to be able to create
      // also static initializator and prevent need for separate private
      // area zeroing...
      if (hasProgramAllocations(addressSpaceHandler)) {
          transformer.createProgramAllocationsAllocation(
              func, globalLimits_, constantLimits_, localLimits_,
              addressSpaceHandler.getPrivateAddressSpace());
      }
      
      // Initialize null pointers for global and private addres spaces
      transformer.initializeAddressSpaceNull(func, globalLimits_);
      if (!addressSpaceHandler.getPrivateAddressSpace().empty()) {
          transformer.initializeAddressSpaceNull(func, privateLimits_);        
      }

      // inject code that does zero initializing for all local memory ranges
      transformer.createLocalAreaZeroing(func, localLimits_);
    }
  };
  
  AddressSpaceLimits& getLimits(clang::Expr *access, clang::VarDecl *decl) {
    
    // TODO: remove if true after better limit resolving is added
    if (true ||decl == NULL) {
      switch( access->getType().getAddressSpace()) {
        case clang::LangAS::opencl_global:   return globalLimits_;
        case clang::LangAS::opencl_constant: return constantLimits_;
        case clang::LangAS::opencl_local:    return localLimits_;
        default: return privateLimits_;
      }
    }
    
    // FUTURE: implement getting specific limits..
    if (declarationLimits_.count(decl) == 0) {
      createDeclarationLimits(decl);
    }
    return *declarationLimits_[decl];
  };
  
  ~KernelHandler() {
    // FUTURE: free memory from declaration limits table
  };

  bool hasProgramAllocations(AddressSpaceHandler &addressSpaceHandler)
  {
      return !addressSpaceHandler.getPrivateAddressSpace().empty() ||
          !globalLimits_.empty() || !constantLimits_.empty() || !localLimits_.empty();
  };

  bool hasConstantLimits()
  {
    return !constantLimits_.empty();
  };

  bool hasGlobalLimits()
  {
    return !globalLimits_.empty();
  };

  bool hasLocalLimits()
  {
      return !localLimits_.empty();
  };

  bool hasPrivateLimits()
  {
    return !privateLimits_.empty();
  };

private:
  AddressSpaceLimits globalLimits_;
  AddressSpaceLimits constantLimits_;
  AddressSpaceLimits localLimits_;
  AddressSpaceLimits privateLimits_;
  
  std::map< clang::VarDecl*, AddressSpaceLimits* > declarationLimits_;

  void createDeclarationLimits(clang::VarDecl *decl) {
    // FUTURE: find out if we can trace single limits for this declaration...
  };
};

/// TODO: refactor when ready to separate file
class MemoryAccessHandler {
public:
    MemoryAccessHandler(
        WebCLAnalyser &analyser, WebCLTransformer &transformer,
        KernelHandler &kernelHandler, clang::ASTContext &context) {

    // go through memory accesses from analyser
    WebCLAnalyser::MemoryAccessMap &pointerAccesses =
      analyser.getPointerAceesses();

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
      unsigned addressSpace = access->getType().getAddressSpace();
      unsigned accessWidth = context.getTypeSize(access->getType());
      if (maxAccess.count(addressSpace) == 0) {
        maxAccess[addressSpace] = accessWidth;
      }
      unsigned oldVal = maxAccess[addressSpace];
      maxAccess[addressSpace] = oldVal > accessWidth ? oldVal : accessWidth;
      
      // add memory check generation to transformer
      transformer.addMemoryAccessCheck(
        access, kernelHandler.getLimits(access, decl));
    }

    // add defines for address space specific minimum memory requirements.
    // this is needed to be able to serve all memory accesses in program
    for (std::map<unsigned, unsigned>::iterator i = maxAccess.begin();
         i != maxAccess.end(); i++) {
      
      transformer.addMinimumRequiredContinuousAreaLimit(i->first, i->second);
    }
  }

  ~MemoryAccessHandler() {};
private:
};


void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    assert(transformer_ != NULL);
  
    traverse(checkingVisitors_, context);

    // Introduce changes in way that after every step we still have correclty
    // running program.

    // FUTURE: Add memory limit dependence ananlysis here
    // (or maybe it could be even separate visitor). Also value range
    // analysis could help in meny cases. Based on dependence
    // anlysis could create separate implementations for different calls
    // if we know which arguments are passed to function

    // Class that for now just moves all the  typedefs
    // and defines to start of program
    InputNormaliser inputNormaliser(analyser_, *transformer_);

    // Collect and organize all the information from analyser_ to create
    // view to different address spaces and to provide replacements for
    // replaced variable references in first stage just create types and collect
    // information about the address spaces for kernel initialization
    // code generation
    AddressSpaceHandler addressSpaceHandler(analyser_, *transformer_);
  
    // Fixes kernel arguments and injects kernel initialization code writes limit
    // struct typedefs according to address spaces and kernel arguments
    // also writes initialization code for local and private memory
    KernelHandler kernelHandler(analyser_, *transformer_, addressSpaceHandler);
  
    // Fixes all the function signatures and calls of internal helper functions
    // with additional wcl_allocs arg
    if (kernelHandler.hasProgramAllocations(addressSpaceHandler)) {
      HelperFunctionHandler helperFunctionHandler(analyser_, *transformer_);
    }
  
    // Now that limits and all new address spaces are created do the replacements
    // so that struct fields are used instead of original variable declarations.
    addressSpaceHandler.doRelocations();

    addressSpaceHandler.removeRelocatedVariables();
    addressSpaceHandler.emitConstantAddressSpaceAllocation();
  
    // Emits pointer checks for all memory accesses and injects
    // required check macro definitions to prolog.
    MemoryAccessHandler memoryAccessHandler(
          analyser_, *transformer_, kernelHandler, context);
  
    // TODO: make sure that when we are calling kernel, that we have enough
    // memory allocated to do all the memory accesses
    // kernelHandler.addMemoryLimitChecks();

    // FUTURE: add class, which goes through builtins and creates corresponding
    //         safe calls and adds safe implementations to source.
  
    traverse(transformingVisitors_, context);
}

void WebCLConsumer::setTransformer(WebCLTransformer &transformer)
{
    transformer_= &transformer;
    for (TransformingVisitors::iterator i = transformingVisitors_.begin();
         i != transformingVisitors_.end(); ++i) {
        WebCLTransformingVisitor *visitor = (*i);
        visitor->setTransformer(transformer);
    }
}

template <typename VisitorSequence>
void WebCLConsumer::traverse(VisitorSequence &sequence, clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();

    for (typename VisitorSequence::iterator i = sequence.begin();
         i != sequence.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLVisitor *visitor = (*i);
            visitor->TraverseDecl(decl);
        }
    }
}

bool WebCLConsumer::hasErrors(clang::ASTContext &context) const
{
   clang::DiagnosticsEngine &diags = context.getDiagnostics();
   return diags.hasErrorOccurred() || diags.hasUnrecoverableErrorOccurred();
}
