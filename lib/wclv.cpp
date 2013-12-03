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
#include <vector>

#include "../lib/WebCLArguments.hpp"
#include "../lib/WebCLVisitor.hpp"

// Proof of concept library API for the library+executable build system
struct WebCLValidator
{
public:

    WebCLValidator(
        const std::string &inputSource,
        const std::set<std::string> &extensions,
        int argc,
        char const* argv[]);
    void run();
    int getExitStatus() const { return exitStatus_; }

    /// Returns validated source after a successful run
    const std::string &getValidatedSource() const { return validatedSource_; }
    /// Ditto for kernel info
    const WebCLAnalyser::KernelList &getKernels() const { return kernels_; }

private:

    WebCLArguments arguments;
    std::set<std::string> extensions;

    // Exit status for run()
    int exitStatus_;
    // Stores validated source after validation is complete.
    std::string validatedSource_;
    /// Ditto for kernel info
    WebCLAnalyser::KernelList kernels_;
};

WebCLValidator::WebCLValidator(
    const std::string &inputSource,
    const std::set<std::string> &extensions,
    int argc,
    char const* argv[])
    : arguments(inputSource, argc, argv), extensions(extensions), exitStatus_(-1)
{
}

void WebCLValidator::run()
{
    // Create only one preprocessor.
    int preprocessorArgc = arguments.getPreprocessorArgc();
    char const **preprocessorArgv = arguments.getPreprocessorArgv();
    char const *preprocessorInput = arguments.getInput(preprocessorArgc, preprocessorArgv, true);
    if (!preprocessorArgc || !preprocessorArgv || !preprocessorInput) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    // Create as many matchers as you like.
    int matcher1Argc = arguments.getMatcherArgc();
    char const **matcher1Argv = arguments.getMatcherArgv();
    char const *matcher1Input = arguments.getInput(matcher1Argc, matcher1Argv, true);
    if (!matcher1Argc || !matcher1Argv || !matcher1Input) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    int matcher2Argc = arguments.getMatcherArgc();
    char const **matcher2Argv = arguments.getMatcherArgv();
    char const *matcher2Input = arguments.getInput(matcher2Argc, matcher2Argv, true);
    if (!matcher2Argc || !matcher2Argv || !matcher2Input) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    // Create only one validator.
    int validatorArgc = arguments.getValidatorArgc();
    char const **validatorArgv = arguments.getValidatorArgv();
    char const *validatorInput = arguments.getInput(validatorArgc, validatorArgv);
    if (!validatorArgc || !validatorArgv) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLPreprocessorTool preprocessorTool(preprocessorArgc, preprocessorArgv,
                                           preprocessorInput, matcher1Input);
    preprocessorTool.setExtensions(extensions);
    const int preprocessorStatus = preprocessorTool.run();
    if (preprocessorStatus) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLMatcher1Tool matcher1Tool(matcher1Argc, matcher1Argv,
                                   matcher1Input, matcher2Input);
    matcher1Tool.setExtensions(extensions);
    const int matcher1Status = matcher1Tool.run();
    if (matcher1Status) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLMatcher2Tool matcher2Tool(matcher2Argc, matcher2Argv,
                                   matcher2Input, validatorInput);
    matcher2Tool.setExtensions(extensions);
    const int matcher2Status = matcher2Tool.run();
    if (matcher2Status) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLValidatorTool validatorTool(validatorArgc, validatorArgv,
                                     validatorInput);
    validatorTool.setExtensions(extensions);
    const int validatorStatus = validatorTool.run();
    validatedSource_ = validatorTool.getValidatedSource();
    kernels_ = validatorTool.getKernels();
    exitStatus_ = validatorStatus;
}

WCLV_API extern "C" wclv_program WCLV_CALL wclvValidate(
    const char *input_source,
    const char **active_extensions,
    const char **user_defines,
    void (CL_CALLBACK *pfn_notify)(wclv_program program, void *user_data),
    void *notify_data,
    cl_int *errcode_ret)
{
    if (!input_source || !*input_source) {
        if (errcode_ret)
            *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }

    std::set<std::string> extensions;
    while (active_extensions && *active_extensions)
        extensions.insert(*(active_extensions++));

    std::set<std::string> defineArgs;
    while (user_defines && *user_defines)
        defineArgs.insert(std::string("-D") + *(user_defines++));

    std::vector<const char *> argv;
    for (std::set<std::string>::const_iterator i = defineArgs.begin(); i != defineArgs.end(); ++i)
        argv.push_back(i->c_str());

    WebCLValidator *validator = new WebCLValidator(input_source, extensions, argv.size(), argv.empty() ? NULL : &argv[0]);

    // TODO: run in thread, call user callback when done if provided
    validator->run();

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;

    return validator;
}

WCLV_API extern "C" wclv_program_status WCLV_CALL wclvGetProgramStatus(
    wclv_program program)
{
    // TODO: make consider callback not yet called, errors, warnings
    return program->getExitStatus() == EXIT_SUCCESS ? WCLV_PROGRAM_ACCEPTED : WCLV_PROGRAM_ILLEGAL;
}

namespace
{
    cl_int returnString(const std::string &ret, size_t ret_buf_size, char *ret_buf, size_t *size_ret)
    {
        if (ret_buf) {
            if (ret_buf_size <= ret.size())
                return CL_INVALID_VALUE;

            ret.copy(ret_buf, ret.size());
            ret_buf[ret.size()] = '\0';
        }

        if (size_ret)
            *size_ret = ret.size() + 1;

        return CL_SUCCESS;
    }
}

WCLV_API cl_int WCLV_CALL wclvProgramGetValidatedSource(
    wclv_program program,
    size_t source_buf_size,
    char *source_buf,
    size_t *source_size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    if (source_buf && !source_buf_size)
        return CL_INVALID_VALUE;

    std::string source;
    if (program->getExitStatus() == EXIT_SUCCESS)
        source = program->getValidatedSource();

    return returnString(source, source_buf_size, source_buf, source_size_ret);
}

WCLV_API extern "C" void WCLV_CALL wclvReleaseProgram(
    wclv_program program)
{
    delete program;
}