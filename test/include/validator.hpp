#ifndef WEBCLVALIDATOR_OPENCLVALIDATOR
#define WEBCLVALIDATOR_OPENCLVALIDATOR

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>
#include <iterator>

class OpenCLValidator
{
public:

    OpenCLValidator() : context_(0), queue_(0), program_(0) {
    }

    virtual ~OpenCLValidator() {
        // Intel's implementation requires cleanup if clBuildProgram
        // fails. Otherwise we get these errors:
        // - pure virtual method called
        // - terminate called without an active exception
        // - Segmentation fault (core dumped)
        if (program_)
            clReleaseProgram(program_);
        if (queue_)
            clReleaseCommandQueue(queue_);
        if (context_)
            clReleaseContext(context_);
    }

    virtual bool createPlatform() {
        cl_uint platforms;
        return clGetPlatformIDs(1, &platform_, &platforms) == CL_SUCCESS;
    }
        
    virtual bool createDevice() {
        cl_uint devices;
        return clGetDeviceIDs(platform_, CL_DEVICE_TYPE_ALL, 1, &device_, &devices) == CL_SUCCESS;
    }

    virtual bool createContext() {
        cl_context_properties properties[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platform_,
            0
        };
        context_ = clCreateContext(properties, 1, &device_, NULL, NULL, NULL);
        return context_ != 0;
    }

    virtual bool createQueue() {
       queue_ =  clCreateCommandQueue(context_, device_, 0, NULL);
       return queue_ != 0;
    }

    virtual bool createProgram() {
        std::cin >> std::noskipws;
        std::string code((std::istream_iterator<char>(std::cin)), std::istream_iterator<char>());
        const char *source = code.c_str();
        program_ = clCreateProgramWithSource(context_, 1, &source, NULL, NULL);
        return program_ != 0;
    }

    virtual bool buildProgram(std::string options) {
        options.append("-Werror");
        return clBuildProgram(program_, 1, &device_, options.c_str(), NULL, NULL) == CL_SUCCESS;
    }

    virtual void printProgramLog() {
        char log[10 * 1024];
        if (clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL) == CL_SUCCESS) {
            std::cerr << log;
        }
    }

protected:

    cl_platform_id platform_;
    cl_device_id device_;

    cl_context context_;
    cl_command_queue queue_;
    cl_program program_;
};

#endif // WEBCLVALIDATOR_OPENCLVALIDATOR
