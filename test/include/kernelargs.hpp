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
