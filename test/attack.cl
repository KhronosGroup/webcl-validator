// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void attack(__global int *array)
{
    const int i = get_global_id(0);
    // CHECK: error:
#ifdef __ONLY_DEFINED_IN_OPENCL_DRIVER__
    _wcl_allocs = 0;
#endif
    array[i] = i;
}
