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
    return location.isValid() && instance_.getSourceManager().isFromMainFile(location);
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
