#include "transformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <sstream>

// WebCLVariableRelocation

WebCLVariableRelocation::WebCLVariableRelocation(
    clang::CompilerInstance &instance,
    clang::DeclStmt *stmt, clang::VarDecl *decl)
    : WebCLReporter(instance_)
    , stmt_(stmt), decl_(decl)
{
}

WebCLVariableRelocation::~WebCLVariableRelocation()
{
}

bool WebCLVariableRelocation::rewrite(clang::Rewriter &rewriter)
{
    clang::SourceRange range = stmt_ ? stmt_->getSourceRange() : decl_->getSourceRange();
    if (!stmt_) {
        range.setEnd(range.getEnd().getLocWithOffset(1));
    }

    const std::string prologue = "\n#if 0\n";
    const std::string epilogue = "\n#endif\n";
    const std::string replacement =
        prologue + rewriter.getRewrittenText(range) + epilogue;
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(range, replacement);
}

// WebCLParameterInsertion

WebCLParameterInsertion::WebCLParameterInsertion(
    clang::CompilerInstance &instance, const std::string &parameter)
    : WebCLReporter(instance), parameter_(parameter)
{
}

// WebCLRecordParameterInsertion

WebCLRecordParameterInsertion::WebCLRecordParameterInsertion(
    clang::CompilerInstance &instance, const std::string &parameter,
    clang::FunctionDecl *decl)
    : WebCLParameterInsertion(instance, parameter)
    , decl_(decl)
{
}

WebCLRecordParameterInsertion::~WebCLRecordParameterInsertion()
{
}

bool WebCLRecordParameterInsertion::rewrite(clang::Rewriter &rewriter)
{
    // Rewriter returns false on success.
    return !rewriter.InsertTextBefore(
        decl_->getParamDecl(0)->getLocStart(), parameter_ + ", ");
}

// WebCLSizeParameterInsertion

WebCLSizeParameterInsertion::WebCLSizeParameterInsertion(
    clang::CompilerInstance &instance, const std::string &parameter,
    clang::ParmVarDecl *decl)
    : WebCLParameterInsertion(instance, parameter)
    , decl_(decl)
{
}

WebCLSizeParameterInsertion::~WebCLSizeParameterInsertion()
{
}

bool WebCLSizeParameterInsertion::rewrite(clang::Rewriter &rewriter)
{
    // Doesn't work as expected:
    //return !rewriter.InsertTextAfter(decl_->getLocEnd(), ", " + parameter_);

    // Rewriter returns false on success.
    const std::string replacement =
        rewriter.getRewrittenText(decl_->getSourceRange()) + ", " + parameter_;
    return !rewriter.ReplaceText(decl_->getSourceRange(), replacement);
}

// WebCLCheckerTransformation

WebCLCheckerTransformation::WebCLCheckerTransformation(
    clang::CompilerInstance &instance, const std::string &checker)
    : WebCLReporter(instance), checker_(checker)
{
}

// WebCLArraySubscriptTransformation

WebCLArraySubscriptTransformation::WebCLArraySubscriptTransformation(
    clang::CompilerInstance &instance, const std::string &checker,
    clang::ArraySubscriptExpr *expr)
    : WebCLCheckerTransformation(instance, checker)
    , expr_(expr)
{
}

WebCLArraySubscriptTransformation::~WebCLArraySubscriptTransformation()
{
}

std::string WebCLArraySubscriptTransformation::getBaseAsText(clang::Rewriter &rewriter)
{
    clang::Expr *base = expr_->getBase();
    if (!base) {
        error(expr_->getLocStart(), "Invalid indexed array.");
        return "";
    }

    return rewriter.getRewrittenText(base->getSourceRange());
}

std::string WebCLArraySubscriptTransformation::getIndexAsText(clang::Rewriter &rewriter)
{
    clang::Expr *index = expr_->getIdx();
    if (!index) {
        error(expr_->getLocStart(), "Invalid array index.");
        return "";
    }
    clang::Expr *plainIndex = index->IgnoreParens();
    if (!plainIndex) {
        error(index->getLocStart(), "Can't remove parentheses from array index.");
        return "";
    }

    return rewriter.getRewrittenText(plainIndex->getSourceRange());
}

