//Functions.h

#pragma once

#include "IDs.h"

typedef void* PrmStructAddress;

typedef struct FunCall {
    FunctionID fid;
    PrmStructAddress prm;
} FunCall;

