// RUN: cat %s | grep -v ^#pragma | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | %FileCheck %s

// We allow only certain extensions to be enabled.

// CHECK-NOT: error: WebCL doesn't support enabling 'cl_khr_fp64' extension.
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_int64_base_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_int64_extended_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
// CHECK-NOT: error: WebCL doesn't support enabling 'cl_khr_fp16' extension.
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_gl_sharing' extension.
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_gl_event' extension.
#pragma OPENCL EXTENSION cl_khr_gl_event : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_d3d10_sharing' extension.
#pragma OPENCL EXTENSION cl_khr_d3d10_sharing : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_global_int32_base_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_global_int32_extended_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_local_int32_base_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_local_int32_extended_atomics' extension.
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_byte_addressable_store' extension.
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
// CHECK: error: WebCL doesn't support enabling 'cl_khr_3d_image_writes' extension.
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

// All extensions can be only disabled.
// CHECK: error: WebCL doesn't support enabling 'all' extension.
#pragma OPENCL EXTENSION all : enable
// CHECK: error: WebCL doesn't support enabling 'there_is_no_such_extension' extension.
#pragma OPENCL EXTENSION there_is_no_such_extension : enable

__kernel void dummy(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
