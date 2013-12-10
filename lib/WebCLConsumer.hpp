#ifndef WEBCLVALIDATOR_WEBCLCONSUMER
#define WEBCLVALIDATOR_WEBCLCONSUMER

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

#include "WebCLPass.hpp"
#include "WebCLPrinter.hpp"
#include "WebCLVisitor.hpp"

#include "clang/AST/ASTConsumer.h"

/// Executes various validation passes that check the AST for errors,
/// analyze the AST and generate transformations based on the
/// analysis.
class WebCLConsumer : public clang::ASTConsumer
{
public:

    explicit WebCLConsumer(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter,
        WebCLTransformer &transformer);
    virtual ~WebCLConsumer();

    /// Runs passes in two stages. The first stage checks errors and
    /// analyzes the AST. The second stage runs transformation passes.
    ///
    /// \see ASTConsumer::HandleTranslationUnit
    virtual void HandleTranslationUnit(clang::ASTContext &context);

    /// Get transformed source
    const std::string &getTransformedSource() const;

    /// Get kernel info
    const WebCLAnalyser::KernelList &getKernels() const;

private:

    /// Runs all error checking and analysis passes.
    void checkAndAnalyze(clang::ASTContext &context);
    /// Runs all transformation passes.
    void transform(clang::ASTContext &context);

    /// \return Whether previous passes have reported errors.
    bool hasErrors(clang::ASTContext &context) const;

    /// Checks AST for WebCL restrictions.
    WebCLRestrictor restrictor_;
    /// Analyzes AST for transformation passes.
    WebCLAnalyser analyser_;
    /// Visitors that check the AST for errors or perform analysis for
    /// transformation passes.
    typedef std::vector<WebCLVisitor*> Visitors;
    Visitors visitors_;

    /// Transformation passes.
    WebCLInputNormaliser inputNormaliser_;
    WebCLAddressSpaceHandler addressSpaceHandler_;
    WebCLKernelHandler kernelHandler_;
    WebCLMemoryAccessHandler memoryAccessHandler_;
    WebCLValidatorPrinter printer_;
    WebCLImageSamplerSafetyHandler imageSampleSafetyHandler_;
    WebCLFunctionCallHandler functionCallHandler_;
    /// Passes that generate transformations based on analysis.
    typedef std::vector<WebCLPass*> Passes;
    Passes passes_;
};

#endif // WEBCLVALIDATOR_WEBCLCONSUMER
