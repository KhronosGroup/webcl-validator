#ifndef WEBCLVALIDATOR_TYPES
#define WEBCLVALIDATOR_TYPES

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

#include "clang/AST/Decl.h"

namespace WebCLTypes {
    /// Maps OpenCL C types to host types: int -> cl_int.
    typedef std::map<std::string, std::string> HostTypes;

    /// Contains OpenCL C builtin types that can occur as kernel
    /// parameters.
    typedef std::set<std::string> BuiltinTypes;

    /// Returns the static list of supported host types
    const HostTypes& hostTypes();

    /// Returns the static list of supported builtin types
    const BuiltinTypes& supportedBuiltinTypes();

    /// Returns the static list of unsupported builtin types
    const BuiltinTypes& unsupportedBuiltinTypes();

    /// Reduce the given type to a host mapping type. If the reduction
    /// can't be done, the original type will be returned without
    /// modifications.
    ///
    /// typedef struct opaque_t* image2d_t;
    /// typedef image2d_t my_image;
    ///
    /// my_image -> image2d_t
    clang::QualType reduceType(const clang::CompilerInstance &instance, clang::QualType type);

    /// \return Correct address space for the type of given
    /// expression.
    ///
    /// Works around a bug in Clang's vector element access
    /// expression.
    unsigned getAddressSpace(clang::Expr *expr);
}

#endif // WEBCLVALIDATOR_TYPES
