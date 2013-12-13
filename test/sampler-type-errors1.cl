// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void foo(sampler_t sampler, int i)
{
    // CHECK: error: sampler_t must always originate from parameters or local variables
    nonexisting((sampler_t) i); // 2
}

void bar(sampler_t* samplerptr)
{
}

// CHECK-NOT: error: sampler_t must always be used as a function argument of a variable initializer
__kernel void sampler_type_errors1(sampler_t sampler, __constant int *ptr, image2d_t image)
{
    // CHECK-NOT: error: variables of type sampler_t must always be initialized
    sampler_t sampler2 = sampler;
    // CHECK: error: sampler_t must always originate from parameters or local variables
    foo((sampler_t) 0, 0); // 7
    // CHECK: sampler_t must always be used as a function argument
    *(int*) sampler = 42; // 8
    // CHECK: sampler_t must always be used as a function argument
    sampler + 0; // 9
    // CHECK: sampler_t must always be used as a function argument
    bar(&sampler); // 11
    // CHECK: sampler_t must always be used as a function argument
    (int) 0 + (int) sampler; // 12
    // CHECK: error: sampler_t must always be used as a function argument
    &sampler;
    // CHECK: error: sampler_t must always be used as a function argument
    ++*&sampler;
}

