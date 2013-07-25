#include "WebCLAction.hpp"
#include "WebCLArguments.hpp"
#include "WebCLDebug.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char const* argv[])
{
    if ((argc == 1) || ((argc > 2) && strcmp(argv[2], "--"))) {
        std::cerr << "Usage: " << argv[0] << " input.cl [-- clang-options]" << std::endl;
        return EXIT_FAILURE;
    }

    const WebCLArguments arguments(argc, argv);
    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();

    clang::tooling::CommonOptionsParser optionsParser(validatorArgc,
                                                      validatorArgv);
    clang::tooling::ClangTool webclTool(optionsParser.GetCompilations(),
                                        optionsParser.GetSourcePathList());

    clang::tooling::FrontendActionFactory *webCLActionFactory =
        clang::tooling::newFrontendActionFactory<WebCLAction>();
    const int status = webclTool.run(webCLActionFactory);
    delete webCLActionFactory;

    return status;
}
