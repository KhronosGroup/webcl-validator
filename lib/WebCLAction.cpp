/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include "WebCLAction.hpp"
#include "WebCLMatcher.hpp"
#include "WebCLPreprocessor.hpp"
#include "WebCLTransformer.hpp"

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/Utils.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Sema/Sema.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/raw_ostream.h"

WebCLAction::WebCLAction(const char *output)
    : clang::FrontendAction()
    , reporter_(NULL), preprocessor_(NULL)
    , usedExtensions_(NULL)
    , output_(output), out_(NULL)
{
}

WebCLAction::~WebCLAction()
{
    // preprocessor_ not deleted intentionally
    delete reporter_;
    reporter_ = NULL;
}

void WebCLAction::setExtensions(const std::set<std::string> &extensions)
{
    extensions_ = extensions;
}

void WebCLAction::setUsedExtensionsStorage(std::set<std::string> *usedExtensions)
{
    usedExtensions_ = usedExtensions;
}

bool WebCLAction::initialize(clang::CompilerInstance &instance)
{
    reporter_ = new WebCLReporter(instance);
    if (!reporter_) {
        llvm::errs() << "Internal error. Can't report errors.\n";
        return false;
    }

    preprocessor_ = new WebCLPreprocessor(instance, extensions_, usedExtensions_);
    if (!preprocessor_) {
        reporter_->fatal("Internal error. Can't create preprocessor callbacks.\n");
        return false;
    }
    clang::Preprocessor &preprocessor = instance.getPreprocessor();
    preprocessor.addPPCallbacks(preprocessor_);

    if (output_) {
        // see clang::PrintPreprocessedAction
        instance.getFrontendOpts().OutputFile = output_;
        out_ = instance.createDefaultOutputFile(true, getCurrentFile());
        if (!out_) {
            reporter_->fatal("Internal error. Can't create output stream.");
            return false;
        }
    }

    const clang::FrontendInputFile &file = getCurrentInput();
    if (file.getKind() != clang::IK_OpenCL) {
        static const char *format =
            "Source file '%0' isn't treated as OpenCL code. Make sure that you "
            "give the '-x cl' option or that the file has a '.cl' extension.\n";
        reporter_->fatal(format) << file.getFile();
        return false;
    }

    return true;
}

WebCLFindUsedExtensionsAction::WebCLFindUsedExtensionsAction()
{
}

WebCLFindUsedExtensionsAction::~WebCLFindUsedExtensionsAction()
{
    // consumer_ not deleted, clang has a reference to it
}

bool WebCLFindUsedExtensionsAction::usesPreprocessorOnly() const
{
    return false;
}

void WebCLFindUsedExtensionsAction::ExecuteAction()
{
    clang::CompilerInstance &instance = getCompilerInstance();

    ParseAST(instance.getPreprocessor(), consumer_, instance.getASTContext());
}

bool WebCLFindUsedExtensionsAction::initialize(clang::CompilerInstance &instance)
{
    if (!WebCLAction::initialize(instance))
        return false;

    consumer_ = finder_.newASTConsumer();
    if (!consumer_) {
        reporter_->fatal("Internal error. Can't create AST consumer.\n");
        return false;
    }
    return true;
}

clang::ASTConsumer* WebCLFindUsedExtensionsAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    if (!initialize(instance)) {
        return NULL;
    }
    return consumer_;
}


WebCLPreprocessorAction::WebCLPreprocessorAction(const char *output, std::string &builtinDecls)
    : WebCLAction(output), builtinDecls_(builtinDecls)
{
}

WebCLPreprocessorAction::~WebCLPreprocessorAction()
{
}

clang::ASTConsumer* WebCLPreprocessorAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    return NULL;
}

