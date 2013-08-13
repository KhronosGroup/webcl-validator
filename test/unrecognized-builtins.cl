// RUN: cat %include/unsafe.h %s | %opencl-validator
// RUN: %webcl-validator %s -I%include -include unsafe.h 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void unrecognized_builtins(
    __constant int *input,
    __global int *output)
{
    const size_t i = get_global_id(0);

    // CHECK-NOT: warning: implicit declaration of function 'unsafe_function'
    // CHECK: error: Unsafe builtin not recognized.
    output[i] = unsafe_function(input, i);
}
