#include "WebCLMatcher.hpp"
#include "WebCLTransformerConfiguration.hpp"

#include "clang/Frontend/CompilerInstance.h"

#include <cstring>

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
        WebCLTransformerConfiguration &cfg,
        clang::tooling::Replacements &replacements,
        clang::ast_matchers::MatchFinder &finder)
    : WebCLMatcher(instance, replacements, finder)
    , anonDeclBinding_("anon-decl")
    , anonDeclMatcher_(
        namedDecl(
            anyOf(
                hasName("<anonymous>"),
                matchesName("^::$"))).bind(anonDeclBinding_))
    , anonDeclHandler_(new WebCLAnonStructHandler(instance, cfg, *this))
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
    WebCLTransformerConfiguration &cfg,
    WebCLAnonStructMatcher &matcher)
    : WebCLMatchHandler<WebCLAnonStructMatcher>(instance, matcher)
    , cfg_(cfg)
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

    clang::SourceLocation loc = getNameLoc(decl);
    if (!isFromMainFile(loc))
        return;

    const std::string name = cfg_.getNameOfAnonymousStructure(decl);

    clang::tooling::Replacements &replacements = matcher_.getReplacements();
    clang::SourceManager &manager = instance_.getSourceManager();
    clang::tooling::Replacement replacement(
        manager, loc, 0, " " + name);
    replacements.insert(replacement);
}

clang::SourceLocation WebCLAnonStructHandler::getNameLoc(const clang::RecordDecl *decl) const
{
    clang::SourceLocation loc = decl->getLocStart();
    const char *kind = decl->getKindName();
    return loc.getLocWithOffset(strlen(kind));
}
