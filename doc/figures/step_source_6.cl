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

void fix_index(_WclProgramAllocations *_wcl_allocs, 
	int index, int *fixed_index) {
	*fixed_index = index*1;
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
    _wcl_allocs->gn = _WCL_SET_NULL(
    	__global uint*, 
    	_WCL_ADDRESS_SPACE_global_MIN,
    	_wcl_allocs->gl.foo_kernel__in_min, 
    	_wcl_allocs->gl.foo_kernel__in_max, 
    	_WCL_SET_NULL(
    		__global uint*, 
    		_WCL_ADDRESS_SPACE_global_MIN,_wcl_allocs->gl.foo_kernel__out_min, 
    		_wcl_allocs->gl.foo_kernel__out_max, 
    		(__global uint*)0));
    // abort and return from kernel if could not find enough memory, 
    // would be nice to be able to throw an error
    if (_wcl_allocs->gn == (__global uint*)0) return;

    _wcl_allocs->pn = _WCL_SET_NULL(
    	__private uint*, 
    	_WCL_ADDRESS_SPACE_private_MIN, 
    	&_wcl_allocs->pa, 
    	(&_wcl_allocs->pa + 1), 
    	(__private uint*)0);
    if (_wcl_allocs->pn == (__private uint*)0) return;

    struct _WclStruct c;

    int i = get_global_id(0);
    int index;
    fix_index(_wcl_allocs, i, &_wcl_allocs->pa._wcl_index);
    out[i] = vload4(_wcl_allocs->pa._wcl_index, in);
}

