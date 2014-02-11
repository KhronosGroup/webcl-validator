// RUN: %webcl-validator %s >&/dev/null

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void dummy(
    __global int *array)
{
  double i = 42;
  double2 i2;
}
