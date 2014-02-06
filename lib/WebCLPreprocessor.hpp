#ifndef WEBCLVALIDATOR_WEBCLPREPROCESSOR
#define WEBCLVALIDATOR_WEBCLPREPROCESSOR

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

#include "WebCLReporter.hpp"

#include "clang/Lex/PPCallbacks.h"

namespace clang {
    class CompilerInstance;
}

/// \brief Preprocessor callbacks for WebCL C code.
class WebCLPreprocessor : public WebCLReporter
                        , public clang::PPCallbacks
{
public:
    explicit WebCLPreprocessor(clang::CompilerInstance &instance, const std::set<std::string> &extensions, std::set<std::string> *usedExtensions);
    ~WebCLPreprocessor();

    /// \brief Complain about include directives in main source files.
    /// Called before AST has been parsed.
    /// \see PPCallbacks::InclusionDirective
    virtual void InclusionDirective(
        clang::SourceLocation HashLoc, const clang::Token &IncludeTok, 
        llvm::StringRef FileName, bool IsAngled,
        clang::CharSourceRange FilenameRange, const clang::FileEntry *File,
        llvm::StringRef SearchPath, llvm::StringRef RelativePath,
        const clang::Module *Imported);

    /// \brief Complain about enabled or disabled OpenCL extensions.
    /// Called when AST is being parsed.
    /// \see PPCallbacks::PragmaOpenCLExtension
    virtual void PragmaOpenCLExtension(
        clang::SourceLocation NameLoc, const clang::IdentifierInfo *Name,
        clang::SourceLocation StateLoc, unsigned State);

private:

    /// OpenCL extensions that can be enabled.
    std::set<std::string> extensions_;

    /// OpenCL extensions that are actually requested; this can only include extensions from the extensions_ set
    std::set<std::string> *usedExtensions_;
};

#endif // WEBCLVALIDATOR_WEBCLPREPROCESSOR
