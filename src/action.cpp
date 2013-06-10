#include "action.hpp"
#include "preprocessor.hpp"
#include "transformer.hpp"

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/Sema.h"

#include "llvm/ADT/OwningPtr.h"

WebCLAction::WebCLAction()
    : clang::FrontendAction()
    , reporter_(0)
    , preprocessor_(0)
    , consumer_(0)
    , transformer_(0)
    , sema_(0)
{
}

WebCLAction::~WebCLAction()
{
    delete reporter_;
    reporter_ = 0;
    // preprocessor_ not deleted intentionally
    // consumer_ not deleted intentionally
    delete transformer_;
    transformer_ = 0;
    // sema_ not deleted intentionally
}

clang::ASTConsumer* WebCLAction::CreateASTConsumer(clang::CompilerInstance &instance,
                                                   llvm::StringRef)
{
    if (!reporter_) {
        reporter_ = new WebCLReporter(instance);
        if (!reporter_) {
            llvm::errs() << "Internal error. Can't report errors.\n";
            return 0;
        }
    }

    const clang::FrontendInputFile &file = getCurrentInput();
    if (file.getKind() != clang::IK_OpenCL) {
        static const char *format =
            "Source file '%0' isn't treated as OpenCL code. Make sure that you "
            "give the '-x cl' option or that the file has a '.cl' extension.\n";
        reporter_->fatal(format) << file.getFile();
        return 0;
    }

    preprocessor_ = new WebCLPreprocessor(instance);
    if (!preprocessor_) {
        reporter_->fatal("Internal error. Can't create preprocessor callbacks.\n");
        return 0;
    }
    clang::Preprocessor &preprocessor = instance.getPreprocessor();
    preprocessor.addPPCallbacks(preprocessor_);

    // Consumer must be allocated dynamically. The framework deletes
    // it.
    consumer_ = new WebCLConsumer(instance);
    if (!consumer_) {
        reporter_->fatal("Internal error. Can't create AST consumer.\n");
        return 0;
    }

    sema_ = new clang::Sema(preprocessor, instance.getASTContext(), *consumer_);
    if (!sema_) {
        reporter_->fatal("Internal error. Can't create semantic actions.\n");
        return 0;
    }

    transformer_ = new WebCLTransformer(instance);
    if (!transformer_) {
        reporter_->fatal("Internal error. Can't create AST transformer.\n");
        return 0;
    }

    consumer_->setTransformer(*transformer_);
        
    return consumer_;
}

void WebCLAction::ExecuteAction()
{
    // We will get assertions if sema_ isn't wrapped here.
    llvm::OwningPtr<clang::Sema> sema(sema_);
    ParseAST(*sema.get());
}

bool WebCLAction::usesPreprocessorOnly() const
{
    return false;
}
