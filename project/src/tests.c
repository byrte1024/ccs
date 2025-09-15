#ifdef INTESTING

#include <raylib.h>
#include <stdio.h>
#include "system/ClassInstance.h"

#include "system/instance_macro.h"
#include "system/macro.h"
#include "system/types/testtype.h"

static int highestLoggedLevel = LOG_INFO;
void TraceLogCB(int loglevel, const char* text, va_list args){

    if(loglevel > highestLoggedLevel){
        highestLoggedLevel = loglevel;
    }

    if(loglevel == LOG_INFO){
        printf("INFO: %s\n", text);
    }
    else if(loglevel == LOG_ERROR){
        printf("ERROR: %s\n", text);
    }
    else if(loglevel == LOG_WARNING){
        printf("WARNING: %s\n", text);
    }
    else if(loglevel == LOG_DEBUG){
        printf("DEBUG: %s\n", text);
    }
    else if(loglevel == LOG_TRACE){
        printf("TRACE: %s\n", text);
    }

    va_end(args);
}

int main(void) {
    SetTraceLogCallback(TraceLogCB);
    SetTraceLogLevel(LOG_INFO);

    TraceLog(LOG_INFO, "Hello, world!");
    
    TraceLog(LOG_INFO, "=== BEGIN TESTING ===");

    C_TESTTYPE_REGISTER();

    ClassReference r = Class_Reference_AllocateEmptyInstance();

    if(r.instance == NULL){
        TraceLog(LOG_ERROR, "Error while allocating instance.");
        return 1;
    }

    Class_Reference_PrintStatus(r);

    r.instance->cid = CID_TESTTYPE;

    Class_Reference_PrintStatus(r);

    if(CALL_R_FUNCTION(r, DEF_CREATE, )->code != FUN_OK){
        TraceLog(LOG_ERROR, "Error while calling create function.");
        return 1;
    }

    Class_Reference_PrintStatus(r);

    if(r.instance->data == NULL){
        TraceLog(LOG_ERROR, "Error while allocating data.");
        return 1;
    }

    TestType* t = (TestType*)r.instance->data;
    if(t == NULL){
        TraceLog(LOG_ERROR, "Error while casting data.");
        return 1;
    }

    TraceLog(LOG_INFO, TextFormat("x: %d, y: %d, z: %d, w: %d", t->x, t->y, t->z, t->w));

    if(t->x != 1 || t->y  != 2 || t->z != 3 || t->w != 4){
        TraceLog(LOG_ERROR, "Error while intializing testtype");
        return 1;
    }

    t->x = 23;
    t->y = 94;
    t->z = 2;
    t->w = 42;

    TraceLog(LOG_INFO, TextFormat("x: %d, y: %d, z: %d, w: %d", t->x, t->y, t->z, t->w));

    if(t->x != 23 || t->y  != 94 || t->z != 2 || t->w != 42){
        TraceLog(LOG_ERROR, "Error while setting testtype");
        return 1;
    }

    if(CALL_R_FUNCTION(r, DEF_DESTROY, )->code != FUN_OK){
        TraceLog(LOG_ERROR, "Error while calling destroy function.");
        return 1;
    }

    if(r.instance->data != NULL){
        TraceLog(LOG_ERROR, "Error while freeing data.");
        return 1;
    }

    ClassInstance* i = r.instance;

    Class_Reference_Forget(r);

    return 0;


}

#endif