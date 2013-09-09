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
