// => General code that doesn't depend on input.

#define _WCL_MEMCPY(dst, src) for(ulong i = 0; i < sizeof((src))/sizeof((src)[0]); i++) { (dst)[i] = (src)[i]; }

#define _WCL_LAST(type, ptr) (((type)(ptr)) - 1)
#define _WCL_FILLCHAR ((uchar)0xCC)

// POCL crashes at run time if the parameters are local character
// pointers.
typedef uint _WclInitType;

// NOTE: this expects that null pointer is type of uint*
#define _WCL_SET_NULL(type, req_bytes, min, max, null) ( ((((type)max)-((type)min))*sizeof(uint) >= req_bytes) ? ((type)min) : (null) )

#ifdef cl_khr_initialize_memory
#pragma OPENCL EXTENSION cl_khr_initialize_memory : enable
#define _WCL_LOCAL_RANGE_INIT(begin, end)
#else

// be careful to edit this, this has been carefully tuned to work on every OpenCL driver
// e.g. % item_count was added to start[(items_offset+i)] = _WCL_FILLCHAR;
// to prevent compiler crash on Apple GeForce 640M
#define _WCL_LOCAL_RANGE_INIT(begin, end) do {               \
    __local uchar *start = (__local uchar *)begin;           \
    __local uchar *stop = (__local uchar *)end;              \
    const size_t z_items = get_local_size(2);                \
    const size_t yz_items = get_local_size(1) * z_items;     \
    const size_t xyz_items = get_local_size(0) * yz_items;   \
    const size_t item_index =                                \
        (get_local_id(0) * yz_items) +                       \
        (get_local_id(1) * z_items) +                        \
        get_local_id(2);                                     \
    size_t item_count = stop - start;                        \
    size_t items_per_kernel = item_count / xyz_items;        \
    size_t items_offset = items_per_kernel * item_index;     \
    size_t reminders = item_count % xyz_items;               \
    if (item_index < reminders) {                            \
        start[xyz_items*items_per_kernel + item_index] = _WCL_FILLCHAR; \
    }                                                                   \
    for (size_t i = 0; i < items_per_kernel; i++) {                     \
        start[(items_offset+i) % item_count] = _WCL_FILLCHAR;           \
    }                                                                   \
} while (0)                                                             \

#endif // cl_khr_initialize_memory

constant int hd4k_workaround_constant = 0;

// <= General code that doesn't depend on input.
