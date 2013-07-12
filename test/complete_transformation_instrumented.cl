// RUN: true

/**
 * Completely unoptimized instrumentation, which does collect all the variables
 * to the address space.
 */

typedef struct {
    float table[3];
} TempStruct;

typedef struct {
    size_t awesomize__gid;
    size_t awesomize__wgid;
    size_t awesomize__wgsize;
    TempStruct awesomize__private_struct;
    float4 flip_to_awesomeness__index_vec;
    float flip_to_awesomeness__index_float;
    int flip_to_awesomeness__index;
} WclPrivates;

typedef struct {
    size_t lottery_winner;
} WclLocals;

typedef struct {
    float4 base_factor;
} WclConstants;

typedef struct {
    __constant WclConstants *wcl_constant_allocations_min;
    __constant WclConstants *wcl_constant_allocations_max;
    __constant float4 *awesomize__factors_min;
    __constant float4 *awesomize__factors_max;
} WclConstantLimits;

typedef struct {
    __local WclLocals *wcl_locals_min; 
    __local WclLocals *wcl_locals_max;
    __local float4 *awesomize__scratch_min;
    __local float4 *awesomize__scratch_max;
} WclLocalLimits;

typedef struct {
    __global float4 *awesomize__input_min;
    __global float4 *awesomize__input_max;
    __global float4 *awesomize__output_min;
    __global float4 *awesomize__output_max;
} WclGlobalLimits;

// we have only the limits passed 
typedef struct {
    WclConstantLimits cl; // all constant memory limits
    WclGlobalLimits   gl; // all global memory limits
    WclLocalLimits    ll; // all local address space limits
    WclPrivates       pa; // private memory data
} WclProgramAllocations;

// actually this probably should not be output
__constant WclConstants wcl_constant_allocations = { (float4)(1.0f,2.0f,3.0f,4.0f) };

#define WCL_MIN(a, b)                           \
    (((a) < (b)) ? (a) : (b))
#define WCL_MAX(a, b)                           \
    (((a) < (b)) ? (b) : (a))
#define WCL_CLAMP(low, value, high)             \
    WCL_MAX((low), WCL_MIN((value), (high)))

// TODO: find out case where min >= max e.g. if memory area to be read is too big
//       for the limits, basically we could avoid that in compile time by preventing 
//       of casting pointers to wider types 
#define WCL_ADDR(type, ptr, min_ptr, max_ptr) \
    WCL_CLAMP( ((type)min_ptr), (ptr), (((type)max_ptr)-1) )

void init_scratch(WclProgramAllocations *wcl_allocs, size_t gid, size_t wgid, TempStruct *additional_shuffle, __global float4* input, __constant float4* factors, __local float4* scratch);
__local float4* flip_to_awesomeness(WclProgramAllocations *wcl_allocs, size_t wgid, size_t wgsize, __local float4* scratch);

void init_scratch(
    WclProgramAllocations *wcl_allocs,
    size_t gid, size_t wgid,
    TempStruct *additional_shuffle,
    __global float4* input, __constant float4* factors, __local float4* scratch) {

    //scratch[wgid] = input[gid]*factors[gid]*additional_shuffle->table[gid%3];
    (*(WCL_ADDR(__local float4*, 
        scratch+wgid, 
        wcl_allocs->ll.awesomize__scratch_min,
        wcl_allocs->ll.awesomize__scratch_max))) = 

        (*(WCL_ADDR(__global float4*, 
            input + gid,
            wcl_allocs->gl.awesomize__input_min,
            wcl_allocs->gl.awesomize__input_max))) * 

        (*(WCL_ADDR(__constant float4*, 
            factors + gid,
            wcl_allocs->cl.awesomize__factors_min,
            wcl_allocs->cl.awesomize__factors_max))) * 

        (*(WCL_ADDR(float*,        
            (*(WCL_ADDR(TempStruct*, 
                additional_shuffle,
                &wcl_allocs->pa,
                (&wcl_allocs->pa + 1)))).table + (gid%3),  
            &wcl_allocs->pa, 
            (&wcl_allocs->pa + 1))));
} 

__local float4* flip_to_awesomeness(WclProgramAllocations *wcl_allocs, size_t wgid, size_t wgsize, __local float4* scratch) {

    wcl_allocs->pa.flip_to_awesomeness__index_vec = (*(WCL_ADDR(__local float4*,
        scratch + (wgid),
        wcl_allocs->ll.awesomize__scratch_min,
        wcl_allocs->ll.awesomize__scratch_max)));
    
    wcl_allocs->pa.flip_to_awesomeness__index_float = 
        wcl_allocs->pa.flip_to_awesomeness__index_vec.x + 
        wcl_allocs->pa.flip_to_awesomeness__index_vec.y + 
        wcl_allocs->pa.flip_to_awesomeness__index_vec.z + 
        wcl_allocs->pa.flip_to_awesomeness__index_vec.w;
    
    wcl_allocs->pa.flip_to_awesomeness__index = (int)(wcl_allocs->pa.flip_to_awesomeness__index_float) % wgsize;
    
    return &(*(WCL_ADDR(__local float4*,
        scratch + (wcl_allocs->pa.flip_to_awesomeness__index),
        wcl_allocs->ll.awesomize__scratch_min,
        wcl_allocs->ll.awesomize__scratch_max)));
}

