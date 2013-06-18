// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: typedef struct {
// CHECK: int value;
// CHECK: int array[1];
// CHECK: int private_value;
// CHECK: int private_array[1];
// CHECK: } WclPrivates;

// CHECK: typedef struct {
// CHECK: int local_value;
// CHECK: int local_array[1];
// CHECK: } WclLocals;

// CHECK: typedef struct {
// CHECK: int constant_value;
// CHECK: int constant_array[1];
// CHECK: } WclConstants;

// CHECK: #if 0
// CHECK: __constant int constant_value = 1;
// CHECK: #endif
__constant int constant_value = 1;
__constant int * __constant constant_pointer = &constant_value;
// CHECK: #if 0
// CHECK: __constant int constant_array[1] = { 2 };
// CHECK: #endif
__constant int constant_array[1] = { 2 };

__kernel void relocate_variables(
    // CHECK-NOT: WclAddressSpaces *wcl_as,
    // CHECK: __global int *result, unsigned long wcl_result_size)
    __global int *result)
{
    // CHECK: WclPrivates wcl_ps;
    // CHECK: WclLocals wcl_ls;
    // CHECK: WclConstants wcl_cs;
    // CHECK: WclAddressSpaces wcl_as = { &wcl_ps, &wcl_ls, &wcl_cs, 0 };

    // CHECK: #if 0
    // CHECK: int value = constant_value;
    // CHECK: #endif
    int value = constant_value;
    int *pointer = &value;
    // CHECK: #if 0
    // CHECK: int array[1] = { *pointer };
    // CHECK: #endif
    int array[1] = { *pointer };

    // CHECK: #if 0
    // CHECK: __local int local_value;
    // CHECK: #endif
    __local int local_value;
    local_value = value + 1;
    __local int *local_pointer;
    local_pointer = &local_value;
    // CHECK: #if 0
    // CHECK: __local int local_array[1];
    // CHECK: #endif
    __local int local_array[1];
    local_array[0] = *local_pointer;

    // CHECK: #if 0
    // CHECK: __private int private_value = local_value + 1;
    // CHECK: #endif
    __private int private_value = local_value + 1;
    __private int *private_pointer = &private_value;
    // CHECK: #if 0
    // CHECK: __private int private_array[1] = { *private_pointer };
    // CHECK: #endif
    __private int private_array[1] = { *private_pointer };

    result[value] = array[0] + local_array[0] +
        constant_array[0] + private_array[0];
}
