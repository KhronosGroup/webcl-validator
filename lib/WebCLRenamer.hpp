#ifndef WEBCLVALIDATOR_WEBCLRENAMER
#define WEBCLVALIDATOR_WEBCLRENAMER

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
#include <string>

namespace clang {
    class NamedDecl;
}

/// Renames variables and types. Sometimes relocated variables and
/// types may use an identical name. An instance of this class can
/// then be used to make sure that all relocated names are unique.
class WebCLRenamer
{
public:

    explicit WebCLRenamer(const std::string &prefix, const std::string &separator);
    ~WebCLRenamer();

    /// Write unique version of given name to the output stream.
    void rename(std::ostream &out, const clang::NamedDecl *decl);

    /// Generate unique version of given name to the output stream.
    void generate(std::ostream &out, const clang::NamedDecl *decl, const std::string &name);

private:

    /// Indicate that a named object needs to be uniquely renamed.
    unsigned int assign(const clang::NamedDecl *decl);

    /// Maps object to serial number '#'. The object could be renamed
    /// as '_wcl#_name' or '_Wcl#Name' depending on the used prefix
    /// and separator. Needed so that object can be renamed quickly.
    typedef std::map<const clang::NamedDecl*, unsigned int> Serials;
    Serials serials_;
    /// Maps object name to number of identically named objects. The
    /// count of a name becomes the serial number of next object with
    /// that name.
    typedef std::map<const std::string, unsigned int> Counts;
    Counts counts_;

    // Freely chosen prefix for renamed variables. Usually '_wcl' or
    // '_Wcl'.
    const std::string prefix_;
    // Separates generated prefix from user defined name. Usually '_'
    // or ''.
    const std::string separator_;
};

#endif // WEBCLVALIDATOR_WEBCLRENAMER
