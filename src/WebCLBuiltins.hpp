#ifndef WEBCLVALIDATOR_WEBCLBUILTINS
#define WEBCLVALIDATOR_WEBCLBUILTINS

#include <set>
#include <string>

class WebCLBuiltins
{
public:

    WebCLBuiltins();
    ~WebCLBuiltins();

    bool isUnsafe(const std::string &builtin) const;
    bool isUnsupported(const std::string &builtin) const;

private:

    typedef std::set<std::string> BuiltinNames;

    void initialize(BuiltinNames &names, const char *patterns[], int numPatterns);

    // The pointer argument points to a single element.
    BuiltinNames unsafeMathBuiltins_;
    // The pointer argument points to an element array.
    BuiltinNames unsafeVectorBuiltins_;
    // The pointer argument points to an (unsigned) integer.
    BuiltinNames unsafeAtomicBuiltins_;
    // Calling is never allowed.
    BuiltinNames unsupportedBuiltins_;
};

#endif // WEBCLVALIDATOR_WEBCLBUILTINS
