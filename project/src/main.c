
#include <raylib.h>
#include <stdio.h>
#include "system/ClassInstance.h"

#include "system/instance_macro.h"
#include "system/types/testtype.h"
#include "utils/MemoryStream.h"

#ifndef INTESTING

int main(void) {

    C_TestType_REGISTER();

    ClassReference r = Class_Reference_AllocateEmptyInstance();
    r.instance->cid = CID_TestType;
    
    Class_Reference_PrintStatus(r);

    CALL_R_FUNCTION(r, DEF_CREATE, );

    Class_Reference_PrintStatus(r);

    S_TestType* t = CALL_R_FUNCTION(r, TestType_GetStruct, )->TestType;
    
    Class_Reference_PrintStatus(r);

    printf("x: %d, y: %d. z: %d, w: %d\n", t->x, t->y, t->z, t->w);
    
    MemoryStream ms = { 0 };

    CALL_R_FUNCTION(r, DEF_TOSTRING, .stream = &ms);
    
    MemoryStream_Log(&ms, LOG_INFO);

    getchar();

    return 0;
}

#endif