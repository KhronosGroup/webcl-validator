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

// we need to be able to find canonical type of image3d_t
#define _CL_HAS_IMAGE_ACCESS 1
typedef int sampler_t;
typedef struct dev_image_t* image2d_t;
typedef struct image3d_t_* image3d_t;
typedef struct dev_image_t* image1d_t;
typedef struct dev_image_t* image1d_buffer_t;
typedef struct dev_image_t* image2d_array_t;
typedef struct dev_image_t* image1d_array_t;

/* Enable double precision. This should really only be done when
   building the run-time library; when building application code, we
   should instead check a macro to see whether the application has
   enabled this. At the moment, always enable this seems fine, since
   all our target devices will support double precision anyway.

   FIX: this is not really true. TCE target is 32-bit scalars
   only. Seems the pragma does not add the macro, so we have the target
   define the macro and the pragma is conditionally enabled.
*/
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
#ifdef cl_khr_fp16
#  define __IF_FP16(x) x
#else
#  define __IF_FP16(x)
#endif
#ifdef cl_khr_fp64
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
#ifndef cl_khr_fp16
typedef struct error_undefined_type_half error_undefined_type_half;
#  define half error_undefined_type_half
#endif
#ifndef cl_khr_fp64
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

#ifdef cl_khr_fp16
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

#ifdef cl_khr_fp64
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

#ifdef cl_khr_fp16
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

#ifdef cl_khr_fp64
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

/* Atomic operations */

#define _CL_DECLARE_ATOMICS(MOD, TYPE)                                  \
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
_CL_DECLARE_ATOMICS(__global, int )
_CL_DECLARE_ATOMICS(__global, uint)
_CL_DECLARE_ATOMICS(__local , int )
_CL_DECLARE_ATOMICS(__local , uint)

_CL_OVERLOADABLE float atomic_xchg(volatile __global float *p, float val);
_CL_OVERLOADABLE float atomic_xchg(volatile __local  float *p, float val);

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

#ifndef _CL_HAS_EVENT_T
typedef uint event_t;
#endif

#define _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE)            \
  _CL_OVERLOADABLE                                              \
  event_t async_work_group_copy (__local GENTYPE *dst,          \
                                 const __global GENTYPE *src,   \
                                 size_t num_gentypes,           \
                                 event_t event);                \
                                                                \
  _CL_OVERLOADABLE                                              \
  event_t async_work_group_copy (__global GENTYPE *dst,         \
                                 const __local GENTYPE *src,    \
                                 size_t num_gentypes,           \
                                 event_t event);                \
  _CL_OVERLOADABLE                                              \
  event_t async_work_group_strided_copy (__local GENTYPE *dst,        \
                                         const __global GENTYPE *src, \
                                         size_t num_gentypes,         \
                                         size_t src_stride,           \
                                         event_t event);              \
  _CL_OVERLOADABLE                                                    \
  event_t async_work_group_strided_copy (__global GENTYPE *dst,       \
                                         const __local GENTYPE *src,  \
                                         size_t num_gentypes,         \
                                         size_t dst_stride,           \
                                         event_t event);              \
                                                                
void wait_group_events (int num_events,                      
                        event_t *event_list);                 

#define _CL_DECLARE_ASYNC_COPY_FUNCS(GENTYPE)      \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE)     \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE##2)   \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE##3)   \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE##4)   \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE##8)   \
  _CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(GENTYPE##16)  \

_CL_DECLARE_ASYNC_COPY_FUNCS(char);
_CL_DECLARE_ASYNC_COPY_FUNCS(uchar);
_CL_DECLARE_ASYNC_COPY_FUNCS(short);
_CL_DECLARE_ASYNC_COPY_FUNCS(ushort);
_CL_DECLARE_ASYNC_COPY_FUNCS(int);
_CL_DECLARE_ASYNC_COPY_FUNCS(uint);
__IF_INT64(_CL_DECLARE_ASYNC_COPY_FUNCS(long));
__IF_INT64(_CL_DECLARE_ASYNC_COPY_FUNCS(ulong));

__IF_FP16(_CL_DECLARE_ASYNC_COPY_FUNCS_SINGLE(half));
_CL_DECLARE_ASYNC_COPY_FUNCS(float);
__IF_FP64(_CL_DECLARE_ASYNC_COPY_FUNCS(double));

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

/* not implemented 
void _CL_OVERLOADABLE write_imagef (image2d_t image, int2 coord,
                                    float4 color);

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
