// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

__kernel void goto_test(void)
{
 label:
  // CHECK: error: WebCL does not support goto
  goto label;
}
