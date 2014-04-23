// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

typedef __read_only image2d_t myimage;

// CHECK: error: Invalid qualifier for typedef type
__kernel void image2d_typedef(__read_only myimage image) {

}

// CHECK-NOT: error: Invalid qualifier for typedef type
__kernel void image2d_typedef2(myimage image) {

}
