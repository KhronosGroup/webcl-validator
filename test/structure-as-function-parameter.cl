// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing" | %opencl-validator

struct main_struct {
    int value;
};

typedef struct main_struct typedef_struct;

// prototype declarations for apple driver
int function_with_structure_parameters(struct main_struct m, typedef_struct t);

// This should be OK. Only kernel parameter types are limited.
int function_with_structure_parameters(
    struct main_struct m,
    typedef_struct t)
{
    return m.value + t.value;
}

__kernel void dummy(int dum) {
}
