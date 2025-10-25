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
#define POOL_AREA (INTERNAL_POOL_SIZE * INTERNAL_POOL_SIZE)
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

#undef TYPE
#define TYPE CentralPixelPool

typedef struct Cell {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    char mass;
    bool isOpaque;

} Cell;


typedef struct CPP_Handle {
    int rectX;
    int rectY;
    int rectWidth;
    int rectHeight;
    int level;
    bool valid;

} CPP_Handle;

typedef struct RectangleInt {
    int x;
    int y;
    int width;
    int height;
} RectangleInt;


#define NODESTATE_ALLOCATED ((char)0x02) //this exact node has been allocated
#define NODESTATE_PARENT ((char)0x01) //this node contains other nodes which have been allocated
#define NODESTATE_FREE ((char)0x00) //this node is free to be allocated
typedef struct NodeState {
    char state;
    char largest_freeblock; //only relavent when NODESTATE_PARENT, represents the largest free block in the node, uses levels. level = charmax means it is fully filled.
} NodeState;

static inline int getNodeSize(int level){
    return MINBLOCKSIZE << level;
}

static inline int getNodeCount(int level) {
    int blockSize = MINBLOCKSIZE << level;     // block size at this level
    int blocksPerDim = POOL_SIZE / blockSize;  // number of blocks per row/column
    return blocksPerDim * blocksPerDim;        // total blocks
}


//where in the nodeStates[MAXLEVEL+1] array is the node at level 'level'
static inline int getNodeIndex(int poolX, int poolY, int level){
    int nodeSize = getNodeSize(level);
    int nodeX = poolX / nodeSize;
    int nodeY = poolY / nodeSize;
    return (nodeY * (POOL_SIZE / nodeSize)) + nodeX;
}

BEGIN_CLASS(0xA001, Central Pixel Pool);

    DEFINE_STRUCT(

        Cell* cells;
        bool* occupied;
        char* rgbaData;

        bool editRectActive;
        RectangleInt editRect;
        char* editRectRGBA;

        NodeState* nodeStates[MAXLEVEL + 1];

    );

    DEFINE_FUNCTION(0x0010, rentHandle, int width; int height; CPP_Handle* out_handle)
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
        IMPL_FUNCTION(rentHandle, {
            int width = prm->width;
            int height = prm->height;
            int bigger = width > height ? width : height;
            width = next_power_of_2(bigger);
            height = next_power_of_2(bigger);
        
            if (width < MINBLOCKSIZE) width = MINBLOCKSIZE;
            if (height < MINBLOCKSIZE) height = MINBLOCKSIZE;
            CPP_Handle* out_handle = prm->out_handle;
        
            int levelNeeded = LOG2_POW2(width) - MINBLOCKLOG;
        
            if (levelNeeded > MAXLEVEL) {
                prm->code = FUN_ERROR;
                return;
            }
        
            int foundX = -1;
            int foundY = -1;
        
            int searchRegionX = 0;
            int searchRegionY = 0;
            int searchRegionWidth = POOL_SIZE;
            int searchRegionHeight = POOL_SIZE;
        
            bool candidateConfirmed = false;
        
            for (int l = MAXLEVEL; l >= levelNeeded; l--) {
                int blockSize = getNodeSize(l);
        
                bool foundCandidate = false;
        
                for (int x = searchRegionX; x < searchRegionX + searchRegionWidth; x += blockSize) {
                    for (int y = searchRegionY; y < searchRegionY + searchRegionHeight; y += blockSize) {
                        int nodeIndex = getNodeIndex(x, y, l);
        
                        if (TCSS.nodeStates[l][nodeIndex].state == NODESTATE_FREE) {
                            foundCandidate = true;
                            searchRegionX = x;
                            searchRegionY = y;
                            searchRegionWidth = blockSize;
                            searchRegionHeight = blockSize;
        
                            if (l == levelNeeded) {
                                candidateConfirmed = true;
                                foundX = x;
                                foundY = y;
                                break;
                            }
                            break;
                        } else if (TCSS.nodeStates[l][nodeIndex].state == NODESTATE_PARENT) {
                            int largest_freeblock = TCSS.nodeStates[l][nodeIndex].largest_freeblock;
                            if (largest_freeblock >= levelNeeded) {
                                foundCandidate = true;
                                searchRegionX = x;
                                searchRegionY = y;
                                searchRegionWidth = blockSize;
                                searchRegionHeight = blockSize;
                                break;
                            }
                        }
                    }
                    if (foundCandidate) break;
                }
            }
        
            if (!candidateConfirmed || foundX < 0 || foundY < 0) {
                prm->code = FUN_ERROR;
                return;
            }
        
            // --- perform allocation and update node states upwards ---
            const char CHAR_FULL = (char)0xFF; // sentinel: fully filled (no free blocks) 
        
            // mark the allocated node 
            int allocIndex = getNodeIndex(foundX, foundY, levelNeeded);
            TCSS.nodeStates[levelNeeded][allocIndex].state = NODESTATE_ALLOCATED;
            // only meaningful for parents, but set to full for clarity 
            TCSS.nodeStates[levelNeeded][allocIndex].largest_freeblock = CHAR_FULL;
        
            // update parents up to MAXLEVEL
            for (int l = levelNeeded + 1; l <= MAXLEVEL; ++l) {
                int parentBlockSize = getNodeSize(l);
                int parentX = (foundX / parentBlockSize) * parentBlockSize;
                int parentY = (foundY / parentBlockSize) * parentBlockSize;
                int parentIndex = getNodeIndex(parentX, parentY, l);
        
                // Parent contains children at level (l-1) with childSize
                int childLevel = l - 1;
                int childSize = getNodeSize(childLevel);
        
                // compute the largest free block among the four children
                char parentLargest = CHAR_FULL; // start as 'no free'
        
                for (int cx = 0; cx <= 1; ++cx) {
                    for (int cy = 0; cy <= 1; ++cy) {
                        int childX = parentX + cx * childSize;
                        int childY = parentY + cy * childSize;
                        int childIndex = getNodeIndex(childX, childY, childLevel);
                        char candidate = CHAR_FULL;
        
                        char cstate = TCSS.nodeStates[childLevel][childIndex].state;
                        if (cstate == NODESTATE_FREE) {
                            candidate = (char)childLevel; // child is entirely free at childLevel
                        } else if (cstate == NODESTATE_PARENT) {
                            candidate = TCSS.nodeStates[childLevel][childIndex].largest_freeblock;
                        } else { // NODESTATE_ALLOCATED or unknown 
                            candidate = CHAR_FULL; // no free inside this child 
                        }
        
                        if (candidate != CHAR_FULL) {
                            if (parentLargest == CHAR_FULL || candidate > parentLargest) {
                                parentLargest = candidate;
                            }
                        }
                    }
                }
        
                // set parent state to indicate it contains allocated nodes 
                TCSS.nodeStates[l][parentIndex].state = NODESTATE_PARENT;
                TCSS.nodeStates[l][parentIndex].largest_freeblock = parentLargest;
            }

            //update occupancy values
            int index = POOL2D_TO_POOL1D(foundX,foundY);
            for(int y = foundY; y < foundY + height; y++){
                for(int x = foundX; x < foundX + width; x++){
                    TCSS.occupied[index++] = true;
                }
                index = index + POOL_SIZE - width;
            }
        
            // Fill out the handle info 
            out_handle->rectX = foundX;
            out_handle->rectY = foundY;
            out_handle->rectWidth = width;
            out_handle->rectHeight = height;
            out_handle->level = levelNeeded;
            out_handle->valid = true;
        
            prm->code = FUN_OK;
            return;
        })
        
        

    DEFINE_FUNCTION(0x0011, evictHandle, CPP_Handle* handle )
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
        IMPL_FUNCTION(evictHandle, {
            CPP_Handle* handle = prm->handle;
        
            int x = handle->rectX;
            int y = handle->rectY;
            int level = handle->level;
        
            const char CHAR_FULL = (char)0xFF;
        
            // Mark the node as free
            int nodeIndex = getNodeIndex(x, y, level);
            TCSS.nodeStates[level][nodeIndex].state = NODESTATE_FREE;
            TCSS.nodeStates[level][nodeIndex].largest_freeblock = level; // entire node is free
        
            // Update parents up to MAXLEVEL
            for (int l = level + 1; l <= MAXLEVEL; ++l) {
                int parentBlockSize = getNodeSize(l);
                int parentX = (x / parentBlockSize) * parentBlockSize;
                int parentY = (y / parentBlockSize) * parentBlockSize;
                int parentIndex = getNodeIndex(parentX, parentY, l);
        
                int childLevel = l - 1;
                int childSize = getNodeSize(childLevel);
        
                char parentLargest = CHAR_FULL;
                bool anyAllocated = false;
        
                for (int cx = 0; cx <= 1; ++cx) {
                    for (int cy = 0; cy <= 1; ++cy) {
                        int childX = parentX + cx * childSize;
                        int childY = parentY + cy * childSize;
                        int childIndex = getNodeIndex(childX, childY, childLevel);
        
                        char cstate = TCSS.nodeStates[childLevel][childIndex].state;
                        char candidate = CHAR_FULL;
        
                        if (cstate == NODESTATE_FREE) {
                            candidate = (char)childLevel;
                        } else if (cstate == NODESTATE_PARENT) {
                            candidate = TCSS.nodeStates[childLevel][childIndex].largest_freeblock;
                            anyAllocated = true;
                        } else if (cstate == NODESTATE_ALLOCATED) {
                            anyAllocated = true;
                        }
        
                        if (candidate != CHAR_FULL) {
                            if (parentLargest == CHAR_FULL || candidate > parentLargest) {
                                parentLargest = candidate;
                            }
                        }
                    }
                }
        
                // Update parent state 
                if (!anyAllocated) {
                    TCSS.nodeStates[l][parentIndex].state = NODESTATE_FREE;
                    TCSS.nodeStates[l][parentIndex].largest_freeblock = (char)l;
                } else {
                    TCSS.nodeStates[l][parentIndex].state = NODESTATE_PARENT;
                    TCSS.nodeStates[l][parentIndex].largest_freeblock = parentLargest;
                }
            }

            //update occupancy values
            int index = POOL2D_TO_POOL1D(handle->rectX,handle->rectY);
            for(int y = handle->rectY; y < handle->rectY + handle->rectHeight; y++){
                for(int x = handle->rectX; x < handle->rectX + handle->rectWidth; x++){
                    TCSS.occupied[index++] = false;
                }
                index = index + POOL_SIZE - handle->rectWidth;
            }
        
            // Mark handle invalid 
            handle->valid = false;
        
            prm->code = FUN_OK;
        })

    DEFINE_FUNCTION(0x001A, startRect, RectangleInt startas )
        DEFINE_FUNCTION_WRAPPER(startRect, {
            if(prm->startas.x < 0 || prm->startas.y < 0 || prm->startas.x + prm->startas.width > POOL_SIZE || prm->startas.y + prm->startas.height > POOL_SIZE){
                prm->code = FUN_WRONGARGS;
                return prm;
            }
            if(TCSS.editRectActive){
                prm->code = FUN_ERROR;
                return prm;
            }
        }, {})
        IMPL_FUNCTION(startRect, {
            RectangleInt startas = prm->startas;
            TCSS.editRect = startas;
            TCSS.editRectActive = true;


        })
    

    DEFINE_FUNCTION(0x001C, finalizeChange, RectangleInt changed )
        DEFINE_FUNCTION_WRAPPER(finalizeChange, {
            if(prm->changed.x < 0 || prm->changed.y < 0 || prm->changed.x + prm->changed.width > POOL_SIZE || prm->changed.y + prm->changed.height > POOL_SIZE){
                prm->code = FUN_WRONGARGS;
                return prm;
            }
        }, {})
        IMPL_FUNCTION(finalizeChange, {
            RectangleInt changed = prm->changed;


            //Copy the RGBA data ordered in POOLSIZExPOOLSIZE to the edit rect to be ready for upload
            for(int yy = 0; yy < changed.height; yy++){
                size_t rowStartLocal = LOCAL2D_TO_LOCAL1D(0, yy, changed.width);
                size_t rowStartPool = POOL2D_TO_POOL1D(changed.x, changed.y + yy);
                size_t rowStartRGBA = POOL1D_TO_COLOR1D(rowStartPool);
                size_t rowLength = changed.width;

                memcpy(TCSS.editRectRGBA + (rowStartLocal * 4), TCSS.rgbaData + rowStartRGBA, rowLength * 4);
            }

            //Here id upload to gpu, but not yet lol

        })

    DEFINE_FUNCTION(0x001B, endRect, )
        DEFINE_FUNCTION_WRAPPER(endRect, {
            if(!TCSS.editRectActive){
                prm->code = FUN_ERROR;
                return prm;
            }
        }, {})
        IMPL_FUNCTION(endRect, {
            TCSS.editRectActive = false;

            CF(CentralPixelPool, CentralPixelPool_finalizeChange, .changed = TCSS.editRect );
        })


    DEFINE_FUNCTION(0x001D, notifyChanges, RectangleInt changed )
        DEFINE_FUNCTION_WRAPPER(notifyChanges, {
            if(prm->changed.x < 0 || prm->changed.y < 0 || prm->changed.x + prm->changed.width > POOL_SIZE || prm->changed.y + prm->changed.height > POOL_SIZE){
                prm->code = FUN_WRONGARGS;
                return prm;
            }
        }, {})
        IMPL_FUNCTION(notifyChanges, {
            RectangleInt changed = prm->changed;

            if(TCSS.editRectActive)
            {
                // if we have an edit rect active, resize it so it fits around our changes.
                TCSS.editRect.width = MAX(TCSS.editRect.width, changed.x + changed.width - TCSS.editRect.x);
                TCSS.editRect.height = MAX(TCSS.editRect.height, changed.y + changed.height - TCSS.editRect.y);
                TCSS.editRect.x = MIN(TCSS.editRect.x, changed.x);
                TCSS.editRect.y = MIN(TCSS.editRect.y, changed.y);
            }
            else 
            {
                // if we dont, update immdeatly.
                CF(CentralPixelPool, CentralPixelPool_finalizeChange, .changed = changed );
            }
        })

    

    IMPL_INITTER({
        TraceLog(LOG_INFO, "Initializing CentralPixelPool");

        TCSS.cells = malloc(sizeof(Cell) * POOL_AREA);
        if(TCSS.cells == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for cells"); prm->code = FUN_ERROR; return; }
        memset(TCSS.cells, 0, sizeof(Cell) * POOL_AREA);

        TCSS.rgbaData = malloc(sizeof(char) * POOL_AREA * 4);
        if(TCSS.rgbaData == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for rgbaData"); prm->code = FUN_ERROR; return; }
        memset(TCSS.rgbaData, 0, sizeof(char) * POOL_AREA * 4);

        TCSS.editRectActive = false;
        
        TCSS.editRect = (RectangleInt){0, 0, 0, 0};

        TCSS.editRectRGBA = malloc(sizeof(char) * POOL_AREA * 4);
        if(TCSS.editRectRGBA == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for editRectRGBA"); prm->code = FUN_ERROR; return; }
        memset(TCSS.editRectRGBA, 0, sizeof(char) * POOL_AREA * 4);

        // Initialize free lists
        for(int i = 0; i <= MAXLEVEL; i++) {
            int blockSize = MINBLOCKSIZE << i;
            int blocksPerDim = POOL_SIZE / blockSize;
            int blockCount = blocksPerDim * blocksPerDim;

            TCSS.nodeStates[i] = malloc(sizeof(NodeState) * blockCount);
            if (!TCSS.nodeStates[i]) { TraceLog(LOG_ERROR, "Failed to allocate memory for nodeStates at level %d", i); prm->code = FUN_ERROR; return; }
            
            for(int j = 0; j < blockCount; j++){
                TCSS.nodeStates[i][j].state = NODESTATE_FREE;
                TCSS.nodeStates[i][j].largest_freeblock = i;
            }

        }

        TCSS.occupied = malloc(sizeof(bool) * POOL_AREA);
        if(TCSS.occupied == NULL) {  TraceLog(LOG_ERROR, "Failed to allocate memory for occupied"); prm->code = FUN_ERROR; return; }
        memset(TCSS.occupied, 0, sizeof(bool) * POOL_AREA);

        TraceLog(LOG_INFO, "Quadtree allocator initialized: MAXLEVEL=%d, MINBLOCKSIZE=%d", MAXLEVEL, MINBLOCKSIZE);
    })

    BEGIN_FUNFIND()

        FUNFIND_INITTER();

        FUNFIND_IMPL(rentHandle);
        FUNFIND_IMPL(evictHandle);

    END_FUNFIND()

END_CLASS()