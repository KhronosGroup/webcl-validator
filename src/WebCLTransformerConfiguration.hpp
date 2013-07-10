#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION
#define WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION

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
    const std::string getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNum) const;
    const std::string getNameOfAddressSpace(clang::QualType type) const;
    const std::string getNameOfAddressSpaceRecord(clang::QualType type) const;
    const std::string getNameOfType(clang::QualType type) const;
    const std::string getNameOfPointerChecker(clang::QualType type) const;
    const std::string getNameOfIndexChecker(clang::QualType type) const;
    const std::string getNameOfIndexChecker() const;
    const std::string getNameOfSizeParameter(clang::ParmVarDecl *decl) const;
    const std::string getNameOfRelocatedVariable(const clang::VarDecl *decl)  const;
    const std::string getReferenceToRelocatedVariable(const clang::VarDecl *decl) const;
    const std::string getIndentation(unsigned int levels) const;

    const std::string getStaticLimitRef(unsigned addressSpaceNum) const;
    const std::string getDynamicLimitRef(const clang::VarDecl *decl) const;

    const std::string prefix_;
    const std::string pointerSuffix_;
    const std::string indexSuffix_;

    const std::string invalid_;
    const std::string indentation_;
    const std::string sizeParameterType_;

    const std::string privateAddressSpace_;
    const std::string privateRecordType_;
    const std::string privateField_;
    const std::string privateRecordName_;

    const std::string localAddressSpace_;
    const std::string localRecordType_;
    const std::string localField_;
    const std::string localRecordName_;

    const std::string constantAddressSpace_;
    const std::string constantRecordType_;
    const std::string constantField_;
    const std::string constantRecordName_;

    const std::string globalAddressSpace_;
    const std::string globalRecordType_;
    const std::string globalField_;
    const std::string globalRecordName_;

    const std::string addressSpaceRecordType_;
    const std::string addressSpaceRecordName_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMERCONFIGURATION
