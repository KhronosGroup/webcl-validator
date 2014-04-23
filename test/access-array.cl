// RUN: %opencl-validator < "%s"
// RUN: %webcl-validator "%s" | %opencl-validator
// RUN: %webcl-validator "%s" | grep -v CHECK | %FileCheck "%s"

// prototypes for apple driver
int get_indexed_value(__global int *array, int index);
void set_indexed_value(__global int *array, int index, int value);

int get_indexed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *array, int index)
{
    const int triple[3] = { 0, 1, 2 };
    // CHECK: const int sum1 = (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(index), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) + (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(0), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(index), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn)));
    const int sum1 = array[index] + array[0] + triple[index];
    // CHECK: const int sum2 = (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(0), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(1), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(2), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn)));
    const int sum2 = triple[0] + triple[1] + triple[2];
    // CHECK: const int sum3 = (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(index), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) + (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(0), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(index), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn)));
#ifndef __PLATFORM_AMD__
    const int sum3 = index[array] + 0[array] + index[triple];
#endif
    // CHECK: const int sum4 = (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(0), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(1), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn))) + (*(_wcl_addr_clamp_private_1_const__int__Ptr((_wcl_allocs->pa._wcl_triple)+(2), 1, (const int *)&_wcl_allocs->pa, (const int *)(&_wcl_allocs->pa + 1), (const int *)_wcl_allocs->pn)));
#ifndef __PLATFORM_AMD__
    const int sum4 = 0[triple] + 1[triple] + 2[triple];
#endif
    return sum1 + sum2
#ifndef __PLATFORM_AMD__
        + sum3 + sum4
#endif
        ;
}

void set_indexed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *array, int index, int value)
{
    // CHECK: (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(index), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) += value;
    array[index] += value;
    // CHECK: (*(_wcl_addr_clamp_global_1__u_uglobal__int__Ptr((array)+(index), 1, (__global int *)_wcl_allocs->gl.access_array__array_min, (__global int *)_wcl_allocs->gl.access_array__array_max, (__global int *)_wcl_allocs->gn))) += value;
#ifndef __PLATFORM_AMD__
    index[array] += value;
#endif
}

__kernel void access_array(
    // CHECK: __global int *array, ulong _wcl_array_size)
    __global int *array)
{
    // CHECK: _WclProgramAllocations _wcl_allocations_allocation = {
    // CHECK:     { &array[0], &array[_wcl_array_size] },
    // CHECK:      0,
    // CHECK:     { }
    // CHECK: };

    const int i = get_global_id(0);

    // CHECK: const int indexed_value = get_indexed_value(_wcl_allocs, array, i);
    const int indexed_value = get_indexed_value(array, i);
    // CHECK: set_indexed_value(_wcl_allocs, array, i, -indexed_value);
    set_indexed_value(array, i, -indexed_value);
}
