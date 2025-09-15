//ClassInstance.h

#pragma once

#include "ClassDef.h"
#include "instance_macro.h"
#include "IDs.h"
#include "macro.h"

#include <raylib.h>
#include <stdlib.h>
#include <string.h>

typedef void* InstanceDataPtr;

typedef struct ClassInstance {

    InstanceDataPtr data;
    ClassID cid;

    size_t ref_count;

} ClassInstance;

typedef struct ClassReference {

    ClassInstance* instance;

} ClassReference;


/*
 *  Brief explenation on class instance states.
 *   __________________________________________________________________
 *  |  InstaPtr  |  DataPtr  |  ClassID  |  RefCount  |      Name      |
 *  |            |           |           |            |                |
 *  |    NULL    |    ???    |    ???    |    ???     |     NULL       | A completely null instance (All other variables cannot be known.)
 *  |  NOT NULL  |    ???    |    ???    |    ???     |     ALLOCATED  | An existing allocated instance.
 *  |  NOT NULL  |    NULL   |    ???    |    ???     |     DEAD       | An instance that contains no data.
 *  |  NOT NULL  |  NOT NULL |    ???    |    ???     |     ALIVE      | An instance that contains data
 *  |  NOT NULL  |    ???    |   STATIC  |    ???     |     UNTYPED    | An instance with a CID that does not support instances. (most commonly CID_DEF)
 *  |  NOT NULL  |    ???    |   DYNAMIC |    ???     |     TYPED      | An instance with a CID that supports instances.
 *  |  NOT NULL  |    ???    |    ???    |    == 0    |     FORGOTTEN  | An instance with no references.
 *  |  NOT NULL  |    ???    |    ???    |    > 0     |     REMEMBERED | An instance with references.
 *  |  NOT NULL  |    NULL   |   CID_DEF |    ???     |     EMPTY      | An instance that contains no data or CID.
 *   ------------------------------------------------------------------
 *
 */
static inline bool Class_Instance_Status_IsNull(const ClassInstance* _instance) {
    return _instance == NULL;
}
static inline bool Class_Instance_Status_IsAllocated(const ClassInstance* _instance) {
    return _instance != NULL;
}
static inline bool Class_Instance_Status_IsDead(const ClassInstance* _instance) {
    return _instance != NULL && _instance->data == NULL;
}
static inline bool Class_Instance_Status_IsAlive(const ClassInstance* _instance) {
    return _instance != NULL && _instance->data != NULL;
}
static inline bool Class_Instance_Status_IsUntyped(const ClassInstance* _instance) {
    return _instance != NULL && _instance->cid == CID_DEF;
}
static inline bool Class_Instance_Status_IsTyped(const ClassInstance* _instance) {
    return _instance != NULL && _instance->cid != CID_DEF;
}
static inline bool Class_Instance_Status_IsForgotten(const ClassInstance* _instance) {
    return _instance != NULL && _instance->ref_count == 0;
}
static inline bool Class_Instance_Status_IsRemembered(const ClassInstance* _instance) {
    return _instance != NULL && _instance->ref_count > 0;
}
static inline bool Class_Instance_Status_IsEmpty(const ClassInstance* _instance) {
    return _instance != NULL && _instance->data == NULL && _instance->cid == CID_DEF;
}

static inline bool Class_Reference_Status_IsNull(const ClassReference _reference){
    return Class_Instance_Status_IsNull(_reference.instance);
}
static inline bool Class_Reference_Status_IsAllocated(const ClassReference _reference){
    return Class_Instance_Status_IsAllocated(_reference.instance);
}
static inline bool Class_Reference_Status_IsDead(const ClassReference _reference){
    return Class_Instance_Status_IsDead(_reference.instance);
}
static inline bool Class_Reference_Status_IsAlive(const ClassReference _reference){
    return Class_Instance_Status_IsAlive(_reference.instance);
}
static inline bool Class_Reference_Status_IsUntyped(const ClassReference _reference){
    return Class_Instance_Status_IsUntyped(_reference.instance);
}
static inline bool Class_Reference_Status_IsTyped(const ClassReference _reference){
    return Class_Instance_Status_IsTyped(_reference.instance);
}
static inline bool Class_Reference_Status_IsForgotten(const ClassReference _reference){
    return Class_Instance_Status_IsForgotten(_reference.instance);
}
static inline bool Class_Reference_Status_IsRemembered(const ClassReference _reference){
    return Class_Instance_Status_IsRemembered(_reference.instance);
}
static inline bool Class_Reference_Status_IsEmpty(const ClassReference _reference){
    return Class_Instance_Status_IsEmpty(_reference.instance);
}

