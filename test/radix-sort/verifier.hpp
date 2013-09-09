#ifndef WEBCLVALIDATOR_RADIXSORTVERIFIER
#define WEBCLVALIDATOR_RADIXSORTVERIFIER

/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

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
