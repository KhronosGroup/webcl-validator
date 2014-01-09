// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void unsafe_builtins(
    image2d_t image)
{
    // this is checked by the clang implementation:
    // CHECK: sampler_t sampler = CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_TRUE;
    sampler_t sampler = CLK_FILTER_LINEAR | CLK_ADDRESS_CLAMP | CLK_NORMALIZED_COORDS_TRUE;
    // CHECK-NOT: warning: implicit declaration of function 'read_imagef'
    read_imagef( image, sampler, (float2)(0, 0));
    // CHECK: /* transformed */ CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE
    sampler_t sampler3 = //
      /* Foo, */
      4/1;
}
