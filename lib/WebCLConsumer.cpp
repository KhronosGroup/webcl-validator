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
#include "WebCLHelper.hpp"
#include "WebCLPass.hpp"

#include "clang/AST/ASTContext.h"

#include "WebCLDebug.hpp"

WebCLConsumer::WebCLConsumer(
    clang::CompilerInstance &instance, clang::Rewriter &rewriter,
    WebCLTransformer &transformer)
    : clang::ASTConsumer()
    , restrictor_(instance)
    , analyser_(instance)
    , visitors_()
    , inputNormaliser_(instance, analyser_, transformer)
    , addressSpaceHandler_(instance, analyser_, transformer)
    , kernelHandler_(instance, analyser_, transformer, addressSpaceHandler_)
    , memoryAccessHandler_(instance, analyser_, transformer, kernelHandler_)
    , printer_(instance, rewriter, analyser_, transformer)
    , imageSampleSafetyHandler_(instance, analyser_, transformer, kernelHandler_)
    , functionCallHandler_(instance, analyser_, transformer, kernelHandler_)
    , passes_()
{
    visitors_.push_back(&restrictor_);

    // Collects information about nodes.
    visitors_.push_back(&analyser_);

    // Checks that when image types are being used, they always originate from
    // function parameters
    passes_.push_back(&imageSampleSafetyHandler_);

    // Moves all typedef and struct declarations to start of module
    // to make sure that they are available when address space types are
    // declared.
    passes_.push_back(&inputNormaliser_);

    // Collects all the variables that should be moved to address space
    // structures and writes out typedefs for address spaces.
    // Also replaces original references to declarations to refer
    // address space struct fields.
    passes_.push_back(&addressSpaceHandler_);
  
    // Collects all the information about memory ranges in the program
    // including the memory passed through kernel arguments. Gives correct
    // limitset to check for each memory access expression.
    // Also fixes kernel and function argument lists to have all necessary
    // extra args.
    passes_.push_back(&kernelHandler_);

    // Adds check macros to every potentially harmful memory access.
    // Collects information of biggest memory access of each address space.
    passes_.push_back(&memoryAccessHandler_);
  
    // Replace calls to builtin functions with versions that check the arguments
    // before calling them. The functions that perform the check are generated.
    passes_.push_back(&functionCallHandler_);
  
    // Prints out the final result.
    passes_.push_back(&printer_);
}

WebCLConsumer::~WebCLConsumer()
{
}

void WebCLConsumer::HandleTranslationUnit(clang::ASTContext &context)
{
    checkAndAnalyze(context);

    // Introduce changes in way that after every step we still have
    // correclty running program.
    
    // FUTURE: Add memory limit dependence analysis here (or maybe it
    //         could be even separate visitor). Also value range
    //         analysis could help in many cases. Based on dependence
    //         analysis, one could create separate implementations for
    //         different calls if we know which arguments are passed
    //         to function.

    transform(context);

    // FUTURE: Add class, which goes through builtins and creates
    //         corresponding safe calls and adds safe implementations
    //         to source.
}

const std::string &WebCLConsumer::getTransformedSource() const
{
    return printer_.getOutput();
}

const WebCLAnalyser::KernelList &WebCLConsumer::getKernels() const
{
    return analyser_.getKernelFunctions();
}

void WebCLConsumer::checkAndAnalyze(clang::ASTContext &context)
{
    clang::TranslationUnitDecl *decl = context.getTranslationUnitDecl();
    for (Visitors::iterator i = visitors_.begin(); i != visitors_.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLVisitor *visitor = (*i);
            visitor->TraverseDecl(decl);
        }
    }
}

void WebCLConsumer::transform(clang::ASTContext &context)
{
    for (Passes::iterator i = passes_.begin(); i != passes_.end(); ++i) {
        // There is no point to continue if an error has been reported.
        if (!hasErrors(context)) {
            WebCLPass *pass = (*i);
            pass->run(context);
        }
    }
}

bool WebCLConsumer::hasErrors(clang::ASTContext &context) const
{
    clang::DiagnosticsEngine &diags = context.getDiagnostics();
    return diags.hasErrorOccurred() || diags.hasUnrecoverableErrorOccurred();
}
