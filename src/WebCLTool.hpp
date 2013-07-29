#ifndef WEBCLVALIDATOR_WEBCLTOOL
#define WEBCLVALIDATOR_WEBCLTOOL

#include <string>
#include <vector>

namespace clang {
    namespace tooling {
        class ClangTool;
        class FixedCompilationDatabase;
    }
}

class WebCLActionFactory;

class WebCLTool
{
public:

    WebCLTool(int argc, char const **argv,
              char const *input, char const *output = NULL);
    ~WebCLTool();

    int run();

private:

    const clang::tooling::FixedCompilationDatabase *compilations_;
    std::vector<std::string> paths_;
    clang::tooling::ClangTool* tool_;
    WebCLActionFactory* factory_;
};

#endif // WEBCLVALIDATOR_WEBCLTOOL
