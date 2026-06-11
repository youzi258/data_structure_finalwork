#include "item_list.h"
#include "statistics.h"
#include "test.h"

#include <string.h>

static Item make_item(int id, const char *location) {
    Item item = {0};

    item.id = id;
    item.type = ITEM_LOST;
    strcpy(item.name, "Campus card");
    strcpy(item.category, "Card");
    strcpy(item.color, "Blue");
    strcpy(item.location, location);
    strcpy(item.keywords, "campus card");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void test_location_counts_are_sorted_descending(void) {
    ItemList lost_items;
    LocationStatsList stats;
    Item library_one = make_item(1, "Library");
    Item canteen = make_item(2, "Canteen");
    Item library_two = make_item(3, "library");
    Item playground = make_item(4, "Playground");
    Item library_three = make_item(5, "Library");

    item_list_init(&lost_items, ITEM_LOST);
    location_stats_list_init(&stats);
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &library_one));
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &canteen));
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &library_two));
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &playground));
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &library_three));

    ASSERT_TRUE(location_stats_build(&lost_items, &stats));
    ASSERT_EQ(3, stats.size);
    ASSERT_TRUE(stats.head != NULL);
    ASSERT_STR_EQ("Library", stats.head->location);
    ASSERT_EQ(3, stats.head->count);
    ASSERT_TRUE(stats.head->next != NULL);
    ASSERT_EQ(1, stats.head->next->count);
    ASSERT_TRUE(stats.head->next->next != NULL);
    ASSERT_EQ(1, stats.head->next->next->count);

    location_stats_list_clear(&stats);
    item_list_clear(&lost_items);
}

int main(void) {
    test_location_counts_are_sorted_descending();
    return test_finish();
}
