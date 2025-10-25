#if INTESTING

#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "system/ClassInstance.h"
#include "system/instance_macro.h"
#include "system/types/testtype.h"
#include "system/types/CentralPixelPool.h"

// Test configuration
#define TEST_LOG_LEVEL LOG_INFO
#define MAX_LOG_MESSAGES 1000
#define LOG_MESSAGE_LENGTH 256

// Test state tracking
typedef struct {
    char messages[MAX_LOG_MESSAGES][LOG_MESSAGE_LENGTH];
    int message_count;
    int error_count;
    int warning_count;
    int info_count;
    int highest_level;
} TestState;

static TestState test_state = {0};

// Custom trace log callback for testing
void TestTraceLog(int logLevel, const char *text, va_list args) {
    if (test_state.message_count < MAX_LOG_MESSAGES) {
        vsnprintf(test_state.messages[test_state.message_count], 
                 LOG_MESSAGE_LENGTH, text, args);
        test_state.message_count++;
    }
    
    if (logLevel > test_state.highest_level) {
        test_state.highest_level = logLevel;
    }
    
    switch (logLevel) {
        case LOG_ERROR: test_state.error_count++; break;
        case LOG_WARNING: test_state.warning_count++; break;
        case LOG_INFO: test_state.info_count++; break;
        default: break;
    }
    
    // Also output to console for debugging
    vprintf(text, args);
    printf("\n");
}

// Test utilities
#define TEST_START(name) \
    printf("\n=== TEST: %s ===\n", name); \
    test_state.message_count = 0; \
    test_state.error_count = 0; \
    test_state.warning_count = 0; \
    test_state.info_count = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s (%s:%d)\n", message, __FILE__, __LINE__); \
            return 1; \
        } else { \
            printf("PASS: %s\n", message); \
        } \
    } while (0)

#define TEST_END() \
    printf("=== TEST COMPLETE ===\n"); \
    if (test_state.error_count > 0) { \
        printf("WARNING: %d errors logged during test\n", test_state.error_count); \
    }

// Helper function to count occupied cells
int count_occupied_cells(void) {
    int count = 0;
    for (int i = 0; i < POOL_AREA; i++) {
        if (CSS(CentralPixelPool).occupied[i]) {
            count++;
        }
    }
    return count;
}

// Helper function to print pool state
void print_pool_state(const char* label) {
    return;
    printf("%s:\n", label);
    for (int y = 0; y < POOL_SIZE; y++) {
        for (int x = 0; x < POOL_SIZE; x++) {
            printf("%c", CSS(CentralPixelPool).occupied[POOL2D_TO_POOL1D(x, y)] ? 'X' : '-');
        }
        printf("\n");
    }
}

// Test 1: Basic initialization
int test_initialization(void) {
    TEST_START("Initialization");
    
    TEST_ASSERT(C_TestType_REGISTER(), "TestType registration");
    TEST_ASSERT(C_CentralPixelPool_REGISTER(), "CentralPixelPool registration");
    
    // Verify pool is empty
    int occupied = count_occupied_cells();
    TEST_ASSERT(occupied == 0, "Pool starts empty");
    
    TEST_END();
    return 0;
}

// Test 2: Single allocation
int test_single_allocation(void) {
    TEST_START("Single Allocation");
    
    CPP_Handle handle = {0};
    
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 32, .height = 32, .out_handle = &handle);
    
    TEST_ASSERT(handle.valid, "Handle is valid after allocation");
    TEST_ASSERT(handle.rectWidth == 32, "Width is correct");
    TEST_ASSERT(handle.rectHeight == 32, "Height is correct");
    TEST_ASSERT(handle.rectX >= 0 && handle.rectX < POOL_SIZE, "X position valid");
    TEST_ASSERT(handle.rectY >= 0 && handle.rectY < POOL_SIZE, "Y position valid");
    
    // Verify cells are marked as occupied
    int occupied = count_occupied_cells();
    TEST_ASSERT(occupied == 32 * 32, "Correct number of cells occupied");
    
    // Evict and verify cleanup
    CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);
    TEST_ASSERT(!handle.valid, "Handle invalid after eviction");
    
    print_pool_state("After eviction");
    
    occupied = count_occupied_cells();
    TEST_ASSERT(occupied == 0, "Pool empty after eviction");
    
    TEST_END();
    return 0;
}

