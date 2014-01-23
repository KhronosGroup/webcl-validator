// RUN: %webcl-validator %s 2>/dev/null

void foo(void);

__kernel void declared_function_test(void)
{
  foo();
}

void foo(void) {
  // ok
}

