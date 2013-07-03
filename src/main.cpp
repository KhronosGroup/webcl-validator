#include "WebCLAction.hpp"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include <iostream>


// TODO: when we port to llvm-3.3 we can just say -target spir and it should
//       have valid address space mapping
class WebCLArgumentsAdjuster : public clang::tooling::ArgumentsAdjuster {
public:
  virtual clang::tooling::CommandLineArguments Adjust(const clang::tooling::CommandLineArguments &args) {
    clang::tooling::CommandLineArguments fixedArgs = args;
    std::cerr << "Fixing cmd args!!\n";
    fixedArgs.push_back("-x");
    fixedArgs.push_back("cl");
    fixedArgs.push_back("-ffake-address-space-map");
    return fixedArgs;
  }
};

int main(int argc, char const* argv[])
{
    clang::tooling::CommonOptionsParser optionsParser(argc, argv);
    clang::tooling::ClangTool webclTool(optionsParser.GetCompilations(),
                                        optionsParser.GetSourcePathList());

// This didn't help at all
//    WebCLArgumentsAdjuster *fixCommandLine = new WebCLArgumentsAdjuster;
//    webclTool.setArgumentsAdjuster(fixCommandLine);
  
    clang::tooling::FrontendActionFactory *webCLActionFactory =
        clang::tooling::newFrontendActionFactory<WebCLAction>();
    const int status = webclTool.run(webCLActionFactory);
    delete webCLActionFactory;

    return status;
}
