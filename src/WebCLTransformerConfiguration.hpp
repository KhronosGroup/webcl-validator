#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION
#define WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION

#include "WebCLRenamer.hpp"

#include "clang/AST/Type.h"

#include <string>

namespace clang {
    class ParmVarDecl;
    class VarDecl;
}

/// \brief A helper structure that configures strings that occur
/// repeatedly in generated code.
class WebCLTransformerConfiguration
{
public:

    WebCLTransformerConfiguration();
    ~WebCLTransformerConfiguration();

    const std::string getNameOfAddressSpace(unsigned addressSpaceNum) const;
    const std::string getNameOfAddressSpaceNull(unsigned addressSpace) const;
    const std::string getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNum) const;
    const std::string getNameOfLimitCheckMacro(
        unsigned addressSpaceNum, int limitCount) const;
    const std::string getNameOfSizeMacro(const std::string &asName) const;
    const std::string getNameOfSizeMacro(unsigned addressSpaceNum) const;
    const std::string getNameOfAlignMacro(const std::string &asName) const;
    const std::string getNameOfAlignMacro(unsigned addressSpaceNum) const;
    const std::string getNameOfLimitMacro() const;
    const std::string getNameOfAddressSpace(clang::QualType type) const;
    const std::string getNameOfType(clang::QualType type) const;
    const std::string getNameOfSizeParameter(clang::ParmVarDecl *decl) const;
    const std::string getNameOfRelocatedVariable(const clang::VarDecl *decl);
    const std::string getNameOfLimitField(const clang::VarDecl *decl, bool isMax) const;
    const std::string getReferenceToRelocatedVariable(const clang::VarDecl *decl);
    const std::string getIndentation(unsigned int levels) const;

    const std::string getStaticLimitRef(unsigned addressSpaceNum) const;
    const std::string getDynamicLimitRef(const clang::VarDecl *decl) const;
    const std::string getNullLimitRef(unsigned addressSpaceNum) const;

    const std::string typePrefix_;
    const std::string variablePrefix_;
    const std::string macroPrefix_;

    const std::string minSuffix_;
    const std::string maxSuffix_;
    const std::string indentation_;
    const std::string sizeParameterType_;

    const std::string privateAddressSpace_;
    const std::string localAddressSpace_;
    const std::string constantAddressSpace_;
    const std::string globalAddressSpace_;

    const std::string privateRecordType_;
    const std::string localRecordType_;
    const std::string constantRecordType_;
    const std::string globalRecordType_;
    const std::string addressSpaceRecordType_;

    const std::string localLimitsType_;
    const std::string constantLimitsType_; 
    const std::string globalLimitsType_;

    const std::string localMinField_;
    const std::string localMaxField_;
    const std::string constantMinField_;
    const std::string constantMaxField_;

    const std::string privatesField_;
    const std::string localLimitsField_;
    const std::string constantLimitsField_;
    const std::string globalLimitsField_;

    const std::string localRecordName_;
    const std::string constantRecordName_;
    const std::string programRecordName_;
    const std::string addressSpaceRecordName_;

    const std::string nullType_;
    const std::string privateNullField_;
    const std::string localNullField_;
    const std::string constantNullField_;
    const std::string globalNullField_;

    const std::string localRangeZeroingMacro_;

private:

    WebCLRenamer localVariableRenamer_;
    WebCLRenamer privateVariableRenamer_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION
