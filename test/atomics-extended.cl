// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

// check only one return argument per argument length, per argument type and per address space

// int global
// CHECK-DAG: return atomic_min(_

// uint global
// CHECK-DAG: return atomic_min(_

// int local
// CHECK-DAG: return atomic_min(_

// uint local
// CHECK-DAG: return atomic_min(_

__kernel void atoms(volatile __global int* global_int_data, volatile __local int* local_int_data,
                      volatile __global uint* global_uint_data, volatile __local uint* local_uint_data,
                      volatile __global float* global_float_data, volatile __local float* local_float_data)
{
  long val_int = 1;
  ulong val_uint = 2;
  uint cmp = 1;

  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_min    (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_max    (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_and    (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_or     (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_xor    (global_int_data, val_int);

  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_min    (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_max    (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_and    (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_or     (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_xor    (global_uint_data, val_uint);

  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_min    (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_max    (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_and    (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_or     (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_xor    (local_int_data, val_int);

  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_min    (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_max    (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_and    (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_or     (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_xor    (local_uint_data, val_uint);
}
