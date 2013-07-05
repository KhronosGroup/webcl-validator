
typedef struct {
    float table[3];
} TempStruct;

__constant float4 base_factor = ((float4)(1.0f,2.0f,3.0f,4.0f));
// NOTE: Clang bug seems to get initializer rewriting wrong for vector types.
//       after the support if fixed we can use initializations wihtout extra braces.
// __constant float4 base_factor = (float4)(1.0f,2.0f,3.0f,4.0f);
// __constant float base_table[] = { 1.0f,2.0f,3.0f,4.0f };

void init_scratch(size_t gid, size_t wgid, TempStruct *additional_shuffle, __global float4* input, __constant float4* factors, __local float4* scratch);
__local float4* flip_to_awesomeness(size_t wgid, size_t wgsize, __local float4* scratch);

void init_scratch(
    size_t gid, size_t wgid,
    TempStruct *additional_shuffle,
    __global float4* input, __constant float4* factors, __local float4* scratch) {
    scratch[wgid] = input[gid]*factors[gid]*additional_shuffle->table[gid%3];
} 

__local float4* flip_to_awesomeness(size_t wgid, size_t wgsize, __local float4* scratch) {
    float4 index_vec = scratch[wgid];
    float index_float = index_vec.x + index_vec.y + index_vec.z + index_vec.w;
    int index = (int)(index_float) % wgsize;
    return &scratch[index];
}

/**
 * input,output and factors needs to be the same size with global work size
 * scratch size should be the same that work group size is.
 */
__kernel void awesomize(
    __global float4* input,  
    __global float4* output,
    __constant float4* factors,
    __local float4* scratch) {

    size_t gid = get_global_id(0);
    size_t wgid = get_local_id(0);
    size_t wgsize = get_local_size(0);

    TempStruct private_struct = { {0, 1, 2} };

    __local size_t lottery_winner;

    init_scratch(gid, wgid, &private_struct, input, factors, scratch);
    lottery_winner = gid;
    barrier(CLK_LOCAL_MEM_FENCE);

    if (gid == 0) {
        output[0] = lottery_winner + input->x + private_struct.table[2];
    } else {
        output[gid] = (*flip_to_awesomeness(wgid, wgsize, scratch))*base_factor;
    }
}

