// RUN: cat %s | grep -v DRIVER-MAY-REJECT %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

int get_indexed_value(__global int *array, int index)
{
    const int triple[3] = { 0, 1, 2 };
    // CHECK: const int sum1 = array[wcl_global_int_idx(NULL, array, index)] + array[wcl_global_int_idx(NULL, array, 0)] + triple[wcl_idx(index, 3UL)];
    const int sum1 = array[index] + array[0] + triple[index];
    // CHECK: const int sum2 = triple[0] + triple[1] + triple[2];
    const int sum2 = triple[0] + triple[1] + triple[2];
    // CHECK: const int sum3 = array[wcl_global_int_idx(NULL, array, index)] + array[wcl_global_int_idx(NULL, array, 0)] + triple[wcl_idx(index, 3UL)];
    const int sum3 = index[array] + 0[array] + index[triple]; // DRIVER-MAY-REJECT
    // CHECK: const int sum4 = 0[triple] + 1[triple] + 2[triple];
    const int sum4 = 0[triple] + 1[triple] + 2[triple]; // DRIVER-MAY-REJECT
    return sum1 + sum2
        + sum3 + sum4 // DRIVER-MAY-REJECT
        ;
}

void set_indexed_value(__global int *array, int index, int value)
{
    // CHECK: array[wcl_global_int_idx(NULL, array, index)] += value;
    array[index] += value;
    // CHECK: array[wcl_global_int_idx(NULL, array, index)] += value;
    index[array] += value; // DRIVER-MAY-REJECT
}

__kernel void access_array(
    __global int *array)
{
    const int i = get_global_id(0);

    const int indexed_value = get_indexed_value(array, i);
    set_indexed_value(array, i, -indexed_value);
}
