#include "WebCLAction.hpp"
#include "WebCLPreprocessor.hpp"
#include "WebCLTransformer.hpp"

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/Sema.h"

#include "llvm/ADT/OwningPtr.h"

WebCLAction::WebCLAction()
    : clang::FrontendAction()
    , reporter_(NULL), preprocessor_(NULL)
{
}

WebCLAction::~WebCLAction()
{
    // preprocessor_ not deleted intentionally
    delete reporter_;
    reporter_ = NULL;
}

bool WebCLAction::initialize(clang::CompilerInstance &instance)
{
    reporter_ = new WebCLReporter(instance);
    if (!reporter_) {
        llvm::errs() << "Internal error. Can't report errors.\n";
        return false;
    }

    preprocessor_ = new WebCLPreprocessor(instance);
    if (!preprocessor_) {
        reporter_->fatal("Internal error. Can't create preprocessor callbacks.\n");
        return false;
    }
    clang::Preprocessor &preprocessor = instance.getPreprocessor();
    preprocessor.addPPCallbacks(preprocessor_);

    const clang::FrontendInputFile &file = getCurrentInput();
    if (file.getKind() != clang::IK_OpenCL) {
        static const char *format =
            "Source file '%0' isn't treated as OpenCL code. Make sure that you "
            "give the '-x cl' option or that the file has a '.cl' extension.\n";
        reporter_->fatal(format) << file.getFile();
        return false;
    }

    return true;
}

WebCLPreprocessorAction::WebCLPreprocessorAction(const char *outputFile)
    : WebCLAction()
    , outputFile_(outputFile)
{
}

WebCLPreprocessorAction::~WebCLPreprocessorAction()
{
}

clang::ASTConsumer* WebCLPreprocessorAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    return NULL;
}

void WebCLPreprocessorAction::ExecuteAction()
{
    clang::CompilerInstance &instance = getCompilerInstance();
    if (!initialize(instance))
        return;

    // see clang::PrintPreprocessedAction
    instance.getFrontendOpts().OutputFile = outputFile_;
    llvm::raw_ostream *out = instance.createDefaultOutputFile(true, getCurrentFile());
    if (!out)
        return;

    clang::PreprocessorOutputOptions& options = instance.getPreprocessorOutputOpts();
    options.ShowComments = 1;
    options.ShowLineMarkers = 0;
    clang::DoPrintPreprocessedInput(
        instance.getPreprocessor(), out, options);
    out->flush();
}

bool WebCLPreprocessorAction::usesPreprocessorOnly() const
{
    return true;
}

WebCLValidatorAction::WebCLValidatorAction()
    : WebCLAction()
    , consumer_(0)
    , transformer_(0)
    , rewriter_(0)
    , sema_(0)
{
}

WebCLValidatorAction::~WebCLValidatorAction()
{
    // consumer_ not deleted intentionally
    delete transformer_;
    transformer_ = 0;
    delete rewriter_;
    rewriter_ = 0;
    // sema_ not deleted intentionally
}

clang::ASTConsumer* WebCLValidatorAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    if (!initialize(instance))
        return NULL;
    return consumer_;
}

void WebCLValidatorAction::ExecuteAction()
{
    // We will get assertions if sema_ isn't wrapped here.
    llvm::OwningPtr<clang::Sema> sema(sema_);
    ParseAST(*sema.get());
}

bool WebCLValidatorAction::usesPreprocessorOnly() const
{
    return false;
}

bool WebCLValidatorAction::initialize(clang::CompilerInstance &instance)
{
    if (!WebCLAction::initialize(instance))
        return false;

    rewriter_ = new clang::Rewriter(
        instance.getSourceManager(), instance.getLangOpts());
    if (!rewriter_) {
        reporter_->fatal("Internal error. Can't create rewriter.\n");
        return false;
    }

    // Consumer must be allocated dynamically. The framework deletes
    // it.
    consumer_ = new WebCLConsumer(instance, *rewriter_);
    if (!consumer_) {
        reporter_->fatal("Internal error. Can't create AST consumer.\n");
        return false;
    }

    sema_ = new clang::Sema(
        instance.getPreprocessor(), instance.getASTContext(), *consumer_);
    if (!sema_) {
        reporter_->fatal("Internal error. Can't create semantic actions.\n");
        return false;
    }

    transformer_ = new WebCLTransformer(instance, *rewriter_);
    if (!transformer_) {
        reporter_->fatal("Internal error. Can't create AST transformer.\n");
        return false;
    }

    consumer_->setTransformer(*transformer_);
    return true;
}

WebCLActionFactory::WebCLActionFactory(const char *outputFile)
    : outputFile_(outputFile)
{
}

WebCLActionFactory::~WebCLActionFactory()
{
}

clang::FrontendAction *WebCLActionFactory::create()
{
    if (outputFile_)
        return new WebCLPreprocessorAction(outputFile_);
    return new WebCLValidatorAction;
}
