#ifndef WEBCLVALIDATOR_WEBCLCONSUMER
#define WEBCLVALIDATOR_WEBCLCONSUMER

#include "visitor.hpp"

#include "clang/AST/ASTConsumer.h"

class WebCLConsumer : public clang::ASTConsumer
{
public:

    explicit WebCLConsumer(clang::CompilerInstance &instance);
    ~WebCLConsumer();

    /// \see ASTConsumer::HandleTranslationUnit
    virtual void HandleTranslationUnit(clang::ASTContext &context);

    void setTransformer(WebCLTransformer &transformer);

private:

    bool hasErrors(clang::ASTContext &context) const;

    WebCLRestrictor restrictor_;
    WebCLAccessor accessor_;
    WebCLPrinter printer_;
};

#endif // WEBCLVALIDATOR_WEBCLCONSUMER
