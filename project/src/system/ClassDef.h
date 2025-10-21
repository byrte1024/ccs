//ClassDef.h

#pragma once

#include <raylib.h>

#include "IDs.h"
#include "Functions.h"
#include "macro.h"

typedef struct ClassDef {
    ClassID id;
    const char* name;

    bool (*hasFunction)(FunctionID);
    void (*callFunction)(FunCall*);
} ClassDef;

static ClassDef class_def[CID_MAX] = { 0 };

#undef TYPE
#define TYPE DEF

BEGIN_CLASS(0x0000, "DEF");


static const ClassDef* Class_System_GetDefinition(const ClassID cid) {
    return &class_def[cid];
}
static bool Class_System_HasDefinition(const ClassID cid) {
    return cid != CID_DEF && class_def[cid].id == cid;
}
static const char* Class_System_GetDefinitionName(const ClassID cid){
    if(cid == CID_DEF){
        return "CID_DEF";
    }
    if(Class_System_HasDefinition(cid)){
        return class_def[cid].name;
    }
    return "???";
}
static bool Class_System_RegisterDefinition(const ClassDef def) {
    if (def.id == CID_DEF) {
        TraceLog(LOG_ERROR, "Class %s has invalid id", def.name);
        return false;
    }
    if (Class_System_HasDefinition(def.id)) {
        TraceLog(LOG_ERROR, "Class %s has already been registered", def.name);
        return false;
    }

    class_def[def.id] = def;
    TraceLog(LOG_INFO, "Class %s has been succesfully registered!", def.name);
    return true;
}

static void Class_Definition_CallFunction(const ClassID cid, FunCall* call) {
    if (Class_System_HasDefinition(cid)) {
        const ClassDef* def = Class_System_GetDefinition(cid);
        if (def->callFunction) {
            def->callFunction((FunCall*)call);
        }
        else {
            TraceLog(LOG_ERROR, "Class %s has no callFunction", def->name);
        }
    }
    else {
        TraceLog(LOG_ERROR, "Class %d has no definition", cid);
    }
}
static bool Class_Definition_HasFunction(const ClassID id, const FunctionID fid) {
    if (Class_System_HasDefinition(id)) {
        const ClassDef* def = Class_System_GetDefinition(id);

        if (def->hasFunction) {
            return def->hasFunction(fid);
        }
        else {
            TraceLog(LOG_ERROR, "Class %s has no hasFunction", def->name);
        }
    }
    return false;
}

