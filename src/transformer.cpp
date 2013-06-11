#include "transformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <sstream>

// WebCLConstantArraySubscriptTransformation

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    clang::CompilerInstance &instance,
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
    : WebCLReporter(instance), expr_(expr), bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::rewrite(clang::Rewriter &rewriter)
{
    clang::Expr *base = expr_->getBase();
    if (!base) {
        error(expr_->getLocStart(), "Invalid indexed array.");
        return false;
    }
    clang::Expr *index = expr_->getIdx();
    if (!index) {
        error(expr_->getLocStart(), "Invalid array index.");
        return false;
    }
    clang::Expr *plainIndex = index->IgnoreParens();
    if (!plainIndex) {
        error(index->getLocStart(), "Can't remove parentheses from array index.");
        return false;
    }

    const std::string replacement =
        rewriter.getRewrittenText(base->getSourceRange()) + "[(" +
        rewriter.getRewrittenText(plainIndex->getSourceRange()) + ") % " +
        bound_.toString(10, false) + "UL]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLTransformer

WebCLTransformer::WebCLTransformer(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , exprTransformations_()
    , checkedPointerTypes_()
    , checkedIndexTypes_()
{
}

WebCLTransformer::~WebCLTransformer()
{
    for (ExprTransformations::iterator i = exprTransformations_.begin();
         i != exprTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        delete transformation;
    }
}

bool WebCLTransformer::rewrite(clang::Rewriter &rewriter)
{
    bool status = true;

    status = status && rewritePrologue(rewriter);
    status = status && rewriteTransformations(rewriter);

    return status;
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    WebCLTransformation *transformation =
        new WebCLConstantArraySubscriptTransformation(instance_, expr, bound);

    if (!transformation) {
        error(expr->getLocStart(), "Internal error. Can't create constant array index check.");
        return;
    }

    const std::pair<ExprTransformations::iterator, bool> status =
        exprTransformations_.insert(
            ExprTransformations::value_type(expr, transformation));

    if (!status.second) {
        error(expr->getLocStart(), "Array index check has been already created.");
        return;
    }
}

void WebCLTransformer::addArrayIndexCheck(clang::ArraySubscriptExpr *expr)
{
    const clang::QualType type = expr->getType();
    addCheckedType(checkedIndexTypes_, type);
}

void WebCLTransformer::addPointerCheck(clang::Expr *expr)
{
    const clang::Type *type = expr->getType().getTypePtrOrNull();
    const clang::QualType pointee = type->getPointeeType();
    addCheckedType(checkedPointerTypes_, pointee);
}

bool WebCLTransformer::rewritePrologue(clang::Rewriter &rewriter)
{
    clang::SourceManager &manager = rewriter.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    std::ostringstream out;
    emitPrologue(out);
    // Rewriter returns false on success.
    return !rewriter.InsertText(start, out.str(), true, true);
}

bool WebCLTransformer::rewriteTransformations(clang::Rewriter &rewriter)
{
    bool status = true;

    for (ExprTransformations::iterator i = exprTransformations_.begin();
         i != exprTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(rewriter);
    }

    return status;
}

std::string WebCLTransformer::getTypeAsString(clang::QualType type)
{
    return getAddressSpaceOfType(type) + "_" + getNameOfType(type);
}

std::string WebCLTransformer::getAddressSpaceOfType(clang::QualType type)
{
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            return "global";
        case clang::LangAS::opencl_local:
            return "local";
        case clang::LangAS::opencl_constant:
            return "constant";
        default:
            return "";
        }
    }

    return "private";
}

std::string WebCLTransformer::getNameOfType(clang::QualType type)
{
    return type.getUnqualifiedType().getAsString();
}

void WebCLTransformer::addCheckedType(CheckedTypes &types, clang::QualType type)
{
    const std::string addressSpace = getAddressSpaceOfType(type);
    const std::string name = getNameOfType(type);
    const CheckedType checked(addressSpace, name);
    types.insert(checked);
}

void WebCLTransformer::emitAddressSpaceRecord(std::ostream &out, const std::string &name)
{
    out << "typedef struct {\n"
        << "} " << name << ";\n"
        << "\n";
}

void WebCLTransformer::emitPrologueRecords(std::ostream &out)
{
    const char *privates = "WclPrivates";
    const char *locals = "WclLocals";
    const char *constants = "WclConstants";
    const char *globals = "WclGlobals";

    emitAddressSpaceRecord(out, privates);
    emitAddressSpaceRecord(out, locals);
    emitAddressSpaceRecord(out, constants);
    emitAddressSpaceRecord(out, globals);

    out << "typedef struct {\n"
        << "    " << privates << " __private *privates;\n"
        << "    " << locals << " __local *locals;\n"
        << "    " << constants << " __constant *constants;\n"
        << "    " << globals << " __global *globals;\n"
        << "} WclAddressSpaces;\n"
        << "\n";
}

