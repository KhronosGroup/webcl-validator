// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void unsupported_builtins(
    const __global float4 *input,
    int input_stride,
    __global float4 *output,
    int output_stride,
    event_t event_k)
{
    __local float4 local_array[10];

    // CHECK: error: WebCL doesn't support prefetch.
    prefetch(input, input_stride);

    // CHECK: error: WebCL doesn't support async_work_group_copy.
    event_t event_1 = async_work_group_copy(
        local_array, input,
        sizeof(local_array) / sizeof(local_array[0]),
        event_k);
    // CHECK: error: WebCL doesn't support async_work_group_copy.
    event_t event_2 = async_work_group_copy(
        output, local_array,
        sizeof(local_array) / sizeof(local_array[0]),
        event_1);

    // CHECK: error: WebCL doesn't support async_work_group_strided_copy.
    event_t event_3 = async_work_group_strided_copy(
        local_array, input,
        sizeof(local_array) / sizeof(local_array[0]),
        input_stride,
        event_2);
    // CHECK: error: WebCL doesn't support async_work_group_strided_copy.
    event_t event_4 = async_work_group_strided_copy(
        output, local_array,
        sizeof(local_array) / sizeof(local_array[0]),
        output_stride,
        event_3);

    // CHECK: error: WebCL doesn't support wait_group_events.
    wait_group_events(1, &event_4);
}
