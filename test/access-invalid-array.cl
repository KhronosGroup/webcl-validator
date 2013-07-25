// RUN: cat %s | grep -v DRIVER-MAY-REJECT | %opencl-validator
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK-NOT: error: Array index is too small.
// CHECK-NOT: error: Array index is too large.
// CHECK-NOT: error: Invalid array index.

//prototypes for apple driver
int get_incorrectly_indexed_value(const int triple[6], int index);
void set_incorrectly_indexed_value(__global int *array, int index, int value);

int get_incorrectly_indexed_value(const int triple[6], int index)
{
    const int sum1 = triple[0] + triple[1] + triple[2];
    const int sum2 = triple[3] + triple[4] + triple[5];
    // CHECK: error: Array index is too small.
    // CHECK: error: Array index is too small.
    // CHECK: error: Array index is too small.
    const int sum3 = triple[-1] + triple[-2] + triple[-3]; // DRIVER-MAY-REJECT
    const int sum4 = triple[6] + triple[7] + triple[8];
    return sum1 + sum2
        + sum3 // DRIVER-MAY-REJECT
        + sum4;
}

// CHECK-NOT: error: Array index is too small.
// CHECK-NOT: error: Array index is too large.
// CHECK-NOT: error: Invalid array index.

void set_incorrectly_indexed_value(__global int *array, int index, int value)
{
    int triple[3] = { 0, 1, 2 };
    // CHECK: error: Array index is too small.
    // CHECK: error: Array index is too small.
    // CHECK: error: Array index is too small.
    triple[0] = triple[-1] + triple[-2] + triple[-3]; // DRIVER-MAY-REJECT
    // CHECK: error: Array index is too large.
    // CHECK: error: Invalid array index.
    triple[1] = triple[4294967296] + triple[9223372036854775808L]; // DRIVER-MAY-REJECT
    // CHECK: error: Array index is too large.
    // CHECK: error: Array index is too large.
    // CHECK: error: Array index is too large.
    triple[2] = triple[3] + triple[4] + triple[5]; // DRIVER-MAY-REJECT
    array[index] = value + triple[0] + triple[1] + triple[2];
}

// CHECK-NOT: error: Array index is too small.
// CHECK-NOT: error: Array index is too large.
// CHECK-NOT: error: Invalid array index.

__kernel void access_invalid_array(
    __global int *array)
{
    const int i = get_global_id(0);

    const int triple[3] = { 0, 1, 2 };
    const int value = get_incorrectly_indexed_value(triple, i);
    set_incorrectly_indexed_value(array, i, value);
}
