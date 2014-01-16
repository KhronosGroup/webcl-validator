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

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/OwningPtr.h"

namespace clang {
    class DiagnosticOptions;
    class LangOptions;
    class Preprocessor;
    class TextDiagnostic;
}

/// Captures Clang warning/error output
/// Partially based on Clang TextDiagnosticPrinter
class WebCLDiag : public clang::DiagnosticConsumer
{
public:

    WebCLDiag(clang::DiagnosticOptions *opts);
    ~WebCLDiag();

    void BeginSourceFile(const clang::LangOptions &LO, const clang::Preprocessor *PP);
    void EndSourceFile();
    void HandleDiagnostic(clang::DiagnosticsEngine::Level Level, const clang::Diagnostic &Info);

private:

    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> opts;
    clang::LangOptions langOpts;
    const clang::Preprocessor *pp;
};
