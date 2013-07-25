// RUN: cat %s | grep -v DRIVER-MAY-REJECT | %opencl-validator
// RUN: %webcl-validator %s -- -include %include/_kernel.h 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s -- -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// prototypes for apple driver
int get_indexed_value(__global int *array, int index);
void set_indexed_value(__global int *array, int index, int value);

int get_indexed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *array, int index)
{
    const int triple[3] = { 0, 1, 2 };
    // CHECK: const int sum1 = (*(_WCL_ADDR_global_1(__global int *, (array)+(index), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) + (*(_WCL_ADDR_global_1(__global int *, (array)+(0), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(index), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
    const int sum1 = array[index] + array[0] + triple[index];
    // CHECK: const int sum2 = (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(0), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(1), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(2), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
    const int sum2 = triple[0] + triple[1] + triple[2];
    // CHECK: const int sum3 = (*(_WCL_ADDR_global_1(__global int *, (array)+(index), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) + (*(_WCL_ADDR_global_1(__global int *, (array)+(0), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(index), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
    const int sum3 = index[array] + 0[array] + index[triple]; // DRIVER-MAY-REJECT
    // CHECK: const int sum4 = (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(0), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(1), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1)))) + (*(_WCL_ADDR_private_1(const int *, (_wcl_allocs->pa.get_indexed_value__triple)+(2), &_wcl_allocs->pa, (&_wcl_allocs->pa + 1))))
    const int sum4 = 0[triple] + 1[triple] + 2[triple]; // DRIVER-MAY-REJECT
    return sum1 + sum2
        + sum3 + sum4 // DRIVER-MAY-REJECT
        ;
}

void set_indexed_value(
    // CHECK: _WclProgramAllocations *_wcl_allocs,
    __global int *array, int index, int value)
{
    // CHECK: (*(_WCL_ADDR_global_1(__global int *, (array)+(index), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) += value;
    array[index] += value;
    // CHECK: (*(_WCL_ADDR_global_1(__global int *, (array)+(index), _wcl_allocs->gl.access_array__array_min, _wcl_allocs->gl.access_array__array_max))) += value;
    index[array] += value; // DRIVER-MAY-REJECT
}

__kernel void access_array(
    // CHECK: __global int *array, unsigned long _wcl_array_size)
    __global int *array)
{
    // CHECK: _WclProgramAllocations _wcl_allocations_allocation = {
    // CHECK:     { &array[0], &array[_wcl_array_size] },
    // CHECK:     { { 0, 1, 2 } }
    // CHECK: };

    const int i = get_global_id(0);

    // CHECK: const int indexed_value = get_indexed_value(_wcl_allocs, array, i);
    const int indexed_value = get_indexed_value(array, i);
    // CHECK: set_indexed_value(_wcl_allocs, array, i, -indexed_value);
    set_indexed_value(array, i, -indexed_value);
}
