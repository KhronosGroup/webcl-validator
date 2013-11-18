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

#include "sorter.hpp"

#include <algorithm>
#include <cstdlib>

RadixSorter::RadixSorter(bool transform, size_t numElements)
  : OpenCLValidator()
  , transform_(transform)
  , numValueBits_(4), numValues_(1 << numValueBits_)
  , numStreamCountElementsPerItem_(4)
  , numStreamCountItems_(64)
  , numStreamCountBlocksPerGroup_(2)
  , numStreamCountGroups_(
      numElements /
      (numStreamCountBlocksPerGroup_ * numStreamCountItems_ * numStreamCountElementsPerItem_))
  , numPrefixScanItems_(128)
  , numPrefixScanGroups_(1)
  , unsortedValues_(0)
  , numUnsortedElements_(numElements)
  , numUnsortedBytes_(numUnsortedElements_ * sizeof(unsigned int))
  , histogram_(0)
  , numHistogramElements_(numValues_ * numStreamCountGroups_)
  , numHistogramBytes_(numHistogramElements_ * sizeof(unsigned int))
  , unsortedValuesBuffer_(0), histogramBuffer_(0)
  , streamCountKernel_(0), prefixScanKernel_(0)
  , correctCounts_(), correctSums_()
{
}
       
RadixSorter::~RadixSorter()
{
    if (streamCountKernel_)
        clReleaseKernel(streamCountKernel_);
    if (prefixScanKernel_)
        clReleaseKernel(prefixScanKernel_);

    if (unsortedValuesBuffer_)
        clReleaseMemObject(unsortedValuesBuffer_);
    if (histogramBuffer_)
        clReleaseMemObject(histogramBuffer_);

    delete[] unsortedValues_;
    unsortedValues_ = 0;
    delete[] histogram_;
    histogram_ = 0;
}

bool RadixSorter::createUnsortedElements()
{
    unsortedValues_ = new unsigned int[numUnsortedElements_];
    if (!unsortedValues_)
        return false;

    // CL_MEM_USE_HOST_PTR causes invalid work group size error on OSX
    // just allocate mem to be able to fill it later on...
    unsortedValuesBuffer_ = clCreateBuffer(
        context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        numUnsortedBytes_, unsortedValues_, NULL);
    if (!unsortedValuesBuffer_)
        return false;

    return true;
}

bool RadixSorter::createHistogram()
{
    histogram_ = new unsigned int[numHistogramElements_];
    if (!histogram_)
        return false;

    histogramBuffer_ =  clCreateBuffer(
        context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        numHistogramBytes_, histogram_, NULL);
    if (!histogramBuffer_)
        return false;

    return true;
}

bool RadixSorter::populateBuffers()
{
    std::srand(0x5eed);
    for (size_t i = 0; i < numUnsortedElements_; ++i)
        unsortedValues_[i] = std::rand() % numValues_;

    const size_t elementsPerBlock = numStreamCountItems_ * numStreamCountElementsPerItem_;
    const size_t elementsPerGroup = numStreamCountBlocksPerGroup_ * elementsPerBlock;

    if (numUnsortedElements_ != (numStreamCountGroups_ * elementsPerGroup))
        return false;

    std::vector<unsigned int> counts(numValues_ * numStreamCountGroups_, 0);
    for (size_t g = 0; g < numStreamCountGroups_; ++g) {
        std::vector<unsigned int> groupCounts(numValues_, 0);
        for (size_t i = 0; i < numStreamCountItems_; ++i) {
            for (size_t b = 0; b < numStreamCountBlocksPerGroup_; ++b) {
                for (size_t e = 0; e < numStreamCountElementsPerItem_; ++e) {
                    const size_t index =
                        (g * elementsPerGroup) +
                        (b * elementsPerBlock) +
                        (i * numStreamCountElementsPerItem_) + e;
                    ++groupCounts.at(unsortedValues_[index]);
                }
            }
        }
        for (size_t v = 0; v < groupCounts.size(); ++v) {
            const size_t index = (v * numStreamCountGroups_) + g;
            counts.at(index) = groupCounts.at(v);
        }
    }

    size_t index = 0;
    unsigned int sum = 0;
    for (size_t v = 0; v < numValues_; ++v) {
        for (size_t g = 0; g < numStreamCountGroups_; ++g) {
            const unsigned int count = counts.at(index);
            correctCounts_.push_back(count);
            correctSums_.push_back(sum);
            sum += count;
            ++index;
        }
    }

    return true;
}

