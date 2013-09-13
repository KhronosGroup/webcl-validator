// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator

int get_value(int value);

// CHECK: struct _Wcl3Struct
// CHECK: struct _Wcl2Struct
// CHECK: struct _WclStruct
__constant // __PLATFORM_AMD__
struct {
    struct {
        struct {
            int f;
        } b;
    } a;
// CHECK: struct _WclStruct c1 = {{{ 1 }}}, c2 = {{{ 2 }}}
} c1 = {{{ 1 }}}, c2 = {{{ 2 }}};

// CHECK: struct _Wcl6Struct
// CHECK: struct _Wcl5Struct
// CHECK: struct _Wcl4Struct
__constant struct {
    struct {
        struct {
            int f;
        } b;
    } a;
// CHECK: __constant struct _Wcl4Struct c3 = {{{ 3 }}}, c4 = {{{ 4 }}}
} c3 = {{{ 3 }}}, c4 = {{{ 4 }}};

int get_value(int value)
{
    // CHECK: struct _Wcl9Struct
    // CHECK: struct _Wcl8Struct
    // CHECK: struct _Wcl7Struct
    struct {
        struct {
            struct {
                int f;
            } b;
        } a;
    // CHECK: struct _Wcl7Struct p1 = {{{ value + 1 }}}, p2 = {{{ value  + 2 }}}
    } p1 = {{{ value + 1 }}}, p2 = {{{ value  + 2 }}};

    // CHECK: struct _Wcl12Struct
    // CHECK: struct _Wcl11Struct
    // CHECK: struct _Wcl10Struct
    __private struct {
        struct {
            struct {
                int f;
            } b;
        } a;
    // CHECK: __private struct _Wcl10Struct p3 = {{{ value + 3 }}}, p4 = {{{ value + 4 }}}
    } p3 = {{{ value + 3 }}}, p4 = {{{ value + 4 }}};

    return p1.a.b.f + p2.a.b.f + p3.a.b.f + p4.a.b.f;
}

// CHECK: struct _Wcl15Struct
// CHECK: struct _Wcl14Struct
// CHECK: struct _Wcl13Struct
__constant // __PLATFORM_AMD__
struct {
    struct {
        struct {
            int f;
        } b;
    } a;
// CHECK: struct _Wcl13Struct c5 = {{{ 5 }}}, c6 = {{{ 6 }}}
} c5 = {{{ 5 }}}, c6 = {{{ 6 }}};

// CHECK: struct _Wcl18Struct
// CHECK: struct _Wcl17Struct
// CHECK: struct _Wcl16Struct
__constant struct {
    struct {
        struct {
            int f;
        } b;
    } a;
// CHECK: __constant struct _Wcl16Struct c7 = {{{ 7 }}}, c8 = {{{ 8 }}}
} c7 = {{{ 7 }}}, c8 = {{{ 8 }}};

__kernel void access_array(
    // CHECK: __global int *array, unsigned long _wcl_array_size)
    __global int *array)
{
    // CHECK: struct _Wcl21Struct
    // CHECK: struct _Wcl20Struct
    // CHECK: struct _Wcl19Struct
    __local struct {
        struct {
            struct {
                int f;
            } b;
        } a;
    // CHECK: __local struct _Wcl19Struct l1, l2;
    } l1, l2;

    l1.a.b.f = 1;
    l2.a.b.f = 2;

    const int i = get_global_id(0);
    array[i] =
        c1.a.b.f + c2.a.b.f + c3.a.b.f + c4.a.b.f +
        c5.a.b.f + c6.a.b.f + c7.a.b.f + c8.a.b.f +
        l1.a.b.f + l2.a.b.f + get_value(i);
}
