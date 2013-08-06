// RUN: cat %s | %opencl-validator
// RUN: %webcl-validator %s 2>/dev/null | grep -v "Processing\|CHECK" | %opencl-validator

__constant struct { int field; } relocated_c1 = { 1 };
__constant struct { int field; } preserved_c1 = { 1 };

__constant struct CRStruct { int field; } relocated_c2 = { 2 };
__constant struct CPStruct { int field; } preserved_c2 = { 2 };

__constant struct CRStruct relocated_c3 = { 3 };
__constant struct CPStruct preserved_c3 = { 3 };

typedef struct { int field; } CRTypedef;
__constant CRTypedef relocated_c4 = { 4 };
typedef struct { int field; } CPTypedef;
__constant CPTypedef preserved_c4 = { 4 };

__constant struct { struct CRStruct rstruct; } relocated_cr1 = { { 1 } };
__constant struct { struct CPStruct pstruct; } relocated_cp1 = { { 1 } };
__constant struct { CRTypedef rstruct; } preserved_cr1 = { { 1 } };
__constant struct { CPTypedef pstruct; } preserved_cp1 = { { 1 } };

__constant struct CRRStruct { struct CRStruct rstruct; } relocated_cr2 = { { 2 } };
__constant struct CRPStruct { struct CPStruct pstruct; } relocated_cp2 = { { 2 } };
__constant struct CPRStruct { CRTypedef rstruct; } preserved_cr2 = { { 2 } };
__constant struct CPPStruct { CPTypedef pstruct; } preserved_cp2 = { { 2 } };

__constant struct CRRStruct relocated_cr3 = { { 3 } };
__constant struct CRPStruct relocated_cp3 = { { 3 } };
__constant struct CPRStruct preserved_cr3 = { { 3 } };
__constant struct CPPStruct preserved_cp3 = { { 3 } };

typedef struct { struct CRStruct rstruct; } CRRTypedef;
__constant CRRTypedef relocated_cr4 = { { 4 } };
typedef struct { struct CPStruct pstruct; } CRPTypedef;
__constant CRPTypedef relocated_cp4 = { { 4 } };
typedef struct { CRTypedef rstruct; } CPRTypedef;
__constant CPRTypedef preserved_cr4 = { { 4 } };
typedef struct { CPTypedef pstruct; } CPPTypedef;
__constant CPPTypedef preserved_cp4 = { { 4 } };

struct CFPStruct;
__constant struct CFRStruct { int field; struct CFRStruct *frstruct; struct CFPStruct *fpstruct; } relocated_cf1 = { 0, 0, 0 };
__constant struct CFPStruct { int field; struct CFRStruct *frstruct; struct CFPStruct *fpstruct; } preserved_cf1 = { 0, 0, 0 };

int relocate_privates(void);

int relocate_privates()
{
    struct { int field; } relocated_1 = { 1 };
    struct { int field; } preserved_1 = { 1 };

    struct RStruct { int field; } relocated_2 = { 2 };
    struct PStruct { int field; } preserved_2 = { 2 };

    struct RStruct relocated_3 = { 3 };
    struct PStruct preserved_3 = { 3 };

    typedef struct { int field; } RTypedef;
    RTypedef relocated_4 = { 4 };
    typedef struct { int field; } PTypedef;
    PTypedef preserved_4 = { 4 };

    struct { struct RStruct rstruct; } relocated_r1 = { { 1 } };
    struct { struct PStruct pstruct; } relocated_p1 = { { 1 } };
    struct { RTypedef rstruct; } preserved_r1 = { { 1 } };
    struct { PTypedef pstruct; } preserved_p1 = { { 1 } };

    struct RRStruct { struct RStruct rstruct; } relocated_r2 = { { 2 } };
    struct RPStruct { struct PStruct pstruct; } relocated_p2 = { { 2 } };
    struct PRStruct { RTypedef rstruct; } preserved_r2 = { { 2 } };
    struct PPStruct { PTypedef pstruct; } preserved_p2 = { { 2 } };

    struct RRStruct relocated_r3 = { { 3 } };
    struct RPStruct relocated_p3 = { { 3 } };
    struct PRStruct preserved_r3 = { { 3 } };
    struct PPStruct preserved_p3 = { { 3 } };

    typedef struct { struct RStruct rstruct; } RRTypedef;
    RRTypedef relocated_r4 = { { 4 } };
    typedef struct { struct PStruct pstruct; } RPTypedef;
    RPTypedef relocated_p4 = { { 4 } };
    typedef struct { RTypedef rstruct; } PRTypedef;
    PRTypedef preserved_r4 = { { 4 } };
    typedef struct { PTypedef pstruct; } PPTypedef;
    PPTypedef preserved_p4 = { { 4 } };

    struct FPStruct;
    struct FRStruct { int field; struct FRStruct *frstruct; struct FPStruct *fpstruct; } relocated_f1 = { 0, 0, 0 };
    struct FPStruct { int field; struct FRStruct *frstruct; struct FPStruct *fpstruct; } preserved_f1 = { 0, 0, 0 };

    int relocated =
        (&relocated_1)->field + (&relocated_2)->field +
        (&relocated_3)->field + (&relocated_4)->field +
        (&relocated_r1)->rstruct.field + (&relocated_p1)->pstruct.field +
        (&relocated_r2)->rstruct.field + (&relocated_p2)->pstruct.field +
        (&relocated_r3)->rstruct.field + (&relocated_p3)->pstruct.field +
        (&relocated_r4)->rstruct.field + (&relocated_p4)->pstruct.field +
        (&relocated_f1)->field;

    int preserved =
        preserved_1.field + preserved_2.field +
        preserved_3.field + preserved_4.field +
        preserved_r1.rstruct.field + preserved_p1.pstruct.field +
        preserved_r2.rstruct.field + preserved_p2.pstruct.field +
        preserved_r3.rstruct.field + preserved_p3.pstruct.field +
        preserved_r4.rstruct.field + preserved_p4.pstruct.field +
        preserved_f1.field;

    return relocated + preserved;
}

