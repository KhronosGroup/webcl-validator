// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// We don't want complaints from builtin function declarations.
// CHECK-NOT: error: WebCL doesn't support 3D images.

typedef image3d_t typedef_image;

// prototypes for apple driver
int function_with_3d_image_parameters(image3d_t m, typedef_image t);

int function_with_3d_image_parameters(
// CHECK: error: WebCL doesn't support 3D images.
    image3d_t m,
// CHECK: error: WebCL doesn't support 3D images.
    typedef_image t)
{
    return get_image_width(m) + get_image_height(t);
}

__kernel void kernel_with_3d_image_parameters(
    __global int *array,
// CHECK: error: WebCL doesn't support 3D images.
    image3d_t m,
// CHECK: error: WebCL doesn't support 3D images.
    typedef_image t)
{
    const int i = get_global_id(0);
    array[i] = function_with_3d_image_parameters(m, t);
}
