#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMER
#define WEBCLVALIDATOR_WEBCLTRANSFORMER

#include "reporter.hpp"
#include "lib/Sema/TreeTransform.h"

/// \brief Performs AST node transformations.
class WebCLTransformer : public WebCLReporter
                       , public clang::TreeTransform<WebCLTransformer>
{
public:

    explicit WebCLTransformer(clang::CompilerInstance &instance,
                              clang::Sema &sema);
    ~WebCLTransformer();

    /// \see clang::TreeTransform::AlwaysRebuild
    bool AlwaysRebuild();

    /// Modify array indexing:
    /// array[index] -> array[index % bound]
    void addArrayIndexCheck(clang::ArraySubscriptExpr *expr, llvm::APInt &bound);
};

/// \brief Mixin class for making AST transformations available after
/// construction.
class WebCLTransformerClient
{
public:

    WebCLTransformerClient();
    ~WebCLTransformerClient();

    WebCLTransformer& getTransformer();
    void setTransformer(WebCLTransformer &transformer);

private:

    WebCLTransformer *transformer_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMER
