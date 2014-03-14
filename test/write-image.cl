// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s
// RUN: %webcl-validator %s | %kernel-runner --webcl --kernel write_image --image 10 10 | grep '9 (0.00, 9.00, 3.00, 4.00), (1.00, 9.00, 3.00, 4.00), (2.00, 9.00, 3.00, 4.00), (3.00, 9.00, 3.00, 4.00), (4.00, 9.00, 3.00, 4.00), (5.00, 9.00, 3.00, 4.00), (6.00, 9.00, 3.00, 4.00), (7.00, 9.00, 3.00, 4.00), (8.00, 9.00, 3.00, 4.00), (9.00, 9.00, 3.00, 4.00)'

// Instrumentation does not guarantee sequence numbers of functions
// CHECK-DAG: void _wcl_write_imagei_[[IMAGEI:[0-4]]](write_only image2d_t arg0

__kernel void write_image(
    __global char* output,
    write_only image2d_t image)
{
    // CHECK-DAG:  _wcl_write_imagei_[[IMAGEI]]
    write_imagei(image, (int2)(0, 0), (int4)(1, 2, 3, 4));
    // CHECK-DAG:  _wcl_write_imageui
    write_imageui(image, (int2)(0, 0), (uint4)(1, 2, 3, 4));
    // CHECK-DAG:  _wcl_write_imagef
    write_imagef(image, (int2)(0, 0), (float4)(1, 2, 3, 4));

    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            write_imagef(image, (int2)(x, y), (float4)(x, y, 3, 4));
        }
    }

    // NOTE: this runs super slow on GPU, better to use get_global_id(0) and
    //       give global size to runner
    for (int y = 10; y < 10000; ++y) {
        for (int x = 10; x < 1000; ++x) {
            write_imagef(image, (int2)(x, y), (float4)(x, y, 3, 4));
        }
    }

    for (int y = -10; y > -10000; --y) {
        for (int x = -10; x > -1000; --x) {
            write_imagef(image, (int2)(x, y), (float4)(x, y, 3, 4));
        }
    }
}
