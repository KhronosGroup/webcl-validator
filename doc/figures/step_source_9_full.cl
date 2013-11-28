/* WebCL Validator JSON header
{
    "version" : "1.0",
    "kernels" :
        {
            "foo_kernel" :
                {
                    "in" :
                        {
                            "index" : 0,
                            "host-type" : "cl_mem",
                            "host-element-type" : "cl_float",
                            "address-space" : "global",
                            "size-parameter" : "_wcl_in_size"
                        },
                    "_wcl_in_size" :
                        {
                            "index" : 1,
                            "host-type" : "cl_ulong"
                        },
                    "out" :
                        {
                            "index" : 2,
                            "host-type" : "cl_mem",
                            "host-element-type" : "cl_float4",
                            "address-space" : "global",
                            "size-parameter" : "_wcl_out_size"
                        },
                    "_wcl_out_size" :
                        {
                            "index" : 3,
                            "host-type" : "cl_ulong"
                        }
                }
        }
}
*/

#define _WCL_ADDRESS_SPACE_private_MIN (((32 + (CHAR_BIT - 1)) / CHAR_BIT))
#define _WCL_ADDRESS_SPACE_private_ALIGNMENT (32/CHAR_BIT)
#define _WCL_ADDRESS_SPACE_global_MIN (((128 + (CHAR_BIT - 1)) / CHAR_BIT))
#define _WCL_ADDRESS_SPACE_global_ALIGNMENT (128/CHAR_BIT)
#define _WCL_ADDRESS_SPACE_local_MIN (((8 + (CHAR_BIT - 1)) / CHAR_BIT))
#define _WCL_ADDRESS_SPACE_local_ALIGNMENT (8/CHAR_BIT)
#define _WCL_ADDRESS_SPACE_constant_MIN (((8 + (CHAR_BIT - 1)) / CHAR_BIT))
#define _WCL_ADDRESS_SPACE_constant_ALIGNMENT (8/CHAR_BIT)

struct _Wcl2Struct {
    int a;
};

struct _WclStruct {
    struct _Wcl2Struct b;
};

typedef struct {
    struct _WclStruct _wcl_c;
    int _wcl_index;
} __attribute__ ((aligned (_WCL_ADDRESS_SPACE_private_ALIGNMENT))) _WclPrivates;

typedef struct {
    __global float *foo_kernel__in_min;
    __global float *foo_kernel__in_max;
    __global float4 *foo_kernel__out_min;
    __global float4 *foo_kernel__out_max;
} _WclGlobalLimits;

typedef struct {
    _WclGlobalLimits gl;
    __global uint *gn;
    _WclPrivates pa;
    __private uint *pn;
} _WclProgramAllocations;

__constant uint _wcl_constant_null[_WCL_ADDRESS_SPACE_constant_MIN] = { 0 };

// => General code that doesn't depend on input.

#define _WCL_MEMCPY(dst, src) \
    for(ulong i = 0; i < sizeof((src))/sizeof((src)[0]); i++) {\
        (dst)[i] = (src)[i];\
    }

#define _WCL_LAST(type, ptr) (((type)(ptr)) - 1)
#define _WCL_FILLCHAR ((uchar)0xCC)

typedef uint _WclInitType;

#define _WCL_SET_NULL(type, req_bytes, min, max, null) \
( ((((type)max)-((type)min))*sizeof(uint) >= req_bytes) ? ((type)min) : (null) )

#ifdef cl_khr_initialize_memory
#pragma OPENCL EXTENSION cl_khr_initialize_memory : enable
#define _WCL_LOCAL_RANGE_INIT(begin, end)
#else

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
        start[items_offset+i] = _WCL_FILLCHAR;                          \
    }                                                                   \
} while (0)                                                             \

#endif // cl_khr_initialize_memory

// <= General code that doesn't depend on input.

#define _WCL_ADDR_CHECK_private_1(type, addr, size, min0, max0) \
    ( 0\
    || ( ((addr) >= ((type)min0)) && ((addr + size - 1) <= _WCL_LAST(type, max0)) ) \
         )
