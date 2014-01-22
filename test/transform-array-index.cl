// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

__kernel void transform_array_index(
    // CHECK: __global int *array, ulong _wcl_array_size)
    __global int *array)
{
    const int i = get_global_id(0);

    int pair[2] = { 0, 0 };
    // CHECK: (*(_wcl_addr_clamp_private_1_int__Ptr((_wcl_allocs->pa._wcl_pair)+(i + 0), 1, (int *)&_wcl_allocs->pa, (int *)(&_wcl_allocs->pa + 1), (int *)_wcl_allocs->pn))) = 0;
    pair[i + 0] = 0;
    // CHECK: (*(_wcl_addr_clamp_private_1_int__Ptr((_wcl_allocs->pa._wcl_pair)+((i + 1)), 1, (int *)&_wcl_allocs->pa, (int *)(&_wcl_allocs->pa + 1), (int *)_wcl_allocs->pn))) = 1;
#ifndef __PLATFORM_AMD__
    (i + 1)[pair] = 1;
#endif

    // CHECK: (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(i), 1, (__global int *)_wcl_allocs->gl.transform_array_index__array_min, (__global int *)_wcl_allocs->gl.transform_array_index__array_max, (__global int *)_wcl_allocs->gn))) = (*(_wcl_addr_clamp_private_1_int__Ptr((_wcl_allocs->pa._wcl_pair)+(i + 0), 1, (int *)&_wcl_allocs->pa, (int *)(&_wcl_allocs->pa + 1), (int *)_wcl_allocs->pn)))
    array[i] = pair[i + 0]
    // CHECK: + (*(_wcl_addr_clamp_private_1_int__Ptr((_wcl_allocs->pa._wcl_pair)+((i + 1)), 1, (int *)&_wcl_allocs->pa, (int *)(&_wcl_allocs->pa + 1), (int *)_wcl_allocs->pn)))
#ifndef __PLATFORM_AMD__
        + (i + 1)[pair]
#endif
        ;
}
