// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// The intention was to write OpenCL code, but the file extension
// suggests C code instead, and the language wasn't forced to OpenCL.
// CHECK: fatal error: Source file '{{[^']*}}' isn't treated as OpenCL code.

__kernel void dummy(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
