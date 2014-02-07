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

#include "WebCLPreprocessor.hpp"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Frontend/CompilerInstance.h"

namespace {
    char const * const ClKhrInitializeMemoryStr = "cl_khr_initialize_memory";
}

WebCLPreprocessor::WebCLPreprocessor(clang::CompilerInstance &instance, const std::set<std::string> &extensions, std::set<std::string> *usedExtensions)
    : WebCLReporter(instance)
    , clang::PPCallbacks()
    , extensions_(extensions)
    , usedExtensions_(usedExtensions)
{
    extensions_.insert(ClKhrInitializeMemoryStr);
}

WebCLPreprocessor::~WebCLPreprocessor()
{
}

void WebCLPreprocessor::InclusionDirective(
    clang::SourceLocation HashLoc, const clang::Token &IncludeTok,
    llvm::StringRef FileName, bool IsAngled,
    clang::CharSourceRange FilenameRange, const clang::FileEntry *File,
    llvm::StringRef SearchPath, llvm::StringRef RelativePath,
    const clang::Module *Imported)
{
    // We want to only complain about include directives in user
    // sources, not about include directives that are injected with
    // the -include option.
    if (!isFromMainFile(HashLoc))
        return;

    // The include directive was in user sources.
    error(HashLoc, "WebCL doesn't support the include directive.\n");
}

void WebCLPreprocessor::PragmaOpenCLExtension(
    clang::SourceLocation NameLoc, const clang::IdentifierInfo *Name,
    clang::SourceLocation StateLoc, unsigned State)
{
    llvm::StringRef name = Name->getName();
    if (State && !extensions_.count(name)) {
        error(NameLoc, "WebCL or platform doesn't support enabling '%0' extension.\n") << name;
    } else if (State == 0 && (name == ClKhrInitializeMemoryStr || name == "all")) {
        // cannot disable all extensions with webcl
        error(NameLoc, "WebCL program cannot disable extension %0.\n") << ClKhrInitializeMemoryStr;
    } else {
        if (usedExtensions_ && isFromMainFile(NameLoc)) {
            usedExtensions_->insert(name);
        }
    }
}
