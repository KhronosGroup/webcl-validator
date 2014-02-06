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

#include <clv/clv.h>

#include "WebCLTool.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "WebCLArguments.hpp"
#include "WebCLDiag.hpp"
#include "WebCLVisitor.hpp"

struct WebCLValidator
{
public:

    WebCLValidator(
        const std::string &inputSource,
        const std::set<std::string> &extensions,
        int argc,
        char const* argv[]);
    ~WebCLValidator();
    void run();
    int getExitStatus() const { return exitStatus_; }

    /// Returns validated source after a successful run
    const std::string &getValidatedSource() const { return validatedSource_; }
    /// Ditto for kernel info
    const WebCLAnalyser::KernelList &getKernels() const { return kernels_; }

    unsigned getNumWarnings() const { return diag->getNumWarnings(); }
    unsigned getNumErrors() const { return diag->getNumErrors(); }

    const std::vector<WebCLDiag::Message> &getLogMessages() const { return diag->messages; }

private:

    WebCLArguments arguments;
    WebCLDiag *diag;
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
    : arguments(inputSource, CharPtrVector(argv, argv + argc))
    , diag(new WebCLDiag())
    , extensions(extensions), exitStatus_(-1)
{
}

WebCLValidator::~WebCLValidator()
{
    delete diag;
}

void WebCLValidator::run()
{
    // Create only one preprocessor.
    CharPtrVector preprocessorArgv = arguments.getPreprocessorArgv();
    char const *preprocessorInput = arguments.getInput(preprocessorArgv);
    arguments.createOutput();
    if (!preprocessorArgv.size() || !preprocessorInput) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    CharPtrVector matcher1Argv = arguments.getMatcherArgv();
    char const *matcher1Input = arguments.getInput(matcher1Argv);
    arguments.createOutput();
    if (!matcher1Argv.size() || !matcher1Input) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    std::set<std::string> usedExtensions;
    // usedExtensions.insert("ALL");
    //WebCLMatcher1Tool findUsedExtensionsTool(matcher1Argv, matcher1Input, 0);
    CharPtrVector findUsedExtensionsArgv = arguments.getFindUsedExtensionsArgv();
    WebCLFindUsedExtensionsTool findUsedExtensionsTool(findUsedExtensionsArgv, preprocessorInput);
    WebCLDiagNull diagNull;
    findUsedExtensionsTool.setDiagnosticConsumer(&diagNull);
    findUsedExtensionsTool.setExtensions(extensions);
    findUsedExtensionsTool.setUsedExtensionsStorage(&usedExtensions);
    const int findUsedExtensionsStatus = findUsedExtensionsTool.run();
    if (findUsedExtensionsStatus) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    if (!arguments.supplyExtensionArguments(usedExtensions)) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    preprocessorArgv = arguments.getPreprocessorArgv(); // take with the new flags
    WebCLPreprocessorTool preprocessorTool(preprocessorArgv, preprocessorInput, matcher1Input);
    preprocessorTool.setDiagnosticConsumer(diag);
    preprocessorTool.setExtensions(extensions);
    const int preprocessorStatus = preprocessorTool.run();
    if (preprocessorStatus) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    // TODO: augment matcher/validator argv with -Dcl_khr_fp16 etc
    // based on which extension enable #pragmas have been encountered in preprocessing

    /*
     * Feed the builtin declarations produced by the preprocessing stage
     * to be included when running the later stages. This fixes issues
     * stemming from builtin functions being assumed to return int by default, etc.
     *
     * Not all builtin functions are declared, but only those which the preprocessing
     * stage detects as possibly having been called by the code being validated.
     *
     * TODO: profile, most importantly to see if:
     *
     *  1) expanding the _CL_DECLARE... macros takes a significant amount of
     *     time in the usual cases. In this case, we could capture the preprocessed
     *     version of the header produced during the first matcher stage and reuse it
     *     in the remaining matcher and validation stages.
     *
     * 2) parsing the literal function declarations produced by macro expansion
     *    is still expensive. The preprocessing stage can only narrow down the set
     *    of functions to declare by textual matching; this can provide false positives
     *    which we could eliminate by looking at the actual function calls in the AST
     *    once parsed.
     */
    if (!arguments.supplyBuiltinDecls(preprocessorTool.getBuiltinDecls())) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    matcher1Argv = arguments.getMatcherArgv();

    CharPtrVector matcher2Argv = arguments.getMatcherArgv();
    char const *matcher2Input = arguments.getInput(matcher2Argv);
    arguments.createOutput();
    if (!matcher2Argv.size() || !matcher2Input) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    // Create only one validator.
    CharPtrVector validatorArgv = arguments.getValidatorArgv();
    char const *validatorInput = arguments.getInput(validatorArgv);
    if (!validatorArgv.size()) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLMatcher1Tool matcher1Tool(matcher1Argv, matcher1Input, matcher2Input);
    matcher1Tool.setDiagnosticConsumer(diag);
    matcher1Tool.setExtensions(extensions);
    const int matcher1Status = matcher1Tool.run();
    if (matcher1Status) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLMatcher2Tool matcher2Tool(matcher2Argv, matcher2Input, validatorInput);
    matcher2Tool.setDiagnosticConsumer(diag);
    matcher2Tool.setExtensions(extensions);
    const int matcher2Status = matcher2Tool.run();
    if (matcher2Status) {
        exitStatus_ = EXIT_FAILURE;
        return;
    }

    WebCLValidatorTool validatorTool(validatorArgv, validatorInput);
    validatorTool.setDiagnosticConsumer(diag);
    validatorTool.setExtensions(extensions);
    const int validatorStatus = validatorTool.run();
    validatedSource_ = validatorTool.getValidatedSource();
    kernels_ = validatorTool.getKernels();
    exitStatus_ = validatorStatus;
}

CLV_API extern "C" clv_program CLV_CALL clvValidate(
    const char *input_source,
    const char **active_extensions,
    const char **user_defines,
    void (CL_CALLBACK *pfn_notify)(clv_program program, void *user_data),
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

CLV_API extern "C" clv_program_status CLV_CALL clvGetProgramStatus(
    clv_program program)
{
    // TODO: make consider callback not yet called

    if (program->getExitStatus() == EXIT_SUCCESS) {
        assert(program->getNumErrors() == 0);

        if (program->getNumWarnings() > 0)
            return CLV_PROGRAM_ACCEPTED_WITH_WARNINGS;
        else
            return CLV_PROGRAM_ACCEPTED;
    } else {
        return CLV_PROGRAM_ILLEGAL;
    }
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

CLV_API extern "C" cl_int CLV_CALL clvGetProgramLogMessageCount(
    clv_program program)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    return program->getLogMessages().size();
}

CLV_API extern "C" clv_program_log_level CLV_CALL clvGetProgramLogMessageLevel(
    clv_program program,
    cl_uint n)
{
    if (!program)
        return CLV_LOG_MESSAGE_ERROR;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CLV_LOG_MESSAGE_ERROR;

    assert(messages[n].level != clang::DiagnosticsEngine::Ignored);

    switch (messages[n].level) {
    case clang::DiagnosticsEngine::Note:
        return CLV_LOG_MESSAGE_NOTE;
    case clang::DiagnosticsEngine::Warning:
        return CLV_LOG_MESSAGE_WARNING;
    case clang::DiagnosticsEngine::Error:
    case clang::DiagnosticsEngine::Fatal:
    default:
        return CLV_LOG_MESSAGE_ERROR;
    }
}

CLV_API extern "C" cl_int CLV_CALL clvGetProgramLogMessageText(
    clv_program program,
    cl_uint n,
    size_t buf_size,
    char *buf,
    size_t *size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CL_INVALID_VALUE;

    return returnString(messages[n].text, buf_size, buf, size_ret);
}

CLV_API extern "C" cl_bool CLV_CALL clvProgramLogMessageHasSource(
    clv_program program,
    cl_uint n)
{
    if (!program)
        return CL_FALSE;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CL_FALSE;

    return messages[n].source != NULL;
}

CLV_API extern "C" cl_long CLV_CALL clvGetProgramLogMessageSourceOffset(
    clv_program program,
    cl_uint n)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CL_INVALID_VALUE;

    if (!clvProgramLogMessageHasSource(program, n))
        return CL_INVALID_OPERATION;

    return messages[n].sourceOffset;
}

CLV_API extern "C" size_t CLV_CALL clvGetProgramLogMessageSourceLen(
    clv_program program,
    cl_uint n)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CL_INVALID_VALUE;

