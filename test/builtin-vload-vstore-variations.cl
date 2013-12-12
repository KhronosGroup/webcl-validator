#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
// Check that no calls produce errors in the validator, and there are no implicit decl warnings
// FIXME: we shouldn't need to -Dcl_khr_fp{16,64} here - they should be defined based on the
//        corresponding extensions being enabled, as they are in the CLI validator
// RUN: %webcl-validator %s -Dcl_khr_fp16 -Dcl_khr_fp64 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK-NOT: error:

__kernel void builtin_wrappers(__global char *output, 
                               __global float *f
#ifdef cl_khr_fp16
                               , __global half *h
#endif
#ifdef cl_khr_fp64
                               , __global double *d
#endif
                               )
{
    __local int offset;

    offset = 0;

    vload2(offset, f);
    vload4(offset, f);
    vload8(offset, f);
    vload16(offset, f);

#ifdef cl_khr_fp16
    // CHECK-NOT: warning: implicit declaration of function 'vload_half2'
    vload_half2(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vload_half4'
    vload_half4(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vload_half8'
    vload_half8(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vload_half16'
    vload_half16(offset, h);

    // CHECK-NOT: warning: implicit declaration of function 'vloada_half2'
    vloada_half2(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vloada_half4'
    vloada_half4(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vloada_half8'
    vloada_half8(offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vloada_half16'
    vloada_half16(offset, h);
#endif // fp16

    vstore2((float2)(0, 0), offset, f);
    vstore4((float4)(0, 0, 0, 0), offset, f);
    vstore8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

#ifdef cl_khr_fp16
    // CHECK-NOT: warning: implicit declaration of function 'vstore_half2'
    vstore_half2((float2)(0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstore_half4'
    vstore_half4((float4)(0, 0, 0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstore_half8'
    vstore_half8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstore_half16'
    vstore_half16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    // Catch all the rounding variants, can't be bothered to write them all down
    // CHECK-NOT: warning: implicit declaration of function 'vstorea?_half1?[2468]_{rte,rtz,rtn,rtp}'

    vstore_half2_rte((float2)(0, 0), offset, h);
    vstore_half4_rte((float4)(0, 0, 0, 0), offset, h);
    vstore_half8_rte((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstore_half16_rte((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstore_half2_rtz((float2)(0, 0), offset, h);
    vstore_half4_rtz((float4)(0, 0, 0, 0), offset, h);
    vstore_half8_rtz((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstore_half16_rtz((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstore_half2_rtn((float2)(0, 0), offset, h);
    vstore_half4_rtn((float4)(0, 0, 0, 0), offset, h);
    vstore_half8_rtn((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstore_half16_rtn((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstore_half2_rtp((float2)(0, 0), offset, h);
    vstore_half4_rtp((float4)(0, 0, 0, 0), offset, h);
    vstore_half8_rtp((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstore_half16_rtp((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    // CHECK-NOT: warning: implicit declaration of function 'vstorea_half2'
    vstorea_half2((float2)(0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstorea_half4'
    vstorea_half4((float4)(0, 0, 0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstorea_half8'
    vstorea_half8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    // CHECK-NOT: warning: implicit declaration of function 'vstorea_half16'
    vstorea_half16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstorea_half2_rte((float2)(0, 0), offset, h);
    vstorea_half4_rte((float4)(0, 0, 0, 0), offset, h);
    vstorea_half8_rte((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstorea_half16_rte((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstorea_half2_rtz((float2)(0, 0), offset, h);
    vstorea_half4_rtz((float4)(0, 0, 0, 0), offset, h);
    vstorea_half8_rtz((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstorea_half16_rtz((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstorea_half2_rtn((float2)(0, 0), offset, h);
    vstorea_half4_rtn((float4)(0, 0, 0, 0), offset, h);
    vstorea_half8_rtn((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstorea_half16_rtn((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);

    vstorea_half2_rtp((float2)(0, 0), offset, h);
    vstorea_half4_rtp((float4)(0, 0, 0, 0), offset, h);
    vstorea_half8_rtp((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, h);
    vstorea_half16_rtp((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, h);
#endif // fp16

#ifdef cl_khr_fp64
    vload2(offset, d);
    vload4(offset, d);
    vload8(offset, d);
    vload16(offset, d);

    vstore2((double2)(0, 0), offset, d);
    vstore4((double4)(0, 0, 0, 0), offset, d);
    vstore8((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore16((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);
#endif
}
