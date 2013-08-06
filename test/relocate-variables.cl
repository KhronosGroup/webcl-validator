// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

typedef struct {
    int int_value;
    char char_value;
    float float_value;
    float4 vector_value;
} PrimitiveStruct;

typedef struct {
    int *int_pointer;
    char *char_pointer;
    float *float_pointer;
    float4 *vector_pointer;
} PrivatePrimitivePointerStruct;

typedef struct {
    __local int *int_pointer;
    __local char *char_pointer;
    __local float *float_pointer;
    __local float4 *vector_pointer;
} LocalPrimitivePointerStruct;

typedef struct {
    __constant int *int_pointer;
    __constant char *char_pointer;
    __constant float *float_pointer;
    __constant float4 *vector_pointer;
} ConstantPrimitivePointerStruct;

typedef struct {
    int int_array[1];
    char char_array[1];
    float float_array[1];
    float4 vector_array[1];
} ArrayStruct;

typedef struct {
    int (*int_array_pointer)[1];
    char (*char_array_pointer)[1];
    float (*float_array_pointer)[1];
    float4 (*vector_array_pointer)[1];
} PrivateArrayPointerStruct;

typedef struct {
    __local int (*int_array_pointer)[1];
    __local char (*char_array_pointer)[1];
    __local float (*float_array_pointer)[1];
    __local float4 (*vector_array_pointer)[1];
} LocalArrayPointerStruct;

typedef struct {
    __constant int (*int_array_pointer)[1];
    __constant char (*char_array_pointer)[1];
    __constant float (*float_array_pointer)[1];
    __constant float4 (*vector_array_pointer)[1];
} ConstantArrayPointerStruct;

typedef struct {
    PrimitiveStruct primitive_struct;
    PrimitiveStruct *primitive_struct_pointer;
    PrimitiveStruct primitive_struct_array[1];
    PrivatePrimitivePointerStruct primitive_pointer_struct;
    PrivatePrimitivePointerStruct *primitive_pointer_struct_pointer;
    PrivatePrimitivePointerStruct primitive_pointer_struct_array[1];
    ArrayStruct array_struct;
    ArrayStruct *array_struct_pointer;
    ArrayStruct array_struct_array[1];
    PrivateArrayPointerStruct array_pointer_struct;
    PrivateArrayPointerStruct *array_pointer_struct_pointer;
    PrivateArrayPointerStruct array_pointer_struct_array[1];
} PrivateMainStruct;

typedef struct {
    PrimitiveStruct primitive_struct;
    __local PrimitiveStruct *primitive_struct_pointer;
    PrimitiveStruct primitive_struct_array[1];
    LocalPrimitivePointerStruct primitive_pointer_struct;
    __local LocalPrimitivePointerStruct *primitive_pointer_struct_pointer;
    LocalPrimitivePointerStruct primitive_pointer_struct_array[1];
    ArrayStruct array_struct;
    __local ArrayStruct *array_struct_pointer;
    ArrayStruct array_struct_array[1];
    LocalArrayPointerStruct array_pointer_struct;
    __local LocalArrayPointerStruct *array_pointer_struct_pointer;
    LocalArrayPointerStruct array_pointer_struct_array[1];
} LocalMainStruct;

typedef struct {
    PrimitiveStruct primitive_struct;
    __constant PrimitiveStruct *primitive_struct_pointer;
    PrimitiveStruct primitive_struct_array[1];
    ConstantPrimitivePointerStruct primitive_pointer_struct;
    __constant ConstantPrimitivePointerStruct *primitive_pointer_struct_pointer;
    ConstantPrimitivePointerStruct primitive_pointer_struct_array[1];
    ArrayStruct array_struct;
    __constant ArrayStruct *array_struct_pointer;
    ArrayStruct array_struct_array[1];
    ConstantArrayPointerStruct array_pointer_struct;
    __constant ConstantArrayPointerStruct *array_pointer_struct_pointer;
    ConstantArrayPointerStruct array_pointer_struct_array[1];
} ConstantMainStruct;

__constant int constant_value = 1;
__constant int * __constant constant_pointer = &constant_value;
__constant int constant_array[1] = { 2 };

