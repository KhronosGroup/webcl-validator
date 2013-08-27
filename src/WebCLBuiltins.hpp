#ifndef WEBCLVALIDATOR_WEBCLBUILTINS
#define WEBCLVALIDATOR_WEBCLBUILTINS

#include <set>
#include <string>

/// Holds information about OpenCL C builtin functions that require
/// pointer arguments. The possibly unsafe builtins are partitioned
/// into several classes depending on what kind of checks need to be
/// performed on their arguments.
class WebCLBuiltins
{
public:

    WebCLBuiltins();
    ~WebCLBuiltins();

    /// \return Whether the builtin takes pointer argument(s), but no
    /// pointer validation needs to be performed.
    bool isSafe(const std::string &builtin) const;

    /// \return Whether the builtin takes pointer argument(s) and a
    /// pointer validity check needs to be performed on them.
    bool isUnsafe(const std::string &builtin) const;

    /// \return Whether WebCL C doesn't support the OpenCL C builtin
    /// at all.
    bool isUnsupported(const std::string &builtin) const;

private:

    /// Data structure for builtin function names.
    typedef std::set<std::string> BuiltinNames;

    /// Expands given patterns into builtin function names. For
    /// example, 'vload#' is expanded to 'vload2' - 'vload16'.
    void initialize(BuiltinNames &names, const char *patterns[], int numPatterns);

    /// The pointer argument points to a single element.
    BuiltinNames unsafeMathBuiltins_;
    /// The pointer argument points to an element array.
    BuiltinNames unsafeVectorBuiltins_;
    /// The pointer argument points to an (unsigned) integer.
    BuiltinNames unsafeAtomicBuiltins_;
    /// Calling is never allowed.
    BuiltinNames unsupportedBuiltins_;
    /// Calling is always safe for these, even if they have pointer
    /// arguments.
    BuiltinNames safeBuiltins_;
};

#endif // WEBCLVALIDATOR_WEBCLBUILTINS
