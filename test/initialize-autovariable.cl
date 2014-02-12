// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s
// RUN: %webcl-validator %s | sed 's@////@@' | %kernel-runner --webcl --kernel zero_private --gcount 1 | grep "123,0,0,0,0,0,0,0,123"

// verify that a and b are actually collected to private address space to make 
// sure that there is no unintialized variables left in code

typedef	struct {
	int integer;
	float4 float_vec;
} InnerStruct;

typedef struct {
	InnerStruct PrivateStruct;
	char another_field;
} OuterStruct;

__kernel void zero_private(__global char* output) {
	OuterStruct a;
	int b;
	output[0] = 123; // marker
	output[1] = a.PrivateStruct.integer;
	output[2] = a.PrivateStruct.float_vec.s0;
	output[3] = a.PrivateStruct.float_vec.s1;
	output[4] = a.PrivateStruct.float_vec.s2;
	output[5] = a.PrivateStruct.float_vec.s3;
	output[6] = a.another_field;
	// CHECK: _wcl_allocs->pa._wcl_b;
	output[7] = b;
	output[8] = 123; // marker
}
