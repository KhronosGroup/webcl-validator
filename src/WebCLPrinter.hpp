#ifndef WEBCLVALIDATOR_WEBCLPRINTER
#define WEBCLVALIDATOR_WEBCLPRINTER

#include "WebCLVisitor.hpp"

/// \brief Transforms and prints original WebCL C source file.
class WebCLPrinter : public WebCLTransformingVisitor
{
public:

    explicit WebCLPrinter(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    ~WebCLPrinter();

    /// Apply transformations to original WebCL C source. If the
    /// transformations apply succesfully, print the transformed
    /// source to standard output.
    ///
    /// \see WebCLVisitor::handleTranslationUnitDecl
    virtual bool handleTranslationUnitDecl(clang::TranslationUnitDecl *decl);

private:

    /// \brief Output transformed WebCL C source.
    bool print();

    clang::Rewriter &rewriter_;
};

#endif // WEBCLVALIDATOR_WEBCLPRINTER
