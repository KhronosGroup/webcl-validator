// RUN: %webcl-validator %s --disable=cl_khr_fp16 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK-DAG: error: WebCL or platform doesn't support enabling 'cl_khr_fp16' extension.

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void dummy()
{
// CHECK: error: variable has incomplete type 'error_undefined_type_half' (aka 'struct error_undefined_type_half')
    half a;
}
