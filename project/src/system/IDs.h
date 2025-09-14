//IDs.h

#pragma once

#include <stdint.h>

typedef uint16_t ClassID;
typedef uint16_t LocalFunctionID;
typedef uint32_t FunctionID; //Function ID is made of a cid and a localfid


#define COMPOSE_FUNCTIONID(localfid, cid) (((FunctionID)(cid) << 16) | ((FunctionID)(localfid)))
#define GET_CID(fid) ((ClassID)((fid) >> 16))
#define GET_LOCALFID(fid) ((LocalFunctionID)((fid) & 0xFFFF))


#define CID_MAX UINT16_MAX
#define LFID_MAX UINT16_MAX
#define FID_MAX UINT32_MAX
