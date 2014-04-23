// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// Not enabled:
//#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

__kernel void atoms(volatile __global long* global_int_data, volatile __local long* local_int_data,
                      volatile __global ulong* global_uint_data, volatile __local ulong* local_uint_data,
                      volatile __global float* global_float_data, volatile __local float* local_float_data)
{
  long val_int = 1;
  ulong val_uint = 2;
  uint cmp = 1;

  // CHECK: error: no matching function for call to 'atomic_min'
  val_int = atomic_min    (global_int_data, val_int);
}
