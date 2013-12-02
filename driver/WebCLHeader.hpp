#ifndef WEBCLVALIDATOR_WEBCLHEADER
#define WEBCLVALIDATOR_WEBCLHEADER

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

#include <iostream>
#include <map>
#include <set>
#include <string>

#include <wclv/wclv.h>

/// Emits JSON headers for kernels.
class WebCLHeader
{
public:

    WebCLHeader();
    ~WebCLHeader();

    /// Creates a JSON header for given set of functions and writes it
    /// to the given stream.
    void emitHeader(std::ostream &out, const WebCLAnalyser::KernelList &kernels);

private:

    /// Emits an integer entry to the given stream:
    /// -> "key" : value
    void emitNumberEntry(std::ostream &out, const std::string &key, int value);
    /// Emits a string entry to the given stream:
    /// -> "key" : "value"
    void emitStringEntry(std::ostream &out, const std::string &key, const std::string &value);

    /// Emits version to the given stream:
    /// -> "version" : "1.0"
    void emitVersion(std::ostream &out);

    /// Contains optional "key" : "string" entries for parameters.
    typedef std::map<std::string, std::string> Fields;

    /// Emits parameter to the given stream:
    /// "int foo", 0
    /// ->
    /// "foo" : { "index" : 0, "host-type" : "cl_int" }
    ///
    /// Additional "key" : "string" entries are emitted if the extra
    /// fields parameter is defined.
    void emitParameter(
        std::ostream &out,
        const std::string &parameter, int index, const std::string &type,
        const Fields &fields = Fields());

    /// Emits builtin parameter to the given stream:
    /// "__read_only image2d_t img", 0
    /// ->
    /// "img" : { "index" : 0, "host-type" : image2d_t", "access" : "read_only" }
    void emitBuiltinParameter(
        std::ostream &out,
        const WebCLAnalyser::KernelArgInfo &parameter, int index, const std::string &type);

    /// Emits a memory object parameter to the given stream:
    /// "__global int *foo", 0
    /// ->
    /// "foo" : {
    ///           "index" : 0,
    ///           "host-type" : "cl_mem",
    ///           "host-element-type" : "cl_int",
    ///           "address-space" : "global",
    ///           "size-parameter" : "_wcl_foo_size"
    ///         }
    void emitArrayParameter(
        std::ostream &out,
        const WebCLAnalyser::KernelArgInfo &parameter, int index);

    /// Emits kernel and its parameters to the given stream:
    /// "__kernel void foo(...)"
    /// ->
    /// "foo" : { ... }
    void emitKernel(std::ostream &out, const WebCLAnalyser::KernelInfo &kernel);

    /// Emits kernels and their parameters to the given stream:
    /// "__kernel void foo(...); __kernel void bar(...)"
    /// ->
    /// "kernels" : {
    ///               "foo" : { ... },
    ///               "bar" : { ... }
    ///             }
    void emitKernels(std::ostream &out, const WebCLAnalyser::KernelList &kernels);

    /// Emits correct indentation based on current indentation level.
    void emitIndentation(std::ostream &out) const;

    /// Basic tab width for indentation.
    const std::string indentation_;
    /// Current indentation level.
    unsigned int level_;
};

#endif // WEBCLVALIDATOR_WEBCLHEADER
