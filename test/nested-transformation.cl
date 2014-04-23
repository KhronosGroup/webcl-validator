// RUN: %opencl-validator < "%s"
// RUN: %webcl-validator "%s" | %opencl-validator

typedef struct {
    int field;
} FStruct;

typedef struct {
    FStruct *fstruct;
} SStruct;

__kernel void nested_transformation(
    // CHECK: __global int *result, unsigned long wcl_result_size)
    __global int *result)
{
    int value = 0;
    int *pointer = &value;

    int **pointer2 = &pointer;

    int array[1] = { 0 };
    int array2[1] = { 0 };

    FStruct fstruct = { 0 };
    FStruct *pfstruct = &fstruct;
    SStruct sstruct = { pfstruct };
    SStruct *psstruct = &sstruct;

    *(pointer + *pointer + array[0] + pfstruct->field) =
        array[*pointer + array[0] + pfstruct->field];

    (**pointer2) = array2[array[0]];
    array2[array[0]] = psstruct->fstruct->field;
    psstruct->fstruct->field = (**pointer2);

    result[get_global_id(0)] = ((psstruct + *pointer)->fstruct + array[0])->field;
}
