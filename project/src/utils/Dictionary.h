// Dictionary.h

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ResizeableArray.h"



// Dictionary key-value pair
typedef struct DictEntry
{
    void *key;
    void *value;
} DictEntry;

// Dictionary structure
typedef struct Dictionary
{
    ResizeableArray *buckets; // Array of ResizeableArray (each bucket stores DictEntry)
    size_t bucket_count;
    size_t size; // total number of key-value pairs

    size_t key_size;
    size_t value_size;

    size_t (*hash_func)(void *key);
    bool (*equals_func)(void *a, void *b);
} Dictionary;

// Default initial per-bucket capacity and load factor
#ifndef DICT_INITIAL_BUCKET_CAPACITY
#define DICT_INITIAL_BUCKET_CAPACITY 2
#endif
#ifndef DICT_LOAD_FACTOR
#define DICT_LOAD_FACTOR 0.75
#endif

// Initialize dictionary
static inline bool Dictionary_init(Dictionary *dict, size_t key_size, size_t value_size,
                                   size_t bucket_count, size_t (*hash_func)(void *),
                                   bool (*equals_func)(void *, void *))
{
    if (!dict || !hash_func || !equals_func || bucket_count == 0)
        return false;

    dict->bucket_count = bucket_count;
    dict->key_size = key_size;
    dict->value_size = value_size;
    dict->hash_func = hash_func;
    dict->equals_func = equals_func;
    dict->size = 0;

    dict->buckets = (ResizeableArray *)calloc(bucket_count, sizeof(ResizeableArray));
    if (!dict->buckets)
        return false;

    for (size_t i = 0; i < bucket_count; ++i)
    {
        ResizeableArray_init(&dict->buckets[i], sizeof(DictEntry), DICT_INITIAL_BUCKET_CAPACITY);
    }

    return true;
}

// Deinitialize dictionary
static inline void Dictionary_deinit(Dictionary *dict)
{
    if (!dict || !dict->buckets)
        return;

    for (size_t i = 0; i < dict->bucket_count; ++i)
    {
        ResizeableArray_deinit(&dict->buckets[i]);
    }

    free(dict->buckets);
    dict->buckets = NULL;
    dict->bucket_count = 0;
    dict->size = 0;
    dict->key_size = 0;
    dict->value_size = 0;
    dict->hash_func = NULL;
    dict->equals_func = NULL;
}

// Internal: rehash to a larger number of buckets
static inline bool Dictionary_rehash(Dictionary *dict, size_t new_bucket_count)
{
    if (!dict || new_bucket_count == 0)
        return false;

    ResizeableArray *new_buckets = (ResizeableArray *)calloc(new_bucket_count, sizeof(ResizeableArray));
    if (!new_buckets)
        return false;

    for (size_t i = 0; i < new_bucket_count; ++i)
    {
        ResizeableArray_init(&new_buckets[i], sizeof(DictEntry), DICT_INITIAL_BUCKET_CAPACITY);
    }

    for (size_t i = 0; i < dict->bucket_count; ++i)
    {
        ResizeableArray *old_bucket = &dict->buckets[i];
        for (size_t j = 0; j < old_bucket->member_count; ++j)
        {
            DictEntry *entry = (DictEntry *)ResizeableArray_get(old_bucket, j);
            if (!entry)
                continue;

            size_t h = dict->hash_func(entry->key);
            size_t idx = h % new_bucket_count;
            if (!ResizeableArray_push(&new_buckets[idx], entry))
            {
                // rollback
                for (size_t k = 0; k < new_bucket_count; ++k)
                    ResizeableArray_deinit(&new_buckets[k]);
                free(new_buckets);
                return false;
            }
        }
    }

    // Deinit old buckets
    for (size_t i = 0; i < dict->bucket_count; ++i)
    {
        ResizeableArray_deinit(&dict->buckets[i]);
    }
    free(dict->buckets);

    dict->buckets = new_buckets;
    dict->bucket_count = new_bucket_count;

    return true;
}

