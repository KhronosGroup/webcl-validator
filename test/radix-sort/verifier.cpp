#include "sorter.hpp"
#include "verifier.hpp"




RadixSortVerifier::RadixSortVerifier(const std::string &name)
    : OpenCLBuilderForOnePlatformAndDevice(name, "Portable OpenCL", "pthread")
{
}

RadixSortVerifier::~RadixSortVerifier()
{
}

bool RadixSortVerifier::verifySorter(RadixSorter &sorter)
{
    if (!compileInput(sorter, ""))
        return false;

    if (!sorter.createUnsortedElements()) {
        std::cerr << name_ << ": Can't create unsorted element buffer." << std::endl;
        return false;
    }
    timeval allocate_buffers_begin;
    gettimeofday(&allocate_buffers_begin, NULL);
    if (!sorter.createHistogram()) {
        std::cerr << name_ << ": Can't create histogram buffer." << std::endl;
        return false;
    }

    if (!sorter.populateBuffers()) {
        std::cerr << name_ << ": Can't populate buffers." << std::endl;
        return false;
    }
    timeval allocate_buffers_end;
    gettimeofday(&allocate_buffers_end, NULL);


    timeval stream_kernel_begin;
    gettimeofday(&stream_kernel_begin, NULL);
    if (!sorter.createStreamCountKernel()) {
        std::cerr << name_ << ": Can't create stream count kernel." << std::endl;
        return false;
    }
    timeval stream_kernel_end;
    gettimeofday(&stream_kernel_end, NULL);

  
    timeval prefix_kernel_begin;
    gettimeofday(&prefix_kernel_begin, NULL);
    if (!sorter.createPrefixScanKernel()) {
        std::cerr << name_ << ": Can't create prefix scan kernel." << std::endl;
        return false;
    }
    timeval prefix_kernel_end;
    gettimeofday(&prefix_kernel_end, NULL);


    timeval stream_count_begin;
    gettimeofday(&stream_count_begin, NULL);
    cl_event streamCountEvent;
    if (!sorter.runStreamCountKernel(&streamCountEvent)) {
        std::cerr << name_ << ": Can't run stream count kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForStreamCountResults(streamCountEvent)) {
        std::cerr << name_ << ": Can't get stream count results." << std::endl;
        return false;
    }
    timeval stream_count_end;
    gettimeofday(&stream_count_end, NULL);



    if (!sorter.verifyStreamCountResults()) {
        std::cerr << name_ << ": Stream count results are invalid." << std::endl;
        return false;
    }

    timeval prefix_scan_begin;
    gettimeofday(&prefix_scan_begin, NULL);
    cl_event prefixScanEvent;
    if (!sorter.runPrefixScanKernel(streamCountEvent, &prefixScanEvent)) {
        std::cerr << name_ << ": Can't run prefix scan kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForPrefixScanResults(prefixScanEvent)) {
        std::cerr << name_ << ": Can't get prefix scan results." << std::endl;
        return false;
    }

    timeval prefix_scan_end;
    gettimeofday(&prefix_scan_end, NULL);
    double prefix_scan_ms = diff_ms_helper(prefix_scan_end, prefix_scan_begin);
    double allocate_buffers_ms = diff_ms_helper(allocate_buffers_end, allocate_buffers_begin);
    double stream_count_ms = diff_ms_helper(stream_count_end, stream_count_begin);
    double prefix_kernel_ms = diff_ms_helper(prefix_kernel_end, prefix_kernel_begin);
    double stream_kernel_ms = diff_ms_helper(prefix_kernel_end, prefix_kernel_begin);

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
