// image2d_t argument should not be relocated
// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

void bar(image2d_t image)
{
    int i;
    int *ptr = &i;
    vload4(0, ptr);
}

__kernel void image2d_type_as_argument(image2d_t image)
{
    // CHECK: bar(_wcl_allocs, image);
    bar(image);
}
