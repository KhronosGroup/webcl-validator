// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void foo(sampler_t sampler, int i)
{
    // CHECK: error: used type 'sampler_t' where arithmetic or pointer type is required
    nonexisting((sampler_t) i); // 2
}

// CHECK-NOT: error: sampler_t must always be used as a function argument of a variable initializer
__kernel void sampler_type_errors1(sampler_t sampler, __constant int *ptr, image2d_t image)
{
    // CHECK-NOT: error: variables of type sampler_t must always be initialized
    sampler_t sampler2 = sampler;
    // CHECK: error: used type 'sampler_t' where arithmetic or pointer type is required
    foo((sampler_t) 0, 0); // 7
    // CHECK: error: operand of type 'sampler_t' where arithmetic or pointer type is required
    *(int*) sampler = 42; // 8
    // CHECK: error: invalid operands to binary expression ('sampler_t' and 'int')
    sampler + 0; // 9
    // CHECK: error: operand of type 'sampler_t' where arithmetic or pointer type is required
    (int) 0 + (int) sampler; // 12
    // CHECK: error: cannot increment value of type 'sampler_t'
    ++*&sampler;
}

