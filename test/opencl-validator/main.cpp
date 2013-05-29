#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>
#include <iterator>
#include <set>
#include <string>

class OpenCLValidator
{
public:

    OpenCLValidator() : context_(0), queue_(0), program_(0) {
    }

    ~OpenCLValidator() {
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

    bool createPlatform() {
        cl_uint platforms;
        return clGetPlatformIDs(1, &platform_, &platforms) == CL_SUCCESS;
    }
        
    bool createDevice() {
        cl_uint devices;
        return clGetDeviceIDs(platform_, CL_DEVICE_TYPE_ALL, 1, &device_, &devices) == CL_SUCCESS;
    }

    bool createContext() {
        cl_context_properties properties[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties)platform_,
            0
        };
        context_ = clCreateContext(properties, 1, &device_, NULL, NULL, NULL);
        return context_ != 0;
    }

    bool createQueue() {
       queue_ =  clCreateCommandQueue(context_, device_, 0, NULL);
       return queue_ != 0;
    }

    bool createProgram() {
        std::cin >> std::noskipws;
        std::string code((std::istream_iterator<char>(std::cin)), std::istream_iterator<char>());
        const char *source = code.c_str();
        program_ = clCreateProgramWithSource(context_, 1, &source, NULL, NULL);
        return program_ != 0;
    }

    bool buildProgram(std::string options) {
        options.append("-Werror");
        return clBuildProgram(program_, 1, &device_, options.c_str(), NULL, NULL) == CL_SUCCESS;
    }

    void printProgramLog() {
        char log[10 * 1024];
        if (clGetProgramBuildInfo(program_, device_, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL) == CL_SUCCESS) {
            std::cerr << log;
        }
    }

private:

    cl_platform_id platform_;
    cl_device_id device_;

    cl_context context_;
    cl_command_queue queue_;
    cl_program program_;
};

int main(int argc, char const* argv[])
{
    std::set<std::string> help;
    help.insert("-h");
    help.insert("-help");
    help.insert("--help");

    if ((argc == 2) && help.count(argv[1])) {
        std::cerr << "Usage: cat FILE | " << argv[0] << " [OPTIONS]"
                  << std::endl;
        std::cerr << "Checks whether the given OpenCL C code file can be compiled."
                  << std::endl
                  << "Any given options will be passed to clBuildProgram."
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::string options;
    for (int i = 1; i < argc; ++i) {
        options.append(argv[i]);
        options.append(" ");
    }

    OpenCLValidator validator;

    if (!validator.createPlatform()) {
        std::cerr << argv[0] << ": Can't find OpenCL platforms." << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!validator.createDevice()) {
        std::cerr << argv[0] << ": Can't find OpenCL devices." << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!validator.createContext()) {
        std::cerr << argv[0] << ": Can't create OpenCL context." << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!validator.createQueue()) {
        std::cerr << argv[0] << ": Can't create OpenCL queue." << std::endl;
        return EXIT_FAILURE;
    }
    
    if (!validator.createProgram()) {
        std::cerr << argv[0] << ": Can't create OpenCL program." << std::endl;
	return EXIT_FAILURE;
    }
    
    if (!validator.buildProgram(options)) {
        std::cerr << argv[0] << ": Can't build OpenCL program." << std::endl;
        validator.printProgramLog();
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
