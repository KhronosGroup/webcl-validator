// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing:" | %check-empty-memory -transformed

__kernel void copy_local_mem(
    __global int *int_result,
    __global char *char_result,
    __global float *float_result,
    __global float4 *vector_result,
    __local int *int_array,
    __local char *char_array,
    __local float *float_array,
    __local float4 *vector_array)
{
    const int i = get_global_id(0);

    int_result[i] = int_array[i];
    char_result[i] = char_array[i];
    float_result[i] = float_array[i];
    vector_result[i] = vector_array[i];
}
