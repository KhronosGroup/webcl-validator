#include "WebCLConsumer.hpp"
#include "WebCLHelper.hpp"

#include "clang/AST/ASTContext.h"

#include <iostream>

WebCLConsumer::WebCLConsumer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , analyser_(instance)
    , checkingVisitors_()
    , relocator_(instance)
    , parameterizer_(instance)
    , accessor_(instance)
    , printer_(instance, rewriter)
    , transformingVisitors_()
    , transformer_(NULL)
{
    // Push in reverse order.
    checkingVisitors_.push_front(&analyser_);
    checkingVisitors_.push_front(&restrictor_);

    // Push in reverse order.
    transformingVisitors_.push_front(&printer_);
/*
    transformingVisitors_.push_front(&accessor_);
    transformingVisitors_.push_front(&parameterizer_);
    transformingVisitors_.push_front(&relocator_);
*/
 }

WebCLConsumer::~WebCLConsumer()
{
}

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
        
        std::cerr << "Adding to AS: " << decl->getDeclName().getAsString() << "\n";
        privates_.insert(decl);
      } else {
        std::cerr << "Skipping: " << decl->getDeclName().getAsString() << "\n";
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
    
    // allocate space for constant address space and pass original constants
    // for initialzations
    transformer_.createConstantAddressSpaceAllocation(getConstantAddressSpace());
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
  
  /// Fixes DeclRefExpr uses of variables to point to address space.
  ///
  /// If address space initialization code generation works correctly,
  /// we can just remove original declarations after this pass. Since
  /// constant address space creation requires initialization generation
  /// we can use the same code to generate static initializer for private
  /// address space.
  void doRelocations() {
    
    std::cerr << "Going through all variable uses: \n"; 
    WebCLAnalyser::DeclRefExprSet &varUses = analyser_.getVariableUses();
    for (WebCLAnalyser::DeclRefExprSet::iterator i = varUses.begin();
         i != varUses.end(); ++i) {
      
      clang::DeclRefExpr *use = *i;
      
      clang::VarDecl *decl = llvm::dyn_cast<clang::VarDecl>(use->getDecl());
      
      if (decl) {
        std::cerr << "Decl:" << use->getDecl()->getNameAsString() << "\n";

        // TODO: if original declaration is function argument, then add assignment
        //       from function argument to address space arg to start of the function
        //
        //
        // for function arg decl llvm::dyn_cast<ParmVarDecl>(decl)
        // i.e.
        // void foo(int arg) { bar(&arg); } ->
        // void foo(WclProgramAllocations *wcl_allocs, int arg) {
        //     wcl_allocs->pa.foo__arg = arg;
        //     bar(&wcl_acllocs->pa.foo__arg); }
        //
      
        // check if declaration has been moved to address space and add
        // transformation if so.
        if (isRelocated(decl)) {
          std::cerr << "--- relocated!\n";
          transformer_.replaceWithRelocated(use, decl);
        }
      }
    }
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
          
          std::cerr << "Adding dynamic limits from kernel:"
                    << func->getDeclName().getAsString()
                    << " arg:" << parm->getDeclName().getAsString() << "\n";

          switch (parm->getType().getTypePtr()->getPointeeType().getAddressSpace()) {
            case clang::LangAS::opencl_global:
              std::cerr << "Global address space!\n";
              globalLimits_.insert(parm);
              break;
            case clang::LangAS::opencl_constant:
              std::cerr << "Constant address space!\n";
              constantLimits_.insert(parm);
              break;
            case clang::LangAS::opencl_local:
              std::cerr << "Local address space!\n";
              localLimits_.insert(parm);
              break;
            default:
              assert(false && "Fail: Kernel arg to private address space!");
          }
        }
      }
    }
    
    // add typedefs for each limit structure
    transformer.createGlobalAddressSpaceLimitsTypedef(globalLimits_);
    transformer.createConstantAddressSpaceLimitsTypedef(constantLimits_);
    transformer.createLocalAddressSpaceLimitsTypedef(localLimits_);
    transformer.createProgramAllocationsTypedef(
      globalLimits_, constantLimits_, localLimits_,
      addressSpaceHandler.getPrivateAddressSpace());
    
    // now that we have all the data about the limit structures, we can actually
    // create the initialization code for each kernel
    for (WebCLAnalyser::FunctionDeclSet::iterator i = kernels.begin();
         i != kernels.end(); ++i) {
      
      clang::FunctionDecl *func = *i;
      
      // Create allocation for local address space according to earlier typedef
      transformer.createLocalAddressSpaceAllocation(func);

      // allocate wcl_allocations_allocation and create the wcl_allocs
      // pointer to it, give all the data it needs to be able to create
      // also static initializator and prevent need for separate private
      // area zeroing...
      transformer.createProgramAllocationsAllocation(
        func, globalLimits_, constantLimits_, localLimits_,
        addressSpaceHandler.getPrivateAddressSpace());

      // inject code that does zero initializing for all local memory ranges
      transformer.createLocalAreaZeroing(func, localLimits_);
    }
  };
  
  ~KernelHandler() {};
  
private:
  AddressSpaceLimits globalLimits_;
  AddressSpaceLimits constantLimits_;
  AddressSpaceLimits localLimits_;
  
};

/// TODO: refactor when ready to separate file
class MemoryAccessHandler {
public:
  MemoryAccessHandler(WebCLAnalyser &analyser,
                WebCLTransformer &transformer,
                KernelHandler &kernelHandler) {

    // TODO: go through memory accesses from analyser

    // TODO: get memory access width to possibly create NULL ptr...
    //       this will not work for global memory, so we should get way to
    //       completetly abort?
    
    // TODO: find out list of limits of the address space
    
    // TODO: if access has multiple address ranges try to do some slight trickery to
    //       get correct limits to check also check if limit check is actally needed
    //       at all
    
    // TODO: add limit check
    
    // TODO: if not yet added, add macro with correct min,max pair count

    // TODO: find out biggest memory access width of each address space and
    //       add assert to start of kernels that we have enough global, local and constant
    //       memory to do those accesses (check kernel size arguments to be in valid range)
    //       we could have added the checks already and just output defines
    //       stating WCL_MAX_ACCESS_WIDTH_TO_LOCAL etc. defines.
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
  
    // TODO: class that for now just moves all the  typedefs
    //       to be in start of file
    // InputCodeOrganiser addressSpaceTransformer(*anlyser_);

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
    HelperFunctionHandler helperFunctionHandler(analyser_, *transformer_);

    // Now that limits and all new address spaces are created do the replacements
    // so that struct fields are used instead of original variable declarations.
    addressSpaceHandler.doRelocations();
  
    // Emits pointer checks for all memory accesses and injects
    // required check macro definitions to prolog.
    MemoryAccessHandler
      memoryAccessHandler(analyser_, *transformer_, kernelHandler);
  
    // FUTURE: add class, which goes through builtins and adds required
    //         things to make calls safe
  
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
