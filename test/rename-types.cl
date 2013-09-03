// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__constant struct { int constant_field; } anonymous_constant = { 0 };
__constant struct CommonStruct { int constant_field; } first_constant_struct = { 0 };
__constant struct CommonStruct second_constant_struct = { 0 };
typedef struct { int constant_field; } CommonTypedef;
__constant CommonTypedef constant_typedef = { 0 };

int rename_privates(void);

int rename_privates()
{
    struct { int private_field_1; } anonymous_private_1 = { 0 };
    struct CommonStruct { int private_field_1; } first_private_struct_1 = { 0 };
    struct CommonStruct second_private_struct_1 = { 0 };
    // CHECK: error: Identically named types aren't supported:
    typedef struct { int private_field_1; } CommonTypedef;
    CommonTypedef private_typedef_1 = { 0 };

    {
        struct { int private_field_2; } anonymous_private_2 = { 0 };
        struct CommonStruct { int private_field_2; } first_private_struct_2 = { 0 };
        struct CommonStruct second_private_struct_2 = { 0 };
        // CHECK: error: Identically named types aren't supported:
        typedef struct { int private_field_2; } CommonTypedef;
        CommonTypedef private_typedef_2 = { 0 };

        anonymous_private_1.private_field_1 = (&anonymous_private_2)->private_field_2;
        first_private_struct_1.private_field_1 = (&first_private_struct_2)->private_field_2;
        second_private_struct_1.private_field_1 = (&second_private_struct_2)->private_field_2;
        private_typedef_1.private_field_1 = (&private_typedef_2)->private_field_2;
    }

    return (&anonymous_private_1)->private_field_1 +
        (&first_private_struct_1)->private_field_1 +
        (&second_private_struct_1)->private_field_1 +
        (&private_typedef_1)->private_field_1;
}

__kernel void rename_types(
    __global int *result)
{
    __local struct { int local_field; } anonymous_local;
    anonymous_local.local_field = 0;
    __local struct CommonStruct { int local_field; } first_local_struct;
    first_local_struct.local_field = 0;
    __local struct CommonStruct second_local_struct;
    second_local_struct.local_field = 0;
    // CHECK: error: Identically named types aren't supported:
    typedef struct { int local_field; } CommonTypedef;
    __local CommonTypedef local_typedef;
    local_typedef.local_field = 0;

    result[get_global_id(0)] = rename_privates() +

        (&anonymous_constant)->constant_field +
        (&first_constant_struct)->constant_field +
        (&second_constant_struct)->constant_field +
        (&constant_typedef)->constant_field +

        (&anonymous_local)->local_field +
        (&first_local_struct)->local_field +
        (&second_local_struct)->local_field +
        (&local_typedef)->local_field;
}
