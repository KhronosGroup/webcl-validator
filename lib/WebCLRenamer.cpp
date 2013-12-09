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

#include "WebCLRenamer.hpp"

#include "clang/AST/Decl.h"

WebCLRenamer::WebCLRenamer(const std::string &prefix, const std::string &separator)
    : serials_(), counts_(), prefix_(prefix), separator_(separator)
{
}

WebCLRenamer::~WebCLRenamer()
{
}

void WebCLRenamer::rename(
    std::ostream &out, const clang::NamedDecl *decl)
{
    generate(out, decl, decl->getName().str());
}

void WebCLRenamer::generate(
    std::ostream &out, const clang::NamedDecl *decl, const std::string &name)
{
    Serials::iterator i = serials_.find(decl);
    unsigned int serial = (i == serials_.end()) ? assign(decl) : i->second;

    // Ensure that the user can't interfere with our renaming
    // scheme. The prefix can be freely chosen.
    out << prefix_;
    if (serial >= 2) {
        // Here we rely on the fact that user variables can't start
        // with a number.
        out << serial << separator_;
    }

    out << name;
}

unsigned int WebCLRenamer::assign(const clang::NamedDecl *decl)
{
    const std::string name = decl->getName();
    Counts::iterator i = counts_.find(name);

    unsigned int serial = 1;
    if (i != counts_.end())
        serial = i->second + 1;

    counts_[name] = serial;
    serials_[decl] = serial;
    return serial;
}
