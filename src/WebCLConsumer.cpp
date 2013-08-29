#include "WebCLConsumer.hpp"
#include "WebCLHelper.hpp"
#include "WebCLPass.hpp"

#include "clang/AST/ASTContext.h"

#include "WebCLDebug.hpp"

WebCLConsumer::WebCLConsumer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter,
    WebCLTransformer &transformer)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , analyser_(instance)
    , visitors_()
    , inputNormaliser_(analyser_, transformer)
    , addressSpaceHandler_(analyser_, transformer)
    , kernelHandler_(analyser_, transformer, addressSpaceHandler_)
    , memoryAccessHandler_(analyser_, transformer, kernelHandler_)
    , printer_(instance, rewriter, analyser_, transformer)
    , passes_()
{
    visitors_.push_back(&restrictor_);
    visitors_.push_back(&analyser_);

    passes_.push_back(&inputNormaliser_);
    passes_.push_back(&addressSpaceHandler_);
    passes_.push_back(&kernelHandler_);
    passes_.push_back(&memoryAccessHandler_);
    passes_.push_back(&printer_);
}

WebCLConsumer::~WebCLConsumer()
{
}

void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    checkAndAnalyze(context);

    // Introduce changes in way that after every step we still have
    // correclty running program.
    
    // FUTURE: Add memory limit dependence analysis here (or maybe it
    //         could be even separate visitor). Also value range
    //         analysis could help in many cases. Based on dependence
    //         analysis, one could create separate implementations for
    //         different calls if we know which arguments are passed
    //         to function.

    transform(context);

    // FUTURE: Add class, which goes through builtins and creates
    //         corresponding safe calls and adds safe implementations
    //         to source.
}

void WebCLConsumer::checkAndAnalyze(clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();
    for (Visitors::iterator i = visitors_.begin(); i != visitors_.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLVisitor *visitor = (*i);
            visitor->TraverseDecl(decl);
        }
    }
}

void WebCLConsumer::transform(clang::ASTContext &context)
{
    for (Passes::iterator i = passes_.begin(); i != passes_.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLPass *pass = (*i);
            pass->run(context);
        }
    }
}

bool WebCLConsumer::hasErrors(clang::ASTContext &context) const
{
    clang::DiagnosticsEngine &diags = context.getDiagnostics();
    return diags.hasErrorOccurred() || diags.hasUnrecoverableErrorOccurred();
}
