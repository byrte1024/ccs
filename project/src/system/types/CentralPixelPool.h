//CentralPixelPool.h

#pragma once

#include "../macro.h"
#include "../ClassInstance.h"
#include "../../utils/BinaryUtils.h"
#include <assert.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdbool.h>
#include <stdio.h>
  

#define LOG2_POW2(n) (__builtin_ctz(n))
#define INTERNAL_POOL_SIZE (128)
//the size before an edit rect just uses the main pool buffer
#define INTERNAL_POOL_EDITRECT_SIZE  (INTERNAL_POOL_SIZE / 2)
#define POOL_AREA (INTERNAL_POOL_SIZE * INTERNAL_POOL_SIZE)
#define EDITRECT_AREA (INTERNAL_POOL_EDITRECT_SIZE * INTERNAL_POOL_EDITRECT_SIZE)
enum { POOL_SIZE = INTERNAL_POOL_SIZE };
enum { POOL_SHIFT = LOG2_POW2(POOL_SIZE) };

_Static_assert((1 << POOL_SHIFT) == POOL_SIZE, "POOL_SHIFT and POOL_SIZE mismatch");

#define MINBLOCKSIZE 2
#define MINBLOCKLOG 1
#define MAXLEVEL (POOL_SHIFT - MINBLOCKLOG)

#define POOL2D_TO_POOL1D(poolX, poolY) (((poolY) << POOL_SHIFT) | (poolX))
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

// Quadtree allocator structures
typedef struct FreeBlock {
    int x, y;           // Position in pool
    struct FreeBlock* next;
} FreeBlock;

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


typedef struct CPP_Handle {
    int rectX;
    int rectY;
    int rectWidth;
    int rectHeight;
    bool valid;

} CPP_Handle;


