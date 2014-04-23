// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// not enabled:
//#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

__kernel void atoms(volatile __global long* global_long_data, volatile __local long* local_long_data,
                      volatile __global ulong* global_ulong_data, volatile __local ulong* local_ulong_data,
                      volatile __global float* global_float_data, volatile __local float* local_float_data)
{
  long val_long = 1;

  // CHECK: error: no matching function for call to 'atomic_add'
  val_long = atomic_add    (global_long_data, val_long);
}
