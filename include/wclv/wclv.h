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

#ifndef WEBCL_VALIDATOR_WCLV_H
#define WEBCL_VALIDATOR_WCLV_H

// These are just temporarily here, until we have the C API
#include "../lib/WebCLArguments.hpp"
#include "../lib/WebCLVisitor.hpp"

#include <set>
#include <string>

// Proof of concept library API for the library+executable build system
class WebCLValidator
{
public:

    WebCLValidator(
        const std::string &inputSource,
        const std::set<std::string> &extensions,
        int argc,
        char const* argv[]);
    int run();

    /// Returns validated source after a successful run
    const std::string &getValidatedSource() const { return validatedSource_; }
    /// Ditto for kernel info
    const WebCLAnalyser::KernelList &getKernels() const { return kernels_; }

private:

    WebCLArguments arguments;
    std::set<std::string> extensions;

    // Stores validated source after validation is complete.
    std::string validatedSource_;
    /// Ditto for kernel info
    WebCLAnalyser::KernelList kernels_;
};

#endif