// Add or update key-value pair
static inline bool Dictionary_set(Dictionary *dict, void *key, void *value)
{
    if (!dict || !key || !value)
        return false;

    // Rehash if load factor exceeded
    if ((double)(dict->size + 1) > dict->bucket_count * DICT_LOAD_FACTOR)
    {
        (void)Dictionary_rehash(dict, dict->bucket_count * 2);
    }

    size_t h = dict->hash_func(key);
    size_t idx = h % dict->bucket_count;
    ResizeableArray *bucket = &dict->buckets[idx];

    // Check if key exists; update value if found
    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        DictEntry *entry = (DictEntry *)ResizeableArray_get(bucket, i);
        if (entry && dict->equals_func(entry->key, key))
        {
            memcpy(entry->value, value, dict->value_size);
            return true;
        }
    }

    // Key not found; insert new entry
    DictEntry new_entry;
    new_entry.key = malloc(dict->key_size);
    new_entry.value = malloc(dict->value_size);
    if (!new_entry.key || !new_entry.value)
    {
        free(new_entry.key);
        free(new_entry.value);
        return false;
    }
    memcpy(new_entry.key, key, dict->key_size);
    memcpy(new_entry.value, value, dict->value_size);

    if (!ResizeableArray_push(bucket, &new_entry))
    {
        free(new_entry.key);
        free(new_entry.value);
        return false;
    }

    dict->size++;
    return true;
}

// Get value by key. Returns pointer to value or NULL if not found.
static inline void *Dictionary_get(Dictionary *dict, void *key)
{
    if (!dict || !key)
        return NULL;

    size_t h = dict->hash_func(key);
    size_t idx = h % dict->bucket_count;
    ResizeableArray *bucket = &dict->buckets[idx];

    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        DictEntry *entry = (DictEntry *)ResizeableArray_get(bucket, i);
        if (entry && dict->equals_func(entry->key, key))
        {
            return entry->value;
        }
    }
    return NULL;
}

// Remove a key-value pair. Returns true if removed.
static inline bool Dictionary_remove(Dictionary *dict, void *key)
{
    if (!dict || !key)
        return false;

    size_t h = dict->hash_func(key);
    size_t idx = h % dict->bucket_count;
    ResizeableArray *bucket = &dict->buckets[idx];

    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        DictEntry *entry = (DictEntry *)ResizeableArray_get(bucket, i);
        if (entry && dict->equals_func(entry->key, key))
        {
            free(entry->key);
            free(entry->value);
            ResizeableArray_remove(bucket, i, NULL);
            dict->size--;
            return true;
        }
    }
    return false;
}

// Remove all entries but keep buckets
static inline void Dictionary_clear(Dictionary *dict)
{
    if (!dict || !dict->buckets)
        return;

    for (size_t i = 0; i < dict->bucket_count; ++i)
    {
        ResizeableArray *bucket = &dict->buckets[i];
        for (size_t j = 0; j < bucket->member_count; ++j)
        {
            DictEntry *entry = (DictEntry *)ResizeableArray_get(bucket, j);
            if (entry)
            {
                free(entry->key);
                free(entry->value);
            }
        }
        ResizeableArray_deinit(bucket);
        ResizeableArray_init(bucket, sizeof(DictEntry), DICT_INITIAL_BUCKET_CAPACITY);
    }
    dict->size = 0;
}

// Return total number of entries
static inline size_t Dictionary_size(Dictionary *dict)
{
    return dict ? dict->size : 0;
}

static inline void Dictionary_foreach(Dictionary *dict,
                                      void (*callback)(void *key, void *value, void *userdata),
                                      void *userdata)
{
    if (!dict || !callback)
        return;

    for (size_t b = 0; b < dict->bucket_count; b++)
    {
        ResizeableArray *bucket = &dict->buckets[b];
        for (size_t i = 0; i < bucket->member_count; i++)
        {
            DictEntry *entry = ResizeableArray_get(bucket, i);
            if (entry)
                callback(entry->key, entry->value, userdata);
        }
    }
}