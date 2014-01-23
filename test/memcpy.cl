// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: error: All declared functions must be defined
void memcpy(long dest, size_t n);

// CHECK: error: All declared functions must be defined
void some_other_fn(void* ptr);

__kernel void memcpy_test(void)
{
  char buffer[4];
  memcpy((int) buffer, 4);
}
