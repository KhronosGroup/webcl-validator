// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void bar(sampler_t* samplerptr)
{
}

__kernel void sampler_type_errors5(sampler_t sampler)
{
    // CHECK: sampler_t must always be used as a function argument
    bar(&sampler); // 11
    // CHECK: error: sampler_t must always be used as a function argument
    &sampler;
}

