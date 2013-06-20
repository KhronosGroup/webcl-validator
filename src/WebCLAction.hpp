#ifndef WEBCLVALIDATOR_WEBCLACTION
#define WEBCLVALIDATOR_WEBCLACTION

#include "WebCLConsumer.hpp"

#include "clang/Frontend/FrontendAction.h"

namespace clang {
    class Sema;
}

class WebCLReporter;
class WebCLPreprocessor;
class WebCLTransformer;

class WebCLAction : public clang::FrontendAction
{
public:

    WebCLAction();
    virtual ~WebCLAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

private:

    WebCLReporter *reporter_;
    WebCLPreprocessor *preprocessor_;
    WebCLConsumer *consumer_;
    WebCLTransformer *transformer_;
    clang::Rewriter *rewriter_;
    clang::Sema *sema_;
};

#endif // WEBCLVALIDATOR_WEBCLACTION
