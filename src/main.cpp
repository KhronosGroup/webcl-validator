#include "WebCLAction.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char const* argv[])
{
    clang::tooling::CommonOptionsParser optionsParser(argc, argv);
    clang::tooling::ClangTool webclTool(optionsParser.GetCompilations(),
                                        optionsParser.GetSourcePathList());

    clang::tooling::FrontendActionFactory *webCLActionFactory = 
        clang::tooling::newFrontendActionFactory<WebCLAction>();
    const int status = webclTool.run(webCLActionFactory);
    delete webCLActionFactory;

    return status;
}
