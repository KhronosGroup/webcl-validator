#include "WebCLConsumer.hpp"

#include "clang/AST/ASTContext.h"

WebCLConsumer::WebCLConsumer(clang::CompilerInstance &instance)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , checkingVisitors_()
    , relocator_(instance)
    , parameterizer_(instance)
    , accessor_(instance)
    , printer_(instance)
    , transformingVisitors_()
{
    // Push in reverse order.
    checkingVisitors_.push_front(&restrictor_);
    // Push in reverse order.
    transformingVisitors_.push_front(&printer_);
    transformingVisitors_.push_front(&accessor_);
    transformingVisitors_.push_front(&parameterizer_);
    transformingVisitors_.push_front(&relocator_);
}

WebCLConsumer::~WebCLConsumer()
{
}

void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    traverse(checkingVisitors_, context);
    traverse(transformingVisitors_, context);
}

void WebCLConsumer::setTransformer(WebCLTransformer &transformer)
{
    for (TransformingVisitors::iterator i = transformingVisitors_.begin();
         i != transformingVisitors_.end(); ++i) {
        WebCLTransformingVisitor *visitor = (*i);
        visitor->setTransformer(transformer);
    }
}

template <typename VisitorSequence>
void WebCLConsumer::traverse(VisitorSequence &sequence, clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();

    for (typename VisitorSequence::iterator i = sequence.begin();
         i != sequence.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLVisitor *visitor = (*i);
            visitor->TraverseDecl(decl);
        }
    }
}

bool WebCLConsumer::hasErrors(clang::ASTContext &context) const
{
   clang::DiagnosticsEngine &diags = context.getDiagnostics();
   return diags.hasErrorOccurred() || diags.hasUnrecoverableErrorOccurred();
}
