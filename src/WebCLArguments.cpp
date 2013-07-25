#include "WebCLArguments.hpp"

#include <algorithm>

// TODO: when we port to llvm-3.3 we can just say -target spir and it
//       should have valid address space mapping
static char const *extraValidatorArgs_[] = {
    "-x", "cl", "-ffake-address-space-map"
};
static int numExtraValidatorArgs_ =
    sizeof(extraValidatorArgs_) / sizeof(extraValidatorArgs_[0]);

WebCLArguments::WebCLArguments(int argc, char const *argv[])
    : validatorArgc_(argc + numExtraValidatorArgs_)
    , validatorArgv_(new char const *[validatorArgc_])
{
    if (validatorArgv_) {
        char const **userBegin = argv;
        char const **userEnd = argv + argc;
        char const **userResult = validatorArgv_;
        std::copy(userBegin, userEnd, userResult);

        char const **toolBegin = extraValidatorArgs_;
        char const **toolEnd = extraValidatorArgs_ + numExtraValidatorArgs_;
        char const **toolResult = validatorArgv_ + argc;
        std::copy(toolBegin, toolEnd, toolResult);
    }
}

WebCLArguments::~WebCLArguments()
{
}

int WebCLArguments::getValidatorArgc() const
{
    return validatorArgc_;
}

char const **WebCLArguments::getValidatorArgv() const
{
    return validatorArgv_;
}
