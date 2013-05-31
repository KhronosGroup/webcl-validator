#ifndef WEBCLVALIDATOR_RADIXSORTER
#define WEBCLVALIDATOR_RADIXSORTER

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
    const size_t numUnsortedElements_;
    const size_t numUnsortedBytes_;
    unsigned int *histogram_;
    const size_t numHistogramElements_;
    const size_t numHistogramBytes_;

    cl_mem unsortedValuesBuffer_;
    cl_mem histogramBuffer_;

    cl_kernel streamCountKernel_;
    cl_kernel prefixScanKernel_;

    std::vector<unsigned int> correctCounts_;
    std::vector<unsigned int> correctSums_;

};

#endif // WEBCLVALIDATOR_RADIXSORTER
