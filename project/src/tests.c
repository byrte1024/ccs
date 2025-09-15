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

// Test functions
int test_macro_expansions() {
    TEST_START("Macro Expansions");
    
    // Test that macros expand correctly
    TEST_ASSERT(CID_TESTTYPE == 0x001b, "CID_TESTTYPE should be 0x001b");
    TEST_ASSERT(FID_TESTTYPE_SAYHI == COMPOSE_FUNCTIONID(FID_LOCAL_TESTTYPE_SAYHI, CID_TESTTYPE), 
                "FID_TESTTYPE_SAYHI should be composed correctly");
    TEST_ASSERT(FID_LOCAL_TESTTYPE_SAYHI == 0x0002, 
                "FID_LOCAL_TESTTYPE_SAYHI should be 0x0002");
    
    // Test that types are defined correctly
    TEST_ASSERT(sizeof(F_TESTTYPE_SAYHI_PRM) > 0, "F_TESTTYPE_SAYHI_PRM should be defined");
    TEST_ASSERT(sizeof(F_DEF_CREATE_PRM) > 0, "F_DEF_CREATE_PRM should be defined");
    
    printf("Macro expansion test completed successfully\n");
    TEST_END();
    return 0;
}

int test_class_registration() {
    TEST_START("Class Registration");
    
    // Test that classes can be registered
    C_TESTTYPE_REGISTER();
    
    // Check that the class was registered
    TEST_ASSERT(Class_System_HasDefinition(CID_TESTTYPE), 
                "TESTTYPE class should be registered");
    
    // Check that the class definition is correct
    const ClassDef* def = Class_System_GetDefinition(CID_TESTTYPE);
    TEST_ASSERT(def != NULL, "Class definition should not be NULL");
    TEST_ASSERT(def->id == CID_TESTTYPE, "Class ID should match");
    TEST_ASSERT(def->hasFunction != NULL, "hasFunction should not be NULL");
    TEST_ASSERT(def->callFunction != NULL, "callFunction should not be NULL");
    
    // Test function existence checking
    TEST_ASSERT(def->hasFunction(FID_TESTTYPE_SAYHI), 
                "SAYHI function should exist in TESTTYPE");
    TEST_ASSERT(def->hasFunction(FID_DEF_CREATE), 
                "CREATE function should exist in TESTTYPE");
    TEST_ASSERT(def->hasFunction(FID_DEF_DESTROY), 
                "DESTROY function should exist in TESTTYPE");
    
    // Test non-existent function
    TEST_ASSERT(!def->hasFunction(0xFFFF), 
                "Non-existent function should return false");
    
    TEST_END();
    return 0;
}

