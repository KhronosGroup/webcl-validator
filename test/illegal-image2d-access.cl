// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

constant sampler_t sampler =
    CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void illegal_image_access(
// CHECK: error: image2d_t parameters can only have read_only or write_only access qualifier
    __read_write image2d_t image)
{
    read_imagef(image, sampler, (float2) (0.0f, 0.0f));
}
