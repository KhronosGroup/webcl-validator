/* pocl/_kernel.h - OpenCL types and runtime library
   functions declarations.

   Copyright (c) 2011 Universidad Rey Juan Carlos
   Copyright (c) 2011-2013 Pekka Jääskeläinen / TUT
   Copyright (c) 2011-2013 Erik Schnetter <eschnetter@perimeterinstitute.ca>
                           Perimeter Institute for Theoretical Physics
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

/* Language feature detection */
#if (__clang_major__ == 3) && (__clang_minor__ >= 3)
#  define _CL_HAS_EVENT_T
#  define _CL_HAS_IMAGE_ACCESS
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef ulong size_t;
typedef long ptrdiff_t;
typedef long intptr_t;
typedef ulong uintptr_t;

#define cles_khr_int64 1

/* Enable double precision. This should really only be done when
   building the run-time library; when building application code, we
   should instead check a macro to see whether the application has
   enabled this. At the moment, always enable this seems fine, since
   all our target devices will support double precision anyway.

   FIX: this is not really true. TCE target is 32-bit scalars
   only. Seems the pragma does not add the macro, so we have the target
   define the macro and the pragma is conditionally enabled.
*/
#if defined( _W2CL_EXTENSION_CL_KHR_FP64) || defined(_W2CL_EXTENSION_ALL)
#  pragma OPENCL EXTENSION cl_khr_fp64: enable
#endif
#ifdef cl_khr_fp16
#  pragma OPENCL EXTENSION cl_khr_fp16: enable
#endif
#ifdef cl_khr_fp64
#  pragma OPENCL EXTENSION cl_khr_fp64: enable
#endif

/* Define some feature macros to help write generic code */
#ifdef cles_khr_int64
#  define __IF_INT64(x) x
#else
#  define __IF_INT64(x)
#endif
#if defined(_W2CL_EXTENSION_CL_KHR_FP16) || defined(_W2CL_EXTENSION_ALL)
#  define __IF_FP16(x) x
#else
#  define __IF_FP16(x)
#endif
#if defined( _W2CL_EXTENSION_CL_KHR_FP64) || defined(_W2CL_EXTENSION_ALL)
#  define __IF_FP64(x) x
#else
#  define __IF_FP64(x)
#endif

#if defined(cl_khr_fp64) && !defined(cles_khr_int64)
#  error "cl_khr_fp64 requires cles_khr_int64"
#endif

/* Function/type attributes supported by Clang/SPIR */
#if __has_attribute(__always_inline__)
#  define _CL_ALWAYSINLINE __attribute__((__always_inline__))
#else
#  define _CL_ALWAYSINLINE
#endif
#if __has_attribute(__noinline__)
#  define _CL_NOINLINE __attribute__((__noinline__))
#else
#  define _CL_NOINLINE
#endif
#if __has_attribute(__overloadable__)
#  define _CL_OVERLOADABLE __attribute__((__overloadable__))
#else
#  define _CL_OVERLOADABLE
#endif
#if (__clang_major__ == 3) && (__clang_minor__ >= 2)
/* This causes an error with Clang 3.1: */
/* #if __has_attribute(__const__) */
#  define _CL_READNONE __attribute__((__const__))
#else
#  define _CL_READNONE
#endif
#if __has_attribute(__pure__)
#  define _CL_READONLY __attribute__((__pure__))
#else
#  define _CL_READONLY
#endif
#if __has_attribute(__unavailable__)
#  define _CL_UNAVAILABLE __attribute__((__unavailable__))
#else
#  define _CL_UNAVAILABLE
#endif

/* A static assert statement to catch inconsistencies at build time */
#if __has_extension(__c_static_assert__)
#  define _CL_STATIC_ASSERT(_t, _x) _Static_assert(_x, #_t)
#else
#  define _CL_STATIC_ASSERT(_t, _x) typedef int __cl_ai##_t[(x) ? 1 : -1];
#endif

typedef enum {
  CLK_LOCAL_MEM_FENCE = 0x1,
  CLK_GLOBAL_MEM_FENCE = 0x2
} cl_mem_fence_flags;



/* Data types */

/* Disable undefined datatypes */
#ifndef cles_khr_int64
typedef struct error_undefined_type_long error_undefined_type_long;
#  define long error_undefined_type_long
typedef struct error_undefined_type_ulong error_undefined_type_ulong;
#  define ulong error_undefined_type_ulong
#endif
#if !defined(_W2CL_EXTENSION_CL_KHR_FP16) && !defined(_W2CL_EXTENSION_ALL)
typedef struct error_undefined_type_half error_undefined_type_half;
#  define half error_undefined_type_half
#endif
#if !defined(_W2CL_EXTENSION_CL_KHR_FP64) && !defined(_W2CL_EXTENSION_ALL)
typedef struct error_undefined_type_double error_undefined_type_double;
#  define double error_undefined_type_double
#endif

// We align the 3-vectors, so that their sizeof is correct. Is there a
// better way? Should we also align the other vectors?

typedef char char2  __attribute__((__ext_vector_type__(2)));
typedef char char3  __attribute__((__ext_vector_type__(3)));
typedef char char4  __attribute__((__ext_vector_type__(4)));
typedef char char8  __attribute__((__ext_vector_type__(8)));
typedef char char16 __attribute__((__ext_vector_type__(16)));

typedef uchar uchar2  __attribute__((__ext_vector_type__(2)));
typedef uchar uchar3  __attribute__((__ext_vector_type__(3)));
typedef uchar uchar4  __attribute__((__ext_vector_type__(4)));
typedef uchar uchar8  __attribute__((__ext_vector_type__(8)));
typedef uchar uchar16 __attribute__((__ext_vector_type__(16)));

typedef short short2  __attribute__((__ext_vector_type__(2)));
typedef short short3  __attribute__((__ext_vector_type__(3)));
typedef short short4  __attribute__((__ext_vector_type__(4)));
typedef short short8  __attribute__((__ext_vector_type__(8)));
typedef short short16 __attribute__((__ext_vector_type__(16)));

typedef ushort ushort2  __attribute__((__ext_vector_type__(2)));
typedef ushort ushort3  __attribute__((__ext_vector_type__(3)));
typedef ushort ushort4  __attribute__((__ext_vector_type__(4)));
typedef ushort ushort8  __attribute__((__ext_vector_type__(8)));
typedef ushort ushort16 __attribute__((__ext_vector_type__(16)));

typedef int int2  __attribute__((__ext_vector_type__(2)));
typedef int int3  __attribute__((__ext_vector_type__(3)));
typedef int int4  __attribute__((__ext_vector_type__(4)));
typedef int int8  __attribute__((__ext_vector_type__(8)));
typedef int int16 __attribute__((__ext_vector_type__(16)));

typedef uint uint2  __attribute__((__ext_vector_type__(2)));
typedef uint uint3  __attribute__((__ext_vector_type__(3)));
typedef uint uint4  __attribute__((__ext_vector_type__(4)));
typedef uint uint8  __attribute__((__ext_vector_type__(8)));
typedef uint uint16 __attribute__((__ext_vector_type__(16)));

#ifdef cles_khr_int64
typedef long long2  __attribute__((__ext_vector_type__(2)));
typedef long long3  __attribute__((__ext_vector_type__(3)));
typedef long long4  __attribute__((__ext_vector_type__(4)));
typedef long long8  __attribute__((__ext_vector_type__(8)));
typedef long long16 __attribute__((__ext_vector_type__(16)));

typedef ulong ulong2  __attribute__((__ext_vector_type__(2)));
typedef ulong ulong3  __attribute__((__ext_vector_type__(3)));
typedef ulong ulong4  __attribute__((__ext_vector_type__(4)));
typedef ulong ulong8  __attribute__((__ext_vector_type__(8)));
typedef ulong ulong16 __attribute__((__ext_vector_type__(16)));
#endif

#if defined(_W2CL_EXTENSION_CL_KHR_FP16) || defined(_W2CL_EXTENSION_ALL)
typedef half half2  __attribute__((__ext_vector_type__(2)));
typedef half half3  __attribute__((__ext_vector_type__(3)));
typedef half half4  __attribute__((__ext_vector_type__(4)));
typedef half half8  __attribute__((__ext_vector_type__(8)));
typedef half half16 __attribute__((__ext_vector_type__(16)));
#endif

typedef float float2  __attribute__((__ext_vector_type__(2)));
typedef float float3  __attribute__((__ext_vector_type__(3)));
typedef float float4  __attribute__((__ext_vector_type__(4)));
typedef float float8  __attribute__((__ext_vector_type__(8)));
typedef float float16 __attribute__((__ext_vector_type__(16)));

#if defined( _W2CL_EXTENSION_CL_KHR_FP64) || defined(_W2CL_EXTENSION_ALL)
typedef double double2  __attribute__((__ext_vector_type__(2)));
typedef double double3  __attribute__((__ext_vector_type__(3)));
typedef double double4  __attribute__((__ext_vector_type__(4)));
typedef double double8  __attribute__((__ext_vector_type__(8)));
typedef double double16 __attribute__((__ext_vector_type__(16)));
#endif

/* Ensure the data types have the right sizes */
_CL_STATIC_ASSERT(char  , sizeof(char  ) == 1);
_CL_STATIC_ASSERT(char2 , sizeof(char2 ) == 2 *sizeof(char));
_CL_STATIC_ASSERT(char3 , sizeof(char3 ) == 4 *sizeof(char));
_CL_STATIC_ASSERT(char4 , sizeof(char4 ) == 4 *sizeof(char));
_CL_STATIC_ASSERT(char8 , sizeof(char8 ) == 8 *sizeof(char));
_CL_STATIC_ASSERT(char16, sizeof(char16) == 16*sizeof(char));

_CL_STATIC_ASSERT(uchar , sizeof(uchar ) == 1);
_CL_STATIC_ASSERT(uchar2 , sizeof(uchar2 ) == 2 *sizeof(uchar));
_CL_STATIC_ASSERT(uchar3 , sizeof(uchar3 ) == 4 *sizeof(uchar));
_CL_STATIC_ASSERT(uchar4 , sizeof(uchar4 ) == 4 *sizeof(uchar));
_CL_STATIC_ASSERT(uchar8 , sizeof(uchar8 ) == 8 *sizeof(uchar));
_CL_STATIC_ASSERT(uchar16, sizeof(uchar16) == 16*sizeof(uchar));

_CL_STATIC_ASSERT(short , sizeof(short ) == 2);
_CL_STATIC_ASSERT(short2 , sizeof(short2 ) == 2 *sizeof(short));
_CL_STATIC_ASSERT(short3 , sizeof(short3 ) == 4 *sizeof(short));
_CL_STATIC_ASSERT(short4 , sizeof(short4 ) == 4 *sizeof(short));
_CL_STATIC_ASSERT(short8 , sizeof(short8 ) == 8 *sizeof(short));
_CL_STATIC_ASSERT(short16, sizeof(short16) == 16*sizeof(short));

_CL_STATIC_ASSERT(ushort, sizeof(ushort) == 2);
_CL_STATIC_ASSERT(ushort2 , sizeof(ushort2 ) == 2 *sizeof(ushort));
_CL_STATIC_ASSERT(ushort3 , sizeof(ushort3 ) == 4 *sizeof(ushort));
_CL_STATIC_ASSERT(ushort4 , sizeof(ushort4 ) == 4 *sizeof(ushort));
_CL_STATIC_ASSERT(ushort8 , sizeof(ushort8 ) == 8 *sizeof(ushort));
_CL_STATIC_ASSERT(ushort16, sizeof(ushort16) == 16*sizeof(ushort));

_CL_STATIC_ASSERT(int   , sizeof(int   ) == 4);
_CL_STATIC_ASSERT(int2 , sizeof(int2 ) == 2 *sizeof(int));
_CL_STATIC_ASSERT(int3 , sizeof(int3 ) == 4 *sizeof(int));
_CL_STATIC_ASSERT(int4 , sizeof(int4 ) == 4 *sizeof(int));
_CL_STATIC_ASSERT(int8 , sizeof(int8 ) == 8 *sizeof(int));
_CL_STATIC_ASSERT(int16, sizeof(int16) == 16*sizeof(int));

_CL_STATIC_ASSERT(uint  , sizeof(uint  ) == 4);
_CL_STATIC_ASSERT(uint2 , sizeof(uint2 ) == 2 *sizeof(uint));
_CL_STATIC_ASSERT(uint3 , sizeof(uint3 ) == 4 *sizeof(uint));
_CL_STATIC_ASSERT(uint4 , sizeof(uint4 ) == 4 *sizeof(uint));
_CL_STATIC_ASSERT(uint8 , sizeof(uint8 ) == 8 *sizeof(uint));
_CL_STATIC_ASSERT(uint16, sizeof(uint16) == 16*sizeof(uint));

#ifdef cles_khr_int64 
_CL_STATIC_ASSERT(long  , sizeof(long  ) == 8);
_CL_STATIC_ASSERT(long2 , sizeof(long2 ) == 2 *sizeof(long));
_CL_STATIC_ASSERT(long3 , sizeof(long3 ) == 4 *sizeof(long));
_CL_STATIC_ASSERT(long4 , sizeof(long4 ) == 4 *sizeof(long));
_CL_STATIC_ASSERT(long8 , sizeof(long8 ) == 8 *sizeof(long));
_CL_STATIC_ASSERT(long16, sizeof(long16) == 16*sizeof(long));

_CL_STATIC_ASSERT(ulong  , sizeof(ulong  ) == 8);
_CL_STATIC_ASSERT(ulong2 , sizeof(ulong2 ) == 2 *sizeof(ulong));
_CL_STATIC_ASSERT(ulong3 , sizeof(ulong3 ) == 4 *sizeof(ulong));
_CL_STATIC_ASSERT(ulong4 , sizeof(ulong4 ) == 4 *sizeof(ulong));
_CL_STATIC_ASSERT(ulong8 , sizeof(ulong8 ) == 8 *sizeof(ulong));
_CL_STATIC_ASSERT(ulong16, sizeof(ulong16) == 16*sizeof(ulong));
#endif

