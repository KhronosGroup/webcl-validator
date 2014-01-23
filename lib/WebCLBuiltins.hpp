#ifndef WEBCLVALIDATOR_WEBCLBUILTINS
#define WEBCLVALIDATOR_WEBCLBUILTINS

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

#include <set>
#include <string>
#include <vector>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"

namespace llvm {
    class raw_ostream;
}

/// Holds information about OpenCL C builtin functions that require
/// pointer arguments. The possibly unsafe builtins are partitioned
/// into several classes depending on what kind of checks need to be
/// performed on their arguments.
class WebCLBuiltins
{
public:

    WebCLBuiltins();
    ~WebCLBuiltins();

    /// \return Whether the builtin takes pointer argument(s), but no
    /// pointer validation needs to be performed.
    bool isSafe(const std::string &builtin) const;

    /// \return Whether the builtin takes pointer argument(s) and a
    /// pointer validity check needs to be performed on them.
    bool isUnsafe(const std::string &builtin) const;

    /// \return Whether WebCL C doesn't support the OpenCL C builtin
    /// at all.
    bool isUnsupported(const std::string &builtin) const;

    /// If the argument is the name of a builtin function not declared
    /// in kernel.h, emit forward declarations for its overloads
    // to the output stream; otherwise do nothing
    void emitDeclarations(llvm::raw_ostream &os, const std::string &builtin);

private:

    /// Data structure for builtin function names.
    typedef std::set<std::string> BuiltinNames;

    /// Expands given patterns into builtin function names. For
    /// example, 'vload#' is expanded to 'vload2' - 'vload16'.
    void initialize(BuiltinNames &names, const char *patterns[], int numPatterns);

    /// The pointer argument points to an element array.
    BuiltinNames unsafeVectorBuiltins_;
    /// Calling is never allowed.
    BuiltinNames unsupportedBuiltins_;
    /// Calling is always safe for these, even if they have pointer
    /// arguments.
    BuiltinNames safeBuiltins_;
    /// Known rounding suffixes for the convert_x, vstore_x etc builtin functions
    /// (must be in a specific order, so vector used)
    std::vector<std::string> roundingSuffixes_;
    /// (Sub)set of rounding suffixes we have already emitted the incredibly
    /// expensive _CL_DECLARE_CONVERT_TYPE... macro for
    BuiltinNames usedConvertSuffixes_;
    /// Ditto for _CL_DECLARE_VSTORE_HALF...
    BuiltinNames usedVstoreHalfSuffixes_;
    /// Mapping from other known builtin function names to magic macro invocations
    /// that declare them
    llvm::StringMap<llvm::SmallVector<const char *, 2> > builtinDecls_;
    /// Have the vload_half functions been declared
    bool vloadHalfDeclared;
};

#endif // WEBCLVALIDATOR_WEBCLBUILTINS