void WebCLPreprocessorAction::ExecuteAction()
{
    clang::CompilerInstance &instance = getCompilerInstance();
    if (!initialize(instance))
        return;

    clang::PreprocessorOutputOptions& options = instance.getPreprocessorOutputOpts();
    options.ShowComments = 1;
    options.ShowLineMarkers = 0;
    clang::DoPrintPreprocessedInput(
        instance.getPreprocessor(), out_, options);
    out_->flush();

    // Iterate over all identifier tokens found in the source, to collect
    // identifiers which might be calls to builtin functions, and then
    // forward-declare the builtin functions with that name, if any
    WebCLBuiltins builtins;
    llvm::raw_string_ostream builtinOut(builtinDecls_);
    clang::IdentifierTable& identifiers = instance.getPreprocessor().getIdentifierTable();
    for (clang::IdentifierTable::const_iterator i = identifiers.begin(); i != identifiers.end(); ++i) {
        // Ignore identifiers that start with __ (no OpenCL builtin is like that, but a lot of Clang
        // internal misc are), and also identifiers the preprocessor has to handle in some way,
        // e.g. macros to expand, trigraph sequences to normalize and so on.
        // (The preprocessor has already done its magic and inserted the expansions to the
        //  identifier table at this point so we get them that way)
        if (!i->first().startswith("__") && !i->second->isHandleIdentifierCase()) {
            builtins.emitDeclarations(builtinOut, i->first().str());
        }
    }
    builtinOut.flush();
}

bool WebCLPreprocessorAction::usesPreprocessorOnly() const
{
    return true;
}

WebCLMatcherAction::WebCLMatcherAction(const char *output)
    : WebCLAction(output)
    , finder_()
    , consumer_(0), rewriter_(0)
    , cfg_(), printer_(0)
{
}

WebCLMatcherAction::~WebCLMatcherAction()
{
    // consumer_ not deleted intentionally
    delete printer_;
    printer_ = 0;
    delete rewriter_;
    rewriter_ = 0;
}

clang::ASTConsumer* WebCLMatcherAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    if (!initialize(instance))
        return NULL;
    return consumer_;
}

bool WebCLMatcherAction::usesPreprocessorOnly() const
{
    return false;
}

bool WebCLMatcherAction::initialize(clang::CompilerInstance &instance)
{
    if (!WebCLAction::initialize(instance))
        return false;

    rewriter_ = new clang::Rewriter(
        instance.getSourceManager(), instance.getLangOpts());
    if (!rewriter_) {
        reporter_->fatal("Internal error. Can't create rewriter.\n");
        return false;
    }

    printer_ = new WebCLPrinter(*rewriter_);
    if (!printer_) {
        reporter_->fatal("Internal error. Can't create printer.\n");
        return false;
    }

    consumer_ = finder_.newASTConsumer();
    if (!consumer_) {
        reporter_->fatal("Internal error. Can't create AST consumer.\n");
        return false;
    }

    return true;
}

WebCLMatcher1Action::WebCLMatcher1Action(const char *output)
    : WebCLMatcherAction(output)
{
}

WebCLMatcher1Action::~WebCLMatcher1Action()
{
}

void WebCLMatcher1Action::ExecuteAction()
{
    clang::CompilerInstance &instance = getCompilerInstance();

    WebCLNamelessStructRenamer namelessStructRenamer(instance, cfg_);

    namelessStructRenamer.prepare(finder_);
    ParseAST(instance.getPreprocessor(), consumer_, instance.getASTContext());
    clang::tooling::Replacements &namelessStructRenamings =
        namelessStructRenamer.complete();

    if (!checkIdentifiers())
        return;

    if (!clang::tooling::applyAllReplacements(namelessStructRenamings, *rewriter_)) {
        reporter_->fatal("Can't apply rename nameless structures.");
        return;
    }

    if (!printer_->print(*out_, "// WebCL Validator: matching stage 1.\n")) {
        reporter_->fatal("Can't print first matcher stage output.");
        return;
    }
}

