#ifndef WEBCLVALIDATOR_WEBCLARGUMENTS
#define WEBCLVALIDATOR_WEBCLARGUMENTS

#include <cstring>

class WebCLArguments
{
public:

    WebCLArguments(int argc, char const *argv[]);
    ~WebCLArguments();

    int getPreprocessorArgc() const;
    char const **getPreprocessorArgv() const;

    int getValidatorArgc() const;
    char const **getValidatorArgv() const;

    char const *getPreprocessorInput() const;
    char const *getValidatorInput() const;

private:

    bool arePreprocessorArgumentsOk() const;
    bool areValidatorArgumentsOk() const;

    char const *createEmptyTemporaryFile(int &fd) const;
    char const *createFullTemporaryFile(int &fd, char const *buffer, size_t length) const;

    int preprocessorArgc_;
    char const **preprocessorArgv_;
    int validatorArgc_;
    char const **validatorArgv_;

    int sourceDescriptor_;
    char const *sourceFilename_;
    int headerDescriptor_;
    char const *headerFilename_;
};

#endif // WEBCLVALIDATOR_WEBCLARGUMENTS
