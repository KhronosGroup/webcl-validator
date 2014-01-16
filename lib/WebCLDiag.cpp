/*
** Copyright (c) 2014 The Khronos Group Inc.
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

#include "WebCLDiag.hpp"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"

#include "clang/Frontend/TextDiagnostic.h"

WebCLDiag::WebCLDiag(clang::DiagnosticOptions *opts)
    : opts(opts), pp(NULL)
{
}

WebCLDiag::~WebCLDiag()
{
}

void WebCLDiag::BeginSourceFile(const clang::LangOptions &langOpts, const clang::Preprocessor *pp)
{
    this->langOpts = langOpts;
    this->pp = pp;
}

void WebCLDiag::EndSourceFile()
{
    this->pp = NULL;
}

void WebCLDiag::HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info)
{
    // Updates the warning/error counts, which is what ultimately causes the tools to abort on error
    DiagnosticConsumer::HandleDiagnostic(level, info);

    /// TODO: don't print, but capture the details for exposing through the API
    /// vvv is adapted from clang::TextDiagnosticPrinter

    llvm::raw_ostream &OS = llvm::errs();

    // Render the diagnostic message into a temporary buffer eagerly. We'll use
    // this later as we print out the diagnostic to the terminal.
    llvm::SmallString<100> OutStr;
    info.FormatDiagnostic(OutStr);

    // Use a dedicated, simpler path for diagnostics without a valid location.
    // This is important as if the location is missing, we may be emitting
    // diagnostics in a context that lacks language options, a source manager, or
    // other infrastructure necessary when emitting more rich diagnostics.
    if (!info.getLocation().isValid()) {
        clang::TextDiagnostic::printDiagnosticLevel(OS, level, opts->ShowColors, opts->CLFallbackMode);
        clang::TextDiagnostic::printDiagnosticMessage(OS, level, OutStr,
            /* irrelevant as there is ... */ 0, /* ... no word wrap */ 0, opts->ShowColors);
        OS.flush();
        return;
    }

    clang::TextDiagnostic formatter(OS, langOpts, opts.getPtr());

    formatter.emitDiagnostic(info.getLocation(), level, OutStr,
        info.getRanges(), llvm::ArrayRef<clang::FixItHint>(),
        &info.getSourceManager());

    OS.flush();
}
