#include "WebCLConsumer.hpp"
#include "WebCLHelper.hpp"
#include "WebCLPass.hpp"

#include "clang/AST/ASTContext.h"

#include "WebCLDebug.hpp"

WebCLConsumer::WebCLConsumer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter,
    WebCLTransformer &transformer)
    : clang::ASTConsumer()
    , transformer_(transformer)
    , restrictor_(instance)
    , analyser_(instance)
    , checkingVisitors_()
    , inputNormaliser_(analyser_, transformer)
    , addressSpaceHandler_(analyser_, transformer)
    , kernelHandler_(analyser_, transformer, addressSpaceHandler_)
    , memoryAccessHandler_(analyser_, transformer, kernelHandler_)
    , printer_(instance, rewriter, analyser_, transformer)
    , transformingPasses_()
{
    // Push in reverse order.
    checkingVisitors_.push_front(&analyser_);
    checkingVisitors_.push_front(&restrictor_);
    // Push in reverse order.
    transformingPasses_.push_front(&printer_);
    transformingPasses_.push_front(&memoryAccessHandler_);
    transformingPasses_.push_front(&kernelHandler_);
    transformingPasses_.push_front(&addressSpaceHandler_);
    transformingPasses_.push_front(&inputNormaliser_);
 }

WebCLConsumer::~WebCLConsumer()
{
}

void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    traverse(checkingVisitors_, context);

    // Introduce changes in way that after every step we still have
    // correclty running program.

    // FUTURE: Add memory limit dependence analysis here (or maybe it
    // could be even separate visitor). Also value range analysis
    // could help in many cases. Based on dependence analysis, one
    // could create separate implementations for different calls if we
    // know which arguments are passed to function.

    traverse(transformingPasses_, context);

    // FUTURE: Add class, which goes through builtins and creates
    //         corresponding safe calls and adds safe implementations
    //         to source.
}

void WebCLConsumer::traverse(Visitors &sequence, clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();

    for (Visitors::iterator i = sequence.begin(); i != sequence.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLVisitor *visitor = (*i);
            visitor->TraverseDecl(decl);
        }
    }
}

void WebCLConsumer::traverse(Passes &sequence, clang::ASTContext &context)
{
    for (Passes::iterator i = sequence.begin(); i != sequence.end(); ++i) {
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