#if defined(_W2CL_EXTENSION_CL_KHR_FP16) || defined(_W2CL_EXTENSION_ALL)
_CL_STATIC_ASSERT(half, sizeof(half) == 2);
_CL_STATIC_ASSERT(half2 , sizeof(half2 ) == 2 *sizeof(half));
_CL_STATIC_ASSERT(half3 , sizeof(half3 ) == 4 *sizeof(half));
_CL_STATIC_ASSERT(half4 , sizeof(half4 ) == 4 *sizeof(half));
_CL_STATIC_ASSERT(half8 , sizeof(half8 ) == 8 *sizeof(half));
_CL_STATIC_ASSERT(half16, sizeof(half16) == 16*sizeof(half));
#endif

_CL_STATIC_ASSERT(float , sizeof(float ) == 4);
_CL_STATIC_ASSERT(float2 , sizeof(float2 ) == 2 *sizeof(float));
_CL_STATIC_ASSERT(float3 , sizeof(float3 ) == 4 *sizeof(float));
_CL_STATIC_ASSERT(float4 , sizeof(float4 ) == 4 *sizeof(float));
_CL_STATIC_ASSERT(float8 , sizeof(float8 ) == 8 *sizeof(float));
_CL_STATIC_ASSERT(float16, sizeof(float16) == 16*sizeof(float));

#if defined( _W2CL_EXTENSION_CL_KHR_FP64) || defined(_W2CL_EXTENSION_ALL)
_CL_STATIC_ASSERT(double, sizeof(double) == 8);
_CL_STATIC_ASSERT(double2 , sizeof(double2 ) == 2 *sizeof(double));
_CL_STATIC_ASSERT(double3 , sizeof(double3 ) == 4 *sizeof(double));
_CL_STATIC_ASSERT(double4 , sizeof(double4 ) == 4 *sizeof(double));
_CL_STATIC_ASSERT(double8 , sizeof(double8 ) == 8 *sizeof(double));
_CL_STATIC_ASSERT(double16, sizeof(double16) == 16*sizeof(double));
#endif

// pointer size can be smaller than these, it is enough that they fit in
//_CL_STATIC_ASSERT(size_t, sizeof(size_t) == sizeof(void*));
//_CL_STATIC_ASSERT(ptrdiff_t, sizeof(ptrdiff_t) == sizeof(void*));
//_CL_STATIC_ASSERT(intptr_t, sizeof(intptr_t) == sizeof(void*));
//_CL_STATIC_ASSERT(uintptr_t, sizeof(uintptr_t) == sizeof(void*));



/* Conversion functions */

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// TODO: Only declare these if used
// This could be an 25% performance improvement!
//
// (Just #if 0'ing them all makes test suite complete
//  in 33s instead of 45s but is obviously incorrect)
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define _CL_DECLARE_AS_TYPE(SRC, DST)           \
  DST _CL_OVERLOADABLE as_##DST(SRC a);

/* 1 byte */
#define _CL_DECLARE_AS_TYPE_1(SRC)              \
  _CL_DECLARE_AS_TYPE(SRC, char)                \
  _CL_DECLARE_AS_TYPE(SRC, uchar)
_CL_DECLARE_AS_TYPE_1(char)
_CL_DECLARE_AS_TYPE_1(uchar)

/* 2 bytes */
#define _CL_DECLARE_AS_TYPE_2(SRC)              \
  _CL_DECLARE_AS_TYPE(SRC, char2)               \
  _CL_DECLARE_AS_TYPE(SRC, uchar2)              \
  _CL_DECLARE_AS_TYPE(SRC, short)               \
  _CL_DECLARE_AS_TYPE(SRC, ushort)
_CL_DECLARE_AS_TYPE_2(char2)
_CL_DECLARE_AS_TYPE_2(uchar2)
_CL_DECLARE_AS_TYPE_2(short)
_CL_DECLARE_AS_TYPE_2(ushort)

/* 4 bytes */
#define _CL_DECLARE_AS_TYPE_4(SRC)              \
  _CL_DECLARE_AS_TYPE(SRC, char4)               \
  _CL_DECLARE_AS_TYPE(SRC, uchar4)              \
  _CL_DECLARE_AS_TYPE(SRC, char3)               \
  _CL_DECLARE_AS_TYPE(SRC, uchar3)              \
  _CL_DECLARE_AS_TYPE(SRC, short2)              \
  _CL_DECLARE_AS_TYPE(SRC, ushort2)             \
  _CL_DECLARE_AS_TYPE(SRC, int)                 \
  _CL_DECLARE_AS_TYPE(SRC, uint)                \
  _CL_DECLARE_AS_TYPE(SRC, float)
_CL_DECLARE_AS_TYPE_4(char4)
_CL_DECLARE_AS_TYPE_4(uchar4)
_CL_DECLARE_AS_TYPE_4(char3)
_CL_DECLARE_AS_TYPE_4(uchar3)
_CL_DECLARE_AS_TYPE_4(short2)
_CL_DECLARE_AS_TYPE_4(ushort2)
_CL_DECLARE_AS_TYPE_4(int)
_CL_DECLARE_AS_TYPE_4(uint)
_CL_DECLARE_AS_TYPE_4(float)

/* 8 bytes */
#define _CL_DECLARE_AS_TYPE_8(SRC)              \
  _CL_DECLARE_AS_TYPE(SRC, char8)               \
  _CL_DECLARE_AS_TYPE(SRC, uchar8)              \
  _CL_DECLARE_AS_TYPE(SRC, short4)              \
  _CL_DECLARE_AS_TYPE(SRC, ushort4)             \
  _CL_DECLARE_AS_TYPE(SRC, short3)              \
  _CL_DECLARE_AS_TYPE(SRC, ushort3)             \
  _CL_DECLARE_AS_TYPE(SRC, int2)                \
  _CL_DECLARE_AS_TYPE(SRC, uint2)               \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long))    \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong))   \
  _CL_DECLARE_AS_TYPE(SRC, float2)              \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double))
_CL_DECLARE_AS_TYPE_8(char8)
_CL_DECLARE_AS_TYPE_8(uchar8)
_CL_DECLARE_AS_TYPE_8(short4)
_CL_DECLARE_AS_TYPE_8(ushort4)
_CL_DECLARE_AS_TYPE_8(short3)
_CL_DECLARE_AS_TYPE_8(ushort3)
_CL_DECLARE_AS_TYPE_8(int2)
_CL_DECLARE_AS_TYPE_8(uint2)
__IF_INT64(_CL_DECLARE_AS_TYPE_8(long))
__IF_INT64(_CL_DECLARE_AS_TYPE_8(ulong))
_CL_DECLARE_AS_TYPE_8(float2)
__IF_FP64(_CL_DECLARE_AS_TYPE_8(double))

/* 16 bytes */
#define _CL_DECLARE_AS_TYPE_16(SRC)             \
  _CL_DECLARE_AS_TYPE(SRC, char16)              \
  _CL_DECLARE_AS_TYPE(SRC, uchar16)             \
  _CL_DECLARE_AS_TYPE(SRC, short8)              \
  _CL_DECLARE_AS_TYPE(SRC, ushort8)             \
  _CL_DECLARE_AS_TYPE(SRC, int4)                \
  _CL_DECLARE_AS_TYPE(SRC, uint4)               \
  _CL_DECLARE_AS_TYPE(SRC, int3)                \
  _CL_DECLARE_AS_TYPE(SRC, uint3)               \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long2))   \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong2))  \
  _CL_DECLARE_AS_TYPE(SRC, float4)              \
  _CL_DECLARE_AS_TYPE(SRC, float3)              \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double2))
_CL_DECLARE_AS_TYPE_16(char16)
_CL_DECLARE_AS_TYPE_16(uchar16)
_CL_DECLARE_AS_TYPE_16(short8)
_CL_DECLARE_AS_TYPE_16(ushort8)
_CL_DECLARE_AS_TYPE_16(int4)
_CL_DECLARE_AS_TYPE_16(uint4)
_CL_DECLARE_AS_TYPE_16(int3)
_CL_DECLARE_AS_TYPE_16(uint3)
__IF_INT64(_CL_DECLARE_AS_TYPE_16(long2))
__IF_INT64(_CL_DECLARE_AS_TYPE_16(ulong2))
_CL_DECLARE_AS_TYPE_16(float4)
_CL_DECLARE_AS_TYPE_16(float3)
__IF_FP64(_CL_DECLARE_AS_TYPE_16(double2))

/* 32 bytes */
#define _CL_DECLARE_AS_TYPE_32(SRC)             \
  _CL_DECLARE_AS_TYPE(SRC, short16)             \
  _CL_DECLARE_AS_TYPE(SRC, ushort16)            \
  _CL_DECLARE_AS_TYPE(SRC, int8)                \
  _CL_DECLARE_AS_TYPE(SRC, uint8)               \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long4))   \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong4))  \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long3))   \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong3))  \
  _CL_DECLARE_AS_TYPE(SRC, float8)              \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double4))  \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double3))
_CL_DECLARE_AS_TYPE_32(short16)
_CL_DECLARE_AS_TYPE_32(ushort16)
_CL_DECLARE_AS_TYPE_32(int8)
_CL_DECLARE_AS_TYPE_32(uint8)
__IF_INT64(_CL_DECLARE_AS_TYPE_32(long4))
__IF_INT64(_CL_DECLARE_AS_TYPE_32(ulong4))
__IF_INT64(_CL_DECLARE_AS_TYPE_32(long3))
__IF_INT64(_CL_DECLARE_AS_TYPE_32(ulong3))
_CL_DECLARE_AS_TYPE_32(float8)
__IF_FP64(_CL_DECLARE_AS_TYPE_32(double4))
__IF_FP64(_CL_DECLARE_AS_TYPE_32(double3))

/* 64 bytes */
#define _CL_DECLARE_AS_TYPE_64(SRC)             \
  _CL_DECLARE_AS_TYPE(SRC, int16)               \
  _CL_DECLARE_AS_TYPE(SRC, uint16)              \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long8))   \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong8))  \
  _CL_DECLARE_AS_TYPE(SRC, float16)             \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double8))
_CL_DECLARE_AS_TYPE_64(int16)
_CL_DECLARE_AS_TYPE_64(uint16)
__IF_INT64(_CL_DECLARE_AS_TYPE_64(long8))
__IF_INT64(_CL_DECLARE_AS_TYPE_64(ulong8))
_CL_DECLARE_AS_TYPE_64(float16)
__IF_FP64(_CL_DECLARE_AS_TYPE_64(double8))

/* 128 bytes */
#define _CL_DECLARE_AS_TYPE_128(SRC)            \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, long16))  \
  __IF_INT64(_CL_DECLARE_AS_TYPE(SRC, ulong16)) \
  __IF_FP64(_CL_DECLARE_AS_TYPE(SRC, double16))
__IF_INT64(_CL_DECLARE_AS_TYPE_128(long16))
__IF_INT64(_CL_DECLARE_AS_TYPE_128(ulong16))
__IF_FP64(_CL_DECLARE_AS_TYPE_128(double16))

/* Conversions between builtin types.
 *
 * Even though the OpenCL specification isn't entirely clear on this
 * matter, we implement all rounding mode combinations even for
 * integer-to-integer conversions. The rounding mode is essentially
 * redundant and thus ignored.
 *
 * Other OpenCL implementations seem to allow this in user code, and some
 * of the test suites/benchmarks out in the wild expect these functions
 * are available.
 *
 * Saturating conversions are only allowed when the destination type
 * is an integer.
 */

#define _CL_DECLARE_CONVERT_TYPE(SRC, DST, SIZE, INTSUFFIX, FLOATSUFFIX) \
  DST##SIZE _CL_OVERLOADABLE                                             \
  convert_##DST##SIZE##INTSUFFIX##FLOATSUFFIX(SRC##SIZE a);

#define _CL_DECLARE_CONVERT_TYPE_DST(SRC, SIZE, FLOATSUFFIX)            \
  _CL_DECLARE_CONVERT_TYPE(SRC, char  , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, char  , SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, uchar , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, uchar , SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, short , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, short , SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, ushort, SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, ushort, SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, int   , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, int   , SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, uint  , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, uint  , SIZE, _sat, FLOATSUFFIX)        \
  __IF_INT64(                                                           \
  _CL_DECLARE_CONVERT_TYPE(SRC, long  , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, long  , SIZE, _sat, FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, ulong , SIZE,     , FLOATSUFFIX)        \
  _CL_DECLARE_CONVERT_TYPE(SRC, ulong , SIZE, _sat, FLOATSUFFIX))       \
  _CL_DECLARE_CONVERT_TYPE(SRC, float , SIZE,     , FLOATSUFFIX)        \
  __IF_FP64(                                                            \
  _CL_DECLARE_CONVERT_TYPE(SRC, double, SIZE,     , FLOATSUFFIX))

#define _CL_DECLARE_CONVERT_TYPE_SRC_DST(SIZE, FLOATSUFFIX) \
  _CL_DECLARE_CONVERT_TYPE_DST(char  , SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(uchar , SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(short , SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(ushort, SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(int   , SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(uint  , SIZE, FLOATSUFFIX)   \
  __IF_INT64(                                               \
  _CL_DECLARE_CONVERT_TYPE_DST(long  , SIZE, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_DST(ulong , SIZE, FLOATSUFFIX))  \
  _CL_DECLARE_CONVERT_TYPE_DST(float , SIZE, FLOATSUFFIX)   \
  __IF_FP64(                                                \
  _CL_DECLARE_CONVERT_TYPE_DST(double, SIZE, FLOATSUFFIX))

#define _CL_DECLARE_CONVERT_TYPE_SRC_DST_SIZE(FLOATSUFFIX) \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST(  , FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST( 2, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST( 3, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST( 4, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST( 8, FLOATSUFFIX)   \
  _CL_DECLARE_CONVERT_TYPE_SRC_DST(16, FLOATSUFFIX)

/* Work-Item Functions */

uint get_work_dim();
size_t get_global_size(uint);
size_t get_global_id(uint);
size_t get_local_size(uint);
size_t get_local_id(uint);
size_t get_num_groups(uint);
size_t get_group_id(uint);
size_t get_global_offset(uint);

void barrier (cl_mem_fence_flags flags);



/* Math Constants */

#define MAXFLOAT  FLT_MAX
#define HUGE_VALF __builtin_huge_valf()
#define INFINITY  (1.0f / 0.0f)
#define NAN       (0.0f / 0.0f)

#define FLT_DIG        6
#define FLT_MANT_DIG   24
#define FLT_MAX_10_EXP +38
#define FLT_MAX_EXP    +128
#define FLT_MIN_10_EXP -37
#define FLT_MIN_EXP    -125
#define FLT_RADIX      2
#define FLT_MAX        0x1.fffffep127f
#define FLT_MIN        0x1.0p-126f
#define FLT_EPSILON    0x1.0p-23f

#define FP_ILOGB0   INT_MIN
#define FP_ILOGBNAN INT_MAX

#define M_E_F        2.71828182845904523536028747135f
#define M_LOG2E_F    1.44269504088896340735992468100f
#define M_LOG10E_F   0.434294481903251827651128918917f
#define M_LN2_F      0.693147180559945309417232121458f
#define M_LN10_F     2.30258509299404568401799145468f
#define M_PI_F       3.14159265358979323846264338328f
#define M_PI_2_F     1.57079632679489661923132169164f
#define M_PI_4_F     0.785398163397448309615660845820f
#define M_1_PI_F     0.318309886183790671537767526745f
#define M_2_PI_F     0.636619772367581343075535053490f
#define M_2_SQRTPI_F 1.12837916709551257389615890312f
#define M_SQRT2_F    1.41421356237309504880168872421f
#define M_SQRT1_2_F  0.707106781186547524400844362105f

#ifdef cl_khr_fp64
#define HUGE_VAL __builtin_huge_val()

#define DBL_DIG        15
#define DBL_MANT_DIG   53
#define DBL_MAX_10_EXP +308
#define DBL_MAX_EXP    +1024
#define DBL_MIN_10_EXP -307
#define DBL_MIN_EXP    -1021
#define DBL_MAX        0x1.fffffffffffffp1023
#define DBL_MIN        0x1.0p-1022
#define DBL_EPSILON    0x1.0p-52

#define M_E        2.71828182845904523536028747135
#define M_LOG2E    1.44269504088896340735992468100
#define M_LOG10E   0.434294481903251827651128918917
#define M_LN2      0.693147180559945309417232121458
#define M_LN10     2.30258509299404568401799145468
#define M_PI       3.14159265358979323846264338328
#define M_PI_2     1.57079632679489661923132169164
#define M_PI_4     0.785398163397448309615660845820
#define M_1_PI     0.318309886183790671537767526745
#define M_2_PI     0.636619772367581343075535053490
#define M_2_SQRTPI 1.12837916709551257389615890312
#define M_SQRT2    1.41421356237309504880168872421
#define M_SQRT1_2  0.707106781186547524400844362105
#endif

/* Math Functions */

/* Naming scheme:
 *    [NAME]_[R]_[A]*
 * where [R] is the return type, and [A] are the argument types:
 *    I: int
 *    J: vector of int
 *    U: vector of uint or ulong
 *    S: scalar (float or double)
 *    F: vector of float
 *    V: vector of float or double
 */

#define _CL_DECLARE_FUNC_V_V(NAME)              \
  float    _CL_OVERLOADABLE NAME(float   );     \
  float2   _CL_OVERLOADABLE NAME(float2  );     \
  float3   _CL_OVERLOADABLE NAME(float3  );     \
  float4   _CL_OVERLOADABLE NAME(float4  );     \
  float8   _CL_OVERLOADABLE NAME(float8  );     \
  float16  _CL_OVERLOADABLE NAME(float16 );     \
  __IF_FP64(                                    \
  double   _CL_OVERLOADABLE NAME(double  );     \
  double2  _CL_OVERLOADABLE NAME(double2 );     \
  double3  _CL_OVERLOADABLE NAME(double3 );     \
  double4  _CL_OVERLOADABLE NAME(double4 );     \
  double8  _CL_OVERLOADABLE NAME(double8 );     \
  double16 _CL_OVERLOADABLE NAME(double16);)
#define _CL_DECLARE_FUNC_V_VV(NAME)                     \
  float    _CL_OVERLOADABLE NAME(float   , float   );   \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  );   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  );   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  );   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  );   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 );   \
  __IF_FP64(                                            \
  double   _CL_OVERLOADABLE NAME(double  , double  );   \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 );   \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 );   \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 );   \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 );   \
  double16 _CL_OVERLOADABLE NAME(double16, double16);)
#define _CL_DECLARE_FUNC_V_VVV(NAME)                                    \
  float    _CL_OVERLOADABLE NAME(float   , float   , float   );         \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , float2  );         \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , float3  );         \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , float4  );         \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , float8  );         \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , float16 );         \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , double  , double  );         \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , double2 );         \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , double3 );         \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , double4 );         \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , double8 );         \
  double16 _CL_OVERLOADABLE NAME(double16, double16, double16);)
#define _CL_DECLARE_FUNC_V_VVPVI(NAME)                                  \
  float    _CL_OVERLOADABLE NAME(float   , float   , __local int *);    \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , __local int2 *);   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , __local int3 *);   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , __local int4 *);   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , __local int8 *);   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , __local int16 *);  \
  __IF_FP64(                                                             \
  double   _CL_OVERLOADABLE NAME(double  , double  , __local int *);    \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , __local int2 *);   \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , __local int3 *);   \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , __local int4 *);   \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , __local int8 *);   \
  double16 _CL_OVERLOADABLE NAME(double16, double16, __local int16 *);) \
  float    _CL_OVERLOADABLE NAME(float   , float   , __private int *);    \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , __private int2 *);   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , __private int3 *);   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , __private int4 *);   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , __private int8 *);   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , __private int16 *);  \
  __IF_FP64(                                                             \
  double   _CL_OVERLOADABLE NAME(double  , double  , __private int *);    \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , __private int2 *);   \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , __private int3 *);   \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , __private int4 *);   \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , __private int8 *);   \
  double16 _CL_OVERLOADABLE NAME(double16, double16, __private int16 *);) \
  float    _CL_OVERLOADABLE NAME(float   , float   , __global int *);    \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , __global int2 *);   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , __global int3 *);   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , __global int4 *);   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , __global int8 *);   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , __global int16 *);  \
  __IF_FP64(                                                             \
  double   _CL_OVERLOADABLE NAME(double  , double  , __global int *);    \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , __global int2 *);   \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , __global int3 *);   \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , __global int4 *);   \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , __global int8 *);   \
  double16 _CL_OVERLOADABLE NAME(double16, double16, __global int16 *);)
#define _CL_DECLARE_FUNC_V_VVS(NAME)                            \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , float );   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , float );   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , float );   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , float );   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , float );   \
  __IF_FP64(                                                    \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , double);   \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , double);   \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , double);   \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , double);   \
  double16 _CL_OVERLOADABLE NAME(double16, double16, double);)
#define _CL_DECLARE_FUNC_V_VSS(NAME)                            \
  float2   _CL_OVERLOADABLE NAME(float2  , float , float );     \
  float3   _CL_OVERLOADABLE NAME(float3  , float , float );     \
  float4   _CL_OVERLOADABLE NAME(float4  , float , float );     \
  float8   _CL_OVERLOADABLE NAME(float8  , float , float );     \
  float16  _CL_OVERLOADABLE NAME(float16 , float , float );     \
  __IF_FP64(                                                    \
  double2  _CL_OVERLOADABLE NAME(double2 , double, double);     \
  double3  _CL_OVERLOADABLE NAME(double3 , double, double);     \
  double4  _CL_OVERLOADABLE NAME(double4 , double, double);     \
  double8  _CL_OVERLOADABLE NAME(double8 , double, double);     \
  double16 _CL_OVERLOADABLE NAME(double16, double, double);)
#define _CL_DECLARE_FUNC_V_SSV(NAME)                            \
  float2   _CL_OVERLOADABLE NAME(float , float , float2  );     \
  float3   _CL_OVERLOADABLE NAME(float , float , float3  );     \
  float4   _CL_OVERLOADABLE NAME(float , float , float4  );     \
  float8   _CL_OVERLOADABLE NAME(float , float , float8  );     \
  float16  _CL_OVERLOADABLE NAME(float , float , float16 );     \
  __IF_FP64(                                                    \
  double2  _CL_OVERLOADABLE NAME(double, double, double2 );     \
  double3  _CL_OVERLOADABLE NAME(double, double, double3 );     \
  double4  _CL_OVERLOADABLE NAME(double, double, double4 );     \
  double8  _CL_OVERLOADABLE NAME(double, double, double8 );     \
  double16 _CL_OVERLOADABLE NAME(double, double, double16);)
#define _CL_DECLARE_FUNC_V_VVJ(NAME)                                    \
  __IF_FP16(                                                            \
  /*half     _CL_OVERLOADABLE NAME(half    , half    , short  );*/      \
  half2    _CL_OVERLOADABLE NAME(half2   , half2   , short2 );          \
  half3    _CL_OVERLOADABLE NAME(half3   , half3   , short3 );          \
  half4    _CL_OVERLOADABLE NAME(half4   , half4   , short4 );          \
  half8    _CL_OVERLOADABLE NAME(half8   , half8   , short8 );          \
  half16   _CL_OVERLOADABLE NAME(half16  , half16  , short16);)         \
  float    _CL_OVERLOADABLE NAME(float   , float   , int    );          \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , int2   );          \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , int3   );          \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , int4   );          \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , int8   );          \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , int16  );          \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , double  , long   );          \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , long2  );          \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , long3  );          \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , long4  );          \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , long8  );          \
  double16 _CL_OVERLOADABLE NAME(double16, double16, long16 );)
#define _CL_DECLARE_FUNC_V_VVU(NAME)                                    \
  __IF_FP16(                                                            \
  /*half     _CL_OVERLOADABLE NAME(half    , half    , ushort  );*/     \
  half2    _CL_OVERLOADABLE NAME(half2   , half2   , ushort2 );         \
  half3    _CL_OVERLOADABLE NAME(half3   , half3   , ushort3 );         \
  half4    _CL_OVERLOADABLE NAME(half4   , half4   , ushort4 );         \
  half8    _CL_OVERLOADABLE NAME(half8   , half8   , ushort8 );         \
  half16   _CL_OVERLOADABLE NAME(half16  , half16  , ushort16);)        \
  float    _CL_OVERLOADABLE NAME(float   , float   , uint    );         \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  , uint2   );         \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  , uint3   );         \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  , uint4   );         \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  , uint8   );         \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 , uint16  );         \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , double  , ulong   );         \
  double2  _CL_OVERLOADABLE NAME(double2 , double2 , ulong2  );         \
  double3  _CL_OVERLOADABLE NAME(double3 , double3 , ulong3  );         \
  double4  _CL_OVERLOADABLE NAME(double4 , double4 , ulong4  );         \
  double8  _CL_OVERLOADABLE NAME(double8 , double8 , ulong8  );         \
  double16 _CL_OVERLOADABLE NAME(double16, double16, ulong16 );)
#define _CL_DECLARE_FUNC_V_U(NAME)              \
  float    _CL_OVERLOADABLE NAME(uint   );      \
  float2   _CL_OVERLOADABLE NAME(uint2  );      \
  float3   _CL_OVERLOADABLE NAME(uint3  );      \
  float4   _CL_OVERLOADABLE NAME(uint4  );      \
  float8   _CL_OVERLOADABLE NAME(uint8  );      \
  float16  _CL_OVERLOADABLE NAME(uint16 );      \
  __IF_FP64(                                    \
  double   _CL_OVERLOADABLE NAME(ulong  );      \
  double2  _CL_OVERLOADABLE NAME(ulong2 );      \
  double3  _CL_OVERLOADABLE NAME(ulong3 );      \
  double4  _CL_OVERLOADABLE NAME(ulong4 );      \
  double8  _CL_OVERLOADABLE NAME(ulong8 );      \
  double16 _CL_OVERLOADABLE NAME(ulong16);)
#define _CL_DECLARE_FUNC_V_VS(NAME)                     \
  float2   _CL_OVERLOADABLE NAME(float2  , float );     \
  float3   _CL_OVERLOADABLE NAME(float3  , float );     \
  float4   _CL_OVERLOADABLE NAME(float4  , float );     \
  float8   _CL_OVERLOADABLE NAME(float8  , float );     \
  float16  _CL_OVERLOADABLE NAME(float16 , float );     \
  __IF_FP64(                                            \
  double2  _CL_OVERLOADABLE NAME(double2 , double);     \
  double3  _CL_OVERLOADABLE NAME(double3 , double);     \
  double4  _CL_OVERLOADABLE NAME(double4 , double);     \
  double8  _CL_OVERLOADABLE NAME(double8 , double);     \
  double16 _CL_OVERLOADABLE NAME(double16, double);)
#define _CL_DECLARE_FUNC_V_VJ(NAME)                     \
  float    _CL_OVERLOADABLE NAME(float   , int  );      \
  float2   _CL_OVERLOADABLE NAME(float2  , int2 );      \
  float3   _CL_OVERLOADABLE NAME(float3  , int3 );      \
  float4   _CL_OVERLOADABLE NAME(float4  , int4 );      \
  float8   _CL_OVERLOADABLE NAME(float8  , int8 );      \
  float16  _CL_OVERLOADABLE NAME(float16 , int16);      \
  __IF_FP64(                                            \
  double   _CL_OVERLOADABLE NAME(double  , int  );      \
  double2  _CL_OVERLOADABLE NAME(double2 , int2 );      \
  double3  _CL_OVERLOADABLE NAME(double3 , int3 );      \
  double4  _CL_OVERLOADABLE NAME(double4 , int4 );      \
  double8  _CL_OVERLOADABLE NAME(double8 , int8 );      \
  double16 _CL_OVERLOADABLE NAME(double16, int16);)
#define _CL_DECLARE_FUNC_J_VV(NAME)                     \
  int    _CL_OVERLOADABLE NAME(float   , float   );     \
  int2   _CL_OVERLOADABLE NAME(float2  , float2  );     \
  int3   _CL_OVERLOADABLE NAME(float3  , float3  );     \
  int4   _CL_OVERLOADABLE NAME(float4  , float4  );     \
  int8   _CL_OVERLOADABLE NAME(float8  , float8  );     \
  int16  _CL_OVERLOADABLE NAME(float16 , float16 );     \
  __IF_FP64(                                            \
  int    _CL_OVERLOADABLE NAME(double  , double  );     \
  long2  _CL_OVERLOADABLE NAME(double2 , double2 );     \
  long3  _CL_OVERLOADABLE NAME(double3 , double3 );     \
  long4  _CL_OVERLOADABLE NAME(double4 , double4 );     \
  long8  _CL_OVERLOADABLE NAME(double8 , double8 );     \
  long16 _CL_OVERLOADABLE NAME(double16, double16);)
#define _CL_DECLARE_FUNC_V_VI(NAME)                     \
  float2   _CL_OVERLOADABLE NAME(float2  , int);        \
  float3   _CL_OVERLOADABLE NAME(float3  , int);        \
  float4   _CL_OVERLOADABLE NAME(float4  , int);        \
  float8   _CL_OVERLOADABLE NAME(float8  , int);        \
  float16  _CL_OVERLOADABLE NAME(float16 , int);        \
  __IF_FP64(                                            \
  double2  _CL_OVERLOADABLE NAME(double2 , int);        \
  double3  _CL_OVERLOADABLE NAME(double3 , int);        \
  double4  _CL_OVERLOADABLE NAME(double4 , int);        \
  double8  _CL_OVERLOADABLE NAME(double8 , int);        \
  double16 _CL_OVERLOADABLE NAME(double16, int);)
#define _CL_DECLARE_FUNC_V_VPV(NAME)                                    \
  float    _CL_OVERLOADABLE NAME(float   , __global  float   *);        \
  float2   _CL_OVERLOADABLE NAME(float2  , __global  float2  *);        \
  float3   _CL_OVERLOADABLE NAME(float3  , __global  float3  *);        \
  float4   _CL_OVERLOADABLE NAME(float4  , __global  float4  *);        \
  float8   _CL_OVERLOADABLE NAME(float8  , __global  float8  *);        \
  float16  _CL_OVERLOADABLE NAME(float16 , __global  float16 *);        \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __global  double  *);        \
  double2  _CL_OVERLOADABLE NAME(double2 , __global  double2 *);        \
  double3  _CL_OVERLOADABLE NAME(double3 , __global  double3 *);        \
  double4  _CL_OVERLOADABLE NAME(double4 , __global  double4 *);        \
  double8  _CL_OVERLOADABLE NAME(double8 , __global  double8 *);        \
  double16 _CL_OVERLOADABLE NAME(double16, __global  double16*);)       \
  float    _CL_OVERLOADABLE NAME(float   , __local   float   *);        \
  float2   _CL_OVERLOADABLE NAME(float2  , __local   float2  *);        \
  float3   _CL_OVERLOADABLE NAME(float3  , __local   float3  *);        \
  float4   _CL_OVERLOADABLE NAME(float4  , __local   float4  *);        \
  float8   _CL_OVERLOADABLE NAME(float8  , __local   float8  *);        \
  float16  _CL_OVERLOADABLE NAME(float16 , __local   float16 *);        \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __local   double  *);        \
  double2  _CL_OVERLOADABLE NAME(double2 , __local   double2 *);        \
  double3  _CL_OVERLOADABLE NAME(double3 , __local   double3 *);        \
  double4  _CL_OVERLOADABLE NAME(double4 , __local   double4 *);        \
  double8  _CL_OVERLOADABLE NAME(double8 , __local   double8 *);        \
  double16 _CL_OVERLOADABLE NAME(double16, __local   double16*);)       \
  float    _CL_OVERLOADABLE NAME(float   , __private float   *);        \
  float2   _CL_OVERLOADABLE NAME(float2  , __private float2  *);        \
  float3   _CL_OVERLOADABLE NAME(float3  , __private float3  *);        \
  float4   _CL_OVERLOADABLE NAME(float4  , __private float4  *);        \
  float8   _CL_OVERLOADABLE NAME(float8  , __private float8  *);        \
  float16  _CL_OVERLOADABLE NAME(float16 , __private float16 *);        \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __private double  *);        \
  double2  _CL_OVERLOADABLE NAME(double2 , __private double2 *);        \
  double3  _CL_OVERLOADABLE NAME(double3 , __private double3 *);        \
  double4  _CL_OVERLOADABLE NAME(double4 , __private double4 *);        \
  double8  _CL_OVERLOADABLE NAME(double8 , __private double8 *);        \
  double16 _CL_OVERLOADABLE NAME(double16, __private double16*);)
#define _CL_DECLARE_FUNC_V_VPVI(NAME)                                   \
  float    _CL_OVERLOADABLE NAME(float   , __global  int   *);          \
  float2   _CL_OVERLOADABLE NAME(float2  , __global  int2  *);          \
  float3   _CL_OVERLOADABLE NAME(float3  , __global  int3  *);          \
  float4   _CL_OVERLOADABLE NAME(float4  , __global  int4  *);          \
  float8   _CL_OVERLOADABLE NAME(float8  , __global  int8  *);          \
  float16  _CL_OVERLOADABLE NAME(float16 , __global  int16 *);          \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __global  int  *);           \
  double2  _CL_OVERLOADABLE NAME(double2 , __global  int2 *);           \
  double3  _CL_OVERLOADABLE NAME(double3 , __global  int3 *);           \
  double4  _CL_OVERLOADABLE NAME(double4 , __global  int4 *);           \
  double8  _CL_OVERLOADABLE NAME(double8 , __global  int8 *);           \
  double16 _CL_OVERLOADABLE NAME(double16, __global  int16*);)          \
  float    _CL_OVERLOADABLE NAME(float   , __local   int *);            \
  float2   _CL_OVERLOADABLE NAME(float2  , __local   int2 *);           \
  float3   _CL_OVERLOADABLE NAME(float3  , __local   int3 *);           \
  float4   _CL_OVERLOADABLE NAME(float4  , __local   int4 *);           \
  float8   _CL_OVERLOADABLE NAME(float8  , __local   int8 *);           \
  float16  _CL_OVERLOADABLE NAME(float16 , __local   int16 *);          \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __local   int *);            \
  double2  _CL_OVERLOADABLE NAME(double2 , __local   int2 *);           \
  double3  _CL_OVERLOADABLE NAME(double3 , __local   int3 *);           \
  double4  _CL_OVERLOADABLE NAME(double4 , __local   int4 *);           \
  double8  _CL_OVERLOADABLE NAME(double8 , __local   int8 *);           \
  double16 _CL_OVERLOADABLE NAME(double16, __local   int16 *);)         \
  float    _CL_OVERLOADABLE NAME(float   , __private int *);            \
  float2   _CL_OVERLOADABLE NAME(float2  , __private int2 *);           \
  float3   _CL_OVERLOADABLE NAME(float3  , __private int3 *);           \
  float4   _CL_OVERLOADABLE NAME(float4  , __private int4 *);           \
  float8   _CL_OVERLOADABLE NAME(float8  , __private int8 *);           \
  float16  _CL_OVERLOADABLE NAME(float16 , __private int16 *);          \
  __IF_FP64(                                                            \
  double   _CL_OVERLOADABLE NAME(double  , __private int *);            \
  double2  _CL_OVERLOADABLE NAME(double2 , __private int2 *);           \
  double3  _CL_OVERLOADABLE NAME(double3 , __private int3 *);           \
  double4  _CL_OVERLOADABLE NAME(double4 , __private int4 *);           \
  double8  _CL_OVERLOADABLE NAME(double8 , __private int8 *);           \
  double16 _CL_OVERLOADABLE NAME(double16, __private int16 *);)
#define _CL_DECLARE_FUNC_H_HPVI(NAME)                               \
  __IF_FP16(                                                        \
  half    _CL_OVERLOADABLE NAME(half   , __global  int   *);        \
  half2   _CL_OVERLOADABLE NAME(half2  , __global  int2  *);        \
  half3   _CL_OVERLOADABLE NAME(half3  , __global  int3  *);        \
  half4   _CL_OVERLOADABLE NAME(half4  , __global  int4  *);        \
  half8   _CL_OVERLOADABLE NAME(half8  , __global  int8  *);        \
  half16  _CL_OVERLOADABLE NAME(half16 , __global  int16 *);        \
  half    _CL_OVERLOADABLE NAME(half   , __local   int   *);        \
  half2   _CL_OVERLOADABLE NAME(half2  , __local   int2  *);        \
  half3   _CL_OVERLOADABLE NAME(half3  , __local   int3  *);        \
  half4   _CL_OVERLOADABLE NAME(half4  , __local   int4  *);        \
  half8   _CL_OVERLOADABLE NAME(half8  , __local   int8  *);        \
  half16  _CL_OVERLOADABLE NAME(half16 , __local   int16 *);        \
  half    _CL_OVERLOADABLE NAME(half   , __private int   *);        \
  half2   _CL_OVERLOADABLE NAME(half2  , __private int2  *);        \
  half3   _CL_OVERLOADABLE NAME(half3  , __private int3  *);        \
  half4   _CL_OVERLOADABLE NAME(half4  , __private int4  *);        \
  half8   _CL_OVERLOADABLE NAME(half8  , __private int8  *);        \
  half16  _CL_OVERLOADABLE NAME(half16 , __private int16 *);)
#define _CL_DECLARE_FUNC_V_SV(NAME)                     \
  float2   _CL_OVERLOADABLE NAME(float , float2  );     \
  float3   _CL_OVERLOADABLE NAME(float , float3  );     \
  float4   _CL_OVERLOADABLE NAME(float , float4  );     \
  float8   _CL_OVERLOADABLE NAME(float , float8  );     \
  float16  _CL_OVERLOADABLE NAME(float , float16 );     \
  __IF_FP64(                                            \
  double2  _CL_OVERLOADABLE NAME(double, double2 );     \
  double3  _CL_OVERLOADABLE NAME(double, double3 );     \
  double4  _CL_OVERLOADABLE NAME(double, double4 );     \
  double8  _CL_OVERLOADABLE NAME(double, double8 );     \
  double16 _CL_OVERLOADABLE NAME(double, double16);)
#define _CL_DECLARE_FUNC_J_V(NAME)              \
  int   _CL_OVERLOADABLE NAME(float   );        \
  int2  _CL_OVERLOADABLE NAME(float2  );        \
  int3  _CL_OVERLOADABLE NAME(float3  );        \
  int4  _CL_OVERLOADABLE NAME(float4  );        \
  int8  _CL_OVERLOADABLE NAME(float8  );        \
  int16 _CL_OVERLOADABLE NAME(float16 );        \
  __IF_FP64(                                    \
  int    _CL_OVERLOADABLE NAME(double  );       \
  long2  _CL_OVERLOADABLE NAME(double2 );       \
  long3  _CL_OVERLOADABLE NAME(double3 );       \
  long4  _CL_OVERLOADABLE NAME(double4 );       \
  long8  _CL_OVERLOADABLE NAME(double8 );       \
  long16 _CL_OVERLOADABLE NAME(double16);)
#define _CL_DECLARE_FUNC_K_V(NAME)              \
  int   _CL_OVERLOADABLE NAME(float   );        \
  int2  _CL_OVERLOADABLE NAME(float2  );        \
  int3  _CL_OVERLOADABLE NAME(float3  );        \
  int4  _CL_OVERLOADABLE NAME(float4  );        \
  int8  _CL_OVERLOADABLE NAME(float8  );        \
  int16 _CL_OVERLOADABLE NAME(float16 );        \
  __IF_FP64(                                    \
  int   _CL_OVERLOADABLE NAME(double  );        \
  int2  _CL_OVERLOADABLE NAME(double2 );        \
  int3  _CL_OVERLOADABLE NAME(double3 );        \
  int4  _CL_OVERLOADABLE NAME(double4 );        \
  int8  _CL_OVERLOADABLE NAME(double8 );        \
  int16 _CL_OVERLOADABLE NAME(double16);)
#define _CL_DECLARE_FUNC_S_V(NAME)              \
  float  _CL_OVERLOADABLE NAME(float   );       \
  float  _CL_OVERLOADABLE NAME(float2  );       \
  float  _CL_OVERLOADABLE NAME(float3  );       \
  float  _CL_OVERLOADABLE NAME(float4  );       \
  float  _CL_OVERLOADABLE NAME(float8  );       \
  float  _CL_OVERLOADABLE NAME(float16 );       \
  __IF_FP64(                                    \
  double _CL_OVERLOADABLE NAME(double  );       \
  double _CL_OVERLOADABLE NAME(double2 );       \
  double _CL_OVERLOADABLE NAME(double3 );       \
  double _CL_OVERLOADABLE NAME(double4 );       \
  double _CL_OVERLOADABLE NAME(double8 );       \
  double _CL_OVERLOADABLE NAME(double16);)
#define _CL_DECLARE_FUNC_H_HHPVI(NAME)                             \
  __IF_FP16(                                                       \
  half _CL_OVERLOADABLE NAME(half, half, __local int *);           \
  half _CL_OVERLOADABLE NAME(half, half, __private int *);         \
  half _CL_OVERLOADABLE NAME(half, half, __global int *);          \
  half2 _CL_OVERLOADABLE NAME(half2, half2, __local int2 *);       \
  half2 _CL_OVERLOADABLE NAME(half2, half2, __private int2 *);     \
  half2 _CL_OVERLOADABLE NAME(half2, half2, __global int2 *);      \
  half3 _CL_OVERLOADABLE NAME(half3, half3, __local int3 *);       \
  half3 _CL_OVERLOADABLE NAME(half3, half3, __private int3 *);     \
  half3 _CL_OVERLOADABLE NAME(half3, half3, __global int3 *);      \
  half4 _CL_OVERLOADABLE NAME(half4, half4, __local int4 *);       \
  half4 _CL_OVERLOADABLE NAME(half4, half4, __private int4 *);     \
  half4 _CL_OVERLOADABLE NAME(half4, half4, __global int4 *);      \
  half8 _CL_OVERLOADABLE NAME(half8, half8, __local int8 *);       \
  half8 _CL_OVERLOADABLE NAME(half8, half8, __private int8 *);     \
  half8 _CL_OVERLOADABLE NAME(half8, half8, __global int8 *);      \
  half16 _CL_OVERLOADABLE NAME(half16, half16, __local int16 *);   \
  half16 _CL_OVERLOADABLE NAME(half16, half16, __private int16 *); \
  half16 _CL_OVERLOADABLE NAME(half16, half16, __global int16 *);)
#define _CL_DECLARE_FUNC_S_VV(NAME)                     \
  float  _CL_OVERLOADABLE NAME(float   , float   );     \
  float  _CL_OVERLOADABLE NAME(float2  , float2  );     \
  float  _CL_OVERLOADABLE NAME(float3  , float3  );     \
  float  _CL_OVERLOADABLE NAME(float4  , float4  );     \
  float  _CL_OVERLOADABLE NAME(float8  , float8  );     \
  float  _CL_OVERLOADABLE NAME(float16 , float16 );     \
  __IF_FP64(                                            \
  double _CL_OVERLOADABLE NAME(double  , double  );     \
  double _CL_OVERLOADABLE NAME(double2 , double2 );     \
  double _CL_OVERLOADABLE NAME(double3 , double3 );     \
  double _CL_OVERLOADABLE NAME(double4 , double4 );     \
  double _CL_OVERLOADABLE NAME(double8 , double8 );     \
  double _CL_OVERLOADABLE NAME(double16, double16);)
#define _CL_DECLARE_FUNC_F_F(NAME)              \
  float    _CL_OVERLOADABLE NAME(float   );     \
  float2   _CL_OVERLOADABLE NAME(float2  );     \
  float3   _CL_OVERLOADABLE NAME(float3  );     \
  float4   _CL_OVERLOADABLE NAME(float4  );     \
  float8   _CL_OVERLOADABLE NAME(float8  );     \
  float16  _CL_OVERLOADABLE NAME(float16 );
#define _CL_DECLARE_FUNC_F_FF(NAME)                     \
  float    _CL_OVERLOADABLE NAME(float   , float   );   \
  float2   _CL_OVERLOADABLE NAME(float2  , float2  );   \
  float3   _CL_OVERLOADABLE NAME(float3  , float3  );   \
  float4   _CL_OVERLOADABLE NAME(float4  , float4  );   \
  float8   _CL_OVERLOADABLE NAME(float8  , float8  );   \
  float16  _CL_OVERLOADABLE NAME(float16 , float16 );

/* Integer Constants */

#define CHAR_BIT  8
#define CHAR_MAX  SCHAR_MAX
#define CHAR_MIN  SCHAR_MIN
#define INT_MAX   2147483647
#define INT_MIN   (-2147483647 - 1)
#ifdef cles_khr_int64
#define LONG_MAX  0x7fffffffffffffffL
#define LONG_MIN  (-0x7fffffffffffffffL - 1)
#endif
#define SCHAR_MAX 127
#define SCHAR_MIN (-127 - 1)
#define SHRT_MAX  32767
#define SHRT_MIN  (-32767 - 1)
#define UCHAR_MAX 255
#define USHRT_MAX 65535
#define UINT_MAX  0xffffffff
#ifdef cles_khr_int64
#define ULONG_MAX 0xffffffffffffffffUL
#endif

/* Integer Functions */
#define _CL_DECLARE_FUNC_G_G(NAME)              \
  char     _CL_OVERLOADABLE NAME(char    );     \
  char2    _CL_OVERLOADABLE NAME(char2   );     \
  char3    _CL_OVERLOADABLE NAME(char3   );     \
  char4    _CL_OVERLOADABLE NAME(char4   );     \
  char8    _CL_OVERLOADABLE NAME(char8   );     \
  char16   _CL_OVERLOADABLE NAME(char16  );     \
  uchar    _CL_OVERLOADABLE NAME(uchar   );     \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  );     \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  );     \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  );     \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  );     \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 );     \
  short    _CL_OVERLOADABLE NAME(short   );     \
  short2   _CL_OVERLOADABLE NAME(short2  );     \
  short3   _CL_OVERLOADABLE NAME(short3  );     \
  short4   _CL_OVERLOADABLE NAME(short4  );     \
  short8   _CL_OVERLOADABLE NAME(short8  );     \
  short16  _CL_OVERLOADABLE NAME(short16 );     \
  ushort   _CL_OVERLOADABLE NAME(ushort  );     \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 );     \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 );     \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 );     \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 );     \
  ushort16 _CL_OVERLOADABLE NAME(ushort16);     \
  int      _CL_OVERLOADABLE NAME(int     );     \
  int2     _CL_OVERLOADABLE NAME(int2    );     \
  int3     _CL_OVERLOADABLE NAME(int3    );     \
  int4     _CL_OVERLOADABLE NAME(int4    );     \
  int8     _CL_OVERLOADABLE NAME(int8    );     \
  int16    _CL_OVERLOADABLE NAME(int16   );     \
  uint     _CL_OVERLOADABLE NAME(uint    );     \
  uint2    _CL_OVERLOADABLE NAME(uint2   );     \
  uint3    _CL_OVERLOADABLE NAME(uint3   );     \
  uint4    _CL_OVERLOADABLE NAME(uint4   );     \
  uint8    _CL_OVERLOADABLE NAME(uint8   );     \
  uint16   _CL_OVERLOADABLE NAME(uint16  );     \
  __IF_INT64(                                   \
  long     _CL_OVERLOADABLE NAME(long    );     \
  long2    _CL_OVERLOADABLE NAME(long2   );     \
  long3    _CL_OVERLOADABLE NAME(long3   );     \
  long4    _CL_OVERLOADABLE NAME(long4   );     \
  long8    _CL_OVERLOADABLE NAME(long8   );     \
  long16   _CL_OVERLOADABLE NAME(long16  );     \
  ulong    _CL_OVERLOADABLE NAME(ulong   );     \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  );     \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  );     \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  );     \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  );     \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 );)
#define _CL_DECLARE_FUNC_G_GG(NAME)                     \
  char     _CL_OVERLOADABLE NAME(char    , char    );   \
  char2    _CL_OVERLOADABLE NAME(char2   , char2   );   \
  char3    _CL_OVERLOADABLE NAME(char3   , char3   );   \
  char4    _CL_OVERLOADABLE NAME(char4   , char4   );   \
  char8    _CL_OVERLOADABLE NAME(char8   , char8   );   \
  char16   _CL_OVERLOADABLE NAME(char16  , char16  );   \
  uchar    _CL_OVERLOADABLE NAME(uchar   , uchar   );   \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar2  );   \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar3  );   \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar4  );   \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar8  );   \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar16 );   \
  short    _CL_OVERLOADABLE NAME(short   , short   );   \
  short2   _CL_OVERLOADABLE NAME(short2  , short2  );   \
  short3   _CL_OVERLOADABLE NAME(short3  , short3  );   \
  short4   _CL_OVERLOADABLE NAME(short4  , short4  );   \
  short8   _CL_OVERLOADABLE NAME(short8  , short8  );   \
  short16  _CL_OVERLOADABLE NAME(short16 , short16 );   \
  ushort   _CL_OVERLOADABLE NAME(ushort  , ushort  );   \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort2 );   \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort3 );   \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort4 );   \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort8 );   \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort16);   \
  int      _CL_OVERLOADABLE NAME(int     , int     );   \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    );   \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    );   \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    );   \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    );   \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   );   \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    );   \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   );   \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   );   \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   );   \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   );   \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  );   \
  __IF_INT64(                                           \
  long     _CL_OVERLOADABLE NAME(long    , long    );   \
  long2    _CL_OVERLOADABLE NAME(long2   , long2   );   \
  long3    _CL_OVERLOADABLE NAME(long3   , long3   );   \
  long4    _CL_OVERLOADABLE NAME(long4   , long4   );   \
  long8    _CL_OVERLOADABLE NAME(long8   , long8   );   \
  long16   _CL_OVERLOADABLE NAME(long16  , long16  );   \
  ulong    _CL_OVERLOADABLE NAME(ulong   , ulong   );   \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong2  );   \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong3  );   \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong4  );   \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong8  );   \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong16 );)
#define _CL_DECLARE_FUNC_G_GGG(NAME)                            \
  char     _CL_OVERLOADABLE NAME(char    , char    , char    ); \
  char2    _CL_OVERLOADABLE NAME(char2   , char2   , char2   ); \
  char3    _CL_OVERLOADABLE NAME(char3   , char3   , char3   ); \
  char4    _CL_OVERLOADABLE NAME(char4   , char4   , char4   ); \
  char8    _CL_OVERLOADABLE NAME(char8   , char8   , char8   ); \
  char16   _CL_OVERLOADABLE NAME(char16  , char16  , char16  ); \
  uchar    _CL_OVERLOADABLE NAME(uchar   , uchar   , uchar   ); \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar2  , uchar2  ); \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar3  , uchar3  ); \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar4  , uchar4  ); \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar8  , uchar8  ); \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar16 , uchar16 ); \
  short    _CL_OVERLOADABLE NAME(short   , short   , short   ); \
  short2   _CL_OVERLOADABLE NAME(short2  , short2  , short2  ); \
  short3   _CL_OVERLOADABLE NAME(short3  , short3  , short3  ); \
  short4   _CL_OVERLOADABLE NAME(short4  , short4  , short4  ); \
  short8   _CL_OVERLOADABLE NAME(short8  , short8  , short8  ); \
  short16  _CL_OVERLOADABLE NAME(short16 , short16 , short16 ); \
  ushort   _CL_OVERLOADABLE NAME(ushort  , ushort  , ushort  ); \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort2 , ushort2 ); \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort3 , ushort3 ); \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort4 , ushort4 ); \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort8 , ushort8 ); \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort16, ushort16); \
  int      _CL_OVERLOADABLE NAME(int     , int     , int     ); \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    , int2    ); \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    , int3    ); \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    , int4    ); \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    , int8    ); \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   , int16   ); \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    , uint    ); \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   , uint2   ); \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   , uint3   ); \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   , uint4   ); \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   , uint8   ); \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  , uint16  ); \
  __IF_INT64(                                                   \
  long     _CL_OVERLOADABLE NAME(long    , long    , long    ); \
  long2    _CL_OVERLOADABLE NAME(long2   , long2   , long2   ); \
  long3    _CL_OVERLOADABLE NAME(long3   , long3   , long3   ); \
  long4    _CL_OVERLOADABLE NAME(long4   , long4   , long4   ); \
  long8    _CL_OVERLOADABLE NAME(long8   , long8   , long8   ); \
  long16   _CL_OVERLOADABLE NAME(long16  , long16  , long16  ); \
  ulong    _CL_OVERLOADABLE NAME(ulong   , ulong   , ulong   ); \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong2  , ulong2  ); \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong3  , ulong3  ); \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong4  , ulong4  ); \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong8  , ulong8  ); \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong16 , ulong16 );)
#define _CL_DECLARE_FUNC_G_GGIG(NAME)                                   \
  char     _CL_OVERLOADABLE NAME(char    , char    , char    );         \
  char2    _CL_OVERLOADABLE NAME(char2   , char2   , char2   );         \
  char3    _CL_OVERLOADABLE NAME(char3   , char3   , char3   );         \
  char4    _CL_OVERLOADABLE NAME(char4   , char4   , char4   );         \
  char8    _CL_OVERLOADABLE NAME(char8   , char8   , char8   );         \
  char16   _CL_OVERLOADABLE NAME(char16  , char16  , char16  );         \
  uchar    _CL_OVERLOADABLE NAME(uchar   , uchar   , char    );         \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar2  , char2   );         \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar3  , char3   );         \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar4  , char4   );         \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar8  , char8   );         \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar16 , char16  );         \
  short    _CL_OVERLOADABLE NAME(short   , short   , short   );         \
  short2   _CL_OVERLOADABLE NAME(short2  , short2  , short2  );         \
  short3   _CL_OVERLOADABLE NAME(short3  , short3  , short3  );         \
  short4   _CL_OVERLOADABLE NAME(short4  , short4  , short4  );         \
  short8   _CL_OVERLOADABLE NAME(short8  , short8  , short8  );         \
  short16  _CL_OVERLOADABLE NAME(short16 , short16 , short16 );         \
  ushort   _CL_OVERLOADABLE NAME(ushort  , ushort  , short   );         \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort2 , short2  );         \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort3 , short3  );         \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort4 , short4  );         \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort8 , short8  );         \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort16, short16 );         \
  int      _CL_OVERLOADABLE NAME(int     , int     , int     );         \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    , int2    );         \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    , int3    );         \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    , int4    );         \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    , int8    );         \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   , int16   );         \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    , int     );         \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   , int2    );         \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   , int3    );         \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   , int4    );         \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   , int8    );         \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  , int16   );         \
  __IF_INT64(                                                           \
  long     _CL_OVERLOADABLE NAME(long    , long    , long    );         \
  long2    _CL_OVERLOADABLE NAME(long2   , long2   , long2   );         \
  long3    _CL_OVERLOADABLE NAME(long3   , long3   , long3   );         \
  long4    _CL_OVERLOADABLE NAME(long4   , long4   , long4   );         \
  long8    _CL_OVERLOADABLE NAME(long8   , long8   , long8   );         \
  long16   _CL_OVERLOADABLE NAME(long16  , long16  , long16  );         \
  ulong    _CL_OVERLOADABLE NAME(ulong   , ulong   , long    );         \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong2  , long2   );         \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong3  , long3   );         \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong4  , long4   );         \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong8  , long8   );         \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong16 , long16  );)
#define _CL_DECLARE_FUNC_G_GGUG(NAME)                                   \
  char     _CL_OVERLOADABLE NAME(char    , char    , uchar    );        \
  char2    _CL_OVERLOADABLE NAME(char2   , char2   , uchar2   );        \
  char3    _CL_OVERLOADABLE NAME(char3   , char3   , uchar3   );        \
  char4    _CL_OVERLOADABLE NAME(char4   , char4   , uchar4   );        \
  char8    _CL_OVERLOADABLE NAME(char8   , char8   , uchar8   );        \
  char16   _CL_OVERLOADABLE NAME(char16  , char16  , uchar16  );        \
  uchar    _CL_OVERLOADABLE NAME(uchar   , uchar   , uchar    );        \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar2  , uchar2   );        \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar3  , uchar3   );        \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar4  , uchar4   );        \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar8  , uchar8   );        \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar16 , uchar16  );        \
  short    _CL_OVERLOADABLE NAME(short   , short   , ushort   );        \
  short2   _CL_OVERLOADABLE NAME(short2  , short2  , ushort2  );        \
  short3   _CL_OVERLOADABLE NAME(short3  , short3  , ushort3  );        \
  short4   _CL_OVERLOADABLE NAME(short4  , short4  , ushort4  );        \
  short8   _CL_OVERLOADABLE NAME(short8  , short8  , ushort8  );        \
  short16  _CL_OVERLOADABLE NAME(short16 , short16 , ushort16 );        \
  ushort   _CL_OVERLOADABLE NAME(ushort  , ushort  , ushort   );        \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort2 , ushort2  );        \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort3 , ushort3  );        \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort4 , ushort4  );        \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort8 , ushort8  );        \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort16, ushort16 );        \
  int      _CL_OVERLOADABLE NAME(int     , int     , uint     );        \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    , uint2    );        \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    , uint3    );        \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    , uint4    );        \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    , uint8    );        \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   , uint16   );        \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    , uint     );        \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   , uint2    );        \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   , uint3    );        \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   , uint4    );        \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   , uint8    );        \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  , uint16   );        \
  __IF_INT64(                                                           \
  long     _CL_OVERLOADABLE NAME(long    , long    , ulong    );        \
  long2    _CL_OVERLOADABLE NAME(long2   , long2   , ulong2   );        \
  long3    _CL_OVERLOADABLE NAME(long3   , long3   , ulong3   );        \
  long4    _CL_OVERLOADABLE NAME(long4   , long4   , ulong4   );        \
  long8    _CL_OVERLOADABLE NAME(long8   , long8   , ulong8   );        \
  long16   _CL_OVERLOADABLE NAME(long16  , long16  , ulong16  );        \
  ulong    _CL_OVERLOADABLE NAME(ulong   , ulong   , ulong    );        \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong2  , ulong2   );        \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong3  , ulong3   );        \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong4  , ulong4   );        \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong8  , ulong8   );        \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong16 , ulong16  );)
#define _CL_DECLARE_FUNC_G_GS(NAME)                     \
  char2    _CL_OVERLOADABLE NAME(char2   , char  );     \
  char3    _CL_OVERLOADABLE NAME(char3   , char  );     \
  char4    _CL_OVERLOADABLE NAME(char4   , char  );     \
  char8    _CL_OVERLOADABLE NAME(char8   , char  );     \
  char16   _CL_OVERLOADABLE NAME(char16  , char  );     \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar );     \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar );     \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar );     \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar );     \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar );     \
  short2   _CL_OVERLOADABLE NAME(short2  , short );     \
  short3   _CL_OVERLOADABLE NAME(short3  , short );     \
  short4   _CL_OVERLOADABLE NAME(short4  , short );     \
  short8   _CL_OVERLOADABLE NAME(short8  , short );     \
  short16  _CL_OVERLOADABLE NAME(short16 , short );     \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort);     \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort);     \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort);     \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort);     \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort);     \
  int2     _CL_OVERLOADABLE NAME(int2    , int   );     \
  int3     _CL_OVERLOADABLE NAME(int3    , int   );     \
  int4     _CL_OVERLOADABLE NAME(int4    , int   );     \
  int8     _CL_OVERLOADABLE NAME(int8    , int   );     \
  int16    _CL_OVERLOADABLE NAME(int16   , int   );     \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint  );     \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint  );     \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint  );     \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint  );     \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint  );     \
  __IF_INT64(                                           \
  long2    _CL_OVERLOADABLE NAME(long2   , long  );     \
  long3    _CL_OVERLOADABLE NAME(long3   , long  );     \
  long4    _CL_OVERLOADABLE NAME(long4   , long  );     \
  long8    _CL_OVERLOADABLE NAME(long8   , long  );     \
  long16   _CL_OVERLOADABLE NAME(long16  , long  );     \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong );     \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong );     \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong );     \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong );     \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong );)
#define _CL_DECLARE_FUNC_UG_G(NAME)             \
  uchar    _CL_OVERLOADABLE NAME(char    );     \
  uchar2   _CL_OVERLOADABLE NAME(char2   );     \
  uchar3   _CL_OVERLOADABLE NAME(char3   );     \
  uchar4   _CL_OVERLOADABLE NAME(char4   );     \
  uchar8   _CL_OVERLOADABLE NAME(char8   );     \
  uchar16  _CL_OVERLOADABLE NAME(char16  );     \
  ushort   _CL_OVERLOADABLE NAME(short   );     \
  ushort2  _CL_OVERLOADABLE NAME(short2  );     \
  ushort3  _CL_OVERLOADABLE NAME(short3  );     \
  ushort4  _CL_OVERLOADABLE NAME(short4  );     \
  ushort8  _CL_OVERLOADABLE NAME(short8  );     \
  ushort16 _CL_OVERLOADABLE NAME(short16 );     \
  uint     _CL_OVERLOADABLE NAME(int     );     \
  uint2    _CL_OVERLOADABLE NAME(int2    );     \
  uint3    _CL_OVERLOADABLE NAME(int3    );     \
  uint4    _CL_OVERLOADABLE NAME(int4    );     \
  uint8    _CL_OVERLOADABLE NAME(int8    );     \
  uint16   _CL_OVERLOADABLE NAME(int16   );     \
  __IF_INT64(                                   \
  ulong    _CL_OVERLOADABLE NAME(long    );     \
  ulong2   _CL_OVERLOADABLE NAME(long2   );     \
  ulong3   _CL_OVERLOADABLE NAME(long3   );     \
  ulong4   _CL_OVERLOADABLE NAME(long4   );     \
  ulong8   _CL_OVERLOADABLE NAME(long8   );     \
  ulong16  _CL_OVERLOADABLE NAME(long16  );)    \
  uchar    _CL_OVERLOADABLE NAME(uchar   );     \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  );     \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  );     \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  );     \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  );     \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 );     \
  ushort   _CL_OVERLOADABLE NAME(ushort  );     \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 );     \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 );     \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 );     \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 );     \
  ushort16 _CL_OVERLOADABLE NAME(ushort16);     \
  uint     _CL_OVERLOADABLE NAME(uint    );     \
  uint2    _CL_OVERLOADABLE NAME(uint2   );     \
  uint3    _CL_OVERLOADABLE NAME(uint3   );     \
  uint4    _CL_OVERLOADABLE NAME(uint4   );     \
  uint8    _CL_OVERLOADABLE NAME(uint8   );     \
  uint16   _CL_OVERLOADABLE NAME(uint16  );     \
  __IF_INT64(                                   \
  ulong    _CL_OVERLOADABLE NAME(ulong   );     \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  );     \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  );     \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  );     \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  );     \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 );)
#define _CL_DECLARE_FUNC_UG_GG(NAME)                    \
  uchar    _CL_OVERLOADABLE NAME(char    , char    );   \
  uchar2   _CL_OVERLOADABLE NAME(char2   , char2   );   \
  uchar3   _CL_OVERLOADABLE NAME(char3   , char3   );   \
  uchar4   _CL_OVERLOADABLE NAME(char4   , char4   );   \
  uchar8   _CL_OVERLOADABLE NAME(char8   , char8   );   \
  uchar16  _CL_OVERLOADABLE NAME(char16  , char16  );   \
  ushort   _CL_OVERLOADABLE NAME(short   , short   );   \
  ushort2  _CL_OVERLOADABLE NAME(short2  , short2  );   \
  ushort3  _CL_OVERLOADABLE NAME(short3  , short3  );   \
  ushort4  _CL_OVERLOADABLE NAME(short4  , short4  );   \
  ushort8  _CL_OVERLOADABLE NAME(short8  , short8  );   \
  ushort16 _CL_OVERLOADABLE NAME(short16 , short16 );   \
  uint     _CL_OVERLOADABLE NAME(int     , int     );   \
  uint2    _CL_OVERLOADABLE NAME(int2    , int2    );   \
  uint3    _CL_OVERLOADABLE NAME(int3    , int3    );   \
  uint4    _CL_OVERLOADABLE NAME(int4    , int4    );   \
  uint8    _CL_OVERLOADABLE NAME(int8    , int8    );   \
  uint16   _CL_OVERLOADABLE NAME(int16   , int16   );   \
  __IF_INT64(                                           \
  ulong    _CL_OVERLOADABLE NAME(long    , long    );   \
  ulong2   _CL_OVERLOADABLE NAME(long2   , long2   );   \
  ulong3   _CL_OVERLOADABLE NAME(long3   , long3   );   \
  ulong4   _CL_OVERLOADABLE NAME(long4   , long4   );   \
  ulong8   _CL_OVERLOADABLE NAME(long8   , long8   );   \
  ulong16  _CL_OVERLOADABLE NAME(long16  , long16  );)  \
  uchar    _CL_OVERLOADABLE NAME(uchar   , uchar   );   \
  uchar2   _CL_OVERLOADABLE NAME(uchar2  , uchar2  );   \
  uchar3   _CL_OVERLOADABLE NAME(uchar3  , uchar3  );   \
  uchar4   _CL_OVERLOADABLE NAME(uchar4  , uchar4  );   \
  uchar8   _CL_OVERLOADABLE NAME(uchar8  , uchar8  );   \
  uchar16  _CL_OVERLOADABLE NAME(uchar16 , uchar16 );   \
  ushort   _CL_OVERLOADABLE NAME(ushort  , ushort  );   \
  ushort2  _CL_OVERLOADABLE NAME(ushort2 , ushort2 );   \
  ushort3  _CL_OVERLOADABLE NAME(ushort3 , ushort3 );   \
  ushort4  _CL_OVERLOADABLE NAME(ushort4 , ushort4 );   \
  ushort8  _CL_OVERLOADABLE NAME(ushort8 , ushort8 );   \
  ushort16 _CL_OVERLOADABLE NAME(ushort16, ushort16);   \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    );   \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   );   \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   );   \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   );   \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   );   \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  );   \
  __IF_INT64(                                           \
  ulong    _CL_OVERLOADABLE NAME(ulong   , ulong   );   \
  ulong2   _CL_OVERLOADABLE NAME(ulong2  , ulong2  );   \
  ulong3   _CL_OVERLOADABLE NAME(ulong3  , ulong3  );   \
  ulong4   _CL_OVERLOADABLE NAME(ulong4  , ulong4  );   \
  ulong8   _CL_OVERLOADABLE NAME(ulong8  , ulong8  );   \
  ulong16  _CL_OVERLOADABLE NAME(ulong16 , ulong16 );)
#define _CL_DECLARE_FUNC_LG_GUG(NAME)                   \
  short    _CL_OVERLOADABLE NAME(char    , uchar   );   \
  short2   _CL_OVERLOADABLE NAME(char2   , uchar2  );   \
  short3   _CL_OVERLOADABLE NAME(char3   , uchar3  );   \
  short4   _CL_OVERLOADABLE NAME(char4   , uchar4  );   \
  short8   _CL_OVERLOADABLE NAME(char8   , uchar8  );   \
  short16  _CL_OVERLOADABLE NAME(char16  , uchar16 );   \
  ushort   _CL_OVERLOADABLE NAME(uchar   , uchar   );   \
  ushort2  _CL_OVERLOADABLE NAME(uchar2  , uchar2  );   \
  ushort3  _CL_OVERLOADABLE NAME(uchar3  , uchar3  );   \
  ushort4  _CL_OVERLOADABLE NAME(uchar4  , uchar4  );   \
  ushort8  _CL_OVERLOADABLE NAME(uchar8  , uchar8  );   \
  ushort16 _CL_OVERLOADABLE NAME(uchar16 , uchar16 );   \
  uint     _CL_OVERLOADABLE NAME(ushort  , ushort  );   \
  uint2    _CL_OVERLOADABLE NAME(ushort2 , ushort2 );   \
  uint3    _CL_OVERLOADABLE NAME(ushort3 , ushort3 );   \
  uint4    _CL_OVERLOADABLE NAME(ushort4 , ushort4 );   \
  uint8    _CL_OVERLOADABLE NAME(ushort8 , ushort8 );   \
  uint16   _CL_OVERLOADABLE NAME(ushort16, ushort16);   \
  int      _CL_OVERLOADABLE NAME(short   , ushort  );   \
  int2     _CL_OVERLOADABLE NAME(short2  , ushort2 );   \
  int3     _CL_OVERLOADABLE NAME(short3  , ushort3 );   \
  int4     _CL_OVERLOADABLE NAME(short4  , ushort4 );   \
  int8     _CL_OVERLOADABLE NAME(short8  , ushort8 );   \
  int16    _CL_OVERLOADABLE NAME(short16 , ushort16);   \
  __IF_INT64(                                           \
  long     _CL_OVERLOADABLE NAME(int     , uint    );   \
  long2    _CL_OVERLOADABLE NAME(int2    , uint2   );   \
  long3    _CL_OVERLOADABLE NAME(int3    , uint3   );   \
  long4    _CL_OVERLOADABLE NAME(int4    , uint4   );   \
  long8    _CL_OVERLOADABLE NAME(int8    , uint8   );   \
  long16   _CL_OVERLOADABLE NAME(int16   , uint16  );   \
  ulong    _CL_OVERLOADABLE NAME(uint    , uint    );   \
  ulong2   _CL_OVERLOADABLE NAME(uint2   , uint2   );   \
  ulong3   _CL_OVERLOADABLE NAME(uint3   , uint3   );   \
  ulong4   _CL_OVERLOADABLE NAME(uint4   , uint4   );   \
  ulong8   _CL_OVERLOADABLE NAME(uint8   , uint8   );   \
  ulong16  _CL_OVERLOADABLE NAME(uint16  , uint16  );)
#define _CL_DECLARE_FUNC_I_IG(NAME)             \
  int _CL_OVERLOADABLE NAME(char   );           \
  int _CL_OVERLOADABLE NAME(char2  );           \
  int _CL_OVERLOADABLE NAME(char3  );           \
  int _CL_OVERLOADABLE NAME(char4  );           \
  int _CL_OVERLOADABLE NAME(char8  );           \
  int _CL_OVERLOADABLE NAME(char16 );           \
  int _CL_OVERLOADABLE NAME(short  );           \
  int _CL_OVERLOADABLE NAME(short2 );           \
  int _CL_OVERLOADABLE NAME(short3 );           \
  int _CL_OVERLOADABLE NAME(short4 );           \
  int _CL_OVERLOADABLE NAME(short8 );           \
  int _CL_OVERLOADABLE NAME(short16);           \
  int _CL_OVERLOADABLE NAME(int    );           \
  int _CL_OVERLOADABLE NAME(int2   );           \
  int _CL_OVERLOADABLE NAME(int3   );           \
  int _CL_OVERLOADABLE NAME(int4   );           \
  int _CL_OVERLOADABLE NAME(int8   );           \
  int _CL_OVERLOADABLE NAME(int16  );           \
  __IF_INT64(                                   \
  int _CL_OVERLOADABLE NAME(long   );           \
  int _CL_OVERLOADABLE NAME(long2  );           \
  int _CL_OVERLOADABLE NAME(long3  );           \
  int _CL_OVERLOADABLE NAME(long4  );           \
  int _CL_OVERLOADABLE NAME(long8  );           \
  int _CL_OVERLOADABLE NAME(long16 );)
#define _CL_DECLARE_FUNC_J_JJ(NAME)                     \
  int      _CL_OVERLOADABLE NAME(int     , int     );   \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    );   \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    );   \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    );   \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    );   \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   );   \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    );   \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   );   \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   );   \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   );   \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   );   \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  );
#define _CL_DECLARE_FUNC_J_JJJ(NAME)                            \
  int      _CL_OVERLOADABLE NAME(int     , int     , int     ); \
  int2     _CL_OVERLOADABLE NAME(int2    , int2    , int2    ); \
  int3     _CL_OVERLOADABLE NAME(int3    , int3    , int3    ); \
  int4     _CL_OVERLOADABLE NAME(int4    , int4    , int4    ); \
  int8     _CL_OVERLOADABLE NAME(int8    , int8    , int8    ); \
  int16    _CL_OVERLOADABLE NAME(int16   , int16   , int16   ); \
  uint     _CL_OVERLOADABLE NAME(uint    , uint    , uint    ); \
  uint2    _CL_OVERLOADABLE NAME(uint2   , uint2   , uint2   ); \
  uint3    _CL_OVERLOADABLE NAME(uint3   , uint3   , uint3   ); \
  uint4    _CL_OVERLOADABLE NAME(uint4   , uint4   , uint4   ); \
  uint8    _CL_OVERLOADABLE NAME(uint8   , uint8   , uint8   ); \
  uint16   _CL_OVERLOADABLE NAME(uint16  , uint16  , uint16  );

/* Vector Functions */

#define _CL_DECLARE_VLOAD_TYPE_WIDTH(TYPE, WIDTH)                                      \
  TYPE##WIDTH _CL_OVERLOADABLE vload##WIDTH (size_t offset, const __global   TYPE *p); \
  TYPE##WIDTH _CL_OVERLOADABLE vload##WIDTH (size_t offset, const __local    TYPE *p); \
  TYPE##WIDTH _CL_OVERLOADABLE vload##WIDTH (size_t offset, const __constant TYPE *p); \
  TYPE##WIDTH _CL_OVERLOADABLE vload##WIDTH (size_t offset, const __private  TYPE *p);

#define _CL_DECLARE_VLOAD_WIDTH(WIDTH)                     \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(char,   WIDTH)              \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(uchar,  WIDTH)              \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(short,  WIDTH)              \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(ushort, WIDTH)              \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(int,    WIDTH)              \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(uint,   WIDTH)              \
  __IF_INT64(                                              \
              _CL_DECLARE_VLOAD_TYPE_WIDTH(long,  WIDTH)   \
              _CL_DECLARE_VLOAD_TYPE_WIDTH(ulong, WIDTH)   \
            )                                              \
  __IF_FP64(                                               \
             _CL_DECLARE_VLOAD_TYPE_WIDTH(double, WIDTH)   \
           )                                               \
  _CL_DECLARE_VLOAD_TYPE_WIDTH(float, WIDTH)

#define _CL_DECLARE_VSTORE_TYPE_WIDTH(TYPE, WIDTH)                                           \
  void _CL_OVERLOADABLE vstore##WIDTH (TYPE##WIDTH data, size_t offset, __global  TYPE *p);  \
  void _CL_OVERLOADABLE vstore##WIDTH (TYPE##WIDTH data, size_t offset, __local   TYPE *p);  \
  void _CL_OVERLOADABLE vstore##WIDTH (TYPE##WIDTH data, size_t offset, __private TYPE *p);

#define _CL_DECLARE_VSTORE_WIDTH(WIDTH)                     \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(char,   WIDTH)              \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(uchar,  WIDTH)              \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(short,  WIDTH)              \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(ushort, WIDTH)              \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(int,    WIDTH)              \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(uint,   WIDTH)              \
  __IF_INT64(                                               \
              _CL_DECLARE_VSTORE_TYPE_WIDTH(long,  WIDTH)   \
              _CL_DECLARE_VSTORE_TYPE_WIDTH(ulong, WIDTH)   \
            )                                               \
  __IF_FP64(                                                \
             _CL_DECLARE_VSTORE_TYPE_WIDTH(double, WIDTH)   \
           )                                                \
  _CL_DECLARE_VSTORE_TYPE_WIDTH(float, WIDTH)

#if defined(_W2CL_EXTENSION_CL_KHR_FP16) || defined(_W2CL_EXTENSION_ALL)
#define _CL_DECLARE_VLOAD_HALF(MOD)                                     \
  float   _CL_OVERLOADABLE vload_half   (size_t offset, const MOD half *p); \
  float2  _CL_OVERLOADABLE vload_half2  (size_t offset, const MOD half *p); \
  float3  _CL_OVERLOADABLE vload_half3  (size_t offset, const MOD half *p); \
  float4  _CL_OVERLOADABLE vload_half4  (size_t offset, const MOD half *p); \
  float8  _CL_OVERLOADABLE vload_half8  (size_t offset, const MOD half *p); \
  float16 _CL_OVERLOADABLE vload_half16 (size_t offset, const MOD half *p); \
  float2  _CL_OVERLOADABLE vloada_half2 (size_t offset, const MOD half *p); \
  float3  _CL_OVERLOADABLE vloada_half3 (size_t offset, const MOD half *p); \
  float4  _CL_OVERLOADABLE vloada_half4 (size_t offset, const MOD half *p); \
  float8  _CL_OVERLOADABLE vloada_half8 (size_t offset, const MOD half *p); \
  float16 _CL_OVERLOADABLE vloada_half16(size_t offset, const MOD half *p);

/* stores to half may have a suffix: _rte _rtz _rtp _rtn */
#define _CL_DECLARE_VSTORE_HALF(MOD, SUFFIX)                            \
  void _CL_OVERLOADABLE vstore_half##SUFFIX   (float   data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstore_half2##SUFFIX  (float2  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstore_half3##SUFFIX  (float3  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstore_half4##SUFFIX  (float4  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstore_half8##SUFFIX  (float8  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstore_half16##SUFFIX (float16 data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstorea_half2##SUFFIX (float2  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstorea_half3##SUFFIX (float3  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstorea_half4##SUFFIX (float4  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstorea_half8##SUFFIX (float8  data, size_t offset, MOD half *p); \
  void _CL_OVERLOADABLE vstorea_half16##SUFFIX(float16 data, size_t offset, MOD half *p);

#else

// cl_khr_fp16 not defined, define no-op macros
#define _CL_DECLARE_VLOAD_HALF(MOD)
#define _CL_DECLARE_VSTORE_HALF(MOD, SUFFIX)

#endif

/* Atomic operations */

// TODO: only declare these if used

// special versions that allow to differentiate between cl_khr_fp16 and  cl_khr_fp64
#define _CL_DECLARE_ATOMICS_BASIC(MOD, TYPE)    \
  _CL_OVERLOADABLE TYPE atomic_add    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_sub    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_xchg   (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_inc    (volatile MOD TYPE *p);           \
  _CL_OVERLOADABLE TYPE atomic_dec    (volatile MOD TYPE *p);           \
  _CL_OVERLOADABLE TYPE atomic_cmpxchg(volatile MOD TYPE *p, TYPE cmp, TYPE val);
  
#define _CL_DECLARE_ATOMICS_EXTENDED(MOD, TYPE)                                  \
  _CL_OVERLOADABLE TYPE atomic_add    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_sub    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_xchg   (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_inc    (volatile MOD TYPE *p);           \
  _CL_OVERLOADABLE TYPE atomic_dec    (volatile MOD TYPE *p);           \
  _CL_OVERLOADABLE TYPE atomic_cmpxchg(volatile MOD TYPE *p, TYPE cmp, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_min    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_max    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_and    (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_or     (volatile MOD TYPE *p, TYPE val); \
  _CL_OVERLOADABLE TYPE atomic_xor    (volatile MOD TYPE *p, TYPE val);

#define _CL_DECLARE_ATOMICS(MOD, TYPE)                                  \
  _CL_DECLARE_ATOMICS_BASIC(MOD, TYPE)                                  \
  _CL_DECLARE_ATOMICS_EXTENDED(MOD, TYPE)

_CL_DECLARE_ATOMICS(__global, int )
_CL_DECLARE_ATOMICS(__global, uint)
_CL_DECLARE_ATOMICS(__local , int )
_CL_DECLARE_ATOMICS(__local , uint)

_CL_OVERLOADABLE float atomic_xchg(volatile __global float *p, float val);
_CL_OVERLOADABLE float atomic_xchg(volatile __local  float *p, float val);

#if defined( _W2CL_EXTENSION_CL_KHR_INT64_BASE_ATOMICS) || defined(_W2CL_EXTENSION_ALL)
_CL_DECLARE_ATOMICS_BASIC(__global, long )
_CL_DECLARE_ATOMICS_BASIC(__global, ulong)
_CL_DECLARE_ATOMICS_BASIC(__local , long )
_CL_DECLARE_ATOMICS_BASIC(__local , ulong)
#endif

#if defined( _W2CL_EXTENSION_CL_KHR_INT64_EXTENDED_ATOMICS) || defined(_W2CL_EXTENSION_ALL)
_CL_DECLARE_ATOMICS_EXTENDED(__global, long )
_CL_DECLARE_ATOMICS_EXTENDED(__global, ulong)
_CL_DECLARE_ATOMICS_EXTENDED(__local , long )
_CL_DECLARE_ATOMICS_EXTENDED(__local , ulong)
#endif

#define atom_add     atomic_add
#define atom_sub     atomic_sub
#define atom_xchg    atomic_xchg
#define atom_inc     atomic_inc
#define atom_dec     atomic_dec
#define atom_cmpxchg atomic_cmpxchg
#define atom_min     atomic_min
#define atom_max     atomic_max
#define atom_and     atomic_and
#define atom_or      atomic_or
#define atom_xor     atomic_xor



/* Miscellaneous Vector Functions */

// This code leads to an ICE in Clang

// #define _CL_DECLARE_SHUFFLE_2(GTYPE, UGTYPE, STYPE, M)                  \
//   GTYPE##2 _CL_OVERLOADABLE shuffle(GTYPE##M x, UGTYPE##2 mask)         \
//   {                                                                     \
//     UGTYPE bits = (UGTYPE)1 << (UGTYPE)M;                               \
//     UGTYPE bmask = bits - (UGTYPE)1;                                    \
//     return __builtin_shufflevector(x, x,                                \
//                                    mask.s0 & bmask, mask.s1 & bmask);   \
//   }
// #define _CL_DECLARE_SHUFFLE_3(GTYPE, UGTYPE, STYPE, M)                  \
//   GTYPE##3 _CL_OVERLOADABLE shuffle(GTYPE##M x, UGTYPE##3 mask)         \
//   {                                                                     \
//     UGTYPE bits = (UGTYPE)1 << (UGTYPE)M;                               \
//     UGTYPE bmask = bits - (UGTYPE)1;                                    \
//     return __builtin_shufflevector(x, x,                                \
//                                    mask.s0 & bmask, mask.s1 & bmask,    \
//                                    mask.s2 & bmask);                    \
//   }
// #define _CL_DECLARE_SHUFFLE_4(GTYPE, UGTYPE, STYPE, M)                  \
//   GTYPE##4 _CL_OVERLOADABLE shuffle(GTYPE##M x, UGTYPE##4 mask)         \
//   {                                                                     \
//     UGTYPE bits = (UGTYPE)1 << (UGTYPE)M;                               \
//     UGTYPE bmask = bits - (UGTYPE)1;                                    \
//     return __builtin_shufflevector(x, x,                                \
//                                    mask.s0 & bmask, mask.s1 & bmask,    \
//                                    mask.s2 & bmask, mask.s3 & bmask);   \
//   }
// #define _CL_DECLARE_SHUFFLE_8(GTYPE, UGTYPE, STYPE, M)                  \
//   GTYPE##8 _CL_OVERLOADABLE shuffle(GTYPE##M x, UGTYPE##8 mask)         \
//   {                                                                     \
//     UGTYPE bits = (UGTYPE)1 << (UGTYPE)M;                               \
//     UGTYPE bmask = bits - (UGTYPE)1;                                    \
//     return __builtin_shufflevector(x, x,                                \
//                                    mask.s0 & bmask, mask.s1 & bmask,    \
//                                    mask.s2 & bmask, mask.s3 & bmask,    \
//                                    mask.s4 & bmask, mask.s5 & bmask,    \
//                                    mask.s6 & bmask, mask.s7 & bmask);   \
//   }
// #define _CL_DECLARE_SHUFFLE_16(GTYPE, UGTYPE, STYPE, M)                 \
//   GTYPE##16 _CL_OVERLOADABLE shuffle(GTYPE##M x, UGTYPE##16 mask)       \
//   {                                                                     \
//     UGTYPE bits = (UGTYPE)1 << (UGTYPE)M;                               \
//     UGTYPE bmask = bits - (UGTYPE)1;                                    \
//     return __builtin_shufflevector(x, x,                                \
//                                    mask.s0 & bmask, mask.s1 & bmask,    \
//                                    mask.s2 & bmask, mask.s3 & bmask,    \
//                                    mask.s4 & bmask, mask.s5 & bmask,    \
//                                    mask.s6 & bmask, mask.s7 & bmask,    \
//                                    mask.s8 & bmask, mask.s9 & bmask,    \
//                                    mask.sa & bmask, mask.sb & bmask,    \
//                                    mask.sc & bmask, mask.sd & bmask,    \
//                                    mask.se & bmask, mask.sf & bmask);   \
//   }
// 
// #define _CL_DECLARE_SHUFFLE(GTYPE, UGTYPE, STYPE, M)    \
//   _CL_DECLARE_SHUFFLE_2 (GTYPE, UGTYPE, STYPE, M)       \
//   _CL_DECLARE_SHUFFLE_3 (GTYPE, UGTYPE, STYPE, M)       \
//   _CL_DECLARE_SHUFFLE_4 (GTYPE, UGTYPE, STYPE, M)       \
//   _CL_DECLARE_SHUFFLE_8 (GTYPE, UGTYPE, STYPE, M)       \
//   _CL_DECLARE_SHUFFLE_16(GTYPE, UGTYPE, STYPE, M)
// 
// _CL_DECLARE_SHUFFLE(char  , uchar , char  , 2 )
// _CL_DECLARE_SHUFFLE(char  , uchar , char  , 3 )
// _CL_DECLARE_SHUFFLE(char  , uchar , char  , 4 )
// _CL_DECLARE_SHUFFLE(char  , uchar , char  , 8 )
// _CL_DECLARE_SHUFFLE(char  , uchar , char  , 16)
// _CL_DECLARE_SHUFFLE(uchar , uchar , char  , 2 )
// _CL_DECLARE_SHUFFLE(uchar , uchar , char  , 3 )
// _CL_DECLARE_SHUFFLE(uchar , uchar , char  , 4 )
// _CL_DECLARE_SHUFFLE(uchar , uchar , char  , 8 )
// _CL_DECLARE_SHUFFLE(uchar , uchar , char  , 16)
// _CL_DECLARE_SHUFFLE(short , ushort, short , 2 )
// _CL_DECLARE_SHUFFLE(short , ushort, short , 3 )
// _CL_DECLARE_SHUFFLE(short , ushort, short , 4 )
// _CL_DECLARE_SHUFFLE(short , ushort, short , 8 )
// _CL_DECLARE_SHUFFLE(short , ushort, short , 16)
// _CL_DECLARE_SHUFFLE(ushort, ushort, short , 2 )
// _CL_DECLARE_SHUFFLE(ushort, ushort, short , 3 )
// _CL_DECLARE_SHUFFLE(ushort, ushort, short , 4 )
// _CL_DECLARE_SHUFFLE(ushort, ushort, short , 8 )
// _CL_DECLARE_SHUFFLE(ushort, ushort, short , 16)
// _CL_DECLARE_SHUFFLE(int   , uint  , int   , 2 )
// _CL_DECLARE_SHUFFLE(int   , uint  , int   , 3 )
// _CL_DECLARE_SHUFFLE(int   , uint  , int   , 4 )
// _CL_DECLARE_SHUFFLE(int   , uint  , int   , 8 )
// _CL_DECLARE_SHUFFLE(int   , uint  , int   , 16)
// _CL_DECLARE_SHUFFLE(uint  , uint  , int   , 2 )
// _CL_DECLARE_SHUFFLE(uint  , uint  , int   , 3 )
// _CL_DECLARE_SHUFFLE(uint  , uint  , int   , 4 )
// _CL_DECLARE_SHUFFLE(uint  , uint  , int   , 8 )
// _CL_DECLARE_SHUFFLE(uint  , uint  , int   , 16)
// _CL_DECLARE_SHUFFLE(long  , ulong , long  , 2 )
// _CL_DECLARE_SHUFFLE(long  , ulong , long  , 3 )
// _CL_DECLARE_SHUFFLE(long  , ulong , long  , 4 )
// _CL_DECLARE_SHUFFLE(long  , ulong , long  , 8 )
// _CL_DECLARE_SHUFFLE(long  , ulong , long  , 16)
// _CL_DECLARE_SHUFFLE(ulong , ulong , long  , 2 )
// _CL_DECLARE_SHUFFLE(ulong , ulong , long  , 3 )
// _CL_DECLARE_SHUFFLE(ulong , ulong , long  , 4 )
// _CL_DECLARE_SHUFFLE(ulong , ulong , long  , 8 )
// _CL_DECLARE_SHUFFLE(ulong , ulong , long  , 16)
// _CL_DECLARE_SHUFFLE(float , uint  , float , 2 )
// _CL_DECLARE_SHUFFLE(float , uint  , float , 3 )
// _CL_DECLARE_SHUFFLE(float , uint  , float , 4 )
// _CL_DECLARE_SHUFFLE(float , uint  , float , 8 )
// _CL_DECLARE_SHUFFLE(float , uint  , float , 16)
// _CL_DECLARE_SHUFFLE(double, ulong , double, 2 )
// _CL_DECLARE_SHUFFLE(double, ulong , double, 3 )
// _CL_DECLARE_SHUFFLE(double, ulong , double, 4 )
// _CL_DECLARE_SHUFFLE(double, ulong , double, 8 )
// _CL_DECLARE_SHUFFLE(double, ulong , double, 16)

// shuffle2



int printf(const /*constant*/ char * restrict format, ...)
  __attribute__((format(printf, 1, 2)));



/* Async Copies from Global to Local Memory, Local to
   Global Memory, and Prefetch */

// TODO: only declare these if used

#ifndef _CL_HAS_EVENT_T
typedef uint event_t;
#endif

_CL_OVERLOADABLE
event_t async_work_group_copy (__local void *dst,
                                const __global void *src,
                                size_t num_gentypes,
                                event_t event);
_CL_OVERLOADABLE
event_t async_work_group_copy (__global void *dst,
                                const __local void *src,
                                size_t num_gentypes,
                                event_t event);
_CL_OVERLOADABLE
event_t async_work_group_strided_copy (__local void *dst,
                                        const __global void *src,
                                        size_t num_gentypes,
                                        size_t src_stride,
                                        event_t event);
_CL_OVERLOADABLE
event_t async_work_group_strided_copy (__global void *dst,
                                        const __local void *src,
                                        size_t num_gentypes,
                                        size_t dst_stride,
                                        event_t event);

void wait_group_events (int num_events,                      
                        event_t *event_list);                 

// Fake prefetch declaration to let it be caught more informatively in WebCLAnalyser
void prefetch(const __global void *, size_t);

// Image support

// Starting from Clang 3.3 the image and sampler are detected
// as opaque types by the frontend. In order to define
// the default builtins we use C functions which require 
// the typedefs to the actual underlying types. Clang 3.2
// the typedefs throughout as the types are not detected
// by the frontend.

#if !defined(_CL_HAS_IMAGE_ACCESS) || defined(POCL_C_BUILTIN)
typedef int sampler_t;
typedef struct dev_image_t* image2d_t;
typedef struct dev_image_t* image3d_t;
typedef struct dev_image_t* image1d_t;
typedef struct dev_image_t* image1d_buffer_t;
typedef struct dev_image_t* image2d_array_t;
typedef struct dev_image_t* image1d_array_t;
#endif


/* cl_channel_order */
#define CL_R                                        0x10B0
#define CL_A                                        0x10B1
#define CL_RG                                       0x10B2
#define CL_RA                                       0x10B3
#define CL_RGB                                      0x10B4
#define CL_RGBA                                     0x10B5
#define CL_BGRA                                     0x10B6
#define CL_ARGB                                     0x10B7
#define CL_INTENSITY                                0x10B8
#define CL_LUMINANCE                                0x10B9
#define CL_Rx                                       0x10BA
#define CL_RGx                                      0x10BB
#define CL_RGBx                                     0x10BC
#define CL_DEPTH                                    0x10BD
#define CL_DEPTH_STENCIL                            0x10BE

/* cl_channel_type */
#define CL_SNORM_INT8                               0x10D0
#define CL_SNORM_INT16                              0x10D1
#define CL_UNORM_INT8                               0x10D2
#define CL_UNORM_INT16                              0x10D3
#define CL_UNORM_SHORT_565                          0x10D4
#define CL_UNORM_SHORT_555                          0x10D5
#define CL_UNORM_INT_101010                         0x10D6
#define CL_SIGNED_INT8                              0x10D7
#define CL_SIGNED_INT16                             0x10D8
#define CL_SIGNED_INT32                             0x10D9
#define CL_UNSIGNED_INT8                            0x10DA
#define CL_UNSIGNED_INT16                           0x10DB
#define CL_UNSIGNED_INT32                           0x10DC
#define CL_HALF_FLOAT                               0x10DD
#define CL_FLOAT                                    0x10DE
#define CL_UNORM_INT24                              0x10DF

/* cl_addressing _mode */
#define CLK_ADDRESS_NONE                            0x00
#define CLK_ADDRESS_MIRRORED_REPEAT                 0x01
#define CLK_ADDRESS_REPEAT                          0x02
#define CLK_ADDRESS_CLAMP_TO_EDGE                   0x03
#define CLK_ADDRESS_CLAMP                           0x04

/* cl_sampler_info */
#define CLK_NORMALIZED_COORDS_FALSE                 0x00
#define CLK_NORMALIZED_COORDS_TRUE                  0x08

/* filter_mode */
#define CLK_FILTER_NEAREST                          0x00
#define CLK_FILTER_LINEAR                           0x10

void _CL_OVERLOADABLE write_imagei (image2d_t image, int2 coord, int4 color);

void _CL_OVERLOADABLE write_imageui (image2d_t image, int2 coord, uint4 color);

void _CL_OVERLOADABLE write_imagef (image2d_t image, int2 coord,
                                    float4 color);
/* not implemented 
void _CL_OVERLOADABLE write_imagef (image2d_array_t image, int4 coord,
                                    float4 color);

void _CL_OVERLOADABLE write_imagei (image2d_array_t image, int4 coord,
                                    int4 color);

void _CL_OVERLOADABLE write_imageui (image2d_array_t image, int4 coord,
                                     uint4 color);

void _CL_OVERLOADABLE write_imagef (image1d_t image, int coord,
                                    float4 color);

void _CL_OVERLOADABLE write_imagei (image1d_t image, int coord,
                                    int4 color);

void _CL_OVERLOADABLE write_imageui (image1d_t image, int coord, 
                                     uint4 color);

void _CL_OVERLOADABLE write_imagef (image1d_buffer_t image, int coord, 
                                    float4 color);

void _CL_OVERLOADABLE write_imagei (image1d_buffer_t image, int coord,
                                     int4 color);

void _CL_OVERLOADABLE write_imageui (image1d_buffer_t image, int coord,
                                     uint4 color);

void _CL_OVERLOADABLE write_imagef (image1d_array_t image, int2 coord,
                                    float4 color);

void _CL_OVERLOADABLE write_imagei (image1d_array_t image, int2 coord,
                                    int4 color);

void _CL_OVERLOADABLE write_imageui (image1d_array_t image, int2 coord,
                                     uint4 color);

void _CL_OVERLOADABLE write_imageui (image3d_t image, int4 coord,
                                     uint4 color);
*/

int _CL_OVERLOADABLE get_image_width (image2d_t image);
int _CL_OVERLOADABLE get_image_height (image2d_t image);

int _CL_OVERLOADABLE get_image_width (image3d_t image);
int _CL_OVERLOADABLE get_image_height (image3d_t image);
