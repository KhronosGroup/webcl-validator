#ifndef WEBCLVALIDATOR_WEBCLRENAMER
#define WEBCLVALIDATOR_WEBCLRENAMER

#include <iostream>
#include <map>
#include <string>

namespace clang {
    class NamedDecl;
}

/// Renames variables and types. Sometimes relocated variables and
/// types may use an identical name. An instance of this class can
/// then be used to make sure that all relocated names are unique.
class WebCLRenamer
{
public:

    explicit WebCLRenamer(const std::string &prefix, const std::string &separator);
    ~WebCLRenamer();

    /// Write unique version of given name to the output stream.
    void rename(std::ostream &out, const clang::NamedDecl *decl);

    /// Generate unique version of given name to the output stream.
    void generate(std::ostream &out, const clang::NamedDecl *decl, const std::string &name);

private:

    /// Indicate that a named object needs to be uniquely renamed.
    unsigned int assign(const clang::NamedDecl *decl);

    /// Maps object to serial number '#'. The object is renamed as
    /// 'prefix#_name'. Needed so that object can be renamed quickly.
    typedef std::map<const clang::NamedDecl*, unsigned int> Serials;
    Serials serials_;
    /// Maps object name to number of identically named objects. The
    /// count of a name becomes the serial number of next object with
    /// that name.
    typedef std::map<const std::string, unsigned int> Counts;
    Counts counts_;

    // Freely chosen prefix for renamed variables.
    const std::string prefix_;
    // Separates generated prefix from user defined name.
    const std::string separator_;
};

#endif // WEBCLVALIDATOR_WEBCLRENAMER
