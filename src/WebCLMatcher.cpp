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

#include "WebCLMatcher.hpp"
#include "WebCLConfiguration.hpp"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <cstring>
#include <sstream>

using namespace clang::ast_matchers;

WebCLMatcher::WebCLMatcher(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
{
}

WebCLMatcher::~WebCLMatcher()
{
}

void WebCLMatcher::prepare(clang::ast_matchers::MatchFinder &finder)
{
}

clang::tooling::Replacements &WebCLMatcher::complete()
{
    return getReplacements();
}

clang::tooling::Replacements &WebCLMatcher::getReplacements()
{
    return replacements_;
}

template <typename Matcher>
WebCLMatchHandler<Matcher>::WebCLMatchHandler(
    clang::CompilerInstance &instance, Matcher &matcher)
    : WebCLReporter(instance)
    , clang::ast_matchers::MatchFinder::MatchCallback()
    , matcher_(matcher)
{
}

template <typename Matcher>
WebCLMatchHandler<Matcher>::~WebCLMatchHandler()
{
}

WebCLNamelessStructRenamer::WebCLNamelessStructRenamer(
    clang::CompilerInstance &instance,
    WebCLConfiguration &cfg)
    : WebCLMatcher(instance)
    , cfg_(cfg)
    , namelessStructBinding_("nameless-struct")
    , namelessStructMatcher_(
        namedDecl(
            anyOf(
                hasName("<anonymous>"),
                matchesName("^::$"))).bind(namelessStructBinding_))
    , namelessStructHandler_(new WebCLNamelessStructHandler(instance, *this))
{
}

WebCLNamelessStructRenamer::~WebCLNamelessStructRenamer()
{
    delete namelessStructHandler_;
    namelessStructHandler_ = NULL;
}

void WebCLNamelessStructRenamer::prepare(clang::ast_matchers::MatchFinder &finder)
{
    if (namelessStructHandler_)
        finder.addMatcher(namelessStructMatcher_, namelessStructHandler_);
    else
        fatal("Internal error. Can't create nameless structure handler.");
}

const char *WebCLNamelessStructRenamer::getNamelessStructBinding() const
{
    return namelessStructBinding_;
}

void WebCLNamelessStructRenamer::handleNamelessStruct(const clang::RecordDecl *decl)
{
    clang::SourceLocation loc = getNameLoc(decl);
    if (!isFromMainFile(loc))
        return;

    clang::tooling::Replacement replacement(
        instance_.getSourceManager(),
        loc, 0,
        " " + cfg_.getNameOfAnonymousStructure(decl));

    replacements_.insert(replacement);
}

clang::SourceLocation WebCLNamelessStructRenamer::getNameLoc(const clang::RecordDecl *decl) const
{
    clang::SourceLocation loc = decl->getLocStart();
    const char *kind = decl->getKindName();
    return loc.getLocWithOffset(strlen(kind));
}

WebCLNamelessStructHandler::WebCLNamelessStructHandler(
    clang::CompilerInstance &instance, WebCLNamelessStructRenamer &matcher)
    : WebCLMatchHandler<WebCLNamelessStructRenamer>(instance, matcher)
{
}

WebCLNamelessStructHandler::~WebCLNamelessStructHandler()
{
}

void WebCLNamelessStructHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    const char *namelessStructBinding = matcher_.getNamelessStructBinding();
    const clang::RecordDecl *decl =
        result.Nodes.getNodeAs<clang::RecordDecl>(namelessStructBinding);
    if (decl && decl->isCompleteDefinition())
        matcher_.handleNamelessStruct(decl);
}

WebCLRenamedStructRelocator::WebCLRenamedStructRelocator(
    clang::CompilerInstance &instance,
    clang::Rewriter &rewriter)
    : WebCLMatcher(instance)
    , innerStructBinding_("inner")
    , outerStructBinding_("outer")
    , contextBinding_("context")
    , innerStructMatcher_(
        namedDecl(
            hasParent(namedDecl()),
            anyOf(
                hasParent(decl()), // __constant
                hasAncestor(       // __private, __local
                    declStmt(
                        containsDeclaration(1, varDecl())
                        ).bind(contextBinding_)
                    )
                )
            ).bind(innerStructBinding_))
    , outerStructMatcher_(
        namedDecl(
            unless(hasParent(namedDecl())),
            anyOf(
                hasParent(decl()), // __constant
                hasParent(         // __private, __local
                    declStmt(
                        containsDeclaration(1, varDecl())
                        ).bind(contextBinding_)
                    )
                )
            ).bind(outerStructBinding_))
    , renamedStructHandler_(new WebCLRenamedStructHandler(instance, *this))
    , definitionRemoval_(instance, rewriter)
    , innerStructs_()
    , outerStructs_()
    , introductions_()
{
}

WebCLRenamedStructRelocator::~WebCLRenamedStructRelocator()
{
    delete renamedStructHandler_;
    renamedStructHandler_ = NULL;
}

void WebCLRenamedStructRelocator::prepare(clang::ast_matchers::MatchFinder &finder)
{
    if (!renamedStructHandler_) {
        fatal("Internal error. Can't create renamed structure handler.");
        return;
    }

    finder.addMatcher(innerStructMatcher_, renamedStructHandler_);
    finder.addMatcher(outerStructMatcher_, renamedStructHandler_);
}

