#ifndef WEBCLVALIDATOR_WEBCLACTION
#define WEBCLVALIDATOR_WEBCLACTION

#include "WebCLConsumer.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"
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

    explicit WebCLAction(const char *output = NULL);
    virtual ~WebCLAction();

protected:

    virtual bool initialize(clang::CompilerInstance &instance);

    WebCLReporter *reporter_;
    // Preprocessing callbacks are needed for both stages. Include
    // directives are handled during preprocessing and pragmas during
    // AST parsing.
    WebCLPreprocessor *preprocessor_;

    const char *output_;
    llvm::raw_ostream *out_;
};

class WebCLPreprocessorAction : public WebCLAction
{
public:

    explicit WebCLPreprocessorAction(const char *output);
    virtual ~WebCLPreprocessorAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;
};

class WebCLMatcherAction : public WebCLAction
{
public:

    explicit WebCLMatcherAction(const char *output);
    virtual ~WebCLMatcherAction();

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

    clang::ast_matchers::MatchFinder finder_;
    clang::tooling::Replacements replacements_;
    clang::ASTConsumer *consumer_;
    clang::Rewriter *rewriter_;

    WebCLPrinter *printer_;
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

#endif // WEBCLVALIDATOR_WEBCLACTION
