// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// CHECK: error: global variables must have a constant address space qualifier
int global_int;
