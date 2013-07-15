// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int get_pointed_value(__global int *pointer);
void set_pointed_value(__global int *pointer, int value);

int get_pointed_value(
    // CHECK: WclProgramAllocations *wcl_allocs,
    __global int *pointer)
{
    // CHECK: return (*(WCL_ADDR_global_1(__global int *, (pointer), wcl_allocs->gl.access_pointer__array_min,wcl_allocs->gl.access_pointer__array_max)));
    return *pointer;
}

void set_pointed_value(
    // CHECK: WclProgramAllocations *wcl_allocs,
    __global int *pointer, int value)
{
    // CHECK: (*(WCL_ADDR_global_1(__global int *, (pointer), wcl_allocs->gl.access_pointer__array_min,wcl_allocs->gl.access_pointer__array_max))) = value;
    *pointer = value;
}

__kernel void access_pointer(
    // CHECK: __global int *array, unsigned long wcl_array_size)
    __global int *array)
{
    // CHECK: WclProgramAllocations wcl_allocations_allocation = {
    // CHECK:     { &array[0],&array[wcl_array_size] }    };
    // CHECK: WclProgramAllocations *wcl_allocs = &wcl_allocations_allocation;

    const int i = get_global_id(0);

    // CHECK: const int pointed_value = get_pointed_value(wcl_allocs, array + i);
    const int pointed_value = get_pointed_value(array + i);
    // CHECK: set_pointed_value(wcl_allocs, array + i, -pointed_value);
    set_pointed_value(array + i, -pointed_value);
}
