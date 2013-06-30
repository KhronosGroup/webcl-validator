// RUN: cat %s | %opencl-validator
// RUN: cat %s | %radix-sort -original
// DISABLED RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v "Processing:" | %radix-sort -transform

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

////////////////////////////
// PHASE 1: Count values. //
////////////////////////////

#define STREAM_COUNT_WORKGROUP_SIZE 64
#define BITS_PER_PASS 4
#define VALUES_PER_PASS (1 << BITS_PER_PASS)

uint get_histogram_index(uint value);
void set_histogram(__local uint *histogram, uint value, uint count);
void clear_histogram(__local uint *histogram);
void inc_histogram(__local uint *histogram, uint value);

uint get_histogram_index(
    uint value)
{
    // [value][get_local_id(0)]
    return (value * STREAM_COUNT_WORKGROUP_SIZE) + get_local_id(0);
}

void set_histogram(
    __local uint *histogram,
    uint value,
    uint count)
{
    histogram[get_histogram_index(value)] = count;
}

void clear_histogram(
    __local uint *histogram)
{
    for (int value = 0; value < VALUES_PER_PASS; ++value) {
        set_histogram(histogram, value, 0);
    }
}

void inc_histogram(
    __local uint *histogram,
    uint value)
{
    ++histogram[get_histogram_index(value)];
}

#define ELEMENTS_PER_WORK_ITEM 4
#define ELEMENTS_PER_BLOCK (STREAM_COUNT_WORKGROUP_SIZE * ELEMENTS_PER_WORK_ITEM)
#define VALUE_MASK (VALUES_PER_PASS - 1)

__kernel __attribute__((reqd_work_group_size(STREAM_COUNT_WORKGROUP_SIZE, 1, 1)))
void stream_count_kernel(
    __constant uint *unsorted_elements,
    __global uint *element_counts,
    const int bit_offset,
    const int num_elements,
    const int num_workgroups,
    const int num_blocks_in_workgroup)
{
    // histogram_matrix[VALUES_PER_PASS][STREAM_COUNT_WORKGROUP_SIZE];
    __local uint histogram_matrix[VALUES_PER_PASS * STREAM_COUNT_WORKGROUP_SIZE];
    clear_histogram(histogram_matrix);
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
            const uint value = unsorted_elements[index];
            const uint pass_value = (value >> bit_offset) & VALUE_MASK;
            inc_histogram(histogram_matrix, pass_value);
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    // element_counts = [[c0_g0, c0_g1, ..., c0_gm], ..., [c15_g0, c15_g1, ..., c15_gm]]
    if (get_local_id(0) < VALUES_PER_PASS) {
        uint sum = 0;
        // sum = cx_i0 + cx_i1 + ... + cx_in
        for (size_t i = 0; i < get_local_size(0); ++i) {
            const size_t index = (get_local_id(0) * STREAM_COUNT_WORKGROUP_SIZE) + ((get_local_id(0) + i) % get_local_size(0));
            sum += histogram_matrix[index];
        }
        const size_t sum_index = (get_local_id(0) * num_workgroups) + get_group_id(0);
        element_counts[sum_index] = sum;
    }
}

////////////////////////////////////
// PHASE 2: Calculate prefix sum. //
////////////////////////////////////

uint prefix_scan_lanes(uint4 *vector_data);
uint prefix_scan_vectors(const uint lane_level_prefix_sum, uint *total_sum, __local uint *total_sums, const size_t num_work_items);
uint4 prefix_scan_128(uint4 vector_histogram, uint *total_sum, __local uint *total_sums);

uint prefix_scan_lanes( // lane level prefix sum
    uint4 *vector_data) // lane counts -> lane prefix sums
{
    uint sum = 0;
    uint4 vector_temp = *vector_data;
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

    *vector_data = vector_temp;

    sum += tmp;
    return sum;
}

