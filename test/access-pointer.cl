// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

int get_pointed_value(__global int *pointer)
{
    // CHECK: return *wcl_global_int_ptr(NULL, pointer);
    return *pointer;
}

void set_pointed_value(__global int *pointer, int value)
{
    // CHECK: *wcl_global_int_ptr(NULL, pointer) = value;
    *pointer = value;
}

__kernel void access_pointer(
    __global int *array)
{
    const int i = get_global_id(0);

    const int pointed_value = get_pointed_value(array + i);
    set_pointed_value(array + i, -pointed_value);
}