/**
 * input,output and factors needs to be the same size with global work size
 * scratch size should be the same that work group size is.
 */
__kernel void awesomize(
    __global float4* input, uint wcl_input_size,
    __global float4* output, uint wcl_output_size,
    __constant float4* factors, uint wcl_factors_size,
    __local float4* scratch, uint wcl_scratch_size) {

    __local WclLocals wcl_locals;
    // trivial local memory zeroing
    for (uint i = 0; i < wcl_scratch_size; i++) scratch[i] = (float4)(0,0,0,0);
    for (uint i = 0; i < sizeof(WclLocals); i++) ((uchar*)(&wcl_locals))[i] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);

    // zeroing private memory and allowing initializations is pretty tough
    // problem because of align
    WclProgramAllocations wcl_allocations_allocation;

    // trivial private memory zeroing which also prevents flood because of padding
    for (uint i = 0; i < sizeof(WclProgramAllocations); i++) ((uchar*)(&wcl_allocations_allocation))[i] = 0;
    WclProgramAllocations *wcl_allocs = &wcl_allocations_allocation;

    // initialize limits structures
    wcl_allocs->cl.wcl_constant_allocations_min = &(&wcl_constant_allocations)[0];
    wcl_allocs->cl.wcl_constant_allocations_max = &(&wcl_constant_allocations)[1];
    wcl_allocs->cl.awesomize__factors_min = &factors[0];
    wcl_allocs->cl.awesomize__factors_max = &factors[wcl_factors_size];

    wcl_allocs->ll.wcl_locals_min = &(&wcl_locals)[0];
    wcl_allocs->ll.wcl_locals_max = &(&wcl_locals)[1];
    wcl_allocs->ll.awesomize__scratch_min = &scratch[0];
    wcl_allocs->ll.awesomize__scratch_max = &scratch[wcl_scratch_size];

    wcl_allocs->gl.awesomize__input_min = &input[0];
    wcl_allocs->gl.awesomize__input_max = &input[wcl_input_size];
    wcl_allocs->gl.awesomize__output_min = &output[0];
    wcl_allocs->gl.awesomize__output_max = &output[wcl_output_size];

    // --- end of extra initializations

    wcl_allocs->pa.awesomize__gid = get_global_id(0);
    wcl_allocs->pa.awesomize__wgid = get_local_id(0);
    wcl_allocs->pa.awesomize__wgsize = get_local_size(0);

    wcl_allocs->pa.awesomize__private_struct.table[0] = 0;
    wcl_allocs->pa.awesomize__private_struct.table[1] = 1;
    wcl_allocs->pa.awesomize__private_struct.table[2] = 2;

    init_scratch(
        wcl_allocs, 
        wcl_allocs->pa.awesomize__gid, 
        wcl_allocs->pa.awesomize__wgid, 
        &wcl_allocs->pa.awesomize__private_struct, 
        input, factors, scratch);
    
    wcl_locals.lottery_winner = wcl_allocs->pa.awesomize__gid;
    barrier(CLK_LOCAL_MEM_FENCE);

    if (wcl_allocs->pa.awesomize__gid == 0) {
        (*(WCL_ADDR(__global float4*,
            output + (0),
            wcl_allocs->gl.awesomize__output_min, 
            wcl_allocs->gl.awesomize__output_max))) = wcl_locals.lottery_winner + 
                    (*(WCL_ADDR(__global float4*, input,
                        wcl_allocs->gl.awesomize__input_min, 
                        wcl_allocs->gl.awesomize__input_max))).x +
            (*(WCL_ADDR(float*,
                wcl_allocs->pa.awesomize__private_struct.table + (2),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
    } else {
        (*(WCL_ADDR(__global float4*,
            output + (wcl_allocs->pa.awesomize__gid), 
            wcl_allocs->gl.awesomize__output_min, 
            wcl_allocs->gl.awesomize__output_max))) = 
                (*(WCL_ADDR(__local float4*,
                    flip_to_awesomeness(wcl_allocs, wcl_allocs->pa.awesomize__wgid, wcl_allocs->pa.awesomize__wgsize, scratch),
                    wcl_allocs->ll.awesomize__scratch_min,
                    wcl_allocs->ll.awesomize__scratch_max))) * wcl_constant_allocations.base_factor;
    }
}

