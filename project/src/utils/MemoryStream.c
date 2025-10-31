
#include "MemoryStream.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "BinaryUtils.h"

#define MS_BUFFER_SIZE 2048
#define MS_TRUNCATE_POSITION 2000

bool MemoryStream_Allocate(MemoryStream* self, size_t capacity)
{
    if (self == NULL) return false;
    if (capacity == 0) return false;

    capacity = next_power_of_2(capacity);

    if (capacity <= self->capacity) return true;

    if (self->data == NULL)
    {
        self->data = malloc(capacity);
        if (self->data == NULL) return false;
        memset(self->data, 0, capacity);
    }
    else
    {
        uint8_t* new_data = realloc(self->data, capacity);
        if (new_data == NULL) return false;
        memset(new_data + self->capacity, 0, capacity - self->capacity);
        self->data = new_data;
    }

    self->capacity = capacity;
    return true;
}
bool MemoryStream_Free(MemoryStream* self)
{
    if (self == NULL || self->data == NULL) return false;

    free(self->data);
    self->data = NULL;
    self->capacity = 0;
    self->cursor = 0;

    return true;
}
bool MemoryStream_EnsureCapacity(MemoryStream* self, size_t capacity)
{
    if (capacity <= self->capacity) return true;
    return MemoryStream_Allocate(self, capacity);
}
bool MemoryStream_Clear(MemoryStream* self)
{
    if (self == NULL || self->data == NULL) return false;

    memset(self->data, 0, self->capacity);
    self->cursor = 0;

    return true;
}
void MemoryStream_Log(MemoryStream* self, TraceLogLevel level)
{
    if (self == NULL || self->data == NULL)
        return;

    bool truncated = false;
    size_t limit = self->cursor;
    if (limit > self->capacity) limit = self->capacity;
    if (limit > MS_TRUNCATE_POSITION) { limit = MS_TRUNCATE_POSITION; truncated = true; }

    if (limit == 0) {
        TraceLog(level, "Empty MemoryStream");
        return;
    }

    // Use dynamic buffer to avoid overflow
    size_t buffer_size = limit * 2 + 32;
    char* buffer = malloc(buffer_size);
    if (!buffer) return;
    size_t pos = snprintf(buffer, buffer_size, "");

    for (size_t i = 0; i < limit; i++)
    {
        char c = (char)self->data[i];
        if (!isprint(c)) {
            pos += snprintf(buffer + pos, buffer_size - pos, "?");
        }
        else {
            pos += snprintf(buffer + pos, buffer_size - pos, "%c", c);
        }
    }

    if (truncated) {
        snprintf(buffer + pos, buffer_size - pos, "...");
    }

    TraceLog(level, buffer);
    free(buffer);
}
void MemoryStream_LogAsHex(MemoryStream* self, TraceLogLevel level)
{
    if (self == NULL || self->data == NULL)
        return;

    bool truncated = false;
    size_t limit = self->cursor;
    if (limit > self->capacity) limit = self->capacity;
    if (limit > MS_TRUNCATE_POSITION) { limit = MS_TRUNCATE_POSITION; truncated = true; }

    if (limit == 0) {
        TraceLog(level, "Empty MemoryStream");
        return;
    }

    size_t buffer_size = limit * 3 + 32;
    char* buffer = malloc(buffer_size);
    if (!buffer) return;
    size_t pos = snprintf(buffer, buffer_size, "\n");

    for (size_t i = 0; i < limit; i++)
    {
        pos += snprintf(buffer + pos, buffer_size - pos, "%02x ", self->data[i]);
        if (i % 8 == 7) pos += snprintf(buffer + pos, buffer_size - pos, "\n");
    }

    if (truncated) snprintf(buffer + pos, buffer_size - pos, "...");

    TraceLog(level, buffer);
    free(buffer);
}

bool MemoryStream_Save(MemoryStream* self, FILE* out, save_method method)
{
    if (!self || !out) return false;

    MemoryStreamHeader header;
    header.datasize = self->cursor; // Only save data up to the cursor
    header.method = method;

    // Write header first
    if (fwrite(&header, sizeof(MemoryStreamHeader), 1, out) != 1)
        return false;

    if (method == SaveMethod_RAW)
    {
        if (fwrite(self->data, 1, header.datasize, out) != header.datasize)
            return false;
    }

    return true;
}
bool MemoryStream_SaveAll(MemoryStream* self, FILE* out, save_method method)
{
    if (!self || !out) return false;

    MemoryStreamHeader header;
    header.datasize = self->capacity; // Save the full allocated memory
    header.method = method;

    // Write header
    if (fwrite(&header, sizeof(MemoryStreamHeader), 1, out) != 1)
        return false;

    if (method == SaveMethod_RAW)
    {
        if (fwrite(self->data, 1, header.datasize, out) != header.datasize)
            return false;
    }

    return true;
}
bool MemoryStream_Load(MemoryStream* self, FILE* in)
{
    if (!self || !in) return false;

    MemoryStreamHeader header;

    // Read header first
    if (fread(&header, sizeof(MemoryStreamHeader), 1, in) != 1)
        return false;

    if (header.datasize == 0) return false;

    // Ensure the memory stream can hold the data
    if (!MemoryStream_EnsureCapacity(self, header.datasize))
        return false;

    if (header.method == SaveMethod_RAW)
    {
        if (fread(self->data, 1, header.datasize, in) != header.datasize)
            return false;
    }

    self->cursor = header.datasize; // Update cursor to end of loaded data

    return true;
}

