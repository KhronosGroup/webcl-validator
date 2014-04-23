// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

#define paste(X, Y) X##Y

__kernel void goto2_test(void)
{
 label:
  // CHECK: error: WebCL does not support goto
  paste(go,to) label;
}
