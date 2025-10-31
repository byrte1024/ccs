#define INSPECTION_IMMUNITY

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

#define CALL_I_FUNCTION(classinstance,name, ...) \
    EP3(F_,name,_EXECUTE_I)(classinstance, &((EP3(F_,name,_PRM)){ __VA_ARGS__  }))

#define CALL_R_FUNCTION(classreference, name, ...) \
    EP3(F_,name,_EXECUTE_I)(classreference.instance, &((EP3(F_,name,_PRM)){ __VA_ARGS__  }))

#define DEFINEANDWRAP_I_FUNCTION(lfid, name, args, prelogic, postlogic) \
    DEFINE_I_FUNCTION(lfid, name, args) \
    DEFINE_I_FUNCTION_WRAPPER(name, prelogic, postlogic)

#define DEFINE_I_STRUCT(vals) \
    typedef struct EP2(S_I_,TYPE) \
    { \
        vals \
            \
    } EP2(S_I_,TYPE); \
    DEFINE_I_FUNCTION(LFID_GETSTRUCT,GetStruct, EP2(S_I_,TYPE)* EP2(TYPE,_out); ) \
    DEFINE_I_FUNCTION_WRAPPER(GetStruct, \
        if(!Class_Instance_Status_IsAlive(prm->self) || !Class_Instance_Status_IsTyped(prm->self)) \
        { \
            prm->code = FUN_WRONGARGS; \
            return prm; \
        } \
        ,       \
                \
    ) \
    IMPL_FUNCTION(GetStruct) { \
        prm->EP2(TYPE,_out) = prm->self->data;\
    }

#define ITYPEOF(type) EP2(S_I_,type)

#define PRIMITIVE_VARIABLE(type, name) type name

#define P_VAR(type, name) PRIMITIVE_VARIABLE(type, name)

#define PVAR(type, name) P_VAR(type, name)

#define C_GET_PRIMITIVE(var) \
    self->var

#define F_GETPVAR(var) \
    C_GET_PRIMITIVE(var)


#undef INSPECTION_IMMUNITY

#define CLASS_I_STRUCT(type) EP2(S_I_,type)

#define CLIS(type) EP2(S_I_,type)

#define GETISELF() CLIS(TYPE)* self = CALL_I_FUNCTION(prm->self, EP2(TYPE,_GetStruct) , )->EP2(TYPE,_out)
#define GETISTRUCT(Instance, type, outvar) CLIS(type)* outvar = CALL_I_FUNCTION(Instance, EP2(type,_GetStruct) , )->EP2(type,_out)

#define IMPL_HEADER_CREATE \
    IMPL_HEADER \
    prm->self->data = malloc(sizeof(CLASS_I_STRUCT(TYPE))); \
    if(prm->self->data == NULL) { \
        FLogError("Failed to allocated memory for instance of type %s", Class_System_GetDefinitionName(prm->self->cid)); \
        prm->code = FUN_ERROR; \
        return; \
    }

#define SERIALIZE_ASSERT(run) \
    if(!(run)) { \
        FLogError("Serialization failed: %s", #run); \
        prm->code = FUN_ERROR; \
        return; \
    }

#define DESERIALIZE_ASSERT(run) \
    if(!(run)) { \
        FLogError("Deserialization failed: %s", #run); \
        prm->code = FUN_ERROR; \
        return; \
    }

#define SERIALIZE_SIMPLEVAR(ms, var) \
    SERIALIZE_ASSERT(MemoryStream_WriteChar(ms, TOKEN_BEGIN_VARIABLE, NULL)); \
    SERIALIZE_ASSERT(MemoryStream_WriteBytes(ms,(uint8_t*)&var, sizeof(var), NULL)); \
    SERIALIZE_ASSERT(MemoryStream_WriteChar(ms, TOKEN_END_VARIABLE, NULL));

#define SERIALIZE_OWN_SIMPLEVAR(ms, varname) SERIALIZE_SIMPLEVAR(ms, F_GETPVAR(varname))

#define DESERIALIZE_SIMPLEVAR(ms, dst, buffer) \
    DESERIALIZE_ASSERT(MemoryStream_ReadChar(ms, &buffer, NULL) || buffer != TOKEN_BEGIN_VARIABLE); \
    DESERIALIZE_ASSERT(MemoryStream_ReadBytes(ms, (uint8_t*)&dst, NULL, sizeof(dst))); \
    DESERIALIZE_ASSERT(MemoryStream_ReadChar(ms, &buffer, NULL) || buffer != TOKEN_END_VARIABLE);

#define DESERIALIZE_OWN_SIMPLEVAR(ms, varname, buffer) DESERIALIZE_SIMPLEVAR(ms, F_GETPVAR(varname), buffer)

#define CIF(classinstance, name, ...) CALL_I_FUNCTION(classinstance, name, __VA_ARGS__)
#define CRF(classreference, name, ...) CALL_R_FUNCTION(classreference, name, __VA_ARGS__)