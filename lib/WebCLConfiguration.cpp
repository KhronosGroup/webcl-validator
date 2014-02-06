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

#include "clang/AST/Decl.h"
#include "clang/Basic/AddressSpaces.h"

#include <sstream>
#include <cctype>

namespace {
    UintList generateWidths(unsigned start, unsigned stop)
    {
	UintList values;
	unsigned value = start;
	while (value != stop) {
	    values.push_back(value);
	    value <<= 1;
	}
	values.push_back(value);
	return values;
    }

    template<typename Container, typename T>
    Container operator+(const Container& list, T value)
    {
	Container l(list);
	l.push_back(value);
	return l;
    }
}

WebCLConfiguration::WebCLConfiguration()
    : typePrefix_("_Wcl")
    , variablePrefix_("_wcl")
    , macroPrefix_("_WCL")
    , functionPrefix_("_wcl")

    , minSuffix_("min")
    , maxSuffix_("max")
    , indentation_("    ")
    , sizeParameterType_("ulong")

    , privateAddressSpace_("private")
    , localAddressSpace_("local")
    , constantAddressSpace_("constant")
    , globalAddressSpace_("global")

    , privateRecordType_(typePrefix_ + "Privates")
    , localRecordType_(typePrefix_ + "Locals")
    , constantRecordType_(typePrefix_ + "Constants")
    , globalRecordType_(typePrefix_ + "Globals")
    , addressSpaceRecordType_(typePrefix_ + "ProgramAllocations")

    , localLimitsType_(typePrefix_ + "LocalLimits")
    , constantLimitsType_(typePrefix_ + "ConstantLimits")
    , globalLimitsType_(typePrefix_ + "GlobalLimits")

    , localMinField_(variablePrefix_ + "_locals_min")
    , localMaxField_(variablePrefix_ + "_locals_max")
    , constantMinField_(variablePrefix_ + "_constant_allocations_min")
    , constantMaxField_(variablePrefix_ + "_constant_allocations_max")

    , privatesField_("pa")
    , localLimitsField_("ll")
    , constantLimitsField_("cl")
    , globalLimitsField_("gl")

    , localRecordName_(variablePrefix_ + "_locals")
    , constantRecordName_(variablePrefix_ + "_constant_allocations")
    , programRecordName_(variablePrefix_ + "_allocations_allocation")
    , addressSpaceRecordName_(variablePrefix_ + "_allocs")

    , nullType_("uint")
    , privateNullField_("pn")
    , localNullField_("ln")
    , constantNullField_("cn")
    , globalNullField_("gn")

    , localRangeZeroingMacro_(macroPrefix_ + "_LOCAL_RANGE_INIT")

    , dataWidths_(generateWidths(2, 16) + 3)
    , roundingModes_(StringList() + "rte" + "rtz" + "rtp" + "rtn")

    , atomicOperations1_(StringList() + "atomic_inc" + "atomic_dec")
    , atomicOperations2_(StringList() + "atomic_add" + "atomic_sub" + "atomic_xchg" + "atomic_min" + "atomic_max" + "atomic_and" + "atomic_or" + "atomic_xor")
    , atomicOperations3_(StringList() + "atomic_cmpxchg")

    , localVariableRenamer_(variablePrefix_ + "_", "_")
    , privateVariableRenamer_(variablePrefix_ + "_", "_")
    , typedefRenamer_("", "")
    , anonymousStructureRenamer_(typePrefix_, "")
{
}

WebCLConfiguration::~WebCLConfiguration()
{
}

const std::string WebCLConfiguration::getNameOfAddressSpace(clang::QualType type) const
{
    return getNameOfAddressSpace(type.getAddressSpace());
}

const std::string WebCLConfiguration::getNameOfAddressSpace(unsigned addressSpaceNumber) const
{
    switch (addressSpaceNumber) {
      case clang::LangAS::opencl_global:
        return globalAddressSpace_;
      case clang::LangAS::opencl_local:
        return localAddressSpace_;
      case clang::LangAS::opencl_constant:
        return constantAddressSpace_;
      default:
        return privateAddressSpace_;
      }
}

