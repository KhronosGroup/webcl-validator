#ifndef WEBCLVALIDATOR_WEBCLCONFIGURATION
#define WEBCLVALIDATOR_WEBCLCONFIGURATION

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

#include "WebCLRenamer.hpp"
#include "WebCLCommon.hpp"

#include "clang/AST/Type.h"

#include <string>

namespace clang {
    class ParmVarDecl;
    class RecordDecl;
    class TypedefDecl;
    class VarDecl;
}

/// A helper class for producing strings that occur repeatedly in
/// generated code.
class WebCLConfiguration
{
public:

    WebCLConfiguration();
    ~WebCLConfiguration();

    /// \return Address space name (with the leading "__" omitted).
    const std::string getNameOfAddressSpace(clang::QualType type) const;
    const std::string getNameOfAddressSpace(unsigned addressSpaceNum) const;

    /// \return Name of variable that reserves space for the largest
    /// memory reference in given address space.
    ///
    /// \see getNameOfAddressSpaceNullPtrRef
    const std::string getNameOfAddressSpaceNull(unsigned addressSpace) const;
    /// \return Reference to address space specific null pointer,
    /// i.e. the area that is big enough to contain the largest memory
    /// reference in that address space.
    ///
    /// \see getNameOfAddressSpaceNull
    const std::string getNameOfAddressSpaceNullPtrRef(unsigned addressSpaceNum) const;

    /// \return Name of macro that can be used to validate pointers. The
    /// generated macro with this name returns a valid pointer by making call to
    /// the macro of name getNameOfLimitCheckFunction.
    ///
    /// \see getStaticLimitRef
    /// \see getDynamicLimitRef
    /// \see getNullLimitRef
    /// \see getNameOfLimitCheckFunction
    const std::string getNameOfLimitClampFunction(
        unsigned addressSpaceNum, int limitCount, std::string type) const;
    /// \return Name of macro that can be used to validate pointers. The
    /// generated macro with this name returns a boolean indicating whether the
    /// access is permitted or not.
    ///
    /// \see getStaticLimitRef
    /// \see getDynamicLimitRef
    /// \see getNullLimitRef
    /// \see getNameOfLimitClampFunction
    const std::string getNameOfLimitCheckFunction(
        unsigned addressSpaceNum, int limitCount, std::string type) const;
    /// \return Name of macro that describes size of largest memory
    /// reference in given address space.
    const std::string getNameOfSizeMacro(const std::string &asName) const;
    const std::string getNameOfSizeMacro(unsigned addressSpaceNum) const;
    /// \return Name of macro that describes alignment for address
    /// space record.
    const std::string getNameOfAlignMacro(const std::string &asName) const;
    const std::string getNameOfAlignMacro(unsigned addressSpaceNum) const;
    /// \return Name of macro that calculates the last addressable
    /// location for some type.
    const std::string getNameOfLimitMacro() const;

    /// \return Type name without qualifiers.
    const std::string getNameOfType(clang::QualType type) const;
    /// \return Name of kernel parameter that contains the size of
    /// the array parameter with the given name.
    const std::string getNameOfSizeParameter(const std::string &arrayParamName) const;
    /// \return Name that should be generated for given anonymous or
    /// nameless structure.
    const std::string getNameOfAnonymousStructure(const clang::RecordDecl *decl);
    /// \return New name for type declaration that needs to be
    /// relocated.
    const std::string getNameOfRelocatedTypeDecl(const clang::NamedDecl *decl);
    /// \return New name for variable declaration that needs to be
    /// relocated.
    const std::string getNameOfRelocatedVariable(const clang::VarDecl *decl);
    /// \return Name of field in a limit structure describing a
    /// minimum or maximum value of some static or dynamic memory
    /// area.
    const std::string getNameOfLimitField(const clang::VarDecl *decl, bool isMax) const;
    /// \return Reference to a variable that was relocated to an
    /// address space record.
    const std::string getReferenceToRelocatedVariable(const clang::VarDecl *decl);
    /// \return The default whitespace sequence repeated the given
    /// number of times.
    const std::string getIndentation(unsigned int levels) const;