void WebCLTransformer::emitLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN(a, b)                                              \\\n"
        << "    (((a) < (b)) ? (a) : (b))\n"

        << "#define WCL_MAX(a, b)                                              \\\n"
        << "    (((a) < (b)) ? (b) : (a))\n"

        << "#define WCL_CLAMP(low, value, high)                                \\\n"
        << "    WCL_MAX((low), WCL_MIN((value), (high)))\n"
        << "\n";
}

void WebCLTransformer::emitPointerLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN_PTR(name, type, field)                             \\\n"
        << "    ((name type *)(field))\n"

        << "#define WCL_MAX_PTR(name, type, field)                             \\\n"
        << "    (WCL_MIN_PTR(name, type, (field) + 1) - 1)\n"
        << "\n";
}

void WebCLTransformer::emitIndexLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN_IDX(name, type, field, ptr)                        \\\n"
        << "    0\n"

        << "#define WCL_MAX_IDX(name, type, field, ptr)                        \\\n"
        << "    (WCL_MAX_PTR(name, type, field) - ptr)\n"
        << "\n";
}

void WebCLTransformer::emitPointerCheckerMacro(std::ostream &out)
{
    out << "#define WCL_PTR_CHECKER(name, field, type)                         \\\n"
        << "    name type *wcl_##name##_##type##_ptr(                          \\\n"
        << "        WclAddressSpaces *as, name type *ptr)                      \\\n"
        << "    {                                                              \\\n"
        << "        return WCL_CLAMP(WCL_MIN_PTR(name, type, as->field),       \\\n"
        << "                         ptr,                                      \\\n"
        << "                         WCL_MAX_PTR(name, type, as->field));      \\\n"
        << "    }\n"
        << "\n";
}

void WebCLTransformer::emitIndexCheckerMacro(std::ostream &out)
{
    out << "#define WCL_IDX_CHECKER(name, field, type)                         \\\n"
        << "    size_t wcl_##name##_##type##_idx(                              \\\n"
        << "        WclAddressSpaces *as, name type *ptr, size_t idx)          \\\n"
        << "    {                                                              \\\n"
        << "        return WCL_CLAMP(WCL_MIN_IDX(name, type, as->field, ptr),  \\\n"
        << "                         idx,                                      \\\n"
        << "                         WCL_MAX_IDX(name, type, as->field, ptr)); \\\n"
        << "    }\n"
        << "\n";
}

void WebCLTransformer::emitPrologueMacros(std::ostream &out)
{
    emitLimitMacros(out);
    emitPointerLimitMacros(out);
    emitIndexLimitMacros(out);
    emitPointerCheckerMacro(out);
    emitIndexCheckerMacro(out);
}

void WebCLTransformer::emitConstantIndexChecker(std::ostream &out)
{
    out << "size_t wcl_idx(size_t idx, size_t limit)\n"
        << "{\n"
        << "    return idx % limit;\n"
        << "}\n"
        << "\n";
}

void WebCLTransformer::emitChecker(std::ostream &out, const CheckedType &type,
                                   const std::string &kind)
{
    out << "WCL_" << kind << "_CHECKER("
        << type.first << ", " << type.first << "s, " << type.second
        << ")\n";
}

void WebCLTransformer::emitCheckers(std::ostream &out, const CheckedTypes &types,
                                    const std::string &kind)
{
    for (CheckedTypes::iterator i = types.begin(); i != types.end(); ++i)
        emitChecker(out, (*i), kind);
    if (types.size())
        out << "\n";
}

void WebCLTransformer::emitPrologueCheckers(std::ostream &out)
{
    emitConstantIndexChecker(out);
    emitCheckers(out, checkedPointerTypes_, "PTR");
    emitCheckers(out, checkedIndexTypes_, "IDX");
}

void WebCLTransformer::emitPrologue(std::ostream &out)
{
    emitPrologueRecords(out);
    emitPrologueMacros(out);
    emitPrologueCheckers(out);
}

// WebCLTransformerClient

WebCLTransformerClient::WebCLTransformerClient()
    : transformer_(0)
{
}

WebCLTransformerClient::~WebCLTransformerClient()
{
}

WebCLTransformer& WebCLTransformerClient::getTransformer()
{
    assert(transformer_);
    return *transformer_;
}

void WebCLTransformerClient::setTransformer(WebCLTransformer &transformer)
{
    transformer_ = &transformer;
}
