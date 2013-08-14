// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

int rename_private(int name);

int rename_private(int name)
{
    // CHECK: _wcl_allocs->pa._wcl_name =
    int *name_pointer = &name;
    int result = name;

    {
        // CHECK: _wcl_allocs->pa._wcl_2_name =
        int name = *name_pointer;
        int *name_pointer = &name;
        result += *name_pointer;
        {
            // CHECK: _wcl_allocs->pa._wcl_3_name =
            int name = *name_pointer;
            int *name_pointer = &name;
            result += *name_pointer;
        }
    }

#ifdef RELOCATION_OF_FOR_DECLARATION

    for (int name = 0; name < 5; ++name) {
        int *name_pointer = &name;
        result += *name_pointer;
        for (int name = 0; name < 5; ++name) {
            int *name_pointer = &name;
            result += *name_pointer;
        }
    }

#endif

    return result;
}

__kernel void rename_variables(
    __global int *result)
{
    const size_t i = get_global_id(0);

    __local int name;
    // CHECK: _wcl_locals._wcl_name =
    name = *result;
    __local int *name_pointer = &name;

    {
        __local int name;
        // CHECK: _wcl_locals._wcl_2_name =
        name = *name_pointer;
        __local int *name_pointer = &name;
        result[i] += *name_pointer;
        {
            __local int name;
            // CHECK: _wcl_locals._wcl_3_name =
            name = *name_pointer;
            __local int *name_pointer = &name;
            result[i] += *name_pointer;
        }
    }

    result[i] = rename_private(result[i]);
}
