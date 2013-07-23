// RUN: cat %s | grep -v DRIVER-MAY-REJECT | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

__kernel void transform_array_index(
    // CHECK: __global int *array, unsigned long _wcl_array_size)
    __global int *array)
{
    const int i = get_global_id(0);

    int pair[2] = { 0, 0 };
    // CHECK: (*(_WCL_ADDR_private_1(int *, (_wcl_allocs->pa.transform_array_index__pair)+(i + 0), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) = 0;
    pair[i + 0] = 0;
    // CHECK: (*(_WCL_ADDR_private_1(int *, (_wcl_allocs->pa.transform_array_index__pair)+((i + 1)), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) = 1;
    (i + 1)[pair] = 1; // DRIVER-MAY-REJECT

    // CHECK: (*(_WCL_ADDR_global_1(__global int *, (array)+(i), _wcl_allocs->gl.transform_array_index__array_min, _wcl_allocs->gl.transform_array_index__array_max))) = (*(_WCL_ADDR_private_1(int *, (_wcl_allocs->pa.transform_array_index__pair)+(i + 0), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
    array[i] = pair[i + 0]
    // CHECK: + (*(_WCL_ADDR_private_1(int *, (_wcl_allocs->pa.transform_array_index__pair)+((i + 1)), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
        + (i + 1)[pair] // DRIVER-MAY-REJECT
        ;
}
