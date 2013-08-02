// => General code that doesn't depend on input.

#define _WCL_LAST(type, ptr) (((type)(ptr)) - 1)
#define _WCL_FILLCHAR ((char)0xCC)

// POCL crashes at run time if the parameters are local character
// pointers.
typedef int _WclInitType;

#ifdef cl_khr_initialize_memory
#pragma OPENCL EXTENSION cl_khr_initialize_memory : enable
#define _WCL_LOCAL_RANGE_INIT(begin, end)
#else

#define _WCL_LOCAL_RANGE_INIT(begin, end)                               \
    _wcl_local_range_init((__local _WclInitType *)begin, (__local _WclInitType *)end)

void _wcl_local_range_init(__local _WclInitType *begin, __local _WclInitType *end)
{
    __local char *start = (__local char *)begin;
    __local char *stop = (__local char *)end;

    const size_t z_items = get_local_size(2);
    const size_t yz_items = get_local_size(1) * z_items;
    const size_t xyz_items = get_local_size(0) * yz_items;

    const size_t item_index =
        (get_local_id(0) * yz_items) +
        (get_local_id(1) * z_items) +
        get_local_id(2);
    const size_t num_elements = stop - start;
    const size_t item_elements = num_elements / xyz_items;

    __local char *item_begin = start + (item_index * item_elements);
    __local const char *item_end = item_begin + item_elements;
    for (__local char *item_i = item_begin; item_i < item_end; ++item_i)
        *item_i = _WCL_FILLCHAR;

    const size_t loop_elements = xyz_items * item_elements;
    __local char *item_final = start + (loop_elements + item_index);
    if (item_final < stop)
        *item_final = _WCL_FILLCHAR;
}

#endif // cl_khr_initialize_memory

// <= General code that doesn't depend on input.
