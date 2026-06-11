#include "item_list.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static Item *item_copy_new(const Item *source) {
    Item *copy = malloc(sizeof(*copy));

    if (copy == NULL) {
        return NULL;
    }
    *copy = *source;
    copy->next = NULL;
    return copy;
}

static int contains_case_insensitive(const char *text, const char *query) {
    size_t query_length;
    size_t index;

    if (text == NULL || query == NULL || query[0] == '\0') {
        return 0;
    }
    query_length = strlen(query);
    while (*text != '\0') {
        for (index = 0; index < query_length; index++) {
            unsigned char text_char = (unsigned char)text[index];
            unsigned char query_char = (unsigned char)query[index];

            if (text_char == '\0' ||
                tolower(text_char) != tolower(query_char)) {
                break;
            }
        }
        if (index == query_length) {
            return 1;
        }
        text++;
    }
    return 0;
}

static const char *field_value(const Item *item, ItemField field) {
    switch (field) {
        case ITEM_FIELD_NAME:
            return item->name;
        case ITEM_FIELD_CATEGORY:
            return item->category;
        case ITEM_FIELD_LOCATION:
            return item->location;
        case ITEM_FIELD_KEYWORDS:
            return item->keywords;
        default:
            return NULL;
    }
}

void item_list_init(ItemList *list, ItemType accepted_type) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->size = 0;
    list->accepted_type = accepted_type;
}

ListResult item_list_append(ItemList *list, const Item *item) {
    Item *copy;
    Item *tail;

    if (list == NULL || item == NULL) {
        return LIST_INVALID_ARGUMENT;
    }
    if (!item_is_valid(item)) {
        return LIST_INVALID_ITEM;
    }
    if (item->type != list->accepted_type) {
        return LIST_WRONG_TYPE;
    }
    if (item_list_find_by_id(list, item->id) != NULL) {
        return LIST_DUPLICATE_ID;
    }

    copy = item_copy_new(item);
    if (copy == NULL) {
        return LIST_OUT_OF_MEMORY;
    }
    if (list->head == NULL) {
        list->head = copy;
    } else {
        tail = list->head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = copy;
    }
    list->size++;
    return LIST_OK;
}

Item *item_list_find_by_id(ItemList *list, int id) {
    Item *current;

    if (list == NULL) {
        return NULL;
    }
    current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

const Item *item_list_find_by_id_const(const ItemList *list, int id) {
    const Item *current;

    if (list == NULL) {
        return NULL;
    }
    current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

ListResult item_list_update(ItemList *list, int id, const Item *replacement) {
    Item *target;
    Item updated;

    if (list == NULL || replacement == NULL) {
        return LIST_INVALID_ARGUMENT;
    }
    target = item_list_find_by_id(list, id);
    if (target == NULL) {
        return LIST_NOT_FOUND;
    }

    updated = *replacement;
    updated.id = target->id;
    updated.type = target->type;
    updated.next = target->next;
    if (!item_is_valid(&updated)) {
        return LIST_INVALID_ITEM;
    }
    *target = updated;
    return LIST_OK;
}

ListResult item_list_remove(ItemList *list, int id, Item *removed) {
    Item *current;
    Item *previous = NULL;

    if (list == NULL) {
        return LIST_INVALID_ARGUMENT;
    }
    current = list->head;
    while (current != NULL && current->id != id) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        return LIST_NOT_FOUND;
    }
    if (previous == NULL) {
        list->head = current->next;
    } else {
        previous->next = current->next;
    }
    if (removed != NULL) {
        *removed = *current;
        removed->next = NULL;
    }
    free(current);
    list->size--;
    return LIST_OK;
}

size_t item_list_visit_matching(
    const ItemList *list,
    ItemField field,
    const char *query,
    ItemVisitor visitor,
    void *context
) {
    const Item *current;
    size_t matches = 0;

    if (list == NULL || query == NULL || visitor == NULL) {
        return 0;
    }
    current = list->head;
    while (current != NULL) {
        const char *value = field_value(current, field);

        if (value != NULL && contains_case_insensitive(value, query)) {
            visitor(current, context);
            matches++;
        }
        current = current->next;
    }
    return matches;
}

void item_list_clear(ItemList *list) {
    Item *current;

    if (list == NULL) {
        return;
    }
    current = list->head;
    while (current != NULL) {
        Item *next = current->next;

        free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}
