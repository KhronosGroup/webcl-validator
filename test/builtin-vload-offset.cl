// RUN: %webcl-validator "%s" | %opencl-validator
// RUN: %webcl-validator "%s" | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^2,3,4,5,0,0,0,0'
// RUN: %webcl-validator "%s" | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^2,3,4,5,0,0,0,0'
// RUN: %webcl-validator "%s" | %kernel-runner --webcl --kernel builtin_wrappers --global float 9 | grep '^2,3,4,5,6,7,8,9,0'

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;
    __global float **ptr = &input;
 
    for (int c = 0; c < 16; ++c) {
        input[c] = c + 1;
    }

    ++input;

    offset = 0;
    float4 r1 = vload4(offset, (*ptr));

    offset = 1;
    float4 r2 = vload4(offset, (*ptr));

    offset = 2;
    float4 r3 = vload4(offset, (*ptr));

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
