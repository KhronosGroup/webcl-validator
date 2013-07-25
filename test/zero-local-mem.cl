// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

void init_local_mem(
    __local int *int_array, int int_value,
    __local char *char_array, char char_value,
    __local float *float_array, float float_value,
    __local float4 *vector_array, float4 vector_value)
{
    int_array[0] = int_value;
    char_array[0] = char_value;
    float_array[0] = float_value;
    vector_array[0] = vector_value;
}

__kernel void zero_local_mem(
    __global bool *result,
    __local int *int_array,
    __local char *char_array,
    __local float *float_array,
    __local float4 *vector_array)
{
    // Each range should be zeroed.

    // CHECK: _WCL_LOCAL_RANGE_INIT(_wcl_allocs->ll._wcl_locals_min, _wcl_allocs->ll._wcl_locals_max);
    // CHECK: _WCL_LOCAL_RANGE_INIT(_wcl_allocs->ll.zero_local_mem__int_array_min, _wcl_allocs->ll.zero_local_mem__int_array_max);
    // CHECK: _WCL_LOCAL_RANGE_INIT(_wcl_allocs->ll.zero_local_mem__char_array_min, _wcl_allocs->ll.zero_local_mem__char_array_max);
    // CHECK: _WCL_LOCAL_RANGE_INIT(_wcl_allocs->ll.zero_local_mem__float_array_min, _wcl_allocs->ll.zero_local_mem__float_array_max);
    // CHECK: _WCL_LOCAL_RANGE_INIT(_wcl_allocs->ll.zero_local_mem__vector_array_min, _wcl_allocs->ll.zero_local_mem__vector_array_max);
    // CHECK: barrier(CLK_LOCAL_MEM_FENCE);

    // Initialization should occur after zeroing.

    __local int int_variable;
    // CHECK: _wcl_locals.zero_local_mem__int_variable = 1;
    int_variable = 1;
    __local char char_variable;
    // CHECK: _wcl_locals.zero_local_mem__char_variable = 'a';
    char_variable = 'a';
    __local float float_variable;
    // CHECK: _wcl_locals.zero_local_mem__float_variable = 1.0f;
    float_variable = 1.0f;
    __local float4 vector_variable;
    // CHECK: _wcl_locals.zero_local_mem__vector_variable = ((float4)(1.0f));
    vector_variable = ((float4)(1.0f));

    const int i = get_global_id(0);

    init_local_mem(int_array, int_variable,
                   char_array, char_variable,
                   float_array, float_variable,
                   vector_array, vector_variable);

    result[i] =
        (int_array[i] == int_variable) &&
        (char_array[i] == char_variable) &&
        (float_array[i] == float_variable) &&
        (vector_array[i].x == vector_variable.x);
}
