#ifndef WEBCLVALIDATOR_OPENCLBUILDER
#define WEBCLVALIDATOR_OPENCLBUILDER

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

#include "validator.hpp"

#include <iostream>
#include <set>
#include <string>

class OpenCLBuilder
{
public:

    typedef std::pair<unsigned int, unsigned int> PlatformAndDevice;
    typedef std::set<PlatformAndDevice> PlatformsAndDevices;

    OpenCLBuilder(const std::string &name) : name_(name) {
    }

    virtual ~OpenCLBuilder() {
    }

    virtual bool compileInput(OpenCLValidator &validator, const std::string &options) {

        PlatformsAndDevices platformsAndDevices;
        if (!collectPlatformsAndDevices(validator, platformsAndDevices))
            return false;

        for (PlatformsAndDevices::iterator i = platformsAndDevices.begin();
             i != platformsAndDevices.end(); ++i) {

            unsigned int platform = (*i).first;
            if (!validator.createPlatform(platform))
                return false;
            unsigned int device = (*i).second;
            if (!validator.createDevice(device))
                return false;

            std::cout << name_ << ": Compiling input for "
                      << validator.getPlatformName() << "/"
                      << validator.getDeviceName() << "." << std::endl;

            if (!compileInputOnCurrentPlatformAndDevice(validator, options))
                return false;
        }

        return true;
    }

protected:

    virtual bool isCurrentPlatformAndDeviceGoodEnough(OpenCLValidator &validator) = 0;
    virtual bool isCurrentPlatformAndDeviceCollected(OpenCLValidator &validator) = 0;

    virtual bool collectPlatformsAndDevices(
        OpenCLValidator &validator, PlatformsAndDevices &platformsAndDevices) {
        const unsigned int numPlatforms = validator.getNumPlatforms();
        if (!numPlatforms) {
            std::cerr << name_ << ": Can't find OpenCL platforms." << std::endl;
            return false;
        }

        PlatformAndDevice fallback(numPlatforms, 0);

        for (unsigned int platform = 0; platform < numPlatforms; ++platform) {
            if (!validator.createPlatform(platform)) {
                std::cerr << name_ << ": Can't create OpenCL platform"
                          << platform + 1 << "/" << numPlatforms << "."
                          << std::endl;
                return false;
            }

            const unsigned int numDevices = validator.getNumDevices();
            if (!numDevices) {
                std::cerr << name_ << ": Can't find OpenCL devices." << std::endl;
                return false;
            }

            for (unsigned int device = 0; device < numDevices; ++device) {
                if (!validator.createDevice(device)) {
                    std::cerr << name_ << ": Can't create OpenCL device"
                              << device + 1 << "/" << numDevices << "."
                              << std::endl;
                    return false;
                }

                if (isCurrentPlatformAndDeviceGoodEnough(validator))
                    fallback = PlatformAndDevice(platform, device);
                if (isCurrentPlatformAndDeviceCollected(validator))
                    platformsAndDevices.insert(PlatformAndDevice(platform, device));
            }
        }

        if (!platformsAndDevices.size()) {
            if (fallback.first == numPlatforms) {
                std::cerr << name_ << ": Can't accept any OpenCL devices." << std::endl;
                return false;
            }
            platformsAndDevices.insert(fallback);
        }

        return true;
    }

    virtual bool compileInputOnCurrentPlatformAndDevice(
        OpenCLValidator &validator, const std::string &options) {
        if (!validator.createContext()) {
            std::cerr << name_ << ": Can't create OpenCL context." << std::endl;
            return false;
        }

        if (!validator.createQueue()) {
            std::cerr << name_ << ": Can't create OpenCL queue." << std::endl;
            return false;
        }
            
        if (!validator.createProgram()) {
            std::cerr << name_ << ": Can't create OpenCL program." << std::endl;
            return false;
        }
            
        if (!validator.buildProgram(options)) {
            std::cerr << name_ << ": Can't build OpenCL program." << std::endl;
            validator.printProgramLog();
            return false;
        }

        return true;
    }

    const std::string name_;
};

class OpenCLBuilderForAllPlatformsAndDevices : public OpenCLBuilder
{
public:

    OpenCLBuilderForAllPlatformsAndDevices(const std::string &name)
        : OpenCLBuilder(name) {
    }

    virtual ~OpenCLBuilderForAllPlatformsAndDevices() {
    }

protected:

    virtual bool isCurrentPlatformAndDeviceGoodEnough(OpenCLValidator &validator) {
        return true;
    }

    virtual bool isCurrentPlatformAndDeviceCollected(OpenCLValidator &validator) {
        return true;
    }
};

class OpenCLBuilderForOnePlatformAndDevice : public OpenCLBuilder
{
public:

    OpenCLBuilderForOnePlatformAndDevice(
        const std::string &name,
        const std::string &platform, const std::string &device)
        : OpenCLBuilder(name), platform_(platform), device_(device) {
    }

    virtual ~OpenCLBuilderForOnePlatformAndDevice() {
    }

protected:

    virtual bool isCurrentPlatformAndDeviceGoodEnough(OpenCLValidator &validator) {
        return true;
    }

    virtual bool isCurrentPlatformAndDeviceCollected(OpenCLValidator &validator) {
        return !validator.getPlatformName().compare(platform_) &&
            !validator.getDeviceName().compare(device_);
    }

    const std::string platform_;
    const std::string device_;
};

#endif // WEBCLVALIDATOR_OPENCLBUILDER
