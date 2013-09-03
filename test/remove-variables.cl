// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator

#ifdef __PLATFORM_AMD__
#error "Avoiding further problems!"
#endif

__constant int removed_constant_1 = 1;
__constant int * __constant removed_constant_pointer_1 = &removed_constant_1;
__constant int removed_constant_array_1[1] = { 1 };

__constant int preserved_constant_f2 = 2, removed_constant_l2 = 2;
__constant int * __constant preserved_constant_pointer_f2 = &removed_constant_l2, * __constant removed_constant_pointer_l2 = &removed_constant_l2;
__constant int preserved_constant_array_f2[1] = { 2 }, removed_constant_array_l2[1] = { 2 };

__constant int preserved_constant_f3 = 3, removed_constant_l3 = 3;
__constant int * __constant preserved_constant_pointer_f3 = &removed_constant_l3, * __constant removed_constant_pointer_l3 = &removed_constant_l3;
__constant int preserved_constant_array_f3[1] = { 3 }, removed_constant_array_l3[1] = { 3 };

__constant int removed_constant_f4 = 4, preserved_constant_l4 = 4;
__constant int * __constant removed_constant_pointer_f4 = &removed_constant_f4, * __constant preserved_constant_pointer_l4 = &removed_constant_f4;
__constant int removed_constant_array_f4[1] = { 4 }, preserved_constant_array_l4[1] = { 4 };

__constant int removed_constant_f5 = 5, preserved_constant_l5 = 5;
__constant int * __constant removed_constant_pointer_f5 = &removed_constant_f5, * __constant preserved_constant_pointer_l5 = &removed_constant_f5;
__constant int removed_constant_array_f5[1] = { 5 }, preserved_constant_array_l5[1] = { 5 };

__constant int preserved_constant_f6 = 6, removed_constant_m6 = 6, preserved_constant_l6 = 6;
__constant int * __constant preserved_constant_pointer_f6 = &removed_constant_m6, * __constant removed_constant_pointer_m6 = &removed_constant_m6, * __constant preserved_constant_pointer_l6 = &removed_constant_m6;
__constant int preserved_constant_array_f6[1] = { 6 }, removed_constant_array_m6[1] = { 6 }, preserved_constant_array_l6[1] = { 6 };

__constant int preserved_constant_f7 = 7, removed_constant_m7 = 7, preserved_constant_l7 = 7;
__constant int * __constant preserved_constant_pointer_f7 = &removed_constant_m7, * __constant removed_constant_pointer_m7 = &removed_constant_m7, * __constant preserved_constant_pointer_l7 = &removed_constant_m7;
__constant int preserved_constant_array_f7[1] = { 7 }, removed_constant_array_m7[1] = { 7 }, preserved_constant_array_l7[1] = { 7 };

__constant int removed_constant_f8 = 8, preserved_constant_m8 = 8, removed_constant_l8 = 8;
__constant int * __constant removed_constant_pointer_f8 = &removed_constant_f8, * __constant preserved_constant_pointer_m8 = &removed_constant_f8, * __constant removed_constant_pointer_l8 = &removed_constant_f8;
__constant int removed_constant_array_f8[1] = { 8 }, preserved_constant_array_m8[1] = { 8 }, removed_constant_array_l8[1] = { 8 };

__constant int removed_constant_f9 = 9, preserved_constant_m9 = 9, removed_constant_l9 = 9;
__constant int * __constant removed_constant_pointer_f9 = &removed_constant_f9, * __constant preserved_constant_pointer_m9 = &removed_constant_f9, * __constant removed_constant_pointer_l9 = &removed_constant_l9;
__constant int removed_constant_array_f9[1] = { 9 }, preserved_constant_array_m9[1] = { 9 }, removed_constant_array_l9[1] = { 9 };