bool RadixSorter::createStreamCountKernel()
{
    streamCountKernel_ = clCreateKernel(program_, "stream_count_kernel", NULL);
    if (!streamCountKernel_)
        return false;

    unsigned int index = 0;

    if (clSetKernelArg(streamCountKernel_, index++, sizeof(unsortedValuesBuffer_), &unsortedValuesBuffer_) != CL_SUCCESS)
        return false;
    if (transform_ && (clSetKernelArg(streamCountKernel_, index++, sizeof(numUnsortedElements_), &numUnsortedElements_) != CL_SUCCESS))
        return false;
    if (clSetKernelArg(streamCountKernel_, index++, sizeof(histogramBuffer_), &histogramBuffer_) != CL_SUCCESS)
        return false;
    if (transform_ && (clSetKernelArg(streamCountKernel_, index++, sizeof(numHistogramElements_), &numHistogramElements_) != CL_SUCCESS))
        return false;
    const cl_int bitOffset = 0;
    if (clSetKernelArg(streamCountKernel_, index++, sizeof(bitOffset), &bitOffset) != CL_SUCCESS)
        return false;
    const cl_int numElements = numUnsortedElements_;
    if (clSetKernelArg(streamCountKernel_, index++, sizeof(numElements), &numElements) != CL_SUCCESS)
        return false;
    const cl_int numWorkGroups = numStreamCountGroups_;
    if (clSetKernelArg(streamCountKernel_, index++, sizeof(numWorkGroups), &numWorkGroups) != CL_SUCCESS)
        return false;
    const cl_int numBlocksInWorkGroup = numStreamCountBlocksPerGroup_;
    if (clSetKernelArg(streamCountKernel_, index++, sizeof(numBlocksInWorkGroup), &numBlocksInWorkGroup) != CL_SUCCESS)
        return false;

    return true;
}

bool RadixSorter::createPrefixScanKernel()
{
    prefixScanKernel_ = clCreateKernel(program_, "prefix_scan_kernel", NULL);
    if (!prefixScanKernel_)
        return false;

    unsigned int index = 0;

    if (clSetKernelArg(prefixScanKernel_, index++, sizeof(histogramBuffer_), &histogramBuffer_) != CL_SUCCESS)
        return false;
    if (transform_ && (clSetKernelArg(prefixScanKernel_, index++, sizeof(numHistogramElements_), &numHistogramElements_) != CL_SUCCESS))
        return false;
    const cl_int numWorkGroups = numStreamCountGroups_;
    if (clSetKernelArg(prefixScanKernel_, index++, sizeof(numWorkGroups), &numWorkGroups) != CL_SUCCESS)
        return false;

    return true;
}

