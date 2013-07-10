#include "WebCLTransformerConfiguration.hpp"

#include "clang/AST/Decl.h"
#include "clang/Basic/AddressSpaces.h"

#include <sstream>

WebCLTransformerConfiguration::WebCLTransformerConfiguration()
    : prefix_("wcl")
    , pointerSuffix_("ptr")
    , indexSuffix_("idx")
    , invalid_("???")
    , indentation_("    ")
    , sizeParameterType_("unsigned long")
    , privateAddressSpace_("private")
    , privateRecordType_("WclPrivates")
    , privateField_("privates")
    , privateRecordName_("wcl_ps")
    , localAddressSpace_("local")
    , localRecordType_("WclLocals")
    , localField_("locals")
    , localRecordName_("wcl_locals")
    , constantAddressSpace_("constant")
    , constantRecordType_("WclConstants")
    , constantField_("constants")
    , constantRecordName_("wcl_cs")
    , globalAddressSpace_("global")
    , globalRecordType_("WclGlobals")
    , globalField_("globals")
    , globalRecordName_("wcl_gs")
    , addressSpaceRecordType_("WclProgramAllocations")
    , addressSpaceRecordName_("wcl_allocs")
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

const std::string WebCLTransformerConfiguration::getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNumber) const
{
    // TODO: implement me! Probably we need to inject null to each address space struct
    return "wcl_allocs";
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpaceRecord(clang::QualType type) const
{
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            return globalRecordName_;
        case clang::LangAS::opencl_local:
            return localRecordName_;
        case clang::LangAS::opencl_constant:
            return constantRecordName_;
        default:
            return invalid_;
        }
    }

    return privateRecordName_;
}

const std::string WebCLTransformerConfiguration::getNameOfType(clang::QualType type) const
{
    return type.getUnqualifiedType().getAsString();
}

const std::string WebCLTransformerConfiguration::getNameOfPointerChecker(clang::QualType type) const
{
    return prefix_ + "_" +
        getNameOfAddressSpace(type) + "_" + getNameOfType(type) +
        "_" + pointerSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfIndexChecker(clang::QualType type) const
{
    return prefix_ + "_" +
        getNameOfAddressSpace(type) + "_" + getNameOfType(type) +
        "_" + indexSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfIndexChecker() const
{
    return prefix_ + "_" + indexSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfSizeParameter(clang::ParmVarDecl *decl) const
{
    const std::string name = decl->getName();
    return prefix_ + "_" + name + "_size";
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

const std::string WebCLTransformerConfiguration::getReferenceToRelocatedVariable(const clang::VarDecl *decl) const
{
  std::string prefix;

  switch (decl->getType().getAddressSpace()) {
    case clang::LangAS::opencl_constant:
      prefix = "wcl_constant_allocations.";
      break;
    case clang::LangAS::opencl_local:
      prefix = "wcl_locals.";
      break;
    default:
      assert(decl->getType().getAddressSpace() == 0);
      prefix = "wcl_allocs->pa.";
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

const std::string WebCLTransformerConfiguration::getStaticLimitRef(unsigned addressSpaceNumber) const
{
  switch (addressSpaceNumber) {
    case clang::LangAS::opencl_constant:
      return "wcl_allocs->cl.wcl_constant_allocations_min,wcl_allocs->cl.wcl_constant_allocations_max";
    case clang::LangAS::opencl_local:
      return "wcl_allocs->ll.wcl_locals_min,wcl_allocs->ll.wcl_locals_max";
    case clang::LangAS::opencl_global:
      assert(false && "There can't be static allocations in global address space.");
    default:
      return "&wcl_allocs->pa,(&wcl_allocs->pa + 1)";
  }
}

const std::string WebCLTransformerConfiguration::getDynamicLimitRef(const clang::VarDecl *decl) const
{
    std::stringstream retVal;
    std::string varName = getNameOfRelocatedVariable(decl);
    std::string prefix;
  
    switch (decl->getType().getTypePtr()->getPointeeType().getAddressSpace()) {
    case clang::LangAS::opencl_global:
      prefix = "wcl_allocs->gl.";
      break;
    case clang::LangAS::opencl_constant:
      prefix = "wcl_allocs->cl.";
      break;
    case clang::LangAS::opencl_local:
      prefix = "wcl_allocs->ll.";
      break;
    default:
      assert(false && "There can't be dynamic limits of private address space.");
    }

    retVal << prefix << varName << "_min," << prefix << varName << "_max";
    return retVal.str();
}




