#include "WebCLHelper.hpp"

AddressSpaceLimits::AddressSpaceLimits(unsigned addressSpace)
    : hasStaticLimits_(false)
    , addressSpace_(addressSpace)
{
}

AddressSpaceLimits::~AddressSpaceLimits()
{
}

void AddressSpaceLimits::insert(clang::ParmVarDecl *parm)
{
    dynamicLimits_.push_back(parm);
}

void AddressSpaceLimits::setStaticLimits(bool hasStaticLimits)
{
      hasStaticLimits_ = hasStaticLimits;
}

bool AddressSpaceLimits::hasStaticallyAllocatedLimits()
{
    return hasStaticLimits_;
}

unsigned AddressSpaceLimits::getAddressSpace()
{
    return addressSpace_;
}
  
bool AddressSpaceLimits::empty()
{
    return !hasStaticLimits_ && dynamicLimits_.empty();
}

unsigned AddressSpaceLimits::count()
{
    return dynamicLimits_.size() + (hasStaticallyAllocatedLimits() ? 1 : 0);
}
    
AddressSpaceLimits::LimitList &AddressSpaceLimits::getDynamicLimits()
{
    return dynamicLimits_;
}
