// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s -DQUALDEFS | grep -v CHECK | %FileCheck %s

typedef char wcl_char;
typedef unsigned char wcl_unsigned_char;
typedef uchar wcl_uchar;
typedef short wcl_short;
typedef unsigned short wcl_unsigned_short;
typedef ushort wcl_ushort;
typedef int wcl_int;
typedef unsigned int wcl_unsigned_int;
typedef uint wcl_uint;
typedef long wcl_long;
typedef unsigned long wcl_unsigned_long;
typedef ulong wcl_ulong;
typedef float wcl_float;

typedef char4 wcl_char4;
typedef uchar4 wcl_uchar4;
typedef short4 wcl_short4;
typedef ushort4 wcl_ushort4;
typedef int4 wcl_int4;
typedef uint4 wcl_uint4;
typedef long4 wcl_long4;
typedef ulong4 wcl_ulong4;
typedef float4 wcl_float4;

typedef image2d_t wcl_image2d_t;
#ifdef QUALDEFS
typedef __read_only image2d_t wcl_ro_image2d_t;
typedef __write_only image2d_t wcl_wo_image2d_t;
#endif
typedef sampler_t wcl_sampler_t;

__kernel void json_builtins(
    wcl_image2d_t image_arg, // __read_only
    __read_only wcl_image2d_t read_image_arg,
    __write_only wcl_image2d_t write_image_arg,
    wcl_sampler_t sampler_arg
#ifdef QUALDEFS
    ,
    wcl_ro_image2d_t read_image_arg2,
    wcl_wo_image2d_t write_image_arg2
#endif
    )
{
}

__kernel void json_scalar_arrays(
    __global wcl_char *char_arg,
    __local wcl_unsigned_char *unsigned_char_arg,
    __constant wcl_uchar *uchar_arg,

    __global wcl_short *short_arg,
    __local wcl_unsigned_short *wcl_unsigned_short_arg,
    __constant wcl_ushort *ushort_arg,

    __global wcl_int *int_arg,
    __local wcl_unsigned_int *unsigned_int_arg,
    __constant wcl_uint *uint_arg,

    __global wcl_long *long_arg,
    __local wcl_unsigned_long *unsiged_long_arg,
    __constant wcl_ulong *ulong_arg,

    const __global wcl_float *float_arg)
{
}

__kernel void json_scalars(
    wcl_char char_arg,
    wcl_unsigned_char unsigned_char_arg,
    wcl_uchar uchar_arg,

    wcl_short short_arg,
    wcl_unsigned_short wcl_unsigned_short_arg,
    wcl_ushort ushort_arg,

    wcl_int int_arg,
    wcl_unsigned_int unsigned_int_arg,
    wcl_uint uint_arg,

    wcl_long long_arg,
    wcl_unsigned_long unsiged_long_arg,
    wcl_ulong ulong_arg,

    const wcl_float float_arg)
{
}

__kernel void json_vector_arrays(
    __constant wcl_char4 *char4_arg,
    __local wcl_uchar4 *uchar4_arg,
    __global wcl_short4 *short4_arg,
    __constant wcl_ushort4 *ushort4_arg,
    __local wcl_int4 *int4_arg,
    __global wcl_uint4 *uint4_arg,
    __constant wcl_long4 *long4_arg,
    __local wcl_ulong4 *ulong4_arg,
    const __global wcl_float4 *float4_arg)
{
}

__kernel void json_vectors(
    wcl_char4 char4_arg, wcl_uchar4 uchar4_arg,
    wcl_short4 short4_arg, wcl_ushort4 ushort4_arg,
    wcl_int4 int4_arg, wcl_uint4 uint4_arg,
    wcl_long4 long4_arg, wcl_ulong4 ulong4_arg,
    const wcl_float4 float4_arg)
{
}

