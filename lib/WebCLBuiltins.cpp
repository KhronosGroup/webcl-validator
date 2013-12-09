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

#include "WebCLBuiltins.hpp"

static const char *hashReplacements[] = {
    "2", "3", "4", "8", "16"
};
static const int numHashReplacements =
    sizeof(hashReplacements) / sizeof(hashReplacements[0]);

static const char *unsafeMathBuiltins[] = {
    "fract", "frexp", "lgamma_r", "modf", "remquo", "sincos"
};
static const int numUnsafeMathBuiltins =
    sizeof(unsafeMathBuiltins) / sizeof(unsafeMathBuiltins[0]);

static const char *unsafeVectorBuiltins[] = {
    "vload#", "vload_half", "vload_half#", "vloada_half#",
    "vstore#", "vstore_half", "vstore_half#", "vstorea_#", "vstorea_half#",
    "vstore_half_rte", "vstore_half_rtz", "vstore_half_rtp", "vstore_half_rtn",
    "vstore_half#_rte", "vstore_half#_rtz", "vstore_half#_rtp", "vstore_half#_rtn",
    "vstorea_half_rte", "vstorea_half_rtz", "vstorea_half_rtp", "vstorea_half_rtn",
    "vstorea_half#_rte", "vstorea_half#_rtz", "vstorea_half#_rtp", "vstorea_half#_rtn"
};
static const int numUnsafeVectorBuiltins =
    sizeof(unsafeVectorBuiltins) / sizeof(unsafeVectorBuiltins[0]);

static const char *unsafeAtomicBuiltins[] = {
    "atomic_add", "atomic_sub",
    "atomic_inc", "atomic_dec",
    "atomic_xchg", "atomic_cmpxchg",
    "atomic_min", "atomic_max",
    "atomic_and", "atomic_or", "atomic_xor"
};
static const int numUnsafeAtomicBuiltins =
    sizeof(unsafeAtomicBuiltins) / sizeof(unsafeAtomicBuiltins[0]);

static const char *unsupportedBuiltins[] = {
    "async_work_group_copy", "async_work_group_strided_copy",
    "wait_group_events", "prefetch"
};
static const int numUnsupportedBuiltins =
    sizeof(unsupportedBuiltins) / sizeof(unsupportedBuiltins[0]);

static const char *safeBuiltins[] = {
  "get_image_width", "get_image_height"
};
static const int numSafeBuiltins =
    sizeof(safeBuiltins) / sizeof(safeBuiltins[0]);

WebCLBuiltins::WebCLBuiltins()
    : unsafeMathBuiltins_()
    , unsafeVectorBuiltins_()
    , unsafeAtomicBuiltins_()
    , unsupportedBuiltins_()
    , safeBuiltins_()
{
    initialize(unsafeMathBuiltins_, unsafeMathBuiltins, numUnsafeMathBuiltins);
    initialize(unsafeVectorBuiltins_, unsafeVectorBuiltins, numUnsafeVectorBuiltins);
    initialize(unsafeAtomicBuiltins_, unsafeAtomicBuiltins, numUnsafeAtomicBuiltins);
    initialize(unsupportedBuiltins_, unsupportedBuiltins, numUnsupportedBuiltins);
    initialize(safeBuiltins_, safeBuiltins, numSafeBuiltins);
}

WebCLBuiltins::~WebCLBuiltins()
{
}


bool WebCLBuiltins::isSafe(const std::string &builtin) const
{
    return safeBuiltins_.count(builtin);
}

bool WebCLBuiltins::isUnsafe(const std::string &builtin) const
{
    return
        unsafeMathBuiltins_.count(builtin) ||
        unsafeVectorBuiltins_.count(builtin) ||
        unsafeAtomicBuiltins_.count(builtin);
}

bool WebCLBuiltins::isUnsupported(const std::string &builtin) const
{
    return unsupportedBuiltins_.count(builtin);
}

void WebCLBuiltins::initialize(
    BuiltinNames &names, const char *patterns[], int numPatterns)
{
    for (int i = 0; i < numPatterns; ++i) {
        const std::string pattern = patterns[i];
        const size_t hashPosition = pattern.find_last_of('#');
        if (hashPosition == std::string::npos) {
            names.insert(pattern);
        } else {
            for (int j = 0; j < numHashReplacements; ++j) {
                std::string name;
                name += pattern.substr(0,
                                       hashPosition);
                name += hashReplacements[j];
                name += pattern.substr(hashPosition + 1,
                                       pattern.length() - hashPosition - 1);
                names.insert(name);
            }
        }
    }
}
