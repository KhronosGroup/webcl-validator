#include "WebCLConsumer.hpp"

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
    analyser_(analyser){
  
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
  
    // TODO: create prologue injection code with address space types
    std::cerr << "Has private address space:" << hasPrivateAddressSpace() << "\n";
    std::cerr << "Has local address space:" << hasLocalAddressSpace() << "\n";
    std::cerr << "Has constant address space:" << hasConstantAddressSpace() << "\n";
  };
  
  ~AddressSpaceHandler() {};

  bool hasPrivateAddressSpace()         { return privates_.size() > 0; };
  bool hasLocalAddressSpace()    { return locals_.size() > 0; };
  bool hasConstantAddressSpace() { return constants_.size() > 0; };
  
  void createLocalAddressSpaceStruct(clang::FunctionDecl *decl) {
      // TODO: add allocating wcl_allocs structure to start of the function
      //       in case if we have static local data.
  };
  
  void addRelocations() {
    
    WebCLAnalyser::DeclRefExprSet &varUses = analyser_.getVariableUses();
    for (WebCLAnalyser::DeclRefExprSet::iterator i = varUses.begin();
         i != varUses.end(); ++i) {
      
      clang::DeclRefExpr *expr = *i;

      // TODO: check if declaration has been moved to address space and add
      //       transformation if so.
      //
      // TODO: if original declaration is function argument, then add assignment
      //       from function argument to address space arg to start of the function
      //       for function arg decl llvm::dyn_cast<ParamVarDec>(decl)
      // i.e.
      // void foo(int arg) { bar(&arg); } ->
      // void foo(WclProgramAllocations *wcl_allocs, int arg) {
      //     wcl_allocs->pa.foo__arg = arg;
      //     bar(&wcl_acllocs->pa.foo__arg); }
      //
    }
  };
  
  // TODO: accessors for other handler to use data analyzed by this handler
  // TODO: accessor to get list of limits of this address space
  
private:
  WebCLAnalyser &analyser_;
  std::set<clang::VarDecl*> privates_;
  std::set<clang::VarDecl*> locals_;
  std::set<clang::VarDecl*> constants_;
  
};

/// TODO: refactor when ready to separate file
class KernelHandler {
public:
  KernelHandler(WebCLAnalyser &analyser,
                WebCLTransformer &transformer,
                AddressSpaceHandler &addressSpaceHandler) {
    
     // TODO: Create limit typedef for every address space
     //   * check from address space manager address spaces, which will need struct
     //   * go through all kernel arguments and sort to address spaces as well

     // TODO: Create tell address space manager to create wcl_locals structure
    
     // TODO: Create WclProgramAllocations structure
     //   * create also wcl_allocs which is pointer pointing to struct

     // TODO: add dummy call to inject initializations for every limit
     //       that requires zeroing e.g. outputtin some comment is enough now
    
    // TODO: Go through every limit pair and store initializations
    //      (ask from address space handler if constant / local address space
    //       requires static limits)
    
  };
  
  KernelHandler() {};
  
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

    // TODO: Collect and organize all the information from
    //       analyser_ to create view to different address spaces
    //       and to provide replacements for replaced variable
    //       references in first stage just create types and collect
    //       information about the address spaces for kernel initialization code
    //       generation
    AddressSpaceHandler addressSpaceHandler(analyser_, *transformer_);
  
    // TODO: Add class that fixes kernel arguments and
    //       injects kernel initialization code writes limit
    //       structures according to address spaces
    //       and kernel arguments
    //       also writes zero initializators for local and private memory
    KernelHandler kernelHandler(analyser_, *transformer_, addressSpaceHandler);
  
    // now that limits and all new address spaces are created do the replacements
    // so that struct fields are used instead of originals.
    addressSpaceHandler.addRelocations();
  
    // TODO: Add class that fixes all the function signatures and calls of non
    //       kernel functions with additional wcl_allocs arg
    HelperFunctionHandler helperFunctionHandler(analyser_, *transformer_);
  

    // TODO: Add class that emits pointer checks for all memory accesses
    //       and injects required check macros to prolog.
  
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
