#ifndef WEBCLVALIDATOR_WEBCLACTION
#define WEBCLVALIDATOR_WEBCLACTION

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

#include "WebCLConsumer.hpp"
#include "WebCLConfiguration.hpp"
#include "WebCLVisitor.hpp"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendAction.h"

#include <set>
#include <string>

namespace clang {
    class Sema;
}

class WebCLReporter;
class WebCLPreprocessor;
class WebCLTransformer;

/// Abstract base class for an action performed by a validation stage.
class WebCLAction : public clang::FrontendAction
{
public:

    /// Constructor. If output filename isn't given, standard output
    /// is used.
    explicit WebCLAction(const char *output = NULL);
    virtual ~WebCLAction();

    void setExtensions(const std::set<std::string> &extensions);
    void setUsedExtensionsStorage(std::set<std::string> *usedExtensions);

protected:

    /// Initializes the action state. Full initialization isn't done
    /// in constructor, because the compiler instance isn't
    /// necessarily available at construction time.
    virtual bool initialize(clang::CompilerInstance &instance);

    /// Error reporting functionality.
    WebCLReporter *reporter_;
    // Preprocessing callbacks may be needed also when AST is
    // parsed. For example, include directives are handled during
    // preprocessing and pragmas during AST parsing.
    WebCLPreprocessor *preprocessor_;
    // Additional OpenCL extensions to allow in preprocessing besides cl_khr_initialize_memory
    std::set<std::string> extensions_;
    // If not null, used extensions are stored here
    std::set<std::string> *usedExtensions_;
    /// Output filename.
    const char *output_;
    /// Stream corresponding to the output filename.
    llvm::raw_ostream *out_;
};

/// Performs used extension detection
class WebCLFindUsedExtensionsAction : public WebCLAction
{
public:
    explicit WebCLFindUsedExtensionsAction();
    virtual ~WebCLFindUsedExtensionsAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

protected:
    /// \see WebCLAction
    virtual bool initialize(clang::CompilerInstance &instance);

    /// Finds matches from AST.
    clang::ast_matchers::MatchFinder finder_;

    /// Runs matchers when AST has been parsed.
    clang::ASTConsumer *consumer_;
};

/// Performs preprocessing stage only. Doesn't parse AST.
class WebCLPreprocessorAction : public WebCLAction
{
public:

    explicit WebCLPreprocessorAction(const char *output, std::string &builtinDecls);
    virtual ~WebCLPreprocessorAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

private:

    /// Where to store additional builtin function forward declarations
    /// needed by later AST parsing passes
    std::string &builtinDecls_;
};

/// A base class for stages that use AST matchers for consuming ASTs.
class WebCLMatcherAction : public WebCLAction
{
public:

    explicit WebCLMatcherAction(const char *output);
    virtual ~WebCLMatcherAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

protected:

    /// \see WebCLAction
    virtual bool initialize(clang::CompilerInstance &instance);

    /// Finds matches from AST.
    clang::ast_matchers::MatchFinder finder_;
    /// Runs matchers when AST has been parsed.
    clang::ASTConsumer *consumer_;
    /// Stores transformations.
    clang::Rewriter *rewriter_;
    /// Interface for naming conventions.
    WebCLConfiguration cfg_;
    /// Outputs stored transformations.
    WebCLPrinter *printer_;
};

/// Performs early normalizations:
///
/// - Complains about illegal identifiers.
/// - A name is generated for anonymous and nameless structures.
class WebCLMatcher1Action : public WebCLMatcherAction
{
public:

    explicit WebCLMatcher1Action(const char *output);
    virtual ~WebCLMatcher1Action();

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

private:

    /// \return Whether there are no illegal identifiers in the input.
    ///
    /// - Identifiers must not exceed 255 characters.
    /// - Identifiers reserved for validations may not be used.
    bool checkIdentifiers();
};

/// Performs late normalizations:
///
/// - Structure definitions are separated from variable declarations.
class WebCLMatcher2Action : public WebCLMatcherAction
{
public:

    explicit WebCLMatcher2Action(const char *output);
    virtual ~WebCLMatcher2Action();

    /// \see clang::FrontendAction
    virtual void ExecuteAction();
};

/// Runs memory validation algorithm after normalizations have been
/// performed.
class WebCLValidatorAction : public WebCLAction
{
public:

    WebCLValidatorAction(std::string &validatedSource, WebCLAnalyser::KernelList &kernels);
    virtual ~WebCLValidatorAction();

    /// \see clang::FrontendAction
    virtual clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance &instance,
                                                  llvm::StringRef);

    /// \see clang::FrontendAction
    virtual void ExecuteAction();

    /// \see clang::FrontendAction
    virtual bool usesPreprocessorOnly() const;

private:

    /// \see WebCLAction
    virtual bool initialize(clang::CompilerInstance &instance);

    /// Traverses back and forth AST nodes after AST has been parsed.
    WebCLConsumer *consumer_;
    /// Creates transformations.
    WebCLTransformer *transformer_;
    /// Stores transformations.
    clang::Rewriter *rewriter_;
    /// Component for generating AST nodes as AST is being parsed. We
    /// don't really need to create this component ourselves, but if
    /// we do, using internal clang components, e.g. in order to
    /// manipulate AST nodes, becomes easier.
    clang::Sema *sema_;
    /// Where to store transformed source after validation is complete.
    std::string &validatedSource_;
    /// Ditto for kernel info
    WebCLAnalyser::KernelList &kernels_;
};

#endif // WEBCLVALIDATOR_WEBCLACTION
