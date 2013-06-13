// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: #if 0
// CHECK: __constant int constant_value = 1;
// CHECK: #endif
__constant int constant_value = 1;
__constant int * __constant constant_pointer = &constant_value;
__constant int constant_array[1] = { 2 };

__kernel void relocate_variables(
    // CHECK-NOT: WclAddressSpaces *wcl_as,
    // CHECK: __global int *result, unsigned long wcl_result_size)
    __global int *result)
{
    // CHECK: #if 0
    // CHECK: int value = constant_value;
    // CHECK: #endif
    int value = constant_value;
    int *pointer = &value;
    int array[1] = { *pointer };

    // CHECK: #if 0
    // CHECK: __local int local_value;
    // CHECK: #endif
    __local int local_value;
    local_value = value + 1;
    __local int *local_pointer;
    local_pointer = &local_value;
    __local int local_array[1];
    local_array[0] = *local_pointer;

    // CHECK: #if 0
    // CHECK: __private int private_value = local_value + 1;
    // CHECK: #endif
    __private int private_value = local_value + 1;
    __private int *private_pointer = &private_value;
    __private int private_array[1] = { *private_pointer };

    result[value] = array[0] + local_array[0] +
        constant_array[0] + private_array[0];
}
