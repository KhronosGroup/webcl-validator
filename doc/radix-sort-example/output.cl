// RUN: %opencl-validator < %s
// RUN: %radix-sort -transform < %s

// Code derives (very loosely) from example in "Heterogeneous
// Computing with OpenCL" published 2011 by Morgan Kaufmann.
//
// The original code has been described in "Introduction to GPU Radix
// Sort" by Takahiro Harada and Lee Howes of AMD. It is copyright 2011
// by Takahiro Harada.
//
// We have modified the code to better suit WebCL validation, which
// means that we don't care about performance anymore. Use the
// original code if you need fast radix sort implementation. The code
// has been formatted on purpose so that it's easy to compare input
// and output source with a merge tool.

// NOTE: this is the first hand instrumented real test case

typedef struct {
    uint prefix_scan_kernel__scalar_histogram[20];
    uint4 prefix_scan_128__vector_histogram;
    uint prefix_scan_kernel__unused_top_level_prefix_sum;
} WclPrivates;

typedef struct {
    uint stream_count_kernel__histogram_matrix[1024];
    uint prefix_scan_kernel__unused_vector_level_prefix_sums[256];
} WclLocals;

typedef struct {
    uint dummy;
} WclConstants;

typedef struct {
    __constant WclConstants *wcl_constant_allocations_min;
    __constant WclConstants *wcl_constant_allocations_max;
    __constant uint *stream_count_kernel__unsorted_elements_min;
    __constant uint *stream_count_kernel__unsorted_elements_max;
} WclConstantLimits;

typedef struct {
    __local WclLocals *wcl_locals_min;
    __local WclLocals *wcl_locals_max;
} WclLocalLimits;

typedef struct {
    __global uint *stream_count_kernel__element_counts_min;
    __global uint *stream_count_kernel__element_counts_max;
    __global uint *prefix_scan_kernel__histogram_min;
    __global uint *prefix_scan_kernel__histogram_max;
} WclGlobalLimits;

// we have only the limits passed 
typedef struct {
    WclConstantLimits cl;
    WclGlobalLimits   gl;
    WclLocalLimits    ll;
    WclPrivates       pa;
} WclProgramAllocations;

#define NULL 0

// actually this probably should not be output
__constant WclConstants wcl_constant_allocations = { 0 };

#define WCL_MIN(a, b)                           \
    (((a) < (b)) ? (a) : (b))
#define WCL_MAX(a, b)                           \
    (((a) < (b)) ? (b) : (a))
#define WCL_CLAMP(low, value, high)             \
    WCL_MAX((low), WCL_MIN((value), (high)))

#define WCL_ADDR(type, ptr, min_ptr, max_ptr) \
    WCL_CLAMP( ((type)min_ptr), (ptr), (((type)max_ptr)-1) )


////////////////////////////
// PHASE 1: Count values. //
////////////////////////////

#define STREAM_COUNT_WORKGROUP_SIZE 64
#define BITS_PER_PASS 4
#define VALUES_PER_PASS (1 << BITS_PER_PASS)

// prototypes for apple
uint get_histogram_index(uint value);
void set_histogram(WclProgramAllocations *wcl_allocs, __local uint *histogram, uint value, uint count);
void clear_histogram(WclProgramAllocations *wcl_allocs, __local uint *histogram);
void inc_histogram(WclProgramAllocations *wcl_allocs, __local uint *histogram, uint value);

uint get_histogram_index(
    uint value)
{
    // [value][get_local_id(0)]
    return (value * STREAM_COUNT_WORKGROUP_SIZE) + get_local_id(0);
}

void set_histogram(
    WclProgramAllocations *wcl_allocs,
    __local uint *histogram,
    uint value,
    uint count)
{
    // histogram[get_histogram_index(value)] = count;
    (*(WCL_ADDR(__local uint*,
        histogram + get_histogram_index(value),
        wcl_allocs->ll.wcl_locals_min,
        wcl_allocs->ll.wcl_locals_max))) = count;
}

void clear_histogram(
    WclProgramAllocations *wcl_allocs,
    __local uint *histogram)
{
    for (int value = 0; value < VALUES_PER_PASS; ++value) {
        set_histogram(wcl_allocs, histogram, value, 0);
    }
}

