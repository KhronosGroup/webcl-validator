#include "WebCLPrinter.hpp"

#include "clang/Frontend/CompilerInstance.h"

WebCLPrinter::WebCLPrinter(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , WebCLTransformerClient()
    , clang::RecursiveASTVisitor<WebCLPrinter>()
    , rewriter_(instance.getSourceManager(), instance.getLangOpts())
{
}

WebCLPrinter::~WebCLPrinter()
{
}

bool WebCLPrinter::VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl)
{
    // Insert a comment at the top of the main source file. This is to
    // ensure that at least some modifications are made so that
    // rewrite buffer becomes available.
    clang::SourceManager &manager = rewriter_.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    const std::string comment = "// Transformed by WebCL Validator.\n\n";
    rewriter_.InsertText(start, comment, true, true);

    // Apply transformer modifications.
    WebCLTransformer &transformer = getTransformer();
    if (!transformer.rewrite(rewriter_))
        return false;

    return print();
}

bool WebCLPrinter::print()
{
    clang::SourceManager &manager = rewriter_.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    const clang::RewriteBuffer *buffer = rewriter_.getRewriteBufferFor(file);
    if (!buffer) {
        // You'll end up here if don't do any transformations.
        fatal("Can't get rewrite buffer.");
        return false;
    }

    llvm::outs() << std::string(buffer->begin(), buffer->end());
    return true;
}
