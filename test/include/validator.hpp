#ifndef WEBCLVALIDATOR_OPENCLVALIDATOR
#define WEBCLVALIDATOR_OPENCLVALIDATOR

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
// Maverics HD4000 driver crashes OS
#define DEVICE_TYPE CL_DEVICE_TYPE_CPU
#else
#include <CL/cl.h>
#define DEVICE_TYPE CL_DEVICE_TYPE_ALL
#endif

#include <iostream>
#include <iterator>

class OpenCLValidator
{
public:

    OpenCLValidator()
        : numPlatforms_(0), platform_(0), numDevices_(0), device_(0)
        , context_(0), queue_(0), program_(0)
        , code_() {
        // We might build the same code for multiple platforms.
        // Therefore we need to consume standard input only once.
        std::cin >> std::noskipws;
        std::istream_iterator<char> pos(std::cin);
        std::istream_iterator<char> end;
        code_ = std::string(pos, end);
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

    virtual unsigned int getNumPlatforms() {
        if (clGetPlatformIDs(0, NULL, &numPlatforms_) != CL_SUCCESS)
            return 0;
        return numPlatforms_;
    }

    virtual bool createPlatform(unsigned int platform) {
        if (platform >= numPlatforms_)
            return false;

        const unsigned int maxPlatforms = 10;
        if (platform >= maxPlatforms)
            return false;

        cl_platform_id platforms[maxPlatforms];
        if (clGetPlatformIDs(numPlatforms_, platforms, NULL) != CL_SUCCESS)
            return false;
        platform_ = platforms[platform];
        return true;
    }

    virtual unsigned int getNumDevices() {
        if (clGetDeviceIDs(platform_, DEVICE_TYPE,
                           0, NULL, &numDevices_) != CL_SUCCESS) {
            return 0;
        }
        return numDevices_;
    }

    virtual bool createDevice(unsigned int device) {
        if (device >= numDevices_)
            return false;

        const unsigned int maxDevices = 10;
        if (device >= maxDevices)
            return false;

        cl_device_id devices[maxDevices];
        if (clGetDeviceIDs(platform_, DEVICE_TYPE,
                           numDevices_, devices, NULL) != CL_SUCCESS) {
            return false;
        }
        device_ = devices[device];
        return true;
    }

    virtual bool createContext() {
        if (context_) {
            clReleaseContext(context_);
            context_ = 0;
        }

        cl_context_properties properties[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platform_,
            0
        };
        context_ = clCreateContext(properties, 1, &device_, NULL, NULL, NULL);
        return context_ != 0;
    }

    virtual bool createQueue() {
        if (queue_) {
            clReleaseCommandQueue(queue_);
            queue_ = 0;
        }
        queue_ =  clCreateCommandQueue(context_, device_, 0, NULL);
        return queue_ != 0;
    }

    virtual bool createProgram() {
        if (program_) {
            clReleaseProgram(program_);
            program_ = 0;
        }

        const char *source = code_.c_str();
        program_ = clCreateProgramWithSource(context_, 1, &source, NULL, NULL);
        return program_ != 0;
    }

    virtual bool buildProgram(std::string options) {
        const std::string platformName = getPlatformName();
        if (platformName.find("AMD") != std::string::npos)
            options.append("-D__PLATFORM_AMD__");
        else if (platformName.find("Portable OpenCL") != std::string::npos)
            options.append("-D__PLATFORM_POCL__");
        else if (platformName.find("Intel(R) OpenCL") != std::string::npos)
            options.append("-D__PLATFORM_INTEL__");

        return clBuildProgram(program_, 1, &device_, options.c_str(), NULL, NULL) == CL_SUCCESS;
    }

    std::string getPlatformName() {
        char name[100];
        if (clGetPlatformInfo(platform_, CL_PLATFORM_NAME, sizeof(name), &name, NULL) != CL_SUCCESS)
            return "?";
        return name;
    }

    std::string getDeviceName() {
        char name[100];
        if (clGetDeviceInfo(device_, CL_DEVICE_NAME, sizeof(name), &name, NULL) != CL_SUCCESS)
            return "?";
        return name;
    }

    virtual void printProgramLog() {
        char log[10 * 1024];
        if (clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL) == CL_SUCCESS) {
            std::cerr << log;
        }
    }

protected:

    cl_uint numPlatforms_;
    cl_platform_id platform_;
    cl_uint numDevices_;
    cl_device_id device_;

    cl_context context_;
    cl_command_queue queue_;
    cl_program program_;

    std::string code_;
};

#endif // WEBCLVALIDATOR_OPENCLVALIDATOR
