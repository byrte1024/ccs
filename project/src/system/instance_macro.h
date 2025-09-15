//instance_macro.h

#pragma once

#include "macro.h"

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