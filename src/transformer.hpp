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

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMER
