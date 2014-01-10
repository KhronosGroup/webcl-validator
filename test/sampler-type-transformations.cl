// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: __constant sampler_t global_sampler = /* transformed */ CLK_ADDRESS_MIRRORED_REPEAT | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE;
__constant sampler_t global_sampler = 1;

void foo(image2d_t image, sampler_t sampler)
{
    // this goes through unmodified
    // CHECK: read_imagef(image, sampler, (float2)(0, 0));
    read_imagef(image, sampler, (float2)(0, 0));
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
    // CHECK: foo(_wcl_allocs, image, sampler);
    foo(image, sampler);
    // CHECK: foo(_wcl_allocs, image, _wcl_constant_allocations.global_sampler);
    foo(image, global_sampler);
}
