// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void foo(image2d_t image, sampler_t sampler)
{
    // this goes through unmodified
    // CHECK: read_imagef(image, sampler, (float2)(0, 0));
    read_imagef(image, sampler, (float2)(0, 0));
    // this goes through transformed
    // CHECK: read_imagef(image,/* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE, (float2)(0, 0));
    read_imagef(image, 1, (float2)(0, 0));
}

void foo2(sampler_t sampler1, sampler_t sampler2, sampler_t sampler3, sampler_t sampler4)
{
    // nothing
}

void foo_const(image2d_t image, sampler_t sampler)
{
    // CHECK: const sampler_t sampler1 = /* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE;
    const sampler_t sampler1 = 1;
    // CHECK: const sampler_t sampler2 = sampler;
    const sampler_t sampler2 = sampler;
    read_imagef(image, sampler1, (float2)(0, 0));
}

__kernel void unsafe_builtins(image2d_t image,
                              sampler_t sampler)
{
    // CHECK: foo(image, sampler);
    foo(image, sampler);
    // CHECK: foo2(/* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE,/* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE, sampler,/* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE);
    foo2(1, 1, sampler, 1);
}
