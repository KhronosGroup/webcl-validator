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

#include <utility>

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"

#include "clang/Basic/SourceManager.h"
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

    // There might be a new SourceManager, existing FileIDs are bound to get reused with different source
    sources.clear();
}

void WebCLDiag::EndSourceFile()
{
    this->pp = NULL;
}

namespace
{
    bool collectSourceLocation(
        const clang::Diagnostic &info,
        std::map<clang::FileID, std::shared_ptr<std::string> > &sources,
        WebCLDiag::Message &message)
    {
        clang::SourceLocation loc = info.getLocation();
        if (!loc.isValid())
            return false;

        clang::SourceManager &sm = info.getSourceManager();

        std::pair<clang::FileID, unsigned> filePosPair = sm.getDecomposedLoc(loc);
        const clang::FileID &file = filePosPair.first;
        const unsigned pos = filePosPair.second;

        if (sources.find(file) == sources.end()) {
            bool invalid = false;
            llvm::StringRef source = sm.getBufferData(file, &invalid);

            if (invalid)
                return false;

            sources[file] = std::shared_ptr<std::string>(new std::string(source));
        }

        message.source = sources[file];
        message.sourceOffset = pos - sm.getColumnNumber(file, pos) + 1;

        std::string::const_iterator endIter = message.source->begin() + pos;
        while (endIter != message.source->end() && *endIter != '\r' && *endIter != '\n')
            ++endIter;
        message.sourceLen = endIter - message.source->begin() - message.sourceOffset;

        return true;
    }
}

void WebCLDiag::HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info)
{
    // Updates the warning/error counts, which is what ultimately causes the tools to abort on error
    DiagnosticConsumer::HandleDiagnostic(level, info);

    Message message(level);
    llvm::raw_string_ostream os(message.text);

    llvm::SmallString<100> OutStr;
    info.FormatDiagnostic(OutStr);

    // TODO: omit level from text

    if (collectSourceLocation(info, this->sources, message)) {
        clang::TextDiagnostic formatter(os, langOpts, opts.getPtr());
        formatter.emitDiagnostic(info.getLocation(), level, OutStr,
            info.getRanges(), llvm::ArrayRef<clang::FixItHint>(),
            &info.getSourceManager());
    } else {
        clang::TextDiagnostic::printDiagnosticLevel(os, level, opts->ShowColors, opts->CLFallbackMode);
        clang::TextDiagnostic::printDiagnosticMessage(os, level, OutStr,
            /* irrelevant as there is ... */ 0, /* ... no word wrap */ 0, opts->ShowColors);
    }

    os.flush();
    messages.push_back(message);
}
