// RUN: cat %s | grep -v ^#pragma | %opencl-validator
// RUN: %webcl-validator %s -- -include %include/_kernel.h

// All extensions are disabled by default in OpenCL. We don't complain
// if extensions are disabled explicitly afterwards.

#pragma OPENCL EXTENSION cl_khr_fp64 : disable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : disable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : disable
#pragma OPENCL EXTENSION cl_khr_fp16 : disable
#pragma OPENCL EXTENSION cl_khr_gl_sharing : disable
#pragma OPENCL EXTENSION cl_khr_gl_event : disable
#pragma OPENCL EXTENSION cl_khr_d3d10_sharing : disable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : disable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : disable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : disable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : disable
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : disable
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : disable
#pragma OPENCL EXTENSION all : disable

__kernel void dummy(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