bool WebCLMatcher1Action::checkIdentifiers()
{
    clang::CompilerInstance &instance = getCompilerInstance();
    clang::ASTContext &context = instance.getASTContext();
    clang::IdentifierTable &table = context.Idents;

    const int numPrefixes = 3;
    const char *prefixes[numPrefixes] = {
        cfg_.typePrefix_.c_str(),
        cfg_.variablePrefix_.c_str(),
        cfg_.macroPrefix_.c_str()
    };
    const size_t lengths[numPrefixes] = {
        cfg_.typePrefix_.size(),
        cfg_.variablePrefix_.size(),
        cfg_.macroPrefix_.size()
    };

    bool status = true;

    for (clang::IdentifierTable::iterator i = table.begin(); i != table.end(); ++i) {
        clang::IdentifierInfo *identifier = i->getValue();
        const char *name = identifier->getNameStart();

        static const unsigned int maxLength = 255;
        if (identifier->getLength() > maxLength) {
            reporter_->error("Identifier '%0' exceeds maximum length of %1 characters.") << name << maxLength;
            status = false;
        }

        for (int p = 0; p < numPrefixes ; ++p) {
            const char *prefix = prefixes[p];

            if (!strncmp(prefix, name, lengths[p])) {
                reporter_->error("Identifier '%0' uses reserved prefix '%1'.") << name << prefix;
                status = false;
            }
        }
    }

    return status;
}

WebCLMatcher2Action::WebCLMatcher2Action(const char *output)
    : WebCLMatcherAction(output)
{
}

WebCLMatcher2Action::~WebCLMatcher2Action()
{
}

void WebCLMatcher2Action::ExecuteAction()
{
    clang::CompilerInstance &instance = getCompilerInstance();

    WebCLRenamedStructRelocator renamedStructRelocator(instance, *rewriter_);

    renamedStructRelocator.prepare(finder_);
    ParseAST(instance.getPreprocessor(), consumer_, instance.getASTContext());
    clang::tooling::Replacements &renamedStructRelocations =
        renamedStructRelocator.complete();

    if (!clang::tooling::applyAllReplacements(renamedStructRelocations, *rewriter_)) {
        reporter_->fatal("Can't relocate renamed structures.");
        return;
    }

    if (!printer_->print(*out_, "// WebCL Validator: matching stage 2.\n")) {
        reporter_->fatal("Can't print second matcher stage output.");
        return;
    }
}

WebCLValidatorAction::WebCLValidatorAction(std::string &validatedSource, WebCLAnalyser::KernelList &kernels)
    : WebCLAction()
    , consumer_(0)
    , transformer_(0)
    , rewriter_(0)
    , sema_(0)
    , validatedSource_(validatedSource)
    , kernels_(kernels)
{
}

WebCLValidatorAction::~WebCLValidatorAction()
{
    // consumer_ not deleted intentionally
    delete transformer_;
    transformer_ = 0;
    delete rewriter_;
    rewriter_ = 0;
    // sema_ not deleted intentionally
}

clang::ASTConsumer* WebCLValidatorAction::CreateASTConsumer(
    clang::CompilerInstance &instance, llvm::StringRef)
{
    if (!initialize(instance))
        return NULL;
    return consumer_;
}

void WebCLValidatorAction::ExecuteAction()
{
    // We will get assertions if sema_ isn't wrapped here.
    llvm::OwningPtr<clang::Sema> sema(sema_);
    ParseAST(*sema.get());
    validatedSource_ = consumer_->getTransformedSource();
    kernels_ = consumer_->getKernels();
}

bool WebCLValidatorAction::usesPreprocessorOnly() const
{
    return false;
}

bool WebCLValidatorAction::initialize(clang::CompilerInstance &instance)
{
    if (!WebCLAction::initialize(instance))
        return false;

    rewriter_ = new clang::Rewriter(
        instance.getSourceManager(), instance.getLangOpts());
    if (!rewriter_) {
        reporter_->fatal("Internal error. Can't create rewriter.\n");
        return false;
    }

    transformer_ = new WebCLTransformer(instance, *rewriter_);
    if (!transformer_) {
        reporter_->fatal("Internal error. Can't create AST transformer.\n");
        return false;
    }

    // Consumer must be allocated dynamically. The framework deletes
    // it.
    consumer_ = new WebCLConsumer(instance, *rewriter_, *transformer_);
    if (!consumer_) {
        reporter_->fatal("Internal error. Can't create AST consumer.\n");
        return false;
    }

    sema_ = new clang::Sema(
        instance.getPreprocessor(), instance.getASTContext(), *consumer_);
    if (!sema_) {
        reporter_->fatal("Internal error. Can't create semantic actions.\n");
        return false;
    }

    return true;
}