// Test 3: Multiple allocations
int test_multiple_allocations(void) {
    TEST_START("Multiple Allocations");
    
    CPP_Handle handles[4] = {0};
    
    // Allocate multiple different sizes
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 32, .height = 32, .out_handle = &handles[0]);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 16, .height = 16, .out_handle = &handles[1]);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 8, .height = 8, .out_handle = &handles[2]);
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 4, .height = 4, .out_handle = &handles[3]);
    
    // Verify all valid
    for (int i = 0; i < 4; i++) {
        TEST_ASSERT(handles[i].valid, "Handle valid");
    }
    
    // Verify no overlaps
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            bool overlap = !(handles[i].rectX + handles[i].rectWidth <= handles[j].rectX ||
                           handles[j].rectX + handles[j].rectWidth <= handles[i].rectX ||
                           handles[i].rectY + handles[i].rectHeight <= handles[j].rectY ||
                           handles[j].rectY + handles[j].rectHeight <= handles[i].rectY);
            TEST_ASSERT(!overlap, "Handles don't overlap");
        }
    }
    
    print_pool_state("After multiple allocations");
    
    // Clean up
    for (int i = 0; i < 4; i++) {
        CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handles[i]);
    }
    
    TEST_END();
    return 0;
}

// Test 4: Power-of-2 rounding
int test_power_of_2_rounding(void) {
    TEST_START("Power-of-2 Rounding");
    
    CPP_Handle handle = {0};
    
    // Request non-power-of-2 size
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 10, .height = 10, .out_handle = &handle);
    
    TEST_ASSERT(handle.valid, "Handle valid for non-power-of-2 request");
    TEST_ASSERT(handle.rectWidth == 16, "Width rounded up to 16");
    TEST_ASSERT(handle.rectHeight == 16, "Height rounded up to 16");
    
    CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);
    
    // Test asymmetric sizes
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 5, .height = 12, .out_handle = &handle);
    
    TEST_ASSERT(handle.rectWidth == 16, "Both dimensions use larger power-of-2");
    TEST_ASSERT(handle.rectHeight == 16, "Both dimensions use larger power-of-2");
    
    CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);
    
    TEST_END();
    return 0;
}

// Test 5: Allocation and deallocation cycle
int test_alloc_dealloc_cycle(void) {
    TEST_START("Allocation/Deallocation Cycle");
    
    CPP_Handle handle = {0};
    
    // Allocate and deallocate multiple times
    for (int i = 0; i < 10; i++) {
        CF(CentralPixelPool, CentralPixelPool_rentHandle, 
           .width = 16, .height = 16, .out_handle = &handle);
        TEST_ASSERT(handle.valid, "Allocation successful in cycle");
        
        CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);
        TEST_ASSERT(!handle.valid, "Deallocation successful in cycle");
    }

    print_pool_state("After constant deallocation");
    
    int occupied = count_occupied_cells();
    TEST_ASSERT(occupied == 0, "Pool empty after cycles");
    
    TEST_END();
    return 0;
}

// Test 6: Fragmentation handling
int test_fragmentation(void) {
    TEST_START("Fragmentation Handling");
    
    CPP_Handle handles[8] = {0};
    
    // Allocate many small blocks
    for (int i = 0; i < 8; i++) {
        CF(CentralPixelPool, CentralPixelPool_rentHandle, 
           .width = 16, .height = 16, .out_handle = &handles[i]);
        TEST_ASSERT(handles[i].valid, "Small block allocated");
    }
    
    
    // Free every other one
    for (int i = 0; i < 8; i += 2) {
        CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handles[i]);
    }
    
    print_pool_state("After freeing every other block");
    
    // Try to allocate in freed spaces
    for (int i = 0; i < 8; i += 2) {
        CF(CentralPixelPool, CentralPixelPool_rentHandle, 
           .width = 16, .height = 16, .out_handle = &handles[i]);
        TEST_ASSERT(handles[i].valid, "Reallocated in freed space");
    }
    
    // Clean up
    for (int i = 0; i < 8; i++) {
        CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handles[i]);
    }
    
    TEST_END();
    return 0;
}

