#ifndef WEBCLVALIDATOR_RADIXSORTVERIFIER
#define WEBCLVALIDATOR_RADIXSORTVERIFIER

#include "builder.hpp"

class RadixSorter;

class RadixSortVerifier : public OpenCLBuilder
{
public:

    RadixSortVerifier(const std::string &name);
    virtual ~RadixSortVerifier();

    bool verifySorter(RadixSorter &sorter);
};

#endif // WEBCLVALIDATOR_RADIXSORTVERIFIER
