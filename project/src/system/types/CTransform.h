//CTransform

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
#define TYPE CTransform

BEGIN_CLASS(0xC001, CTransform)

    DEFINE_I_STRUCT(

        PVAR(float, x);    //object center X
        PVAR(float, y);    //object center Y
        PVAR(float, z);    //object center Z

        PVAR(float, pivot_x);   //object offset X
        PVAR(float, pivot_y);   //object offset Y
        PVAR(float, pivot_z);   //object offset Z

        PVAR(float, rotation_x);   //object rotation X
        PVAR(float, rotation_y);   //object rotation Y
        PVAR(float, rotation_z);   //object rotation Z

        PVAR(float, scale_x);   //object scale X
        PVAR(float, scale_y);   //object scale Y
        PVAR(float, scale_z);   //object scale Z

        PVAR(float, bounds_x);   //object bounds min X
        PVAR(float, bounds_y);   //object bounds min Y
        PVAR(float, bounds_z);   //object bounds min Z
        PVAR(float, bounds_width);   //object bounds width
        PVAR(float, bounds_height);   //object bounds height
        PVAR(float, bounds_depth);   //object bounds depth

    );

    IMPLOTHER_FUNCTION(DEF_CREATE){
        IMPL_HEADER_CREATE

        CLASS_I_STRUCT(TYPE)* self = prm->self->data;

        F_GETPVAR(x) = 0.0f;
        F_GETPVAR(y) = 0.0f;
        F_GETPVAR(z) = 0.0f;

        F_GETPVAR(pivot_x) = 0.0f;
        F_GETPVAR(pivot_y) = 0.0f;
        F_GETPVAR(pivot_z) = 0.0f;

        F_GETPVAR(rotation_x) = 0.0f;
        F_GETPVAR(rotation_y) = 0.0f;
        F_GETPVAR(rotation_z) = 0.0f;

        F_GETPVAR(scale_x) = 1.0f;
        F_GETPVAR(scale_y) = 1.0f;
        F_GETPVAR(scale_z) = 1.0f;

        F_GETPVAR(bounds_x) = 0.0f;
        F_GETPVAR(bounds_y) = 0.0f;
        F_GETPVAR(bounds_z) = 0.0f;
        F_GETPVAR(bounds_width) = 0.0f;
        F_GETPVAR(bounds_height) = 0.0f;
        F_GETPVAR(bounds_depth) = 0.0f;
    }
    IMPLOTHER_FUNCTION(DEF_DESTROY){
        IMPL_HEADER

        CLASS_I_STRUCT(TYPE)* self = prm->self->data;
        free(self);
    }
    IMPLOTHER_FUNCTION(DEF_TOSTRING){
        IMPL_HEADER
        GETISELF();
        MemoryStream* ms = prm->stream;

        MemoryStream_WriteFormat(ms, "x=%f, y=%f, z=%f, pivot_x=%f, pivot_y=%f, pivot_z=%f, rotation_x=%f, rotation_y=%f, rotation_z=%f, scale_x=%f, scale_y=%f, scale_z=%f, bounds_x=%f, bounds_y=%f, bounds_z=%f, bounds_width=%f, bounds_height=%f, bounds_depth=%f",
            F_GETPVAR(x), F_GETPVAR(y), F_GETPVAR(z),
            F_GETPVAR(pivot_x), F_GETPVAR(pivot_y), F_GETPVAR(pivot_z),
            F_GETPVAR(rotation_x), F_GETPVAR(rotation_y), F_GETPVAR(rotation_z),
            F_GETPVAR(scale_x), F_GETPVAR(scale_y), F_GETPVAR(scale_z),
            F_GETPVAR(bounds_x), F_GETPVAR(bounds_y), F_GETPVAR(bounds_z),
            F_GETPVAR(bounds_width), F_GETPVAR(bounds_height), F_GETPVAR(bounds_depth));
    }
    IMPLOTHER_FUNCTION(DEF_SERIALIZE){
        IMPL_HEADER
        GETISELF();
        MemoryStream* ms = prm->stream;
        
        SERIALIZE_OWN_SIMPLEVAR(ms, x);
        SERIALIZE_OWN_SIMPLEVAR(ms, y);
        SERIALIZE_OWN_SIMPLEVAR(ms, z);

        SERIALIZE_OWN_SIMPLEVAR(ms, pivot_x);
        SERIALIZE_OWN_SIMPLEVAR(ms, pivot_y);
        SERIALIZE_OWN_SIMPLEVAR(ms, pivot_z);

        SERIALIZE_OWN_SIMPLEVAR(ms, rotation_x);
        SERIALIZE_OWN_SIMPLEVAR(ms, rotation_y);
        SERIALIZE_OWN_SIMPLEVAR(ms, rotation_z);

        SERIALIZE_OWN_SIMPLEVAR(ms, scale_x);
        SERIALIZE_OWN_SIMPLEVAR(ms, scale_y);
        SERIALIZE_OWN_SIMPLEVAR(ms, scale_z);
        
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_x);
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_y);
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_z);
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_width);
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_height);
        SERIALIZE_OWN_SIMPLEVAR(ms, bounds_depth);
        
    }

    IMPLOTHER_FUNCTION(DEF_DESERIALIZE){
        IMPL_HEADER
        GETISELF();
        MemoryStream* ms = prm->stream;
        char buff;

        DESERIALIZE_OWN_SIMPLEVAR(ms, x, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, y, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, z, buff);
        
        DESERIALIZE_OWN_SIMPLEVAR(ms, pivot_x, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, pivot_y, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, pivot_z, buff);

        DESERIALIZE_OWN_SIMPLEVAR(ms, rotation_x, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, rotation_y, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, rotation_z, buff);

        DESERIALIZE_OWN_SIMPLEVAR(ms, scale_x, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, scale_y, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, scale_z, buff);
        
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_x, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_y, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_z, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_width, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_height, buff);
        DESERIALIZE_OWN_SIMPLEVAR(ms, bounds_depth, buff);
        
    }

    BEGIN_FUNFIND()

        FUNFIND_IMPL(GetStruct);

        FUNFIND_IMPLOTHER(DEF_CREATE);
        FUNFIND_IMPLOTHER(DEF_DESTROY);
        FUNFIND_IMPLOTHER(DEF_TOSTRING);
        FUNFIND_IMPLOTHER(DEF_SERIALIZE);
        FUNFIND_IMPLOTHER(DEF_DESERIALIZE);

    END_FUNFIND()

END_CLASS()