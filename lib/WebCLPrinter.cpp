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

#include "WebCLPrinter.hpp"
#include "WebCLTransformer.hpp"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "llvm/Support/raw_ostream.h"

WebCLPrinter::WebCLPrinter(clang::Rewriter &rewriter)
    : rewriter_(rewriter)
{
}

WebCLPrinter::~WebCLPrinter()
{
}

bool WebCLPrinter::print(llvm::raw_ostream &out, const std::string &comment)
{
    // Insert a comment at the top of the main source file. This is to
    // ensure that at least some modifications are made so that
    // rewrite buffer becomes available.
    clang::SourceManager &manager = rewriter_.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    rewriter_.InsertText(start, comment, true, true);

    const clang::RewriteBuffer *buffer = rewriter_.getRewriteBufferFor(file);
    if (!buffer) {
        // You'll end up here if don't do any transformations.
        return false;
    }

    out << std::string(buffer->begin(), buffer->end());
    out.flush();
    return true;
}

WebCLValidatorPrinter::WebCLValidatorPrinter(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter,
    WebCLAnalyser &analyser, WebCLTransformer &transformer)
    : WebCLPrinter(rewriter)
    , WebCLPass(instance, analyser, transformer)
{
}

WebCLValidatorPrinter::~WebCLValidatorPrinter()
{
}

void WebCLValidatorPrinter::run(clang::ASTContext &context)
{
    // Apply transformer modifications.
    if (!transformer_.rewrite())
        return;

    output_.clear();
    llvm::raw_string_ostream os(output_);
    if (!print(os, "// WebCL Validator: validation stage.\n")) {
        fatal("Can't print validator output.");
        return;
    }
}
