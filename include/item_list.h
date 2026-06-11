#ifndef ITEM_LIST_H
#define ITEM_LIST_H

#include "item.h"

#include <stddef.h>

typedef enum {
    LIST_OK = 0,
    LIST_INVALID_ARGUMENT,
    LIST_INVALID_ITEM,
    LIST_DUPLICATE_ID,
    LIST_WRONG_TYPE,
    LIST_NOT_FOUND,
    LIST_OUT_OF_MEMORY
} ListResult;

typedef enum {
    ITEM_FIELD_NAME,
    ITEM_FIELD_CATEGORY,
    ITEM_FIELD_LOCATION,
    ITEM_FIELD_KEYWORDS
} ItemField;

typedef struct {
    Item *head;
    size_t size;
    ItemType accepted_type;
} ItemList;

typedef void (*ItemVisitor)(const Item *item, void *context);

void item_list_init(ItemList *list, ItemType accepted_type);
ListResult item_list_append(ItemList *list, const Item *item);
Item *item_list_find_by_id(ItemList *list, int id);
const Item *item_list_find_by_id_const(const ItemList *list, int id);
ListResult item_list_update(ItemList *list, int id, const Item *replacement);
ListResult item_list_remove(ItemList *list, int id, Item *removed);
size_t item_list_visit_matching(
    const ItemList *list,
    ItemField field,
    const char *query,
    ItemVisitor visitor,
    void *context
);
void item_list_clear(ItemList *list);

#endif
