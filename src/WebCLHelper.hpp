#ifndef WEBCLVALIDATOR_WEBCLHELPER
#define WEBCLVALIDATOR_WEBCLHELPER

#include "clang/AST/Type.h"

namespace clang {
    class Expr;
    class ValueDecl;
    class VarDecl;
    class ParmVarDecl;
}

/// \brief Mixin class for examining AST nodes.
class WebCLHelper
{
public:

    WebCLHelper();
    ~WebCLHelper();

    /// Assume that the expression refers to a pointer and return the
    /// type of the pointed value.
    clang::QualType getPointeeType(clang::Expr *expr);

    /// Remove implicit casts and parentheses.
    clang::Expr *pruneExpression(clang::Expr *expr);

    /// Prune the expression, assume that it refers to value
    /// declaration and finally return the value declaration.
    clang::ValueDecl *pruneValue(clang::Expr *expr);
};

/// vector is used after when address space variables are ordered and possible
/// paddings are added
typedef std::vector<clang::VarDecl*> AddressSpaceInfo;

/// Contains all information of allocated memory of an address space
///
/// Used to pass information to transformer so that it can write typedefs
/// and initializers for it.
class AddressSpaceLimits {
public:
  AddressSpaceLimits(bool hasStaticallyAllocatedLimits, unsigned addressSpace) :
    hasStaticallyAllocatedLimits_(hasStaticallyAllocatedLimits)
  , addressSpace_(addressSpace){};
  ~AddressSpaceLimits() {};
  
  void insert(clang::ParmVarDecl *parm) {
    dynamicLimits_.push_back(parm);
  };

  bool hasStaticallyAllocatedLimits() { return hasStaticallyAllocatedLimits_; };
  unsigned getAddressSpace() { return addressSpace_; };
  
  bool empty() {
    return !hasStaticallyAllocatedLimits_ && dynamicLimits_.empty();
  };

  unsigned count() {
    return hasStaticallyAllocatedLimits() ?
      (dynamicLimits_.size()+1) : dynamicLimits_.size();
  };
    
  typedef std::vector<clang::ParmVarDecl*> LimitList;
  
  LimitList& getDynamicLimits() { return dynamicLimits_; };

private:
  bool hasStaticallyAllocatedLimits_;
  unsigned addressSpace_;
  LimitList dynamicLimits_;
  
};


#endif // WEBCLVALIDATOR_WEBCLHELPER