#undef TYPE
#define TYPE DEF

DEFINE_I_FUNCTION(0x0001,   CREATE       , );
DEFINE_I_FUNCTION(0x0002,   DESTROY      , );
DEFINE_I_FUNCTION(0x0003,   TOSTRING     , );
DEFINE_I_FUNCTION(0x0003,   SERIALIZE    , );
DEFINE_I_FUNCTION(0x0004,   DESERIALIZE  , );


DEFINE_I_FUNCTION_WRAPPER(  CREATE       , {
    if (!Class_Instance_Status_IsAllocated(prm->self)) {
        prm->code = FUN_WRONGARGS;
        return prm;
    }
    if (!Class_Instance_Status_IsTyped(prm->self)) {
        prm->code = FUN_WRONGARGS;
        return prm;
    }
    if (Class_Instance_Status_IsAlive(prm->self)) {
        if (CALL_I_FUNCTION(prm->self,DEF_DESTROY,)->code != FUN_OK) {
            prm->code = FUN_ERROR;
            return prm;
        }
    }
},{

});
DEFINE_I_FUNCTION_WRAPPER(  DESTROY      , {
    if (!Class_Instance_Status_IsAllocated(prm->self)) {
        prm->code = FUN_WRONGARGS;
        return prm;
    }
    if (Class_Instance_Status_IsDead(prm->self)) {
        prm->code = FUN_WRONGARGS;
        return prm;
    }

},{
    prm->self->data = NULL;
});

static ClassInstance* Class_Instance_AllocateEmpty() {
    ClassInstance* instance = malloc(sizeof(ClassInstance));
    if(instance == NULL){
        TraceLog(LOG_ERROR, "Error while allocating instance.");
        return NULL;
    }
    memset(instance, 0, sizeof(ClassInstance));
    instance->data = NULL;
    instance->cid = CID_DEF;
    instance->ref_count = 0;
    return instance;
}

static void Class_Instance_FreeEmpty(ClassInstance* instance) {
    if(Class_Instance_Status_IsEmpty(instance)){
        free(instance);
    }
    else{
        TraceLog(LOG_ERROR, "Instance is not empty.");
    }
}


static void Class_Instance_PrintStatus(ClassInstance* instance) {
    TraceLog(LOG_INFO, TextFormat("Instance: %p, Data: %p, CID: %d, RefCount: %d",
        instance,
        instance == NULL ? NULL : instance->data,
        instance == NULL ? CID_DEF : instance->cid,
        instance == NULL ? (long long int)-1 : (long long int)instance->ref_count)
        );
}



static ClassReference Class_Reference_CreateOf(ClassInstance* instance ){
    ClassReference reference;
    reference.instance = instance;
    if(instance){
        instance->ref_count++;
    }
    return reference;
}

static ClassReference Class_Reference_CreateFrom(ClassReference reference ){
    if(reference.instance){
        reference.instance->ref_count++;
    }
    return reference;
} 

static ClassReference Class_Reference_AllocateEmptyInstance(){
    return Class_Reference_CreateOf(Class_Instance_AllocateEmpty());
}

static void INTERNAL_Class_Reference_Forget(ClassReference* reference){
    if(reference->instance){
        reference->instance->ref_count--;

        if(reference->instance->ref_count == 0){

            if(Class_Reference_Status_IsTyped(*reference)){

                if(Class_Reference_Status_IsAlive(*reference)){

                    
                    if(CALL_I_FUNCTION(reference->instance, DEF_DESTROY, )->code != FUN_OK) {

                        TraceLog(LOG_ERROR, "Error while destroying instance.");
                        
                    }
                }
            }
            reference->instance->data = NULL;   
            reference->instance->cid = CID_DEF;

            Class_Instance_FreeEmpty(reference->instance);

        }

        reference->instance = NULL;
    }
}
#define Class_Reference_Forget(reference) INTERNAL_Class_Reference_Forget(&reference)

#define Class_Reference_PrintStatus(reference) Class_Instance_PrintStatus(reference.instance)