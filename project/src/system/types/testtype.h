//testtype.h

#pragma once

#include "../macro.h"
#include "../ClassInstance.h"
#include <raylib.h>

#undef TYPE
#define TYPE TestType

BEGIN_CLASS(0x001b,"TestType");

    DEFINE_I_STRUCT( 
        P_VAR(int, x)
        P_VAR(int, y)
        P_VAR(int, z)
        P_VAR(int, w)
    )

    DEFINE_CONSTRUCTOR(
        F_GETPVAR(x) = 0;
        F_GETPVAR(y) = 1;
        F_GETPVAR(z) = 2;
        F_GETPVAR(w) = 3;
    )

    DEFINE_DESTRUCTOR(
        
    )

    DEFINE_TOSTRING(
        MemoryStream_WriteFormat(prm->stream, "x=%d, y=%d, z=%d, w=%d", F_GETPVAR(x), F_GETPVAR(y), F_GETPVAR(z), F_GETPVAR(w));
    )


    DEFINE_FUNCTION(0x0002, SAYHI, const char* name;)

    DEFINE_FUNCTION_WRAPPER(SAYHI, {
        if(prm->name == NULL){
            prm->code = FUN_WRONGARGS;
            return prm;
        }
    } , {

    })

    IMPL_FUNCTION(SAYHI, {
        TraceLog(LOG_INFO,TextFormat("My name is: %s",prm->name));
    })


    BEGIN_FUNFIND()

        FUNFIND_CONSTRUCTOR();
        FUNFIND_DESTRUCTOR();
        FUNFIND_TOSTRING();

        FUNFIND_IMPL(GetStruct);

        FUNFIND_IMPL(SAYHI);

    END_FUNFIND()

END_CLASS()

