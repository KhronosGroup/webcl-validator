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

#include <map>
#include <set>
#include <string>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"

#include "WebCLTypes.hpp"

namespace WebCLTypes {
    HostTypes hostTypes_;

    BuiltinTypes supportedBuiltinTypes_;

    /// Constains OpenCL C builtin types that may not occur as kernel
    /// parameters.
    BuiltinTypes unsupportedBuiltinTypes_;

    namespace {
	/// Add host mappings for scalar types:
	/// int -> cl_int
	void initializeScalarTypes()
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

	/// Add host mappings for vector types:
	/// float4 -> cl_float4
	void initializeVectorTypes()
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

	/// Add host mappings for builtin types:
	/// image2d_t -> image2d_t
	void initializeSpecialTypes()
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

	struct Initializer {
	    Initializer() {
		initializeScalarTypes();
		initializeVectorTypes();
		initializeSpecialTypes();
	    }
	} initializer;
    }

    const HostTypes& hostTypes()
    {
	return hostTypes_;
    }

    const BuiltinTypes& supportedBuiltinTypes()
    {
	return supportedBuiltinTypes_;
    }

    const BuiltinTypes& unsupportedBuiltinTypes()
    {
	return unsupportedBuiltinTypes_;
    }
    
    clang::QualType reduceType(const clang::CompilerInstance &instance, clang::QualType type)
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

	    type = type.getSingleStepDesugaredType(instance.getASTContext());
	    type = type.getUnqualifiedType();
	} while (type != reducedType);

	return reducedType;
    }

    unsigned getAddressSpace(clang::Expr *expr)
    {
	clang::ExtVectorElementExpr *vecExpr =
	    llvm::dyn_cast<clang::ExtVectorElementExpr>(expr);
	clang::QualType exprType = (vecExpr && vecExpr->isArrow()) ?
	    vecExpr->getBase()->getType().getTypePtr()->getPointeeType() :
	    expr->getType();
	return exprType.getAddressSpace();
    }
}
