//macro.h

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#define P1(a) a
#define P2(a, b) a##b
#define P3(a, b, c) a##b##c
#define P4(a, b, c, d) a##b##c##d
#define P5(a, b, c, d, e) a##b##c##d##e
#define P6(a, b, c, d, e, f) a##b##c##d##e##f
#define P7(a, b, c, d, e, f, g) a##b##c##d##e##f##g
#define P8(a, b, c, d, e, f, g, h) a##b##c##d##e##f##g##h

#define EP1(a) P1(a)
#define EP2(a, b) P2(a, b)
#define EP3(a, b, c) P3(a, b, c)
#define EP4(a, b, c, d) P4(a, b, c, d)
#define EP5(a, b, c, d, e) P5(a, b, c, d, e)
#define EP6(a, b, c, d, e, f) P6(a, b, c, d, e, f)
#define EP7(a, b, c, d, e, f, g) P7(a, b, c, d, e, f, g)
#define EP8(a, b, c, d, e, f, g, h) P8(a, b, c, d, e, f, g, h)

#define STR(x) #x
#define XSTR(x) STR(x)

#define FUN_OK ((char)0xFF)
#define FUN_NOTFOUND ((char)0x00)
#define FUN_ERROR ((char)0x02)
#define FUN_WRONGARGS ((char)0x01)
#define FUN_STACKOVER ((char)0x03)

//Naming convention macros

#define BEGIN_CLASS(cid, name) \
    enum { \
        EP2(CID_,TYPE) = cid \
    }; \
    static const char* EP3(C_,TYPE,_NAME) = #name



#define DEFINE_FUNCTION_WRAPPER(name, prelogic, postlogic) \
    static inline EP5(F_,TYPE,_,name,_PRM*) EP5(F_,TYPE,_,name,_EXECUTE)( ClassID cid , EP5(F_,TYPE,_,name,_PRM*) prm) \
    { \
      \
      prm->code = FUN_NOTFOUND; \
      FunCall fc = { \
        .fid = EP4(FID_,TYPE,_,name), \
        .prm = prm, \
      }; \
      prelogic \
      Class_Definition_CallFunction(cid, &fc); \
      postlogic \
      return prm; \
    }

#define DEFINE_FUNCTION_WRAPPER_SKELETON(name) \
    static inline EP5(F_,TYPE,_,name,_PRM*) EP5(F_,TYPE,_,name,_EXECUTE)( ClassID cid , EP5(F_,TYPE,_,name,_PRM*) prm);

#define DEFINEANDWRAP_FUNCTION(lfid, name, args, prelogic, postlogic) \
    DEFINE_FUNCTION(lfid, name, args) \
    DEFINE_FUNCTION_WRAPPER(name, prelogic, postlogic)

#define DEFINE_FUNCTION(lfid ,name, args) \
    enum { \
        EP4(FID_LOCAL_,TYPE,_,name) = lfid, \
        EP4(FID_,TYPE,_,name) = COMPOSE_FUNCTIONID(EP4(FID_LOCAL_,TYPE,_,name),EP2(CID_,TYPE)) \
    }; \
    const char* EP5(F_,TYPE,_,name,_NAME) = XSTR(name); \
    typedef struct EP5(F_,TYPE,_,name,_PRM) { \
        char code; \
        args \
    } EP5(F_,TYPE,_,name,_PRM); \
    DEFINE_FUNCTION_WRAPPER_SKELETON(name)

#define CALL_FUNCTION(classname,name, args) \
    EP3(F_,name,_EXECUTE)(EP2(CID_,classname),&((EP3(F_,name,_PRM)){ args }))


#define IMPL_FUNCTION(name, code) \
    IMPLOTHER_FUNCTION(EP3(TYPE,_,name),code)

#define IMPLOTHER_FUNCTION(name, code) \
    static void EP4(F_EXEC_,TYPE,_,name)(EP3(F_,name,_PRM*) prm) \
    { \
    EP2(prm->c,ode) = FUN_OK; \
    code \
    }

#define BEGIN_FUNFIND() \
    static void* EP3(C_,TYPE,_FUNFIND)(FunctionID fid) \
    { \
        switch (fid) { \

#define FUNFIND_IMPL(name) \
        case EP4(FID_,TYPE,_,name): \
            return EP6(F_EXEC_,TYPE,_,TYPE,_,name)

#define FUNFIND_IMPLOTHER(name) \
        case EP2(FID_,name):\
            return EP4(F_EXEC_,TYPE,_,name);

#define END_FUNFIND() \
    default: \
        return NULL; \
    } \
}

#define END_CLASS() \
    static void EP3(C_,TYPE,_CALLFUNCTION)(FunCall* funcall) \
    {  \
        void* ptr = EP3(C_,TYPE,_FUNFIND)(funcall->fid); \
        if(ptr) \
        { \
            void (*funptr)(void*) = (void (*)(void* ))ptr ; \
            funptr(funcall->prm); \
        } \
    \
    } \
    static bool EP3(C_,TYPE,_HASFUNCTION)(FunctionID fid) \
    { \
        return EP3(C_,TYPE,_FUNFIND)(fid) != NULL; \
    } \
    static void EP3(C_,TYPE,_REGISTER)() \
    { \
      Class_System_RegisterDefinition( (ClassDef) { .id = EP2(CID_,TYPE), .name = EP3(C_,TYPE,_NAME), .hasFunction = EP3(C_,TYPE,_HASFUNCTION), .callFunction = EP3(C_,TYPE,_CALLFUNCTION) });  \
    }

#define FUNPRM(name) EP3(F_,name,_PRM)*

#define TOKEN_BEGIN_SIZE ((char)'(')
#define TOKEN_END_SIZE ((char)')')

#define TOKEN_BEGIN_TYPE ((char)'[')
#define TOKEN_END_TYPE ((char)']')

#define TOKEN_BEGIN_DATA ((char)'{')
#define TOKEN_END_DATA ((char)'}')

#define TOKEN_BEGIN_VARIABLE ((char)'<')
#define TOKEN_END_VARIABLE ((char)'>')

#define TOKEN_NULL ((char)'N')