    /// \return Minimum and maximum limits of an address space
    /// structure.
    const std::string getStaticLimitRef(unsigned addressSpaceNum, std::string cast = "") const;
    /// \return Minimum and maximum limits of a memory object passed
    /// to a kernel.
    const std::string getDynamicLimitRef(const clang::VarDecl *decl, std::string cast = "") const;
    /// \return Minimum and maximum limits of a null memory area.
    const std::string getNullLimitRef(unsigned addressSpaceNum) const;

    /// \return Stripped version of str in purpose of using it as an identifier
    /// Currently only handles spaces and asterisks. The generated sequences
    /// should be so that the user is not able to generate colliisions
    /// by choosing identifiers in a certain way.
    const std::string getIdentifierForString(std::string str) const;

    /// \return the name of the #define associated with the extension. _WCL_EXTENSION_(extension name in uppercase)
    const std::string getExtensionDefineName(std::string extension) const;

    /// Prefixes for generated types, variables and macros.
    const std::string typePrefix_;
    const std::string variablePrefix_;
    const std::string macroPrefix_;
    const std::string functionPrefix_;

    /// Suffixes for memory area lower and upper bounds.
    const std::string minSuffix_;
    const std::string maxSuffix_;
    /// Basic whitespace sequence for single indentation level.
    const std::string indentation_;
    /// Type of size parameters generated for memory objects passed to
    /// kernels.
    const std::string sizeParameterType_;

    /// Address space names without the "__" prefix.
    const std::string privateAddressSpace_;
    const std::string localAddressSpace_;
    const std::string constantAddressSpace_;
    const std::string globalAddressSpace_;

    /// Names of address space structure types.
    const std::string privateRecordType_;
    const std::string localRecordType_;
    const std::string constantRecordType_;
    const std::string globalRecordType_;
    /// Name of highest level structure type containing address space
    /// structures and limits.
    const std::string addressSpaceRecordType_;

    /// Names of structure types containing limits of address space
    /// structures as well as memory objects passed to kernels.
    const std::string localLimitsType_;
    const std::string constantLimitsType_; 
    const std::string globalLimitsType_;

    /// Limits of address space records for address spaces that may
    /// have both static and dynamic limits. Privates have only static
    /// limits, globals have only dynamic limits, but locals and
    /// constants may have both.
    const std::string localMinField_;
    const std::string localMaxField_;
    const std::string constantMinField_;
    const std::string constantMaxField_;

    /// Fields of the main allocation structure needed for determining
    /// memory area limits.
    const std::string privatesField_;
    const std::string localLimitsField_;
    const std::string constantLimitsField_;
    const std::string globalLimitsField_;

    /// Names of address space structure instances that won't be
    /// embedded directly into the main allocation structure.
    const std::string localRecordName_;
    const std::string constantRecordName_;
    /// Name used to declare main allocation structure.
    const std::string programRecordName_;
    /// Name used to refer to main allocation structure.
    const std::string addressSpaceRecordName_;

    /// Basic unit of null areas.
    const std::string nullType_;
    /// Pointers to null areas of each address space.
    const std::string privateNullField_;
    const std::string localNullField_;
    const std::string constantNullField_;
    const std::string globalNullField_;

    /// Name of macro for zeroing local memory areas.
    const std::string localRangeZeroingMacro_;

    // List of data widths: 2, 4, 8, 16
    const UintList dataWidths_;

    /// List of rounding modes for vstore_half: rte, rtz, rpt, rtn
    const StringList roundingModes_;

    /// List of single-argument atomic (not atom) operations (inc, dec)
    const StringList atomicOperations1_;

    /// List of two-argument atomic operations (not atom) (add, sub, xchg, min, max, and, or, xor)
    const StringList atomicOperations2_;

    /// List of three-argument atomic operations (not atom) (cmpxchg)
    const StringList atomicOperations3_;

private:

    /// Renamer of variables relocated to local address space
    /// structure.
    WebCLRenamer localVariableRenamer_;
    /// Renamer of variables relocated to private address space
    /// structure.
    WebCLRenamer privateVariableRenamer_;
    /// Renamer of relocated typedefs.
    WebCLRenamer typedefRenamer_;
    /// Name generator for anonymous or nameless structures.
    WebCLRenamer anonymousStructureRenamer_;
};

#endif // WEBCLVALIDATOR_WEBCLCONFIGURATION
