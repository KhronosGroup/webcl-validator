#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif
// Just check that no calls produce errors in the validator
// RUN: %webcl-validator %s

__kernel void builtin_wrappers(__global char *output, 
                               __global float *f
#ifdef cl_khr_fp64
                               , __global float *d
#endif
                               )
{
    __local int offset;

    offset = 0;

    vload2(offset, f);
    vload4(offset, f);
    vload8(offset, f);
    vload16(offset, f);

    vload_half2(offset, f);
    vload_half4(offset, f);
    vload_half8(offset, f);
    vload_half16(offset, f);

    vloada_half2(offset, f);
    vloada_half4(offset, f);
    vloada_half8(offset, f);
    vloada_half16(offset, f);

    vstore2((float2)(0, 0), offset, f);
    vstore4((float4)(0, 0, 0, 0), offset, f);
    vstore8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstore_half2((float2)(0, 0), offset, f);
    vstore_half4((float4)(0, 0, 0, 0), offset, f);
    vstore_half8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore_half16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstore_half2_rte((float2)(0, 0), offset, f);
    vstore_half4_rte((float4)(0, 0, 0, 0), offset, f);
    vstore_half8_rte((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore_half16_rte((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstore_half2_rtz((float2)(0, 0), offset, f);
    vstore_half4_rtz((float4)(0, 0, 0, 0), offset, f);
    vstore_half8_rtz((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore_half16_rtz((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstore_half2_rtn((float2)(0, 0), offset, f);
    vstore_half4_rtn((float4)(0, 0, 0, 0), offset, f);
    vstore_half8_rtn((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore_half16_rtn((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstore_half2_rtp((float2)(0, 0), offset, f);
    vstore_half4_rtp((float4)(0, 0, 0, 0), offset, f);
    vstore_half8_rtp((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstore_half16_rtp((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstorea_half2((float2)(0, 0), offset, f);
    vstorea_half4((float4)(0, 0, 0, 0), offset, f);
    vstorea_half8((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstorea_half16((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstorea_half2_rte((float2)(0, 0), offset, f);
    vstorea_half4_rte((float4)(0, 0, 0, 0), offset, f);
    vstorea_half8_rte((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstorea_half16_rte((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstorea_half2_rtz((float2)(0, 0), offset, f);
    vstorea_half4_rtz((float4)(0, 0, 0, 0), offset, f);
    vstorea_half8_rtz((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstorea_half16_rtz((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstorea_half2_rtn((float2)(0, 0), offset, f);
    vstorea_half4_rtn((float4)(0, 0, 0, 0), offset, f);
    vstorea_half8_rtn((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstorea_half16_rtn((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

    vstorea_half2_rtp((float2)(0, 0), offset, f);
    vstorea_half4_rtp((float4)(0, 0, 0, 0), offset, f);
    vstorea_half8_rtp((float8)(0, 0, 0, 0, 0, 0, 0, 0), offset, f);
    vstorea_half16_rtp((float16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, f);

#ifdef cl_khr_fp64
    vload2(offset, d);
    vload4(offset, d);
    vload8(offset, d);
    vload16(offset, d);

    vload_half2(offset, d);
    vload_half4(offset, d);
    vload_half8(offset, d);
    vload_half16(offset, d);

    vloada_half2(offset, d);
    vloada_half4(offset, d);
    vloada_half8(offset, d);
    vloada_half16(offset, d);

    vstore2((double2)(0, 0), offset, d);
    vstore4((double4)(0, 0, 0, 0), offset, d);
    vstore8((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore16((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstore_half2((double2)(0, 0), offset, d);
    vstore_half4((double4)(0, 0, 0, 0), offset, d);
    vstore_half8((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore_half16((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstore_half2_rte((double2)(0, 0), offset, d);
    vstore_half4_rte((double4)(0, 0, 0, 0), offset, d);
    vstore_half8_rte((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore_half16_rte((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstore_half2_rtz((double2)(0, 0), offset, d);
    vstore_half4_rtz((double4)(0, 0, 0, 0), offset, d);
    vstore_half8_rtz((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore_half16_rtz((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstore_half2_rtn((double2)(0, 0), offset, d);
    vstore_half4_rtn((double4)(0, 0, 0, 0), offset, d);
    vstore_half8_rtn((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore_half16_rtn((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstore_half2_rtp((double2)(0, 0), offset, d);
    vstore_half4_rtp((double4)(0, 0, 0, 0), offset, d);
    vstore_half8_rtp((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstore_half16_rtp((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstorea_half2((double2)(0, 0), offset, d);
    vstorea_half4((double4)(0, 0, 0, 0), offset, d);
    vstorea_half8((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstorea_half16((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstorea_half2_rte((double2)(0, 0), offset, d);
    vstorea_half4_rte((double4)(0, 0, 0, 0), offset, d);
    vstorea_half8_rte((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstorea_half16_rte((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstorea_half2_rtz((double2)(0, 0), offset, d);
    vstorea_half4_rtz((double4)(0, 0, 0, 0), offset, d);
    vstorea_half8_rtz((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstorea_half16_rtz((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstorea_half2_rtn((double2)(0, 0), offset, d);
    vstorea_half4_rtn((double4)(0, 0, 0, 0), offset, d);
    vstorea_half8_rtn((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstorea_half16_rtn((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);

    vstorea_half2_rtp((double2)(0, 0), offset, d);
    vstorea_half4_rtp((double4)(0, 0, 0, 0), offset, d);
    vstorea_half8_rtp((double8)(0, 0, 0, 0, 0, 0, 0, 0), offset, d);
    vstorea_half16_rtp((double16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), offset, d);
#endif
}
