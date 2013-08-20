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

class WebCLTool : public clang::tooling::FrontendActionFactory
{
public:

    WebCLTool(int argc, char const **argv,
              char const *input, char const *output = NULL);
    virtual ~WebCLTool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create() = 0;

    int run();

protected:

    const clang::tooling::FixedCompilationDatabase *compilations_;
    std::vector<std::string> paths_;
    clang::tooling::ClangTool* tool_;
    const char *output_;
};

class WebCLPreprocessorTool : public WebCLTool
{
public:
    WebCLPreprocessorTool(int argc, char const **argv,
                          char const *input, char const *output);
    virtual ~WebCLPreprocessorTool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

class WebCLMatcher1Tool : public WebCLTool
{
public:
    WebCLMatcher1Tool(int argc, char const **argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher1Tool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

class WebCLMatcher2Tool : public WebCLTool
{
public:
    WebCLMatcher2Tool(int argc, char const **argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher2Tool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

class WebCLValidatorTool : public WebCLTool
{
public:
    WebCLValidatorTool(int argc, char const **argv,
                       char const *input);
    virtual ~WebCLValidatorTool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

#endif // WEBCLVALIDATOR_WEBCLTOOL
