// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK-NOT: Pointer access needs to be checked.

int get_pointed_value(__global int *pointer)
{
    // CHECK: Pointer access needs to be checked.
    return *pointer;
}

// CHECK-NOT: Pointer access needs to be checked.

void set_pointed_value(__global int *pointer, int value)
{
    // CHECK: Pointer access needs to be checked.
    *pointer = value;
}

// CHECK-NOT: Pointer access needs to be checked.

__kernel void access_pointer(
    __global int *array)
{
    const int i = get_global_id(0);

    const int pointed_value = get_pointed_value(array + i);
    set_pointed_value(array + i, -pointed_value);
}
