#ifndef WEBCLVALIDATOR_WEBCLACTION
#define WEBCLVALIDATOR_WEBCLACTION

#include "WebCLConsumer.hpp"

#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

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

protected:

    virtual bool initialize(clang::CompilerInstance &instance);

    WebCLReporter *reporter_;
    // Preprocessing callbacks are needed for both stages. Include
    // directives are handled during preprocessing and pragmas during
    // AST parsing.
    WebCLPreprocessor *preprocessor_;
};

class WebCLPreprocessorAction : public WebCLAction
{
public:

    explicit WebCLPreprocessorAction(const char *outputFile);
    virtual ~WebCLPreprocessorAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

private:

    const char *outputFile_;
};

class WebCLValidatorAction : public WebCLAction
{
public:

    WebCLValidatorAction();
    virtual ~WebCLValidatorAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

private:

    /// \see WebCLAction
    virtual bool initialize(clang::CompilerInstance &instance);

    WebCLConsumer *consumer_;
    WebCLTransformer *transformer_;
    clang::Rewriter *rewriter_;
    clang::Sema *sema_;
};

class WebCLActionFactory : public clang::tooling::FrontendActionFactory
{
public:

    WebCLActionFactory(const char *outputFile = NULL);
    virtual ~WebCLActionFactory();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();

private:

    const char *outputFile_;
};

#endif // WEBCLVALIDATOR_WEBCLACTION
