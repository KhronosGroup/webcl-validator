#ifndef WEBCLVALIDATOR_WEBCLTOOL
#define WEBCLVALIDATOR_WEBCLTOOL

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

#include "clang/Tooling/Tooling.h"

#include "WebCLVisitor.hpp"
#include "WebCLCommon.hpp"

#include <string>
#include <vector>

namespace clang {
    class DiagnosticConsumer;

    namespace tooling {
        class FixedCompilationDatabase;
    }
}

class WebCLActionFactory;

/// Abstract base class for tools representing various validation
/// stages. Each tool accepts a WebCL C program as its input and
/// produces a transformed WebCL C program as its output. The inputs
/// and outputs are chained to form a pipeline of tools.
///
/// Validating in multiple stages makes the transformations of each
/// stage less complex.
///
/// \see WebCLAction
/// \see WebCLArguments
class WebCLTool : public clang::tooling::FrontendActionFactory
{
public:

    /// Constructor. Inputs and outputs are filenames. If output isn't
    /// given, standard output is used.
    WebCLTool(const CharPtrVector &argv,
              char const *input, char const *output = NULL);
    virtual ~WebCLTool();

    void setDiagnosticConsumer(clang::DiagnosticConsumer *diag);
    void setExtensions(const std::set<std::string> &extensions);
    // set the storage to return used extensions in; this approach is used to pierce through the layers
    void setUsedExtensionsStorage(std::set<std::string> *usedExtensions);

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create() = 0;

    /// Process input to produce transformed output.
    int run();

protected:

    /// Jobs to perform based on command line options.
    const clang::tooling::FixedCompilationDatabase *compilations_;
    /// Source file to process.
    std::vector<std::string> paths_;
    // Additional OpenCL extensions to allow in preprocessing besides cl_khr_initialize_memory
    std::set<std::string> extensions_;
    // If not null, actually used extensions are placed here
    std::set<std::string> *usedExtensions_;
    /// Tool representing a validation stage.
    clang::tooling::ClangTool* tool_;
    /// Target file for transformations.
    const char *output_;
};

/// Runs preprocessing stage. Takes the user source file as input.
///
/// \see WebCLPreprocessorAction
class WebCLPreprocessorTool : public WebCLTool
{
public:
    WebCLPreprocessorTool(const CharPtrVector &argv,
                          char const *input, char const *output);
    virtual ~WebCLPreprocessorTool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();

    /// Gets builtin function declarations to include in later stages
    const std::string &getBuiltinDecls() const { return builtinDecls_; }

private:
    /// Collects possibly required builtin function declarations to
    /// be included for later stages
    std::string builtinDecls_;
};

/// Runs first stage of AST matcher based transformations. Takes the
/// output of preprocessing stage as input.
///
/// \see WebCLMatcher1Action
class WebCLFindUsedExtensionsTool : public WebCLTool
{
public:
    WebCLFindUsedExtensionsTool(const CharPtrVector &argv,
        char const *input);
    virtual ~WebCLFindUsedExtensionsTool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs first stage of AST matcher based transformations. Takes the
/// output of preprocessing stage as input.
///
/// \see WebCLMatcher1Action
class WebCLMatcher1Tool : public WebCLTool
{
public:
    WebCLMatcher1Tool(const CharPtrVector &argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher1Tool();

    /// \brief see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs second stage of AST matcher based transformations. Takes the
/// output of previous AST matcher stage as input.
///
/// \see WebCLMatcher2Action
class WebCLMatcher2Tool : public WebCLTool
{
public:
    WebCLMatcher2Tool(const CharPtrVector &argv,
                      char const *input, char const *output);
    virtual ~WebCLMatcher2Tool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();
};

/// Runs memory access validation algorithm. Takes the output of last
/// AST matcher stage as input.
///
/// \see WebCLValidatorAction
class WebCLValidatorTool : public WebCLTool
{
public:
    WebCLValidatorTool(const CharPtrVector &argv,
                       char const *input);
    virtual ~WebCLValidatorTool();

    /// \see clang::tooling::FrontendActionFactory
    virtual clang::FrontendAction *create();

    /// Returns validated source after a successful run
    const std::string &getValidatedSource() const { return validatedSource_; }
    /// Ditto for kernel info
    const WebCLAnalyser::KernelList &getKernels() const { return kernels_; }

private:

    // Stores validated source after validation is complete.
    std::string validatedSource_;
    // ditto for kernels
    WebCLAnalyser::KernelList kernels_;
};

#endif // WEBCLVALIDATOR_WEBCLTOOL
