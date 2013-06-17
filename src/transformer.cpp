#include "transformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <sstream>

// WebCLTransformation

WebCLTransformation::WebCLTransformation(clang::CompilerInstance &instance)
    : WebCLReporter(instance)
{
}

WebCLTransformation::~WebCLTransformation()
{
}

// WebCLVariableRelocation

WebCLVariableRelocation::WebCLVariableRelocation(
    clang::CompilerInstance &instance,
    clang::DeclStmt *stmt, clang::VarDecl *decl)
    : WebCLTransformation(instance)
    , stmt_(stmt), decl_(decl)
{
}

WebCLVariableRelocation::~WebCLVariableRelocation()
{
}

bool WebCLVariableRelocation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    clang::SourceRange range = stmt_ ? stmt_->getSourceRange() : decl_->getSourceRange();
    if (!stmt_) {
        range.setEnd(range.getEnd().getLocWithOffset(1));
    }

    static const std::string prologue = "\n#if 0\n";
    static const std::string epilogue = "\n#endif\n";
    const std::string replacement =
        prologue + rewriter.getRewrittenText(range) + epilogue;
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(range, replacement);
}

// WebCLRecordParameterInsertion

WebCLRecordParameterInsertion::WebCLRecordParameterInsertion(
    clang::CompilerInstance &instance, clang::FunctionDecl *decl)
    : WebCLTransformation(instance), decl_(decl)
{
}

WebCLRecordParameterInsertion::~WebCLRecordParameterInsertion()
{
}

bool WebCLRecordParameterInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string parameter =
        cfg.addressSpaceRecordType_ + " *" + cfg.addressSpaceRecordName_;
    // Rewriter returns false on success.
    return !rewriter.InsertTextBefore(
        decl_->getParamDecl(0)->getLocStart(), parameter + ", ");
}

// WebCLRecordArgumentInsertion

WebCLRecordArgumentInsertion::WebCLRecordArgumentInsertion(
    clang::CompilerInstance &instance, clang::CallExpr *expr)
    : WebCLTransformation(instance), expr_(expr)
{
}

WebCLRecordArgumentInsertion::~WebCLRecordArgumentInsertion()
{
}

bool WebCLRecordArgumentInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const unsigned int numArguments = expr_->getNumArgs();

    clang::SourceLocation location = expr_->getRParenLoc();
    if (numArguments) {
        clang::Expr *first = expr_->getArg(0);
        if (!first) {
            error(expr_->getLocStart(), "Invalid first argument.");
            return false;
        }
        location = first->getLocStart();
    }

    const std::string comma = (numArguments > 0) ? ", " : "";
    const std::string argument = cfg.addressSpaceRecordName_ + comma;

    // Rewriter returns false on success.
    return !rewriter.InsertTextBefore(location, argument);
}

// WebCLSizeParameterInsertion

WebCLSizeParameterInsertion::WebCLSizeParameterInsertion(
    clang::CompilerInstance &instance, clang::ParmVarDecl *decl)
    : WebCLTransformation(instance), decl_(decl)
{
}

WebCLSizeParameterInsertion::~WebCLSizeParameterInsertion()
{
}

bool WebCLSizeParameterInsertion::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string parameter =
        cfg.sizeParameterType_ + " " + cfg.getNameOfSizeParameter(decl_);

    // Doesn't work as expected:
    //return !rewriter.InsertTextAfter(decl_->getLocEnd(), ", " + parameter);

    // Rewriter returns false on success.
    const std::string replacement =
        rewriter.getRewrittenText(decl_->getSourceRange()) + ", " + parameter;
    return !rewriter.ReplaceText(decl_->getSourceRange(), replacement);
}

// WebCLArraySubscriptTransformation

WebCLArraySubscriptTransformation::WebCLArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr)
    : WebCLTransformation(instance), expr_(expr)
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

bool WebCLArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker(expr_->getType()) + "(" +
        cfg.addressSpaceRecordName_ + ", " + base + ", " + index +
        ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLConstantArraySubscriptTransformation

WebCLConstantArraySubscriptTransformation::WebCLConstantArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
    llvm::APInt &bound)
    : WebCLArraySubscriptTransformation(instance, expr)
    , bound_(bound)
{
}

WebCLConstantArraySubscriptTransformation::~WebCLConstantArraySubscriptTransformation()
{
}

bool WebCLConstantArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "("
        + index + ", " + bound_.toString(10, false) + "UL" +
        ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLKernelArraySubscriptTransformation

WebCLKernelArraySubscriptTransformation::WebCLKernelArraySubscriptTransformation(
    clang::CompilerInstance &instance, clang::ArraySubscriptExpr *expr,
    const std::string &bound)
    : WebCLArraySubscriptTransformation(instance, expr)
    , bound_(bound)
{
}