__constant int constant_int_value = 0;
__constant char constant_char_value = '\0';
__constant float constant_float_value = 0.0f;
__constant float4 constant_vector_value = ((float4)(0.0f));
__constant PrimitiveStruct constant_primitive_struct = {
    0, '\0', 0.0f, ((float4)(0.0f))
};
__constant ConstantPrimitivePointerStruct constant_primitive_pointer_struct = {
    &constant_int_value,
    &constant_char_value,
    &constant_float_value,
    &constant_vector_value
};
__constant ArrayStruct constant_array_struct = {
    { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) }
};
__constant ConstantArrayPointerStruct constant_array_pointer_struct = {
    &constant_array_struct.int_array,
    &constant_array_struct.char_array,
    &constant_array_struct.float_array,
    &constant_array_struct.vector_array
};
__constant ConstantMainStruct constant_main_struct = {
    { 0, '\0', 0.0f, ((float4)(0.0f)) },
    &constant_primitive_struct,
    { { 0, '\0', 0.0f, ((float4)(0.0f)) } },

    { 0, 0, 0, 0 },
    &constant_primitive_pointer_struct,
    { { 0, 0, 0, 0 } },

    { { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) } },
    &constant_array_struct,
    { { { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) } } },

    { 0, 0, 0, 0 },
    &constant_array_pointer_struct,
    { { 0, 0, 0, 0 } }
};

int relocate_private_variables(void);

int relocate_private_variables()
{
    int int_value = 0;
    char char_value = '\0';
    float float_value = 0.0f;
    float4 vector_value = ((float4)(0.0f));

    PrimitiveStruct empty_primitive_struct;
    empty_primitive_struct.int_value = int_value;
    empty_primitive_struct.char_value = char_value;
    empty_primitive_struct.float_value = float_value;
    empty_primitive_struct.vector_value = vector_value;

    PrimitiveStruct constant_primitive_struct = {
        0, '\0', 0.0f, ((float4)(0.0f))
    };

    PrimitiveStruct primitive_struct = {
        int_value,
        char_value,
        float_value,
        vector_value
    };

    PrivatePrimitivePointerStruct empty_primitive_pointer_struct;
    empty_primitive_pointer_struct.int_pointer = &empty_primitive_struct.int_value;
    empty_primitive_pointer_struct.char_pointer = &empty_primitive_struct.char_value;
    empty_primitive_pointer_struct.float_pointer = &empty_primitive_struct.float_value;
    empty_primitive_pointer_struct.vector_pointer = &empty_primitive_struct.vector_value;

    PrivatePrimitivePointerStruct constant_primitive_pointer_struct = {
        &constant_primitive_struct.int_value,
        &constant_primitive_struct.char_value,
        &constant_primitive_struct.float_value,
        &constant_primitive_struct.vector_value
    };

    PrivatePrimitivePointerStruct primitive_pointer_struct = {
        &primitive_struct.int_value,
        &primitive_struct.char_value,
        &primitive_struct.float_value,
        &primitive_struct.vector_value
    };

    ArrayStruct empty_array_struct;
    empty_array_struct.int_array[0] = *empty_primitive_pointer_struct.int_pointer;
    empty_array_struct.char_array[0] = *empty_primitive_pointer_struct.char_pointer;
    empty_array_struct.float_array[0] = *empty_primitive_pointer_struct.float_pointer;
    empty_array_struct.vector_array[0] = *empty_primitive_pointer_struct.vector_pointer;

    ArrayStruct constant_array_struct = {
        { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) }
    };

    ArrayStruct array_struct = {
        { *primitive_pointer_struct.int_pointer },
        { *primitive_pointer_struct.char_pointer },
        { *primitive_pointer_struct.float_pointer },
        { *primitive_pointer_struct.vector_pointer }
    };

    PrivateArrayPointerStruct empty_array_pointer_struct;
    empty_array_pointer_struct.int_array_pointer = &empty_array_struct.int_array;
    empty_array_pointer_struct.char_array_pointer = &empty_array_struct.char_array;
    empty_array_pointer_struct.float_array_pointer = &empty_array_struct.float_array;
    empty_array_pointer_struct.vector_array_pointer = &empty_array_struct.vector_array;

    PrivateArrayPointerStruct constant_array_pointer_struct = {
        &constant_array_struct.int_array,
        &constant_array_struct.char_array,
        &constant_array_struct.float_array,
        &constant_array_struct.vector_array
    };

    PrivateArrayPointerStruct array_pointer_struct = {
        &array_struct.int_array,
        &array_struct.char_array,
        &array_struct.float_array,
        &array_struct.vector_array
    };

    PrivateMainStruct empty_main_struct;
    empty_main_struct.primitive_struct = empty_primitive_struct;
    empty_main_struct.primitive_struct_pointer = &empty_primitive_struct;
    empty_main_struct.primitive_struct_array[0] = empty_primitive_struct;

    empty_main_struct.primitive_pointer_struct = empty_primitive_pointer_struct;
    empty_main_struct.primitive_pointer_struct_pointer = &empty_primitive_pointer_struct;
    empty_main_struct.primitive_pointer_struct_array[0] = empty_primitive_pointer_struct;

    empty_main_struct.array_struct = empty_array_struct;
    empty_main_struct.array_struct_pointer = &empty_array_struct;
    empty_main_struct.array_struct_array[0] = empty_array_struct;

    empty_main_struct.array_pointer_struct = empty_array_pointer_struct;
    empty_main_struct.array_pointer_struct_pointer = &empty_array_pointer_struct;
    empty_main_struct.array_pointer_struct_array[0] = empty_array_pointer_struct;

    PrivateMainStruct constant_main_struct = {
        { 0, '\0', 0.0f, ((float4)(0.0f)) },
        &constant_primitive_struct,
        { { 0, '\0', 0.0f, ((float4)(0.0f)) } },

        { 0, 0, 0, 0 },
        &constant_primitive_pointer_struct,
        { { 0, 0, 0, 0 } },

        { { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) } },
        &constant_array_struct,
        { { { 0 }, { '\0' }, { 0.0f }, { ((float4)(0.0f)) } } },

        { 0, 0, 0, 0 },
        &constant_array_pointer_struct,
        { { 0, 0, 0, 0 } }
    };

    PrivateMainStruct main_struct = {
        primitive_struct,
        &primitive_struct,
        { primitive_struct },

        primitive_pointer_struct,
        &primitive_pointer_struct,
        { primitive_pointer_struct },

        array_struct,
        &array_struct,
        { array_struct },

        array_pointer_struct,
        &array_pointer_struct,
        { array_pointer_struct }
    };

    return empty_main_struct.primitive_struct.int_value +
        main_struct.primitive_struct.int_value;
}

