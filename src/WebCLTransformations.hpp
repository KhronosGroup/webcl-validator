#ifndef WEBCLVALIDATOR_WEBCLTRANSFORMATIONS
#define WEBCLVALIDATOR_WEBCLTRANSFORMATIONS

#include "WebCLReporter.hpp"

#include <map>

namespace clang {
    class Decl;
    class Expr;
    class Rewriter;
}

class WebCLTransformation;
class WebCLTransformerConfiguration;

/// \brief Container for all transformations.
class WebCLTransformations : public WebCLReporter
{
public:

    explicit WebCLTransformations(clang::CompilerInstance &instance);
    ~WebCLTransformations();

    void addTransformation(clang::Decl *decl, WebCLTransformation *transformation);
    void addTransformation(clang::Expr *expr, WebCLTransformation *transformation);
    bool rewriteTransformations(
        WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);
    bool contains(clang::Decl *decl);

    typedef std::map<const clang::Decl*, WebCLTransformation*> DeclTransformations;
    DeclTransformations &getDeclarationTransformations();
    typedef std::map<const clang::Expr*, WebCLTransformation*> ExprTransformations;
    ExprTransformations &getExpressionTransformations();

private:

    template <typename NodeMap, typename Node>
    void addTransformation(NodeMap &map, const Node *node, WebCLTransformation *transformation);
    template <typename NodeMap>
    void deleteTransformations(NodeMap &map);
    template <typename NodeMap>
    bool rewriteTransformations(
        NodeMap &map, WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter);

    DeclTransformations declTransformations_;
    ExprTransformations exprTransformations_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMATIONS
