// => General code that doesn't depend on input.

// POCL crashes at run time if the parameters are local character
// pointers.
typedef int _WclInitType;

#define _WCL_LOCAL_RANGE_INIT(begin, end)       \
    _wcl_local_range_init((__local _WclInitType *)begin, (__local _WclInitType *)end);

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
        *item_i = '\0';

    const size_t loop_elements = xyz_items * item_elements;
    __local char *item_final = start + (loop_elements + item_index);
    if (item_final < stop)
        *item_final = '\0';
}

// <= General code that doesn't depend on input.