const std::string WebCLConfiguration::getNameOfAddressSpaceNull (unsigned addressSpaceNum) const
{
      return variablePrefix_ + "_" + getNameOfAddressSpace(addressSpaceNum) + "_null";
}

const std::string WebCLConfiguration::getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNum) const
{
    const std::string prefix = addressSpaceRecordName_ + "->";

    switch (addressSpaceNum)
    {
    case clang::LangAS::opencl_global:
        return prefix + globalNullField_;
    case clang::LangAS::opencl_constant:
        return prefix + constantNullField_;
    case clang::LangAS::opencl_local:
        return prefix + localNullField_;
    }

    return prefix + privateNullField_;
}

const std::string WebCLConfiguration::getIdentifierForString(std::string str) const
{
    std::string result;
    for (unsigned c = 0; c < str.size(); ++c) {
        switch (str[c]) {
        case ' ': {
            result += "__";
        } break;
        case '_': {
            result += "_u";
        } break;
        case 'P': {
            result += "PP";
        } break;
        case '*': {
            result += "Ptr";
        } break;
        default:
            result += str[c];
        }
    }
    // no additional underscore at the end
    return result;
}

const std::string WebCLConfiguration::getExtensionDefineName(std::string extension) const
{
    //std::string define = macroPrefix_ + "_EXTENSION_";
    std::string define = "_W2CL_EXTENSION_";
    for (std::string::const_iterator it = extension.begin();
         it != extension.end();
         ++it) {
        define += std::toupper(*it);
    }
    return define;
}

const std::string WebCLConfiguration::getNameOfLimitClampFunction(
    unsigned addressSpaceNum, int limitCount, std::string type) const
{
    std::stringstream result;
    result << functionPrefix_ << "_addr_clamp_" << getNameOfAddressSpace(addressSpaceNum) << "_" << limitCount << "_" << getIdentifierForString(type);
    return result.str();
}

const std::string WebCLConfiguration::getNameOfLimitCheckFunction(
    unsigned addressSpaceNum, int limitCount, std::string type) const
{
    std::stringstream result;
    result << functionPrefix_ << "_addr_check_" << getNameOfAddressSpace(addressSpaceNum) << "_" << limitCount << "_" << getIdentifierForString(type);
    return result.str();
}

const std::string WebCLConfiguration::getNameOfSizeMacro(const std::string &asName) const
{
  const std::string name =
  macroPrefix_ + "_ADDRESS_SPACE_" + asName + "_MIN";
  return name;
}

const std::string WebCLConfiguration::getNameOfSizeMacro(unsigned addressSpaceNum) const
{
  return getNameOfSizeMacro(getNameOfAddressSpace(addressSpaceNum));
}

const std::string WebCLConfiguration::getNameOfAlignMacro(const std::string &asName) const
{
  const std::string name =
  macroPrefix_ + "_ADDRESS_SPACE_" + asName + "_ALIGNMENT";
  return name;
}

const std::string WebCLConfiguration::getNameOfAlignMacro(unsigned addressSpaceNum) const
{
  return getNameOfAlignMacro(getNameOfAddressSpace(addressSpaceNum));
}

const std::string WebCLConfiguration::getNameOfLimitMacro() const
{
    return macroPrefix_ + "_LAST";
}

const std::string WebCLConfiguration::getNameOfType(clang::QualType type) const
{
    return type.getUnqualifiedType().getAsString();
}

const std::string WebCLConfiguration::getNameOfSizeParameter(const std::string &arrayParamName) const
{
    return variablePrefix_ + "_" + arrayParamName + "_size";
}

const std::string WebCLConfiguration::getNameOfAnonymousStructure(const clang::RecordDecl *decl)
{
    static const std::string name = "Struct";

    std::ostringstream out;
    anonymousStructureRenamer_.generate(out, decl, name);
    return out.str();
}

const std::string WebCLConfiguration::getNameOfRelocatedTypeDecl(const clang::NamedDecl *decl)
{
    std::ostringstream out;

    typedefRenamer_.rename(out, decl);

    // Indicate error if renaming is needed.
    const std::string original = decl->getName();
    const std::string renamed = out.str();
    if (original.compare(renamed))
        return std::string();

    // Renaming wasn't needed.
    return original;
}

