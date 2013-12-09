#ifndef WEBCLVALIDATOR_WEBCLPRINTER
#define WEBCLVALIDATOR_WEBCLPRINTER

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
#include "WebCLPass.hpp"

#include <string>

namespace llvm {
    class raw_ostream;
}

namespace clang {
    class Rewriter;
}

/// Prints rewriter contents to a file or to standard output.
class WebCLPrinter
{
public:

    WebCLPrinter(clang::Rewriter &rewriter);
    virtual ~WebCLPrinter();

    /// \brief Output rewritten results. Optionally insert text at the
    /// beginning of output to describe validation stage for example.
    bool print(llvm::raw_ostream &out, const std::string &comment);

protected:

    /// Stores transformations.
    clang::Rewriter &rewriter_;
};

/// \brief Transforms and prints original WebCL C source file.
class WebCLValidatorPrinter : public WebCLPrinter
                            , public WebCLPass
{
public:

    WebCLValidatorPrinter(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLValidatorPrinter();

    /// Apply transformations to original WebCL C source. If the
    /// transformations apply succesfully, print the transformed
    /// source to the output_ variable.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);

    /// Get transformed source
    const std::string &getOutput() const { return output_; }

private:

    /// Stores transformed source after a succesful run
    std::string output_;
};

#endif // WEBCLVALIDATOR_WEBCLPRINTER
