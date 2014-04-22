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

#ifndef WEBCL_VALIDATOR_CLV_H
#define WEBCL_VALIDATOR_CLV_H


#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _MSC_VER
#define CLV_CALL __cdecl
#else
#define CLV_CALL
#endif

#ifdef CREATE_CLV_STATIC
#define CLV_API 
#else
#ifdef CREATE_CLV_DLL_EXPORTS
#define CLV_API __declspec(dllexport)
#else
// This was not required after all #define CLV_API __declspec(dllimport)
#define CLV_API
#endif
#endif

typedef struct WebCLValidator *clv_program;

// Run validation
CLV_API clv_program CLV_CALL clvValidate(
    const char *input_source,
    const char **active_extensions,
    const char **user_defines,
    void (CL_CALLBACK *pfn_notify)(clv_program program, void *user_data),
    void *notify_data,
    cl_int *errcode_ret);

typedef enum {
    /// Callback used, validation still running
    CLV_PROGRAM_VALIDATING,
    /// Illegal code encountered
    CLV_PROGRAM_ILLEGAL,
    /// Warnings from validation exist, but program can be run
    CLV_PROGRAM_ACCEPTED_WITH_WARNINGS,
    /// Program validated cleanly
    CLV_PROGRAM_ACCEPTED
} clv_program_status;

// Determine outcome of validation
CLV_API clv_program_status CLV_CALL clvGetProgramStatus(
    clv_program program);

// Get number of validation log messages (notes, warnings, errors)
CLV_API cl_int CLV_CALL clvGetProgramLogMessageCount(
    clv_program program);

// Severity level of a validation log message
typedef enum {
    // Informative note
    CLV_LOG_MESSAGE_NOTE,
    // Warning
    CLV_LOG_MESSAGE_WARNING,
    // Error
    CLV_LOG_MESSAGE_ERROR
} clv_program_log_level;

// Get severity level of a validation log message
CLV_API clv_program_log_level CLV_CALL clvGetProgramLogMessageLevel(
    clv_program program,
    cl_uint n);

// Get text of a validation log message
CLV_API cl_int CLV_CALL clvGetProgramLogMessageText(
    clv_program program,
    cl_uint n,
    size_t buf_size,
    char *buf,
    size_t *size_ret);

// Determine if the given validation log message has associated source code
CLV_API cl_bool CLV_CALL clvProgramLogMessageHasSource(
    clv_program program,
    cl_uint n);

// Get the start offset of the relevant part of the source code
CLV_API cl_long CLV_CALL clvGetProgramLogMessageSourceOffset(
    clv_program program,
    cl_uint n);

// Get the length of relevant part of the source code
CLV_API size_t CLV_CALL clvGetProgramLogMessageSourceLen(
    clv_program program,
    cl_uint n);

// Get (a substring of) the source code associated with the log message
CLV_API cl_int CLV_CALL clvGetProgramLogMessageSourceText(
    clv_program program,
    cl_uint n,
    cl_long offset, // 0 for full source
    size_t len, // size_t(-1) for full source
    size_t buf_size,
    char *buf,
    size_t *size_ret);

// Get number of kernels found in program
CLV_API cl_int CLV_CALL clvGetProgramKernelCount(
    clv_program program);

// Get name of nth kernel in program
CLV_API cl_int CLV_CALL clvGetProgramKernelName(
    clv_program program,
    cl_uint n,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret);

// Get number of arguments for the nth kernel in the program
CLV_API cl_int CLV_CALL clvGetKernelArgCount(
    clv_program program,
    cl_uint n);

// Get name of a kernel argument in the program
CLV_API cl_int CLV_CALL clvGetKernelArgName(
    clv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret);

// Get type of a kernel argument in the program
CLV_API cl_int CLV_CALL clvGetKernelArgType(
    clv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t type_buf_size,
    char *type_buf,
    size_t *type_size_ret);

// Determine if the given kernel argument is a pointer
CLV_API cl_bool CLV_CALL clvKernelArgIsPointer(
    clv_program program,
    cl_uint kernel,
    cl_uint arg);

// Return the address qualifier of the given pointer kernel argument
CLV_API cl_kernel_arg_address_qualifier CLV_CALL clvGetKernelArgAddressQual(
    clv_program program,
    cl_uint kernel,
    cl_uint arg);

// Determine if the given kernel argument is an image
CLV_API cl_bool CLV_CALL clvKernelArgIsImage(
    clv_program program,
    cl_uint kernel,
    cl_uint arg);

// Return the access qualifier of the given image kernel argument
CLV_API cl_kernel_arg_access_qualifier CLV_CALL clvGetKernelArgAccessQual(
    clv_program program,
    cl_uint kernel,
    cl_uint arg);

// Get validated source, ready to pass on to compiler
CLV_API cl_int CLV_CALL clvGetProgramValidatedSource(
    clv_program program,
    size_t source_buf_size,
    char *source_buf,
    size_t *source_size_ret);

// Release resources allocated by clvValidate()
CLV_API void CLV_CALL clvReleaseProgram(
    clv_program program);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
