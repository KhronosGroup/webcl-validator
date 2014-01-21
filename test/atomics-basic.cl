// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

// check only one return argument per argument length, per argument type and per address space

// long global
// CHECK: return atomic_add(_WCL_ADDR_CLAMP_global_3(volatile __global long *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn), arg1);
// CHECK: return atomic_inc(_WCL_ADDR_CLAMP_global_3(volatile __global long *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn));
// CHECK: return atomic_cmpxchg(_WCL_ADDR_CLAMP_global_3(volatile __global long *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn), arg1, arg2);

// ulong global
// CHECK: return atomic_add(_WCL_ADDR_CLAMP_global_3(volatile __global ulong *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn), arg1);
// CHECK: return atomic_inc(_WCL_ADDR_CLAMP_global_3(volatile __global ulong *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn));
// CHECK: return atomic_cmpxchg(_WCL_ADDR_CLAMP_global_3(volatile __global ulong *, arg0, 1, _wcl_allocs->gl.atoms__global_long_data_min, _wcl_allocs->gl.atoms__global_long_data_max, _wcl_allocs->gl.atoms__global_ulong_data_min, _wcl_allocs->gl.atoms__global_ulong_data_max, _wcl_allocs->gl.atoms__global_float_data_min, _wcl_allocs->gl.atoms__global_float_data_max, _wcl_allocs->gn), arg1, arg2);

// long local
// CHECK: return atomic_add(_WCL_ADDR_CLAMP_local_3(volatile __local long *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln), arg1);
// CHECK: return atomic_inc(_WCL_ADDR_CLAMP_local_3(volatile __local long *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln));
// CHECK: return atomic_cmpxchg(_WCL_ADDR_CLAMP_local_3(volatile __local long *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln), arg1, arg2);

// ulong local
// CHECK: return atomic_add(_WCL_ADDR_CLAMP_local_3(volatile __local ulong *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln), arg1);
// CHECK: return atomic_inc(_WCL_ADDR_CLAMP_local_3(volatile __local ulong *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln));
// CHECK: return atomic_cmpxchg(_WCL_ADDR_CLAMP_local_3(volatile __local ulong *, arg0, 1, _wcl_allocs->ll.atoms__local_long_data_min, _wcl_allocs->ll.atoms__local_long_data_max, _wcl_allocs->ll.atoms__local_ulong_data_min, _wcl_allocs->ll.atoms__local_ulong_data_max, _wcl_allocs->ll.atoms__local_float_data_min, _wcl_allocs->ll.atoms__local_float_data_max, _wcl_allocs->ln), arg1, arg2);

__kernel void atoms(volatile __global long* global_long_data, volatile __local long* local_long_data,
                      volatile __global ulong* global_ulong_data, volatile __local ulong* local_ulong_data,
                      volatile __global float* global_float_data, volatile __local float* local_float_data)
{
  long val_long = 1;
  ulong val_ulong = 2;
  ulong cmp = 1;

  // CHECK: (_wcl_allocs, global_long_data, val_long);
  val_long = atomic_add    (global_long_data, val_long);
  // CHECK: (_wcl_allocs, global_long_data, val_long);
  val_long = atomic_sub    (global_long_data, val_long);
  // CHECK: (_wcl_allocs, global_long_data, val_long);
  val_long = atomic_xchg   (global_long_data, val_long);
  // CHECK: (_wcl_allocs, global_long_data);
  val_long = atomic_inc    (global_long_data);
  // CHECK: (_wcl_allocs, global_long_data);
  val_long = atomic_dec    (global_long_data);
  // CHECK: (_wcl_allocs, global_long_data, cmp, val_long);
  val_long = atomic_cmpxchg(global_long_data, cmp, val_long);

  // CHECK: (_wcl_allocs, global_ulong_data, val_ulong);
  val_ulong = atomic_add    (global_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, global_ulong_data, val_ulong);
  val_ulong = atomic_sub    (global_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, global_ulong_data, val_ulong);
  val_ulong = atomic_xchg   (global_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, global_ulong_data);
  val_ulong = atomic_inc    (global_ulong_data);
  // CHECK: (_wcl_allocs, global_ulong_data);
  val_ulong = atomic_dec    (global_ulong_data);
  // CHECK: (_wcl_allocs, global_ulong_data, cmp, val_ulong);
  val_ulong = atomic_cmpxchg(global_ulong_data, cmp, val_ulong);

  // CHECK: (_wcl_allocs, local_long_data, val_long);
  val_long = atomic_add    (local_long_data, val_long);
  // CHECK: (_wcl_allocs, local_long_data, val_long);
  val_long = atomic_sub    (local_long_data, val_long);
  // CHECK: (_wcl_allocs, local_long_data, val_long);
  val_long = atomic_xchg   (local_long_data, val_long);
  // CHECK: (_wcl_allocs, local_long_data);
  val_long = atomic_inc    (local_long_data);
  // CHECK: (_wcl_allocs, local_long_data);
  val_long = atomic_dec    (local_long_data);
  // CHECK: (_wcl_allocs, local_long_data, cmp, val_long);
  val_long = atomic_cmpxchg(local_long_data, cmp, val_long);

  // CHECK:  (_wcl_allocs, local_ulong_data, val_ulong);
  val_ulong = atomic_add    (local_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, local_ulong_data, val_ulong);
  val_ulong = atomic_sub    (local_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, local_ulong_data, val_ulong);
  val_ulong = atomic_xchg   (local_ulong_data, val_ulong);
  // CHECK: (_wcl_allocs, local_ulong_data);
  val_ulong = atomic_inc    (local_ulong_data);
  // CHECK: (_wcl_allocs, local_ulong_data);
  val_ulong = atomic_dec    (local_ulong_data);
  // CHECK: (_wcl_allocs, local_ulong_data, cmp, val_ulong);
  val_ulong = atomic_cmpxchg(local_ulong_data, cmp, val_ulong);
}
