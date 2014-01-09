// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void sampler_type_errors4(sampler_t sampler1)
{
    // CHECK: error: variables of type sampler_t must always be initialized
    sampler_t sampler2;
}

