#include "consumer.hpp"

#include "clang/AST/ASTContext.h"

WebCLConsumer::WebCLConsumer(clang::CompilerInstance &instance)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , parameterizer_(instance)
    , accessor_(instance)
    , printer_(instance)
{
}

WebCLConsumer::~WebCLConsumer()
{
}

void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();
    // There is no point to continue if an error has been reported.
    if (!hasErrors(context))
        restrictor_.TraverseDecl(decl);
    if (!hasErrors(context))
        parameterizer_.TraverseDecl(decl);
    if (!hasErrors(context))
        accessor_.TraverseDecl(decl);
    if (!hasErrors(context))
        printer_.TraverseDecl(decl);
}

void WebCLConsumer::setTransformer(WebCLTransformer &transformer)
{
    parameterizer_.setTransformer(transformer);
    accessor_.setTransformer(transformer);
    printer_.setTransformer(transformer);
}

bool WebCLConsumer::hasErrors(clang::ASTContext &context) const
{
   clang::DiagnosticsEngine &diags = context.getDiagnostics();
   return diags.hasErrorOccurred() || diags.hasUnrecoverableErrorOccurred();
}
