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
#include "WebCLDebug.hpp"

#include <cstring>
#include <iostream>

#include "llvm/Support/raw_ostream.h"

static const char *hashReplacements[] = {
    "2", "3", "4", "8", "16"
};
static const int numHashReplacements =
    sizeof(hashReplacements) / sizeof(hashReplacements[0]);

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

static const char *unsupportedBuiltins[] = {
    "async_work_group_copy", "async_work_group_strided_copy",
    "wait_group_events", "prefetch"
};
static const int numUnsupportedBuiltins =
    sizeof(unsupportedBuiltins) / sizeof(unsupportedBuiltins[0]);

static const char *safeBuiltins[] = {
    "get_image_width", "get_image_height",
    "atomic_add", "atomic_sub",
    "atomic_inc", "atomic_dec",
    "atomic_xchg", "atomic_cmpxchg",
    "atomic_min", "atomic_max",
    "atomic_and", "atomic_or", "atomic_xor",
    "fract", "frexp", "lgamma_r", "modf", "remquo", "sincos"
};
static const int numSafeBuiltins =
    sizeof(safeBuiltins) / sizeof(safeBuiltins[0]);

static const char *roundingSuffixes[] = {
    "_rtz", "_rte", "_rtp", "_rtn",
    "" // must be last to avoid emitting by accident
};
static const int numRoundingSuffixes =
    sizeof(roundingSuffixes) / sizeof(roundingSuffixes[0]);

static const char *vloadHalfPrefix = "vload_half";
static const char *vloadaHalfPrefix = "vloada_half";

static const char *vstoreHalfPrefix = "vstorea_half";
static const char *vstoreaHalfPrefix = "vstorea_half";