// CHECK:{
// CHECK:    "version" : "1.0",
// CHECK:    "kernels" :
// CHECK:        {
// CHECK:            "json_builtins" :
// CHECK:                {
// CHECK:                    "image_arg" :
// CHECK:                        {
// CHECK:                            "index" : 0,
// CHECK:                            "type" : "image2d_t",
// CHECK:                            "access" : "read_only"
// CHECK:                        },
// CHECK:                    "read_image_arg" :
// CHECK:                        {
// CHECK:                            "index" : 1,
// CHECK:                            "type" : "image2d_t",
// CHECK:                            "access" : "read_only"
// CHECK:                        },
// CHECK:                    "write_image_arg" :
// CHECK:                        {
// CHECK:                            "index" : 2,
// CHECK:                            "type" : "image2d_t",
// CHECK:                            "access" : "write_only"
// CHECK:                        },
// CHECK:                    "sampler_arg" :
// CHECK:                        {
// CHECK:                            "index" : 3,
// CHECK:                            "type" : "sampler_t"
// CHECK:                        },
// CHECK:                    "read_image_arg2" :
// CHECK:                        {
// CHECK:                            "index" : 4,
// CHECK:                            "type" : "image2d_t",
// CHECK:                            "access" : "read_only"
// CHECK:                        },
// CHECK:                    "write_image_arg2" :
// CHECK:                        {
// CHECK:                            "index" : 5,
// CHECK:                            "type" : "image2d_t",
// CHECK:                            "access" : "write_only"
// CHECK:                        }
// CHECK:                },
// CHECK:            "json_scalar_arrays" :
// CHECK:                {
// CHECK:                    "char_arg" :
// CHECK:                        {
// CHECK:                            "index" : 0,
// CHECK:                            "type" : "char *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_char_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_char_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 1,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "unsigned_char_arg" :
// CHECK:                        {
// CHECK:                            "index" : 2,
// CHECK:                            "type" : "unsigned char *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_unsigned_char_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_unsigned_char_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 3,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "uchar_arg" :
// CHECK:                        {
// CHECK:                            "index" : 4,
// CHECK:                            "type" : "uchar *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_uchar_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_uchar_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 5,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "short_arg" :
// CHECK:                        {
// CHECK:                            "index" : 6,
// CHECK:                            "type" : "short *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_short_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_short_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 7,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "wcl_unsigned_short_arg" :
// CHECK:                        {
// CHECK:                            "index" : 8,
// CHECK:                            "type" : "unsigned short *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_wcl_unsigned_short_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_wcl_unsigned_short_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 9,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "ushort_arg" :
// CHECK:                        {
// CHECK:                            "index" : 10,
// CHECK:                            "type" : "ushort *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_ushort_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_ushort_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 11,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "int_arg" :
// CHECK:                        {
// CHECK:                            "index" : 12,
// CHECK:                            "type" : "int *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_int_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_int_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 13,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "unsigned_int_arg" :
// CHECK:                        {
// CHECK:                            "index" : 14,
// CHECK:                            "type" : "unsigned int *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_unsigned_int_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_unsigned_int_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 15,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "uint_arg" :
// CHECK:                        {
// CHECK:                            "index" : 16,
// CHECK:                            "type" : "uint *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_uint_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_uint_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 17,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "long_arg" :
// CHECK:                        {
// CHECK:                            "index" : 18,
// CHECK:                            "type" : "long *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_long_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_long_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 19,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "unsiged_long_arg" :
// CHECK:                        {
// CHECK:                            "index" : 20,
// CHECK:                            "type" : "unsigned long *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_unsiged_long_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_unsiged_long_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 21,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "ulong_arg" :
// CHECK:                        {
// CHECK:                            "index" : 22,
// CHECK:                            "type" : "ulong *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_ulong_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_ulong_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 23,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "float_arg" :
// CHECK:                        {
// CHECK:                            "index" : 24,
// CHECK:                            "type" : "float *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_float_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_float_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 25,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        }
// CHECK:                },
// CHECK:            "json_scalars" :
// CHECK:                {
// CHECK:                    "char_arg" :
// CHECK:                        {
// CHECK:                            "index" : 0,
// CHECK:                            "type" : "char"
// CHECK:                        },
// CHECK:                    "unsigned_char_arg" :
// CHECK:                        {
// CHECK:                            "index" : 1,
// CHECK:                            "type" : "unsigned char"
// CHECK:                        },
// CHECK:                    "uchar_arg" :
// CHECK:                        {
// CHECK:                            "index" : 2,
// CHECK:                            "type" : "uchar"
// CHECK:                        },
// CHECK:                    "short_arg" :
// CHECK:                        {
// CHECK:                            "index" : 3,
// CHECK:                            "type" : "short"
// CHECK:                        },
// CHECK:                    "wcl_unsigned_short_arg" :
// CHECK:                        {
// CHECK:                            "index" : 4,
// CHECK:                            "type" : "unsigned short"
// CHECK:                        },
// CHECK:                    "ushort_arg" :
// CHECK:                        {
// CHECK:                            "index" : 5,
// CHECK:                            "type" : "ushort"
// CHECK:                        },
// CHECK:                    "int_arg" :
// CHECK:                        {
// CHECK:                            "index" : 6,
// CHECK:                            "type" : "int"
// CHECK:                        },
// CHECK:                    "unsigned_int_arg" :
// CHECK:                        {
// CHECK:                            "index" : 7,
// CHECK:                            "type" : "unsigned int"
// CHECK:                        },
// CHECK:                    "uint_arg" :
// CHECK:                        {
// CHECK:                            "index" : 8,
// CHECK:                            "type" : "uint"
// CHECK:                        },
// CHECK:                    "long_arg" :
// CHECK:                        {
// CHECK:                            "index" : 9,
// CHECK:                            "type" : "long"
// CHECK:                        },
// CHECK:                    "unsiged_long_arg" :
// CHECK:                        {
// CHECK:                            "index" : 10,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "ulong_arg" :
// CHECK:                        {
// CHECK:                            "index" : 11,
// CHECK:                            "type" : "ulong"
// CHECK:                        },
// CHECK:                    "float_arg" :
// CHECK:                        {
// CHECK:                            "index" : 12,
// CHECK:                            "type" : "float"
// CHECK:                        }
// CHECK:                },
// CHECK:            "json_vector_arrays" :
// CHECK:                {
// CHECK:                    "char4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 0,
// CHECK:                            "type" : "char4 *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_char4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_char4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 1,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "uchar4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 2,
// CHECK:                            "type" : "uchar4 *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_uchar4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_uchar4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 3,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "short4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 4,
// CHECK:                            "type" : "short4 *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_short4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_short4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 5,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "ushort4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 6,
// CHECK:                            "type" : "ushort4 *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_ushort4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_ushort4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 7,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "int4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 8,
// CHECK:                            "type" : "int4 *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_int4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_int4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 9,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "uint4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 10,
// CHECK:                            "type" : "uint4 *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_uint4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_uint4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 11,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "long4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 12,
// CHECK:                            "type" : "long4 *",
// CHECK:                            "address-space" : "constant",
// CHECK:                            "size-parameter" : "_wcl_long4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_long4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 13,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "ulong4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 14,
// CHECK:                            "type" : "ulong4 *",
// CHECK:                            "address-space" : "local",
// CHECK:                            "size-parameter" : "_wcl_ulong4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_ulong4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 15,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        },
// CHECK:                    "float4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 16,
// CHECK:                            "type" : "float4 *",
// CHECK:                            "address-space" : "global",
// CHECK:                            "size-parameter" : "_wcl_float4_arg_size"
// CHECK:                        },
// CHECK:                    "_wcl_float4_arg_size" :
// CHECK:                        {
// CHECK:                            "index" : 17,
// CHECK:                            "type" : "unsigned long"
// CHECK:                        }
// CHECK:                },
// CHECK:            "json_vectors" :
// CHECK:                {
// CHECK:                    "char4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 0,
// CHECK:                            "type" : "char4"
// CHECK:                        },
// CHECK:                    "uchar4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 1,
// CHECK:                            "type" : "uchar4"
// CHECK:                        },
// CHECK:                    "short4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 2,
// CHECK:                            "type" : "short4"
// CHECK:                        },
// CHECK:                    "ushort4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 3,
// CHECK:                            "type" : "ushort4"
// CHECK:                        },
// CHECK:                    "int4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 4,
// CHECK:                            "type" : "int4"
// CHECK:                        },
// CHECK:                    "uint4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 5,
// CHECK:                            "type" : "uint4"
// CHECK:                        },
// CHECK:                    "long4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 6,
// CHECK:                            "type" : "long4"
// CHECK:                        },
// CHECK:                    "ulong4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 7,
// CHECK:                            "type" : "ulong4"
// CHECK:                        },
// CHECK:                    "float4_arg" :
// CHECK:                        {
// CHECK:                            "index" : 8,
// CHECK:                            "type" : "float4"
// CHECK:                        }
// CHECK:                }
// CHECK:        }
// CHECK:}
