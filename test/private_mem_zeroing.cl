// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %kernel-runner --webcl --kernel zero_private --gcount 40 | grep "1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,"

// TODO: (add another possible result to grep for other endianess) 

// C99 States that {} initializer for struct should do zero-initialization
// [1] http://www.open-std.org/jtc1/sc22/WG14/www/docs/n1256.pdf chapter 6.7.8 paragraphs 10 and 21 

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
