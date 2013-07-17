// RUN: cat %include/1st-level.h %s | %opencl-validator -I%include
// RUN: %webcl-validator %s -- -x cl -I%include -include _kernel.h -include 1st-level.h 2>/dev/null | grep -v "Processing\|CHECK\|DRIVER-MAY-REJECT" | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -I%include -include _kernel.h -include 1st-level.h 2>&1 | grep -v CHECK | %FileCheck %s

// This should be OK. We aren't using the include directive.
__kernel void use_function_from_include_option(
    __global int *array)
{
    const int i = get_global_id(0);
    // CHECK-NOT: warning: implicit declaration of function 'first_level_function'
    array[i] = i
        + first_level_function(i) // DRIVER-MAY-REJECT
        ;
}
