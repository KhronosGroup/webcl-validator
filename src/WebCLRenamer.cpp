#include "WebCLRenamer.hpp"

#include "clang/AST/Decl.h"

WebCLRenamer::WebCLRenamer(const std::string &prefix, const std::string &separator)
    : serials_(), counts_(), prefix_(prefix), separator_(separator)
{
}

WebCLRenamer::~WebCLRenamer()
{
}

void WebCLRenamer::rename(
    std::ostream &out, const clang::NamedDecl *decl)
{
    generate(out, decl, decl->getName().str());
}

void WebCLRenamer::generate(
    std::ostream &out, const clang::NamedDecl *decl, const std::string &name)
{
    Serials::iterator i = serials_.find(decl);
    unsigned int serial = (i == serials_.end()) ? assign(decl) : i->second;

    // Ensure that the user can't interfere with our renaming
    // scheme. The prefix can be freely chosen.
    out << prefix_;
    if (serial >= 2) {
        // Here we rely on the fact that user variables can't start
        // with a number.
        out << serial << separator_;
    }

    out << name;
}

unsigned int WebCLRenamer::assign(const clang::NamedDecl *decl)
{
    const std::string name = decl->getName();
    Counts::iterator i = counts_.find(name);

    unsigned int serial = 1;
    if (i != counts_.end())
        serial = i->second + 1;

    counts_[name] = serial;
    serials_[decl] = serial;
    return serial;
}
