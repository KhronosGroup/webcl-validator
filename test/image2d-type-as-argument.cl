// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void foo(image2d_t image, int i)
{
    // CHECK-NOT: error: image2d_t must always originate from parameters
    nonexisting(image); // 1
    // CHECK: error: image2d_t must always originate from parameters
    nonexisting((image2d_t) i); // 2
}

__kernel void image2d_type_as_argument(image2d_t image, __constant int *ptr)
{
    image2d_t image2;
    // CHECK-NOT: error: image2d_t must always originate from parameters
    nonexisting(image); // 3
    // CHECK: error: image2d_t must always originate from parameters
    nonexisting(image2); // 4
    // CHECK-NOT: error: image2d_t must always originate from parameters
    foo(image, 0); // 5
    // CHECK: error: image2d_t must always originate from parameters
    foo(image2, 0); // 6
    // CHECK: error: image2d_t must always originate from parameters
    foo((image2d_t) 0, 0); // 7
    // CHECK: image2d_t must always be used as a function argument
    *(int*) image = 42; // 8
    // CHECK: image2d_t must always be used as a function argument
    image; // 9
}