int test_instance_creation_and_destruction() {
    TEST_START("Instance Creation and Destruction");
    
    // Create an empty instance
    ClassReference r = Class_Reference_AllocateEmptyInstance();
    TEST_ASSERT(!Class_Reference_Status_IsNull(r), "Instance should not be null");
    TEST_ASSERT(Class_Reference_Status_IsAllocated(r), "Instance should be allocated");
    TEST_ASSERT(Class_Reference_Status_IsEmpty(r), "Instance should be empty");
    TEST_ASSERT(Class_Reference_Status_IsUntyped(r), "Instance should be untyped");
    TEST_ASSERT(Class_Reference_Status_IsDead(r), "Instance should be dead");
    TEST_ASSERT(!Class_Reference_Status_IsForgotten(r), "Instance should not be forgotten");
    
    // Set the class ID
    r.instance->cid = CID_TESTTYPE;
    TEST_ASSERT(Class_Reference_Status_IsTyped(r), "Instance should be typed");
    TEST_ASSERT(Class_Reference_Status_IsDead(r), "Instance should still be dead");
    
    // Call CREATE function
    F_DEF_CREATE_PRM* create_result = CALL_R_FUNCTION(r, DEF_CREATE, );
    TEST_ASSERT(create_result->code == FUN_OK, "CREATE should succeed");
    TEST_ASSERT(Class_Reference_Status_IsAlive(r), "Instance should be alive");
    TEST_ASSERT(r.instance->data != NULL, "Instance data should not be NULL");
    
    // Verify data initialization
    TestType* data = (TestType*)r.instance->data;
    TEST_ASSERT(data->x == 1, "x should be initialized to 1");
    TEST_ASSERT(data->y == 2, "y should be initialized to 2");
    TEST_ASSERT(data->z == 3, "z should be initialized to 3");
    TEST_ASSERT(data->w == 4, "w should be initialized to 4");
    
    // Modify data
    data->x = 10;
    data->y = 20;
    data->z = 30;
    data->w = 40;
    TEST_ASSERT(data->x == 10, "x should be modified to 10");
    TEST_ASSERT(data->y == 20, "y should be modified to 20");
    TEST_ASSERT(data->z == 30, "z should be modified to 30");
    TEST_ASSERT(data->w == 40, "w should be modified to 40");
    
    // Call DESTROY function
    F_DEF_DESTROY_PRM* destroy_result = CALL_R_FUNCTION(r, DEF_DESTROY, );
    TEST_ASSERT(destroy_result->code == FUN_OK, "DESTROY should succeed");
    TEST_ASSERT(Class_Reference_Status_IsDead(r), "Instance should be dead after destroy");
    TEST_ASSERT(r.instance->data == NULL, "Instance data should be NULL after destroy");
    
    // Forget the reference
    Class_Reference_Forget(r);
    
    TEST_END();
    return 0;
}

int test_function_calls() {
    TEST_START("Function Calls");
    
    F_TESTTYPE_SAYHI_PRM* sayhi_result = CALL_FUNCTION(TESTTYPE, TESTTYPE_SAYHI, .name = "TestName" );
    TEST_ASSERT(sayhi_result->code == FUN_OK, "SAYHI should succeed with valid name");
    
    TEST_END();
    return 0;
}

int test_reference_counting() {
    TEST_START("Reference Counting");
    
    // Create first reference
    ClassReference r1 = Class_Reference_AllocateEmptyInstance();
    TEST_ASSERT(r1.instance->ref_count == 1, "Reference count should be 1 after creation");
    
    // Create second reference
    ClassReference r2 = Class_Reference_CreateFrom(r1);
    TEST_ASSERT(r1.instance->ref_count == 2, "Reference count should be 2 after second reference");
    TEST_ASSERT(r1.instance == r2.instance, "Both references should point to the same instance");
    
    // Set class ID and create data
    r1.instance->cid = CID_TESTTYPE;
    CALL_R_FUNCTION(r1, DEF_CREATE, );
    
    // Create third reference
    ClassReference r3 = Class_Reference_CreateFrom(r1);
    TEST_ASSERT(r1.instance->ref_count == 3, "Reference count should be 3 after third reference");
    
    // Forget first reference
    Class_Reference_Forget(r1);
    TEST_ASSERT(r2.instance->ref_count == 2, "Reference count should be 2 after forgetting first reference");
    
    // Forget second reference
    Class_Reference_Forget(r2);
    TEST_ASSERT(r3.instance->ref_count == 1, "Reference count should be 1 after forgetting second reference");
    
    // Forget third reference (should trigger destruction)
    Class_Reference_Forget(r3);
    
    TEST_END();
    return 0;
}

int test_error_conditions() {
    TEST_START("Error Conditions");
    
    // Test calling function on NULL instance
    ClassReference null_ref = {NULL};
    F_DEF_CREATE_PRM* null_result = CALL_R_FUNCTION(null_ref, DEF_CREATE, );
    TEST_ASSERT(null_result->code == FUN_WRONGARGS, "CREATE should fail on NULL instance");
    
    // Test calling function on dead instance
    ClassReference dead_ref = Class_Reference_AllocateEmptyInstance();
    dead_ref.instance->cid = CID_TESTTYPE;
    // Don't call CREATE, so instance remains dead
    F_DEF_DESTROY_PRM* dead_result = CALL_R_FUNCTION(dead_ref, DEF_DESTROY, );
    TEST_ASSERT(dead_result->code == FUN_WRONGARGS, "DESTROY should fail on dead instance");
    Class_Reference_Forget(dead_ref);
    
    // Test calling non-existent function
    ClassReference valid_ref = Class_Reference_AllocateEmptyInstance();
    valid_ref.instance->cid = CID_TESTTYPE;
    CALL_R_FUNCTION(valid_ref, DEF_CREATE, );
    
    // Create a fake function call with invalid ID
    FunCall invalid_call = {0xFFFFFFFF, NULL};
    Class_Definition_CallFunction(CID_TESTTYPE, &invalid_call);
    // Should log an error but not crash
    
    CALL_R_FUNCTION(valid_ref, DEF_DESTROY, );
    Class_Reference_Forget(valid_ref);
    
    TEST_END();
    return 0;
}

