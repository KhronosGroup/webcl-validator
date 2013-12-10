// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void foo(sampler_t sampler, int i)
{
    // CHECK-NOT: error: sampler_t must always originate from parameters
    nonexisting(sampler); // 1
    // CHECK: error: sampler_t must always originate from parameters
    nonexisting((sampler_t) i); // 2
}

void bar(sampler_t *sampler)
{
    // this code involved sampler_t*
    // CHECK-NOT: error: sampler_t must always originate from parameters
    *(int*) sampler = 42;
}

__kernel void sampler_type_as_argument(sampler_t sampler, __constant int *ptr, image2d_t image)
{
    sampler_t sampler2;
    // CHECK-NOT: error: sampler_t must always originate from parameters
    nonexisting(sampler); // 3
    // CHECK: error: sampler_t must always originate from parameters
    nonexisting(sampler2); // 4
    // CHECK-NOT: error: sampler_t must always originate from parameters
    foo(sampler, 0); // 5
    // CHECK: error: sampler_t must always originate from parameters
    foo(sampler2, 0); // 6
    // CHECK: error: sampler_t must always originate from parameters
    foo((sampler_t) 0, 0); // 7
    // CHECK: sampler_t must always be used as a function argument
    *(int*) sampler = 42; // 8
    // CHECK: sampler_t must always be used as a function argument
    sampler; // 9
    // CHECK: sampler_t must always be used as a function argument
    bar(&sampler); // 11
    // CHECK: sampler_t must always be used as a function argument
    (int) sampler; // 12
    
    baz(image);
}
