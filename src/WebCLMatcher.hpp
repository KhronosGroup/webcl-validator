#ifndef WEBCLVALIDATOR_WEBCLMATCHER
#define WEBCLVALIDATOR_WEBCLMATCHER

#include "WebCLRenamer.hpp"
#include "WebCLHelper.hpp"
#include "WebCLReporter.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Refactoring.h"

class WebCLTransformerConfiguration;

namespace clang {
    class Rewriter;
    namespace ast_matchers {
        class MatchFinder;
    }
}

class WebCLMatcher : public WebCLReporter
{
public:

    WebCLMatcher(clang::CompilerInstance &instance);
    virtual ~WebCLMatcher();

    // Register finder callbacks before AST has been parsed.
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);
    // \return Replacements after AST has been parsed.
    virtual clang::tooling::Replacements &complete();

    clang::tooling::Replacements &getReplacements();

protected:

    clang::tooling::Replacements replacements_;
};

template <typename Matcher>
class WebCLMatchHandler : public WebCLReporter
                        , public clang::ast_matchers::MatchFinder::MatchCallback
{
public:

    WebCLMatchHandler(clang::CompilerInstance &instance, Matcher &matcher);
    virtual ~WebCLMatchHandler();

protected:

    Matcher &matcher_;
};

class WebCLNamelessStructHandler;

class WebCLNamelessStructRenamer : public WebCLMatcher
{
public:

    WebCLNamelessStructRenamer(
        clang::CompilerInstance &instance,
        WebCLTransformerConfiguration &cfg);
    virtual ~WebCLNamelessStructRenamer();

    /// \see WebCLMatcher
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);

    const char *getNamelessStructBinding() const;
    void handleNamelessStruct(const clang::RecordDecl *decl);

private:

    clang::SourceLocation getNameLoc(const clang::RecordDecl *decl) const;

    WebCLTransformerConfiguration &cfg_;
    const char *namelessStructBinding_;
    clang::ast_matchers::DeclarationMatcher namelessStructMatcher_;
    WebCLNamelessStructHandler *namelessStructHandler_;
};

class WebCLNamelessStructHandler : public WebCLMatchHandler<WebCLNamelessStructRenamer>
{
public:
    WebCLNamelessStructHandler(
        clang::CompilerInstance &instance, WebCLNamelessStructRenamer &matcher);
    virtual ~WebCLNamelessStructHandler();

    /// \see clang::ast_matchers::MatchFinder::MatchCallback
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};

class WebCLRenamedStructHandler;

class WebCLRenamedStructRelocator : public WebCLMatcher
{
public:

    WebCLRenamedStructRelocator(
        clang::CompilerInstance &instance,
        clang::Rewriter &rewriter,
        WebCLTransformerConfiguration &cfg);
    virtual ~WebCLRenamedStructRelocator();

    /// \see WebCLMatcher
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);
    /// \see WebCLMatcher
    virtual clang::tooling::Replacements &complete();

    const char *getInnerStructBinding() const;
    const char *getOuterStructBinding() const;
    const char *getContextBinding() const;

    void handleRenamedStruct(
        const clang::RecordDecl *decl, clang::SourceLocation context,
        const char *binding);

private:

    void completeRenamedStruct(
        const clang::RecordDecl *decl, clang::SourceLocation context,
        bool isOuter);

    clang::Rewriter &rewriter_;
    WebCLTransformerConfiguration &cfg_;

    const char *innerStructBinding_;
    const char *outerStructBinding_;
    const char *contextBinding_;
    clang::ast_matchers::DeclarationMatcher innerStructMatcher_;
    clang::ast_matchers::DeclarationMatcher outerStructMatcher_;
    WebCLRenamedStructHandler *renamedStructHandler_;
    WebCLRewriter definitionRemoval_;

    // Reverse visiting order.
    typedef std::pair<const clang::RecordDecl*, clang::SourceLocation> RenamedStruct;
    typedef std::vector<RenamedStruct> RenamedStructs;
    RenamedStructs innerStructs_;
    RenamedStructs outerStructs_;

    typedef std::map<clang::SourceLocation, std::string> Introductions;
    Introductions introductions_;
};

class WebCLRenamedStructHandler : public WebCLMatchHandler<WebCLRenamedStructRelocator>
{
public:
    WebCLRenamedStructHandler(
        clang::CompilerInstance &instance, WebCLRenamedStructRelocator &matcher);
    virtual ~WebCLRenamedStructHandler();

    /// \see clang::ast_matchers::MatchFinder::MatchCallback
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);

    void handle(const clang::ast_matchers::MatchFinder::MatchResult &result, 
                const char *structBinding, const char *contextBinding);

private:

    const clang::RecordDecl *getContext(
        const clang::RecordDecl *decl, const clang::DeclContext *context);
    const clang::VarDecl *getContext(const clang::RecordDecl *decl);
    clang::SourceLocation getContext(
        const clang::RecordDecl *decl, const clang::DeclStmt *context);
};

#endif // WEBCLVALIDATOR_WEBCLMATCHER