const std::string WebCLConfiguration::getNameOfRelocatedVariable(const clang::VarDecl *decl)
{
    std::ostringstream out;

    switch (decl->getType().getAddressSpace()) {
    case 0:
        privateVariableRenamer_.rename(out, decl);
        break;
    case clang::LangAS::opencl_local:
        localVariableRenamer_.rename(out, decl);
        break;
    default:
        out << decl->getName().str();
        break;
    }

    return out.str();
}

const std::string WebCLConfiguration::getNameOfLimitField(
    const clang::VarDecl *decl, bool isMax) const
{
    std::ostringstream out;

    const clang::FunctionDecl *function =
        llvm::dyn_cast<clang::FunctionDecl>(decl->getParentFunctionOrMethod());
    if (function)
        out << function->getName().str() << "__";

    out << decl->getName().str() << "_" << (isMax ? maxSuffix_ : minSuffix_);

    return out.str();
}

const std::string WebCLConfiguration::getReferenceToRelocatedVariable(const clang::VarDecl *decl)
{
  std::string prefix;

  switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_constant:
      prefix = constantRecordName_ + ".";
      break;
    case clang::LangAS::opencl_local:
      prefix = localRecordName_ + ".";
      break;
    case clang::LangAS::opencl_global:
        assert(false &&
               "There can't be relocated variables in global address space.");
        break;
    default:
      assert((decl->getType().getAddressSpace() == 0) &&
             "Expected private address space.");
      prefix = addressSpaceRecordName_ + "->" + privatesField_ + ".";
      break;
  }

  return prefix + getNameOfRelocatedVariable(decl);
}

const std::string WebCLConfiguration::getIndentation(unsigned int levels) const
{
    std::string indentation;
    for (unsigned int i = 0; i < levels; ++i)
        indentation.append(indentation_);
    return indentation;
}

const std::string WebCLConfiguration::getStaticLimitRef(unsigned addressSpaceNum, std::string cast) const
{
    std::string prefix = addressSpaceRecordName_ + "->";

    switch (addressSpaceNum) {
    case clang::LangAS::opencl_constant:
        prefix += constantLimitsField_ + ".";
        return cast + prefix + constantMinField_ + ", " + cast + prefix + constantMaxField_;

    case clang::LangAS::opencl_local:
        prefix += localLimitsField_ + ".";
        return cast + prefix + localMinField_ + ", " + cast + prefix + localMaxField_;

    case clang::LangAS::opencl_global:
        assert(false && "There can't be static allocations in global address space.");
        return "0, 0";

    default:
        prefix += privatesField_;
        return cast + "&" + prefix + ", " + cast + "(&" + prefix + " + 1)";
    }
}

const std::string WebCLConfiguration::getDynamicLimitRef(const clang::VarDecl *decl, std::string cast) const
{
    std::string prefix = addressSpaceRecordName_ + "->";

    switch (decl->getType().getTypePtr()->getPointeeType().getAddressSpace()) {
    case clang::LangAS::opencl_global:
        prefix += globalLimitsField_;
        break;
    case clang::LangAS::opencl_constant:
        prefix += constantLimitsField_;
        break;
    case clang::LangAS::opencl_local:
        prefix += localLimitsField_;
        break;
    default:
        assert(false && "There can't be dynamic limits of private address space.");
        break;
    }

    prefix += ".";

    std::stringstream retVal;
    retVal << cast << prefix << getNameOfLimitField(decl, false) << ", "
           << cast << prefix << getNameOfLimitField(decl, true);
    return retVal.str();
}

const std::string WebCLConfiguration::getNullLimitRef(unsigned addressSpaceNum) const
{
    assert((addressSpaceNum == clang::LangAS::opencl_local) &&
           "Expected local address space.");
    const std::string localNull = getNameOfAddressSpaceNull(addressSpaceNum);
    return localNull + ", " + localNull + " + " + getNameOfSizeMacro(addressSpaceNum);
}

