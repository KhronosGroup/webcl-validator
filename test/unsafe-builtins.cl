// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// We should be declaring all builtins at the moment
// CHECK-NOT: warning: implicit declaration of function

__kernel void unsafe_builtins(
    float x,
    __constant float *input,
    __global float *output)
{
    float x_floor;
    // CHECK: error: Builtin argument check is required.
    float x_fract = fract(x, &x_floor);
}
