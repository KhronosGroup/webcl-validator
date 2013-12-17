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

#include "WebCLReporter.hpp"

#include "clang/Frontend/CompilerInstance.h"

WebCLReporter::WebCLReporter(clang::CompilerInstance &instance)
    : instance_(instance)
{
}

WebCLReporter::~WebCLReporter()
{
}

clang::DiagnosticBuilder WebCLReporter::info(clang::SourceLocation location, const char *format)
{
  // NOTE: set this from
  //       clang::DiagnosticsEngine::Note to hide.
  //       clang::DiagnosticsEngine::Warning to see
  //       output where WebCLAnalyser has found stuff.
  return message(clang::DiagnosticsEngine::Note, format, &location);
}

clang::DiagnosticBuilder WebCLReporter::warning(
    clang::SourceLocation location, const char *format)
{
    return message(clang::DiagnosticsEngine::Warning, format, &location);
}

clang::DiagnosticBuilder WebCLReporter::error(
    clang::SourceLocation location, const char *format)
{
    return message(clang::DiagnosticsEngine::Error, format, &location);
}

clang::DiagnosticBuilder WebCLReporter::info(const char *format)
{
    return message(clang::DiagnosticsEngine::Note, format);
}

clang::DiagnosticBuilder WebCLReporter::warning(const char *format)
{
    return message(clang::DiagnosticsEngine::Warning, format);
}

clang::DiagnosticBuilder WebCLReporter::error(const char *format)
{
    return message(clang::DiagnosticsEngine::Error, format);
}

clang::DiagnosticBuilder WebCLReporter::fatal(const char *format)
{
    return message(clang::DiagnosticsEngine::Fatal, format);
}

bool WebCLReporter::isFromMainFile(clang::SourceLocation location) const
{
    return location.isValid() && instance_.getSourceManager().isWrittenInMainFile(location);
}

clang::DiagnosticBuilder WebCLReporter::message(
    clang::DiagnosticsEngine::Level level, const char *format,
    clang::SourceLocation *location)
{
    clang::SourceLocation sourceLocation;
    if (location) {
        sourceLocation = *location;
    }

    clang::DiagnosticsEngine &diags = instance_.getDiagnostics();
    return diags.Report(sourceLocation, diags.getCustomDiagID(level, format));
}
