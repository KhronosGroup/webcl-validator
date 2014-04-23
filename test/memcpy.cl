// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void memcpy(long dest, size_t n);

// CHECK-NOT: error: All declared functions that are called must be defined
void some_other_fn(void* ptr);

__kernel void memcpy_test(void)
{
  char buffer[4];
  // CHECK: error: All declared functions that are called must be defined
  memcpy((int) buffer, 4);
}
