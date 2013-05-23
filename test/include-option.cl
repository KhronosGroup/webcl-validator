// RUN: cat %include/1st-level.h %s | %opencl-validator -I%include
// RUN: %webcl-validator %s -- -x cl -I%include -include _kernel.h -include 1st-level.h

// This should be OK. We aren't using the include directive.
__kernel void use_function_from_include_option(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = first_level_function(i);
}
