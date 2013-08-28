#ifndef WEBCLVALIDATOR_WEBCLCONSUMER
#define WEBCLVALIDATOR_WEBCLCONSUMER

#include "WebCLPass.hpp"
#include "WebCLPrinter.hpp"
#include "WebCLVisitor.hpp"

#include "clang/AST/ASTConsumer.h"

class WebCLConsumer : public clang::ASTConsumer
{
public:

    explicit WebCLConsumer(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter,
        WebCLTransformer &transformer);
    ~WebCLConsumer();

    /// \see ASTConsumer::HandleTranslationUnit
    virtual void HandleTranslationUnit(clang::ASTContext &context);

private:

    typedef std::list<WebCLVisitor*> Visitors;
    typedef std::list<WebCLPass*> Passes;

    void traverse(Visitors &sequence, clang::ASTContext &context);
    void traverse(Passes &sequence, clang::ASTContext &context);

    bool hasErrors(clang::ASTContext &context) const;

    WebCLTransformer &transformer_;

    /// Checking visitors.
    WebCLRestrictor restrictor_;
    WebCLAnalyser analyser_;
    Visitors checkingVisitors_;

    /// Transforming passes.
    WebCLInputNormaliser inputNormaliser_;
    WebCLAddressSpaceHandler addressSpaceHandler_;
    WebCLKernelHandler kernelHandler_;
    WebCLMemoryAccessHandler memoryAccessHandler_;
    WebCLValidatorPrinter printer_;
    Passes transformingPasses_;
};

#endif // WEBCLVALIDATOR_WEBCLCONSUMER
