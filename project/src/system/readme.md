# Class System Documentation (UNFINISHED)

> OUTDATED! NEEDS UPDATE!

This system provides a macro-based framework for defining, managing, and interacting with classes and their instances in C. It allows for class registration, function wrapping, instance management, and reference counting. The system is heavily macro-driven to simplify repetitive boilerplate code.

---

## Core

0. [Aliases](#aliases)
1. [ID Management](#id-management)
2. [Class Definitions](#class-definitions)
3. [Function Definitions](#function-definitions)
4. [Function Dispatch](#function-dispatch)
5. [Class Instances](#class-instances)
6. [Instance Functions](#instance-functions)
7. [_DEF Functions](#def-functions)
8. [Memory Streams](#memory-streams)
9. [File Management](#file-management)
10. [Utilities](#utilities)

---

## Aliases

* `ClassID`: `cid`
* `LocalFunctionID`: `lfid` / `localfid`
* `FunctionID`: `fid`

---

## ID Management

The system uses a hierarchical ID system to uniquely identify classes and functions. This is managed by the `IDs.h` header.

### Types

* `ClassID`: A `uint16_t` representing a unique class identifier.
* `LocalFunctionID`: A `uint16_t` representing a unique function within a specific class.
* `FunctionID`: A `uint32_t` that combines a `ClassID` and a `LocalFunctionID` to create a globally unique function identifier.

#### Utilities

- `COMPOSE_FUNCTIONID(localfid, cid)`: Combines a local function ID and class ID into a global function ID.
- `GET_CID(fid)`: Extracts the class ID from a function ID.
- `GET_LOCALFID(fid)`: Extracts the local function ID from a function ID.

---

## Class Definitions

Classes are described by the `ClassDef` structure in `ClassDef.h`:

```c
typedef struct ClassDef {
    ClassID id;
    const char* name;
    bool (*hasFunction)(FunctionID);
    void (*callFunction)(FunCall*);
} ClassDef;
```

* `id`: Unique identifier for the class.
* `name`: Name of the class.
* `hasFunction`: Function pointer to check if a function exists for this class.
* `callFunction`: Function pointer to dispatch a function call for this class.

### Registration

Class registration (along with everything else) is very complex manually, so macros are there to help you with doing so. the following is an example class definition!

```c

//First, we must define the class name. this must be done at the start of every class header.

//The name of the class should be in uppercase.
#undef TYPE
#define TYPE TESTTYPE


//Next, we define the class data, first we detail a ClassID, this MUST be unique, or registration will fail.

//The second argument should be a literal name of the class, this will be used for debugging purposes.

//This creates two important constants, one called CID_{TYPE} (in our case CID_TESTTYPE) and one called C_{TYPE}_NAME (in our case C_TESTTYPE_NAME)
BEGIN_CLASS(0x001b,"TestType");


// Begin the function finding process. this will create C_{TYPE}_FUNFIND (in our case C_TESTTYPE_FUNFIND), a function which can be used to find the function pointer of any function in this class.
BEGIN_FUNFIND()

END_FUNFIND()

//End the class definition, This will define everything needed for your class, and importantly it will create the function: C_{TYPE}_REGISTER (in our case C_TESTTYPE_REGISTER) which can be used to register the class with the system.
END_CLASS()
```

---

## Function Definitions

Functions are called using the `FunCall` structure, which is defined in `Functions.h`:

```c
//PrmStructAddress == void*
//This is the address of the parameter struct, which is passed to the function.
//The parameter struct is defined in the function definition, which you will see soon

typedef struct FunCall {
    FunctionID fid;
    PrmStructAddress prm;
} FunCall;
```

To making a function work, there are two important seperate parts, defining and implementing.

### Defining a Function

First, a function must be defined, definitions must be made inside a class, after `BEGIN_CLASS` but before `BEGIN_FUNFIND`.

Defining a function requires defining its paramaters, name, and will create the function: `CID`-`LFID`

```c

// ... BEGIN_CLASS ...

//Define a few test functions!

DEFINE_FUNCTION(0x0002, SAYHI, const char* name;)
DEFINE_FUNCTION(0x0005, EXPLODE, int power; );

// ... BEGIN_FUNFIND ...

```

Woohoo! Our functions are now defined, this has created `FID_LOCAL_{NAME}` and `FID_{NAME}` which can be used to access the ID! (although you will rarely need it.)

Next, we need to implement the wrappers.
**after** all functions have been defined then we can define the wrappers.
This is code that always executes even if the implementation is handled by another class. You can use this to verify that the correct arguments have been passed, and some pre/post handling!

Lets change our previous code to use this!

```c
// ... BEGIN_CLASS ...

//Define a few test functions!

DEFINE_FUNCTION(0x0002, SAYHI, const char* name;)
DEFINE_FUNCTION(0x0005, EXPLODE, int power; );

//In the first bracket is pre-handling, this is code that is executed before the function is called.
//In the second bracket is post-handling, this is code that is executed after the function is called.
DEFINE_FUNCTION_WRAPPER(SAYHI, {
    //The parameters are passed as `prm`. importantly all functions contain the paramater "code" which can be used to specify if an error had occured. paramaters will be the same name as you gave them in function definition.
    //When returning early, make sure to type "return prm".
    if(prm->name == NULL){
        prm->code = FUN_WRONGARGS;
        return prm;
    }

} , {
    //Post handling code goes here, nothing in our case.
})

DEFINE_FUNCTION_WRAPPER(EXPLODE, {

    if(prm->power < 0){
        prm->code = FUN_WRONGARGS;
        return prm;
    }

} , {
    //Post handling code goes here, nothing in our case.
})


// ... BEGIN_FUNFIND ...

```

But right now, while the functions are defined our class does not yet support them, so lets cover the next target...

### Implementing a Function

Implementing functions is done in two different ways, one is implementing a function that we defined, and a function defined externally, they are very similar but with *slightly* different syntax for technical reasons.

for each implementation, we will need to add a line both after all the definitions and wrappers, and another one in between the funfind.

functions also define some basic error codes, which are:

* `FUN_OK` - No error occured.
* `FUN_ERROR` - An error occured.
* `FUN_NOTFOUND` - The function was not found.
* `FUN_WRONGARGS` - The function was called with the wrong arguments.

#### Implementing a Local Function

```c
// ...  ...

//when we call sayhi, we will print: "My name is: %s"!
IMPL_FUNCTION(SAYHI, {
    LogInfo(TextFormat("My name is: %s",prm->name));
})

//when we call explode, we will print a message depending on the value!
IMPL_FUNCTION(EXPLODE, {
    if(prm->power < 5){
        LogInfo(TextFormat("Thats it? I am not going to explode!"));
    }else if(prm->power < 10){
        LogInfo(TextFormat("Owe, that kinda hurt, but I am not going to explode!"));
    }
    else{
        LogInfo(TextFormat("I am going to explode!"));
    }
})

BEGIN_FUNFIND()

    FUNFIND_IMPL(SAYHI);
    FUNFIND_IMPL(EXPLODE);

END_FUNFIND()

// ... END_FUNFIND ...

// ... END_CLASS ...

```

#### Implementing an External Function

Lets say we create a seperate class, and we wanted it to support a SAYHI and EXPLODE functions, we would do the following!
for example, AngryTestType!

Notice that unlike local functions, we have to type the full name.

```c
// ...  ...

//when we call sayhi, we will print our name AGGRESIVELY!
IMPLOTHER_FUNCTION(TESTTYPE_SAYHI, {
    LogInfo(TextFormat("MY NAME IS %!!!!!!!!!!!",prm->name));
})

//when we call explode, we will print a message depending on the value!
IMPLOTHER_FUNCTION(TESTTYPE_EXPLODE, {
    if(prm->power < 5){
        LogInfo(TextFormat("THATS IT??? THATS ALL YOU HAVE???"));
    }else if(prm->power < 10){
        LogInfo(TextFormat("THATS NOT ENOUGH!!!! NOT ENOUGHHH!!!!!"));
    }
    else{
        LogInfo(TextFormat("YESSSSSSSSSS!! I AM GOING TO EXPLODE!!!!"));
    }
})

BEGIN_FUNFIND()

    FUNFIND_IMPLOTHER(TESTTYPE_SAYHI);
    FUNFIND_IMPLOTHER(TESTTYPE_EXPLODE);

END_FUNFIND()

// ... END_FUNFIND ...

// ... END_CLASS ...

```

---

## Function Dispatch

Function dispatch works dynamically, and since all our functions are currently static,  we will call a function based on the function name and the class name (and arguments, obviously)

This can be done like this:

```c

// first argument is the class name
// second argument is the function name
// third argument is paramater struct!
CALL_FUNCTION(TESTTYPE, TESTTYPE_SAYHI, .name = "OoooOooOoOoO";)

// we can also call it like this:
CALL_FUNCTION(TESTTYPE, TESTTYPE_EXPLODE, .power = 10;)

```

In the arguments section, we can pass any arguments the struct accepts, kind of like how we can initialize a struct with `{ .var1 = 232; var2 = 322; }`. but no need for the `{ }`.

`CALL_FUNCTION` also returns back the paramater struct, so we can check the `code` variable to see if an error had occured or generally access!

```c

FUNPRM(TESTTYPE_SAYHI) prm = CALL_FUNCTION(TESTTYPE, TESTTYPE_SAYHI, .name = "Weee" );
if(prm->code != FUN_OK){
    LogError(TextFormat("Error occured!"));
}

```

However do remember prm from CALL_FUNCTION is local to the current scope, so if you want to access it later, you will have to copy it.
