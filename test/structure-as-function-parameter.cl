// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s -- -x cl -include %include/_kernel.h

struct main_struct {
    int value;
};

typedef struct main_struct typedef_struct;

// This should be OK. Only kernel parameter types are limited.
int function_with_structure_parameters(
    struct main_struct m,
    typedef_struct t)
{
    return m.value + t.value;
}
