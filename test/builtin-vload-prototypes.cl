// Check that the vload functions we declare on demand have the correct parameter lists
// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// We should be declaring all builtins at the moment
// CHECK-NOT: warning: implicit declaration of function

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;

    // CHECK: error: no matching function for call to 'vload4'
    vload4(input, offset);

    // CHECK: error: no matching function for call to 'vload4'
    vload4(input);
}
