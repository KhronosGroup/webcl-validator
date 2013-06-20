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
class WebCLRecursiveTransformation;
class WebCLTransformerConfiguration;

/// \brief Container for all transformations.
class WebCLTransformations : public WebCLReporter
{
public:

    WebCLTransformations(
        clang::CompilerInstance &instance, clang::Rewriter &rewriter,
        WebCLTransformerConfiguration &cfg);
    ~WebCLTransformations();

    clang::CompilerInstance &getCompilerInstance();
    clang::Rewriter &getRewriter();
    WebCLTransformerConfiguration &getConfiguration();

    void addTransformation(clang::Decl *decl, WebCLTransformation *transformation);
    void addTransformation(clang::Expr *expr, WebCLRecursiveTransformation *transformation);

    WebCLTransformation* getTransformation(const clang::Decl *decl);
    WebCLRecursiveTransformation* getTransformation(const clang::Expr *expr);

    bool rewriteTransformations();
    bool contains(clang::Decl *decl);

private:

    template <typename NodeMap, typename Node, typename NodeTransformation>
    void addTransformation(NodeMap &map, const Node *node, NodeTransformation *transformation);
    template <typename NodeMap, typename Node, typename NodeTransformation>
    NodeTransformation *getTransformation(NodeMap &map, const Node *node);
    template <typename NodeMap>
    void deleteTransformations(NodeMap &map);
    template <typename NodeMap>
    bool rewriteTransformations(NodeMap &map);

    typedef std::map<const clang::Decl*, WebCLTransformation*> DeclTransformations;
    DeclTransformations declTransformations_;
    typedef std::map<const clang::Expr*, WebCLRecursiveTransformation*> ExprTransformations;
    ExprTransformations exprTransformations_;

    clang::Rewriter &rewriter_;
    WebCLTransformerConfiguration &cfg_;
};

#endif // WEBCLVALIDATOR_WEBCLTRANSFORMATIONS