static struct {
    const char *name;
    const char *decl;
} builtinDecls[] = {
    { "acos", "_CL_DECLARE_FUNC_V_V(acos)\n" },
    { "acosh", "_CL_DECLARE_FUNC_V_V(acosh)\n" },
    { "acospi", "_CL_DECLARE_FUNC_V_V(acospi)\n" },
    { "asin", "_CL_DECLARE_FUNC_V_V(asin)\n" },
    { "asinh", "_CL_DECLARE_FUNC_V_V(asinh)\n" },
    { "asinpi", "_CL_DECLARE_FUNC_V_V(asinpi)\n" },
    { "atan", "_CL_DECLARE_FUNC_V_V(atan)\n" },
    { "atan2", "_CL_DECLARE_FUNC_V_VV(atan2)\n" },
    { "atan2pi", "_CL_DECLARE_FUNC_V_VV(atan2pi)\n" },
    { "atanh", "_CL_DECLARE_FUNC_V_V(atanh)\n" },
    { "atanpi", "_CL_DECLARE_FUNC_V_V(atanpi)\n" },
    { "cbrt", "_CL_DECLARE_FUNC_V_V(cbrt)\n" },
    { "ceil", "_CL_DECLARE_FUNC_V_V(ceil)\n" },
    { "copysign", "_CL_DECLARE_FUNC_V_VV(copysign)\n" },
    { "cos", "_CL_DECLARE_FUNC_V_V(cos)\n" },
    { "cosh", "_CL_DECLARE_FUNC_V_V(cosh)\n" },
    { "cospi", "_CL_DECLARE_FUNC_V_V(cospi)\n" },
    { "dot", "_CL_DECLARE_FUNC_S_VV(dot)\n" },
    { "erfc", "_CL_DECLARE_FUNC_V_V(erfc)\n" },
    { "erf", "_CL_DECLARE_FUNC_V_V(erf)\n" },
    { "exp", "_CL_DECLARE_FUNC_V_V(exp)\n" },
    { "exp2", "_CL_DECLARE_FUNC_V_V(exp2)\n" },
    { "exp10", "_CL_DECLARE_FUNC_V_V(exp10)\n" },
    { "expm1", "_CL_DECLARE_FUNC_V_V(expm1)\n" },
    { "fabs", "_CL_DECLARE_FUNC_V_V(fabs)\n" },
    { "fdim", "_CL_DECLARE_FUNC_V_VV(fdim)\n" },
    { "floor", "_CL_DECLARE_FUNC_V_V(floor)\n" },
    { "fma", "_CL_DECLARE_FUNC_V_VVV(fma)\n" },
    { "fmax", "_CL_DECLARE_FUNC_V_VV(fmax)\n" },
    { "fmax", "_CL_DECLARE_FUNC_V_VS(fmax)\n" },
    { "fmin", "_CL_DECLARE_FUNC_V_VV(fmin)\n" },
    { "fmin", "_CL_DECLARE_FUNC_V_VS(fmin)\n" },
    { "fmod", "_CL_DECLARE_FUNC_V_VV(fmod)\n" },
    { "fract", "_CL_DECLARE_FUNC_V_VPV(fract)\n" },
    { "frexp", "_CL_DECLARE_FUNC_V_VPVI(frexp)\n_CL_DECLARE_FUNC_H_HPVI(frexp)\n" },
    { "hypot", "_CL_DECLARE_FUNC_V_VV(hypot)\n" },
    { "ilogb", "_CL_DECLARE_FUNC_K_V(ilogb)\n" },
    { "ldexp", "_CL_DECLARE_FUNC_V_VJ(ldexp)\n" },
    { "ldexp", "_CL_DECLARE_FUNC_V_VI(ldexp)\n" },
    { "lgamma", "_CL_DECLARE_FUNC_V_V(lgamma)\n" },
    { "lgamma_r", "_CL_DECLARE_FUNC_V_VPVI(lgamma_r)\n_CL_DECLARE_FUNC_H_HPVI(lgamma_r)\n" },
    { "log", "_CL_DECLARE_FUNC_V_V(log)\n" },
    { "log2", "_CL_DECLARE_FUNC_V_V(log2)\n" },
    { "log10", "_CL_DECLARE_FUNC_V_V(log10)\n" },
    { "log1p", "_CL_DECLARE_FUNC_V_V(log1p)\n" },
    { "logb", "_CL_DECLARE_FUNC_V_V(logb)\n" },
    { "mad", "_CL_DECLARE_FUNC_V_VVV(mad)\n" },
    { "maxmag", "_CL_DECLARE_FUNC_V_VV(maxmag)\n" },
    { "minmag", "_CL_DECLARE_FUNC_V_VV(minmag)\n" },
    { "nan", "_CL_DECLARE_FUNC_V_U(nan)\n" },
    { "nextafter", "_CL_DECLARE_FUNC_V_VV(nextafter)\n" },
    { "pow", "_CL_DECLARE_FUNC_V_VV(pow)\n" },
    { "pown", "_CL_DECLARE_FUNC_V_VJ(pown)\n" },
    { "pown", "_CL_DECLARE_FUNC_V_VI(pown)\n" },
    { "powr", "_CL_DECLARE_FUNC_V_VV(powr)\n" },
    { "remainder", "_CL_DECLARE_FUNC_V_VV(remainder)\n" },
    { "rint", "_CL_DECLARE_FUNC_V_V(rint)\n" },
    { "rootn", "_CL_DECLARE_FUNC_V_VJ(rootn)\n" },
    { "rootn", "_CL_DECLARE_FUNC_V_VI(rootn)\n" },
    { "round", "_CL_DECLARE_FUNC_V_V(round)\n" },
    { "rsqrt", "_CL_DECLARE_FUNC_V_V(rsqrt)\n" },
    { "sin", "_CL_DECLARE_FUNC_V_V(sin)\n" },
    { "sincos", "_CL_DECLARE_FUNC_V_VPV(sincos)\n" },
    { "sinh", "_CL_DECLARE_FUNC_V_V(sinh)\n" },
    { "sinpi", "_CL_DECLARE_FUNC_V_V(sinpi)\n" },
    { "sqrt", "_CL_DECLARE_FUNC_V_V(sqrt)\n" },
    { "tan", "_CL_DECLARE_FUNC_V_V(tan)\n" },
    { "tanh", "_CL_DECLARE_FUNC_V_V(tanh)\n" },
    { "tanpi", "_CL_DECLARE_FUNC_V_V(tanpi)\n" },
    { "tgamma", "_CL_DECLARE_FUNC_V_V(tgamma)\n" },
    { "trunc", "_CL_DECLARE_FUNC_V_V(trunc)\n" },
    { "half_cos", "_CL_DECLARE_FUNC_F_F(half_cos)\n" },
    { "half_divide", "_CL_DECLARE_FUNC_F_FF(half_divide)\n" },
    { "half_exp", "_CL_DECLARE_FUNC_F_F(half_exp)\n" },
    { "half_exp2", "_CL_DECLARE_FUNC_F_F(half_exp2)\n" },
    { "half_exp10", "_CL_DECLARE_FUNC_F_F(half_exp10)\n" },
    { "half_log", "_CL_DECLARE_FUNC_F_F(half_log)\n" },
    { "half_log2", "_CL_DECLARE_FUNC_F_F(half_log2)\n" },
    { "half_log10", "_CL_DECLARE_FUNC_F_F(half_log10)\n" },
    { "half_powr", "_CL_DECLARE_FUNC_F_FF(half_powr)\n" },
    { "half_recip", "_CL_DECLARE_FUNC_F_F(half_recip)\n" },
    { "half_rsqrt", "_CL_DECLARE_FUNC_F_F(half_rsqrt)\n" },
    { "half_sin", "_CL_DECLARE_FUNC_F_F(half_sin)\n" },
    { "half_sqrt", "_CL_DECLARE_FUNC_F_F(half_sqrt)\n" },
    { "half_tan", "_CL_DECLARE_FUNC_F_F(half_tan)\n" },
    { "native_cos", "_CL_DECLARE_FUNC_F_F(native_cos)\n" },
    { "native_divide", "_CL_DECLARE_FUNC_F_FF(native_divide)\n" },
    { "native_exp", "_CL_DECLARE_FUNC_F_F(native_exp)\n" },
    { "native_exp2", "_CL_DECLARE_FUNC_F_F(native_exp2)\n" },
    { "native_exp10", "_CL_DECLARE_FUNC_F_F(native_exp10)\n" },
    { "native_log", "_CL_DECLARE_FUNC_F_F(native_log)\n" },
    { "native_log2", "_CL_DECLARE_FUNC_F_F(native_log2)\n" },
    { "native_log10", "_CL_DECLARE_FUNC_F_F(native_log10)\n" },
    { "native_powr", "_CL_DECLARE_FUNC_F_FF(native_powr)\n" },
    { "native_recip", "_CL_DECLARE_FUNC_F_F(native_recip)\n" },
    { "native_rsqrt", "_CL_DECLARE_FUNC_F_F(native_rsqrt)\n" },
    { "native_sin", "_CL_DECLARE_FUNC_F_F(native_sin)\n" },
    { "native_sqrt", "_CL_DECLARE_FUNC_F_F(native_sqrt)\n" },
    { "native_tan", "_CL_DECLARE_FUNC_F_F(native_tan)\n" },
    { "abs", "_CL_DECLARE_FUNC_UG_G(abs)\n" },
    { "abs_diff", "_CL_DECLARE_FUNC_UG_GG(abs_diff)\n" },
    { "add_sat", "_CL_DECLARE_FUNC_G_GG(add_sat)\n" },
    { "hadd", "_CL_DECLARE_FUNC_G_GG(hadd)\n" },
    { "remquo", "_CL_DECLARE_FUNC_V_VVPVI(remquo)\n_CL_DECLARE_FUNC_H_HHPVI(remquo)\n" },
    { "rhadd", "_CL_DECLARE_FUNC_G_GG(rhadd)\n" },
    { "clamp", "_CL_DECLARE_FUNC_G_GGG(clamp)\n" },
    { "clz", "_CL_DECLARE_FUNC_G_G(clz)\n" },
    { "mad_hi", "_CL_DECLARE_FUNC_G_GGG(mad_hi)\n" },
    { "mad_sat", "_CL_DECLARE_FUNC_G_GGG(mad_sat)\n" },
    { "max", "_CL_DECLARE_FUNC_G_GG(max)\n" },
    { "max", "_CL_DECLARE_FUNC_G_GS(max)\n" },
    { "min", "_CL_DECLARE_FUNC_G_GG(min)\n" },
    { "min", "_CL_DECLARE_FUNC_G_GS(min)\n" },
    { "mul_hi", "_CL_DECLARE_FUNC_G_GG(mul_hi)\n" },
    { "rotate", "_CL_DECLARE_FUNC_G_GG(rotate)\n" },
    { "sub_sat", "_CL_DECLARE_FUNC_G_GG(sub_sat)\n" },
    { "upsample", "_CL_DECLARE_FUNC_LG_GUG(upsample)\n" },
    { "popcount", "_CL_DECLARE_FUNC_G_G(popcount)\n" },
    { "mad24", "_CL_DECLARE_FUNC_J_JJJ(mad24)\n" },
    { "mul24", "_CL_DECLARE_FUNC_J_JJ(mul24)\n" },
    { "clamp", "_CL_DECLARE_FUNC_V_VVV(clamp)\n" },
    { "clamp", "_CL_DECLARE_FUNC_V_VSS(clamp)\n" },
    { "degrees", "_CL_DECLARE_FUNC_V_V(degrees)\n" },
    { "max", "_CL_DECLARE_FUNC_V_VV(max)\n" },
    { "max", "_CL_DECLARE_FUNC_V_VS(max)\n" },
    { "min", "_CL_DECLARE_FUNC_V_VV(min)\n" },
    { "min", "_CL_DECLARE_FUNC_V_VS(min)\n" },
    { "mix", "_CL_DECLARE_FUNC_V_VVV(mix)\n" },
    { "mix", "_CL_DECLARE_FUNC_V_VVS(mix)\n" },
    { "modf", "_CL_DECLARE_FUNC_V_VPV(modf)\n" },
    { "radians", "_CL_DECLARE_FUNC_V_V(radians)\n" },
    { "sincos", "_CL_DECLARE_FUNC_V_VPV(sincos)\n" },
    { "step", "_CL_DECLARE_FUNC_V_VV(step)\n" },
    { "step", "_CL_DECLARE_FUNC_V_SV(step)\n" },
    { "smoothstep", "_CL_DECLARE_FUNC_V_VVV(smoothstep)\n" },
    { "smoothstep", "_CL_DECLARE_FUNC_V_SSV(smoothstep)\n" },
    { "sign", "_CL_DECLARE_FUNC_V_V(sign)\n" },
    { "dot", "_CL_DECLARE_FUNC_S_VV(dot)\n" },
    { "distance", "_CL_DECLARE_FUNC_S_VV(distance)\n" },
    { "length", "_CL_DECLARE_FUNC_S_V(length)\n" },
    { "normalize", "_CL_DECLARE_FUNC_V_V(normalize)\n" },
    { "fast_distance", "_CL_DECLARE_FUNC_S_VV(fast_distance)\n" },
    { "fast_length", "_CL_DECLARE_FUNC_S_V(fast_length)\n" },
    { "fast_normalize", "_CL_DECLARE_FUNC_V_V(fast_normalize)\n" },
    { "isequal", "_CL_DECLARE_FUNC_J_VV(isequal)\n" },
    { "isnotequal", "_CL_DECLARE_FUNC_J_VV(isnotequal)\n" },
    { "isgreater", "_CL_DECLARE_FUNC_J_VV(isgreater)\n" },
    { "isgreaterequal", "_CL_DECLARE_FUNC_J_VV(isgreaterequal)\n" },
    { "isless", "_CL_DECLARE_FUNC_J_VV(isless)\n" },
    { "islessequal", "_CL_DECLARE_FUNC_J_VV(islessequal)\n" },
    { "islessgreater", "_CL_DECLARE_FUNC_J_VV(islessgreater)\n" },
    { "isfinite", "_CL_DECLARE_FUNC_J_V(isfinite)\n" },
    { "isinf", "_CL_DECLARE_FUNC_J_V(isinf)\n" },
    { "isnan", "_CL_DECLARE_FUNC_J_V(isnan)\n" },
    { "isnormal", "_CL_DECLARE_FUNC_J_V(isnormal)\n" },
    { "isordered", "_CL_DECLARE_FUNC_J_VV(isordered)\n" },
    { "isunordered", "_CL_DECLARE_FUNC_J_VV(isunordered)\n" },
    { "signbit", "_CL_DECLARE_FUNC_J_V(signbit)\n" },
    { "any", "_CL_DECLARE_FUNC_I_IG(any)\n" },
    { "all", "_CL_DECLARE_FUNC_I_IG(all)\n" },
    { "bitselect", "_CL_DECLARE_FUNC_G_GGG(bitselect)\n" },
    { "bitselect", "_CL_DECLARE_FUNC_V_VVV(bitselect)\n" },
    { "select", "_CL_DECLARE_FUNC_G_GGIG(select)\n" },
    { "select", "_CL_DECLARE_FUNC_G_GGUG(select)\n" },
    { "select", "_CL_DECLARE_FUNC_V_VVJ(select)\n" },
    { "select", "_CL_DECLARE_FUNC_V_VVU(select)\n" },
    { "cross", "float4 _CL_OVERLOADABLE cross(float4, float4);\n"
               "float3 _CL_OVERLOADABLE cross(float3, float3);\n"
                "#ifdef cl_khr_fp64\n"
                "double4 _CL_OVERLOADABLE cross(double4, double4);\n"
                "double3 _CL_OVERLOADABLE cross(double3, double3);\n"
                "#endif\n" },
    { "read_imagef", "float4 _CL_OVERLOADABLE read_imagef (image2d_t image, sampler_t sampler, int2 coord);\n"
                     "float4 _CL_OVERLOADABLE read_imagef (image2d_t image, sampler_t sampler, float2 coord);\n" },
    { "read_imageui", "uint4 _CL_OVERLOADABLE read_imageui (image2d_t image, sampler_t sampler, int2 coord);\n"
                      "uint4 _CL_OVERLOADABLE read_imageui (image2d_t image, sampler_t sampler, float2 coord);\n" },
    { "read_imagei", "int4 _CL_OVERLOADABLE read_imagei (image2d_t image, sampler_t sampler, int2 coord);\n"
                     "int4 _CL_OVERLOADABLE read_imagei (image2d_t image, sampler_t sampler, float2 coord);\n" },
    { "vload2", "_CL_DECLARE_VLOAD_WIDTH(2)\n" },
    { "vload3", "_CL_DECLARE_VLOAD_WIDTH(3)\n" },
    { "vload4", "_CL_DECLARE_VLOAD_WIDTH(4)\n" },
    { "vload8", "_CL_DECLARE_VLOAD_WIDTH(8)\n" },
    { "vload16", "_CL_DECLARE_VLOAD_WIDTH(16)\n" },
    { "vstore2", "_CL_DECLARE_VSTORE_WIDTH(2)\n" },
    { "vstore3", "_CL_DECLARE_VSTORE_WIDTH(3)\n" },
    { "vstore4", "_CL_DECLARE_VSTORE_WIDTH(4)\n" },
    { "vstore8", "_CL_DECLARE_VSTORE_WIDTH(8)\n" },
    { "vstore16", "_CL_DECLARE_VSTORE_WIDTH(16)\n" }
};
static const int numBuiltinDecls =
    sizeof(builtinDecls) / sizeof(builtinDecls[0]);