bool RadixSorter::runStreamCountKernel(cl_event *streamCountEvent)
{
    cl_event unsortedValuesEvent;
    if (clEnqueueWriteBuffer(queue_, unsortedValuesBuffer_, CL_FALSE,
                             0, numUnsortedBytes_, unsortedValues_,
                             0, NULL, &unsortedValuesEvent) != CL_SUCCESS) {
      std::cerr << "Could not enqueue write buffer!\n";
      return false;
    }

    size_t localWorkSize = numStreamCountItems_;
    size_t globalWorkSize = numUnsortedElements_ /
        (numStreamCountElementsPerItem_ * numStreamCountBlocksPerGroup_);

    // query work group size from kernel attrbutes
    size_t wgs[3];
    clGetKernelWorkGroupInfo(streamCountKernel_, device_, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(wgs), wgs, NULL);
    size_t maxSize, kernelMaxSize;
    clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxSize), &maxSize, NULL);
    clGetKernelWorkGroupInfo(streamCountKernel_, device_, CL_KERNEL_WORK_GROUP_SIZE, sizeof(kernelMaxSize), &kernelMaxSize, NULL);

    std::cerr << "Local work group max size (CL_DEVICE_MAX_WORK_GROUP_SIZE): " << maxSize << "\n";
    std::cerr << "Kernel work group max size (CL_KERNEL_WORK_GROUP_SIZE): " << kernelMaxSize << "\n";
    std::cerr << "Local work group size: " << localWorkSize << " Queried wgs: " << wgs[0] << "," << wgs[1] << "," << wgs[2] << "\n";
    std::cerr << "Global work group size: " << globalWorkSize << "\n";


    cl_int retVal = clEnqueueNDRangeKernel(queue_, streamCountKernel_, 1,
                                           NULL, &globalWorkSize, &localWorkSize,
                                           1, &unsortedValuesEvent, streamCountEvent);

    if (retVal != CL_SUCCESS) {
      std::cerr << "CL_INVALID_WORK_GROUP_SIZE code: " << CL_INVALID_WORK_GROUP_SIZE << "\n";
      std::cerr << "CL_OUT_OF_RESOURCES code: " << CL_OUT_OF_RESOURCES << "\n";
      std::cerr << "clEnqueueNDRangeKernel got error code: " << retVal << "\n";
      return false;
    };
    return true;
}

bool RadixSorter::runPrefixScanKernel(cl_event streamCountEvent, cl_event *prefixScanEvent)
{
    // must be 128 and 20 (defined in kernel code)
    const size_t localWorkSize = 128;
    const size_t globalWorkSize = localWorkSize;

    if (numHistogramElements_ != localWorkSize*20) {
        std::cerr << "Fail: Fix this to work on bigger element count.\n";
        return false;
    }

    std::cerr << "Going to run prefix scan kernel:\n";
    std::cerr << "Local work size: " << localWorkSize << "\n";
    std::cerr << "Global work size: " << globalWorkSize << "\n";
    
    cl_int retVal = clEnqueueNDRangeKernel(queue_, prefixScanKernel_, 1,
                                           NULL, &globalWorkSize, &localWorkSize,
                                           1, &streamCountEvent, prefixScanEvent);
    if (retVal != CL_SUCCESS) {
      std::cerr << "clEnqueueNDRangeKernel got error code: " << retVal << "\n";
      return false;
    };
    return true;
}

bool RadixSorter::waitForStreamCountResults(cl_event streamCountEvent)
{
    return waitForHistogramResults(streamCountEvent);
}

bool RadixSorter::waitForPrefixScanResults(cl_event prefixScanEvent)
{
    return waitForHistogramResults(prefixScanEvent);
}

bool RadixSorter::verifyStreamCountResults()
{
    return verifyHistogramResults(correctCounts_);
}

bool RadixSorter::verifyPrefixScanResults()
{
    return verifyHistogramResults(correctSums_);
}

bool RadixSorter::waitForHistogramResults(cl_event kernelEvent)
{
    cl_event histogramEvent;
    if (clEnqueueReadBuffer(queue_, histogramBuffer_, CL_FALSE,
                            0, numHistogramBytes_, histogram_,
                            1, &kernelEvent, &histogramEvent) != CL_SUCCESS) {
        return false;
    }
    return clWaitForEvents(1, &histogramEvent) == CL_SUCCESS;
}

bool RadixSorter::verifyHistogramResults(const std::vector<unsigned int> &correctValues)
{
    if (correctValues.size() != numHistogramElements_)
        return false;
    return std::equal(correctValues.begin(), correctValues.end(), histogram_);
}