uint prefix_scan_vectors(
    const uint lane_level_prefix_sum, // partial prefix sum within single work item
    uint *total_sum, // total prefix sum of all work items
    __local uint *total_sums, // [0..127]: zeroed, [128..255]: prefix sums of all work items
    const size_t num_work_items)
{
    const size_t work_item = get_local_id(0);
    const size_t index = num_work_items + work_item;

    // [0..127]: zeroed, [128..255]: partial prefix sums
    total_sums[work_item] = 0;
    total_sums[index] = lane_level_prefix_sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    // [128..255]: total prefix sums
    for (uint i = 1; i <= (num_work_items / 2); i *= 2) {
        const uint partial_sum = total_sums[index - i];
        barrier(CLK_LOCAL_MEM_FENCE);
        total_sums[index] += partial_sum;
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Total prefix sum of all work items.
    *total_sum =
        total_sums[(num_work_items * 2) - 1];
    // Total prefix sum of work item.
    return total_sums[work_item + num_work_items - 1];
}

#define PREFIX_SCAN_WORKGROUP_SIZE 128

uint4 prefix_scan_128(
    uint4 vector_histogram,
    uint *total_sum,
    __local uint *total_sums)
{
    const uint lane_prefix_sum = prefix_scan_lanes(
        &vector_histogram);
    const uint vector_prefix_sum = prefix_scan_vectors(
        lane_prefix_sum, total_sum, total_sums, PREFIX_SCAN_WORKGROUP_SIZE);
    return vector_histogram + (uint4)(vector_prefix_sum);
}

#define PREFIX_SCAN_LANES 4
#define PREFIX_SCAN_LANE_ELEMENTS 5
#define PREFIX_SCAN_ELEMENTS (PREFIX_SCAN_LANES * PREFIX_SCAN_LANE_ELEMENTS)

__kernel __attribute__((reqd_work_group_size(PREFIX_SCAN_WORKGROUP_SIZE, 1, 1)))
void prefix_scan_kernel(
    __global uint *histogram, // [[c0_g0, c0_g1, ..., c0_gm], ..., [c15_g0, c15_g1, ..., c15_gm]] ->
                              // [[s0_g0, s0_g1, ..., s0_gm], ..., [s15_g0, s15_g1, ..., s15_gm]]
    const int num_workgroups)
{
    const size_t element_counts_size = VALUES_PER_PASS * num_workgroups;
    const size_t work_item_base = get_local_id(0) * PREFIX_SCAN_ELEMENTS;

    // Each element of 'scalar_histogram' contains count of possibly
    // ascending four bit values.
    uint scalar_histogram[PREFIX_SCAN_ELEMENTS];
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        scalar_histogram[i] =
            (index < element_counts_size) ?
            histogram[index] :
            0;
    }

    // The 20 elements of 'scalar_histogram' are divided into 4 lanes
    // containing 5 elements each. The 4 elements of
    // 'vector_histogram' contain the number of values in each lane.
    uint4 vector_histogram = (uint4)(0);
    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        vector_histogram.x +=
            scalar_histogram[(0 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.y +=
            scalar_histogram[(1 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.z +=
            scalar_histogram[(2 * PREFIX_SCAN_LANE_ELEMENTS) + element];
        vector_histogram.w +=
            scalar_histogram[(3 * PREFIX_SCAN_LANE_ELEMENTS) + element];
    }

    // After the call, 'unused_top_level_prefix_sum' contains the
    // number of values in 'histogram'.
    uint unused_top_level_prefix_sum;
    // The elements of 'histogram' are divided into 20-element
    // vectors. After the call, 'unused_vector_level_sums' contains
    // prefix sums of the vectors.
    __local uint unused_vector_level_prefix_sums[PREFIX_SCAN_WORKGROUP_SIZE * 2];
    // The combined prefix sum of at lane and vector levels is retured.
    const uint4 lane_and_vector_level_prefix_sum = prefix_scan_128(
        vector_histogram,
        &unused_top_level_prefix_sum,
        unused_vector_level_prefix_sums);

    // scalar_histogram[index] = prefix sum of elements within corresponding lane
    for (size_t lane = 0; lane < PREFIX_SCAN_LANES; ++lane) {
        uint sum = 0;
        for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
            const size_t index = (lane * PREFIX_SCAN_LANE_ELEMENTS) + element;
            const uint tmp = scalar_histogram[index];
            scalar_histogram[index] = sum;
            sum += tmp;
        }
    }

    // scalar_histogram[index] = prefix sum of elements within lane
    //                         + prefix sum of lanes within vector
    //                         + prefix sum of vectors
    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        scalar_histogram[(0 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
            lane_and_vector_level_prefix_sum.x;
        scalar_histogram[(1 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
            lane_and_vector_level_prefix_sum.y;
        scalar_histogram[(2 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
            lane_and_vector_level_prefix_sum.z;
        scalar_histogram[(3 * PREFIX_SCAN_LANE_ELEMENTS) + element] +=
            lane_and_vector_level_prefix_sum.w;
    }

    // Copy work item result into work group result.
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        histogram[index] =
            scalar_histogram[i];
    }
}

///////////////////////////////////
// PHASE 3: Not implemented yet. //
///////////////////////////////////
