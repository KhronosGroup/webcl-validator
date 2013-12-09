#ifndef WEBCLVALIDATOR_WEBCLMATCHER
#define WEBCLVALIDATOR_WEBCLMATCHER

/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include "WebCLRenamer.hpp"
#include "WebCLReporter.hpp"
#include "WebCLRewriter.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Refactoring.h"

class WebCLConfiguration;

namespace clang {
    class Rewriter;
    namespace ast_matchers {
        class MatchFinder;
    }
}

/// Creates a set of source code replacements by taking advantage of
/// AST matching features.
class WebCLMatcher : public WebCLReporter
{
public:

    WebCLMatcher(clang::CompilerInstance &instance);
    virtual ~WebCLMatcher();

    /// Register finder callbacks before AST has been parsed. The
    /// callbacks will be called when AST is being parsed.
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);
    /// \return Replacements after AST has been parsed.
    virtual clang::tooling::Replacements &complete();

    /// \return Currently added replacements. This should be called
    /// only from callbacks to add new replacements.
    clang::tooling::Replacements &getReplacements();

protected:

    /// Source code replacements.
    clang::tooling::Replacements replacements_;
};

/// Abstract base class for accepting AST matches. Delegates more
/// complex tasks, such as creation of source code replacements, back
/// to the corresponding AST matcher.
///
/// Matcher: Finds matches from the AST.
/// Matcher: Delegates found matches to Handler for acceptance.
/// Handler: Performs checks on matches that are difficult to do
///          otherwise.
/// Handler: Delegates accepted matches back to Matcher.
/// Matcher: Generates source code replacements for accepted matches.
template <typename Matcher>
class WebCLMatchHandler : public WebCLReporter
                        , public clang::ast_matchers::MatchFinder::MatchCallback
{
public:

    WebCLMatchHandler(clang::CompilerInstance &instance, Matcher &matcher);
    virtual ~WebCLMatchHandler();

protected:

    /// A component that matches AST nodes and generates replacements.
    Matcher &matcher_;
};

class WebCLNamelessStructHandler;

/// Generates a name for each anonymous and nameless structure.
///
/// struct { int field; }
/// ->
/// struct _WclStruct { int field; }
class WebCLNamelessStructRenamer : public WebCLMatcher
{
public:

    WebCLNamelessStructRenamer(
        clang::CompilerInstance &instance,
        WebCLConfiguration &cfg);
    virtual ~WebCLNamelessStructRenamer();

    /// \see WebCLMatcher
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);

    /// Identifies a structure definition in a match.
    const char *getNamelessStructBinding() const;

    /// Creates a replacement that generates name for the given
    /// structure definition.
    void handleNamelessStruct(const clang::RecordDecl *decl);

private:

    /// \return Location after 'struct' tag. The position where a
    /// generated name can be inserted.
    clang::SourceLocation getNameLoc(const clang::RecordDecl *decl) const;

    /// Interface for generating structure names.
    WebCLConfiguration &cfg_;
    /// Key for finding a structure definition from a match.
    const char *namelessStructBinding_;
    /// Selects anonymous and nameless structure definitions.
    clang::ast_matchers::DeclarationMatcher namelessStructMatcher_;
    /// Accepts selected structure definitions for further processing.
    WebCLNamelessStructHandler *namelessStructHandler_;
};

/// Checks whether a name should be generated for an anonymous or a
/// nameless structure. Delegates renaming back to the matcher object
/// that found the structure.
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

/// Separates structure definitions from variable declarations.
///
/// struct A { int field; } a;
/// ->
/// struct A { int field; }; struct A a;
class WebCLRenamedStructRelocator : public WebCLMatcher
{
public:

