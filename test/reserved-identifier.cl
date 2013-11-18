// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s -ferror-limit=0 2>&1 | grep "error: " | %FileCheck %s

struct _wcl_struct { int _wcl_field; };
struct _WclStruct { int _WclField; };
struct _WCL_STRUCT { int _WCL_FIELD; };

typedef struct { int _wcl_field; } _wcl_typedef;
typedef struct { int _WclField; } _WclTypedef;
typedef struct { int _WCL_FIELD; } _WCL_TYPEDEF;

int _wcl_function(int _wcl_parameter);
int _WclFunction(int _WclParameter);
int _WCL_FUNCTION(int _WCL_PARAMETER);

int _wcl_function(int _wcl_parameter)
{
    struct _wcl_struct _wcl_variable = { 1 };
    return _wcl_variable._wcl_field + _wcl_parameter;
}

int _WclFunction(int _WclParameter)
{
    struct _WclStruct _WclVariable = { 2 };
    return _WclVariable._WclField + _WclParameter;
}

int _WCL_FUNCTION(int _WCL_PARAMETER)
{
    struct _WCL_STRUCT _WCL_VARIABLE = { 3 };
    return _WCL_VARIABLE._WCL_FIELD + _WCL_PARAMETER;
}

__kernel void _wcl_reserved_identifier(__global int *_wcl_array)
{
    _wcl_typedef _wcl_variable = { 0 };
    _WclTypedef _WclVariable = { 0 };
    _WCL_TYPEDEF _WCL_VARIABLE = { 0 };

    _wcl_array[get_global_id(0)] =
        _wcl_function(_wcl_variable._wcl_field) +
        _WclFunction(_WclVariable._WclField) +
        _WCL_FUNCTION(_WCL_VARIABLE._WCL_FIELD);
}

// CHECK-DAG: error: Identifier '_WCL_FIELD' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WCL_FUNCTION' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WCL_PARAMETER' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WCL_STRUCT' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WCL_TYPEDEF' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WCL_VARIABLE' uses reserved prefix '_WCL'.
// CHECK-DAG: error: Identifier '_WclField' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_WclFunction' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_WclParameter' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_WclStruct' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_WclTypedef' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_WclVariable' uses reserved prefix '_Wcl'.
// CHECK-DAG: error: Identifier '_wcl_array' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_field' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_function' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_parameter' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_reserved_identifier' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_struct' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_typedef' uses reserved prefix '_wcl'.
// CHECK-DAG: error: Identifier '_wcl_variable' uses reserved prefix '_wcl'.
// CHECK-NOT: fatal error: too many errors emitted
