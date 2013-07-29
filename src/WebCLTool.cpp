#include "WebCLAction.hpp"
#include "WebCLTool.hpp"

#include "clang/Tooling/CompilationDatabase.h"

#include <iostream>

WebCLTool::WebCLTool(int argc, char const **argv,
                     char const *input, char const *output)
    : compilations_(NULL), paths_(), tool_(NULL), factory_(NULL)
{
    compilations_ =
        clang::tooling::FixedCompilationDatabase::loadFromCommandLine(argc, argv);
    if (!compilations_) {
        std::cerr << "Internal error. Can't create compilation database." << std::endl;
        return;
    }

    paths_.push_back(input);
    tool_ = new clang::tooling::ClangTool(*compilations_, paths_);
    if (!tool_) {
        std::cerr << "Internal error. Can't create tool." << std::endl;
        return;
    }

    factory_ = new WebCLActionFactory(output);
    if (!factory_) {
        std::cerr << "Internal error. Can't create factory." << std::endl;
        return;
    }
}

WebCLTool::~WebCLTool()
{
    delete factory_;
    factory_ = NULL;
    delete tool_;
    tool_ = NULL;
    delete compilations_;
    compilations_ = NULL;
}

int WebCLTool::run()
{
    if (!compilations_ || !tool_)
        return EXIT_FAILURE;
    return tool_->run(factory_);
}
