#ifndef INTESTING

#define VERBOSE
#include "system/Log.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define RAYGUI_IMPLEMENTATION
#include <raylib.h>
#include <raygui.h>
#undef RAYGUI_IMPLEMENTATION

#include "system/ClassInstance.h"
#include "system/macro.h"
#include "system/instance_macro.h"
#include "system/types/CentralPixelPool.h"
#include "system/types/GameObject.h"

#include "system/types/CTransform.h"

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

    ASSERT(C_CentralPixelPool_REGISTER());
    ASSERT(C_GameObject_REGISTER());
    ASSERT(C_CTransform_REGISTER());

    CPP_Handle handle;
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width = 512, .height = 512, .out_handle = &handle);

    CF(CentralPixelPool, CentralPixelPool_startRect, .startas = (RectangleInt){0,0,0,0} );
    for (int i = 0; i < 512 * 512; i++) {
        int x = LOCAL1D_TO_LOCALX(i, handle.rectWidth);
        int y = LOCAL1D_TO_LOCALY(i, handle.rectWidth);
    
        float fx = (float)x / 512.0f;
        float fy = (float)y / 512.0f;
    
        float dx = fx - 0.5f;
        float dy = fy - 0.5f;
    
        float dist = sqrt(dx * dx + dy * dy) * 2.0f;
        float angle = atan2(dy, dx);
    
        float warp = sin(fx * 40.0f + fy * 37.0f) * 0.5f + cos(fx * 90.0f - fy * 80.0f) * 0.5f;

        float harshR = (sin(dist * 120.0f + angle * 60.0f) > 0.0f) ? 1.0f : 0.0f;
        float harshG = (sin(dist * 80.0f - angle * 90.0f + warp * 3.0f) > 0.0f) ? 1.0f : 0.0f;
        float harshB = (sin(dist * 200.0f + angle * 20.0f) > 0.0f) ? 1.0f : 0.0f;

        float flicker = (2.0f * fabsf(fx - 0.5f)) * (2.0f * fabsf(fy - 0.5f) + 1e-5f);
    
        char r = (char)CLAMP(harshR * 255.0f * fabsf(flicker), 0.0f, 255.0f);
        char g = (char)CLAMP(harshG * 255.0f * fabsf(1.0f - flicker), 0.0f, 255.0f);
        char b = (char)CLAMP(harshB * 255.0f, 0.0f, 255.0f);
        char a = 255;
    
        SET_LOCAL2D_COLOR(x, y, handle.rectX, handle.rectY, r, g, b, a);

    }
    
    CF(CentralPixelPool, CentralPixelPool_notifyChanges, .changed = (RectangleInt){ .x = handle.rectX , .y = handle.rectY, .width = handle.rectWidth, .height = handle.rectHeight });
    CF(CentralPixelPool, CentralPixelPool_endRect, );

    ClassReference transformRef = Class_Reference_AllocateEmptyInstance();
    transformRef.instance->cid = CID_CTransform;
    CRF(transformRef, DEF_CREATE, );
    Class_Reference_PrintStatus(transformRef);

    GETISTRUCT(transformRef.instance, CTransform, transform);

    transform->x = 1;
    transform->y = 23;
    transform->z = 596;

    

    MemoryStream ms = { 0 };
    CRF(transformRef, DEF_SERIALIZE, .stream = &ms );



    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //DrawText("Quadtree", 10, 10, 20, DARKGRAY);

        DrawRectangle(0, 0, POOL_SIZE, POOL_SIZE, BLACK);
        DrawTextureEx(CSS(CentralPixelPool).gpuTex, (Vector2){0,0}, 0, 1, WHITE);

        EndDrawing();
    }

    return 0;
}


#endif