
#include "hashmap.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

// 32bit
#define FNV1A_PRIME 0x01000193
#define FNV1A_OFFSET 0x811c9dc5

static size_t
hash_fnv1a(const char* key)
{
    size_t hash = FNV1A_OFFSET;

    for (size_t i = 0; i < strlen(key); i++) {
        hash = (hash ^ key[i]) * FNV1A_PRIME;
    }

    return hash;
}

static size_t
get_index(struct hash_table* ht, const char* key)
{
    return hash_fnv1a(key) % ht->cap;
}

static void
bucket_free(struct ht_bucket* bucket)
{
    if (!bucket)
        return;

    struct ht_entry* entry = bucket->front;
    struct ht_entry* tmp = NULL;

    while (entry) {
        tmp = entry;
        entry = entry->next;

        free(tmp->key);
        free(tmp);
    }

    free(bucket);
}

struct hash_table*
ht_create(size_t cap)
{
    assert(cap /*Invalid cap size*/);

    struct hash_table* ht = malloc(sizeof *ht);

    if (!ht)
        return NULL;

    ht->cap = cap;
    ht->buckets = calloc(cap, sizeof *ht->buckets);

    if (!ht->buckets) {
        free(ht);

        return NULL;
    }

    return ht;
}

void
ht_free(struct hash_table* ht)
{
    for (size_t i = 0; i < ht->cap; i++) {
        struct ht_bucket* bucket = ht->buckets[i];

        if (bucket) {
            bucket_free(bucket);
        }
    }

    free(ht->buckets);
    free(ht);
}

void*
ht_get(struct hash_table* ht, const char* key)
{
    if (!strlen(key))
        return NULL;

    size_t index = get_index(ht, key);
    struct ht_bucket* bucket = ht->buckets[index];

    if (!bucket)
        return NULL;

    struct ht_entry* entry = bucket->front;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }

        entry = entry->next;
    }

    return NULL;
}

void
ht_set(struct hash_table* ht, const char* key, void* value)
{
    size_t keylen = strlen(key);
    if (!keylen)
        return;

    size_t index = get_index(ht, key);
    struct ht_bucket* bucket = ht->buckets[index];

    if (!bucket) {
        bucket = malloc(sizeof *bucket);

        if (!bucket) {
            assert(false /* Bucket initialization failed */);
            return;
        }

        bucket->front = NULL;
        ht->buckets[index] = bucket;
    }

    struct ht_entry* entry = bucket->front;
    struct ht_entry* prev = NULL;

    while (entry) {
        // key exists, update its value
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return;
        }

        prev = entry;
        entry = entry->next;
    }

    entry = malloc(sizeof *entry);

    if (!entry) {
        assert(false /* Entry initialization failed */);
        return;
    }

    entry->key = strdup(key);
    entry->value = value;
    entry->next = NULL;

    if (prev) {
        prev->next = entry;
    } else {
        bucket->front = entry;
    }
}

void*
ht_del(struct hash_table* ht, const char* key)
{
    if (!strlen(key))
        return NULL;

    size_t index = get_index(ht, key);
    struct ht_bucket* bucket = ht->buckets[index];

    if (!bucket)
        return NULL;

    struct ht_entry* entry = bucket->front;
    struct ht_entry* prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            void* value = entry->value;

            if (prev) {
                prev->next = entry->next;
            } else {
                bucket->front = entry->next;
            }

            return value;
        }

        prev = entry;
        entry = entry->next;
    }

    return NULL;
}