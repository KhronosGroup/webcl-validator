// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void safe_image2d(image2d_t image,
                           __constant int *ptr)
{
    // CHECK-NOT: error: Builtin argument check is required.
    nonexisting(image);
    // CHECK: error: Builtin argument check is required.
    nonexisting2(ptr);
}