// Cursor functions
bool MemoryStream_Seek_Set(MemoryStream* self, size_t cursor)
{
    if (self == NULL) return false;
    if (cursor > self->capacity) return false;
    self->cursor = cursor;
    return true;
}
bool MemoryStream_Seek_Cur(MemoryStream* self, int offset)
{
    if (self == NULL) return false;
    if ((offset < 0) && ((size_t)(-offset) > self->cursor)) return false;

    return MemoryStream_Seek_Set(self, self->cursor + offset);
}

// Dynamic functions
bool MemoryStream_WriteChar(MemoryStream* self, uint8_t character, uint8_t** ptr_to_data_ptr)
{
    if (!MemoryStream_EnsureCapacity(self, self->cursor + 1)) return false;

    uint8_t* ptr = self->data + self->cursor;
    *ptr = character;
    self->cursor += 1;

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;

    return true;
}
bool MemoryStream_WriteBytes(MemoryStream* self, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr)
{
    if (count == 0) return false;
    if (!MemoryStream_EnsureCapacity(self, self->cursor + count)) return false;

    uint8_t* ptr = self->data + self->cursor;
    memcpy(ptr, bytes, count);
    self->cursor += count;

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;
    return true;
}
bool MemoryStream_WriteCstr(MemoryStream* self, const char* string, size_t buff_len, uint8_t** ptr_to_data_ptr)
{
    size_t string_len = strnlen_s(string, buff_len);
    return MemoryStream_WriteBytes(self, (const uint8_t*)string, string_len, ptr_to_data_ptr);
}
bool MemoryStream_WriteHex(MemoryStream* self, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr)
{
    if (!self || !bytes || count == 0) return false;

    size_t required_capacity = self->cursor + (count * 2);
    if (!MemoryStream_EnsureCapacity(self, required_capacity)) return false;

    uint8_t* ptr = self->data + self->cursor;

    for (size_t i = 0; i < count; i++)
    {
        char hex[3] = {0};
        charToHexChars(bytes[i], hex);
        ptr[0] = (uint8_t)hex[0];
        ptr[1] = (uint8_t)hex[1];
        ptr += 2;
    }

    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + self->cursor;
    self->cursor += count * 2;

    return true;
}
bool MemoryStream_SetBytes(MemoryStream* self, uint8_t value, size_t count, uint8_t** ptr_to_data_ptr)
{
    if (!MemoryStream_EnsureCapacity(self, self->cursor + count)) return false;
    memset(self->data + self->cursor, value, count);

    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + self->cursor;
    self->cursor += count;
    return true;
}
bool MemoryStream_ReadChar(MemoryStream* self, char* out, uint8_t** ptr_to_data_ptr)
{
    if (!self || self->cursor >= self->capacity) return false;
    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + self->cursor;
    if(out) *out = (char)self->data[self->cursor];
    self->cursor += 1;
    return true;
}
bool MemoryStream_ReadBytes(MemoryStream* self, uint8_t* out, uint8_t** ptr_to_data_ptr, size_t count)
{
    if (!self || self->cursor + count > self->capacity) return false;
    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + self->cursor;
    if(out) memcpy(out, self->data + self->cursor, count);
    self->cursor += count;
    return true;
}

// Static functions
bool MemoryStream_Static_WriteChar(MemoryStream* self, size_t cursor, uint8_t character, uint8_t** ptr_to_data_ptr)
{
    if (!MemoryStream_EnsureCapacity(self, cursor + 1)) return false;

    uint8_t* ptr = self->data + cursor;
    *ptr = character;

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;
    return true;
}
bool MemoryStream_Static_WriteBytes(MemoryStream* self, size_t cursor, const uint8_t* bytes, size_t count, uint8_t** ptr_to_data_ptr)
{
    if (count == 0) return false;
    if (!MemoryStream_EnsureCapacity(self, cursor + count)) return false;

    uint8_t* ptr = self->data + cursor;
    memcpy(ptr, bytes, count);

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;
    return true;
}
bool MemoryStream_Static_WriteCstr(MemoryStream* self, size_t cursor, const char* string, uint8_t** ptr_to_data_ptr)
{
    size_t string_len = strlen(string);
    return MemoryStream_Static_WriteBytes(self, cursor, (const uint8_t*)string, string_len, ptr_to_data_ptr);
}
bool MemoryStream_Static_WriteHex(MemoryStream* self, size_t cursor, uint8_t character, uint8_t** ptr_to_data_ptr)
{
    if (!MemoryStream_EnsureCapacity(self, cursor + 2)) return false;

    char hex[3] = {0};
    charToHexChars(character, hex);

    uint8_t* ptr = self->data + cursor;
    ptr[0] = (uint8_t)hex[0];
    ptr[1] = (uint8_t)hex[1];

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;
    return true;
}
bool MemoryStream_Static_SetBytes(MemoryStream* self, size_t cursor, uint8_t value, size_t count, uint8_t** ptr_to_data_ptr)
{
    if (!MemoryStream_EnsureCapacity(self, cursor + count)) return false;

    uint8_t* ptr = self->data + cursor;
    memset(ptr, value, count);

    if (ptr_to_data_ptr) *ptr_to_data_ptr = ptr;
    return true;
}
bool MemoryStream_Static_ReadChar(MemoryStream* self, size_t cursor, uint8_t** ptr_to_data_ptr)
{
    if (!self || cursor >= self->capacity) return false;

    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + cursor;
    return true;
}
bool MemoryStream_Static_ReadBytes(MemoryStream* self, size_t cursor, uint8_t** ptr_to_data_ptr, size_t count)
{
    if (!self || cursor + count > self->capacity) return false;

    if (ptr_to_data_ptr) *ptr_to_data_ptr = self->data + cursor;
    return true;
}
