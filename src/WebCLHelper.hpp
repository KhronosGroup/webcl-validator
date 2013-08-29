#ifndef WEBCLVALIDATOR_WEBCLHELPER
#define WEBCLVALIDATOR_WEBCLHELPER

#include "WebCLDebug.hpp"

#include "clang/AST/Type.h"

#include <map>
#include <set>

namespace clang {
    class Expr;
    class ValueDecl;
    class VarDecl;
    class ParmVarDecl;
    class Rewriter;
    class CompilerInstance;
}

/// vector is used after when address space variables are ordered and possible
/// paddings are added
typedef std::vector<clang::VarDecl*> AddressSpaceInfo;

/// Contains all information of allocated memory of an address space
///
/// Used to pass information to transformer so that it can write typedefs
/// and initializers for it.
class AddressSpaceLimits {
public:
  AddressSpaceLimits(unsigned addressSpace) :
    hasStaticallyAllocatedLimits_(false)
  , addressSpace_(addressSpace){};
  ~AddressSpaceLimits() {};
  
  void insert(clang::ParmVarDecl *parm) {
    dynamicLimits_.push_back(parm);
  };

  void setStaticLimits(bool hasStaticLimits) {
      hasStaticallyAllocatedLimits_ = hasStaticLimits;
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
