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

#if 0
void WebCLHeader::emitBuiltinParameter(
    std::ostream &out,
    const WebCLAnalyser::KernelArgInfo &parameter, int index, const std::string &type)
{
    Fields fields;

    // Add "access" : "qualifier" field for images.
    if (!type.compare(image2d)) {
        std::string access = "read_only";

        switch (parameter.imageKind) {
        case WebCLAnalyser::READABLE_IMAGE:
            access = "read_only";
            break;
        case WebCLAnalyser::WRITABLE_IMAGE:
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

    emitParameter(out, parameter.name, index, type, fields);
}

namespace {
    std::string buildSizeParameterName(const WebCLAnalyser::KernelArgInfo &arrayParam)
    {
        return std::string(sizeParameterPrefix) + "_" + arrayParam.name + "_size";
    }
}

void WebCLHeader::emitArrayParameter(
    std::ostream &out,
    const WebCLAnalyser::KernelArgInfo &parameter, int index)
{
    emitIndentation(out);
    out << "\"" << parameter.name << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    emitNumberEntry(out, "index", index);
    out << ",\n";

    emitStringEntry(out, "type", parameter.reducedTypeName);
    out << ",\n";

    switch (parameter.pointerKind) {
    case WebCLAnalyser::GLOBAL_POINTER:
        emitStringEntry(out, "address-space", "global");
        break;
    case WebCLAnalyser::CONSTANT_POINTER:
        emitStringEntry(out, "address-space", "constant");
        break;
    case WebCLAnalyser::LOCAL_POINTER:
        emitStringEntry(out, "address-space", "local");
        break;
    default:
        assert(false && "WebCLKernelHandler::run() should have rejected array of private memory kernel parameter");
        break;
    }
    out << ",\n";

    emitStringEntry(out, "size-parameter", buildSizeParameterName(parameter));
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitKernel(std::ostream &out, const WebCLAnalyser::KernelInfo &kernel)
{
    emitIndentation(out);
    out << "\"" << kernel.name << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    unsigned index = 0;
    for (std::vector<WebCLAnalyser::KernelArgInfo>::const_iterator i = kernel.args.begin();
        i != kernel.args.end(); ++i) {
        const WebCLAnalyser::KernelArgInfo &parameter = *i;

        if (i != kernel.args.begin())
            out << ",\n";

        if (parameter.imageKind != WebCLAnalyser::NOT_IMAGE || parameter.reducedTypeName == "sampler_t") {
            // images and samplers
            emitBuiltinParameter(out, parameter, index, parameter.reducedTypeName);
        } else if (parameter.pointerKind != WebCLAnalyser::NOT_POINTER) {
            // memory objects
            emitArrayParameter(out, parameter, index);
            ++index;
            out << ",\n";
            emitParameter(out, buildSizeParameterName(parameter), index, sizeParameterType);
        } else {
            // primitives
            emitParameter(out, parameter.name, index, parameter.reducedTypeName);
        }
        ++index;
    }
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}
#endif

void WebCLHeader::emitKernels(std::ostream &out, wclv_program program)
{
    emitIndentation(out);
    out << "\"kernels\" :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    // TODO: iterate over kernels in program
    /*
    for (WebCLAnalyser::KernelList::const_iterator i = kernels.begin(); i != kernels.end(); ++i) {
        const WebCLAnalyser::KernelInfo &kernel = *i;

        if (i != kernels.begin())
            out << ",\n";
        emitKernel(out, kernel);
    }
    out << "\n";
    */

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