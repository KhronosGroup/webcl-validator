// RUN: %opencl-validator < "%s"
// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// The intention was to write OpenCL code, but the file extension
// suggests C code instead.

// CHECK-NOT: fatal
// CHECK-NOT: error

__kernel void dummy(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
