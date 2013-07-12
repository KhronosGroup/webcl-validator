// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int get_pointed_value(__global int *pointer);
void set_pointed_value(__global int *pointer, int value);

int get_pointed_value(
    // CHECK: WclAddressSpaces *wcl_as,
    __global int *pointer)
{
    // CHECK: return *wcl_global_int_ptr(wcl_as, pointer);
    return *pointer;
}

void set_pointed_value(
    // CHECK: WclAddressSpaces *wcl_as,
    __global int *pointer, int value)
{
    // CHECK: *wcl_global_int_ptr(wcl_as, pointer) = value;
    *pointer = value;
}

__kernel void access_pointer(
    // CHECK: __global int *array, unsigned long wcl_array_size)
    __global int *array)
{
    // CHECK: WclAddressSpaces wcl_as = { 0, 0, 0, 0 };
    const int i = get_global_id(0);

    // CHECK: const int pointed_value = get_pointed_value(wcl_as, array + i);
    const int pointed_value = get_pointed_value(array + i);
    // CHECK: set_pointed_value(wcl_as, array + i, -pointed_value);
    set_pointed_value(array + i, -pointed_value);
}
