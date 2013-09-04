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

#include "WebCLReporter.hpp"

#include "clang/AST/Type.h"

#include <iostream>
#include <map>
#include <set>
#include <string>

namespace clang {
    class CompilerInstance;
    class FunctionDecl;
    class ParmVarDecl;
}

class WebCLConfiguration;

/// Emits JSON headers for kernels.
class WebCLHeader : WebCLReporter
{
public:

    WebCLHeader(clang::CompilerInstance &instance, WebCLConfiguration &cfg);
    ~WebCLHeader();

    /// A set of functions to include in the JSON header.
    typedef std::set<clang::FunctionDecl*> Kernels;
    /// Creates a JSON header for given set of functions and writes it
    /// to the given stream.
    void emitHeader(std::ostream &out, Kernels &kernels);

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

    /// Emits a host type to the given stream:
    /// "unsigned long" -> "host-type" : "cl_ulong"
    void emitHostType(std::ostream &out, const std::string &key, const std::string &type);

    /// Emits parameter to the given stream:
    /// "int foo", 0
    /// ->
    /// "foo" : { "index" : 0, "host-type" : "cl_int" }
    void emitParameter(
        std::ostream &out,
        const std::string &parameter, int index, const std::string &type);

    /// Emits a size parameter for the given memory object parameter:
    /// "__global int *foo", 0
    /// ->
    /// "_wcl_foo_size" : { "index" : 0, "host-type" : "cl_ulong" }
    void emitSizeParameter(
        std::ostream &out,
        const clang::ParmVarDecl *parameter, int index);

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
        const clang::ParmVarDecl *parameter, int index);

    /// Emits kernel and its parameters to the given stream:
    /// "__kernel void foo(...)"
    /// ->
    /// "foo" : { ... }
    void emitKernel(std::ostream &out, const clang::FunctionDecl *kernel);

    /// Emits kernels and their parameters to the given stream:
    /// "__kernel void foo(...); __kernel void bar(...)"
    /// ->
    /// "kernels" : {
    ///               "foo" : { ... },
    ///               "bar" : { ... }
    ///             }
    void emitKernels(std::ostream &out, Kernels &kernels);

    /// Emits correct indentation based on current indentation level.
    void emitIndentation(std::ostream &out) const;

    /// Add host mappings for scalar types:
    /// int -> cl_int
    void initializeScalarTypes();
    /// Add host mappings for vector types:
    /// float4 -> cl_float4
    void initializeVectorTypes();
    /// Add host mappings for builtin types:
    /// image2d_t -> image2d_t
    void initializeSpecialTypes();

    /// Reduce the given type to a host mapping type. If the reduction
    /// can't be done, the original type will be returned without
    /// modifications.
    ///
    /// typedef struct opaque_t* image2d_t;
    /// typedef image2d_t my_image;
    ///
    /// my_image -> image2d_t
    clang::QualType reduceType(clang::QualType type);

    /// Contains information about recurring names.
    WebCLConfiguration &cfg_;
    /// Maps OpenCL C types to host types: int -> cl_int.
    typedef std::map<std::string, std::string> HostTypes;
    HostTypes hostTypes_;
    /// Contains OpenCL C builtin types that can occur as kernel
    /// parameters.
    typedef std::set<std::string> BuiltinTypes;
    BuiltinTypes supportedBuiltinTypes_;
    /// Constains OpenCL C builtin types that may not occur as kernel
    /// parameters.
    BuiltinTypes unsupportedBuiltinTypes_;
    /// Basic tab width for indentation.
    const std::string indentation_;
    /// Current indentation level.
    unsigned int level_;
};

#endif // WEBCLVALIDATOR_WEBCLHEADER
