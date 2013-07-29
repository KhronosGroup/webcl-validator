#include "WebCLArguments.hpp"
#include "WebCLTool.hpp"

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

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

    int preprocessorArgc = arguments.getPreprocessorArgc();
    char const **preprocessorArgv = arguments.getPreprocessorArgv();
    char const *preprocessorInput = arguments.getPreprocessorInput();
    if (!preprocessorArgc || !preprocessorArgv || !preprocessorInput)
        return EXIT_FAILURE;

    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();
    char const *validatorInput = arguments.getValidatorInput();
    if (!validatorArgc || !validatorArgv || !validatorInput)
        return EXIT_FAILURE;

    WebCLTool preprocessorTool(preprocessorArgc, preprocessorArgv,
                               preprocessorInput, validatorInput);
    const int preprocessorStatus = preprocessorTool.run();
    if (preprocessorStatus)
        return preprocessorStatus;

    WebCLTool validatorTool(validatorArgc, validatorArgv,
                            validatorInput);
    const int validatorStatus = validatorTool.run();
    return validatorStatus;
}
