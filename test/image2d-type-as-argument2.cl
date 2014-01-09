// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

void foo(image2d_t image, int i)
{
    // CHECK: warning: implicit declaration of function 'nonexisting' is invalid in C99 [-Wimplicit-function-declaration]
    // CHECK-NOT: error: image2d_t must always originate from parameters
    nonexisting(image); // 1
}

void bar(image2d_t *image)
{
    // this code involved image2d_t*. it's ok, because you cannot get an image2d_t*
    // CHECK-NOT: error: image2d_t must always originate from parameters
    *(int*) image = 42;
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
    // CHECK: image2d_t must always be used as a function argument
    image; // 9
    // CHECK: image2d_t must always be used as a function argument
    bar(&image); // 11
}
