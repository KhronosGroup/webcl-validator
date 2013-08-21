// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2> /dev/null| grep -v "Processing\|CHECK" | sed 's@////@@' | %kernel-runner --webcl --kernel read_memory --constant int 100 --global int 100 --local int 100 | grep "33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,1,1,1,1,0,1,1,1,1,0,1,1,1,0,"

__constant uchar16 constant_vec = ((uchar16)(33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33));

#define REL(x) relocate((void*)(&x))
void relocate(void *param) {}

__kernel void read_memory(__global char* output, 
    __constant int *factors, 
    __global int *input, 
    __local int *scratch) {

    __local uchar16 local_vec;
    __local long   local_var;
    local_var = 44;
    int i = get_global_id(0);
    uchar16 private_vec = constant_vec;

    REL(local_vec);
    REL(local_var);
    REL(i);
    REL(private_vec);

    // Code commented with //// is enabled after running the instrumentation

    // initialize every address space null ptr with 33
    __global uint *global_null     =  &(*((__global   uint*)0xffffff));
    __local  uint *local_null      =  &(*((__local    uint*)0xffffff));
    __constant uint *constant_null =  &(*((__constant uint*)0xffffff));
    __private uint *private_null   =  &(*((__private  uint*)0xffffff));

    // if you like some more debug make also these //// comments
    // //printf("global null: %X\n" , _wcl_allocs->pa._wcl_global_null);
    // //printf("local null: %X\n" , _wcl_allocs->pa._wcl_local_null);
    // //printf("constant null: %X\n" , _wcl_allocs->pa._wcl_constant_null);
    // //printf("private null: %X\n" , _wcl_allocs->pa._wcl_private_null);
    // //printf("gn,cn,ln,pn: %X,%X,%X,%X\n", _wcl_allocs->gn, _wcl_allocs->cn, _wcl_allocs->ln, _wcl_allocs->pn);
    
    // initialize null pointer to 33 
    *((__global  uchar16*)0xffffff) = private_vec;
    *((__local   uchar16*)0xffffff) = private_vec;
    *((__private uchar16*)0xffffff) = private_vec;

    // Make sure that all null pointers are set properly
    ////output[16] = (_wcl_allocs->pn == _wcl_allocs->pa._wcl_private_null);
    ////output[17] = (_wcl_allocs->ln == _wcl_allocs->pa._wcl_local_null);
    ////output[18] = (_wcl_allocs->gn == _wcl_allocs->pa._wcl_global_null);
    ////output[19] = (_wcl_allocs->cn == _wcl_allocs->pa._wcl_constant_null);

    // try to read over some limits
    output[21] = ((scratch[100])&0xff) == 33;
    output[22] = ((input[100])&0xff) == 33;
    output[23] = ((__constant uint*)&(factors[100]) == constant_null);
    output[24] = ((__constant uint*)&(factors[99]) != constant_null);

    output[26] = ((*(scratch-1))&0xff) == 33;
    output[27] = ((*(input-1))&0xff) == 33;
    output[28] = ((__constant uint*)&(*(&constant_vec-1)) == constant_null);
}