__kernel void relocate_types(
    __global int *result)
{
    __local struct { int field; } relocated_1;
    relocated_1.field = 1;
    __local struct { int field; } preserved_1;
    preserved_1.field = 1;

    __local struct RStruct { int field; } relocated_2;
    relocated_2.field = 2;
    __local struct PStruct { int field; } preserved_2;
    preserved_2.field = 2;

    __local struct RStruct relocated_3;
    relocated_3.field = 3;
    __local struct PStruct preserved_3;
    preserved_3.field = 3;

    typedef struct { int field; } RTypedef;
    __local RTypedef relocated_4;
    relocated_4.field = 4;
    typedef struct { int field; } PTypedef;
    __local PTypedef preserved_4;
    preserved_4.field = 4;

    __local struct { struct RStruct rstruct; } relocated_r1;
    relocated_r1.rstruct.field = 1;
    __local struct { struct PStruct pstruct; } relocated_p1;
    relocated_p1.pstruct.field = 1;
    __local struct { RTypedef rstruct; } preserved_r1;
    preserved_r1.rstruct.field = 1;
    __local struct { PTypedef pstruct; } preserved_p1;
    preserved_p1.pstruct.field = 1;

    __local struct RRStruct { struct RStruct rstruct; } relocated_r2;
    relocated_r2.rstruct.field = 2;
    __local struct RPStruct { struct PStruct pstruct; } relocated_p2;
    relocated_p2.pstruct.field = 2;
    __local struct PRStruct { RTypedef rstruct; } preserved_r2;
    preserved_r2.rstruct.field = 2;
    __local struct PPStruct { PTypedef pstruct; } preserved_p2;
    preserved_p2.pstruct.field = 2;

    __local struct RRStruct relocated_r3;
    relocated_r3.rstruct.field = 3;
    __local struct RPStruct relocated_p3;
    relocated_p3.pstruct.field = 3;
    __local struct PRStruct preserved_r3;
    preserved_r3.rstruct.field = 3;
    __local struct PPStruct preserved_p3;
    preserved_p3.pstruct.field = 3;

    typedef struct { struct RStruct rstruct; } RRTypedef;
    __local RRTypedef relocated_r4;
    relocated_r4.rstruct.field = 4;
    typedef struct { struct PStruct pstruct; } RPTypedef;
    __local RPTypedef relocated_p4;
    relocated_p4.pstruct.field = 4;
    typedef struct { RTypedef rstruct; } PRTypedef;
    __local PRTypedef preserved_r4;
    preserved_r4.rstruct.field = 4;
    typedef struct { PTypedef pstruct; } PPTypedef;
    __local PPTypedef preserved_p4;
    preserved_p4.pstruct.field = 4;

    struct FPStruct;
    __local struct FRStruct { int field; struct FRStruct *frstruct; struct FPStruct *fpstruct; } relocated_f1;
    relocated_f1.field = 0;
    relocated_f1.frstruct = 0;
    relocated_f1.fpstruct = 0;
    __local struct FPStruct { int field; struct FRStruct *frstruct; struct FPStruct *fpstruct; } preserved_f1;
    preserved_f1.field = 0;
    preserved_f1.frstruct = 0;
    preserved_f1.fpstruct = 0;

    int relocated_constants =
        (&relocated_c1)->field + (&relocated_c2)->field +
        (&relocated_c3)->field + (&relocated_c4)->field +
        (&relocated_cr1)->rstruct.field + (&relocated_cp1)->pstruct.field +
        (&relocated_cr2)->rstruct.field + (&relocated_cp2)->pstruct.field +
        (&relocated_cr3)->rstruct.field + (&relocated_cp3)->pstruct.field +
        (&relocated_cr4)->rstruct.field + (&relocated_cp4)->pstruct.field +
        (&relocated_cf1)->field;

    int preserved_constants =
        preserved_c1.field + preserved_c2.field +
        preserved_c3.field + preserved_c4.field +
        preserved_cr1.rstruct.field + preserved_cp1.pstruct.field +
        preserved_cr2.rstruct.field + preserved_cp2.pstruct.field +
        preserved_cr3.rstruct.field + preserved_cp3.pstruct.field +
        preserved_cr4.rstruct.field + preserved_cp4.pstruct.field +
        preserved_cf1.field;

    int relocated_locals =
        (&relocated_1)->field + (&relocated_2)->field +
        (&relocated_3)->field + (&relocated_4)->field +
        (&relocated_r1)->rstruct.field + (&relocated_p1)->pstruct.field +
        (&relocated_r2)->rstruct.field + (&relocated_p2)->pstruct.field +
        (&relocated_r3)->rstruct.field + (&relocated_p3)->pstruct.field +
        (&relocated_r4)->rstruct.field + (&relocated_p4)->pstruct.field +
        (&relocated_f1)->field;

    int preserved_locals =
        preserved_1.field + preserved_2.field +
        preserved_3.field + preserved_4.field +
        preserved_r1.rstruct.field + preserved_p1.pstruct.field +
        preserved_r2.rstruct.field + preserved_p2.pstruct.field +
        preserved_r3.rstruct.field + preserved_p3.pstruct.field +
        preserved_r4.rstruct.field + preserved_p4.pstruct.field +
        preserved_f1.field;

    result[get_global_id(0)] = relocate_privates() +
        relocated_constants + preserved_constants +
        relocated_locals + preserved_locals;
}
