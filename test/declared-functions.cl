// RUN: %webcl-validator "%s" 2>/dev/null

// ok, defined later
void foo(void);

// ok, not called
void foo2(void);

__kernel void declared_function_test(void)
{
  foo();
}

void foo(void) {
  // ok
}

