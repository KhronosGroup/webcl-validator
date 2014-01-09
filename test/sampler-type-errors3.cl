// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: error: Global scope variables must be in constant address space.
sampler_t globalsampler1;

// CHECK: error: Constant address space variables must be initialized.
__constant sampler_t globalsampler2;

__kernel void sampler_type_errors3(sampler_t sampler1)
{
    sampler_t sampler = CLK_ADDRESS_CLAMP;
}

