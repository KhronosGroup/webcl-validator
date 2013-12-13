// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void baz()
{ 
    int a = 0;
    // CHECK: error: initializer is not of type sampler_t
    sampler_t sampler = a;
}

__kernel void sampler_type_errors2(sampler_t sampler1)
{
    // CHECK: error: variables of type sampler_t must always be initialized
    sampler_t sampler2;
    // CHECK: error: sampler_t is not a constant integer expression or originate from function parameters
    sampler_t sampler3 = sampler2 ? 0 : 0;
    // CHECK: error: sampler_t is not a constant integer expression or originate from function parameters
    sampler_t sampler4 = 1 ? sampler2 : 0;
    // CHECK: error: sampler_t is not a constant integer expression or originate from function parameters
    sampler_t sampler5 = sampler1 | 0;
}