int test_edge_cases() {
    TEST_START("Edge Cases");
    
    // Test creating multiple instances
    ClassReference refs[10];
    for (int i = 0; i < 10; i++) {
        refs[i] = Class_Reference_AllocateEmptyInstance();
        refs[i].instance->cid = CID_TESTTYPE;
        CALL_R_FUNCTION(refs[i], DEF_CREATE, );
        
        TestType* data = (TestType*)refs[i].instance->data;
        data->x = i;
        
        TEST_ASSERT(data->x == i, "Instance data should be set correctly");
    }
    
    // Verify all instances are independent
    for (int i = 0; i < 10; i++) {
        TestType* data = (TestType*)refs[i].instance->data;
        TEST_ASSERT(data->x == i, "Instance data should maintain independence");
    }
    
    // Clean up all instances
    for (int i = 0; i < 10; i++) {
        CALL_R_FUNCTION(refs[i], DEF_DESTROY, );
        Class_Reference_Forget(refs[i]);
    }
    
    // Test creating an instance with CID_DEF (should not have data)
    ClassReference def_ref = Class_Reference_AllocateEmptyInstance();
    def_ref.instance->cid = CID_DEF;
    TEST_ASSERT(Class_Reference_Status_IsUntyped(def_ref), "Instance with CID_DEF should be untyped");
    TEST_ASSERT(Class_Reference_Status_IsEmpty(def_ref), "Instance with CID_DEF should be empty");
    
    // Try to call CREATE on CID_DEF instance (should fail)
    F_DEF_CREATE_PRM* def_create_result = CALL_R_FUNCTION(def_ref, DEF_CREATE, );
    TEST_ASSERT(def_create_result->code != FUN_OK, "CREATE should fail on CID_DEF instance");
    
    Class_Reference_Forget(def_ref);
    
    TEST_END();
    return 0;
}

int test_memory_management() {
    TEST_START("Memory Management");
    
    // Test that memory is properly freed
    int initial_errors = test_state.error_count;
    
    // Create and destroy many instances to check for memory leaks
    for (int i = 0; i < 100; i++) {
        ClassReference r = Class_Reference_AllocateEmptyInstance();
        r.instance->cid = CID_TESTTYPE;
        CALL_R_FUNCTION(r, DEF_CREATE, );
        
        // Modify data
        TestType* data = (TestType*)r.instance->data;
        data->x = i * 10;
        
        CALL_R_FUNCTION(r, DEF_DESTROY, );
        Class_Reference_Forget(r);
    }
    
    // Check that no errors occurred during the mass creation/destruction
    TEST_ASSERT(test_state.error_count == initial_errors, 
                "No memory errors should occur during mass creation/destruction");
    
    TEST_END();
    return 0;
}

int main(void) {
    // Set up custom trace logging
    SetTraceLogCallback(TestTraceLog);
    SetTraceLogLevel(TEST_LOG_LEVEL);
    
    printf("Starting comprehensive test suite...\n");
    
    // Run all tests
    int failures = 0;
    
    failures += test_macro_expansions();
    failures += test_class_registration();
    failures += test_instance_creation_and_destruction();
    failures += test_function_calls();
    failures += test_reference_counting();
    failures += test_error_conditions();
    failures += test_edge_cases();
    failures += test_memory_management();
    
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