__kernel void relocate_variables(
    // CHECK: __global int *result, unsigned long wcl_result_size)
    __global int *result)
{
    int value = constant_value;
    int *pointer = &value;
    int array[1] = { *pointer };

    __local int local_value;
    local_value = value + 1;
    __local int * __local local_pointer;
    local_pointer = &local_value;
    __local int local_array[1];
    local_array[0] = *local_pointer;

    __private int private_value = local_value + 1;
    __private int * /* __private */ private_pointer = &private_value;
    __private int private_array[1] = { *private_pointer };

    __local int int_value;
    int_value = 0;
    __local char char_value;
    char_value = '\0';
    __local float float_value;
    float_value = 0.0f;
    __local float4 vector_value;
    vector_value = ((float4)(0.0f));

    __local PrimitiveStruct primitive_struct;
    primitive_struct.int_value = int_value;
    primitive_struct.char_value = char_value;
    primitive_struct.float_value = float_value;
    primitive_struct.vector_value = vector_value;

    __local LocalPrimitivePointerStruct primitive_pointer_struct;
    primitive_pointer_struct.int_pointer = &primitive_struct.int_value;
    primitive_pointer_struct.char_pointer = &primitive_struct.char_value;
    primitive_pointer_struct.float_pointer = &primitive_struct.float_value;
    primitive_pointer_struct.vector_pointer = &primitive_struct.vector_value;

    __local ArrayStruct array_struct;
    array_struct.int_array[0] = *primitive_pointer_struct.int_pointer;
    array_struct.char_array[0] = *primitive_pointer_struct.char_pointer;
    array_struct.float_array[0] = *primitive_pointer_struct.float_pointer;
    array_struct.vector_array[0] = *primitive_pointer_struct.vector_pointer;

    __local LocalArrayPointerStruct array_pointer_struct;
    array_pointer_struct.int_array_pointer = &array_struct.int_array;
    array_pointer_struct.char_array_pointer = &array_struct.char_array;
    array_pointer_struct.float_array_pointer = &array_struct.float_array;
    array_pointer_struct.vector_array_pointer = &array_struct.vector_array;

    __local LocalMainStruct main_struct;
    main_struct.primitive_struct = primitive_struct;
    main_struct.primitive_struct_pointer = &primitive_struct;
    main_struct.primitive_struct_array[0] = primitive_struct;

    main_struct.primitive_pointer_struct = primitive_pointer_struct;
    main_struct.primitive_pointer_struct_pointer = &primitive_pointer_struct;
    main_struct.primitive_pointer_struct_array[0] = primitive_pointer_struct;

    main_struct.array_struct = array_struct;
    main_struct.array_struct_pointer = &array_struct;
    main_struct.array_struct_array[0] = array_struct;

    main_struct.array_pointer_struct = array_pointer_struct;
    main_struct.array_pointer_struct_pointer = &array_pointer_struct;
    main_struct.array_pointer_struct_array[0] = array_pointer_struct;

    result[value] = array[0] + local_array[0] +
        constant_array[0] + private_array[0] +
        relocate_private_variables();
}
