// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | sed 's@////@@' | %kernel-runner --webcl --kernel read_memory --constant int 100 --global int 100 --local int 100 | grep "33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,1,1,1,1,0,1,1,1,1,0,1,1,1,0,"

__constant uchar16 constant_vec = ((uchar16)(33,33,33,33,33,33,33,33,33,33,33,33,33,33,33,33));

#define REL(x) (&x)

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

    // store null pointer in a way that private variable storing the address
    // cannot be overridden by this program (we use last values of these tables to be sure)
    __global uint* global_null[8]     =  { 0,0,0,0,0,0,0, &(*((__global   uint*)0xffffff)) };
    __local  uint* local_null[8]      =  { 0,0,0,0,0,0,0, &(*((__local    uint*)0xffffff)) };
    __constant uint* constant_null[8] =  { 0,0,0,0,0,0,0, &(*((__constant uint*)0xffffff)) };
    __private uint* private_null[8]   =  { 0,0,0,0,0,0,0, &(*((__private  uint*)0xffffff)) };

    // if you like some more debug make also these //// comments
    // //printf("global null: %X\n" , _wcl_allocs->pa._wcl_global_null[7]);
    // //printf("local null: %X\n" , _wcl_allocs->pa._wcl_local_null[7]);
    // //printf("constant null: %X\n" , _wcl_allocs->pa._wcl_constant_null[7]);
    // //printf("private null: %X\n" , _wcl_allocs->pa._wcl_private_null[7]);
    // //printf("gn,cn,ln,pn: %X,%X,%X,%X\n", _wcl_allocs->gn, _wcl_allocs->cn, _wcl_allocs->ln, _wcl_allocs->pn);
    
    // initialize every address space null ptr with 33..
    *((__global  uchar16*)0xffffff) = private_vec;
    *((__local   uchar16*)0xffffff) = private_vec;
    *((__private uchar16*)0xffffff) = private_vec;

    // Make sure that all null pointers are set properly (output vals: should be ...33,1,1,1,1 ... )
    ////output[16] = (_wcl_allocs->pn == _wcl_allocs->pa._wcl_private_null[7]);
    ////output[17] = (_wcl_allocs->ln == _wcl_allocs->pa._wcl_local_null[7]);
    ////output[18] = (_wcl_allocs->gn == _wcl_allocs->pa._wcl_global_null[7]);
    ////output[19] = (_wcl_allocs->cn == _wcl_allocs->pa._wcl_constant_null[7]);

    // output[20] should be implicitly 0 because memory is zero initialized by default

    // try to read over some limits
    output[21] = ((scratch[100])&0xff) == 33;
    output[22] = ((input[100])&0xff) == 33;
    output[23] = ((__constant uint*)&(factors[100]) == constant_null[7]);
    output[24] = ((__constant uint*)&(factors[99]) != constant_null[7]);

    // output[25] should be implicitly 0 because memory is zero initialized by default

    output[26] = ((*(scratch-1))&0xff) == 33;
    output[27] = ((*(input-1))&0xff) == 33;
    output[28] = ((__constant uint*)&(*(&constant_vec-1)) == constant_null[7]);
}