void inc_histogram(
    WclProgramAllocations *wcl_allocs,
    __local uint *histogram,
    uint value)
{
    // ++histogram[get_histogram_index(value)];
    ++(*(WCL_ADDR(__local uint*,
        histogram + (get_histogram_index(value)),
        wcl_allocs->ll.wcl_locals_min,
        wcl_allocs->ll.wcl_locals_max)));
}

#define ELEMENTS_PER_WORK_ITEM 4
#define ELEMENTS_PER_BLOCK (STREAM_COUNT_WORKGROUP_SIZE * ELEMENTS_PER_WORK_ITEM)
#define VALUE_MASK (VALUES_PER_PASS - 1)

__kernel __attribute__((reqd_work_group_size(STREAM_COUNT_WORKGROUP_SIZE, 1, 1)))
void stream_count_kernel(
    __constant uint *unsorted_elements,
    const unsigned long wcl_unsorted_elements_size,
    __global uint *element_counts,
    const unsigned long wcl_element_counts_size,
    const int bit_offset,
    const int num_elements,
    const int num_workgroups,
    const int num_blocks_in_workgroup)
{
    __local WclLocals wcl_locals;

    WclProgramAllocations wcl_allocations_allocation;
    // = {  
    //    {&(&wcl_constant_allocations)[0],&(&wcl_constant_allocations)[1],&unsorted_elements[0], &unsorted_elements[wcl_unsorted_elements_size]}, 
    //    {&element_counts[0],&element_counts[wcl_element_counts_size],NULL,NULL}, 
    //    {&(&wcl_locals)[0],&(&wcl_locals)[1]}, 
    //    {{0}, (uint4)(0,0,0,0), 0 }
    //};
    for (uint i = 0; i < sizeof(WclProgramAllocations); i++) ((uchar*)(&wcl_allocations_allocation))[i] = 0;

    WclProgramAllocations *wcl_allocs = &wcl_allocations_allocation;

    // setup constant allocation areas
    wcl_allocs->cl.wcl_constant_allocations_min = &(&wcl_constant_allocations)[0];
    wcl_allocs->cl.wcl_constant_allocations_max = &(&wcl_constant_allocations)[1];
    wcl_allocs->cl.stream_count_kernel__unsorted_elements_min = &unsorted_elements[0];
    wcl_allocs->cl.stream_count_kernel__unsorted_elements_max = &unsorted_elements[wcl_unsorted_elements_size];

    // setup global limit allocation areas
    wcl_allocs->gl.stream_count_kernel__element_counts_min = &element_counts[0];
    wcl_allocs->gl.stream_count_kernel__element_counts_max = &element_counts[wcl_element_counts_size];

    // and local address space limits
    wcl_allocs->ll.wcl_locals_min = &(&wcl_locals)[0];
    wcl_allocs->ll.wcl_locals_max = &(&wcl_locals)[1];

    clear_histogram(wcl_allocs, wcl_locals.stream_count_kernel__histogram_matrix);
    barrier(CLK_LOCAL_MEM_FENCE);

    const int num_total_blocks = num_elements / ELEMENTS_PER_BLOCK;
    const int num_preceding_blocks = get_group_id(0) * num_blocks_in_workgroup;
    const int num_remaining_blocks = min(num_blocks_in_workgroup, num_total_blocks - num_preceding_blocks);

    // base index in 'unsorted_elements' for work item
    size_t element_base = (num_preceding_blocks * ELEMENTS_PER_BLOCK) + (get_local_id(0) * ELEMENTS_PER_WORK_ITEM);

    // histogram_matrix = [[c0_i0, c0_i1, ..., c0_in], ..., [c15_i0, c15_i1, ..., c15_in]]
    for (int block = 0; block < num_remaining_blocks; ++block, element_base += ELEMENTS_PER_BLOCK) {
        for (size_t element_index = 0; element_index < ELEMENTS_PER_WORK_ITEM; ++element_index) {
            const size_t index = element_base + element_index;
          
            // Was: const uint value = unsorted_elements[index];
            const uint value = (*(WCL_ADDR(__constant uint*,
                unsorted_elements + (index),
                wcl_allocs->cl.stream_count_kernel__unsorted_elements_min,
                wcl_allocs->cl.stream_count_kernel__unsorted_elements_max)));

            const uint pass_value = (value >> bit_offset) & VALUE_MASK;
            inc_histogram(
                wcl_allocs, wcl_locals.stream_count_kernel__histogram_matrix,
                pass_value);
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // element_counts = [[c0_g0, c0_g1, ..., c0_gm], ..., [c15_g0, c15_g1, ..., c15_gm]]
    if (get_local_id(0) < VALUES_PER_PASS) {
        uint sum = 0;
        // sum = cx_i0 + cx_i1 + ... + cx_in
        for (size_t i = 0; i < get_local_size(0); ++i) {
            const size_t index = (get_local_id(0) * STREAM_COUNT_WORKGROUP_SIZE) + ((get_local_id(0) + i) % get_local_size(0));
            // Was: sum += wcl_locals.stream_count_kernel__histogram_matrix[index];
            sum += (*(WCL_ADDR(__local uint*,
                wcl_locals.stream_count_kernel__histogram_matrix + (index),
                wcl_allocs->ll.wcl_locals_min,
                wcl_allocs->ll.wcl_locals_max)));
        }
        const size_t sum_index = (get_local_id(0) * num_workgroups) + get_group_id(0);

        // Was: element_counts[sum_index] = sum;
        (*(WCL_ADDR(__global uint*,
            element_counts + (sum_index),
            wcl_allocs->gl.stream_count_kernel__element_counts_min,
            wcl_allocs->gl.stream_count_kernel__element_counts_max))) = sum;
    }
}

////////////////////////////////////
// PHASE 2: Calculate prefix sum. //
////////////////////////////////////

// prototypes for apple driver
uint prefix_scan_lanes(WclProgramAllocations *wcl_allocs, uint4 *vector_data);
uint prefix_scan_vectors(WclProgramAllocations *wcl_allocs, const uint lane_level_prefix_sum, uint *total_sum, __local uint *total_sums, const size_t num_work_items);
uint4 prefix_scan_128(WclProgramAllocations *wcl_allocs, uint4 vector_histogram, uint *total_sum, __local uint *total_sums);

uint prefix_scan_lanes( // lane level prefix sum
    WclProgramAllocations *wcl_allocs,
    uint4 *vector_data) // lane counts -> lane prefix sums
{
    uint sum = 0;
    // Was: uint4 vector_temp = *vector_data;
    uint4 vector_temp = *(WCL_ADDR(uint4*, 
        vector_data, 
        &wcl_allocs->pa, (&wcl_allocs->pa + 1)));

    uint tmp = vector_temp.x;
    vector_temp.x = sum;

    sum += tmp;
    tmp = vector_temp.y;
    vector_temp.y = sum;

    sum += tmp;
    tmp = vector_temp.z;
    vector_temp.z = sum;

    sum += tmp;
    tmp = vector_temp.w;
    vector_temp.w = sum;

    // Was: *vector_data = vector_temp;
    *(WCL_ADDR(uint4*, 
        vector_data, 
        &wcl_allocs->pa, (&wcl_allocs->pa + 1))) = vector_temp;
    
    sum += tmp;
    return sum;
}

uint prefix_scan_vectors(
    WclProgramAllocations *wcl_allocs,
    const uint lane_level_prefix_sum, // partial prefix sum within single work item
    uint *total_sum, // total prefix sum of all work items
    __local uint *total_sums, // [0..127]: zeroed, [128..255]: prefix sums of all work items
    const size_t num_work_items)
{
    const size_t work_item = get_local_id(0);
    const size_t index = num_work_items + work_item;

    // [0..127]: zeroed, [128..255]: partial prefix sums
    // Was: total_sums[work_item] = 0;
    (*(WCL_ADDR(__local uint*,
        total_sums + (work_item),
        wcl_allocs->ll.wcl_locals_min,
        wcl_allocs->ll.wcl_locals_max))) = 0;

    // Was: total_sums[index] = lane_level_prefix_sum;
    (*(WCL_ADDR(__local uint*,
        total_sums + (index),
        wcl_allocs->ll.wcl_locals_min,
        wcl_allocs->ll.wcl_locals_max))) = lane_level_prefix_sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    // [128..255]: total prefix sums
    for (uint i = 1; i <= (num_work_items / 2); i *= 2) {
        // Was: const uint partial_sum = total_sums[index - i];
        const uint partial_sum = (*(WCL_ADDR(__local uint*,
            total_sums + (index - i),
            wcl_allocs->ll.wcl_locals_min,
            wcl_allocs->ll.wcl_locals_max)));
        barrier(CLK_LOCAL_MEM_FENCE);
        // Was: total_sums[index] += partial_sum;
        (*(WCL_ADDR(__local uint*,
            total_sums + (index),
            wcl_allocs->ll.wcl_locals_min,
            wcl_allocs->ll.wcl_locals_max))) += partial_sum;
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Total prefix sum of all work items.
    // Was: *total_sum = total_sums[(num_work_items * 2) - 1];
    *(WCL_ADDR(uint*,
        total_sum,
        &wcl_allocs->pa, (&wcl_allocs->pa + 1))) = 
    (*(WCL_ADDR(__local uint*,
            total_sums + ((num_work_items * 2) - 1),
            wcl_allocs->ll.wcl_locals_min,
            wcl_allocs->ll.wcl_locals_max)));

    // Total prefix sum of work item.
    // Was: return total_sums[work_item + num_work_items - 1];
    return (*(WCL_ADDR(__local uint*,
            total_sums + (work_item + num_work_items - 1),
            wcl_allocs->ll.wcl_locals_min,
            wcl_allocs->ll.wcl_locals_max)));
}

#define PREFIX_SCAN_WORKGROUP_SIZE 128

uint4 prefix_scan_128(
    WclProgramAllocations *wcl_allocs,
    uint4 vector_histogram,
    uint *total_sum,
    __local uint *total_sums)
{
    wcl_allocs->pa.prefix_scan_128__vector_histogram = vector_histogram;

    const uint lane_prefix_sum = prefix_scan_lanes(wcl_allocs,
        &wcl_allocs->pa.prefix_scan_128__vector_histogram);

    const uint vector_prefix_sum = prefix_scan_vectors(wcl_allocs,
        lane_prefix_sum, total_sum, total_sums, PREFIX_SCAN_WORKGROUP_SIZE);

    return wcl_allocs->pa.prefix_scan_128__vector_histogram +
        (uint4)(vector_prefix_sum);

}

#define PREFIX_SCAN_LANES 4
#define PREFIX_SCAN_LANE_ELEMENTS 5
#define PREFIX_SCAN_ELEMENTS (PREFIX_SCAN_LANES * PREFIX_SCAN_LANE_ELEMENTS)

__kernel __attribute__((reqd_work_group_size(PREFIX_SCAN_WORKGROUP_SIZE, 1, 1)))
void prefix_scan_kernel(
    __global uint *histogram, // [[c0_g0, c0_g1, ..., c0_gm], ..., [c15_g0, c15_g1, ..., c15_gm]] ->
                              // [[s0_g0, s0_g1, ..., s0_gm], ..., [s15_g0, s15_g1, ..., s15_gm]]
    const unsigned long wcl_histogram_size,
    const int num_workgroups)
{
    __local WclLocals wcl_locals;
    WclProgramAllocations wcl_allocations_allocation;
    for (uint i = 0; i < sizeof(WclProgramAllocations); i++) ((uchar*)(&wcl_allocations_allocation))[i] = 0;
    WclProgramAllocations *wcl_allocs = &wcl_allocations_allocation;

    // setup constant allocation areas
    wcl_allocs->cl.wcl_constant_allocations_min = &(&wcl_constant_allocations)[0];
    wcl_allocs->cl.wcl_constant_allocations_max = &(&wcl_constant_allocations)[1];

    // setup global limit allocation areas
    wcl_allocs->gl.prefix_scan_kernel__histogram_min = &histogram[0];
    wcl_allocs->gl.prefix_scan_kernel__histogram_max = &histogram[wcl_histogram_size];

    // and local address space limits
    wcl_allocs->ll.wcl_locals_min = &(&wcl_locals)[0];
    wcl_allocs->ll.wcl_locals_max = &(&wcl_locals)[1];

    const size_t element_counts_size = VALUES_PER_PASS * num_workgroups;
    const size_t work_item_base = get_local_id(0) * PREFIX_SCAN_ELEMENTS;

    // Each element of 'scalar_histogram' contains count of possibly
    // ascending four bit values.
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        // Was:
        // wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[i] =
        //     (index < element_counts_size) ?
        //     histogram[index] :
        //     0;
        (*(WCL_ADDR(uint*, 
            wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + (i),
            &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) =
                (index < element_counts_size) ?
                (*(WCL_ADDR(__global uint*,
                    histogram + (index),
                    wcl_allocs->gl.prefix_scan_kernel__histogram_min,
                    wcl_allocs->gl.prefix_scan_kernel__histogram_max))) :
                0;
    }


    // The 20 elements of 'scalar_histogram' are divided into 4 lanes
    // containing 5 elements each. The 4 elements of
    // 'vector_histogram' contain the number of values in each lane.
    uint4 vector_histogram = (uint4)(0);

    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        // Was: 
        // vector_histogram.x +=
        //     wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(0 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.x +=
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((0 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
        // Was: 
        // vector_histogram.y +=
        //     wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(1 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.y +=
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((1 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
        // Was: 
        // vector_histogram.z +=
        //     wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(2 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.z +=
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((2 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
        // Was: 
        // vector_histogram.w +=
        //     wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(3 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.w +=
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((3 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
    }

    // After the call, 'unused_top_level_prefix_sum' contains the
    // number of values in 'histogram'.

    // The elements of 'histogram' are divided into 20-element
    // vectors. After the call, 'unused_vector_level_sums' contains
    // prefix sums of the vectors.

    // The combined prefix sum of at lane and vector levels is retured.

    const uint4 lane_and_vector_level_prefix_sum = prefix_scan_128(
        wcl_allocs,
        vector_histogram,
        &wcl_allocs->pa.prefix_scan_kernel__unused_top_level_prefix_sum,
        wcl_locals.prefix_scan_kernel__unused_vector_level_prefix_sums);

    // scalar_histogram[index] = prefix sum of elements within corresponding lane

    for (size_t lane = 0; lane < PREFIX_SCAN_LANES; ++lane) {
        uint sum = 0;
        for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
            const size_t index = (lane * PREFIX_SCAN_LANE_ELEMENTS) + element;
            // Was: const uint tmp = wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[index];
            const uint tmp = (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + (index),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
            // Was: wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[index] = sum;
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + (index),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) = sum;
            sum += tmp;
        }
    }

    // scalar_histogram[index] = prefix sum of elements within lane
    //                         + prefix sum of lanes within vector
    //                         + prefix sum of vectors
    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        // Was:
        // wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(0 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
        //     lane_and_vector_level_prefix_sum.x;
        (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((0 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) +=
            lane_and_vector_level_prefix_sum.x;
        // Was:
        // wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(1 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
        //     lane_and_vector_level_prefix_sum.y;
        (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((1 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) +=
            lane_and_vector_level_prefix_sum.y;
        // Was:
        // wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(2 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
        //     lane_and_vector_level_prefix_sum.z;
        (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((2 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) +=
            lane_and_vector_level_prefix_sum.z;
        // Was:
        // wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[(3 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
        //     lane_and_vector_level_prefix_sum.w;
        (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + ((3 * PREFIX_SCAN_LANE_ELEMENTS) + element),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1)))) += 
            lane_and_vector_level_prefix_sum.w;
    }

    // Copy work item result into work group result.
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        // Was: histogram[index] = wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[i];
        // histogram[index] = wcl_allocs->pa.prefix_scan_kernel__scalar_histogram[i];
        (*(WCL_ADDR(__global uint *,
                histogram + (index),
                wcl_allocs->gl.prefix_scan_kernel__histogram_min,
                wcl_allocs->gl.prefix_scan_kernel__histogram_max))) =
            (*(WCL_ADDR(uint*, 
                wcl_allocs->pa.prefix_scan_kernel__scalar_histogram + (i),
                &wcl_allocs->pa, (&wcl_allocs->pa + 1))));
    }
}

///////////////////////////////////
// PHASE 3: Not implemented yet. //
///////////////////////////////////
