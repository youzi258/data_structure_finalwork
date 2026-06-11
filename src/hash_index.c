/* 哈希索引模块实现：使用链地址法把类别或关键词映射到物品节点。
 */

#include "hash_index.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int valid_mode(HashIndexMode mode) {
    return mode == HASH_INDEX_CATEGORY || mode == HASH_INDEX_KEYWORDS;
}

static int ascii_tolower(int value) {
    return tolower((unsigned char)value);
}

static int strings_equal_case_insensitive(const char *left, const char *right) {
    if (left == NULL || right == NULL) {
        return 0;
    }
    while (*left != '\0' && *right != '\0') {
        if (ascii_tolower(*left) != ascii_tolower(*right)) {
            return 0;
        }
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static void copy_lower(char *dest, size_t dest_size, const char *source) {
    size_t index = 0;

    if (dest_size == 0) {
        return;
    }
    if (source == NULL) {
        dest[0] = '\0';
        return;
    }
    while (source[index] != '\0' && index + 1 < dest_size) {
        dest[index] = (char)ascii_tolower(source[index]);
        index++;
    }
    dest[index] = '\0';
}

static size_t hash_key(const char *key) {
    size_t hash = 5381;

    while (*key != '\0') {
        hash = ((hash << 5) + hash) + (unsigned char)ascii_tolower(*key);
        key++;
    }
    return hash;
}

static void hash_index_remove_entries(HashIndex *index) {
    size_t bucket;

    if (index == NULL || index->buckets == NULL) {
        return;
    }
    for (bucket = 0; bucket < index->bucket_count; bucket++) {
        HashIndexEntry *current = index->buckets[bucket];

        while (current != NULL) {
            HashIndexEntry *next = current->next;

            free(current);
            current = next;
        }
        index->buckets[bucket] = NULL;
    }
}

static int hash_index_insert(HashIndex *index, const char *raw_key, const Item *item) {
    char key[ITEM_KEYWORDS_LENGTH];
    size_t bucket;
    HashIndexEntry *entry;

    if (index == NULL || index->buckets == NULL ||
        raw_key == NULL || raw_key[0] == '\0' || item == NULL) {
        return 1;
    }
    copy_lower(key, sizeof(key), raw_key);
    bucket = hash_key(key) % index->bucket_count;
    entry = malloc(sizeof(*entry));
    if (entry == NULL) {
        return 0;
    }
    strcpy(entry->key, key);
    entry->item = item;
    entry->next = index->buckets[bucket];
    index->buckets[bucket] = entry;
    return 1;
}

static size_t copy_token(char *dest, size_t dest_size, const char **cursor) {
    size_t length = 0;

    while (**cursor != '\0' && !isalnum((unsigned char)**cursor)) {
        (*cursor)++;
    }
    while (**cursor != '\0' && isalnum((unsigned char)**cursor)) {
        if (length + 1 < dest_size) {
            dest[length++] = **cursor;
        }
        (*cursor)++;
    }
    if (dest_size > 0) {
        dest[length] = '\0';
    }
    return length;
}

static int item_has_indexed_token(
    const HashIndex *index,
    const Item *item,
    const char *raw_key
) {
    char key[ITEM_KEYWORDS_LENGTH];
    size_t bucket;
    HashIndexEntry *entry;

    copy_lower(key, sizeof(key), raw_key);
    bucket = hash_key(key) % index->bucket_count;
    entry = index->buckets[bucket];
    while (entry != NULL) {
        if (entry->item == item &&
            strings_equal_case_insensitive(entry->key, key)) {
            return 1;
        }
        entry = entry->next;
    }
    return 0;
}

/* 关键词字段按字母数字 token 拆分，同一记录的重复 token 不重复入桶。 */
static int hash_index_insert_keywords(HashIndex *index, const Item *item) {
    const char *cursor = item->keywords;

    while (*cursor != '\0') {
        char token[ITEM_KEYWORDS_LENGTH];

        if (copy_token(token, sizeof(token), &cursor) == 0) {
            break;
        }
        if (!item_has_indexed_token(index, item, token) &&
            !hash_index_insert(index, token, item)) {
            return 0;
        }
    }
    return 1;
}

int hash_index_init(HashIndex *index, size_t bucket_count, HashIndexMode mode) {
    if (index == NULL || bucket_count == 0 || !valid_mode(mode)) {
        return 0;
    }
    index->buckets = calloc(bucket_count, sizeof(*index->buckets));
    if (index->buckets == NULL) {
        index->bucket_count = 0;
        return 0;
    }
    index->bucket_count = bucket_count;
    index->mode = mode;
    return 1;
}

int hash_index_build(HashIndex *index, const ItemList *items) {
    const Item *current;

    if (index == NULL || index->buckets == NULL || items == NULL) {
        return 0;
    }
    hash_index_remove_entries(index);
    current = items->head;
    while (current != NULL) {
        if (index->mode == HASH_INDEX_CATEGORY) {
            if (!hash_index_insert(index, current->category, current)) {
                hash_index_remove_entries(index);
                return 0;
            }
        } else if (!hash_index_insert_keywords(index, current)) {
            hash_index_remove_entries(index);
            return 0;
        }
        current = current->next;
    }
    return 1;
}

/* 查询时只遍历目标桶内的冲突链，命中后通过回调输出原始记录。 */
size_t hash_index_visit(
    const HashIndex *index,
    const char *key,
    ItemVisitor visitor,
    void *context
) {
    char normalized[ITEM_KEYWORDS_LENGTH];
    HashIndexEntry *current;
    size_t matches = 0;

    if (index == NULL || index->buckets == NULL ||
        key == NULL || visitor == NULL || index->bucket_count == 0) {
        return 0;
    }
    copy_lower(normalized, sizeof(normalized), key);
    current = index->buckets[hash_key(normalized) % index->bucket_count];
    while (current != NULL) {
        if (strcmp(current->key, normalized) == 0) {
            visitor(current->item, context);
            matches++;
        }
        current = current->next;
    }
    return matches;
}

void hash_index_clear(HashIndex *index) {
    if (index == NULL) {
        return;
    }
    hash_index_remove_entries(index);
    free(index->buckets);
    index->buckets = NULL;
    index->bucket_count = 0;
}
