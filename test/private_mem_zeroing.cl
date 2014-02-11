// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | sed 's@////@@' | %kernel-runner --webcl --kernel zero_private --gcount 40 | grep "2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,3,0,0,0,0"

// NOTE: ADD ANOTHER OPTION FOR RESULTS IF THIS FAILS ON BIG ENDIAN DEVICE

// C99 States that {} initializer for struct should do zero-initialization
// [1] http://www.open-std.org/jtc1/sc22/WG14/www/docs/n1256.pdf chapter 6.7.8 paragraphs 10 and 21 

// struct size should be 4 + 4*4 + 4*4*4 bytes
typedef struct { int first; float4 second; uint3 third[4]; } PrivateStruct;

__kernel void zero_private(__global char* output) {
    PrivateStruct ps;
    PrivateStruct ps2;
    PrivateStruct *psPtr = &ps;
    psPtr = &ps2;
    ps.third[3] = ((uint3)(1,2,3));
    ps.first = 2;
    ps2.first = 2;
    ps2.third[3] = ((uint3)(1,2,3));
    // These lines are uncommented before running the code (check sed in RUN script)
    //// char *firstAddr = (char*)(_wcl_allocs->pn);
    //// for (int i = 0; i < sizeof(_WclPrivates); i++) output[i] = *(firstAddr+i);
}
