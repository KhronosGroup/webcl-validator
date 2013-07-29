#include "WebCLPrinter.hpp"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

WebCLPrinter::WebCLPrinter(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter)
    : WebCLTransformingVisitor(instance), rewriter_(rewriter)
{
}

WebCLPrinter::~WebCLPrinter()
{
}

bool WebCLPrinter::handleTranslationUnitDecl(clang::TranslationUnitDecl *decl)
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
    if (!transformer.rewrite())
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
    llvm::outs().flush();

    return true;
}
