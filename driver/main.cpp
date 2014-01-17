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

#include <clv/clv.h>

#include <stdlib.h>

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "WebCLHeader.hpp"

namespace {
    bool readAll(std::ifstream &from, std::string &to)
    {
        from.seekg(0, std::ios::end);
        to.resize(from.tellg());
        from.seekg(0, std::ios::beg);
        return from.read(&to[0], to.size());
    }
}

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

    // Handle -I and -include (these aren't allowed to be passed in the library API, so
    // we do it here for the CLI use case, which is mostly testing the CLI validator)
    //
    // This is actually very sensible, as this way we avoid any possible system-specific
    // default include paths where clang might start searching for included files.

    std::vector<std::string> includePaths;
    includePaths.push_back(".");
    for (int i = 2; i < argc; ++i) {
        char const *option = argv[i];
        if (!std::string(option).substr(0, 2).compare("-I"))
            includePaths.push_back(option + 2);
    }

    for (int i = 2; i < argc; ++i) {
        char const *option = argv[i];
        if (!std::string(option).compare("-include")) {
            if (++i == argc) {
                std::cerr << "Option requires an argument: -include; exiting\n";
                return EXIT_FAILURE;
            }

            const std::string filename = argv[i];

            bool success = false;
            for (std::vector<std::string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i) {
                const std::string path = *i + "/" + filename;

                std::ifstream ifs(path.c_str(), std::ios::binary);
                if (ifs.good()) {
                    std::string fileContents;
                    if (readAll(ifs, fileContents)) {
                        inputSource.append(fileContents);
                        inputSource.append("\n"); // make sure there's a newline between each header
                        success = true;
                        break;
                    }
                }
            }

            if (!success) {
                std::cerr << "Failed to find include file " << filename << ", exiting\n";
                return EXIT_FAILURE;
            }
        }
    }

    // Read the actual input file
    const std::string inputFilename(argv[1]);

    if (inputFilename == "-") {
        // input is stdin

        std::cin >> std::noskipws;

        std::istreambuf_iterator<char> begin(std::cin.rdbuf());
        std::istreambuf_iterator<char> end;
        inputSource.append(begin, end);
    } else {
        // input is a file
        std::ifstream ifs(inputFilename.c_str(), std::ios::binary);

        if (!ifs.good()) {
            std::cerr << "Failed to open input file \"" << inputFilename << "\", exiting" << std::endl;
            return EXIT_FAILURE;
        }

        std::string fileContents;
        if (!readAll(ifs, fileContents)) {
            std::cerr << "Failed to read from input file \"" << inputFilename << "\", exiting" << std::endl;
            return EXIT_FAILURE;
        }
        inputSource.append(fileContents);
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

    // TODO: handle arguments like -ferror-limit as webcl-validator CLI specific options;
    // that specific one should affect error printing

    // Run validator
    cl_int err = CL_SUCCESS;
    clv_program prog = clvValidate(inputSource.c_str(), &extensions[0], &userDefines[0], NULL, NULL, &err);
    if (!prog) {
        std::cerr << "Failed to call validator: " << err << '\n';
        return EXIT_FAILURE;
    }

    // Print validation log
    for (cl_int i = 0; i < clvGetProgramLogMessageCount(prog); i++) {
        // TODO: print line n/o info once we can reasonably map source lines

        // Print severity
        switch (clvGetProgramLogMessageLevel(prog, i)) {
        case CLV_LOG_MESSAGE_NOTE:
            std::cerr << "note: ";
            break;
        case CLV_LOG_MESSAGE_WARNING:
            std::cerr << "warning: ";
            break;
        case CLV_LOG_MESSAGE_ERROR:
            std::cerr << "error: ";
            break;
        }

        // Determine message text size
        size_t textSize = 0;
        err = clvGetProgramLogMessageText(prog, i, 0, NULL, &textSize);
        assert(err == CL_SUCCESS);

        // Get and print message text
        std::string text(textSize, '\0');
        err = clvGetProgramLogMessageText(prog, i, text.size(), &text[0], &textSize);
        assert(err == CL_SUCCESS);
        text.erase(text.size() - 1); // erase NUL
        std::cerr << text << '\n';

        // Print relevant source code
        if (clvProgramLogMessageHasSource(prog, i)) {
            std::string source(clvGetProgramLogMessageSourceLen(prog, i) + 1, '\0');
            err = clvGetProgramLogMessageSourceText(
                prog, i,
                clvGetProgramLogMessageSourceOffset(prog, i), source.size() - 1,
                source.size(), &source[0], NULL);
            assert(err == CL_SUCCESS);
            source.erase(source.size() - 1); // erase NUL
            std::cerr << source << '\n';
        }
    }

    int exitStatus = EXIT_SUCCESS;
    if (clvGetProgramStatus(prog) == CLV_PROGRAM_ACCEPTED ||
        clvGetProgramStatus(prog) == CLV_PROGRAM_ACCEPTED_WITH_WARNINGS) {
        // Success, print output

        // Print JSON header
        WebCLHeader header;
        header.emitHeader(std::cout, prog);

        // Determine source size
        size_t sourceSize = 0;
        err = clvGetProgramValidatedSource(prog, 0, NULL, &sourceSize);
        assert(err == CL_SUCCESS);

        // Get source
        std::string validatedSource(sourceSize, '\0');
        err = clvGetProgramValidatedSource(prog, validatedSource.size(), &validatedSource[0], NULL);
        assert(err == CL_SUCCESS);

        // Strip terminating NUL, we don't need it
        assert(validatedSource[validatedSource.size() - 1] == '\0');
        validatedSource.erase(validatedSource.size() - 1);

        // Print source
        std::cout << validatedSource;
    } else {
        // Validation failed

        exitStatus = EXIT_FAILURE;
    }

    clvReleaseProgram(prog);

    return exitStatus;
}
