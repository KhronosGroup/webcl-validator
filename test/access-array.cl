// RUN: cat %s | grep -v DRIVER-MAY-REJECT %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int get_indexed_value(__global int *array, int index);
void set_indexed_value(__global int *array, int index, int value);

int get_indexed_value(
    // CHECK: WclAddressSpaces *wcl_as,
    __global int *array, int index)
{
    // CHECK: #if 0
    // CHECK: const int triple[3] = { 0, 1, 2 };
    // CHECK: #endif
    const int triple[3] = { 0, 1, 2 };
    // CHECK: const int sum1 = array[wcl_global_int_idx(wcl_as, array, index)] + array[wcl_global_int_idx(wcl_as, array, 0)] + triple[wcl_idx(index, 3UL)];
    const int sum1 = array[index] + array[0] + triple[index];
    // CHECK: const int sum2 = triple[0] + triple[1] + triple[2];
    const int sum2 = triple[0] + triple[1] + triple[2];
    // CHECK: const int sum3 = array[wcl_global_int_idx(wcl_as, array, index)] + array[wcl_global_int_idx(wcl_as, array, 0)] + triple[wcl_idx(index, 3UL)];
    const int sum3 = index[array] + 0[array] + index[triple]; // DRIVER-MAY-REJECT
    // CHECK: const int sum4 = 0[triple] + 1[triple] + 2[triple];
    const int sum4 = 0[triple] + 1[triple] + 2[triple]; // DRIVER-MAY-REJECT
    return sum1 + sum2
        + sum3 + sum4 // DRIVER-MAY-REJECT
        ;
}

void set_indexed_value(
    // CHECK: WclAddressSpaces *wcl_as,
    __global int *array, int index, int value)
{
    // CHECK: array[wcl_global_int_idx(wcl_as, array, index)] += value;
    array[index] += value;
    // CHECK: array[wcl_global_int_idx(wcl_as, array, index)] += value;
    index[array] += value; // DRIVER-MAY-REJECT
}

__kernel void access_array(
    // CHECK: __global int *array, unsigned long wcl_array_size)
    __global int *array)
{
    // CHECK: WclPrivates wcl_ps = {
    // CHECK:     { 0, 1, 2 }
    // CHECK: };
    // CHECK: WclAddressSpaces wcl_as = { &wcl_ps, 0, 0, 0 };
    const int i = get_global_id(0);

    // CHECK: const int indexed_value = get_indexed_value(wcl_as, array, i);
    const int indexed_value = get_indexed_value(array, i);
    // CHECK: set_indexed_value(wcl_as, array, i, -indexed_value);
    set_indexed_value(array, i, -indexed_value);
}
