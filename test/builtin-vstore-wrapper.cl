// RUN: sed 's/@SIZE@/4/g' %s | %opencl-validator
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %opencl-validator
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | grep -v CHECK | %FileCheck %s
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^1,2,3,4,-1,-1,-1,0'
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^1,2,3,4,1,2,3,4,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^-1,-1,-1,-1,-1,-1,-1,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^1,2,3,4,5,6,7,8,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 16 | grep '^1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8,0'

// We should be declaring all builtins at the moment
// CHECK-NOT: warning: implicit declaration of function

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;

    for (int c = 0; c < 16; ++c) {
        input[c] = c + 1;
    }

    float@SIZE@ value = vload@SIZE@(0, input);

    for (int c = 0; c < 16; ++c) {
        input[c] = -1;
    }

    offset = 0;
    // CHECK-DAG: _wcl_vstore4_{{[0-3]}}(_wcl_allocs, value, _wcl_locals._wcl_offset, input);
    vstore@SIZE@(value, offset, input);

    offset = 1;
    // CHECK-DAG: _wcl_vstore4_{{[0-3]}}(_wcl_allocs, value, _wcl_locals._wcl_offset, input);
    vstore@SIZE@(value, offset, input);

    offset = 2;
    // CHECK-DAG: _wcl_vstore4_{{[0-3]}}(_wcl_allocs, value, _wcl_locals._wcl_offset, input);
    vstore@SIZE@(value, offset, input);

    for (int c = 0; c < 16; ++c) {
        output[c] = input[c];
    }
}