bool WebCLArraySubscriptTransformation::rewrite(clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + checker_ + "(NULL, " + base + ", " + index + ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLConstantArraySubscriptTransformation

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    clang::CompilerInstance &instance, const std::string &checker,
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
    : WebCLArraySubscriptTransformation(instance, checker, expr)
    , bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::rewrite(clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + checker_ + "(" + index + ", " + bound_.toString(10, false) + "UL)]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLKernelArraySubscriptTransformation

WebCLKernelArraySubscriptTransformation::WebCLKernelArraySubscriptTransformation(
    clang::CompilerInstance &instance, const std::string &checker,
    clang::ArraySubscriptExpr *expr, const std::string &bound)
    : WebCLArraySubscriptTransformation(instance, checker, expr)
    , bound_(bound)
{
}

WebCLKernelArraySubscriptTransformation::~WebCLKernelArraySubscriptTransformation()
{
}

bool WebCLKernelArraySubscriptTransformation::rewrite(clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + checker_ + "(" + index + ", " + bound_ + ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLPointerDereferenceTransformation

WebCLPointerDereferenceTransformation::WebCLPointerDereferenceTransformation(
    clang::CompilerInstance &instance, const std::string &checker,
    clang::Expr *expr)
    : WebCLCheckerTransformation(instance, checker)
    , expr_(expr)
{
}

WebCLPointerDereferenceTransformation::~WebCLPointerDereferenceTransformation()
{
}

bool WebCLPointerDereferenceTransformation::rewrite(clang::Rewriter &rewriter)
{
    const std::string replacement =
        checker_ + "(NULL, " + rewriter.getRewrittenText(expr_->getSourceRange()) + ")";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLTransformer

WebCLTransformer::WebCLTransformer(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
    , declTransformations_()
    , exprTransformations_()
    , checkedPointerTypes_()
    , checkedIndexTypes_()
    , relocatedGlobals_()
    , relocatedLocals_()
    , relocatedConstants_()
    , relocatedPrivates_()
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

void WebCLTransformer::addRelocatedVariable(clang::DeclStmt *stmt, clang::VarDecl *decl)
{
    addTransformation(
        decl,
        new WebCLVariableRelocation(
            instance_, stmt, decl));
    addRelocatedVariable(decl);
}

void WebCLTransformer::addRecordParameter(clang::FunctionDecl *decl)
{
    addTransformation(
        decl,
        new WebCLRecordParameterInsertion(
            instance_, "WclAddressSpaces *wcl_as", decl));
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    const std::string name = getNameOfSizeParameter(decl);
    const std::string parameter = "unsigned long " + name;
    addTransformation(
        decl,
        new WebCLSizeParameterInsertion(
            instance_, parameter, decl));
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    addTransformation(
        expr,
        new WebCLConstantArraySubscriptTransformation(
            instance_, "wcl_idx", expr, bound));
}

void WebCLTransformer::addArrayIndexCheck(clang::ArraySubscriptExpr *expr)
{
    if (clang::ParmVarDecl *var = getDeclarationOfArray(expr)) {
        addTransformation(
            expr,
            new WebCLKernelArraySubscriptTransformation(
                instance_, "wcl_idx", expr, getNameOfSizeParameter(var)));
    } else {
        const clang::QualType type = expr->getType();
        addCheckedType(checkedIndexTypes_, type);

        const std::string checker = getNameOfChecker(type) + "_idx";
        addTransformation(
            expr,
            new WebCLArraySubscriptTransformation(instance_, checker, expr));
    }
}

void WebCLTransformer::addPointerCheck(clang::Expr *expr)
{
    const clang::Type *type = expr->getType().getTypePtrOrNull();
    const clang::QualType pointee = type->getPointeeType();
    addCheckedType(checkedPointerTypes_, pointee);

    const std::string checker = getNameOfChecker(pointee) + "_ptr";
    addTransformation(
        expr,
        new WebCLPointerDereferenceTransformation(instance_, checker, expr));
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

    for (DeclTransformations::iterator i = declTransformations_.begin();
         i != declTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(rewriter);
    }

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

std::string WebCLTransformer::getNameOfChecker(clang::QualType type)
{
    const std::string addressSpace = getAddressSpaceOfType(type);
    const std::string name = getNameOfType(type);
    return "wcl_" + addressSpace + "_" + name;
}

clang::ParmVarDecl *WebCLTransformer::getDeclarationOfArray(clang::ArraySubscriptExpr *expr)
{
    clang::Expr *base = expr->getBase();
    if (!base)
        return NULL;

    clang::Expr *pruned = base->IgnoreImpCasts();
    if (!pruned)
        return NULL;

    clang::DeclRefExpr *ref = llvm::dyn_cast<clang::DeclRefExpr>(pruned);
    if (!ref)
        return NULL;

    clang::ParmVarDecl *var = llvm::dyn_cast<clang::ParmVarDecl>(ref->getDecl());
    if (!var)
        return NULL;

    if (!declTransformations_.count(var))
        return NULL;

    return var;
}

std::string WebCLTransformer::getNameOfSizeParameter(clang::ParmVarDecl *decl)
{
    const std::string name = decl->getName();
    return "wcl_" + name + "_size";
}

void WebCLTransformer::addCheckedType(CheckedTypes &types, clang::QualType type)
{
    const std::string addressSpace = getAddressSpaceOfType(type);
    const std::string name = getNameOfType(type);
    const CheckedType checked(addressSpace, name);
    types.insert(checked);
}

void WebCLTransformer::addTransformation(
    const clang::Decl *decl, WebCLTransformation *transformation)
{
    if (!transformation) {
        error(decl->getLocStart(), "Internal error. Can't create declaration transformation.");
        return;
    }

    const std::pair<DeclTransformations::iterator, bool> status =
        declTransformations_.insert(
            DeclTransformations::value_type(decl, transformation));

    if (!status.second) {
        error(decl->getLocStart(), "Declaration transformation has been already created.");
        return;
    }
}

void WebCLTransformer::addTransformation(
    const clang::Expr *expr, WebCLTransformation *transformation)
{
    if (!transformation) {
        error(expr->getLocStart(), "Internal error. Can't create expression transformation.");
        return;
    }

    const std::pair<ExprTransformations::iterator, bool> status =
        exprTransformations_.insert(
            ExprTransformations::value_type(expr, transformation));

    if (!status.second) {
        error(expr->getLocStart(), "Expression transformation has been already created.");
        return;
    }
}

void WebCLTransformer::addRelocatedVariable(clang::VarDecl *decl)
{
    clang::QualType type = decl->getType();
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            relocatedGlobals_.insert(decl);
            return;
        case clang::LangAS::opencl_local:
            relocatedLocals_.insert(decl);
            return;
        case clang::LangAS::opencl_constant:
            relocatedConstants_.insert(decl);
            return;
        default:
            error(decl->getLocStart(), "Unknown address space.");
            return;
        }
    }

    relocatedPrivates_.insert(decl);
}

void WebCLTransformer::emitVariable(std::ostream &out, const clang::VarDecl *decl)
{
    clang::QualType type = decl->getType();
    clang::Qualifiers qualifiers = type.getQualifiers();
    qualifiers.removeAddressSpace();

    std::string variable;
    llvm::raw_string_ostream stream(variable);
    clang::PrintingPolicy policy(instance_.getLangOpts());
    clang::QualType::print(
        type.getTypePtrOrNull(), qualifiers, stream, policy, decl->getName());
    out << stream.str();
}

void WebCLTransformer::emitAddressSpaceRecord(
    std::ostream &out, const VariableDeclarations &variables, const std::string &name)
{
    out << "typedef struct {";

    for (VariableDeclarations::iterator i = variables.begin(); i != variables.end(); ++i) {
        if (i != variables.begin())
            out << ",";
        out << "\n    ";
        emitVariable(out, (*i));
    }

    out << "\n} " << name << ";\n"
        << "\n";
}

void WebCLTransformer::emitPrologueRecords(std::ostream &out)
{
    const char *privates = "WclPrivates";
    const char *locals = "WclLocals";
    const char *constants = "WclConstants";
    const char *globals = "WclGlobals";

    emitAddressSpaceRecord(out, relocatedPrivates_, privates);
    emitAddressSpaceRecord(out, relocatedLocals_, locals);
    emitAddressSpaceRecord(out, relocatedConstants_, constants);
    emitAddressSpaceRecord(out, relocatedGlobals_, globals);

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
