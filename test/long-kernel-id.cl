// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h | grep -v CHECK | %FileCheck %s

// Long kernel names aren't allowed.
// CHECK: error: WebCL restricts kernel name lengths to 255 characters.
__kernel void a_kernel_identifier_that_is_256_characters_long_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_12345678(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = i;
}
