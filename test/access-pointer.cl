// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int get_pointed_value(__global int *pointer);
void set_pointed_value(__global int *pointer, int value);

int get_pointed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *pointer)
{
    // CHECK: return (*(_WCL_ADDR_CLAMP_global_1(__global int *, (pointer), 1, _wcl_allocs->gl.access_pointer__array_min, _wcl_allocs->gl.access_pointer__array_max, _wcl_allocs->gn)));
    return *pointer;
}

void set_pointed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *pointer, int value)
{
    // CHECK: (*(_WCL_ADDR_CLAMP_global_1(__global int *, (pointer), 1, _wcl_allocs->gl.access_pointer__array_min, _wcl_allocs->gl.access_pointer__array_max, _wcl_allocs->gn))) = value;
    *pointer = value;
}

__kernel void access_pointer(
    // CHECK: __global int *array, ulong _wcl_array_size)
    __global int *array)
{
    // CHECK: _WclProgramAllocations _wcl_allocations_allocation = {
    // CHECK:     { &array[0], &array[_wcl_array_size] }
    // CHECK: };
    // CHECK: _WclProgramAllocations *_wcl_allocs = &_wcl_allocations_allocation;

    const int i = get_global_id(0);

    // CHECK: const int pointed_value = get_pointed_value(_wcl_allocs, array + i);
    const int pointed_value = get_pointed_value(array + i);
    // CHECK: set_pointed_value(_wcl_allocs, array + i, -pointed_value);
    set_pointed_value(array + i, -pointed_value);
}
