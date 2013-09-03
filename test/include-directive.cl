// RUN: %opencl-validator -I%include/.. < %s
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// We don't allow include directives in main source files.
// CHECK: error: WebCL doesn't support the include directive.
#include "include/1st-level.h"
// We allow include directives in included files.
// CHECK-NOT: error: WebCL doesn't support the include directive.

__kernel void use_function_from_include_directive(
    __global int *array)
{
    const int i = get_global_id(0);
    array[i] = first_level_function(i);
}
