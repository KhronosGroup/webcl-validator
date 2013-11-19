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
#include "verifier.hpp"

#include "llvm/Support/TimeValue.h"


RadixSortVerifier::RadixSortVerifier(const std::string &name)
    : OpenCLBuilderForOnePlatformAndDevice(name, "Portable OpenCL", "pthread")
{
}

RadixSortVerifier::~RadixSortVerifier()
{
}

bool RadixSortVerifier::verifySorter(RadixSorter &sorter)
{
    using llvm::sys::TimeValue;

    if (!compileInput(sorter, ""))
        return false;

    if (!sorter.createUnsortedElements()) {
        std::cerr << name_ << ": Can't create unsorted element buffer." << std::endl;
        return false;
    }

    TimeValue allocate_buffers_begin = TimeValue::now();
    if (!sorter.createHistogram()) {
        std::cerr << name_ << ": Can't create histogram buffer." << std::endl;
        return false;
    }

    if (!sorter.populateBuffers()) {
        std::cerr << name_ << ": Can't populate buffers." << std::endl;
        return false;
    }
    TimeValue allocate_buffers_end = TimeValue::now();

    TimeValue stream_kernel_begin = TimeValue::now();
    if (!sorter.createStreamCountKernel()) {
        std::cerr << name_ << ": Can't create stream count kernel." << std::endl;
        return false;
    }
    TimeValue stream_kernel_end = TimeValue::now();

    TimeValue prefix_kernel_begin = TimeValue::now();
    if (!sorter.createPrefixScanKernel()) {
        std::cerr << name_ << ": Can't create prefix scan kernel." << std::endl;
        return false;
    }
    TimeValue prefix_kernel_end = TimeValue::now();

    TimeValue stream_count_begin = TimeValue::now();
    cl_event streamCountEvent;
    if (!sorter.runStreamCountKernel(&streamCountEvent)) {
        std::cerr << name_ << ": Can't run stream count kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForStreamCountResults(streamCountEvent)) {
        std::cerr << name_ << ": Can't get stream count results." << std::endl;
        return false;
    }
    TimeValue stream_count_end = TimeValue::now();



    if (!sorter.verifyStreamCountResults()) {
        std::cerr << name_ << ": Stream count results are invalid." << std::endl;
        return false;
    }

    TimeValue prefix_scan_begin = TimeValue::now();
    cl_event prefixScanEvent;
    if (!sorter.runPrefixScanKernel(streamCountEvent, &prefixScanEvent)) {
        std::cerr << name_ << ": Can't run prefix scan kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForPrefixScanResults(prefixScanEvent)) {
        std::cerr << name_ << ": Can't get prefix scan results." << std::endl;
        return false;
    }

    TimeValue prefix_scan_end = TimeValue::now();
    double prefix_scan_ms = (prefix_scan_end - prefix_scan_begin).msec();
    double allocate_buffers_ms = (allocate_buffers_end - allocate_buffers_begin).msec();
    double stream_count_ms = (stream_count_end - stream_count_begin).msec();
    double prefix_kernel_ms = (prefix_kernel_end - prefix_kernel_begin).msec();
    double stream_kernel_ms = (stream_kernel_end - stream_kernel_begin).msec();

    double elapsed_ms = allocate_buffers_ms + stream_kernel_ms + prefix_kernel_ms + stream_count_ms + prefix_scan_ms;

    std::cerr << "allocate_buffers: " << allocate_buffers_ms << "ms\n"
	      << "stream_compile_kernel: " << stream_kernel_ms << "ms\n"
	      << "prefix_compile_kernel: " << prefix_kernel_ms << "ms\n"
	      << "stream_count_kernel: " << stream_count_ms << "ms\n"
	      << "prefix_scan_kernel:" << prefix_scan_ms << "ms\n"
	      << "total:" << elapsed_ms << "ms\n";

    if (!sorter.verifyPrefixScanResults()) {
        std::cerr << name_ << ": Prefix scan results are invalid." << std::endl;
        return false;
    }

    return true;
}
