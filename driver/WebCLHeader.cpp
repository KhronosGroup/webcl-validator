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

#include "WebCLHeader.hpp"

#include <cassert>

static const char *image2d = "image2d_t";

// must be the same as WebCLConfiguration::variablePrefix_
static const char *sizeParameterPrefix = "_wcl";
// must be the same as WebCLConfiguration::sizeParameterType_
static const char *sizeParameterType = "ulong";

WebCLHeader::WebCLHeader()
    : indentation_("    ")
    , level_(0)
{
    // nothing
}

WebCLHeader::~WebCLHeader()
{
}

void WebCLHeader::emitHeader(std::ostream &out, wclv_program program)
{
    out << "\n";
    emitIndentation(out);
    out << "/* WebCL Validator JSON header\n";
    emitIndentation(out);
    out << "{\n";

    ++level_;
    emitVersion(out);
    out << ",\n";
    emitKernels(out, program);
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}\n";
    emitIndentation(out);
    out << "*/\n\n";
}

void WebCLHeader::emitNumberEntry(
    std::ostream &out,
    const std::string &key, int value)
{
    emitIndentation(out);
    out << "\"" << key << "\" : " << value;
}

void WebCLHeader::emitStringEntry(
    std::ostream &out,
    const std::string &key, const std::string &value)
{
    emitIndentation(out);
    out << "\"" << key << "\" : \"" << value << "\"";
}

void WebCLHeader::emitVersion(std::ostream &out)
{
    emitStringEntry(out, "version", "1.0");
}

void WebCLHeader::emitParameter(
    std::ostream &out,
    const std::string &parameter, int index, const std::string &type,
    const Fields &fields)
{
    emitIndentation(out);
    out << "\"" << parameter << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    emitNumberEntry(out, "index", index);
    out << ",\n";
    emitStringEntry(out, "type", type);

    for (Fields::const_iterator i = fields.begin(); i != fields.end(); ++i) {
        out << ",\n";
        emitStringEntry(out, i->first, i->second);
    }
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitBuiltinParameter(
    std::ostream &out,
    const std::string &name, int index, const std::string &type, cl_kernel_arg_access_qualifier accessQual)
{
    Fields fields;

    // Add "access" : "qualifier" field for images.
    if (!type.compare(image2d)) {
        std::string access = "read_only";

        switch (accessQual) {
        case CL_KERNEL_ARG_ACCESS_READ_ONLY:
            access = "read_only";
            break;
        case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
            access = "write_only";
            break;
        default:
            // ImageSafetyHandler errors on this for all image parameters which are passed on to image access functions;
            // others can be considered read_only with 0 reads as it is not possible to access images directly
            access = "read_only";
            break;
        }

        fields["access"] = access;
    }

    emitParameter(out, name, index, type, fields);
}

namespace {
    std::string buildSizeParameterName(const std::string &arrayParamName)
    {
        return std::string(sizeParameterPrefix) + "_" + arrayParamName + "_size";
    }
}

void WebCLHeader::emitArrayParameter(
    std::ostream &out,
    const std::string &name, int index, const std::string &type, cl_kernel_arg_address_qualifier addressQual)
{
    emitIndentation(out);
    out << "\"" << name << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    emitNumberEntry(out, "index", index);
    out << ",\n";

    emitStringEntry(out, "type", type);
    out << ",\n";

    switch (addressQual) {
    case CL_KERNEL_ARG_ADDRESS_GLOBAL:
        emitStringEntry(out, "address-space", "global");
        break;
    case CL_KERNEL_ARG_ADDRESS_CONSTANT:
        emitStringEntry(out, "address-space", "constant");
        break;
    case CL_KERNEL_ARG_ADDRESS_LOCAL:
        emitStringEntry(out, "address-space", "local");
        break;
    default:
        assert(false && "WebCLKernelHandler::run() should have rejected array of private memory kernel parameter");
        break;
    }
    out << ",\n";

    emitStringEntry(out, "size-parameter", buildSizeParameterName(name));
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitKernel(std::ostream &out, wclv_program program, cl_int kernel)
{
    cl_int err = CL_SUCCESS;
    size_t nameSize = 0;

    err = wclvGetProgramKernelName(program, kernel, 0, NULL, &nameSize);
    assert(err == CL_SUCCESS);

    std::string name(nameSize, '\0');
    err = wclvGetProgramKernelName(program, kernel, name.size(), &name[0], NULL);
    assert(err == CL_SUCCESS);
    name.erase(name.size() - 1);

    emitIndentation(out);
    out << "\"" << name << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    cl_int numArgs = wclvGetKernelArgCount(program, kernel);
    assert(numArgs >= 0);
    unsigned index = 0;
    for (cl_int arg = 0; arg < numArgs; ++arg) {
        if (arg != 0)
            out << ",\n";

        // Get argument name
        err = wclvGetKernelArgName(program, kernel, arg, 0, NULL, &nameSize);
        assert(err == CL_SUCCESS);
        name.resize(nameSize, '\0');

        err = wclvGetKernelArgName(program, kernel, arg, name.size(), &name[0], NULL);
        assert(err == CL_SUCCESS);
        name.erase(name.size() - 1);

        // Get argument type
        size_t typeSize = 0;

        err = wclvGetKernelArgType(program, kernel, arg, 0, NULL, &typeSize);
        assert(err == CL_SUCCESS);

        std::string type(typeSize, '\0');
        err = wclvGetKernelArgType(program, kernel, arg, type.size(), &type[0], NULL);
        assert(err == CL_SUCCESS);
        type.erase(type.size() - 1);

        if (wclvKernelArgIsImage(program, kernel, arg) || type == "sampler_t") {
            // images and samplers
            emitBuiltinParameter(out, name, index, type, wclvGetKernelArgAccessQual(program, kernel, arg));
        } else if (wclvKernelArgIsPointer(program, kernel, arg)) {
            // memory objects
            emitArrayParameter(out, name, index, type, wclvGetKernelArgAddressQual(program, kernel, arg));
            ++index;
            out << ",\n";
            emitParameter(out, buildSizeParameterName(name), index, sizeParameterType);
        } else {
            // primitives
            emitParameter(out, name, index, type);
        }
        ++index;
    }
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitKernels(std::ostream &out, wclv_program program)
{
    emitIndentation(out);
    out << "\"kernels\" :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    cl_int numKernels = wclvGetProgramKernelCount(program);
    assert(numKernels >= 0);
    for (cl_int i = 0; i < numKernels; ++i) {
        if (i != 0)
            out << ",\n";

        emitKernel(out, program, i);
    }
    out << "\n";

    --level_;
    emitIndentation(out);
    out <<"}";
    --level_;
}

void WebCLHeader::emitIndentation(std::ostream &out) const
{
    for (unsigned int i = 0; i < level_; ++i)
        out << indentation_;
}