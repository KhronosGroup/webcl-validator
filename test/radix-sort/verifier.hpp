#ifndef WEBCLVALIDATOR_RADIXSORTVERIFIER
#define WEBCLVALIDATOR_RADIXSORTVERIFIER

#include "builder.hpp"

#include <sys/time.h>

class RadixSorter;

class RadixSortVerifier : public OpenCLBuilderForOnePlatformAndDevice
{
public:

    RadixSortVerifier(const std::string &name);
    virtual ~RadixSortVerifier();

    bool verifySorter(RadixSorter &sorter);

private:
  int diff_ms_helper(timeval t1, timeval t2){
    return (((t1.tv_sec - t2.tv_sec)* 1000000) + 
	    (t1.tv_usec - t2.tv_usec))/1000;

  };
};

#endif // WEBCLVALIDATOR_RADIXSORTVERIFIER