__constant int removed_constant_f10 = 10, removed_constant_m10 = 10, removed_constant_l10 = 10;
__constant int * __constant removed_constant_pointer_f10 = &removed_constant_m10, * __constant removed_constant_pointer_m10 = &removed_constant_m10, * __constant removed_constant_pointer_l10 = &removed_constant_m10;
__constant int removed_constant_array_f10[1] = { 10 }, removed_constant_array_m10[1] = { 10 }, removed_constant_array_l10[1] = { 10 };

__constant int removed_constant_f11 = 11, removed_constant_m11 = 11, removed_constant_l11 = 11;
__constant int * __constant removed_constant_pointer_f11 = &removed_constant_f11, * __constant removed_constant_pointer_m11 = &removed_constant_m11, * __constant removed_constant_pointer_l11 = &removed_constant_l11;
__constant int removed_constant_array_f11[1] = { 11 }, removed_constant_array_m11[1] = { 11 }, removed_constant_array_l11[1] = { 11 };

__kernel void remove_variables(
    // CHECK: __global int *result, unsigned long wcl_result_size)
    __global int *result)
{
    __local int removed_local_1;
    removed_local_1 = 1;
    __local int * __local removed_local_pointer_1;
    removed_local_pointer_1 = &removed_local_1;
    __local int removed_local_array_1[1];
    removed_local_array_1[0] = removed_local_1;

    __local int preserved_local_f2, removed_local_l2;
    preserved_local_f2 = 2;
    removed_local_l2 = preserved_local_f2;
    __local int * __local preserved_local_pointer_f2, * __local removed_local_pointer_l2;
    preserved_local_pointer_f2 = &removed_local_l2;
    removed_local_pointer_l2 = preserved_local_pointer_f2;
    __local int preserved_local_array_f2[1], removed_local_array_l2[1];
    preserved_local_array_f2[0] = preserved_local_f2;
    removed_local_array_l2[0] = preserved_local_array_f2[0];

    __local int removed_local_f3, preserved_local_l3;
    removed_local_f3 = 3;
    preserved_local_l3 = removed_local_f3;
    __local int * __local removed_local_pointer_f3, * __local preserved_local_pointer_l3;
    removed_local_pointer_f3 = &removed_local_f3;
    preserved_local_pointer_l3 = removed_local_pointer_f3;
    __local int removed_local_array_f3[1], preserved_local_array_l3[1];
    removed_local_array_f3[0] = removed_local_f3;
    preserved_local_array_l3[0] = removed_local_array_f3[0];

    __local int preserved_local_f4, removed_local_m4, preserved_local_l4;
    preserved_local_f4 = 4;
    removed_local_m4 = preserved_local_f4;
    preserved_local_l4 = removed_local_m4;
    __local int * __local preserved_local_pointer_f4, * __local removed_local_pointer_m4, * __local preserved_local_pointer_l4;
    preserved_local_pointer_f4 = &removed_local_m4;
    removed_local_pointer_m4 = preserved_local_pointer_f4;
    preserved_local_pointer_l4 = removed_local_pointer_m4;
    __local int preserved_local_array_f4[1], removed_local_array_m4[1], preserved_local_array_l4[1];
    preserved_local_array_f4[0] = preserved_local_f4;
    removed_local_array_m4[0] = preserved_local_array_f4[0];
    preserved_local_array_l4[0] = removed_local_array_m4[0];

    __local int removed_local_f5, preserved_local_m5, removed_local_l5;
    removed_local_f5 = 5;
    preserved_local_m5 = removed_local_f5;
    removed_local_l5 = preserved_local_m5;
    __local int * __local removed_local_pointer_f5, * __local preserved_local_pointer_m5, * __local removed_local_pointer_l5;
    removed_local_pointer_f5 = &removed_local_f5;
    preserved_local_pointer_m5 = removed_local_pointer_f5;
    removed_local_pointer_l5 = preserved_local_pointer_m5;
    __local int removed_local_array_f5[1], preserved_local_array_m5[1], removed_local_array_l5[1];
    removed_local_array_f5[0] = removed_local_f5;
    preserved_local_array_m5[0] = removed_local_array_f5[0];
    removed_local_array_l5[0] = preserved_local_array_m5[0];

    __local int removed_local_f6, removed_local_m6, removed_local_l6;
    removed_local_f6 = 6;
    removed_local_m6 = removed_local_f6;
    removed_local_l6 = removed_local_m6;
    __local int * __local removed_local_pointer_f6, * __local removed_local_pointer_m6, * __local removed_local_pointer_l6;
    removed_local_pointer_f6 = &removed_local_f6;
    removed_local_pointer_m6 = removed_local_pointer_f6;
    removed_local_pointer_l6 = removed_local_pointer_m6;
    __local int removed_local_array_f6[1], removed_local_array_m6[1], removed_local_array_l6[1];
    removed_local_array_f6[0] = removed_local_f6;
    removed_local_array_m6[0] = removed_local_array_f6[0];
    removed_local_array_l6[0] = removed_local_array_m6[0];

    int removed_constants =
        *(&removed_constant_1) + *removed_constant_pointer_1 + removed_constant_array_1[0] +
        *(&removed_constant_l2) + *removed_constant_pointer_l2 + removed_constant_array_l2[0] +
        *(&removed_constant_l3) + *removed_constant_pointer_l3 + removed_constant_array_l3[0] +
        *(&removed_constant_f4) + *removed_constant_pointer_f4 + removed_constant_array_f4[0] +
        *(&removed_constant_f5) + *removed_constant_pointer_f5 + removed_constant_array_f5[0] +
        *(&removed_constant_m6) + *removed_constant_pointer_m6 + removed_constant_array_m6[0] +
        *(&removed_constant_m7) + *removed_constant_pointer_m7 + removed_constant_array_m7[0] +
        *(&removed_constant_f8) + *removed_constant_pointer_f8 + removed_constant_array_f8[0] +
        *(&removed_constant_l8) + *removed_constant_pointer_l8 + removed_constant_array_l8[0] +
        *(&removed_constant_f9) + *removed_constant_pointer_f9 + removed_constant_array_f9[0] +
        *(&removed_constant_l9) + *removed_constant_pointer_l9 + removed_constant_array_l9[0] +
        *(&removed_constant_f10) + *removed_constant_pointer_f10 + removed_constant_array_f10[0] +
        *(&removed_constant_m10) + *removed_constant_pointer_m10 + removed_constant_array_m10[0] +
        *(&removed_constant_l10) + *removed_constant_pointer_l10 + removed_constant_array_l10[0] +
        *(&removed_constant_f11) + *removed_constant_pointer_f11 + removed_constant_array_f11[0] +
        *(&removed_constant_m11) + *removed_constant_pointer_m11 + removed_constant_array_m11[0] +
        *(&removed_constant_l11) + *removed_constant_pointer_l11 + removed_constant_array_l11[0];

    int removed_locals =
        *(&removed_local_1) + *removed_local_pointer_1 + removed_local_array_1[0] +
        *(&removed_local_l2) + *removed_local_pointer_l2 + removed_local_array_l2[0] +
        *(&removed_local_f3) + *removed_local_pointer_f3 + removed_local_array_f3[0] +
        *(&removed_local_m4) + *removed_local_pointer_m4 + removed_local_array_m4[0] +
        *(&removed_local_f5) + *removed_local_pointer_f5 + removed_local_array_f5[0] +
        *(&removed_local_l5) + *removed_local_pointer_l5 + removed_local_array_l5[0] +
        *(&removed_local_f6) + *removed_local_pointer_f6 + removed_local_array_f6[0] +
        *(&removed_local_m6) + *removed_local_pointer_m6 + removed_local_array_m6[0] +
        *(&removed_local_l6) + *removed_local_pointer_l6 + removed_local_array_l6[0];

    result[get_global_id(0)] = removed_constants + removed_locals;
}
