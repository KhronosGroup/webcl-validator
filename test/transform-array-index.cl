// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | %FileCheck %s

__kernel void transform_array_index(
    __global int *array)
{
    const int i = get_global_id(0);

    int pair[2] = { 0, 0 };
    // CHECK: pair[(i + 0) % 2UL] = 0;
    pair[i + 0] = 0;
    // CHECK: pair[(i + 1) % 2UL] = 1;
    (i + 1)[pair] = 1;

    // CHECK: array[i] = pair[(i + 0) % 2UL] + pair[(i + 1) % 2UL];
    array[i] = pair[i + 0] + (i + 1)[pair];
}
