


#pragma once

#include "../macro.h"
#include "../ClassInstance.h"
#include <assert.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdio.h>
  

#undef TYPE
#define TYPE CentralPixelPool

typedef struct Cell {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    char mass;
    bool isOccupied;
    bool isOpaque;

} Cell;

#define LOG2_POW2(n) (__builtin_ctz(n))
#define INTERNAL_POOL_SIZE (2048)
#define POOL_AREA (INTERNAL_POOL_SIZE * INTERNAL_POOL_SIZE)
enum { POOL_SIZE = INTERNAL_POOL_SIZE };
enum { POOL_SHIFT = LOG2_POW2(POOL_SIZE) };

#define POOL2D_TO_POOL1D(poolX, poolY) (((poolX) << POOL_SHIFT) | (poolY))
#define POOL1D_TO_POOLX(poolIndex) (poolIndex & (POOL_SIZE - 1))
#define POOL1D_TO_POOLY(poolIndex) (poolIndex >> POOL_SHIFT)

#define POOL1D_TO_COLOR1D(poolIndex) (poolIndex * 4)
#define POOL1D_TO_COLOR1D_R(poolIndex) (POOL1D_TO_COLOR1D(poolIndex) + 0)
#define POOL1D_TO_COLOR1D_G(poolIndex) (POOL1D_TO_COLOR1D(poolIndex) + 1)
#define POOL1D_TO_COLOR1D_B(poolIndex) (POOL1D_TO_COLOR1D(poolIndex) + 2)
#define POOL1D_TO_COLOR1D_A(poolIndex) (POOL1D_TO_COLOR1D(poolIndex) + 3)

#define LOCAL2D_TO_LOCAL1D(localX, localY, rectW) ((localY * rectW) + localX)
#define LOCAL1D_TO_LOCALX(localIndex, rectW) (localIndex % rectW)
#define LOCAL1D_TO_LOCALY(localIndex, rectW) (localIndex / rectW)
#define LOCALX_TO_POOLX(localX, rectX) (localX + rectX)
#define LOCALY_TO_POOLY(localY, rectY) (localY + rectY)
#define LOCAL2D_TO_POOL1D(localX, localY, rectX, rectY) (POOL2D_TO_POOL1D(localX + rectX, localY + rectY))



BEGIN_CLASS(0xA001, Central Pixel Pool);

    DEFINE_STRUCT(
        int val1; 
        int val2;
    )

    inline int wee = 0;

    

    
    

