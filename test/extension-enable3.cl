// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK-DAG: error: use of undeclared identifier 'double2'
// CHEcK-DAG: error: use of type 'double' requires cl_khr_fp64 extension to be enabled

__kernel void dummy(
    __global int *array)
{
    double i = 42;
    double2 i2;
}