WebCLBuiltins::WebCLBuiltins()
    : unsafeVectorBuiltins_()
    , unsupportedBuiltins_()
    , safeBuiltins_()
    , roundingSuffixes_(roundingSuffixes, roundingSuffixes + numRoundingSuffixes)
    , vloadHalfDeclared(false)
{
    initialize(unsafeVectorBuiltins_, unsafeVectorBuiltins, numUnsafeVectorBuiltins);
    initialize(unsupportedBuiltins_, unsupportedBuiltins, numUnsupportedBuiltins);
    initialize(safeBuiltins_, safeBuiltins, numSafeBuiltins);

    for (int i = 0; i < numBuiltinDecls; ++i) {
        builtinDecls_[builtinDecls[i].name].push_back(builtinDecls[i].decl);
    }
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
        unsafeVectorBuiltins_.count(builtin);
}

bool WebCLBuiltins::isUnsupported(const std::string &builtin) const
{
    return unsupportedBuiltins_.count(builtin);
}

namespace
{
    bool hasPrefix(const std::string &str, const std::string &prefix)
    {
        return str.substr(0, prefix.size()) == prefix;
    }

    bool hasSuffix(const std::string &str, const std::string &suffix)
    {
        return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
    }

    std::string firstMatchingSuffix(
        const std::string &str,
        const std::vector<std::string> &suffixes)
    {
        for (std::vector<std::string>::const_iterator i = suffixes.begin(); i != suffixes.end(); ++i) {
            if (hasSuffix(str, *i))
                return *i;
        }
        return std::string();
    }
}

