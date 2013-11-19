#ifndef WEBCLVALIDATOR_RADIXSORTER
#define WEBCLVALIDATOR_RADIXSORTER

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

#include "validator.hpp"

#include <vector>

class RadixSorter : public OpenCLValidator
{
public:

    explicit RadixSorter(bool transform, size_t numElements);
    virtual ~RadixSorter();

    bool createUnsortedElements();
    bool createHistogram();
    bool populateBuffers();
    bool createStreamCountKernel();
    bool createPrefixScanKernel();
    bool runStreamCountKernel(cl_event *streamCountEvent);
    bool runPrefixScanKernel(cl_event streamCountEvent, cl_event *prefixScanEvent);
    bool waitForStreamCountResults(cl_event streamCountEvent);
    bool waitForPrefixScanResults(cl_event prefixScanEvent);
    bool verifyStreamCountResults();
    bool verifyPrefixScanResults();

private:

    bool waitForHistogramResults(cl_event kernelEvent);
    bool verifyHistogramResults(const std::vector<unsigned int> &correctValues);

    const bool transform_;

    const size_t numValueBits_;
    const size_t numValues_;

    const size_t numStreamCountElementsPerItem_;
    const size_t numStreamCountItems_;
    const size_t numStreamCountBlocksPerGroup_;
    const size_t numStreamCountGroups_;

    const size_t numPrefixScanItems_;
    const size_t numPrefixScanGroups_;

    unsigned int *unsortedValues_;
    const cl_ulong numUnsortedElements_; // size_t can't be passed to AMD
    const size_t numUnsortedBytes_;
    unsigned int *histogram_;
    const cl_ulong numHistogramElements_; // size_t can't be passed to AMD
    const size_t numHistogramBytes_;

    cl_mem unsortedValuesBuffer_;
    cl_mem histogramBuffer_;

    cl_kernel streamCountKernel_;
    cl_kernel prefixScanKernel_;

    std::vector<unsigned int> correctCounts_;
    std::vector<unsigned int> correctSums_;

};

#endif // WEBCLVALIDATOR_RADIXSORTER
