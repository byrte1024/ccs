//testtype.h

#pragma once

#include "../macro.h"
#include "../ClassInstance.h"

#undef TYPE
#define TYPE TESTTYPE

BEGIN_CLASS(0x001b,"Test Type");

    typedef struct TestType {
        int x;
        int y;
        int z;
        int w;
    } TestType;

    DEFINEANDWRAP_FUNCTION(0x0002, SAYHI, const char* name;, , )

    IMPL_FUNCTION(SAYHI, {
        TraceLog(LOG_INFO,TextFormat("My name is: %s",prm->name));
    })

    IMPLOTHER_FUNCTION(DEF_CREATE, {
        ClassInstance* i = prm->self;

        TestType* t = malloc(sizeof(TestType));
        if(t == NULL){
            prm->code = FUN_ERROR;
            return;
        }
        t->x = 1;
        t->y = 2;
        t->z = 3;
        t->w = 4;

        i->data = t;
    })
    IMPLOTHER_FUNCTION(DEF_DESTROY, {
        ClassInstance* i = prm->self;

        TestType* t = i->data;
        free(t);
    })


    BEGIN_FUNFIND()

        FUNFIND_IMPLOTHER(DEF_CREATE);
        FUNFIND_IMPLOTHER(DEF_DESTROY);

        FUNFIND_IMPL(SAYHI);

    END_FUNFIND()

END_CLASS()

