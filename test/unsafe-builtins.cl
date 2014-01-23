#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif
#ifdef cl_khr_fp64
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

// RUN: %webcl-validator %s -Dcl_khr_fp16 -Dcl_khr_fp64 2>&1 | grep -v CHECK | %FileCheck %s

// We should be declaring all builtins at the moment
// CHECK-NOT: warning: implicit declaration of function

__kernel void unsafe_builtins(
    float x,
    __constant float *input,
    __global float *output)
{
    float y, z, f;
    int i;
    int2 i2;
    half2 h2;
    double2 d2;

    // CHECK: _wcl_frac
    f = fract(x, &y);
    // CHECK: _wcl_frac
    d2 = fract(d2, &d2);
    // CHECK: _wcl_frexp
    f = frexp(x, &i);
    // CHECK: _wcl_frexp
    h2 = frexp(h2, &i2);
    // CHECK: _wcl_frexp
    d2 = frexp(d2, &i2);
    // CHECK: _wcl_modf
    f = modf(x, &y);
    // CHECK: _wcl_lgamma_r
    f = lgamma_r(x, &i);
    // CHECK: _wcl_lgamma_r
    h2 = lgamma_r(h2, &i2);
    // CHECK: _wcl_lgamma_r
    d2 = lgamma_r(d2, &i2);
    // CHECK: _wcl_remquo
    f = remquo(x, z, &i);
    // CHECK: _wcl_remquo
    h2 = remquo(h2, h2, &i2);
    // CHECK: _wcl_remquo
    d2 = remquo(d2, d2, &i2);
}
