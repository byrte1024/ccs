//Functions.h

#pragma once

#include "IDs.h"
#include <stdbool.h>

typedef void* PrmStructAddress;
typedef struct FunCallStack FunCallStack;
typedef struct FunCall FunCall;

typedef struct FunCall {
    FunctionID fid;
    PrmStructAddress prm;
} FunCall;

typedef struct FunCallStack {

    FunCallStack* up;
    size_t depth;    

    FunCall* current;


} FunCallStack;

