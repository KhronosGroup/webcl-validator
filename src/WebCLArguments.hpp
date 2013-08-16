#ifndef WEBCLVALIDATOR_WEBCLARGUMENTS
#define WEBCLVALIDATOR_WEBCLARGUMENTS

#include <cstring>
#include <utility>
#include <vector>

class WebCLArguments
{
public:

    WebCLArguments(int argc, char const *argv[]);
    ~WebCLArguments();

    int getPreprocessorArgc() const;
    char const **getPreprocessorArgv() const;

    int getMatcherArgc() const;
    char const **getMatcherArgv() const;

    int getValidatorArgc() const;
    char const **getValidatorArgv() const;

    char const *getInput(int argc, char const **argv, bool createOutput = false);

private:

    bool areArgumentsOk(int argc, char const **argv) const;

    char const *createEmptyTemporaryFile(int &fd) const;
    char const *createFullTemporaryFile(int &fd, char const *buffer, size_t length) const;

    int preprocessorArgc_;
    char const **preprocessorArgv_;
    int validatorArgc_;
    char const **validatorArgv_;

    /// Save information about temporary files so that we can close
    /// and remove them.
    typedef std::pair<int, char const *> TemporaryFile;
    typedef std::vector<TemporaryFile> TemporaryFiles;
    TemporaryFiles files_;

    /// Save information about output files so that we can chain
    /// different tools properly.
    typedef std::vector<char const *> OutputFiles;
    OutputFiles outputs_;
};

#endif // WEBCLVALIDATOR_WEBCLARGUMENTS
