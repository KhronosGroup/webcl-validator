#ifndef WEBCLVALIDATOR_WEBCLMATCHER
#define WEBCLVALIDATOR_WEBCLMATCHER

#include "WebCLReporter.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Refactoring.h"

namespace clang {
    namespace ast_matchers {
        class MatchFinder;
    }
}

class WebCLMatcher : public WebCLReporter
{
public:

    WebCLMatcher(
        clang::CompilerInstance &instance,
        clang::tooling::Replacements &replacements,
        clang::ast_matchers::MatchFinder &finder);
    virtual ~WebCLMatcher();

    clang::tooling::Replacements &getReplacements();

protected:

    clang::ast_matchers::MatchFinder &finder_;
    clang::tooling::Replacements &replacements_;
};

template <typename Matcher>
class WebCLMatchHandler : public WebCLReporter
                        , public clang::ast_matchers::MatchFinder::MatchCallback
{
public:

    WebCLMatchHandler(
        clang::CompilerInstance &instance,
        Matcher &matcher);
    virtual ~WebCLMatchHandler();

protected:

    Matcher &matcher_;
};

class WebCLAnonStructHandler;

class WebCLAnonStructMatcher : public WebCLMatcher
{
public:

    WebCLAnonStructMatcher(
        clang::CompilerInstance &instance,
        clang::tooling::Replacements &replacements,
        clang::ast_matchers::MatchFinder &finder);
    virtual ~WebCLAnonStructMatcher();

    const char *getAnonDeclBinding() const;

private:

    const char *anonDeclBinding_;
    clang::ast_matchers::DeclarationMatcher anonDeclMatcher_;
    WebCLAnonStructHandler *anonDeclHandler_;
};

class WebCLAnonStructHandler : public WebCLMatchHandler<WebCLAnonStructMatcher>
{
public:

    WebCLAnonStructHandler(
        clang::CompilerInstance &instance,
        WebCLAnonStructMatcher &matcher);
    virtual ~WebCLAnonStructHandler();

    /// \see clang::ast_matchers::MatchFinder::MatchCallback
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};

#endif // WEBCLVALIDATOR_WEBCLMATCHER