void WebCLBuiltins::emitDeclarations(llvm::raw_ostream &os, const std::string &builtin)
{
    static const std::string convertPrefix = "convert_";
    if (builtin.substr(0, convertPrefix.size()) == convertPrefix) {
        // One of the convert_##DST##SIZE##INTSUFFIX##ROUNDINGSUFFIX overloads
        const std::string suffix = firstMatchingSuffix(builtin, roundingSuffixes_);
        if (!usedConvertSuffixes_.count(suffix)) {
            DEBUG( std::cerr << "declaring for " << builtin << " builtin convert_..." << suffix << '\n'; );
            os << "_CL_DECLARE_CONVERT_TYPE_SRC_DST_SIZE(" << suffix << ")\n";
            usedConvertSuffixes_.insert(suffix);
        }
    } else {
        if (builtinDecls_.count(builtin)) {
            // Just your average run-off-the-mill builtin
            const llvm::SmallVector<const char *, 2> &decls = builtinDecls_.lookup(builtin);
            DEBUG( std::cerr << "declaring builtin " << builtin << '\n'; );
            for (llvm::SmallVector<const char *, 2>::const_iterator i = decls.begin(); i != decls.end(); ++i) {
                os << *i;
            }
        } else if (hasPrefix(builtin, vloadHalfPrefix) || hasPrefix(builtin, vloadaHalfPrefix)) {
            // One of the vload_half functions, declare them all (can't declare just one by name)
            if (!vloadHalfDeclared) {
                DEBUG( std::cerr << "declaring vloada?_half...(...)\n"; );
                os << "_CL_DECLARE_VLOAD_HALF(__global)\n"
                      "_CL_DECLARE_VLOAD_HALF(__local)\n"
                      "_CL_DECLARE_VLOAD_HALF(__constant)\n"
                      "_CL_DECLARE_VLOAD_HALF(__private)\n";
                vloadHalfDeclared = true;
            }
        } else if (hasPrefix(builtin, vstoreHalfPrefix) || hasPrefix(builtin, vstoreaHalfPrefix)) {
            // One of the vstore_half functions, declare all matching the rounding suffix
            const std::string suffix = firstMatchingSuffix(builtin, roundingSuffixes_);
            if (!usedVstoreHalfSuffixes_.count(suffix)) {
                DEBUG( std::cerr << "declaring vstorea?_half_..." << suffix << "(...)\n"; );
                os << "_CL_DECLARE_VSTORE_HALF(__global, " << suffix << ")\n"
                      "_CL_DECLARE_VSTORE_HALF(__local, " << suffix << ")\n"
                      "_CL_DECLARE_VSTORE_HALF(__private, " << suffix << ")\n";
                usedVstoreHalfSuffixes_.insert(suffix);
            }
        } else {
            // No builtin at all; this is reached for all things like user kernel, parameter
            // and variable names and even keywords. No way to differentiate until
            // we are actually parsing the source, not just lexing and preprocessing!
        }
    }
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
