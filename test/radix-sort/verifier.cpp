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

    if (!sorter.createHistogram()) {
        std::cerr << name_ << ": Can't create histogram buffer." << std::endl;
        return false;
    }

    if (!sorter.populateBuffers()) {
        std::cerr << name_ << ": Can't populate buffers." << std::endl;
        return false;
    }

    if (!sorter.createStreamCountKernel()) {
        std::cerr << name_ << ": Can't create stream count kernel." << std::endl;
        return false;
    }

    if (!sorter.createPrefixScanKernel()) {
        std::cerr << name_ << ": Can't create prefix scan kernel." << std::endl;
        return false;
    }

    cl_event streamCountEvent;
    if (!sorter.runStreamCountKernel(&streamCountEvent)) {
        std::cerr << name_ << ": Can't run stream count kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForStreamCountResults(streamCountEvent)) {
        std::cerr << name_ << ": Can't get stream count results." << std::endl;
        return false;
    }

    if (!sorter.verifyStreamCountResults()) {
        std::cerr << name_ << ": Stream count results are invalid." << std::endl;
        return false;
    }

    cl_event prefixScanEvent;
    if (!sorter.runPrefixScanKernel(streamCountEvent, &prefixScanEvent)) {
        std::cerr << name_ << ": Can't run prefix scan kernel." << std::endl;
        return false;
    }

    if (!sorter.waitForPrefixScanResults(prefixScanEvent)) {
        std::cerr << name_ << ": Can't get prefix scan results." << std::endl;
        return false;
    }

    if (!sorter.verifyPrefixScanResults()) {
        std::cerr << name_ << ": Prefix scan results are invalid." << std::endl;
        return false;
    }

    return true;
}
