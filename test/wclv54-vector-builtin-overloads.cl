// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

kernel void vectorMathBuiltinBug() 
{
// CHECK-NOT: error: passing 'float4' to parameter of incompatible type 'double'
// CHECK:    "version" : "1.0",
      float4 foo = sqrt((float4)(1.0f));
}
