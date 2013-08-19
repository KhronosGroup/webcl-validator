// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s | grep -v "Processing\|CHECK" | %FileCheck %s

// CHECK

// TODO: explain what this test is exactly testing? preprocessing is 
//       done and undefined ifdef is removed?
__kernel void attack(__global int *array)
{
    const int i = get_global_id(0);

    // CHECK-NOT: #ifdef __ONLY_DEFINED_IN_OPENCL_DRIVER__
    // CHECK-NOT:     _wcl_allocs = 0;
    // CHECK-NOT: #endif
#ifdef __ONLY_DEFINED_IN_OPENCL_DRIVER__
    _wcl_allocs = 0;
#endif

    array[i] = i;
}
// CHECK: #endif // cl_khr_initialize_memory
