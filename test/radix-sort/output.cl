// RUN: cat %s | %opencl-validator
// RUN: cat %s | %radix-sort -transform

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

typedef struct {
    uint scalar_histogram[20];
    uint4 vector_histogram;
    uint unused_top_level_prefix_sum;
} WclPrivates;

typedef struct {
    uint histogram_matrix[1024];
    uint unused_vector_level_prefix_sums[256];
} WclLocals;

typedef struct {
} WclConstants;

typedef struct {
} WclGlobals;

typedef struct {
    WclPrivates __private *privates;
    WclLocals __local *locals;
    WclConstants __constant *constants;
    WclGlobals __global *globals;
} WclAddressSpaces;

#define WCL_MIN(a, b)                           \
    (((a) < (b)) ? (a) : (b))
#define WCL_MAX(a, b)                           \
    (((a) < (b)) ? (b) : (a))
#define WCL_CLAMP(low, value, high)             \
    WCL_MAX((low), WCL_MIN((value), (high)))

#define WCL_MIN_PTR(name, type, field)          \
    ((name type *)(field))
#define WCL_MAX_PTR(name, type, field)          \
    (WCL_MIN_PTR(name, type, (field) + 1) - 1)
#define WCL_MIN_IDX(name, type, field, ptr)     \
    0
    // negative indices need to be taken into account
    // (ptr - WCL_MIN_PTR(name, type, field))
#define WCL_MAX_IDX(name, type, field, ptr)     \
    (WCL_MAX_PTR(name, type, field) - ptr)

#define WCL_PTR_CHECKER(name, field, type)                              \
    name type *wcl_##name##_##type##_ptr(                               \
        WclAddressSpaces *as, name type *ptr);                          \
    name type *wcl_##name##_##type##_ptr(                               \
        WclAddressSpaces *as, name type *ptr)                           \
    {                                                                   \
        return WCL_CLAMP(WCL_MIN_PTR(name, type, as->field),            \
                         ptr,                                           \
                         WCL_MAX_PTR(name, type, as->field));           \
    }
#define WCL_IDX_CHECKER(name, field, type)                              \
    size_t wcl_##name##_##type##_idx(                                   \
        WclAddressSpaces *as, name type *ptr, size_t idx);              \
    size_t wcl_##name##_##type##_idx(                                   \
        WclAddressSpaces *as, name type *ptr, size_t idx)               \
    {                                                                   \
        return WCL_CLAMP((size_t)WCL_MIN_IDX(name, type, as->field, ptr),       \
                         idx,                                           \
                         (size_t)WCL_MAX_IDX(name, type, as->field, ptr));      \
    }

WCL_PTR_CHECKER(private, privates, uint)
WCL_PTR_CHECKER(private, privates, uint4)
WCL_IDX_CHECKER(private, privates, uint4)
WCL_IDX_CHECKER(local, locals, uint)

size_t wcl_idx(size_t idx, size_t limit);

size_t wcl_idx(size_t idx, size_t limit)
{
    return idx % limit;
}

////////////////////////////
// PHASE 1: Count values. //
////////////////////////////

#define STREAM_COUNT_WORKGROUP_SIZE 64
#define BITS_PER_PASS 4
#define VALUES_PER_PASS (1 << BITS_PER_PASS)

// prototypes for apple
uint get_histogram_index(uint value);
void set_histogram(WclAddressSpaces *wcl_as, __local uint *histogram, uint value, uint count);
void clear_histogram(WclAddressSpaces *wcl_as, __local uint *histogram);
void inc_histogram(WclAddressSpaces *wcl_as, __local uint *histogram, uint value);

uint get_histogram_index(
    uint value)
{
    // [value][get_local_id(0)]
    return (value * STREAM_COUNT_WORKGROUP_SIZE) + get_local_id(0);
}

void set_histogram(
    WclAddressSpaces *wcl_as,
    __local uint *histogram,
    uint value,
    uint count)
{
    histogram[wcl_local_uint_idx(wcl_as, histogram, get_histogram_index(value))] = count;
}

void clear_histogram(
    WclAddressSpaces *wcl_as,
    __local uint *histogram)
{
    for (int value = 0; value < VALUES_PER_PASS; ++value) {
        set_histogram(wcl_as, histogram, value, 0);
    }
}

