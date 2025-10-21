//instance_macro.h

#pragma once

#include "macro.h"

#define LFID_GETSTRUCT 0x0000

#define DEFINE_I_FUNCTION(lfid ,name, args) \
    enum { \
        EP4(FID_LOCAL_,TYPE,_,name) = lfid, \
        EP4(FID_,TYPE,_,name) = COMPOSE_FUNCTIONID(EP4(FID_LOCAL_,TYPE,_,name),EP2(CID_,TYPE)) \
    }; \
    typedef struct EP5(F_,TYPE,_,name,_PRM) { \
        char code; \
        ClassInstance* self; \
        args \
    } EP5(F_,TYPE,_,name,_PRM); \
    DEFINE_I_FUNCTION_WRAPPER_SKELETON(name)

#define DEFINE_I_FUNCTION_WRAPPER(name, prelogic, postlogic) \
    DEFINE_FUNCTION_WRAPPER(name, prelogic, postlogic) \
    static inline EP5(F_,TYPE,_,name,_PRM*) EP5(F_,TYPE,_,name,_EXECUTE_I)( ClassInstance* instance , EP5(F_,TYPE,_,name,_PRM*) PRM) \
    { \
        ClassID cid = CID_DEF; \
        PRM->self = instance; \
        if(instance != NULL) { \
            cid = instance->cid; \
        } \
        PRM = EP5(F_,TYPE,_,name,_EXECUTE)(cid, PRM); \
        return PRM; \
    } 

#define DEFINE_I_FUNCTION_WRAPPER_SKELETON(name) \
    DEFINE_FUNCTION_WRAPPER_SKELETON(name) \
    static inline EP5(F_,TYPE,_,name,_PRM*) EP5(F_,TYPE,_,name,_EXECUTE_I)( ClassInstance* instance , EP5(F_,TYPE,_,name,_PRM*) PRM);

#define CALL_I_FUNCTION(classinstance,name, args) \
    EP3(F_,name,_EXECUTE_I)(classinstance, &((EP3(F_,name,_PRM)){ args }))

#define CALL_R_FUNCTION(classreference, name, args) \
    EP3(F_,name,_EXECUTE_I)(classreference.instance, &((EP3(F_,name,_PRM)){ args }))

#define DEFINEANDWRAP_I_FUNCTION(lfid, name, args, prelogic, postlogic) \
    DEFINE_I_FUNCTION(lfid, name, args) \
    DEFINE_I_FUNCTION_WRAPPER(name, prelogic, postlogic)

#define DEFINE_I_STRUCT(vals) \
    typedef struct EP2(S_,TYPE) \
    { \
        vals \
            \
    } EP2(S_,TYPE); \
    DEFINE_I_FUNCTION(LFID_GETSTRUCT,GetStruct, EP2(S_,TYPE)* EP1(TYPE); ) \
    DEFINE_I_FUNCTION_WRAPPER(GetStruct, \
        if(!Class_Instance_Status_IsAlive(prm->self) || !Class_Instance_Status_IsTyped(prm->self)) \
        { \
            prm->code = FUN_WRONGARGS; \
            return prm; \
        } \
        ,       \
                \
    ) \
    IMPL_FUNCTION(GetStruct, { \
        prm->EP1(TYPE) = prm->self->data;\
    })

#define DEFINE_I_STRUCT_NOWRAP

#define PRIMITIVE_VARIABLE(type, name) \
    type name;

#define P_VAR(type, name) \
    PRIMITIVE_VARIABLE(type, name)

#define DEFINE_CONSTRUCTOR(CDD) \
    IMPLOTHER_FUNCTION(DEF_CREATE, \
    { \
        ClassInstance* i = prm->self; \
        EP2(S_,TYPE)* t = malloc(sizeof(EP2(S_,TYPE))); \
        if(t == NULL){ \
            prm->code = FUN_ERROR; \
            return; \
        } \
        memset(t,0,sizeof(EP2(S_,TYPE))); \
        EP1(CDD)\
        \
        i->data = t; \
    } \
    )

#define C_GET_PRIMITIVE(var) \
    t->var

#define F_GETPVAR(var) \
    C_GET_PRIMITIVE(var)

#define DEFINE_DESTRUCTOR(CDD) \
    IMPLOTHER_FUNCTION(DEF_DESTROY, \
        { \
        ClassInstance* i = prm->self; \
        EP2(S_,TYPE)* t = i->data; \
        EP1(CDD) \
        free(t);\
    })

#define DEFINE_TOSTRING(CDD) \
    IMPLOTHER_FUNCTION(DEF_TOSTRING, \
        { \
        ClassInstance* i = prm->self; \
        EP2(S_,TYPE)* t = i->data; \
        MemoryStream* stream = prm->stream; \
        EP1(CDD) \
    })

#define FUNFIND_CONSTRUCTOR() \
    FUNFIND_IMPLOTHER(DEF_CREATE);

#define FUNFIND_DESTRUCTOR() \
    FUNFIND_IMPLOTHER(DEF_DESTROY);

#define FUNFIND_TOSTRING() \
    FUNFIND_IMPLOTHER(DEF_TOSTRING);

#define FUNFIND_SERIALIZE() \
    FUNFIND_IMPLOTHER(DEF_SERIALIZE);

#define FUNFIND_DESERIALIZE() \
    FUNFIND_IMPLOTHER(DEF_DESERIALIZE);

