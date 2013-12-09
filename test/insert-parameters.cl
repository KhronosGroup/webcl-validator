// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int no_parameters(void);
int value_parameters(int index);
int unused_parameters(__global int *global_array, __local int *local_array, __constant int *constant_array, __private int *private_array);
int used_parameters(__global int *global_array, __local int *local_array, __constant int *constant_array, __private int *private_array);

// CHECK: int no_parameters(_WclProgramAllocations *_wcl_allocs)
int no_parameters(void)
{
    return 0;
}

int value_parameters(
    // CHECK: _WclProgramAllocations *_wcl_allocs, int index)
    int index)
{
    return index + 1;
}

int unused_parameters(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    // CHECK: __global int *global_array, __local int *local_array,
    __global int *global_array, __local int *local_array,
    // CHECK: __constant int *constant_array, __private int *private_array)
    __constant int *constant_array, __private int *private_array)
{
    return 0;
}

int used_parameters(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    // CHECK: __global int *global_array, __local int *local_array,
    __global int *global_array, __local int *local_array,
    // CHECK: __constant int *constant_array, __private int *private_array)
    __constant int *constant_array, __private int *private_array)
{
    const int index = value_parameters(no_parameters());
    return global_array[index] + local_array[index] +
        constant_array[index] + private_array[index];
}

// CHECK: __kernel void insert_parameters(__global int *global_array, ulong _wcl_global_array_size, __local int *local_array, ulong _wcl_local_array_size, __constant int *constant_array, ulong _wcl_constant_array_size)
__kernel void insert_parameters(__global int *global_array, __local int *local_array, __constant int *constant_array)
{
    const int i = get_global_id(0);

    int private_array[2] = { 0 };

    global_array[i + 0] = unused_parameters(global_array, local_array,
                                            constant_array, private_array);
    global_array[i + 1] = used_parameters(global_array, local_array,
                                          constant_array, private_array);
}
