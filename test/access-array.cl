// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | %FileCheck %s

// CHECK-NOT: warning: Array size not known until run-time.
// CHECK-NOT: warning: Index value not known until run-time.

int get_indexed_value(__global int *array, int index)
{
    const int triple[3] = { 0, 1, 2 };
    // array[index]:
    // CHECK: warning: Array size not known until run-time.
    // CHECK: warning: Index value not known until run-time.
    // array[0]:
    // CHECK: warning: Array size not known until run-time.
    // triple[index]:
    // CHECK: warning: Index value not known until run-time.
    const int sum1 = array[index] + array[0] + triple[index];
    const int sum2 = triple[0] + triple[1] + triple[2];
    // index[array]:
    // CHECK: warning: Array size not known until run-time.
    // CHECK: warning: Index value not known until run-time.
    // 0[array]:
    // CHECK: warning: Array size not known until run-time.
    // index[triple]:
    // CHECK: warning: Index value not known until run-time.
    const int sum3 = index[array] + 0[array] + index[triple];
    const int sum4 = 0[triple] + 1[triple] + 2[triple];
    return sum1 + sum2 + sum3 + sum4;
}

// CHECK-NOT: warning: Array size not known until run-time.
// CHECK-NOT: warning: Index value not known until run-time.

void set_indexed_value(__global int *array, int index, int value)
{
    // array[index]:
    // CHECK: warning: Array size not known until run-time.
    // CHECK: warning: Index value not known until run-time.
    array[index] += value;
    // index[array]:
    // CHECK: warning: Array size not known until run-time.
    // CHECK: warning: Index value not known until run-time.
    index[array] += value;
}

// CHECK-NOT: warning: Array size not known until run-time.
// CHECK-NOT: warning: Index value not known until run-time.

__kernel void access_array(
    __global int *array)
{
    const int i = get_global_id(0);

    const int indexed_value = get_indexed_value(array, i);
    set_indexed_value(array, i, -indexed_value);
}
