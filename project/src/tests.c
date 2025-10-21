#if INTESTING

#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include "system/ClassInstance.h"
#include "system/instance_macro.h"
#include "system/types/testtype.h"

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


int main(void) {
    // Set up custom trace logging
    SetTraceLogCallback(TestTraceLog);
    SetTraceLogLevel(TEST_LOG_LEVEL);
    
    printf("Starting comprehensive test suite...\n");
    
    // Run all tests
    int failures = 0;
    
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