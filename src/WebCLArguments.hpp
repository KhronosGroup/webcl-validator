#ifndef WEBCLVALIDATOR_WEBCLARGUMENTS
#define WEBCLVALIDATOR_WEBCLARGUMENTS

class WebCLArguments
{
public:

    WebCLArguments(int argc, char const *argv[]);
    ~WebCLArguments();

    int getValidatorArgc() const;
    char const **getValidatorArgv() const;

private:

    int validatorArgc_;
    char const **validatorArgv_;
};

#endif // WEBCLVALIDATOR_WEBCLARGUMENTS
