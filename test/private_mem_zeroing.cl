// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2> /dev/null| grep -v "Processing\|CHECK" | %kernel-runner --webcl --kernel zero_private --gcount 40 --global int 40 | grep "1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,"

// TODO: fix test to work with different target layouts and still verify that
//       private memory was not leaked. 
//       (add another possible result to grep for other endianess) 

typedef struct { int first; float4 second; uint3 third[4]; } PrivateStruct;

__kernel void zero_private(__global char* output) {
    char starter = 1;
    PrivateStruct ps;
    PrivateStruct ps2;
    PrivateStruct *psPtr = &ps;
    psPtr = &ps2;
    ps.third[3] = ((uint3)(1,2,3));
    ps.first = 2;
    ps2.first = 2;
    ps2.third[3] = ((uint3)(1,2,3));
    char *firstAddr = (char*)(&starter);
    for (int i = 0; i < 1024; i++) output[i] = *(firstAddr+i);
}
