// ResizeableArray.h

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "BinaryUtils.h"

typedef struct ResizeableArray
{
    void *data;
    size_t member_size_bytes;

    size_t member_count;
    size_t member_capacity;

    size_t member_count_bytes;
    size_t member_capacity_bytes;

} ResizeableArray;

static inline void ResizeableArray_init(ResizeableArray *arr, size_t member_size_bytes, size_t member_capacity)
{
    if (arr == NULL)
        return;

    arr->member_size_bytes = member_size_bytes;

    arr->member_count = 0;
    arr->member_capacity = member_capacity;

    arr->member_count_bytes = 0;
    arr->member_capacity_bytes = member_capacity * member_size_bytes;

    arr->data = malloc(arr->member_capacity_bytes);
    if (arr->data == NULL)
        return;
    memset(arr->data, 0, arr->member_capacity_bytes);
}

static inline void ResizeableArray_deinit(ResizeableArray *arr)
{
    if (arr == NULL)
        return;
    if (arr->data != NULL)
    {
        free(arr->data);
        arr->data = NULL;
    }
    arr->member_count = 0;
    arr->member_capacity = 0;
    arr->member_count_bytes = 0;
    arr->member_capacity_bytes = 0;
}

static inline void ResizeableArray_realloc(ResizeableArray *arr, size_t new_capacity)
{
    if (arr == NULL)
        return;
    if (new_capacity == 0)
    {
        ResizeableArray_deinit(arr);
        return;
    }

    void *new_data = realloc(arr->data, new_capacity * arr->member_size_bytes);
    if (new_data == NULL)
        return; // allocation failed, keep old data

    // Only zero out the newly allocated portion
    if (new_capacity > arr->member_capacity)
    {
        size_t old_bytes = arr->member_capacity * arr->member_size_bytes;
        size_t new_bytes = new_capacity * arr->member_size_bytes;
        memset((uint8_t *)new_data + old_bytes, 0, new_bytes - old_bytes);
    }

    arr->data = new_data;
    arr->member_capacity = new_capacity;
    arr->member_capacity_bytes = new_capacity * arr->member_size_bytes;

    if (arr->member_count > new_capacity)
    {
        arr->member_count = new_capacity;
        arr->member_count_bytes = new_capacity * arr->member_size_bytes;
    }
}

static inline void *ResizeableArray_push(ResizeableArray *arr, void *memberPtr)
{
    if (arr == NULL || memberPtr == NULL)
        return NULL;

    // Resize if needed
    if (arr->member_count >= arr->member_capacity)
    {
        size_t new_capacity = arr->member_capacity > 0 ? arr->member_capacity * 2 : 1;
        ResizeableArray_realloc(arr, new_capacity);
        if (arr->member_count >= arr->member_capacity)
            return NULL; // realloc failed
    }

    void *dest = (uint8_t *)arr->data + arr->member_count * arr->member_size_bytes;
    memcpy(dest, memberPtr, arr->member_size_bytes);
    arr->member_count++;
    arr->member_count_bytes = arr->member_count * arr->member_size_bytes;

    return dest;
}

static inline bool ResizeableArray_pop(ResizeableArray *arr, void *out_member)
{
    if (arr == NULL || arr->member_count == 0)
        return false;

    arr->member_count--;
    arr->member_count_bytes = arr->member_count * arr->member_size_bytes;

    if (out_member != NULL)
    {
        void *src = (uint8_t *)arr->data + arr->member_count * arr->member_size_bytes;
        memcpy(out_member, src, arr->member_size_bytes);
    }

    return true;
}

static inline void *ResizeableArray_get(ResizeableArray *arr, size_t index)
{
    if (arr == NULL || index >= arr->member_count)
        return NULL;
    return (uint8_t *)arr->data + index * arr->member_size_bytes;
}

static inline bool ResizeableArray_set(ResizeableArray *arr, size_t index, void *memberPtr)
{
    if (arr == NULL || memberPtr == NULL || index >= arr->member_count)
        return false;
    void *dest = (uint8_t *)arr->data + index * arr->member_size_bytes;
    memcpy(dest, memberPtr, arr->member_size_bytes);
    return true;
}

// Insert a member at a specific index
static inline bool ResizeableArray_insert(ResizeableArray *arr, size_t index, void *memberPtr)
{
    if (arr == NULL || memberPtr == NULL)
        return false;
    if (index > arr->member_count)
        return false; // can insert at arr->member_count (end)

    // Resize if needed
    if (arr->member_count >= arr->member_capacity)
    {
        size_t new_capacity = arr->member_capacity > 0 ? arr->member_capacity * 2 : 1;
        ResizeableArray_realloc(arr, new_capacity);
        if (arr->member_count >= arr->member_capacity)
            return false; // realloc failed
    }

    void *dest = (uint8_t *)arr->data + index * arr->member_size_bytes;
    // Move existing elements to the right
    memmove((uint8_t *)dest + arr->member_size_bytes, dest, (arr->member_count - index) * arr->member_size_bytes);

    memcpy(dest, memberPtr, arr->member_size_bytes);
    arr->member_count++;
    arr->member_count_bytes = arr->member_count * arr->member_size_bytes;

    return true;
}

// Remove a member at a specific index
static inline bool ResizeableArray_remove(ResizeableArray *arr, size_t index, void *out_member)
{
    if (arr == NULL || index >= arr->member_count)
        return false;

    void *src = (uint8_t *)arr->data + index * arr->member_size_bytes;
    if (out_member != NULL)
    {
        memcpy(out_member, src, arr->member_size_bytes);
    }

    // Move elements after index to the left
    if (index < arr->member_count - 1)
    {
        memmove(src, (uint8_t *)src + arr->member_size_bytes, (arr->member_count - index - 1) * arr->member_size_bytes);
    }

    arr->member_count--;
    arr->member_count_bytes = arr->member_count * arr->member_size_bytes;

    return true;
}

static inline void ResizeableArray_foreach(ResizeableArray *arr,
                                           void (*callback)(void *element, size_t index, void *userdata),
                                           void *userdata)
{

    if (!arr || !callback)
        return;
    for (size_t i = 0; i < arr->member_count; i++)
    {
        void *elem = (uint8_t *)arr->data + i * arr->member_size_bytes;
        callback(elem, i, userdata);
    }
}