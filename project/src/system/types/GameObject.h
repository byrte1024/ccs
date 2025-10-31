//GameObject.h

#pragma once

#include "../macro.h"
#include "../ClassInstance.h"
#include "../../utils/BinaryUtils.h"
#include "../../utils/ResizeableArray.h"
#include "../../utils/Dictionary.h"
#include <assert.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdio.h>

#undef TYPE
#define TYPE GameObject

#define GameObject_Name_Max 64

BEGIN_CLASS(0x1E00, GameObject)

    DEFINE_I_STRUCT(

        char name[GameObject_Name_Max];
        uint32_t id;
        bool enabled;
        

        ResizeableArray components; //ResizeableArray<ClassReference<>>

    );


    DEFINE_I_FUNCTION(0xA000, Propagate, FunctionID fid; PrmStructAddress prm; );
    DEFINE_I_FUNCTION_WRAPPER(Propagate, {
        if(prm->prm == NULL){
            prm->code = FUN_WRONGARGS;
            return prm;
        }
        if(prm->fid == FID_GameObject_Propagate){
            prm->code = FUN_WRONGARGS;
            return prm;
        }
    },{

    })
    IMPL_FUNCTION(Propagate) {
        IMPL_HEADER;
        GETISELF();
    }

    BEGIN_FUNFIND()

        FUNFIND_IMPL(Propagate);

    END_FUNFIND()

END_CLASS()