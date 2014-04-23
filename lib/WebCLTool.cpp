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

#include "WebCLAction.hpp"
#include "WebCLTool.hpp"

#include "clang/Tooling/CompilationDatabase.h"

#include <iostream>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif


namespace {
    // TODO: use clang::tooling::FixedCompilationDatabase::loadFromCommandLine.
    // This is a copy of an older version of it from the 3.2 release.
    clang::tooling::FixedCompilationDatabase *
    loadFromCommandLine(CharPtrVector &Argv,
        clang::Twine Directory = clang::Twine(".")) {
        CharPtrVector::iterator DoubleDash = std::find(Argv.begin(), Argv.end(), clang::StringRef("--"));

        if (DoubleDash == Argv.end())
            return NULL;
        ++DoubleDash; // uh.. we don't use std::next, as it's c++11
        std::vector<std::string> CommandLine(DoubleDash, Argv.end());
        --DoubleDash;
        Argv.erase(DoubleDash, Argv.end());
        return new clang::tooling::FixedCompilationDatabase(Directory, CommandLine);
    }
}

WebCLTool::WebCLTool(const CharPtrVector &argv,
                     char const *input, char const *output)
    : compilations_(NULL), paths_(), usedExtensions_(NULL), tool_(NULL), output_(output)
{
    CharPtrVector argvCmdLine = argv;
    // FIXME: is it essential that the modified argv is used by the caller?
    compilations_ = loadFromCommandLine(argvCmdLine);

    paths_.push_back(input);
    tool_ = new clang::tooling::ClangTool(*compilations_, paths_);
    if (!tool_) {
        std::cerr << "Internal error. Can't create tool." << std::endl;
        return;
    }
}

WebCLTool::~WebCLTool()
{
    delete tool_;
    tool_ = NULL;
    delete compilations_;
    compilations_ = NULL;
}

void WebCLTool::setDiagnosticConsumer(clang::DiagnosticConsumer *diag)
{
    tool_->setDiagnosticConsumer(diag);
}

void WebCLTool::setExtensions(const std::set<std::string> &extensions)
{
    extensions_ = extensions;
}

void WebCLTool::setUsedExtensionsStorage(std::set<std::string> *usedExtensions)
{
    usedExtensions_ = usedExtensions;
}

int WebCLTool::run()
{
    if (!compilations_ || !tool_)
        return EXIT_FAILURE;
    return tool_->run(this);
}

WebCLPreprocessorTool::WebCLPreprocessorTool(const CharPtrVector &argv,
                                             char const *input, char const *output)
    : WebCLTool(argv, input, output)
{
}

WebCLPreprocessorTool::~WebCLPreprocessorTool()
{
}

clang::FrontendAction *WebCLPreprocessorTool::create()
{
    WebCLAction *action = new WebCLPreprocessorAction(output_, builtinDecls_);
    action->setExtensions(extensions_);
    action->setUsedExtensionsStorage(usedExtensions_);
    return action;
}

WebCLFindUsedExtensionsTool::WebCLFindUsedExtensionsTool(const CharPtrVector &argv,
    char const *input)
    : WebCLTool(argv, input)
{
}

WebCLFindUsedExtensionsTool::~WebCLFindUsedExtensionsTool()
{
}

clang::FrontendAction *WebCLFindUsedExtensionsTool::create()
{
    WebCLAction *action = new WebCLFindUsedExtensionsAction();
    action->setExtensions(extensions_);
    action->setUsedExtensionsStorage(usedExtensions_);
    return action;
}

WebCLMatcher1Tool::WebCLMatcher1Tool(const CharPtrVector &argv,
                                     char const *input, char const *output)
    : WebCLTool(argv, input, output)
{
}

WebCLMatcher1Tool::~WebCLMatcher1Tool()
{
}

clang::FrontendAction *WebCLMatcher1Tool::create()
{
    WebCLAction *action = new WebCLMatcher1Action(output_);
    action->setExtensions(extensions_);
    action->setUsedExtensionsStorage(usedExtensions_);
    return action;
}

WebCLMatcher2Tool::WebCLMatcher2Tool(const CharPtrVector &argv,
                                     char const *input, char const *output)
    : WebCLTool(argv, input, output)
{
}

WebCLMatcher2Tool::~WebCLMatcher2Tool()
{
}

clang::FrontendAction *WebCLMatcher2Tool::create()
{
    WebCLAction *action = new WebCLMatcher2Action(output_);
    action->setExtensions(extensions_);
    action->setUsedExtensionsStorage(usedExtensions_);
    return action;
}

WebCLValidatorTool::WebCLValidatorTool(const CharPtrVector &argv,
                                       char const *input)
    : WebCLTool(argv, input)
{
}

WebCLValidatorTool::~WebCLValidatorTool()
{
}

clang::FrontendAction *WebCLValidatorTool::create()
{
    WebCLAction *action = new WebCLValidatorAction(validatedSource_, kernels_);
    action->setExtensions(extensions_);
    action->setUsedExtensionsStorage(usedExtensions_);
    return action;
}