BEGIN_CLASS(0xA001, Central Pixel Pool);

    DEFINE_STRUCT(

        Cell* cells;
        char* rgbaData;

        bool editRectActive;
        Rectangle editRect;
        char* editRectRGBA;

        // Quadtree allocator data
        // freeLists[level] contains all free blocks of size (MINBLOCKSIZE << level)
        // level 0 = 2x2, level 1 = 4x4, ..., level MAXLEVEL = 128x128
        FreeBlock* freeLists[MAXLEVEL + 1];
        
        // Block pool for free list nodes (preallocated to avoid malloc in hot path)
        FreeBlock* blockPool;
        int blockPoolSize;
        int blockPoolUsed;

    );

    DEFINE_FUNCTION(0x001A, rentHandle, int width; int height; CPP_Handle* out_handle)
    DEFINE_FUNCTION_WRAPPER(rentHandle, {
        if(prm->width <= 0 || prm->height <= 0 || prm->width > POOL_SIZE || prm->height > POOL_SIZE || prm->out_handle == NULL){
            prm->code = FUN_WRONGARGS;
            return prm;
        }
    },{
        if(!prm->out_handle->valid){
            prm->code = FUN_ERROR;
            return prm;
        }
    })

    DEFINE_FUNCTION(0x001B, evictHandle, CPP_Handle* handle )
    DEFINE_FUNCTION_WRAPPER(evictHandle, {
        if(prm->handle == NULL || !prm->handle->valid){
            prm->code = FUN_WRONGARGS;
            return prm;
        }
    }, {
        if(prm->handle->valid){
            prm->code = FUN_ERROR;
            return prm;
        }
    })

    IMPL_INITTER({
        TraceLog(LOG_INFO, "Initializing CentralPixelPool");

        TCSS.cells = malloc(sizeof(Cell) * POOL_AREA);
        if(TCSS.cells == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for cells"); prm->code = FUN_ERROR; return; }
        memset(TCSS.cells, 0, sizeof(Cell) * POOL_AREA);

        TCSS.rgbaData = malloc(sizeof(char) * POOL_AREA * 4);
        if(TCSS.rgbaData == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for rgbaData"); prm->code = FUN_ERROR; free(TCSS.cells); return; }
        memset(TCSS.rgbaData, 0, sizeof(char) * POOL_AREA * 4);

        TCSS.editRectActive = false;
        
        TCSS.editRect = (Rectangle){0, 0, 0, 0};

        TCSS.editRectRGBA = malloc(sizeof(char) * EDITRECT_AREA * 4);
        if(TCSS.editRectRGBA == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for editRectRGBA"); prm->code = FUN_ERROR; free(TCSS.cells); free(TCSS.rgbaData); return; }
        memset(TCSS.editRectRGBA, 0, sizeof(char) * EDITRECT_AREA * 4);

        // Initialize quadtree allocator
        // Worst case: all blocks are minimum size = (POOL_SIZE/MINBLOCKSIZE)^2 blocks
        TCSS.blockPoolSize = (POOL_SIZE / MINBLOCKSIZE) * (POOL_SIZE / MINBLOCKSIZE);
        TCSS.blockPool = malloc(sizeof(FreeBlock) * TCSS.blockPoolSize);
        if(TCSS.blockPool == NULL) {
            TraceLog(LOG_ERROR, "Failed to allocate block pool");
            prm->code = FUN_ERROR;
            free(TCSS.cells);
            free(TCSS.rgbaData);
            free(TCSS.editRectRGBA);
            return;
        }
        TCSS.blockPoolUsed = 0;

        // Initialize free lists
        for(int i = 0; i <= MAXLEVEL; i++) {
            TCSS.freeLists[i] = NULL;
        }

        // Start with one free block at max level (entire pool)
        TCSS.freeLists[MAXLEVEL] = &TCSS.blockPool[TCSS.blockPoolUsed++];
        TCSS.freeLists[MAXLEVEL]->x = 0;
        TCSS.freeLists[MAXLEVEL]->y = 0;
        TCSS.freeLists[MAXLEVEL]->next = NULL;

        TraceLog(LOG_INFO, "Quadtree allocator initialized: MAXLEVEL=%d, MINBLOCKSIZE=%d", MAXLEVEL, MINBLOCKSIZE);
    })

    

    IMPLOTHER_FUNCTION(CentralPixelPool_rentHandle, {
        int width = prm->width;
        int height = prm->height;
        int bigger = width > height ? width : height;
        width = next_power_of_2(bigger);
        height = next_power_of_2(bigger);
        
        if (width < MINBLOCKSIZE) width = MINBLOCKSIZE;
        if (height < MINBLOCKSIZE) height = MINBLOCKSIZE;
        CPP_Handle* out_handle = prm->out_handle;
    
        if (width <= 0 || height <= 0 || width > POOL_SIZE || height > POOL_SIZE) {
            TraceLog(LOG_ERROR, "Invalid handle size requested: %dx%d", width, height);
            out_handle->valid = false;
            prm->code = FUN_ERROR;
            return;
        }

        // Calculate required level (block size = MINBLOCKSIZE << level)
        int size = (width > height) ? width : height;
        if (size < MINBLOCKSIZE) size = MINBLOCKSIZE;
        int level = LOG2_POW2(size) - MINBLOCKLOG;

        if (level < 0 || level > MAXLEVEL) {
            TraceLog(LOG_ERROR, "Invalid level %d for size %d", level, size);
            out_handle->valid = false;
            prm->code = FUN_ERROR;
            return;
        }

        // Try to find a free block at this level
        FreeBlock* block = TCSS.freeLists[level];
        
        // If no block at this level, split from higher levels
        if (block == NULL) {
            // Find the smallest available block that's larger
            int splitLevel = -1;
            for (int l = level + 1; l <= MAXLEVEL; l++) {
                if (TCSS.freeLists[l] != NULL) {
                    splitLevel = l;
                    break;
                }
            }

            if (splitLevel == -1) {
                TraceLog(LOG_WARNING, "No space left in pool for size %d (level %d)", size, level);
                out_handle->valid = false;
                prm->code = FUN_ERROR;
                return;
            }

            // Split blocks down to required level
            for (int l = splitLevel; l > level; l--) {
                // Take a block from level l
                FreeBlock* largeBlock = TCSS.freeLists[l];
                TCSS.freeLists[l] = largeBlock->next;

                int blockSize = MINBLOCKSIZE << l;
                int halfSize = blockSize / 2;
                int bx = largeBlock->x;
                int by = largeBlock->y;

                // Split into 4 quadrants and add to level l-1
                for (int q = 0; q < 4; q++) {
                    if (TCSS.blockPoolUsed >= TCSS.blockPoolSize) {
                        TraceLog(LOG_ERROR, "Block pool exhausted!");
                        out_handle->valid = false;
                        prm->code = FUN_ERROR;
                        return;
                    }

                    FreeBlock* newBlock = &TCSS.blockPool[TCSS.blockPoolUsed++];
                    newBlock->x = bx + ((q & 1) ? halfSize : 0);
                    newBlock->y = by + ((q & 2) ? halfSize : 0);
                    newBlock->next = TCSS.freeLists[l - 1];
                    TCSS.freeLists[l - 1] = newBlock;
                }
            }

            block = TCSS.freeLists[level];
        }

        // Allocate the block
        if (block == NULL) {
            TraceLog(LOG_ERROR, "Allocation failed after split!");
            out_handle->valid = false;
            prm->code = FUN_ERROR;
            return;
        }

        TCSS.freeLists[level] = block->next;

        int foundX = block->x;
        int foundY = block->y;

        // Mark region as occupied
        for (int yy = 0; yy < height; yy++) {
            for (int xx = 0; xx < width; xx++) {
                int idx = POOL2D_TO_POOL1D(foundX + xx, foundY + yy);
                TCSS.cells[idx].isOccupied = true;
            }
        }

        out_handle->rectX = foundX;
        out_handle->rectY = foundY;
        out_handle->rectWidth = width;
        out_handle->rectHeight = height;
        out_handle->valid = true;

        TraceLog(LOG_INFO, "Rented handle at (%d, %d) size %dx%d (level %d)", foundX, foundY, width, height, level);
        prm->code = FUN_OK;
    })    

    IMPLOTHER_FUNCTION(CentralPixelPool_evictHandle, {
        CPP_Handle* handle = prm->handle;
        if (handle == NULL || !handle->valid) {
            TraceLog(LOG_WARNING, "Tried to evict invalid handle");
            prm->code = FUN_ERROR;
            return;
        }
    
        int x = handle->rectX;
        int y = handle->rectY;
        int w = handle->rectWidth;
        int h = handle->rectHeight;

        // Calculate level
        int size = (w > h) ? w : h;
        int level = LOG2_POW2(size) - MINBLOCKLOG;

        // Free region
        for (int yy = 0; yy < h; yy++) {
            for (int xx = 0; xx < w; xx++) {
                int idx = POOL2D_TO_POOL1D(x + xx, y + yy);
                TCSS.cells[idx].isOccupied = false;
            }
        }

        // Return block to free list
        if (TCSS.blockPoolUsed >= TCSS.blockPoolSize) {
            TraceLog(LOG_ERROR, "Block pool exhausted during eviction!");
            handle->valid = false;
            prm->code = FUN_ERROR;
            return;
        }

        FreeBlock* freedBlock = &TCSS.blockPool[TCSS.blockPoolUsed++];
        freedBlock->x = x;
        freedBlock->y = y;
        freedBlock->next = TCSS.freeLists[level];
        TCSS.freeLists[level] = freedBlock;

        handle->valid = false;
    
        TraceLog(LOG_INFO, "Evicted handle at (%d, %d) size %dx%d (level %d)", x, y, w, h, level);
        prm->code = FUN_OK;
    })
    


    BEGIN_FUNFIND()

        FUNFIND_INITTER();

        FUNFIND_IMPL(rentHandle);
        FUNFIND_IMPL(evictHandle);

    END_FUNFIND()

END_CLASS()