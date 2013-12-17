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
#include "builder.hpp"

#include <stdlib.h>

#include <set>

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

    OpenCLValidator validator;
    std::string options;
    for (int i = 1; i < argc; ++i) {
        options.append(argv[i]);
        options.append(" ");
    }

    OpenCLBuilderForAllPlatformsAndDevices builder(argv[0]);
    if (!builder.compileInput(validator, options))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
