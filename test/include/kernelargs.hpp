#ifndef KERNELARGS_HPP
#define KERNELARGS_HPP

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

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>

/**
 * Helper for setting kernel arguments.
 *
 * Provides automatic indexing and can generate size arguments
 * required by WebCL kernels.
 */
class KernelArgs
{
public:
    KernelArgs(cl_kernel kernel, bool genArraySizeArgs) :
        kernel_(kernel),
        argi_(0),
        genArraySizeArgs_(genArraySizeArgs)
    {
    }

    template <typename T>
    void appendLocalArray(cl_ulong arrayLength)
    {
        appendArray(sizeof(T) * arrayLength, NULL, arrayLength);
    }

    template <typename T>
    void appendArray(const T *arg_value, cl_ulong arrayLength)
    {
        appendArray(sizeof(T), static_cast<const void *>(arg_value), arrayLength);
    }

    void appendArray(size_t argsize, const void *argval, cl_ulong arrayLength)
    {
        cl_int ret = clSetKernelArg(kernel_, argi_++, argsize, static_cast<const void *>(argval));
        if (ret != CL_SUCCESS)
            std::cerr << "clSetKernelArg failed for arg " << (argi_ - 1) << std::endl;

        if (genArraySizeArgs_)
        {
            ret = clSetKernelArg(kernel_, argi_++, sizeof(arrayLength), &arrayLength);
            if (ret != CL_SUCCESS)
                std::cerr << "clSetKernelArg failed for array length parameter" << std::endl;
        }
    }

    void appendImage(cl_mem image) {
        cl_int ret = clSetKernelArg(kernel_, argi_++, sizeof(image), &image);
        if (ret != CL_SUCCESS)
            std::cerr << "clSetKernelArg failed for cl_mem (image)." << std::endl;
    }

    void appendInt(cl_int val) {
        cl_int ret = clSetKernelArg(kernel_, argi_++, sizeof(val), &val);
        if (ret != CL_SUCCESS)
            std::cerr << "clSetKernelArg failed for cl_int." << std::endl;
    }

    void appendFloat(cl_float val) {
        cl_int ret = clSetKernelArg(kernel_, argi_++, sizeof(val), &val);
        if (ret != CL_SUCCESS)
            std::cerr << "clSetKernelArg failed for cl_float." << std::endl;
    }

private:

    cl_kernel kernel_;
    cl_uint argi_;
    bool genArraySizeArgs_;
};

#endif