    if (!clvProgramLogMessageHasSource(program, n))
        return CL_INVALID_OPERATION;

    return messages[n].sourceLen;
}

CLV_API extern "C" cl_int CLV_CALL clvGetProgramLogMessageSourceText(
    clv_program program,
    cl_uint n,
    cl_long offset,
    size_t len,
    size_t buf_size,
    char *buf,
    size_t *size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const std::vector<WebCLDiag::Message> &messages = program->getLogMessages();

    if (n >= messages.size())
        return CL_INVALID_VALUE;

    if (!clvProgramLogMessageHasSource(program, n))
        return CL_INVALID_OPERATION;

    std::string::size_type sourceSize = messages[n].source->size();
    std::string sourceText =
        messages[n].source->substr(offset > sourceSize ? sourceSize : offset, len);

    return returnString(sourceText, buf_size, buf, size_ret);
}

CLV_API extern "C" cl_int CLV_CALL clvGetProgramKernelCount(
    clv_program program)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    if (program->getExitStatus() != EXIT_SUCCESS)
        return 0;

    return program->getKernels().size();
}

CLV_API extern "C" cl_int CLV_CALL clvGetProgramKernelName(
    clv_program program,
    cl_uint n,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (n >= kernels.size())
        return CL_INVALID_VALUE;

    if (name_buf && !name_buf_size)
        return CL_INVALID_VALUE;

    return returnString(kernels[n].name, name_buf_size, name_buf, name_size_ret);
}