    WebCLRenamedStructRelocator(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    virtual ~WebCLRenamedStructRelocator();

    /// \see WebCLMatcher
    virtual void prepare(clang::ast_matchers::MatchFinder &finder);
    /// \see WebCLMatcher
    virtual clang::tooling::Replacements &complete();

    /// Identifies structure definitions that are enclosed inside a
    /// higher level structure definition.
    const char *getInnerStructBinding() const;
    /// Identifies structure definitions that aren't enclosed inside a
    /// higher level structure definition.
    const char *getOuterStructBinding() const;
    /// Identifies the closest location where a structure definition
    /// can be moved.
    const char *getContextBinding() const;

    /// Reverses completion order of structure definitions that need
    /// to be handled.
    ///
    /// struct A { struct B { struct C { int field; } c; } b; } a;
    /// ->
    /// Handling order: A, B, C.
    /// Completion order: C, B, A.
    void handleRenamedStruct(
        const clang::RecordDecl *decl, clang::SourceLocation context,
        const char *binding);

private:

    /// Generates replacement for each separated structure.
    ///
    /// struct A { struct B { int field; } b; } a;
    /// ->
    /// struct B { int field };
    /// struct A { struct B b; };
    /// struct A a;
    void completeRenamedStruct(
        const clang::RecordDecl *decl, clang::SourceLocation context,
        bool isOuter);

    /// Key for finding enclosed structure definitions from matches.
    const char *innerStructBinding_;
    /// Key for finding enclosing structure definitions from matches.
    const char *outerStructBinding_;
    /// Key for finding a target location for separated structure
    /// definitions.
    const char *contextBinding_;
    /// Selects enclosed structure definitions.
    clang::ast_matchers::DeclarationMatcher innerStructMatcher_;
    /// Selects enclosing structure definitions.
    clang::ast_matchers::DeclarationMatcher outerStructMatcher_;
    /// Accepts selected structures for further processing.
    WebCLRenamedStructHandler *renamedStructHandler_;
    /// Helper for converting nested transformations into shallow
    /// linear transformations.
    WebCLRewriter definitionRemoval_;

    /// Stack for reversing visiting and processing orders.
    typedef std::pair<const clang::RecordDecl*, clang::SourceLocation> RenamedStruct;
    typedef std::vector<RenamedStruct> RenamedStructs;
    /// Enclosed structures to be processed.
    RenamedStructs innerStructs_;
    /// Enclosing structures to be processed.
    RenamedStructs outerStructs_;

    /// Type definitions introduced before corresponding variable
    /// declarations.
    typedef std::map<clang::SourceLocation, std::string> Introductions;
    /// struct A { int field; } a;
    /// ->
    /// Introduction: struct A { int field; };
    /// Declaration: struct A a;
    Introductions introductions_;
};

/// Checks whether a structure definition is part of variable
/// declaration. Delegates separation of definition from declaration
/// back to the matcher object that found the definition.
class WebCLRenamedStructHandler : public WebCLMatchHandler<WebCLRenamedStructRelocator>
{
public:
    WebCLRenamedStructHandler(
        clang::CompilerInstance &instance, WebCLRenamedStructRelocator &matcher);
    virtual ~WebCLRenamedStructHandler();

    /// \see clang::ast_matchers::MatchFinder::MatchCallback
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);

    /// Accepts a structure definition match.
    void handle(const clang::ast_matchers::MatchFinder::MatchResult &result, 
                const char *structBinding, const char *contextBinding);

private:

    /// \return Enclosing declaration context for enclosed
    /// structure. Works around the limitations of parent traversal
    /// methods in clang::DeclContext.
    const clang::RecordDecl *getContext(
        const clang::RecordDecl *decl, const clang::DeclContext *context);

    /// \return New structure definition location in translation unit.
    ///
    /// __constant struct A { int field; } a;
    /// ^ <------- ^^^^^^^^^^^^^^^^^^^^^^^
    const clang::VarDecl *getContext(const clang::RecordDecl *decl);

    /// \return New structure definition location in functions.
    ///
    /// void function() { ... __private struct A { int field; } a; ... }
    ///                       ^ <------ ^^^^^^^^^^^^^^^^^^^^^^^
    clang::SourceLocation getContext(
        const clang::RecordDecl *decl, const clang::DeclStmt *context);
};

#endif // WEBCLVALIDATOR_WEBCLMATCHER
