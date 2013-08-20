// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2> /dev/null| grep -v "Processing\|CHECK" | %kernel-runner --webcl --kernel zero_private --webcl --kernel zero_private --gcount 40 --global int 40 | grep ABCDEFG

__kernel void zero_private(__global char* output, __global int *test) {
    const size_t i = get_global_id(0);
    output[i] = 'A' + test[i];
}
