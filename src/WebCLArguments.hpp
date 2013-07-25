#ifndef WEBCLVALIDATOR_WEBCLARGUMENTS
#define WEBCLVALIDATOR_WEBCLARGUMENTS

#include <cstring>

class WebCLArguments
{
public:

    WebCLArguments(int argc, char const *argv[]);
    ~WebCLArguments();

    int getValidatorArgc() const;
    char const **getValidatorArgv() const;

private:

    bool areValidatorArgumentsOk() const;

    char const *createEmptyTemporaryFile(int &fd) const;
    char const *createFullTemporaryFile(int &fd, char const *buffer, size_t length) const;

    int validatorArgc_;
    char const **validatorArgv_;

    int headerDescriptor_;
    char const *headerFilename_;
};

#endif // WEBCLVALIDATOR_WEBCLARGUMENTS