// Test 7: Out of memory handling
int test_out_of_memory(void) {
    TEST_START("Out of Memory Handling");

    
    CPP_Handle handles[POOL_AREA / 8] = {0};
    int allocated_count = 0;
    
    // Try to allocate until we run out
    for (int i = 0; i < (POOL_AREA / 8); i++) {
        CF(CentralPixelPool, CentralPixelPool_rentHandle, 
           .width = 8, .height = 8, .out_handle = &handles[i]);
        
        if (handles[i].valid) {
            allocated_count++;
        } else {
            break;
        }
    }
    
    printf("Allocated %d blocks before running out\n", allocated_count);
    TEST_ASSERT(allocated_count > 0, "Some allocations succeeded");
    TEST_ASSERT(allocated_count < POOL_AREA / 8, "Eventually ran out of space");
    
    // Clean up
    for (int i = 0; i < allocated_count; i++) {
        CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handles[i]);
    }
    
    TEST_END();
    return 0;
}

// Test 8: Invalid parameters
int test_invalid_parameters(void) {
    TEST_START("Invalid Parameters");
    
    CPP_Handle handle = {0};
    
    // Test zero size
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 0, .height = 0, .out_handle = &handle);
    TEST_ASSERT(!handle.valid, "Zero size rejected");
    
    // Test negative size
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = -1, .height = -1, .out_handle = &handle);
    TEST_ASSERT(!handle.valid, "Negative size rejected");
    
    // Test oversized
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = POOL_SIZE + 1, .height = POOL_SIZE + 1, .out_handle = &handle);
    TEST_ASSERT(!handle.valid, "Oversized request rejected");
    
    // Test null handle
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 16, .height = 16, .out_handle = NULL);
    // Should fail gracefully
    
    TEST_END();
    return 0;
}

// Test 9: Minimum block size
int test_minimum_block_size(void) {
    TEST_START("Minimum Block Size");
    
    CPP_Handle handle = {0};
    
    // Request 1x1, should get MINBLOCKSIZE
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = 1, .height = 1, .out_handle = &handle);
    
    TEST_ASSERT(handle.valid, "Minimum size allocated");
    TEST_ASSERT(handle.rectWidth == MINBLOCKSIZE, "Width is MINBLOCKSIZE");
    TEST_ASSERT(handle.rectHeight == MINBLOCKSIZE, "Height is MINBLOCKSIZE");
    
    CF(CentralPixelPool, CentralPixelPool_evictHandle, .handle = &handle);
    
    TEST_END();
    return 0;
}

// Test 10: Maximum block size
int test_maximum_block_size(void) {
    TEST_START("Maximum Block Size");
    
    CPP_Handle handle = {0};
    
    // Request full pool
    CF(CentralPixelPool, CentralPixelPool_rentHandle, 
       .width = POOL_SIZE, .height = POOL_SIZE, .out_handle = &handle);
    
    TEST_ASSERT(handle.valid, "Maximum size should pass");
    
    int occupied = count_occupied_cells();
    
    TEST_END();
    return 0;
}

int main(void) {
    // Set up custom trace logging
    SetTraceLogCallback(TestTraceLog);
    SetTraceLogLevel(TEST_LOG_LEVEL);
    
    printf("Starting comprehensive CentralPixelPool test suite...\n");
    
    // Run all tests
    int failures = 0;
    
    failures += test_initialization();
    failures += test_single_allocation();
    failures += test_multiple_allocations();
    failures += test_power_of_2_rounding();
    failures += test_alloc_dealloc_cycle();
    failures += test_fragmentation();
    failures += test_out_of_memory();
    failures += test_invalid_parameters();
    failures += test_minimum_block_size();
    failures += test_maximum_block_size();
    
    // Print summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Tests completed with %d failures\n", failures);
    printf("Log statistics: %d errors, %d warnings, %d info messages\n",
           test_state.error_count, test_state.warning_count, test_state.info_count);
    
    if (failures == 0) {
        printf("ALL TESTS PASSED!\n");
    } else {
        printf("SOME TESTS FAILED!\n");
    }
    
    return failures;
}

#endif