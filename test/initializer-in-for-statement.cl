// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>&1 | grep "Cannot currently relocate variables declared inside for statement"

__kernel void initializer_in_for_statement(__global int *array)
{
    // this should be asserted since we do not allow relocation of
    // variables initialized in for statement 
    for (int i = 0, k = 100; i < 1; i++) {
        int* ptr = (&k);
        ptr++;
    }
}