void inc_histogram(
    WclAddressSpaces *wcl_as,
    __local uint *histogram,
    uint value)
{
    ++histogram[wcl_local_uint_idx(wcl_as, histogram, get_histogram_index(value))];
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
    __local WclLocals wcl_ls;
    WclAddressSpaces wcl_as = { 0, &wcl_ls, 0, 0 };

    // histogram_matrix[VALUES_PER_PASS][STREAM_COUNT_WORKGROUP_SIZE];
#if 0
    __local uint histogram_matrix[VALUES_PER_PASS * STREAM_COUNT_WORKGROUP_SIZE];
#endif
    clear_histogram(
        &wcl_as, wcl_ls.histogram_matrix);
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
            const uint value =
                unsorted_elements[wcl_idx(index, wcl_unsorted_elements_size)];
            const uint pass_value = (value >> bit_offset) & VALUE_MASK;
            inc_histogram(
                &wcl_as, wcl_ls.histogram_matrix,
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
            sum += wcl_ls.histogram_matrix[wcl_idx(index, 1024)];
        }
        const size_t sum_index = (get_local_id(0) * num_workgroups) + get_group_id(0);
        element_counts[wcl_idx(sum_index, wcl_element_counts_size)] = sum;
    }
}

////////////////////////////////////
// PHASE 2: Calculate prefix sum. //
////////////////////////////////////

// prototypes for apple driver
uint prefix_scan_lanes(WclAddressSpaces *wcl_as, uint4 *vector_data);
uint prefix_scan_vectors(WclAddressSpaces *wcl_as, const uint lane_level_prefix_sum, uint *total_sum, __local uint *total_sums, const size_t num_work_items);
uint4 prefix_scan_128(WclAddressSpaces *wcl_as, uint4 vector_histogram, uint *total_sum, __local uint *total_sums);

uint prefix_scan_lanes( // lane level prefix sum
    WclAddressSpaces *wcl_as,
    uint4 *vector_data) // lane counts -> lane prefix sums
{
    uint sum = 0;
    uint tmp = (*wcl_private_uint4_ptr(wcl_as, vector_data)).x;
    (*wcl_private_uint4_ptr(wcl_as, vector_data)).x = sum;

    sum += tmp;
    tmp = (*wcl_private_uint4_ptr(wcl_as, vector_data)).y;
    (*wcl_private_uint4_ptr(wcl_as, vector_data)).y = sum;

    sum += tmp;
    tmp = vector_data[wcl_private_uint4_idx(wcl_as, vector_data, 0)].z;
    vector_data[wcl_private_uint4_idx(wcl_as, vector_data, 0)].z = sum;

    sum += tmp;
    tmp = vector_data[wcl_private_uint4_idx(wcl_as, vector_data, 0)].w;
    vector_data[wcl_private_uint4_idx(wcl_as, vector_data, 0)].w = sum;

    sum += tmp;
    return sum;
}

