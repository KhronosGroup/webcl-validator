// RUN: %webcl-validator %s 2>&1 | grep -v CHECK | %FileCheck %s

// CHECK: error: Global scope variables must be in constant address space.
sampler_t globalsampler1;

// CHECK: error: Constant address space variables must be initialized.
__constant sampler_t globalsampler2;
