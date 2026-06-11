#ifndef HASH_INDEX_H
#define HASH_INDEX_H

#include "item_list.h"

#include <stddef.h>

typedef enum {
    HASH_INDEX_CATEGORY,
    HASH_INDEX_KEYWORDS
} HashIndexMode;

typedef struct HashIndexEntry {
    char key[ITEM_KEYWORDS_LENGTH];
    const Item *item;
    struct HashIndexEntry *next;
} HashIndexEntry;

typedef struct {
    HashIndexEntry **buckets;
    size_t bucket_count;
    HashIndexMode mode;
} HashIndex;

int hash_index_init(HashIndex *index, size_t bucket_count, HashIndexMode mode);
int hash_index_build(HashIndex *index, const ItemList *items);
size_t hash_index_visit(
    const HashIndex *index,
    const char *key,
    ItemVisitor visitor,
    void *context
);
void hash_index_clear(HashIndex *index);

#endif