uint prefix_scan_vectors(
    WclAddressSpaces *wcl_as,
    const uint lane_level_prefix_sum, // partial prefix sum within single work item
    uint *total_sum, // total prefix sum of all work items
    __local uint *total_sums, // [0..127]: zeroed, [128..255]: prefix sums of all work items
    const size_t num_work_items)
{
    const size_t work_item = get_local_id(0);
    const size_t index = num_work_items + work_item;

    // [0..127]: zeroed, [128..255]: partial prefix sums
    total_sums[wcl_local_uint_idx(wcl_as, total_sums, work_item)] = 0;
    total_sums[wcl_local_uint_idx(wcl_as, total_sums, index)] = lane_level_prefix_sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    // [128..255]: total prefix sums
    for (uint i = 1; i <= (num_work_items / 2); i *= 2) {
//        const uint partial_sum = total_sums[wcl_local_uint_idx(wcl_as, total_sums, index - i)];
        const uint partial_sum = total_sums[index - i];
        barrier(CLK_LOCAL_MEM_FENCE);
        total_sums[wcl_local_uint_idx(wcl_as, total_sums, index)] += partial_sum;
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // Total prefix sum of all work items.
    *wcl_private_uint_ptr(wcl_as, total_sum) =
        total_sums[wcl_local_uint_idx(wcl_as, total_sums, (num_work_items * 2) - 1)];
    // Total prefix sum of work item.
    return total_sums[wcl_local_uint_idx(wcl_as, total_sums, work_item + num_work_items - 1)];
}

#define PREFIX_SCAN_WORKGROUP_SIZE 128

uint4 prefix_scan_128(
    WclAddressSpaces *wcl_as,
    uint4 vector_histogram,
    uint *total_sum,
    __local uint *total_sums)
{
    wcl_as->privates->vector_histogram = vector_histogram;
    const uint lane_prefix_sum = prefix_scan_lanes(
        wcl_as,
        &wcl_as->privates->vector_histogram);
    const uint vector_prefix_sum = prefix_scan_vectors(
        wcl_as,
        lane_prefix_sum, total_sum, total_sums, PREFIX_SCAN_WORKGROUP_SIZE);
    return wcl_as->privates->vector_histogram + (uint4)(vector_prefix_sum);
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
    __private WclPrivates wcl_ps;
    __local WclLocals wcl_ls;
    WclAddressSpaces wcl_as = { &wcl_ps, &wcl_ls, 0, 0 };

    const size_t element_counts_size = VALUES_PER_PASS * num_workgroups;
    const size_t work_item_base = get_local_id(0) * PREFIX_SCAN_ELEMENTS;

    // Each element of 'scalar_histogram' contains count of possibly
    // ascending four bit values.
#if 0
    uint scalar_histogram[PREFIX_SCAN_ELEMENTS];
#endif
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        wcl_ps.scalar_histogram[wcl_idx(i, 20)] =
            (index < element_counts_size) ?
            histogram[wcl_idx(index, wcl_histogram_size)] :
            0;
    }

    // The 20 elements of 'scalar_histogram' are divided into 4 lanes
    // containing 5 elements each. The 4 elements of
    // 'vector_histogram' contain the number of values in each lane.
#if 0
    uint4 vector_histogram = (uint4)(0);
#endif
    wcl_ps.vector_histogram = (uint4)(0);
    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        wcl_ps.vector_histogram.x +=
            wcl_ps.scalar_histogram[wcl_idx((0 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)];
        wcl_ps.vector_histogram.y +=
            wcl_ps.scalar_histogram[wcl_idx((1 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)];
        wcl_ps.vector_histogram.z +=
            wcl_ps.scalar_histogram[wcl_idx((2 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)];
        wcl_ps.vector_histogram.w +=
            wcl_ps.scalar_histogram[wcl_idx((3 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)];
    }

    // After the call, 'unused_top_level_prefix_sum' contains the
    // number of values in 'histogram'.
#if 0
    uint unused_top_level_prefix_sum;
#endif
    // The elements of 'histogram' are divided into 20-element
    // vectors. After the call, 'unused_vector_level_sums' contains
    // prefix sums of the vectors.
#if 0
    __local uint unused_vector_level_prefix_sums[PREFIX_SCAN_WORKGROUP_SIZE * 2];
#endif
    // The combined prefix sum of at lane and vector levels is retured.
    const uint4 lane_and_vector_level_prefix_sum = prefix_scan_128(
        &wcl_as,
        wcl_ps.vector_histogram,
        &wcl_ps.unused_top_level_prefix_sum,
        wcl_ls.unused_vector_level_prefix_sums);

    // scalar_histogram[index] = prefix sum of elements within corresponding lane
    for (size_t lane = 0; lane < PREFIX_SCAN_LANES; ++lane) {
        uint sum = 0;
        for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
            const size_t index = (lane * PREFIX_SCAN_LANE_ELEMENTS) + element;
            const uint tmp = wcl_ps.scalar_histogram[wcl_idx(index, 20)];
            wcl_ps.scalar_histogram[wcl_idx(index, 20)] = sum;
            sum += tmp;
        }
    }

    // scalar_histogram[index] = prefix sum of elements within lane
    //                         + prefix sum of lanes within vector
    //                         + prefix sum of vectors
    for (size_t element = 0; element < PREFIX_SCAN_LANE_ELEMENTS; ++element) {
        wcl_ps.scalar_histogram[wcl_idx((0 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)] +=
            lane_and_vector_level_prefix_sum.x;
        wcl_ps.scalar_histogram[wcl_idx((1 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)] +=
            lane_and_vector_level_prefix_sum.y;
        wcl_ps.scalar_histogram[wcl_idx((2 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)] +=
            lane_and_vector_level_prefix_sum.z;
        wcl_ps.scalar_histogram[wcl_idx((3 * PREFIX_SCAN_LANE_ELEMENTS) + element, 20)] +=
            lane_and_vector_level_prefix_sum.w;
    }

    // Copy work item result into work group result.
    for (size_t i = 0; i < PREFIX_SCAN_ELEMENTS; ++i) {
        const size_t index = work_item_base + i;
        histogram[wcl_idx(index, wcl_histogram_size)] =
            wcl_ps.scalar_histogram[wcl_idx(i, 20)];
    }
}

///////////////////////////////////
// PHASE 3: Not implemented yet. //
///////////////////////////////////
