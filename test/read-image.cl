// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void unsafe_builtins(
    image2d_t image)
{
    // CHECK-NOT: warning: implicit declaration of function 'read_imagef'
    // CHECK: read_imagef( image,/* transformed */ CLK_ADDRESS_NONE | CLK_FILTER_LINEAR | CLK_NORMALIZED_COORDS_TRUE, (float2)(0, 0));
    read_imagef( image, CLK_FILTER_LINEAR | CLK_ADDRESS_NONE | CLK_NORMALIZED_COORDS_TRUE, (float2)(0, 0));
    // CHECK-NOT: warning: implicit declaration of function 'read_imagef'
    // CHECK: read_imagef(image,/* transformed */ CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE, (float2)(0, 0));
    read_imagef(image, 3, (float2)(0, 0));
    // CHECK-NOT: warning: implicit declaration of function 'read_imagei'
    read_imagei(image // f,oo
    // CHECK: /*,bar , * */,/* transformed */ CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE,
                /*,bar , * */,4/1,
                (int2)(1, 2));
}
