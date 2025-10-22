#include "system/macro.h"
#ifndef INTESTING

#include <raylib.h>
#include <stdio.h>
#include "system/ClassInstance.h"
#include "system/instance_macro.h"
#include "system/types/testtype.h"
#include "system/types/CentralPixelPool.h"
#include "utils/MemoryStream.h"

int main(void) {

    C_TestType_REGISTER();

    getchar();

    

    return 0;
}

#endif