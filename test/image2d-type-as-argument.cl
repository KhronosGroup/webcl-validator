// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void foo(image2d_t image, int i)
{
    // CHECK: error: used type 'image2d_t' where arithmetic or pointer type is required
    nonexisting((image2d_t) i); // 2
}

__kernel void image2d_type_as_argument(image2d_t image, __constant int *ptr)
{
    // CHECK: error: used type 'image2d_t' where arithmetic or pointer type is required
    foo((image2d_t) 0, 0); // 7
    // CHECK: error: operand of type 'image2d_t' where arithmetic or pointer type is required
    *(int*) image = 42; // 8
    // CHECK: error: operand of type 'image2d_t' where arithmetic or pointer type is required
    (int) image; // 12
}