#define _WCL_ADDR_CLAMP_private_1(type, addr, size, min0, max0, asnull) \
    ( _WCL_ADDR_CHECK_private_1(type, addr, size, min0, max0) ? (addr) : (type)(asnull))

#define _WCL_ADDR_CHECK_global_2(type, addr, size, min0, max0, min1, max1) \
    ( 0\
    || ( ((addr) >= ((type)min0)) && ((addr + size - 1) <= _WCL_LAST(type, max0)) ) \
        || ( ((addr) >= ((type)min1)) && ((addr + size - 1) <= _WCL_LAST(type, max1)) ) \
             )
#define _WCL_ADDR_CLAMP_global_2(type, addr, size, min0, max0, min1, max1, asnull) \
    ( _WCL_ADDR_CHECK_global_2(type, addr, size, min0, max0, min1, max1) ? (addr) : (type)(asnull))


float4 _wcl_vload4_0(_WclProgramAllocations* _wcl_allocs, int arg0, __global float * arg1)
{
    __global float * ptr = arg1 + 4 * (size_t) arg0;
    if (_WCL_ADDR_CHECK_global_2(
        __global float *, ptr, 4, 
        _wcl_allocs->gl.foo_kernel__in_min, 
        _wcl_allocs->gl.foo_kernel__in_max, 
        _wcl_allocs->gl.foo_kernel__out_min, 
        _wcl_allocs->gl.foo_kernel__out_max))
        return vload4(0, ptr);
    else
        return (float) 0;
}

void fix_index(_WclProgramAllocations *_wcl_allocs, 
    int index, int *fixed_index) {
    (*(_WCL_ADDR_CLAMP_private_1(int *, (fixed_index), 1, 
        &_wcl_allocs->pa, (&_wcl_allocs->pa + 1), 
        _wcl_allocs->pn))) = index*1;
}

kernel foo_kernel(
    global float *in, 
    unsigned long _wcl_in_size, 
    global float4 *out, 
    unsigned long _wcl_out_size) {

    __local uint _wcl_local_null[_WCL_ADDRESS_SPACE_local_MIN];

    _WclProgramAllocations _wcl_allocations_allocation = {
        { &in[0], &in[_wcl_in_size],&out[0], &out[_wcl_out_size] },
        0,
        { },
        0
    };

    _WclProgramAllocations *_wcl_allocs = &_wcl_allocations_allocation;
    _wcl_allocs->gn = _WCL_SET_NULL(__global uint*, 
        _WCL_ADDRESS_SPACE_global_MIN,
        _wcl_allocs->gl.foo_kernel__in_min,
        _wcl_allocs->gl.foo_kernel__in_max,
        _WCL_SET_NULL(__global uint*, 
            _WCL_ADDRESS_SPACE_global_MIN,
            _wcl_allocs->gl.foo_kernel__out_min, 
            _wcl_allocs->gl.foo_kernel__out_max, 
            (__global uint*)0));
    if (_wcl_allocs->gn == (__global uint*)0) return;

    _wcl_allocs->pn = _WCL_SET_NULL(__private uint*, 
        _WCL_ADDRESS_SPACE_private_MIN, 
        &_wcl_allocs->pa, (&_wcl_allocs->pa + 1), 
        (__private uint*)0);
    if (_wcl_allocs->pn == (__private uint*)0) return;

    struct _WclStruct c;

    int i = get_global_id(0);
    int index;
    fix_index(_wcl_allocs, i, &_wcl_allocs->pa._wcl_index);
    (*(_WCL_ADDR_CLAMP_global_2(__global float4 *, 
        (out)+(i), 1, 
        _wcl_allocs->gl.foo_kernel__in_min, _wcl_allocs->gl.foo_kernel__in_max, 
        _wcl_allocs->gl.foo_kernel__out_min, _wcl_allocs->gl.foo_kernel__out_max, 
        _wcl_allocs->gn))) = _wcl_vload4_0(_wcl_allocs, _wcl_allocs->pa._wcl_index, in);
}


