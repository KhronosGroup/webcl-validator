// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h 2>&1 | grep -v CHECK | %FileCheck %s

struct main_struct {
    int value;
};

typedef struct main_struct typedef_struct;

// Kernel structure parameters aren't allowed.
__kernel void kernel_with_structure_parameters(
    __global int *array,
// CHECK: error: WebCL doesn't support structures as kernel parameters.
    struct main_struct m,
// CHECK: error: WebCL doesn't support structures as kernel parameters.
    typedef_struct t)
{
    const int i = get_global_id(0);
    array[i] = m.value + t.value;
}
