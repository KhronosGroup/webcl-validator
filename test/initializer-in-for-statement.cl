// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void initializer_in_for_statement(__global int *array)
{
    // This should produce an error since we don't currently allow
    // relocation of variables initialized in for statement.
    //
    // CHECK: error: Cannot currently relocate variables declared inside for statement.
    for (int i = 0, k = 100; i < 1; i++) {
        int* ptr = (&k);
        ptr++;
    }
}
