#ifndef KERNELARGS_HPP
#define KERNELARGS_HPP

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
        _kernel(kernel),
        _argi(0),
        _genArraySizeArgs(genArraySizeArgs)
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
        cl_int ret = clSetKernelArg(_kernel, _argi++, argsize, static_cast<const void *>(argval));
        if (ret != CL_SUCCESS)
            std::cerr << "clSetKernelArg failed for arg " << (_argi - 1) << std::endl;

        if (_genArraySizeArgs)
        {
            ret = clSetKernelArg(_kernel, _argi++, sizeof(arrayLength), &arrayLength);
            if (ret != CL_SUCCESS)
                std::cerr << "clSetKernelArg failed for array length parameter" << std::endl;
        }
    }

private:
    cl_kernel _kernel;
    cl_uint _argi;
    bool _genArraySizeArgs;
};

#endif
