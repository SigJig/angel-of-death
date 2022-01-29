
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdlib.h>

struct ht_entry {
    char* key;
    void* value;
    struct ht_entry* next;
};

struct ht_bucket {
    struct ht_entry* front;
};

struct hash_table {
    struct ht_bucket** buckets;
    size_t cap;
};

// cap should be 1.3 * expected num of items, and should be a prime number
struct hash_table* ht_create(size_t cap);
void ht_free(struct hash_table* ht);

void* ht_get(struct hash_table* ht, const char* key);

void ht_set(struct hash_table* ht, const char* key, void* value);
void* ht_del(struct hash_table* ht, const char* key);

#endif // HASHMAP_H