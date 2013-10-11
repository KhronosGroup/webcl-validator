// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;

    // CHECK: error: vload4 argument number 2 must be a pointer
    vload4(input, offset);

    // CHECK: error: Builtin argument check is required.
    vload4(input);
}
