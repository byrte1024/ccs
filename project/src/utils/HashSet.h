// HashSet.h

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ResizeableArray.h"

// Simple hashset structure
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ResizeableArray.h"

// Tunables
#ifndef HASHSET_INITIAL_BUCKET_CAPACITY
#define HASHSET_INITIAL_BUCKET_CAPACITY 2
#endif

#ifndef HASHSET_LOAD_FACTOR
#define HASHSET_LOAD_FACTOR 0.75
#endif

typedef struct HashSet
{
    ResizeableArray *buckets; // dynamic array of buckets (each is a ResizeableArray)
    size_t bucket_count;
    size_t size; // total number of elements stored
    size_t member_size_bytes;

    // user-provided functions
    size_t (*hash_func)(void *element);
    bool (*equals_func)(void *a, void *b);
} HashSet;

static inline size_t int_hash_func(void* element) {
    int x = *(int*)element;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return (size_t)x;
}
bool int_equals_func(void* a, void* b){
    int* int_a = (int*)a;
    int* int_b = (int*)b;
    return *int_a == *int_b;

}
static inline size_t long_hash_func(void* element) {
    uint64_t x = *(uint64_t*)element;
    x = (~x) + (x << 21); // x = (x << 21) - x - 1;
    x = x ^ (x >> 24);
    x = (x + (x << 3)) + (x << 8); // x * 265
    x = x ^ (x >> 14);
    x = (x + (x << 2)) + (x << 4); // x * 21
    x = x ^ (x >> 28);
    x = x + (x << 31);
    return (size_t)x;
}
static inline bool long_equals_func(void* a, void* b) {
    uint64_t x = *(uint64_t*)a;
    uint64_t y = *(uint64_t*)b;
    return x == y;
}

static inline size_t bytes_hash_func(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    size_t h = 14695981039346656037ULL; // FNV offset basis
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 1099511628211ULL; // FNV prime
    }
    return h;
}

static inline bool bytes_equals_func(const void* data, size_t len){
    const uint8_t *a = (const uint8_t *)data;
    const uint8_t *b = (const uint8_t *)b;
    return memcmp(a, b, len) == 0;
}


// Initialize a hashset. Returns true on success.
static inline bool HashSet_init(HashSet *set, size_t member_size_bytes, size_t bucket_count,
                                size_t (*hash_func)(void *), bool (*equals_func)(void *, void *))
{
    if (!set || !hash_func || !equals_func || bucket_count == 0)
        return false;

    set->member_size_bytes = member_size_bytes;
    set->bucket_count = bucket_count;
    set->hash_func = hash_func;
    set->equals_func = equals_func;
    set->size = 0;

    // Allocate zeroed memory for buckets
    set->buckets = (ResizeableArray *)calloc(bucket_count, sizeof(ResizeableArray));
    if (!set->buckets)
    {
        set->bucket_count = 0;
        return false;
    }

    // Initialize each bucket with a small initial capacity to avoid many reallocs
    for (size_t i = 0; i < bucket_count; ++i)
    {
        ResizeableArray_init(&set->buckets[i], member_size_bytes, HASHSET_INITIAL_BUCKET_CAPACITY);
        // Note: ResizeableArray_init may set data==NULL if allocation failed for that bucket;
        // that's acceptable because ResizeableArray_push will realloc when first used.
    }

    return true;
}

// Deinitialize a hashset
static inline void HashSet_deinit(HashSet *set)
{
    if (!set)
        return;
    if (set->buckets)
    {
        for (size_t i = 0; i < set->bucket_count; ++i)
        {
            ResizeableArray_deinit(&set->buckets[i]);
        }
        free(set->buckets);
        set->buckets = NULL;
    }
    set->bucket_count = 0;
    set->size = 0;
    set->member_size_bytes = 0;
    set->hash_func = NULL;
    set->equals_func = NULL;
}

