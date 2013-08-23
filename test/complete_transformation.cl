// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s
// RUN: cat %s | %kernel-runner  --kernel awesomize --global float4 50 --global float4 50 --constant float4 50 --local float4 1024 --gcount 50 | grep "2,16,-128,0,55,-59,0,-75,38,0,-63,53,0,91,41,0,76,2,0,-108,-119,0,106,44,0,-105,125,0,82,-77,0,100,5,0,4,5,0,-60,-77,0,73,125,0,37,44,0,88,-119,0,25,0"
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing\|CHECK" | %kernel-runner  --kernel awesomize --global float4 50 --global float4 50 --constant float4 50 --local float4 1024 --gcount 50 --webcl | grep "2,16,-128,0,55,-59,0,-75,38,0,-63,53,0,91,41,0,76,2,0,-108,-119,0,106,44,0,-105,125,0,82,-77,0,100,5,0,4,5,0,-60,-77,0,73,125,0,37,44,0,88,-119,0,25,0"

// TODO: this fails badly with POCL. Is it POCL bug or test code?

typedef struct {
    float table[3];
} TempStruct;

__constant float4 base_factor = ((float4)(1.0f,2.0f,3.0f,4.0f));
// NOTE: Clang bug seems to get initializer rewriting wrong for vector types.
//       after the support if fixed we can use initializations wihtout extra braces.
// __constant float4 base_factor = (float4)(1.0f,2.0f,3.0f,4.0f);
// __constant float base_table[] = { 1.0f,2.0f,3.0f,4.0f };

void empty_params(void);
void init_scratch(size_t gid, size_t wgid, TempStruct *additional_shuffle, __global float4* input, __constant float4* factors, __local float4* scratch);
// __local float4* flip_to_awesomeness(size_t wgid, size_t wgsize, __local float4* scratch);
//
// CHECK: empty_params(_WclProgramAllocations *_wcl_allocs)
void empty_params() {
    int table[3];
} 

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
    __global char* kernel_runner_output,
    __global float4* input,  
    __global float4* output,
    __constant float4* factors,
    __local float4* scratch) {

    // check empty arg list conversion
    empty_params();
    // check removing qualifiers from tables and structs
    __local size_t localTable[3];
    __local TempStruct localStruct;
    __local TempStruct localStructTable[2];

    // Tables of pointers are always allocated from private memory
    // __local int __global * localTableOfGlobalASPtrs[2];
    __global int* tableOfGlobalASPtrs[2];
    __local int* tableOfLocalASPtrs[2];

    size_t gid = get_global_id(0);
    size_t wgid = get_local_id(0);
    size_t wgsize = get_local_size(0);

    TempStruct private_struct = { {0, 1, 2} };
    int uninit_table[3];
    int table[3] = {1,2,3};
    table[1] = 1;
    __local size_t lottery_winner;

    init_scratch(gid, wgid, &private_struct, input, factors, scratch);
    barrier(CLK_LOCAL_MEM_FENCE);

    if (gid == 0) {
#ifdef __PLATFORM_AMD__
        output[0] = (*input).x + private_struct.table[2];
#else
        output[0] = input->x + private_struct.table[2];
#endif
    } else {
        output[gid] = (*flip_to_awesomeness(wgid, wgsize, scratch))*base_factor;
    }

    //float output_float = output[gid].x;
    //unsigned expotent = (*((unsigned*)(&output_float))>>22)&0xff;
    // kernel_runner_output[gid%1024] = expotent;
    kernel_runner_output[gid%1024] = (unsigned)(output[gid].x)%201;
}

