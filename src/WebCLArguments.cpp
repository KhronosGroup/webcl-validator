#include "WebCLArguments.hpp"
#include "kernel.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

WebCLArguments::WebCLArguments(int argc, char const *argv[])
    : validatorArgc_(0)
    , validatorArgv_(NULL)
    , headerDescriptor_(-1)
    , headerFilename_(NULL)
{
    char const *buffer = reinterpret_cast<char const*>(kernel_cl);
    size_t length = kernel_cl_len;
    headerFilename_ = createFullTemporaryFile(headerDescriptor_, buffer, length);

    // TODO: when we port to llvm-3.3 we can just say -target spir and it
    //       should have valid address space mapping
    char const *toolOptions[] = {
        "-x", "cl",
        "-ffake-address-space-map",
        "-include", headerFilename_
    };
    const int numToolOptions = sizeof(toolOptions) / sizeof(toolOptions[0]);

    validatorArgc_ = argc + 1 + numToolOptions;
    validatorArgv_ = new char const *[validatorArgc_];
    if (!validatorArgv_) {
        std::cerr << "Internal error. Can't create argument list." << std::endl;
        return;
    }

    const int userOptionOffset = 2;
    const int toolOptionOffset = 3;
    assert((argc >= userOptionOffset) && (validatorArgc_ >= toolOptionOffset));
    // command name and input filename
    std::copy(argv, argv + userOptionOffset, validatorArgv_);
    // separator before all options
    validatorArgv_[toolOptionOffset - 1] = "--";

    // user options
    const int numUserOptions = argc - userOptionOffset;
    char const **userBegin = argv + userOptionOffset;
    char const **userEnd = userBegin + numUserOptions;
    char const **userResult = validatorArgv_ + toolOptionOffset;
    std::copy(userBegin, userEnd, userResult);

    // tool options
    char const **toolBegin = toolOptions;
    char const **toolEnd = toolBegin + numToolOptions;
    char const **toolResult = userResult + numUserOptions;
    std::copy(toolBegin, toolEnd, toolResult);
}

WebCLArguments::~WebCLArguments()
{
    delete[] validatorArgv_;
    validatorArgv_ = NULL;

    close(headerDescriptor_);
    remove(headerFilename_);

    delete[] headerFilename_;
    headerFilename_ = NULL;
}

int WebCLArguments::getValidatorArgc() const
{
    if (!areValidatorArgumentsOk())
        return 0;
    return validatorArgc_;
}

char const **WebCLArguments::getValidatorArgv() const
{
    if (!areValidatorArgumentsOk())
        return NULL;
    return validatorArgv_;
}

bool WebCLArguments::areValidatorArgumentsOk() const
{
    return validatorArgv_ && headerFilename_;
}

char const *WebCLArguments::createEmptyTemporaryFile(int &fd) const
{
    char const *directory = P_tmpdir;
    char const *filename = "/wclXXXXXX";

    const int directoryLength = strlen(directory);
    const int filenameLength = strlen(filename);
    const int templateLength = directoryLength + filenameLength;

    char *result = new char[templateLength + 1];
    if (!result) {
        std::cerr << "Internal error. Can't create temporary filename." << std::endl;
        return NULL;
    }
 
    std::copy(directory, directory + directoryLength, result);
    std::copy(filename, filename + filenameLength, result + directoryLength);
    result[templateLength] = '\0';

    fd = mkstemp(result);
    if (fd == -1) {
        std::cerr << "Internal error. Can't create temporary file." << std::endl;
        delete[] result;
        return NULL;
    }

    return result;
}

char const *WebCLArguments::createFullTemporaryFile(int &fd, char const *buffer, size_t length) const
{
    char const *filename = createEmptyTemporaryFile(fd);
    if (!filename)
        return NULL;

    const size_t bytes = write(fd, buffer, length);
    if (bytes != length) {
        std::cerr << "Internal error. Can't populate temporary file." << std::endl;
        delete[] filename;
        close(fd);
        return NULL;
    }

    return filename;
}
