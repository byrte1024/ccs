//MemoryStream.h

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <stdio.h>

typedef uint8_t save_method;

#define SaveMethod_RAW 0

typedef struct MemoryStream {
    uint8_t* data;
    size_t capacity;

    size_t cursor;

} MemoryStream;

typedef struct MemoryStreamHeader {
    size_t datasize; //how big the data is (does not include header)
    save_method method;
} MemoryStreamHeader;

//Generic functions


bool MemoryStream_Allocate(MemoryStream* self, size_t capacity); //Can be used for both initial allocation, and reallocation
bool MemoryStream_Free(MemoryStream* self); //Frees the memory of the memory stream
bool MemoryStream_EnsureCapacity(MemoryStream* self, size_t capacity); //Ensures that the memory stream has enough capacity.
bool MemoryStream_Clear(MemoryStream* self); //Clears the memory stream, but does not free the memory.
void MemoryStream_Log(MemoryStream* self, TraceLogLevel level); //Logs the memory stream up until Cursor. Each byte Logs its Char value
void MemoryStream_LogAsHex(MemoryStream* self, TraceLogLevel level); //Logs the memory stream up until Cursor. Each byte Logs its Hex value

bool MemoryStream_Save(MemoryStream* self, FILE* out, save_method method); //Saves the memory stream up until the cursor.
bool MemoryStream_SaveAll(MemoryStream* self, FILE* out, save_method method); //Saves the entire allocated memory stream.
bool MemoryStream_Load(MemoryStream* self, FILE* in);

//Cursor related functions:
bool MemoryStream_Seek_Set(MemoryStream* self, size_t cursor); //Manually sets the cursor position. will call ensure capacity.
bool MemoryStream_Seek_Cur(MemoryStream* self, int offset); //Offsets the cursor position. will call ensure capacity.

//Dynamic data functions: (will modify the cursor)
bool MemoryStream_WriteChar(MemoryStream* self, uint8_t character, uint8_t** ptr_to_data_ptr);
bool MemoryStream_WriteBytes(MemoryStream* self, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr);
bool MemoryStream_WriteCstr(MemoryStream* self, const char* string, size_t buff_len, uint8_t** ptr_to_data_ptr);
bool MemoryStream_WriteHex(MemoryStream* self, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr);
bool MemoryStream_SetBytes(MemoryStream* self, uint8_t value, size_t count, uint8_t** ptr_to_data_ptr);
bool MemoryStream_ReadChar(MemoryStream* self, uint8_t** ptr_to_data_ptr);
bool MemoryStream_ReadBytes(MemoryStream* self, uint8_t** ptr_to_data_ptr, size_t count);

//Static data functions: (will not use the cursor)
bool MemoryStream_Static_WriteChar(MemoryStream* self, size_t cursor, uint8_t character, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_WriteBytes(MemoryStream* self, size_t cursor, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_WriteCstr(MemoryStream* self, size_t cursor, const char* string, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_WriteHex(MemoryStream* self, size_t cursor, uint8_t character, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_SetBytes(MemoryStream* self, size_t cursor, uint8_t value, size_t count, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_ReadChar(MemoryStream* self, size_t cursor, uint8_t** ptr_to_data_ptr);
bool MemoryStream_Static_ReadBytes(MemoryStream* self, size_t cursor, uint8_t** ptr_to_data_ptr, size_t count);

#define CSTRCOM(string) string , strlen(string)

static inline bool MemoryStream_WriteVFormat(MemoryStream* self, const char* format, va_list args) {
    char toadd[256];
    int len = vsnprintf(toadd, sizeof(toadd), format, args);
    if (len < 0) return false;

    if ((size_t)len >= sizeof(toadd)) len = sizeof(toadd) - 1;
    return MemoryStream_WriteCstr(self, toadd, len, NULL);
}


static inline bool MemoryStream_WriteFormat(MemoryStream* self, const char* format, ...) {
    va_list args;
    va_start(args, format);
    bool ret = MemoryStream_WriteVFormat(self, format, args);
    va_end(args);
    return ret;
}

