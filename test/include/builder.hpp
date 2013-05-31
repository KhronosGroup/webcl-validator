#ifndef WEBCLVALIDATOR_OPENCLBUILDER
#define WEBCLVALIDATOR_OPENCLBUILDER

#include "validator.hpp"

#include <iostream>
#include <string>

class OpenCLBuilder
{
public:

    OpenCLBuilder(const std::string &name) : name_(name) {
    }

    virtual ~OpenCLBuilder() {
    }

    virtual bool compileInput(OpenCLValidator &validator, const std::string &options) {
        if (!validator.createPlatform()) {
            std::cerr << name_ << ": Can't find OpenCL platforms." << std::endl;
            return false;
        }
    
        if (!validator.createDevice()) {
            std::cerr << name_ << ": Can't find OpenCL devices." << std::endl;
            return false;
        }
    
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

protected:

    const std::string name_;
};

#endif // WEBCLVALIDATOR_OPENCLBUILDER
