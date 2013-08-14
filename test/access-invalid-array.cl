// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -vE "(CHECK|Processing)" | %opencl-validator

//prototypes for apple driver
int get_incorrectly_indexed_value(const int triple[6], int index);
void set_incorrectly_indexed_value(__global int *array, int index, int value);

int get_incorrectly_indexed_value(const int triple[6], int index)
{
    const int sum1 = triple[0] + triple[1] + triple[2];
    const int sum2 = triple[3] + triple[4] + triple[5];
#ifndef __PLATFORM_AMD__
    const int sum3 = triple[-1] + triple[-2] + triple[-3];
#endif
    const int sum4 = triple[6] + triple[7] + triple[8];
    return sum1 + sum2
#ifndef __PLATFORM_AMD__
        + sum3
#endif
        + sum4;
}

void set_incorrectly_indexed_value(__global int *array, int index, int value)
{
    int triple[3] = { 0, 1, 2 };
#ifndef __PLATFORM_AMD__
    triple[0] = triple[-1] + triple[-2] + triple[-3];
#endif
#ifndef __PLATFORM_AMD__
    triple[1] = triple[4294967296] + triple[9223372036854775808L];
#endif
#ifndef __PLATFORM_AMD__
    triple[2] = triple[3] + triple[4] + triple[5];
#endif
    array[index] = value + triple[0] + triple[1] + triple[2];
}

__kernel void access_invalid_array(
    __global int *array)
{
    const int i = get_global_id(0);

    const int triple[3] = { 0, 1, 2 };
    const int value = get_incorrectly_indexed_value(triple, i);
    set_incorrectly_indexed_value(array, i, value);
}
