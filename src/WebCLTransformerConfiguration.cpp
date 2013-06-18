#include "WebCLTransformerConfiguration.hpp"

#include "clang/AST/Decl.h"
#include "clang/Basic/AddressSpaces.h"

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
    , localRecordName_("wcl_ls")
    , constantAddressSpace_("constant")
    , constantRecordType_("WclConstants")
    , constantField_("constants")
    , constantRecordName_("wcl_cs")
    , globalAddressSpace_("global")
    , globalRecordType_("WclGlobals")
    , globalField_("globals")
    , globalRecordName_("wcl_gs")
    , addressSpaceRecordType_("WclAddressSpaces")
    , addressSpaceRecordName_("wcl_as")
{
}

WebCLTransformerConfiguration::~WebCLTransformerConfiguration()
{
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpace(clang::QualType type) const
{
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            return globalAddressSpace_;
        case clang::LangAS::opencl_local:
            return localAddressSpace_;
        case clang::LangAS::opencl_constant:
            return constantAddressSpace_;
        default:
            return invalid_;
        }
    }

    return privateAddressSpace_;
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
    return decl->getName();
}

const std::string WebCLTransformerConfiguration::getIndentation(unsigned int levels) const
{
    std::string indentation;
    for (unsigned int i = 0; i < levels; ++i)
        indentation.append(indentation_);
    return indentation;
}
