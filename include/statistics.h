#ifndef STATISTICS_H
#define STATISTICS_H

#include "item_list.h"

#include <stddef.h>

typedef struct LocationStat {
    char location[ITEM_LOCATION_LENGTH];
    size_t count;
    struct LocationStat *next;
} LocationStat;

typedef struct {
    LocationStat *head;
    size_t size;
} LocationStatsList;

void location_stats_list_init(LocationStatsList *list);
int location_stats_build(const ItemList *lost_items, LocationStatsList *stats);
void location_stats_list_clear(LocationStatsList *list);

#endif
