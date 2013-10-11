// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s
// RUN: %webcl-validator %s | %kernel-runner --webcl --kernel builtin_wrappers --global float 7 | grep '^1,2,3,4,0,0,0,0,0'
// RUN: %webcl-validator %s | %kernel-runner --webcl --kernel builtin_wrappers --global float 8 | grep '^1,2,3,4,5,6,7,8,0'

__kernel void builtin_wrappers(__global char *output, 
                               __global float *input)
{
    __local int offset;
    __global float **ptr = &input;
 
    for (int c = 0; c < 16; ++c) {
        input[c] = c + 1;
    }

    offset = 0;
    // CHECK: float4 r1 = _wcl_vload4_0(_wcl_allocs, _wcl_locals._wcl_offset, ((*(_WCL_ADDR_CLAMP_private_1(__global float **, (_wcl_allocs->pa._wcl_ptr), 1, &_wcl_allocs->pa, (&_wcl_allocs->pa + 1), _wcl_allocs->pn)))));
    float4 r1 = vload4(offset, (*ptr));

    offset = 1;
    // CHECK: float4 r2 = _wcl_vload4_1(_wcl_allocs, _wcl_locals._wcl_offset, ((*(_WCL_ADDR_CLAMP_private_1(__global float **, (_wcl_allocs->pa._wcl_ptr), 1, &_wcl_allocs->pa, (&_wcl_allocs->pa + 1), _wcl_allocs->pn)))));
    float4 r2 = vload4(offset, (*ptr));

    offset = 2;
    // CHECK: float4 r3 = _wcl_vload4_2(_wcl_allocs, _wcl_locals._wcl_offset, ((*(_WCL_ADDR_CLAMP_private_1(__global float **, (_wcl_allocs->pa._wcl_ptr), 1, &_wcl_allocs->pa, (&_wcl_allocs->pa + 1), _wcl_allocs->pn)))));
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
