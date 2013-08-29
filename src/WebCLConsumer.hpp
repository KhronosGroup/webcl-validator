#ifndef WEBCLVALIDATOR_WEBCLCONSUMER
#define WEBCLVALIDATOR_WEBCLCONSUMER

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
    typedef std::list<WebCLVisitor*> Visitors;
    Visitors visitors_;

    /// Transformation passes.
    WebCLInputNormaliser inputNormaliser_;
    WebCLAddressSpaceHandler addressSpaceHandler_;
    WebCLKernelHandler kernelHandler_;
    WebCLMemoryAccessHandler memoryAccessHandler_;
    WebCLValidatorPrinter printer_;
    /// Passes that generate transformations based on analysis.
    typedef std::list<WebCLPass*> Passes;
    Passes passes_;
};

#endif // WEBCLVALIDATOR_WEBCLCONSUMER
