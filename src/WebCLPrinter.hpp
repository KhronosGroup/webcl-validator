#ifndef WEBCLVALIDATOR_WEBCLPRINTER
#define WEBCLVALIDATOR_WEBCLPRINTER

#include "WebCLReporter.hpp"
#include "WebCLPass.hpp"

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
class WebCLValidatorPrinter : public WebCLReporter
                            , public WebCLPrinter
                            , public WebCLPass
{
public:

    WebCLValidatorPrinter(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter,
        WebCLAnalyser &analyser, WebCLTransformer &transformer);
    virtual ~WebCLValidatorPrinter();

    /// Apply transformations to original WebCL C source. If the
    /// transformations apply succesfully, print the transformed
    /// source to standard output.
    ///
    /// \see WebCLPass
    virtual void run(clang::ASTContext &context);
};

#endif // WEBCLVALIDATOR_WEBCLPRINTER
