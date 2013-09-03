// RUN: %opencl-validator < %s
// RUN: %webcl-validator %s | %opencl-validator
// RUN: %webcl-validator %s | grep -v CHECK | %FileCheck %s

int get_pointed_value // (shouldn't match these parenthesis)
    // CHECK: (_WclProgramAllocations *_wcl_allocs, __global int *pointer, char* dummy)
    (__global int *pointer, char* dummy);

int get_pointed_value // (shouldn't match these parenthesis)
    // CHECK: (_WclProgramAllocations *_wcl_allocs, __global int *pointer, char* dummy)
    (__global int *pointer, char* dummy)
{
    return *pointer;
}

__kernel void find_works_fine
    (__global int *array)
{
    // CHECK: int i = get_global_id(0);;_wcl_allocs->pa._wcl_i = i;
    int i = get_global_id(0);
    int pointed_value = get_pointed_value(array + i, (char *)"dont;match;here") // "dont;match;here"
    // CHECK: ;;_wcl_allocs->pa._wcl_pointed_value = pointed_value;
    ;
    // CHECK: int *iptr = &_wcl_allocs->pa._wcl_i /* don't ;match;here; */;;_wcl_allocs->pa._wcl_iptr = iptr;
    int *iptr = &i /* don't ;match;here; */;
    // CHECK: test = 0 + (char *)"dont;match;here";;_wcl_allocs->pa._wcl_test = test;
    char* test = 0 + (char *)"dont;match;here";
    test = (char*)&pointed_value;
}

