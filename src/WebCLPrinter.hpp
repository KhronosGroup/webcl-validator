#ifndef WEBCLVALIDATOR_WEBCLPRINTER
#define WEBCLVALIDATOR_WEBCLPRINTER

#include "WebCLReporter.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

/// \brief Transforms and prints original WebCL C source file.
class WebCLPrinter : public WebCLReporter
                   , public WebCLTransformerClient
                   , public clang::RecursiveASTVisitor<WebCLPrinter>
{
public:

    explicit WebCLPrinter(clang::CompilerInstance &instance);
    ~WebCLPrinter();

    /// Apply transformations to original WebCL C source. If the
    /// transformations apply succesfully, print the transformed
    /// source to standard output.
    ///
    /// \see RecursiveASTVisitor::VisitTranslationUnitDecl
    bool VisitTranslationUnitDecl(clang::TranslationUnitDecl *decl);

private:

    /// \brief Output transformed WebCL C source.
    bool print();

    clang::Rewriter rewriter_;
};

#endif // WEBCLVALIDATOR_WEBCLPRINTER
