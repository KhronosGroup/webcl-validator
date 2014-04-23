// RUN: %webcl-validator "%s" 2>&1 | grep -v CHECK | %FileCheck "%s"

// CHECK: Constant address space variables must be initialized.
__constant int global_const_int_fail;

// CHECK-NOT: Constant address space variables must be initialized.
__constant int global_const_int_ok = 1;
