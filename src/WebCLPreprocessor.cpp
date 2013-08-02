#include "WebCLPreprocessor.hpp"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Frontend/CompilerInstance.h"

namespace {
    char const * const ClKhrInitializeMemoryStr = "cl_khr_initialize_memory";
}

WebCLPreprocessor::WebCLPreprocessor(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , clang::PPCallbacks()
    , instance_(instance)
    , extensions_()
{
    extensions_.insert("cl_khr_fp64");
    extensions_.insert("cl_khr_fp16");
    extensions_.insert("cl_khr_gl_sharing");
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
        error(NameLoc, "WebCL doesn't support enabling '%0' extension.\n") << name;
    } else if (State == 0 && (name == ClKhrInitializeMemoryStr || name == "all")) {
        // TODO: For keyword "all", error should not be generated if the extension is not present.
        error(NameLoc, "WebCL program cannot disable extension %0.\n") << ClKhrInitializeMemoryStr;
    }
}
