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
#include "clang/AST/Attr.h"
#include "clang/Basic/OpenCL.h"

#include "WebCLConfiguration.hpp"
#include "WebCLCommon.hpp"
#include "WebCLDebug.hpp"
#include "WebCLTypes.hpp"

namespace WebCLTypes {
    HostTypes hostTypes_;

    BuiltinTypes supportedBuiltinTypes_;

    /// Constains OpenCL C builtin types that may not occur as kernel
    /// parameters.
    BuiltinTypes unsupportedBuiltinTypes_;

    InitialZeroValues initialZeroValues_;

    namespace {
        /// Add host mappings for scalar types:
        /// int -> cl_int
        void initializeScalarTypes()
        {
            hostTypes_["char"] = "cl_char";
            initialZeroValues_["char"] = "(char) 0";
            hostTypes_["unsigned char"] = "cl_uchar";
            initialZeroValues_["unsigned char"] = "(unsigned char) 0";
            hostTypes_["uchar"] = "cl_uchar";
            initialZeroValues_["uchar"] = "(uchar) 0";

            hostTypes_["short"] = "cl_short";
            initialZeroValues_["short"] = "(short) 0";
            hostTypes_["unsigned short"] = "cl_ushort";
            initialZeroValues_["unsigned short"] = "(unsigned short) 0";
            hostTypes_["ushort"] = "cl_ushort";
            initialZeroValues_["ushort"] = "(ushort) 0";

            hostTypes_["int"] = "cl_int";
            initialZeroValues_["int"] = "(int) 0";
            hostTypes_["unsigned int"] = "cl_uint";
            initialZeroValues_["unsigned int"] = "(unsigned int) 0";
            hostTypes_["uint"] = "cl_uint";
            initialZeroValues_["uint"] = "(uint) 0";

            hostTypes_["long"] = "cl_long";
            initialZeroValues_["long"] = "(long) 0";
            hostTypes_["unsigned long"] = "cl_ulong";
            initialZeroValues_["unsigned long"] = "(unsigned long) 0";
            hostTypes_["ulong"] = "cl_ulong";
            initialZeroValues_["ulong"] = "(ulong) 0";

            hostTypes_["double"] = "cl_double";
            initialZeroValues_["double"] = "(double) 0";
            hostTypes_["float"] = "cl_float";
            initialZeroValues_["float"] = "(float) 0";
            hostTypes_["half"] = "cl_half";
            initialZeroValues_["half"] = "(half) 0";
        }

        std::string zeroInitializer(std::string type, unsigned length)
        {
            std::stringstream st;
            st << "(" << type << length << ") (";
            for (unsigned i = 0; i < length; ++i) {
                if (i) {
                    st << ", ";
                }
                st << "0";
            }
            st << ")";
            return st.str();
        }

