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

#include <wclv/wclv.h>

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

//#include "WebCLHeader.hpp"

int main(int argc, char const* argv[])
{
    std::set<std::string> help;
    help.insert("-h");
    help.insert("-help");
    help.insert("--help");

    if ((argc == 1) || ((argc == 2) && help.count(argv[1]))) {
        std::cerr << "Usage: " << argv[0] << " input.cl [clang-options]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputSource;
    const std::string inputFilename(argv[1]);

    if (inputFilename == "-") {
        // input is stdin

        std::cin >> std::noskipws;

        std::istreambuf_iterator<char> begin(std::cin.rdbuf());
        std::istreambuf_iterator<char> end;
        inputSource = std::string(begin, end);
    } else {
        // input is a file
        std::ifstream ifs(inputFilename.c_str(), std::ios::binary);

        if (!ifs.good()) {
            std::cerr << "Failed to open input file \"" << inputFilename << "\", exiting" << std::endl;
            return EXIT_FAILURE;
        }

        ifs.seekg(0, std::ios::end);
        inputSource.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        ifs.read(&inputSource[0], inputSource.size());
    }

    // Enable usual extensions
    std::vector<const char *> extensions;
    extensions.push_back("cl_khr_fp64");
    extensions.push_back("cl_khr_fp16");
    extensions.push_back("cl_khr_gl_sharing");
    extensions.push_back(0);

    // Parse user defines
    std::vector<const char *> userDefines;
    for (int i = 2; i < argc; ++i) {
        char const *option = argv[i];
        if (!std::string(option).substr(0, 2).compare("-D"))
            userDefines.push_back(option + 2);
    }
    userDefines.push_back(0);

    // Run validator
    cl_int err = CL_SUCCESS;
    wclv_program prog = wclvValidate(inputSource.c_str(), &extensions[0], &userDefines[0], NULL, NULL, &err);
    if (!prog) {
        std::cerr << "Failed to call validator: " << err << '\n';
        return EXIT_FAILURE;
    }

    int exitStatus = EXIT_SUCCESS;
    if (wclvGetProgramStatus(prog) == WCLV_PROGRAM_ACCEPTED ||
        wclvGetProgramStatus(prog) == WCLV_PROGRAM_ACCEPTED_WITH_WARNINGS) {
        // Success, print output

        // TODO: print warnings, if any
        // TODO: print JSON header

        // Determine source size
        size_t sourceSize = 0;
        err = wclvProgramGetValidatedSource(prog, 0, NULL, &sourceSize);
        assert(err == CL_SUCCESS);

        // Get source
        std::string validatedSource(sourceSize, '\0');
        err = wclvProgramGetValidatedSource(prog, validatedSource.size(), &validatedSource[0], NULL);
        assert(err == CL_SUCCESS);

        // Strip terminating NUL, we don't need it
        assert(validatedSource[validatedSource.size() - 1] == '\0');
        validatedSource.erase(validatedSource.size() - 1);

        // Print source
        std::cout << validatedSource;
    } else {
        // Validation failed

        // TODO: print errors

        exitStatus = EXIT_FAILURE;
    }

    wclvReleaseProgram(prog);

    return exitStatus;
}
