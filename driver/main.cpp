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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

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

    std::set<std::string> extensions;
    extensions.insert("cl_khr_fp64");
    extensions.insert("cl_khr_fp16");
    extensions.insert("cl_khr_gl_sharing");

    // Only pass the rest of the arguments to the lib
    WebCLValidator validator(inputSource, extensions, argc - 2, argv + 2);

    int exitStatus = validator.run();

    if (exitStatus == 0) {
        // success, print output
        std::cout << validator.getValidatedSource();
    }

    return exitStatus;
}
