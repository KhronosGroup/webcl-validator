#include "kernelargs.hpp"

#include <set>
#include <vector>
#include <iostream>
#include <iterator>


namespace
{
    const unsigned char FillChar = 0xCC;
    typedef std::vector<cl_platform_id> platform_vector;
    typedef std::vector<cl_device_id> device_vector;
}

platform_vector getPlatformsIDs()
{
    const platform_vector::size_type maxPlatforms = 10;
    platform_vector platformIds(maxPlatforms);

    cl_uint numOfPlatforms = 0;
    cl_int ret = clGetPlatformIDs(maxPlatforms, platformIds.data(), &numOfPlatforms);
    if (ret != CL_SUCCESS)
    {
        std::cerr << "Failed to get platform ids." << std::endl;
        numOfPlatforms = 0;
    }
    platformIds.resize(numOfPlatforms);
    return platformIds;
}

device_vector getDevices(cl_platform_id const& platformId)
{
    cl_uint num_devices = 0;
    if (CL_SUCCESS != clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices))
    {
        std::cerr << "Failed to get number of devices." << std::endl;
        return device_vector();
    }

    device_vector devices(num_devices);
    if (CL_SUCCESS != clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, num_devices, devices.data(), NULL))
    {
        std::cerr << "clGetDeviceIDs failed." << std::endl;
        num_devices = 0;
    }
    devices.resize(num_devices);
    return devices;
}


void printDevInfo(cl_device_id device)
{
    char devbuf[128];
    char verbuf[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, 128, devbuf, NULL);
    clGetDeviceInfo(device, CL_DEVICE_VERSION, 128, verbuf, NULL);
    std::cout << "Device " << devbuf << ", version " << verbuf << std::endl;
}

std::string readAllInput()
{
    // don't skip the whitespace while reading
    std::cin >> std::noskipws;

    // use stream iterators to copy the stream to a string
    std::istream_iterator<char> begin(std::cin);
    std::istream_iterator<char> end;
    return std::string(begin, end);
}

bool verifyZeroed(const unsigned char *buf, int size)
{
    const unsigned char *end = buf + size;
    for (const unsigned char *p = buf; p != end; ++p)
    {
        if (*p != FillChar)
            return false;
    }
    return true;
}