        /// Add host mappings for vector types:
        /// float4 -> cl_float4
        void initializeVectorTypes()
        {
            WebCLConfiguration cfg;

            for (UintList::const_iterator it = cfg.dataWidths_.begin();
                it != cfg.dataWidths_.end();
                ++it) {
                    unsigned length = *it;
                    const std::string lengthStr = stringify(length);

                    hostTypes_["char" + lengthStr] = "cl_char" + lengthStr;
                    initialZeroValues_["char" + lengthStr] = zeroInitializer("char", length);
                    hostTypes_["uchar" + lengthStr] = "cl_uchar" + lengthStr;
                    initialZeroValues_["uchar" + lengthStr] = zeroInitializer("uchar", length);

                    hostTypes_["short" + lengthStr] = "cl_short" + lengthStr;
                    initialZeroValues_["short" + lengthStr] = zeroInitializer("short", length);
                    hostTypes_["ushort" + lengthStr] = "cl_ushort" + lengthStr;
                    initialZeroValues_["ushort" + lengthStr] = zeroInitializer("ushort", length);

                    hostTypes_["int" + lengthStr] = "cl_int" + lengthStr;
                    initialZeroValues_["int" + lengthStr] = zeroInitializer("int", length);
                    hostTypes_["uint" + lengthStr] = "cl_uint" + lengthStr;
                    initialZeroValues_["uint" + lengthStr] = zeroInitializer("uint", length);

                    hostTypes_["long" + lengthStr] = "cl_long" + lengthStr;
                    initialZeroValues_["long" + lengthStr] = zeroInitializer("long", length);
                    hostTypes_["ulong" + lengthStr] = "cl_ulong" + lengthStr;
                    initialZeroValues_["ulong" + lengthStr] = zeroInitializer("ulong", length);

                    hostTypes_["double" + lengthStr] = "cl_double" + lengthStr;
                    initialZeroValues_["double" + lengthStr] = zeroInitializer("double", length);
                    hostTypes_["float" + lengthStr] = "cl_float" + lengthStr;
                    initialZeroValues_["float" + lengthStr] = zeroInitializer("float", length);
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

    const BuiltinTypes& allOclTypes()
    {
        static BuiltinTypes types;
        if (types.empty()) {
            for (HostTypes::const_iterator i = hostTypes_.begin(); i != hostTypes_.end(); ++i)
                types.insert(i->first);
            types.insert(supportedBuiltinTypes_.begin(), supportedBuiltinTypes_.end());
            types.insert(unsupportedBuiltinTypes_.begin(), unsupportedBuiltinTypes_.end());
        }
        return types;
    }

    const InitialZeroValues& initialZeroValues()
    {
        return initialZeroValues_;
    }

    clang::QualType reduceType(const clang::CompilerInstance &instance, clang::QualType type)
    {
        static std::string indent;

        clang::QualType reducedType = type;

        DEBUG( std::cerr << indent << "Reducing " << reducedType.getAsString() << '\n'; )

        // First, clean up qualifiers (at the current indirection level in case of pointers)
        if (reducedType.hasQualifiers()) {
            reducedType = reducedType.getUnqualifiedType();
            DEBUG( std::cerr << indent << "  w/o quals " << reducedType.getAsString() << '\n'; )
        }

        // Clean up initial user typedefs, but stop when we encounter an OpenCL type
        // (in Clang, some OpenCL types like image2d_t are typedefs, but we want to preserve them)
        clang::QualType nextType;
        while (!allOclTypes().count(reducedType.getAsString())
            && (nextType = reducedType.getSingleStepDesugaredType(instance.getASTContext())) != reducedType) {
                reducedType = nextType;
                DEBUG( std::cerr << indent << "  desugared " << reducedType.getAsString() << '\n'; )
        }

        // Clean up pointer (to pointer (...)) types recursively
        // ... except OpenCL types like image2d_t, which are actually pointers in the clang impl
        if (reducedType.getTypePtr()->isPointerType() && !allOclTypes().count(reducedType.getAsString())) {
            DEBUG( std::cerr << indent << "  Handling pointer recursively\n"; )
            DEBUG( indent.append("    "); )

            clang::QualType pointerType = instance.getASTContext().getPointerType(
                reduceType(instance, reducedType.getTypePtr()->getPointeeType()));
            reducedType = pointerType;

            DEBUG( indent.erase(indent.size() - 4); )
            DEBUG( std::cerr << "  After pointer recursion " << reducedType.getAsString() << '\n'; )
        }

        DEBUG( std::cerr << indent << "Finally " << reducedType.getAsString() << '\n'; )

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

    ImageKind imageKind(const clang::Type* type, const clang::Decl* decl)
    {
        ImageKind kind = NOT_IMAGE;
        if (type->isImageType()) {
            if (const clang::TypedefType *typedefType = clang::dyn_cast<clang::TypedefType>(type)) {
                if (decl->hasAttr<clang::OpenCLImageAccessAttr>()) {
                    kind = INVALID_TYPEDEF_ACCESS;
                } else {
                    kind = imageKind(
                        typedefType->getDecl()->getUnderlyingType().getTypePtr(),
                        typedefType->getDecl());
                }
            } else {
                // matches image3d_t as well, but that's handled elsewhere
                if (decl->hasAttr<clang::OpenCLImageAccessAttr>()) {
                    switch (decl->getAttr<clang::OpenCLImageAccessAttr>()->getAccess()) {
                    case 0:
                        // no access qualifier, fall through to the default of read-only
                    case clang::CLIA_read_only:
                        kind = READABLE_IMAGE;
                        break;
                    case clang::CLIA_write_only:
                        kind = WRITABLE_IMAGE;
                        break;
                    case clang::CLIA_read_write:
                        // This will cause an error later on
                        kind = RW_IMAGE;
                        break;
                    default:
                        kind = UNKNOWN_ACCESS_IMAGE;
                        break;
                    }
                } else {
                    kind = UNKNOWN_ACCESS_IMAGE;
                }
            }
        }
        return kind;
    }

    const clang::DeclRefExpr *declRefExprViaImplicit(const clang::Expr *expr)
    {
        const clang::DeclRefExpr *declRefExpr = clang::dyn_cast<clang::DeclRefExpr>(expr);
        if (!declRefExpr) {
            if (const clang::ImplicitCastExpr *implicitCastExpr = clang::dyn_cast<const clang::ImplicitCastExpr>(expr)) {
                declRefExpr = clang::dyn_cast<const clang::DeclRefExpr>(*implicitCastExpr->child_begin());
            }
        }
        return declRefExpr;
    }
}
