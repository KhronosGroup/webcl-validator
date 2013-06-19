#include "WebCLTransformation.hpp"
#include "WebCLTransformations.hpp"

#include "clang/AST/Expr.h"

WebCLTransformations::WebCLTransformations(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , declTransformations_()
    , exprTransformations_()
{
}

WebCLTransformations::~WebCLTransformations()
{
    deleteTransformations(declTransformations_);
    deleteTransformations(exprTransformations_);
}

void WebCLTransformations::addTransformation(
    clang::Decl *decl, WebCLTransformation *transformation)
{
    addTransformation(declTransformations_, decl, transformation);
}

void WebCLTransformations::addTransformation(
    clang::Expr *expr, WebCLTransformation *transformation)
{
    addTransformation(exprTransformations_, expr, transformation);
}

bool WebCLTransformations::rewriteTransformations(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    bool status = true;

    status = status && rewriteTransformations(declTransformations_, cfg, rewriter);
    status = status && rewriteTransformations(exprTransformations_, cfg, rewriter);

    return status;
}

bool WebCLTransformations::contains(clang::Decl *decl)
{
    return declTransformations_.count(decl) > 0;
}

WebCLTransformations::DeclTransformations &WebCLTransformations::getDeclarationTransformations()
{
    return declTransformations_;
}

WebCLTransformations::ExprTransformations &WebCLTransformations::getExpressionTransformations()
{
    return exprTransformations_;
}

template <typename NodeMap, typename Node>
void WebCLTransformations::addTransformation(
    NodeMap &map, const Node *node, WebCLTransformation *transformation)
{
    if (!transformation) {
        error(node->getLocStart(), "Internal error. Can't create transformation.");
        return;
    }

    const std::pair<typename NodeMap::iterator, bool> status =
        map.insert(typename NodeMap::value_type(node, transformation));

    if (!status.second) {
        error(node->getLocStart(), "Transformation has been already created.");
        return;
    }
}

template <typename NodeMap>
void WebCLTransformations::deleteTransformations(NodeMap &map)
{
    for (typename NodeMap::iterator i = map.begin(); i != map.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        delete transformation;
    }
}

template <typename NodeMap>
bool WebCLTransformations::rewriteTransformations(
    NodeMap &map, WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    bool status = true;

    for (typename NodeMap::iterator i = map.begin(); i != map.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(cfg, rewriter);
    }

    return status;
}
