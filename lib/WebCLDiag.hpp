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

#include <map>
#include <string>
#include <vector>

namespace clang {
    class FileID;
    class LangOptions;
    class Preprocessor;
}

/// Captures Clang warning/error output
class WebCLDiag : public clang::DiagnosticConsumer
{
public:

    WebCLDiag();
    ~WebCLDiag();

    void BeginSourceFile(const clang::LangOptions &LO, const clang::Preprocessor *PP);
    void EndSourceFile();
    void HandleDiagnostic(clang::DiagnosticsEngine::Level Level, const clang::Diagnostic &Info);

    struct Message {
        clang::DiagnosticsEngine::Level level;
        std::string text;

        std::string *source;
        std::string::size_type sourceOffset;
        std::string::size_type sourceLen;

        Message(clang::DiagnosticsEngine::Level level)
            : level(level)
            , source(NULL), sourceOffset(std::string::npos), sourceLen(std::string::npos)
        {
        }
    };

    std::vector<Message> messages;

private:

    bool collectSourceLocation(
        const clang::Diagnostic &info,
        WebCLDiag::Message &message);

    int nextSourceID;
    std::map<clang::FileID, int /* sourceID */> sourceIDs;
    std::map<int /* sourceID */, std::string> sources;
};

// A diagnostics capture that simply discards all messages
class WebCLDiagNull : public clang::DiagnosticConsumer
{
public:
    WebCLDiagNull();
    ~WebCLDiagNull();

    void BeginSourceFile(const clang::LangOptions &LO, const clang::Preprocessor *PP);
    void EndSourceFile();
    void HandleDiagnostic(clang::DiagnosticsEngine::Level Level, const clang::Diagnostic &Info);

private:
};
