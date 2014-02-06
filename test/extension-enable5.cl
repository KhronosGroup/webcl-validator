// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// do not enable halfs
//#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void dummy()
{
    // CHECK: error: variable has incomplete type 'error_undefined_type_half'
    half a;
}
