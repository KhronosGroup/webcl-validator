// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

__kernel void unsafe_builtins(
    image2d_t image)
{
    // CHECK: error: CLK_ADDRESS_NONE is not a valid address mode for sampler_t
    sampler_t sampler = CLK_FILTER_LINEAR | CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_TRUE;
    read_imagef( image, sampler, (float2)(0, 0));
}
