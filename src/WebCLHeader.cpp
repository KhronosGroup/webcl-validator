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

#include "WebCLConfiguration.hpp"
#include "WebCLHeader.hpp"

#include "clang/AST/Decl.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Frontend/CompilerInstance.h"

WebCLHeader::WebCLHeader(
    clang::CompilerInstance &instance, WebCLConfiguration &cfg)
    : WebCLReporter(instance)
    , cfg_(cfg)
    , hostTypes_()
    , supportedBuiltinTypes_()
    , unsupportedBuiltinTypes_()
    , indentation_("    ")
    , level_(0)
{
    initializeScalarTypes();
    initializeVectorTypes();
    initializeSpecialTypes();
}

WebCLHeader::~WebCLHeader()
{
}

void WebCLHeader::emitHeader(std::ostream &out, Kernels &kernels)
{
    out << "\n";
    emitIndentation(out);
    out << "/* WebCL Validator JSON header\n";
    emitIndentation(out);
    out << "{\n";

    ++level_;
    emitVersion(out);
    out << ",\n";
    emitKernels(out, kernels);
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

void WebCLHeader::emitHostType(
    std::ostream &out,
    const std::string &key, const std::string &type)
{
    HostTypes::iterator i = hostTypes_.find(type);
    if (i == hostTypes_.end()) {
        error("Can't determine host type for %0.") << type;
        return;
    }

    const std::string &hostType = i->second;
    emitStringEntry(out, key, hostType);
}

void WebCLHeader::emitParameter(
    std::ostream &out,
    const std::string &parameter, int index, const std::string &type)
{
    emitIndentation(out);
    out << "\"" << parameter << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    emitNumberEntry(out, "index", index);
    out << ",\n";
    emitHostType(out, "host-type", type);
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitSizeParameter(
    std::ostream &out,
    const clang::ParmVarDecl *parameter, int index)
{
    const std::string parameterName = cfg_.getNameOfSizeParameter(parameter);
    const std::string typeName = cfg_.sizeParameterType_;
    emitParameter(out, parameterName, index, typeName);
}

void WebCLHeader::emitArrayParameter(
    std::ostream &out,
    const clang::ParmVarDecl *parameter, int index)
{
    emitIndentation(out);
    out << "\"" << parameter->getName().str() << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    emitNumberEntry(out, "index", index);
    out << ",\n";

    clang::QualType arrayType = parameter->getType();
    clang::QualType elementType = arrayType.getTypePtr()->getPointeeType();

    emitStringEntry(out, "host-type", "cl_mem");
    out << ",\n";
    emitHostType(out, "host-element-type", reduceType(elementType).getAsString());
    out << ",\n";

    unsigned addressSpace = elementType.getAddressSpace();
    switch (addressSpace) {
    case clang::LangAS::opencl_global:
        // fall through intentionally
    case clang::LangAS::opencl_constant:
        // fall through intentionally
    case clang::LangAS::opencl_local:
        emitStringEntry(out, "address-space", cfg_.getNameOfAddressSpace(addressSpace));
        out << ",\n";
        break;
    default:
        error(parameter->getLocStart(), "Invalid address space.");
        break;
    }

    emitStringEntry(out, "size-parameter", cfg_.getNameOfSizeParameter(parameter));
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitKernel(std::ostream &out, const clang::FunctionDecl *kernel)
{
    emitIndentation(out);
    out << "\"" << kernel->getName().str() << "\"" << " :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    unsigned index = 0;
    const unsigned int numParameters = kernel->getNumParams();
    for (unsigned int i = 0; i < numParameters; ++i) {
        const clang::ParmVarDecl *parameter = kernel->getParamDecl(i);

        if (i > 0)
            out << ",\n";

        clang::QualType originalType = parameter->getType();
        clang::QualType reducedType = reduceType(originalType);
        const std::string reducedName = reducedType.getAsString();

        if (supportedBuiltinTypes_.count(reducedName)) {
            // images and samplers
            emitParameter(out, parameter->getName(), index, reducedName);
        } else if (parameter->getType().getTypePtr()->isPointerType()) {
            // memory objects
            emitArrayParameter(out, parameter, index);
            ++index;
            out << ",\n";
            emitSizeParameter(out, parameter, index);
        } else {
            // primitives
            emitParameter(out, parameter->getName(), index, reducedName);
        }
        ++index;
    }
    out << "\n";

    --level_;
    emitIndentation(out);
    out << "}";
    --level_;
}

void WebCLHeader::emitKernels(std::ostream &out, Kernels &kernels)
{
    emitIndentation(out);
    out << "\"kernels\" :\n";
    ++level_;
    emitIndentation(out);
    out << "{\n";
    ++level_;

    for (Kernels::iterator i = kernels.begin(); i != kernels.end(); ++i) {
        const clang::FunctionDecl *kernel = *i;

        if (i != kernels.begin())
            out << ",\n";
        emitKernel(out, kernel);
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

void WebCLHeader::initializeScalarTypes()
{
    hostTypes_["char"] = "cl_char";
    hostTypes_["unsigned char"] = "cl_uchar";
    hostTypes_["uchar"] = "cl_uchar";

    hostTypes_["short"] = "cl_short";
    hostTypes_["unsigned short"] = "cl_ushort";
    hostTypes_["ushort"] = "cl_ushort";

    hostTypes_["int"] = "cl_int";
    hostTypes_["unsigned int"] = "cl_uint";
    hostTypes_["uint"] = "cl_uint";

    hostTypes_["long"] = "cl_long";
    hostTypes_["unsigned long"] = "cl_ulong";
    hostTypes_["ulong"] = "cl_ulong";

    hostTypes_["double"] = "cl_double";
    hostTypes_["float"] = "cl_float";
    hostTypes_["half"] = "cl_half";
}

void WebCLHeader::initializeVectorTypes()
{
    static const char *lengths[] = { "2", "3", "4", "8", "16" };
    static const int numLengths = sizeof(lengths) / sizeof(lengths[0]);

    for (int i = 0; i < numLengths; ++i) {
        const std::string length = lengths[i];

        hostTypes_["char" + length] = "cl_char" + length;
        hostTypes_["uchar" + length] = "cl_uchar" + length;

        hostTypes_["short" + length] = "cl_short" + length;
        hostTypes_["ushort" + length] = "cl_ushort" + length;

        hostTypes_["int" + length] = "cl_int" + length;
        hostTypes_["uint" + length] = "cl_uint" + length;

        hostTypes_["long" + length] = "cl_long" + length;
        hostTypes_["ulong" + length] = "cl_ulong" + length;

        hostTypes_["double" + length] = "cl_double" + length;
        hostTypes_["float" + length] = "cl_float" + length;
    }
}

void WebCLHeader::initializeSpecialTypes()
{
    static const char *image2d = "image2d_t";
    static const char *image3d = "image3d_t";
    static const char *sampler = "sampler_t";

    supportedBuiltinTypes_.insert(image2d);
    supportedBuiltinTypes_.insert(image3d);
    supportedBuiltinTypes_.insert(sampler);

    hostTypes_[image2d] = image2d;
    hostTypes_[image3d] = image3d;
    hostTypes_[sampler] = sampler;

    static const char *event = "event_t";

    unsupportedBuiltinTypes_.insert(event);
}

clang::QualType WebCLHeader::reduceType(clang::QualType type)
{
    clang::QualType reducedType = type;

    if (reducedType.isCanonical() && !reducedType.hasQualifiers())
        return reducedType;

    do {
        reducedType = type.getUnqualifiedType();
        const std::string reducedName = reducedType.getAsString();
        if (unsupportedBuiltinTypes_.count(reducedName))
            return reducedType;
        if (hostTypes_.count(reducedName))
            return reducedType;

        type = type.getSingleStepDesugaredType(instance_.getASTContext());
        type = type.getUnqualifiedType();
    } while (type != reducedType);

    return reducedType;
}
