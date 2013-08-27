#ifndef WEBCLVALIDATOR_WEBCLPRINTER
#define WEBCLVALIDATOR_WEBCLPRINTER

#include "WebCLVisitor.hpp"

namespace llvm {
    class raw_ostream;
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
                            , public WebCLTransformingVisitor
{
public:

    WebCLValidatorPrinter(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    ~WebCLValidatorPrinter();

    /// Apply transformations to original WebCL C source. If the
    /// transformations apply succesfully, print the transformed
    /// source to standard output.
    ///
    /// \see WebCLVisitor::handleTranslationUnitDecl
    virtual bool handleTranslationUnitDecl(clang::TranslationUnitDecl *decl);
};

#endif // WEBCLVALIDATOR_WEBCLPRINTER
