/* 统计模块实现：统计失物高发地点并按数量降序输出。
 */

#include "statistics.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

static LocationStat *find_location(LocationStatsList *stats, const char *location) {
    LocationStat *current;

    if (stats == NULL || location == NULL) {
        return NULL;
    }
    current = stats->head;
    while (current != NULL) {
        if (strings_equal_case_insensitive(current->location, location)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static int append_location(LocationStatsList *stats, const char *location) {
    LocationStat *node;
    LocationStat *tail;

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }
    strncpy(node->location, location, sizeof(node->location) - 1);
    node->location[sizeof(node->location) - 1] = '\0';
    node->count = 1;
    node->next = NULL;
    if (stats->head == NULL) {
        stats->head = node;
    } else {
        tail = stats->head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = node;
    }
    stats->size++;
    return 1;
}

static void insert_stat_sorted(LocationStatsList *sorted, LocationStat *node) {
    LocationStat *current;

    if (sorted->head == NULL || node->count > sorted->head->count) {
        node->next = sorted->head;
        sorted->head = node;
        sorted->size++;
        return;
    }
    current = sorted->head;
    while (current->next != NULL && current->next->count >= node->count) {
        current = current->next;
    }
    node->next = current->next;
    current->next = node;
    sorted->size++;
}

static void sort_stats_descending(LocationStatsList *stats) {
    LocationStatsList sorted;
    LocationStat *current;

    location_stats_list_init(&sorted);
    current = stats->head;
    while (current != NULL) {
        LocationStat *next = current->next;

        current->next = NULL;
        insert_stat_sorted(&sorted, current);
        current = next;
    }
    stats->head = sorted.head;
    stats->size = sorted.size;
}

void location_stats_list_init(LocationStatsList *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->size = 0;
}

int location_stats_build(const ItemList *lost_items, LocationStatsList *stats) {
    const Item *current;

    if (lost_items == NULL || stats == NULL) {
        return 0;
    }
    location_stats_list_clear(stats);
    current = lost_items->head;
    while (current != NULL) {
        LocationStat *existing = find_location(stats, current->location);

        if (existing != NULL) {
            existing->count++;
        } else if (!append_location(stats, current->location)) {
            location_stats_list_clear(stats);
            return 0;
        }
        current = current->next;
    }
    sort_stats_descending(stats);
    return 1;
}

void location_stats_list_clear(LocationStatsList *list) {
    LocationStat *current;

    if (list == NULL) {
        return;
    }
    current = list->head;
    while (current != NULL) {
        LocationStat *next = current->next;

        free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}
