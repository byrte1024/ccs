#include "system/macro.h"
#ifndef INTESTING

#include <raylib.h>
#include <stdio.h>

#include <raylib.h>

#include "system/ClassInstance.h"
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

    assert(C_TestType_REGISTER());
    assert(C_CentralPixelPool_REGISTER());

    CPP_Handle handle = { 0 };

    SetRandomSeed(23);

    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width =32, .height = 32, .out_handle = &handle);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width =4, .height = 4, .out_handle = &handle);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width =16, .height = 16, .out_handle = &handle);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width =2, .height = 2, .out_handle = &handle);

    for(int y = 0; y < POOL_SIZE; y++) {
        for(int x = 0; x < POOL_SIZE; x++) {
            Cell cell = CSS(CentralPixelPool).cells[POOL2D_TO_POOL1D(x,y)];
            if(cell.isOccupied){
                printf("X");
            }
            else{
                printf("-");
            }
        }
        printf("\n");
    }

    CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);

    
    for(int y = 0; y < POOL_SIZE; y++) {
        for(int x = 0; x < POOL_SIZE; x++) {
            Cell cell = CSS(CentralPixelPool).cells[POOL2D_TO_POOL1D(x,y)];
            if(cell.isOccupied){
                printf("X");
            }
            else{
                printf("-");
            }
        }
        printf("\n");
    }

    
    CF(CentralPixelPool, CentralPixelPool_rentHandle, .width =2, .height = 2, .out_handle = &handle);

    
    
    for(int y = 0; y < POOL_SIZE; y++) {
        for(int x = 0; x < POOL_SIZE; x++) {
            Cell cell = CSS(CentralPixelPool).cells[POOL2D_TO_POOL1D(x,y)];
            if(cell.isOccupied){
                printf("X");
            }
            else{
                printf("-");
            }
        }
        printf("\n");
    }
    
    getchar();

    return 0;
}

#endif