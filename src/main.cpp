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
    int matcher1Argc = arguments.getMatcherArgc();
    char const **matcher1Argv = arguments.getMatcherArgv();
    char const *matcher1Input = arguments.getInput(matcher1Argc, matcher1Argv, true);
    if (!matcher1Argc || !matcher1Argv || !matcher1Input)
        return EXIT_FAILURE;
    int matcher2Argc = arguments.getMatcherArgc();
    char const **matcher2Argv = arguments.getMatcherArgv();
    char const *matcher2Input = arguments.getInput(matcher2Argc, matcher2Argv, true);
    if (!matcher2Argc || !matcher2Argv || !matcher2Input)
        return EXIT_FAILURE;

    // Create only one validator.
    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();
    char const *validatorInput = arguments.getInput(validatorArgc, validatorArgv);
    if (!validatorArgc || !validatorArgv)
        return EXIT_FAILURE;

    WebCLPreprocessorTool preprocessorTool(preprocessorArgc, preprocessorArgv,
                                           preprocessorInput, matcher1Input);
    const int preprocessorStatus = preprocessorTool.run();
    if (preprocessorStatus)
        return preprocessorStatus;

    WebCLMatcher1Tool matcher1Tool(matcher1Argc, matcher1Argv,
                                   matcher1Input, matcher2Input);
    const int matcher1Status = matcher1Tool.run();
    if (matcher1Status)
        return matcher1Status;
    WebCLMatcher2Tool matcher2Tool(matcher2Argc, matcher2Argv,
                                   matcher2Input, validatorInput);
    const int matcher2Status = matcher2Tool.run();
    if (matcher2Status)
        return matcher2Status;

    WebCLValidatorTool validatorTool(validatorArgc, validatorArgv,
                                     validatorInput);
    const int validatorStatus = validatorTool.run();
    return validatorStatus;
}
