#ifndef WEBCLVALIDATOR_WEBCLHELPER
#define WEBCLVALIDATOR_WEBCLHELPER

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

#include <vector>

namespace clang {
    class ParmVarDecl;
    class VarDecl;
}

/// Represents all relocated variables. The variables are ordered so
/// that the corresponding address space structure can be padded.
typedef std::vector<clang::VarDecl*> AddressSpaceInfo;

/// Represents all disjoint memory areas of an address space.
/// - Static areas consist of relocated variables.
/// - Dynamic areas consist of kernel memory object parameters.
///
/// Used to write limit structures and initializers for the address
/// space.
class AddressSpaceLimits
{
public:

    AddressSpaceLimits(unsigned addressSpace);
    ~AddressSpaceLimits();

    /// Inform about a kernel memory object parameter that represents
    /// a dynamic memory area.
    void insert(clang::ParmVarDecl *parm);

    /// Inform whether some address space variables are accessed
    /// through the address space structure.
    void setStaticLimits(bool hasStaticLimits);
    /// \return Whether some address space variables are accessed
    /// through the address space structure.
    bool hasStaticallyAllocatedLimits();

    /// \return Address space.
    unsigned getAddressSpace();

    /// \return Whether the address space has static or dynamic limits.
    bool empty();
    /// \return The number of disjoint memory areas in the address
    /// space.
    unsigned count();

    /// Disjoint dynamic memory areas.
    typedef std::vector<clang::ParmVarDecl*> LimitList;
    // \return Memory object parameters that represent dynamic memory
    // areas.
    LimitList &getDynamicLimits();

private:

    /// Whether variables have been relocated.
    bool hasStaticLimits_;
    /// Identifies the address space.
    unsigned addressSpace_;
    /// Disjoint memory areas passed as kernel parameters.
    LimitList dynamicLimits_;
};

#endif // WEBCLVALIDATOR_WEBCLHELPER
