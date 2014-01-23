// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// check only one return argument per argument length, per argument type and per address space

// int global
// CHECK-DAG: return atomic_add(_
// CHECK-DAG: return atomic_inc(_
// CHECK-DAG: return atomic_cmpxchg(_

// uint global
// CHECK-DAG: atomic_add(_
// CHECK-DAG: return atomic_inc(_
// CHECK-DAG: return atomic_cmpxchg(_

// int local
// CHECK-DAG: return atomic_add(_
// CHECK-DAG: return atomic_inc(_
// CHECK-DAG: return atomic_cmpxchg(_

// uint local
// CHECK-DAG: return atomic_add(_
// CHECK-DAG: return atomic_inc(_
// CHECK-DAG: return atomic_cmpxchg(_

// float global
// CHECK-DAG: return atomic_xchg(_

// float local
// CHECK-DAG: return atomic_xchg(_

__kernel void atomics(volatile __global int* global_int_data, volatile __local int* local_int_data,
                      volatile __global uint* global_uint_data, volatile __local uint* local_uint_data,
                      volatile __global float* global_float_data, volatile __local float* local_float_data)
{
  int val_int = 1;
  int val_uint = 2;
  uint cmp = 1;
  float val_float = 3;

  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_add    (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_sub    (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data, val_int);
  val_int = atomic_xchg   (global_int_data, val_int);
  // CHECK: (_wcl_allocs, global_int_data);
  val_int = atomic_inc    (global_int_data);
  // CHECK: (_wcl_allocs, global_int_data);
  val_int = atomic_dec    (global_int_data);
  // CHECK: (_wcl_allocs, global_int_data, cmp, val_int);
  val_int = atomic_cmpxchg(global_int_data, cmp, val_int);
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
  val_uint = atomic_add    (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_sub    (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data, val_uint);
  val_uint = atomic_xchg   (global_uint_data, val_uint);
  // CHECK: (_wcl_allocs, global_uint_data);
  val_uint = atomic_inc    (global_uint_data);
  // CHECK: (_wcl_allocs, global_uint_data);
  val_uint = atomic_dec    (global_uint_data);
  // CHECK: (_wcl_allocs, global_uint_data, cmp, val_uint);
  val_uint = atomic_cmpxchg(global_uint_data, cmp, val_uint);
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
  val_int = atomic_add    (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_sub    (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data, val_int);
  val_int = atomic_xchg   (local_int_data, val_int);
  // CHECK: (_wcl_allocs, local_int_data);
  val_int = atomic_inc    (local_int_data);
  // CHECK: (_wcl_allocs, local_int_data);
  val_int = atomic_dec    (local_int_data);
  // CHECK: (_wcl_allocs, local_int_data, cmp, val_int);
  val_int = atomic_cmpxchg(local_int_data, cmp, val_int);
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

  // CHECK:  (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_add    (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_sub    (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data, val_uint);
  val_uint = atomic_xchg   (local_uint_data, val_uint);
  // CHECK: (_wcl_allocs, local_uint_data);
  val_uint = atomic_inc    (local_uint_data);
  // CHECK: (_wcl_allocs, local_uint_data);
  val_uint = atomic_dec    (local_uint_data);
  // CHECK: (_wcl_allocs, local_uint_data, cmp, val_uint);
  val_uint = atomic_cmpxchg(local_uint_data, cmp, val_uint);
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

  // CHECK: (_wcl_allocs, global_float_data, val_float);
  val_float = atomic_xchg(global_float_data, val_float);
  // CHECK: (_wcl_allocs, local_float_data, val_float);
  val_float = atomic_xchg(local_float_data, val_float);
}