WebCLKernelArraySubscriptTransformation::~WebCLKernelArraySubscriptTransformation()
{
}

bool WebCLKernelArraySubscriptTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string base = getBaseAsText(rewriter);
    const std::string index = getIndexAsText(rewriter);

    const std::string replacement =
        base + "[" + cfg.getNameOfIndexChecker() + "(" + index + ", " + bound_ + ")]";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLPointerDereferenceTransformation

WebCLPointerDereferenceTransformation::WebCLPointerDereferenceTransformation(
    clang::CompilerInstance &instance, clang::Expr *expr)
    : WebCLTransformation(instance), expr_(expr)
{
}

WebCLPointerDereferenceTransformation::~WebCLPointerDereferenceTransformation()
{
}

bool WebCLPointerDereferenceTransformation::rewrite(
    WebCLTransformerConfiguration &cfg, clang::Rewriter &rewriter)
{
    const std::string replacement =
        cfg.getNameOfPointerChecker(expr_->getType().getTypePtr()->getPointeeType()) + "(" +
        cfg.addressSpaceRecordName_ + ", " + rewriter.getRewrittenText(expr_->getSourceRange()) +
        ")";
    // Rewriter returns false on success.
    return !rewriter.ReplaceText(expr_->getSourceRange(), replacement);
}

// WebCLTransformerConfiguration

WebCLTransformerConfiguration::WebCLTransformerConfiguration()
    : prefix_("wcl")
    , pointerSuffix_("ptr")
    , indexSuffix_("idx")
    , indentation_("    ")
    , sizeParameterType_("unsigned long")
    , privateAddressSpace_("private")
    , privateRecordType_("WclPrivates")
    , privateField_("privates")
    , privateRecordName_("wcl_ps")
    , localAddressSpace_("local")
    , localRecordType_("WclLocals")
    , localField_("locals")
    , localRecordName_("wcl_ls")
    , constantAddressSpace_("constant")
    , constantRecordType_("WclConstants")
    , constantField_("constants")
    , constantRecordName_("wcl_cs")
    , globalAddressSpace_("global")
    , globalRecordType_("WclGlobals")
    , globalField_("globals")
    , globalRecordName_("wcl_gs")
    , addressSpaceRecordType_("WclAddressSpaces")
    , addressSpaceRecordName_("wcl_as")
{
}

WebCLTransformerConfiguration::~WebCLTransformerConfiguration()
{
}

const std::string WebCLTransformerConfiguration::getNameOfAddressSpace(clang::QualType type) const
{
    if (const unsigned int space = type.getAddressSpace()) {
        switch (space) {
        case clang::LangAS::opencl_global:
            return globalAddressSpace_;
        case clang::LangAS::opencl_local:
            return localAddressSpace_;
        case clang::LangAS::opencl_constant:
            return constantAddressSpace_;
        default:
            return "???";
        }
    }

    return privateAddressSpace_;
}

const std::string WebCLTransformerConfiguration::getNameOfType(clang::QualType type) const
{
    return type.getUnqualifiedType().getAsString();
}

