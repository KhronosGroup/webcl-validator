#ifndef WEBCLVALIDATOR_WEBCLCONSUMER
#define WEBCLVALIDATOR_WEBCLCONSUMER

#include "WebCLPrinter.hpp"
#include "WebCLVisitor.hpp"

#include "clang/AST/ASTConsumer.h"

class WebCLConsumer : public clang::ASTConsumer
{
public:

    explicit WebCLConsumer(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter);
    ~WebCLConsumer();

    /// \see ASTConsumer::HandleTranslationUnit
    virtual void HandleTranslationUnit(clang::ASTContext &context);

    void setTransformer(WebCLTransformer &transformer);

private:

    typedef std::list<WebCLVisitor*> Visitors;
    typedef std::list<WebCLTransformingVisitor*> TransformingVisitors;

    template <typename VisitorSequence>
    void traverse(VisitorSequence &sequence, clang::ASTContext &context);

    bool hasErrors(clang::ASTContext &context) const;

    WebCLRestrictor restrictor_;
    WebCLAnalyser analyser_;
    Visitors checkingVisitors_;

    WebCLValidatorPrinter printer_;
    TransformingVisitors transformingVisitors_;

    WebCLTransformer *transformer_;
};

#endif // WEBCLVALIDATOR_WEBCLCONSUMER
