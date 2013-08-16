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

    WebCLArguments arguments(argc, argv);

    // Create only one preprocessor.
    int preprocessorArgc = arguments.getPreprocessorArgc();
    char const **preprocessorArgv = arguments.getPreprocessorArgv();
    char const *preprocessorInput = arguments.getInput(preprocessorArgc, preprocessorArgv, true);
    if (!preprocessorArgc || !preprocessorArgv || !preprocessorInput)
        return EXIT_FAILURE;

    // Create as many matchers as you like.
    int matcherArgc = arguments.getMatcherArgc();
    char const **matcherArgv = arguments.getMatcherArgv();
    char const *matcherInput = arguments.getInput(matcherArgc, matcherArgv, true);
    if (!matcherArgc || !matcherArgv || !matcherInput)
        return EXIT_FAILURE;

    // Create only one validator.
    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();
    char const *validatorInput = arguments.getInput(validatorArgc, validatorArgv);
    if (!validatorArgc || !validatorArgv)
        return EXIT_FAILURE;

    WebCLPreprocessorTool preprocessorTool(preprocessorArgc, preprocessorArgv,
                                           preprocessorInput, matcherInput);
    const int preprocessorStatus = preprocessorTool.run();
    if (preprocessorStatus)
        return preprocessorStatus;

    WebCLMatcherTool matcherTool(matcherArgc, matcherArgv,
                                 matcherInput, validatorInput);
    const int matcherStatus = matcherTool.run();
    if (matcherStatus)
        return matcherStatus;

    WebCLValidatorTool validatorTool(validatorArgc, validatorArgv,
                                     validatorInput);
    const int validatorStatus = validatorTool.run();
    return validatorStatus;
}