// Internal: rehash to new_bucket_count. Returns true on success, false on failure (no state change on failure).
static inline bool HashSet_rehash(HashSet *set, size_t new_bucket_count)
{
    if (!set || new_bucket_count == 0)
        return false;
    if (new_bucket_count == set->bucket_count)
        return true;

    ResizeableArray *new_buckets = (ResizeableArray *)calloc(new_bucket_count, sizeof(ResizeableArray));
    if (!new_buckets)
        return false;

    // init new buckets
    for (size_t i = 0; i < new_bucket_count; ++i)
    {
        ResizeableArray_init(&new_buckets[i], set->member_size_bytes, HASHSET_INITIAL_BUCKET_CAPACITY);
    }

    // Move all elements from old buckets to new_buckets
    for (size_t b = 0; b < set->bucket_count; ++b)
    {
        ResizeableArray *old = &set->buckets[b];
        for (size_t j = 0; j < old->member_count; ++j)
        {
            void *elem = ResizeableArray_get(old, j);
            if (!elem)
                continue; // defensive
            size_t h = set->hash_func(elem);
            size_t idx = h % new_bucket_count;
            // push into new bucket — check for failure
            void *push_ret = ResizeableArray_push(&new_buckets[idx], elem);
            if (!push_ret)
            {
                // push failed (likely OOM) -> rollback: deinit new buckets and free memory
                for (size_t k = 0; k < new_bucket_count; ++k)
                {
                    ResizeableArray_deinit(&new_buckets[k]);
                }
                free(new_buckets);
                return false;
            }
        }
    }

    // Success: teardown old buckets and adopt new ones
    for (size_t i = 0; i < set->bucket_count; ++i)
    {
        ResizeableArray_deinit(&set->buckets[i]);
    }
    free(set->buckets);

    set->buckets = new_buckets;
    set->bucket_count = new_bucket_count;
    // set->size remains same
    return true;
}

// Returns true if element is present
static inline bool HashSet_contains(const HashSet *set, void *element)
{
    if (!set || !element)
        return false;
    if (set->bucket_count == 0 || !set->buckets)
        return false;

    size_t hash = set->hash_func(element);
    size_t index = hash % set->bucket_count;
    ResizeableArray *bucket = &((HashSet *)set)->buckets[index]; // cast away const for ResizeableArray_get

    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        void *existing = ResizeableArray_get(bucket, i);
        if (existing && set->equals_func(existing, element))
        {
            return true;
        }
    }
    return false;
}

// Add an element. Returns true if added, false if already present or on allocation failure.
static inline bool HashSet_add(HashSet *set, void *element)
{
    if (!set || !element)
        return false;

    // Expand if load factor exceeded (try to grow before inserting)
    if ((double)(set->size + 1) > (double)set->bucket_count * HASHSET_LOAD_FACTOR)
    {
        // attempt to double buckets; if it fails, we continue and try to insert into current buckets
        (void)HashSet_rehash(set, set->bucket_count * 2);
    }

    size_t hash = set->hash_func(element);
    size_t index = hash % set->bucket_count;
    ResizeableArray *bucket = &set->buckets[index];

    // Check duplicate
    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        void *existing = ResizeableArray_get(bucket, i);
        if (existing && set->equals_func(existing, element))
        {
            return false; // already present
        }
    }

    // Insert and check success
    void *push_ret = ResizeableArray_push(bucket, element);
    if (!push_ret)
    {
        return false; // allocation failed
    }

    set->size++;
    return true;
}

// Remove an element. Returns true if removed.
static inline bool HashSet_remove(HashSet *set, void *element)
{
    if (!set || !element)
        return false;
    if (set->bucket_count == 0 || !set->buckets)
        return false;

    size_t hash = set->hash_func(element);
    size_t index = hash % set->bucket_count;
    ResizeableArray *bucket = &set->buckets[index];

    for (size_t i = 0; i < bucket->member_count; ++i)
    {
        void *existing = ResizeableArray_get(bucket, i);
        if (existing && set->equals_func(existing, element))
        {
            bool ok = ResizeableArray_remove(bucket, i, NULL);
            if (ok)
            {
                if (set->size > 0)
                    set->size--;
                // Optionally shrink buckets when very sparse — not implemented by default
            }
            return ok;
        }
    }
    return false;
}

// Remove all elements (but keep bucket array)
static inline void HashSet_clear(HashSet *set)
{
    if (!set || !set->buckets)
        return;
    for (size_t i = 0; i < set->bucket_count; ++i)
    {
        ResizeableArray_deinit(&set->buckets[i]);
        ResizeableArray_init(&set->buckets[i], set->member_size_bytes, HASHSET_INITIAL_BUCKET_CAPACITY);
    }
    set->size = 0;
}

static inline size_t HashSet_size(const HashSet *set)
{
    return set ? set->size : 0;
}

static inline void HashSet_foreach(HashSet *set,
                                   void (*callback)(void *element, void *userdata),
                                   void *userdata)
{
    if (!set || !callback)
        return;
    for (size_t b = 0; b < set->bucket_count; b++)
    {
        ResizeableArray *bucket = &set->buckets[b];
        for (size_t i = 0; i < bucket->member_count; i++)
        {
            void *elem = ResizeableArray_get(bucket, i);
            callback(elem, userdata);
        }
    }
}