#include "WebCLMatcher.hpp"

#include "clang/Frontend/CompilerInstance.h"

using namespace clang::ast_matchers;

WebCLMatcher::WebCLMatcher(
    clang::CompilerInstance &instance,
    clang::tooling::Replacements &replacements,
    clang::ast_matchers::MatchFinder &finder)
    : WebCLReporter(instance)
    , finder_(finder), replacements_(replacements)
{
}

WebCLMatcher::~WebCLMatcher()
{
}

clang::tooling::Replacements &WebCLMatcher::getReplacements()
{
    return replacements_;
}

template <typename Matcher>
WebCLMatchHandler<Matcher>::WebCLMatchHandler(
    clang::CompilerInstance &instance,
    Matcher &matcher)
    : WebCLReporter(instance)
    , clang::ast_matchers::MatchFinder::MatchCallback()
    , matcher_(matcher)
{
}

template <typename Matcher>
WebCLMatchHandler<Matcher>::~WebCLMatchHandler()
{
}

WebCLAnonStructMatcher::WebCLAnonStructMatcher(
        clang::CompilerInstance &instance,
        clang::tooling::Replacements &replacements,
        clang::ast_matchers::MatchFinder &finder)
    : WebCLMatcher(instance, replacements, finder)
    , anonDeclBinding_("anon-decl")
    , anonDeclMatcher_(
        namedDecl(
            hasName("<anonymous>")).bind(anonDeclBinding_))
    , anonDeclHandler_(new WebCLAnonStructHandler(instance, *this))
{
    if (anonDeclHandler_)
        finder.addMatcher(anonDeclMatcher_, anonDeclHandler_);
    else
        fatal("Internal error. Can't create anonymous structure handler.");
}

WebCLAnonStructMatcher::~WebCLAnonStructMatcher()
{
}

const char *WebCLAnonStructMatcher::getAnonDeclBinding() const
{
    return anonDeclBinding_;
}

WebCLAnonStructHandler::WebCLAnonStructHandler(
    clang::CompilerInstance &instance,
    WebCLAnonStructMatcher &matcher)
    : WebCLMatchHandler<WebCLAnonStructMatcher>(instance, matcher)
{
}

WebCLAnonStructHandler::~WebCLAnonStructHandler()
{
}

void WebCLAnonStructHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    const char *anonDeclBinding = matcher_.getAnonDeclBinding();
    const clang::RecordDecl *decl =
        result.Nodes.getNodeAs<clang::RecordDecl>(anonDeclBinding);
    if (!decl)
        return;

    clang::SourceLocation loc = decl->getLocStart();
    if (!isFromMainFile(loc))
        return;

    clang::tooling::Replacements &replacements = matcher_.getReplacements();
    clang::SourceManager &manager = instance_.getSourceManager();
    clang::tooling::Replacement replacement(
        manager, loc, 0, " /* anonymous struct */ ");
    replacements.insert(replacement);
}
