// RUN: sed 's/@SIZE@/4/g' %s | %opencl-validator
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %opencl-validator
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | grep -v CHECK | %FileCheck %s
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^1,2,3,4,0,0,0,0,0'
// RUN: sed 's/@SIZE@/4/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^1,2,3,4,5,6,7,8,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^0,0,0,0,0,0,0,0,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^1,2,3,4,0,0,0,0,0'
// RUN: sed 's/@SIZE@/8/g' %s | %webcl-validator - | %kernel-runner --webcl --kernel builtin_wrappers --global float 16 | grep '^1,2,3,4,9,10,11,12'

// We should be declaring all builtins at the moment
// CHECK-NOT: warning: implicit declaration of function

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;

    for (int c = 0; c < 16; ++c) {
        input[c] = c + 1;
    }

    offset = 0;
    // CHECK: float4 r1 = _wcl_vload4_0(_wcl_allocs, _wcl_locals._wcl_offset, input);
    float@SIZE@ r1 = vload@SIZE@(offset, input);

    offset = 1;
    // CHECK: float4 r2 = _wcl_vload4_1(_wcl_allocs, _wcl_locals._wcl_offset, input);
    float@SIZE@ r2 = vload@SIZE@(offset, input);

    offset = 2;
    // CHECK: float4 r3 = _wcl_vload4_2(_wcl_allocs, _wcl_locals._wcl_offset, input);
    float@SIZE@ r3 = vload@SIZE@(offset, input);

    output[0] = r1.x;
    output[1] = r1.y;
    output[2] = r1.z;
    output[3] = r1.w;

    output[4] = r2.x;
    output[5] = r2.y;
    output[6] = r2.z;
    output[7] = r2.w;

    output[8] = r3.x;
    output[9] = r3.y;
    output[10] = r3.z;
    output[11] = r3.w;
}