bool testSource(cl_device_id device, std::string const& source, bool isTransformed)
{
    cl_int ret = CL_SUCCESS;

    // Create an OpenCL context
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        std::cerr << "Failed to create OpenCL context." << std::endl;
        return false;
    }

    // Create the program
    const char *buf = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&buf, NULL, &ret);

    // Build the program
    if (CL_SUCCESS != clBuildProgram(program, 1, &device, NULL, NULL, NULL))
    {
        std::cerr << "Failed to build program." << std::endl;
    }
    else
    {
        std::cout << "Build ok!" << std::endl;
    }

    cl_command_queue command_queue = clCreateCommandQueue(context, device, 0, &ret);

    const int Length = 100;
    cl_mem outArg0 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, Length * sizeof(cl_int), NULL, &ret);
    cl_mem outArg1 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, Length * sizeof(cl_char), NULL, &ret);
    cl_mem outArg2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, Length * sizeof(cl_float), NULL, &ret);
    cl_mem outArg3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, Length * sizeof(cl_float4), NULL, &ret);
    cl_kernel kernel = clCreateKernel(program, "copy_local_mem", &ret);

    KernelArgs args(kernel, isTransformed);
    args.appendArray(&outArg0, Length);
    args.appendArray(&outArg1, Length);
    args.appendArray(&outArg2, Length);
    args.appendArray(&outArg3, Length);
    args.appendLocalArray<cl_int>(Length);
    args.appendLocalArray<cl_char>(Length);
    args.appendLocalArray<cl_float>(Length);
    args.appendLocalArray<cl_float4>(Length);

    // Execute the OpenCL kernel on the list
    size_t global_item_size = 100; // Process the entire lists
    size_t local_item_size = 20; // Divide work items into groups of 20
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1,
                                 NULL, &global_item_size, &local_item_size,
                                 0, NULL, NULL);

    bool testPass = true;
    if (ret != CL_SUCCESS)
    {
        std::cerr << "clEnqueueNDRangeKernel failed with code " << ret << std::endl;
    }
    else
    {
        ret = clFinish(command_queue);

        unsigned char *buf = new unsigned char[Length * sizeof(cl_float4)]; // allocate by largest buffer
        int sizeInBytes = Length * sizeof(cl_int);
        ret = clEnqueueReadBuffer(command_queue, outArg0, CL_TRUE, 0, sizeInBytes, buf, 0, NULL, NULL);
        if (!verifyZeroed(buf, sizeInBytes))
        {
            std::cerr << "int buffer not emptied" << std::endl;
            testPass = false;
        }

        sizeInBytes = Length * sizeof(cl_char);
        ret = clEnqueueReadBuffer(command_queue, outArg1, CL_TRUE, 0, sizeInBytes, buf, 0, NULL, NULL);
        if (!verifyZeroed(buf, sizeInBytes))
        {
            std::cerr << "char buffer not emptied" << std::endl;
            testPass = false;
        }

        sizeInBytes = Length * sizeof(cl_float);
        ret = clEnqueueReadBuffer(command_queue, outArg2, CL_TRUE, 0, sizeInBytes, buf, 0, NULL, NULL);
        if (!verifyZeroed(buf, sizeInBytes))
        {
            std::cerr << "float buffer not emptied" << std::endl;
            testPass = false;
        }

        sizeInBytes = Length * sizeof(cl_float4);
        ret = clEnqueueReadBuffer(command_queue, outArg3, CL_TRUE, 0, sizeInBytes, buf, 0, NULL, NULL);
        if (!verifyZeroed(buf, sizeInBytes))
        {
            std::cerr << "float4 buffer not emptied" << std::endl;
            testPass = false;
        }

        delete [] buf;
        buf = 0;
    }

    // // Clean up
    bool cleanupOk = true;
    cleanupOk &= CL_SUCCESS == clFinish(command_queue);
    cleanupOk &= CL_SUCCESS == clReleaseKernel(kernel);
    cleanupOk &= CL_SUCCESS == clReleaseProgram(program);
    cleanupOk &= CL_SUCCESS == clReleaseMemObject(outArg0);
    cleanupOk &= CL_SUCCESS == clReleaseMemObject(outArg1);
    cleanupOk &= CL_SUCCESS == clReleaseMemObject(outArg2);
    cleanupOk &= CL_SUCCESS == clReleaseMemObject(outArg3);
    cleanupOk &= CL_SUCCESS == clReleaseCommandQueue(command_queue);
    cleanupOk &= CL_SUCCESS == clReleaseContext(context);
    if (!cleanupOk)
        std::cerr << "OpenCL program run was not cleaned up properly." << std::endl;

    return testPass;
}

int main(int argc, char const* argv[])
{
    const std::string original = "-original";
    const std::string transformed = "-transformed";
    std::set<std::string> mode;
    mode.insert(original);
    mode.insert(transformed);

    if ((argc != 2) || ((argc == 2) && !mode.count(argv[1]))) {
        std::cerr << "Usage: cat FILE | " << argv[0] << " -original|-transformed"
                  << std::endl;
        std::cerr << "Check local memory is zeroed before copied back to system memory."
                  << std::endl
                  << "Use \"-original\" for opencl code and \"-transformed\" for webcl code."
                  << std::endl;
        return EXIT_FAILURE;
    }


    std::string source = readAllInput();
    if (source.size() == 0)
    {
        std::cerr << "No OpenCL source file read from stdin." << std::endl;
        return EXIT_FAILURE;
    }

    const bool isInputTransformed = (transformed == argv[1]);
    std::cout << "Treating input source as " << (isInputTransformed ? "transformed" : "not transformed") << std::endl;


    platform_vector const& platforms = getPlatformsIDs();
    for (platform_vector::const_iterator platform = platforms.begin();
         platform != platforms.end(); ++platform)
    {
        char platname[100];
        clGetPlatformInfo(*platform, CL_PLATFORM_NAME, 100, &platname, NULL);
        std::cout << "Platform: " << platname << std::endl;

        device_vector devices = getDevices(*platform);
        for (device_vector::const_iterator device = devices.begin();
             device != devices.end(); ++device)
        {
            printDevInfo(*device);
            if (!testSource(*device, source, isInputTransformed))
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
