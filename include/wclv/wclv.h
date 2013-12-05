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

#ifndef WEBCL_VALIDATOR_WCLV_H
#define WEBCL_VALIDATOR_WCLV_H

#include <CL/cl.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _MSC_VER
#define WCLV_CALL __cdecl
#else
#define WCLV_CALL
#endif

// TODO: dll export / import magic once we go dynamic
#define WCLV_API

typedef struct WebCLValidator *wclv_program;

// Run validation
WCLV_API wclv_program WCLV_CALL wclvValidate(
    const char *input_source,
    const char **active_extensions,
    const char **user_defines,
    void (CL_CALLBACK *pfn_notify)(wclv_program program, void *user_data),
    void *notify_data,
    cl_int *errcode_ret);

typedef enum {
    /// Callback used, validation still running
    WCLV_PROGRAM_VALIDATING,
    /// Illegal code encountered
    WCLV_PROGRAM_ILLEGAL,
    /// Warnings from validation exist, but program can be run
    WCLV_PROGRAM_ACCEPTED_WITH_WARNINGS,
    /// Program validated cleanly
    WCLV_PROGRAM_ACCEPTED
} wclv_program_status;

// Determine outcome of validation
WCLV_API wclv_program_status WCLV_CALL wclvGetProgramStatus(
    wclv_program program);

// Get number of kernels found in program
WCLV_API cl_int WCLV_CALL wclvGetProgramKernelCount(
    wclv_program program);

// Get name of nth kernel in program
WCLV_API cl_int WCLV_CALL wclvGetProgramKernelName(
    wclv_program program,
    cl_uint n,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret);

// Get number of arguments for the nth kernel in the program
WCLV_API cl_int WCLV_CALL wclvGetKernelArgCount(
    wclv_program program,
    cl_uint n);

// Get name of a kernel argument in the program
WCLV_API cl_int WCLV_CALL wclvGetKernelArgName(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t name_buf_size,
    char *name_buf,
    size_t *name_size_ret);

// Get type of a kernel argument in the program
WCLV_API cl_int WCLV_CALL wclvGetKernelArgType(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg,
    size_t type_buf_size,
    char *type_buf,
    size_t *type_size_ret);

// Determine if the given kernel argument is a pointer
WCLV_API cl_bool WCLV_CALL wclvKernelArgIsPointer(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg);

// Return the address qualifier of the given pointer kernel argument
WCLV_API cl_kernel_arg_address_qualifier WCLV_CALL wclvGetKernelArgAddressQual(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg);

// Determine if the given kernel argument is an image
WCLV_API cl_bool WCLV_CALL wclvKernelArgIsImage(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg);

// Return the access qualifier of the given image kernel argument
WCLV_API cl_kernel_arg_access_qualifier WCLV_CALL wclvGetKernelArgAccessQual(
    wclv_program program,
    cl_uint kernel,
    cl_uint arg);

// Get validated source, ready to pass on to compiler
WCLV_API cl_int WCLV_CALL wclvProgramGetValidatedSource(
    wclv_program program,
    size_t source_buf_size,
    char *source_buf,
    size_t *source_size_ret);

// Release resources allocated by wclv_validate()
WCLV_API void WCLV_CALL wclvReleaseProgram(
    wclv_program program);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
