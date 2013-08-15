#include "WebCLTransformerConfiguration.hpp"

#include "clang/AST/Decl.h"
#include "clang/Basic/AddressSpaces.h"

#include <sstream>

WebCLTransformerConfiguration::WebCLTransformerConfiguration()
    : typePrefix_("_Wcl")
    , variablePrefix_("_wcl")
    , macroPrefix_("_WCL")

    , minSuffix_("min")
    , maxSuffix_("max")
    , indentation_("    ")
    , sizeParameterType_("unsigned long")

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
{
}

WebCLTransformerConfiguration::~WebCLTransformerConfiguration()
{
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpace(clang::QualType type) const {
    return getNameOfAddressSpace(type.getAddressSpace());
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpace(unsigned addressSpaceNumber) const
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

const std::string WebCLTransformerConfiguration::getNameOfAddressSpaceNull(unsigned addressSpaceNum) const
{
    return variablePrefix_ + "_" + getNameOfAddressSpace(addressSpaceNum) + "_null";
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNum) const
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

const std::string WebCLTransformerConfiguration::getNameOfLimitCheckMacro(
    unsigned addressSpaceNum, int limitCount) const
{
    std::stringstream result;
    result << macroPrefix_ << "_ADDR_" << getNameOfAddressSpace(addressSpaceNum) << "_" << limitCount;
    return result.str();
}

const std::string WebCLTransformerConfiguration::getNameOfSizeMacro(const std::string &asName) const
{
  const std::string name =
  macroPrefix_ + "_ADDRESS_SPACE_" + asName + "_MIN";
  return name;
}

const std::string WebCLTransformerConfiguration::getNameOfSizeMacro(unsigned addressSpaceNum) const
{
  return getNameOfSizeMacro(getNameOfAddressSpace(addressSpaceNum));
}

const std::string WebCLTransformerConfiguration::getNameOfAlignMacro(const std::string &asName) const
{
  const std::string name =
  macroPrefix_ + "_ADDRESS_SPACE_" + asName + "_ALIGNMENT";
  return name;
}

const std::string WebCLTransformerConfiguration::getNameOfAlignMacro(unsigned addressSpaceNum) const
{
  return getNameOfAlignMacro(getNameOfAddressSpace(addressSpaceNum));
}

const std::string WebCLTransformerConfiguration::getNameOfLimitMacro() const
{
    return macroPrefix_ + "_LAST";
}

const std::string WebCLTransformerConfiguration::getNameOfType(clang::QualType type) const
{
    return type.getUnqualifiedType().getAsString();
}

const std::string WebCLTransformerConfiguration::getNameOfSizeParameter(clang::ParmVarDecl *decl) const
{
    const std::string name = decl->getName();
    return variablePrefix_ + "_" + name + "_size";
}

const std::string WebCLTransformerConfiguration::getNameOfRelocatedVariable(const clang::VarDecl *decl) const
{
  if (decl->getType().getAddressSpace() == clang::LangAS::opencl_constant) {
    return decl->getName().str();
  }
  
  const clang::FunctionDecl *func =
    llvm::dyn_cast<clang::FunctionDecl>(decl->getParentFunctionOrMethod());
  if (func) return func->getName().str() + "__" + decl->getName().str();
  return decl->getName().str();
}

const std::string WebCLTransformerConfiguration::getNameOfLimitField(
    const clang::VarDecl *decl, bool isMax) const
{
    const std::string name = getNameOfRelocatedVariable(decl);
    return name + "_" + (isMax ? maxSuffix_ : minSuffix_);
}

const std::string WebCLTransformerConfiguration::getReferenceToRelocatedVariable(const clang::VarDecl *decl) const
{
  std::string prefix;

  switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_constant:
      prefix = constantRecordName_ + ".";
      break;
    case clang::LangAS::opencl_local:
      prefix = localRecordName_ + ".";
      break;
    default:
      assert(decl->getType().getAddressSpace() == 0);
      prefix = addressSpaceRecordName_ + "->" + privatesField_ + ".";
      break;
  }

  return prefix + getNameOfRelocatedVariable(decl);
}

const std::string WebCLTransformerConfiguration::getIndentation(unsigned int levels) const
{
    std::string indentation;
    for (unsigned int i = 0; i < levels; ++i)
        indentation.append(indentation_);
    return indentation;
}

const std::string WebCLTransformerConfiguration::getStaticLimitRef(unsigned addressSpaceNum) const
{
    std::string prefix = addressSpaceRecordName_ + "->";

    switch (addressSpaceNum) {
    case clang::LangAS::opencl_constant:
        prefix += constantLimitsField_ + ".";
        return prefix + constantMinField_ + ", " + prefix + constantMaxField_;

    case clang::LangAS::opencl_local:
        prefix += localLimitsField_ + ".";
        return prefix + localMinField_ + ", " + prefix + localMaxField_;

    case clang::LangAS::opencl_global:
        assert(false && "There can't be static allocations in global address space.");

    default:
        prefix += privatesField_;
        return "&" + prefix + ", (&" + prefix + " + 1)";
    }
}

const std::string WebCLTransformerConfiguration::getDynamicLimitRef(const clang::VarDecl *decl) const
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
    }

    prefix += ".";

    std::stringstream retVal;
    retVal << prefix << getNameOfLimitField(decl, false) << ", "
           << prefix << getNameOfLimitField(decl, true);
    return retVal.str();
}

const std::string WebCLTransformerConfiguration::getNullLimitRef(unsigned addressSpaceNum) const
{
    assert(addressSpaceNum == clang::LangAS::opencl_local);
    const std::string localNull = getNameOfAddressSpaceNull(addressSpaceNum);
    return localNull + ", " + localNull + " + " + getNameOfSizeMacro(addressSpaceNum);
}
