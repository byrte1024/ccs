#ifndef INTESTING

#include <raylib.h>
#include <stdio.h>

#include <raylib.h>

#include "system/ClassInstance.h"
#include "system/macro.h"
#include "system/instance_macro.h"
#include "system/types/testtype.h"
#include "system/types/CentralPixelPool.h"

#include "utils/MemoryStream.h"
#include "utils/ResizeableArray.h"
#include "utils/HashSet.h"
#include "utils/Dictionary.h"

size_t int_hash(void* val) {
    return *(int*)val;
}
bool int_equals(void* a, void* b) {
    return *(int*)a == *(int*)b;
}

int main(void) {

    InitWindow(1080,608, "Test");

    assert(C_TestType_REGISTER());
    assert(C_CentralPixelPool_REGISTER());

    CPP_Handle handle;
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width = 256, .height = 256, .out_handle = &handle);

    CF(CentralPixelPool, CentralPixelPool_startRect, .startas = (RectangleInt){0,0,0,0} );
    for(int i = 0; i < 256*256; i++){
        int x = LOCAL1D_TO_LOCALX(i, handle.rectWidth);
        int y = LOCAL1D_TO_LOCALY(i, handle.rectWidth);

        int px = LOCALX_TO_POOLX(x, handle.rectX);
        int py = LOCALY_TO_POOLY(y, handle.rectY);

        int pi = POOL2D_TO_POOL1D(px, py);

        int ri = POOL1D_TO_COLOR1D_R(pi);
        int gi = POOL1D_TO_COLOR1D_G(pi);
        int bi = POOL1D_TO_COLOR1D_B(pi);
        int ai = POOL1D_TO_COLOR1D_A(pi);

        char r = (char)ri * 1;
        char g = (char)gi * 3;
        char b = (char)bi * 7;
        char a = (char)255;

        CSS(CentralPixelPool).cells[pi].r = r;
        CSS(CentralPixelPool).cells[pi].g = g;
        CSS(CentralPixelPool).cells[pi].b = b;
        CSS(CentralPixelPool).cells[pi].a = a;

        CSS(CentralPixelPool).rgbaData[ri] = r;
        CSS(CentralPixelPool).rgbaData[gi] = g;
        CSS(CentralPixelPool).rgbaData[bi] = b;
        CSS(CentralPixelPool).rgbaData[ai] = a;
    }
    CF(CentralPixelPool, CentralPixelPool_notifyChanges, .changed = (RectangleInt){ .x = handle.rectX , .y = handle.rectY, .width = handle.rectWidth, .height = handle.rectHeight });
    CF(CentralPixelPool, CentralPixelPool_endRect, );

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //DrawText("Quadtree", 10, 10, 20, DARKGRAY);

        DrawRectangle(0, 0, POOL_SIZE, POOL_SIZE, BLACK);
        DrawTexture(CSS(CentralPixelPool).gpuTex, 0, 0, WHITE);

        EndDrawing();
    }

    return 0;
}

#endif