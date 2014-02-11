// RUN: %webcl-validator %s >&/dev/null

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void dummy()
{
    half a;
}
