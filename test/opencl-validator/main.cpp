#include "validator.hpp"
#include "builder.hpp"

#include <set>

int main(int argc, char const* argv[])
{
    std::set<std::string> help;
    help.insert("-h");
    help.insert("-help");
    help.insert("--help");

    if ((argc == 2) && help.count(argv[1])) {
        std::cerr << "Usage: cat FILE | " << argv[0] << " [OPTIONS]"
                  << std::endl;
        std::cerr << "Checks whether the given OpenCL C code file can be compiled."
                  << std::endl
                  << "Any given options will be passed to clBuildProgram."
                  << std::endl;
        return EXIT_FAILURE;
    }

    OpenCLValidator validator;
    std::string options;
    for (int i = 1; i < argc; ++i) {
        options.append(argv[i]);
        options.append(" ");
    }

    OpenCLBuilder builder(argv[0]);
    if (!builder.compileInput(validator, options))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
