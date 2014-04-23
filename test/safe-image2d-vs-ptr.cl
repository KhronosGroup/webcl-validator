// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

__kernel void safe_image2d(image2d_t image,
                           __constant int *ptr)
{
    // CHECK-NOT: error: Unsafe builtin not recognized.
    nonexisting(image);
    // CHECK: error: Unsafe builtin not recognized.
    nonexisting2(ptr);
}
