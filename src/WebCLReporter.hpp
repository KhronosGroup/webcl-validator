#ifndef WEBCLVALIDATOR_WEBCLREPORTER
#define WEBCLVALIDATOR_WEBCLREPORTER

#include "clang/Basic/Diagnostic.h"

namespace clang {
    class CompilerInstance;
}

/// \brief Mixin class for reporting errors.
class WebCLReporter
{
public:

    explicit WebCLReporter(clang::CompilerInstance &instance);
    ~WebCLReporter();

    /// Prints informational message.
    clang::DiagnosticBuilder info(
        clang::SourceLocation location, const char *format);
    /// Prints warning message.
    clang::DiagnosticBuilder warning(
        clang::SourceLocation location, const char *format);
    /// Prints error message.
    clang::DiagnosticBuilder error(
        clang::SourceLocation location, const char *format);

    /// Prints informational message that can't be associated with a
    /// source code location.
    clang::DiagnosticBuilder info(const char *format);
    /// Prints warning message that can't be associated with a source
    /// code location.
    clang::DiagnosticBuilder warning(const char *format);
    /// Prints error message that can't be associated with a source
    /// code location.
    clang::DiagnosticBuilder error(const char *format);
    /// Prints fatal error message that can't be associated with a
    /// source code location.
    clang::DiagnosticBuilder fatal(const char *format);

    /// \return Whether the location comes from user provided WebCL C
    /// source code. True is returned for all locations in the input
    /// file provided by the user. False is returned for headers
    /// included with the '-include' option.
    ///
    /// \see WebCLArguments
    bool isFromMainFile(clang::SourceLocation location) const;

protected:

    /// Provides access to diagnostics engine and source file manager.
    clang::CompilerInstance &instance_;

private:

    /// Helper for printing informational messages, warning, errors
    /// and fatal errors.
    clang::DiagnosticBuilder message(
        clang::DiagnosticsEngine::Level level, const char *format,
        clang::SourceLocation *location = 0);

};

#endif // WEBCLVALIDATOR_WEBCLREPORTER
