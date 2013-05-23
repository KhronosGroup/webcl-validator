#include "reporter.hpp"

#include "clang/Frontend/CompilerInstance.h"

WebCLReporter::WebCLReporter(clang::CompilerInstance &instance)
    : instance_(instance)
{
}

WebCLReporter::~WebCLReporter()
{
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

clang::DiagnosticBuilder WebCLReporter::fatal(const char *format)
{
    return message(clang::DiagnosticsEngine::Fatal, format);
}

bool WebCLReporter::isFromMainFile(clang::SourceLocation location)
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
    } else {
        clang::SourceManager &manager = instance_.getSourceManager();
        sourceLocation = manager.getLocForEndOfFile(manager.getMainFileID());
    }

    clang::DiagnosticsEngine &diags = instance_.getDiagnostics();
    return diags.Report(sourceLocation, diags.getCustomDiagID(level, format));
}
