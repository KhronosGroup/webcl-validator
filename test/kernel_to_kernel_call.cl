// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

//CHECK: error: Calling kernels is not allowed.

kernel void bar() {
}

kernel void foo() {
	bar();
}