clang::tooling::Replacements &WebCLRenamedStructRelocator::complete()
{
    size_t i = innerStructs_.size();
    while (i) {
        RenamedStruct &innerStruct = innerStructs_.at(--i);
        completeRenamedStruct(innerStruct.first, innerStruct.second, false);
    }

    i = outerStructs_.size();
    while (i) {
        RenamedStruct &outerStruct = outerStructs_.at(--i);
        completeRenamedStruct(outerStruct.first, outerStruct.second, true);
    }

    clang::SourceManager &manager = instance_.getSourceManager();
    for (Introductions::iterator it = introductions_.begin();
         it != introductions_.end(); ++it) {
        clang::tooling::Replacement replacement(
            manager, it->first, 0, it->second);
        replacements_.insert(replacement);
    }

    return WebCLMatcher::complete();
}

const char *WebCLRenamedStructRelocator::getInnerStructBinding() const
{
    return innerStructBinding_;
}

const char *WebCLRenamedStructRelocator::getOuterStructBinding() const
{
    return outerStructBinding_;
}

const char *WebCLRenamedStructRelocator::getContextBinding() const
{
    return contextBinding_;
}

void WebCLRenamedStructRelocator::handleRenamedStruct(
    const clang::RecordDecl *decl, clang::SourceLocation context, const char *binding)
{
    if (binding == innerStructBinding_)
        innerStructs_.push_back(RenamedStruct(decl, context));
    if (binding == outerStructBinding_)
        outerStructs_.push_back(RenamedStruct(decl, context));
}

void WebCLRenamedStructRelocator::completeRenamedStruct(
    const clang::RecordDecl *decl, clang::SourceLocation context, bool isOuter)
{
    clang::SourceRange range(decl->getSourceRange());
    if (!isFromMainFile(range.getBegin()))
        return;

    const std::string kind = decl->getKindName();
    // Partial range is needed to prevent context range from
    // overlapping with declaration range.
    clang::SourceRange partialRange(
        range.getBegin().getLocWithOffset(kind.size()),
        range.getEnd());

    const std::string partialDeclarationWithoutDefinition = 
        " " + decl->getName().str();
    const std::string declarationWithoutDefinition =
        kind + partialDeclarationWithoutDefinition;

    const std::string introduction =
        definitionRemoval_.getTransformedText(range);
    introductions_[context] += introduction + "; ";

    definitionRemoval_.replaceText(
        partialRange,
        partialDeclarationWithoutDefinition);

    if (isOuter) {
        clang::tooling::Replacement replacement(
            instance_.getSourceManager(),
            clang::CharSourceRange(partialRange, true),
            partialDeclarationWithoutDefinition);

        replacements_.insert(replacement);
    }
}

WebCLRenamedStructHandler::WebCLRenamedStructHandler(
    clang::CompilerInstance &instance, WebCLRenamedStructRelocator &matcher)
    : WebCLMatchHandler<WebCLRenamedStructRelocator>(instance, matcher)
{
}

WebCLRenamedStructHandler::~WebCLRenamedStructHandler()
{
}

void WebCLRenamedStructHandler::run(
    const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    handle(result, matcher_.getInnerStructBinding(), matcher_.getContextBinding());
    handle(result, matcher_.getOuterStructBinding(), matcher_.getContextBinding());
}

void WebCLRenamedStructHandler::handle(
    const clang::ast_matchers::MatchFinder::MatchResult &result, 
    const char *structBinding, const char *contextBinding)
{
    const clang::RecordDecl *decl =
        result.Nodes.getNodeAs<clang::RecordDecl>(structBinding);

    if (!decl || !decl->isCompleteDefinition())
        return;

    const clang::DeclStmt *context =
        result.Nodes.getNodeAs<clang::DeclStmt>(contextBinding);
    clang::SourceLocation location = getContext(decl, context);
    if (!location.isValid())
        return;

    matcher_.handleRenamedStruct(decl, location, structBinding);
}

const clang::RecordDecl *WebCLRenamedStructHandler::getContext(
    const clang::RecordDecl *decl, const clang::DeclContext *context)
{

    for (clang::DeclContext::decl_iterator i = context->decls_begin();
         i != context->decls_end(); ++i) {
        const clang::Decl *candidate = *i;
        if (candidate) {
            const clang::RecordDecl *record =
                llvm::dyn_cast<clang::RecordDecl>(candidate);
            if (record) {
                if (record == decl)
                    return record;
                if (getContext(decl, record))
                    return record;
            }
        }
    }

    return NULL;
}

const clang::VarDecl *WebCLRenamedStructHandler::getContext(
    const clang::RecordDecl *decl)
{
    const clang::RecordDecl *ancestor = getContext(decl, decl->getDeclContext());
    if (!ancestor)
        return NULL;

    const clang::Decl *next = ancestor->getNextDeclInContext();
    if (!next)
        return NULL;

    const clang::VarDecl *var = llvm::dyn_cast<clang::VarDecl>(next);
    if (!var)
        return NULL;

    if (decl->getLocStart() < var->getLocStart())
        return NULL;

    return var;
}

clang::SourceLocation WebCLRenamedStructHandler::getContext(
    const clang::RecordDecl *decl, const clang::DeclStmt *context)
{
    if (context)
        return context->getLocStart();

    const clang::VarDecl *alternative = getContext(decl);
    if (alternative)
        return alternative->getLocStart();

    return clang::SourceLocation();
}
