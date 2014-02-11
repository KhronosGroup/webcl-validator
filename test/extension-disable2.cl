// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

#pragma OPENCL EXTENSION cl_khr_fp64 : disable

// CHECK: error: use of type 'double' requires cl_khr_fp64 extension to be enabled

__kernel void dummy(
    __global double *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