const std::string WebCLTransformerConfiguration::getNameOfPointerChecker(clang::QualType type) const
{
    return prefix_ + "_" +
        getNameOfAddressSpace(type) + "_" + getNameOfType(type) +
        "_" + pointerSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfIndexChecker(clang::QualType type) const
{
    return prefix_ + "_" +
        getNameOfAddressSpace(type) + "_" + getNameOfType(type) +
        "_" + indexSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfIndexChecker() const
{
    return prefix_ + "_" + indexSuffix_;
}

const std::string WebCLTransformerConfiguration::getNameOfSizeParameter(clang::ParmVarDecl *decl) const
{
    const std::string name = decl->getName();
    return prefix_ + "_" + name + "_size";
}

const std::string WebCLTransformerConfiguration::getIndentation(unsigned int levels) const
{
    std::string indentation;
    for (unsigned int i = 0; i < levels; ++i)
        indentation.append(indentation_);
    return indentation;
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
    , kernels_()
    , cfg_()
{
}

WebCLTransformer::~WebCLTransformer()
{
    for (DeclTransformations::iterator i = declTransformations_.begin();
         i != declTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        delete transformation;
    }

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

void WebCLTransformer::addKernel(clang::FunctionDecl *decl)
{
    kernels_.insert(decl);
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
            instance_, decl));
}

void WebCLTransformer::addRecordArgument(clang::CallExpr *expr)
{
    addTransformation(
        expr,
        new WebCLRecordArgumentInsertion(
            instance_, expr));
}

void WebCLTransformer::addSizeParameter(clang::ParmVarDecl *decl)
{
    addTransformation(
        decl,
        new WebCLSizeParameterInsertion(
            instance_, decl));
}

void WebCLTransformer::addArrayIndexCheck(
    clang::ArraySubscriptExpr *expr, llvm::APInt &bound)
{
    addTransformation(
        expr,
        new WebCLConstantArraySubscriptTransformation(
            instance_, expr, bound));
}

void WebCLTransformer::addArrayIndexCheck(clang::ArraySubscriptExpr *expr)
{
    if (clang::ParmVarDecl *var = getDeclarationOfArray(expr)) {
        addTransformation(
            expr,
            new WebCLKernelArraySubscriptTransformation(
                instance_, expr, cfg_.getNameOfSizeParameter(var)));
    } else {
        addCheckedType(checkedIndexTypes_, expr->getType());

        addTransformation(
            expr,
            new WebCLArraySubscriptTransformation(
                instance_, expr));
    }
}

void WebCLTransformer::addPointerCheck(clang::Expr *expr)
{
    addCheckedType(checkedPointerTypes_, expr->getType().getTypePtr()->getPointeeType());

    addTransformation(
        expr,
        new WebCLPointerDereferenceTransformation(
            instance_, expr));
}

bool WebCLTransformer::rewritePrologue(clang::Rewriter &rewriter)
{
    clang::SourceManager &manager = rewriter.getSourceMgr();
    clang::FileID file = manager.getMainFileID();
    clang::SourceLocation start = manager.getLocForStartOfFile(file);
    std::ostringstream out;
    emitPrologue(out);
    // Rewriter returns false on success.
    return !rewriter.InsertTextAfter(start, out.str());
}

bool WebCLTransformer::rewriteKernelPrologue(
    const clang::FunctionDecl *kernel, clang::Rewriter &rewriter)
{
    clang::Stmt *body = kernel->getBody();
    if (!body) {
        error(kernel->getLocStart(), "Kernel has no body.");
        return false;
    }

    std::ostringstream out;
    emitKernelPrologue(out);
    // Rewriter returns false on success.
    return !rewriter.InsertTextAfter(
        body->getLocStart().getLocWithOffset(1), out.str());
}

bool WebCLTransformer::rewriteTransformations(clang::Rewriter &rewriter)
{
    bool status = true;

    for (Kernels::iterator i = kernels_.begin(); i != kernels_.end(); ++i) {
        const clang::FunctionDecl *kernel = (*i);
        status = status && rewriteKernelPrologue(kernel, rewriter);
    }

    for (DeclTransformations::iterator i = declTransformations_.begin();
         i != declTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(cfg_, rewriter);
    }

    for (ExprTransformations::iterator i = exprTransformations_.begin();
         i != exprTransformations_.end(); ++i) {
        WebCLTransformation *transformation = i->second;
        status = status && transformation->rewrite(cfg_, rewriter);
    }

    return status;
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

void WebCLTransformer::addCheckedType(CheckedTypes &types, clang::QualType type)
{
    const CheckedType checked(cfg_.getNameOfAddressSpace(type), cfg_.getNameOfType(type));
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
        out << "\n" <<  cfg_.indentation_;
        emitVariable(out, (*i));
    }

    out << "\n} " << name << ";\n"
        << "\n";
}

void WebCLTransformer::emitPrologueRecords(std::ostream &out)
{
    emitAddressSpaceRecord(out, relocatedPrivates_, cfg_.privateRecordType_);
    emitAddressSpaceRecord(out, relocatedLocals_, cfg_.localRecordType_);
    emitAddressSpaceRecord(out, relocatedConstants_, cfg_.constantRecordType_);
    emitAddressSpaceRecord(out, relocatedGlobals_, cfg_.globalRecordType_);

    out << "typedef struct {\n"

        << cfg_.indentation_ << cfg_.privateRecordType_
        << " __" << cfg_.privateAddressSpace_ << " *" << cfg_.privateField_ << ";\n"

        << cfg_.indentation_ << cfg_.localRecordType_
        << " __" << cfg_.localAddressSpace_ << " *" << cfg_.localField_ << ";\n"

        << cfg_.indentation_ << cfg_.constantRecordType_
        << " __" << cfg_.constantAddressSpace_ << " *" << cfg_.constantField_ << ";\n"

        << cfg_.indentation_ << cfg_.globalRecordType_
        << " __" << cfg_.globalAddressSpace_ << " *" << cfg_.globalField_ << ";\n"

        << "} " << cfg_.addressSpaceRecordType_ << ";\n"
        << "\n";
}

void WebCLTransformer::emitLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN(a, b) \\\n"
        << cfg_.indentation_ << "(((a) < (b)) ? (a) : (b))\n"

        << "#define WCL_MAX(a, b) \\\n"
        << cfg_.indentation_ << "(((a) < (b)) ? (b) : (a))\n"

        << "#define WCL_CLAMP(low, value, high) \\\n"
        << cfg_.indentation_ << "WCL_MAX((low), WCL_MIN((value), (high)))\n"
        << "\n";
}

void WebCLTransformer::emitPointerLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN_PTR(name, type, field) \\\n"
        << cfg_.indentation_ << "((name type *)(field))\n"

        << "#define WCL_MAX_PTR(name, type, field) \\\n"
        << cfg_.indentation_ << "(WCL_MIN_PTR(name, type, (field) + 1) - 1)\n"
        << "\n";
}

