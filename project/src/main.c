
#include <raylib.h>
#include <stdio.h>
#include "system/ClassInstance.h"

#include "system/instance_macro.h"
#include "system/types/testtype.h"

#ifndef INTESTING

int main(void) {

    C_TESTTYPE_REGISTER();

    ClassReference r = Class_Reference_AllocateEmptyInstance();

    Class_Reference_PrintStatus(r);

    r.instance->cid = CID_TESTTYPE;

    Class_Reference_PrintStatus(r);

    CALL_R_FUNCTION(r, DEF_CREATE, );

    Class_Reference_PrintStatus(r);


    FUNPRM(TESTTYPE_SAYHI) P = CALL_FUNCTION(TESTTYPE, TESTTYPE_SAYHI, .name = "Weee" );
    
    

    getchar();

    return 0;
}

#endif