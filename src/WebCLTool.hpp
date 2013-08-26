#ifndef WEBCLVALIDATOR_WEBCLTOOL
#define WEBCLVALIDATOR_WEBCLTOOL

#include "clang/Tooling/Tooling.h"

#include <string>
#include <vector>

namespace clang {
    namespace tooling {
        class FixedCompilationDatabase;
    }
}

class WebCLActionFactory;

/// Abstract base class for tools representing various validation
/// stages. Each tool accepts a WebCL C program as its input and
/// produces a transformed WebCL C program as its output. The inputs
/// and outputs are chained to form a pipeline of tools.
///
/// Validating in multiple stages makes the transformations of each
/// stage less complex.
///
/// \see WebCLAction
/// \see WebCLArguments
class WebCLTool : public clang::tooling::FrontendActionFactory
{
public:

    /// Constructor. Inputs and outputs are filenames. If output isn't
    /// given, standard output is used.
    WebCLTool(int argc, char const **argv,
              char const *input, char const *output = NULL);
    virtual ~WebCLTool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create() = 0;

    /// Process input to produce transformed output.
    int run();

protected:

    /// Jobs to perform based on command line options.
    const clang::tooling::FixedCompilationDatabase *compilations_;
    /// Source file to process.
    std::vector<std::string> paths_;
    /// Tool representing a validation stage.
    clang::tooling::ClangTool* tool_;
    /// Target file for transformations.
    const char *output_;
};

/// Runs preprocessing stage. Takes the user source file as input.
///
/// \see WebCLPreprocessorAction
class WebCLPreprocessorTool : public WebCLTool
{
public:
    WebCLPreprocessorTool(int argc, char const **argv,
                          char const *input, char const *output);
    virtual ~WebCLPreprocessorTool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs first stage of AST matcher based transformations. Takes the
/// output of preprocessing stage as input.
///
/// \see WebCLMatcher1Action
class WebCLMatcher1Tool : public WebCLTool
{
public:
    WebCLMatcher1Tool(int argc, char const **argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher1Tool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs second stage of AST matcher based transformations. Takes the
/// output of previous AST matcher stage as input.
///
/// \see WebCLMatcher2Action
class WebCLMatcher2Tool : public WebCLTool
{
public:
    WebCLMatcher2Tool(int argc, char const **argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher2Tool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs memory access validation algorithm. Takes the output of last
/// AST matcher stage as input.
///
/// \see WebCLValidatorAction
class WebCLValidatorTool : public WebCLTool
{
public:
    WebCLValidatorTool(int argc, char const **argv,
                       char const *input);
    virtual ~WebCLValidatorTool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

#endif // WEBCLVALIDATOR_WEBCLTOOL
