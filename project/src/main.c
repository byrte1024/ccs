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


    
    getchar();

    return 0;
}

#endif