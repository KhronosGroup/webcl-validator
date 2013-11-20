/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include <wclv/wclv.h>

#include "WebCLTool.hpp"

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

WebCLValidator::WebCLValidator(int argc, char const* argv[])
    : arguments(argc, argv)
{
}

int WebCLValidator::run()
{
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