void WebCLTransformer::emitIndexLimitMacros(std::ostream &out)
{
    out << "#define WCL_MIN_IDX(name, type, field, ptr) \\\n"
        << cfg_.indentation_ << "0\n"

        << "#define WCL_MAX_IDX(name, type, field, ptr) \\\n"
        << cfg_.indentation_ << "(WCL_MAX_PTR(name, type, field) - ptr)\n"
        << "\n";
}

void WebCLTransformer::emitPointerCheckerMacro(std::ostream &out)
{
    static const std::string functionName =
        cfg_.prefix_ + "_##name##_##type##_" + cfg_.pointerSuffix_;
    static const std::string asParameter =
        cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;
    static const std::string asField =
        cfg_.addressSpaceRecordName_ + "->field";
    
    out << "#define WCL_PTR_CHECKER(name, field, type) \\\n"
        << cfg_.getIndentation(1) << "name type *" << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr) \\\n"
        << cfg_.getIndentation(1) << "{ \\\n"
        << cfg_.getIndentation(2) << "return WCL_CLAMP( \\\n"
        << cfg_.getIndentation(3) << "WCL_MIN_PTR(name, type, " << asField << "), \\\n"
        << cfg_.getIndentation(3) << "ptr, \\\n"
        << cfg_.getIndentation(3) << "WCL_MAX_PTR(name, type, " << asField << ")); \\\n"
        << cfg_.getIndentation(1) << "}\n"
        << "\n";
}

void WebCLTransformer::emitIndexCheckerMacro(std::ostream &out)
{
    static const std::string functionName =
        cfg_.prefix_ + "_##name##_##type##_" + cfg_.indexSuffix_;
    static const std::string asParameter =
        cfg_.addressSpaceRecordType_ + " *" + cfg_.addressSpaceRecordName_;
    static const std::string asField =
        cfg_.addressSpaceRecordName_ + "->field";

    out << "#define WCL_IDX_CHECKER(name, field, type) \\\n"
        << cfg_.getIndentation(1) << "size_t " << functionName << "( \\\n"
        << cfg_.getIndentation(2) << asParameter << ", name type *ptr, size_t idx) \\\n"
        << cfg_.getIndentation(1) << "{ \\\n"
        << cfg_.getIndentation(2) << "return WCL_CLAMP( \\\n"
        << cfg_.getIndentation(3) << "WCL_MIN_IDX(name, type, " << asField << ", ptr), \\\n"
        << cfg_.getIndentation(3) << "idx, \\\n"
        << cfg_.getIndentation(3) << "WCL_MAX_IDX(name, type, " << asField << ", ptr)); \\\n"
        << cfg_.getIndentation(1) << "}\n"
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
    out << "size_t " << cfg_.getNameOfIndexChecker() << "(size_t idx, size_t limit)\n"
        << "{\n"
        << cfg_.getIndentation(1) << "return idx % limit;\n"
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

void WebCLTransformer::emitKernelPrologue(std::ostream &out)
{
    out << "\n";

    std::string privateString = cfg_.privateRecordName_;
    if (relocatedPrivates_.size()) {
        out << cfg_.indentation_
            << cfg_.privateRecordType_ << " " << privateString
            << ";\n";
        privateString = "&" + privateString;
    } else {
        privateString = "0";
    }

    std::string localString = cfg_.localRecordName_;
    if (relocatedLocals_.size()) {
        out << cfg_.indentation_
            << cfg_.localRecordType_ << " " << localString
            << ";\n";
        localString = "&" + localString;
    } else {
        localString = "0";
    }

    std::string constantString = cfg_.constantRecordName_;
    if (relocatedConstants_.size()) {
        out << cfg_.indentation_
            << cfg_.constantRecordType_ << " " << constantString
            << ";\n";
        constantString = "&" + constantString;
    } else {
        constantString = "0";
    }

    std::string globalString = cfg_.globalRecordName_;
    if (relocatedGlobals_.size()) {
        out << cfg_.indentation_
            << cfg_.globalRecordType_ << " " << globalString
            << ";\n";
        globalString = "&" + globalString;
    } else {
        globalString = "0";
    }

    out << cfg_.indentation_
        << cfg_.addressSpaceRecordType_ << " " << cfg_.addressSpaceRecordName_
        << " = { "
        << privateString << ", "
        << localString << ", "
        << constantString << ", "
        << globalString
        << " };\n";
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
