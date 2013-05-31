#include "sorter.hpp"
#include "verifier.hpp"

#include <set>

int main(int argc, char const* argv[])
{
    const std::string original = "-original";
    const std::string transform = "-transform";
    std::set<std::string> mode;
    mode.insert(original);
    mode.insert(transform);

    if ((argc != 2) || ((argc == 2) && !mode.count(argv[1]))) {
        std::cerr << "Usage: cat FILE | " << argv[0] << " -original|-transform"
                  << std::endl;
        std::cerr << "Checks whether radix sort kernels (FILE) produce correct results."
                  << "\n"
                  << "Use -original for input.cl and -transform for output.cl."
                  << std::endl;
        return EXIT_FAILURE;
    }

    RadixSortVerifier verifier(argv[0]);
    RadixSorter sorter(!transform.compare(argv[1]), 160 * 2 * 64 * 4);
    if (!verifier.verifySorter(sorter))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
