#include "WebCLAction.hpp"
#include "WebCLArguments.hpp"
#include "WebCLDebug.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char const* argv[])
{
    std::set<std::string> help;
    help.insert("-h");
    help.insert("-help");
    help.insert("--help");

    if ((argc == 1) || ((argc == 2) && help.count(argv[1]))) {
        std::cerr << "Usage: " << argv[0] << " input.cl [clang-options]" << std::endl;
        return EXIT_FAILURE;
    }

    const WebCLArguments arguments(argc, argv);
    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();
    if (!validatorArgc || !validatorArgv)
        return EXIT_FAILURE;

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