CLV_API extern "C" cl_int CLV_CALL clvGetKernelArgCount(
    clv_program program,
    cl_uint n)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (n >= kernels.size())
        return CL_INVALID_VALUE;

    return kernels[n].args.size();
}

CLV_API extern "C" cl_int CLV_CALL clvGetKernelArgName(
    clv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_INVALID_VALUE;

    if (arg >= kernels[kernel].args.size())
        return CL_INVALID_ARG_INDEX;

    return returnString(kernels[kernel].args[arg].name, name_buf_size, name_buf, name_size_ret);
}

CLV_API extern "C" cl_int CLV_CALL clvGetKernelArgType(
    clv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t type_buf_size,
    char *type_buf,
    size_t *type_size_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_INVALID_VALUE;

    if (arg >= kernels[kernel].args.size())
        return CL_INVALID_ARG_INDEX;

    return returnString(kernels[kernel].args[arg].reducedTypeName, type_buf_size, type_buf, type_size_ret);
}

CLV_API extern "C" cl_bool CLV_CALL clvKernelArgIsPointer(
    clv_program program,
    cl_uint kernel,
    cl_uint arg)
{
    if (!program)
        return CL_FALSE;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_FALSE;

    if (arg >= kernels[kernel].args.size())
        return CL_FALSE;

    return kernels[kernel].args[arg].pointerKind == WebCLTypes::NOT_POINTER ? CL_FALSE : CL_TRUE;
}

CLV_API extern "C" cl_kernel_arg_address_qualifier CLV_CALL clvGetKernelArgAddressQual(
    clv_program program,
    cl_uint kernel,
    cl_uint arg)
{
    if (!program)
        return CL_KERNEL_ARG_ADDRESS_PRIVATE;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_KERNEL_ARG_ADDRESS_PRIVATE;

    if (arg >= kernels[kernel].args.size())
        return CL_KERNEL_ARG_ADDRESS_PRIVATE;

    switch (kernels[kernel].args[arg].pointerKind) {
    default:
    case WebCLTypes::NOT_POINTER:
        return CL_KERNEL_ARG_ADDRESS_PRIVATE;
    case WebCLTypes::LOCAL_POINTER:
        return CL_KERNEL_ARG_ADDRESS_LOCAL;
    case WebCLTypes::CONSTANT_POINTER:
        return CL_KERNEL_ARG_ADDRESS_CONSTANT;
    case WebCLTypes::GLOBAL_POINTER:
        return CL_KERNEL_ARG_ADDRESS_GLOBAL;
    }
}

CLV_API extern "C" cl_bool CLV_CALL clvKernelArgIsImage(
    clv_program program,
    cl_uint kernel,
    cl_uint arg)
{
    if (!program)
        return CL_FALSE;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_FALSE;

    if (arg >= kernels[kernel].args.size())
        return CL_FALSE;

    return kernels[kernel].args[arg].imageKind == WebCLTypes::NOT_IMAGE ? CL_FALSE : CL_TRUE;
}

CLV_API extern "C" cl_kernel_arg_access_qualifier CLV_CALL clvGetKernelArgAccessQual(
    clv_program program,
    cl_uint kernel,
    cl_uint arg)
{
    if (!program)
        return CL_KERNEL_ARG_ACCESS_NONE;

    const WebCLAnalyser::KernelList &kernels = program->getKernels();

    if (kernel >= kernels.size())
        return CL_KERNEL_ARG_ACCESS_NONE;

    if (arg >= kernels[kernel].args.size())
        return CL_KERNEL_ARG_ACCESS_NONE;

    switch (kernels[kernel].args[arg].imageKind) {
    default:
    case WebCLTypes::NOT_IMAGE:
        return CL_KERNEL_ARG_ACCESS_NONE;
    case WebCLTypes::READABLE_IMAGE:
        return CL_KERNEL_ARG_ACCESS_READ_ONLY;
    case WebCLTypes::WRITABLE_IMAGE:
        return CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
    case WebCLTypes::RW_IMAGE:
        return CL_KERNEL_ARG_ACCESS_READ_WRITE;
    }
}

CLV_API cl_int CLV_CALL clvGetProgramValidatedSource(
    clv_program program,
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

CLV_API extern "C" void CLV_CALL clvReleaseProgram(
    clv_program program)
{
    delete program;
}
