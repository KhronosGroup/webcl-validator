// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

__kernel void attack(__global int *array)
{
    const int i = get_global_id(0);

    // Check that we don't leave branching preprocessor directives to
    // the validated sources. Otherwise a malicious user could nullify
    // our validations.
    //
    // CHECK-NOT: #ifdef __ONLY_DEFINED_IN_OPENCL_DRIVER__
    // CHECK-NOT:     _wcl_allocs = 0;
    // CHECK-NOT: #endif // __ONLY_DEFINED_IN_OPENCL_DRIVER__
#ifdef __ONLY_DEFINED_IN_OPENCL_DRIVER__
    _wcl_allocs = 0;
#endif // __ONLY_DEFINED_IN_OPENCL_DRIVER__

    array[i] = i;
}
