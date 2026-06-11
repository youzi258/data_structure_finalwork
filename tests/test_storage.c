#include "storage.h"
#include "test.h"

#include <stdio.h>
#include <string.h>

#define ROUND_TRIP_PATH "tests/tmp/round_trip.csv"
#define MALFORMED_PATH "tests/tmp/malformed.csv"
#define EMPTY_PATH "tests/tmp/empty.csv"
#define MISSING_PATH "tests/tmp/missing.csv"

static Item make_item(int id, ItemType type, const char *name) {
    Item item = {0};

    item.id = id;
    item.type = type;
    strcpy(item.name, name);
    strcpy(item.category, "Card");
    strcpy(item.color, "Blue");
    strcpy(item.location, "Library");
    strcpy(item.keywords, "campus student");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void assert_same_persisted_fields(const Item *expected, const Item *actual) {
    ASSERT_TRUE(actual != NULL);
    if (actual == NULL) {
        return;
    }
    ASSERT_EQ(expected->id, actual->id);
    ASSERT_EQ(expected->type, actual->type);
    ASSERT_STR_EQ(expected->name, actual->name);
    ASSERT_STR_EQ(expected->category, actual->category);
    ASSERT_STR_EQ(expected->color, actual->color);
    ASSERT_STR_EQ(expected->location, actual->location);
    ASSERT_STR_EQ(expected->keywords, actual->keywords);
    ASSERT_STR_EQ(expected->contact, actual->contact);
    ASSERT_EQ(expected->year, actual->year);
    ASSERT_EQ(expected->month, actual->month);
    ASSERT_EQ(expected->day, actual->day);
    ASSERT_EQ(expected->hour, actual->hour);
    ASSERT_EQ(expected->minute, actual->minute);
    ASSERT_EQ(expected->status, actual->status);
}

static void test_save_and_load_round_trip(void) {
    ItemList lost_items;
    ItemList found_items;
    ItemList loaded_lost;
    ItemList loaded_found;
    StorageReport report = {0};
    Item lost = make_item(1, ITEM_LOST, "Campus card");
    Item found = make_item(2, ITEM_FOUND, "Student card");

    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    item_list_init(&loaded_lost, ITEM_LOST);
    item_list_init(&loaded_found, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &lost));
    ASSERT_EQ(LIST_OK, item_list_append(&found_items, &found));

    ASSERT_EQ(STORAGE_OK, storage_save_items(
        ROUND_TRIP_PATH, &lost_items, &found_items));
    ASSERT_EQ(STORAGE_OK, storage_load_items(
        ROUND_TRIP_PATH, &loaded_lost, &loaded_found, &report));
    ASSERT_EQ(2, report.loaded_rows);
    ASSERT_EQ(0, report.skipped_rows);
    assert_same_persisted_fields(
        &lost, item_list_find_by_id_const(&loaded_lost, 1));
    assert_same_persisted_fields(
        &found, item_list_find_by_id_const(&loaded_found, 2));

    item_list_clear(&lost_items);
    item_list_clear(&found_items);
    item_list_clear(&loaded_lost);
    item_list_clear(&loaded_found);
    remove(ROUND_TRIP_PATH);
}

static void test_malformed_and_duplicate_rows_are_skipped(void) {
    FILE *file = fopen(MALFORMED_PATH, "w");
    ItemList lost_items;
    ItemList found_items;
    StorageReport report = {0};

    ASSERT_TRUE(file != NULL);
    if (file == NULL) {
        return;
    }
    fputs(STORAGE_HEADER "\n", file);
    fputs("1,LOST,Campus card,Card,Blue,Library,campus,13800000001,"
          "2026,6,1,14,30,UNPROCESSED\n", file);
    fputs("bad,row\n", file);
    fputs("1,FOUND,Duplicate,Card,Blue,Library,campus,13800000002,"
          "2026,6,1,15,30,UNPROCESSED\n", file);
    fputs("3,FOUND,Invalid date,Card,Blue,Library,campus,13800000003,"
          "2026,13,1,15,30,UNPROCESSED\n", file);
    fclose(file);

    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    ASSERT_EQ(STORAGE_OK, storage_load_items(
        MALFORMED_PATH, &lost_items, &found_items, &report));
    ASSERT_EQ(1, report.loaded_rows);
    ASSERT_EQ(3, report.skipped_rows);
    ASSERT_EQ(1, lost_items.size);
    ASSERT_EQ(0, found_items.size);

    item_list_clear(&lost_items);
    item_list_clear(&found_items);
    remove(MALFORMED_PATH);
}

static void test_empty_and_missing_files(void) {
    FILE *file = fopen(EMPTY_PATH, "w");
    ItemList lost_items;
    ItemList found_items;
    StorageReport report = {0};

    ASSERT_TRUE(file != NULL);
    if (file != NULL) {
        fclose(file);
    }
    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);

    ASSERT_EQ(STORAGE_OK, storage_load_items(
        EMPTY_PATH, &lost_items, &found_items, &report));
    ASSERT_EQ(0, report.loaded_rows);
    ASSERT_EQ(0, report.skipped_rows);
    ASSERT_EQ(STORAGE_OPEN_FAILED, storage_load_items(
        MISSING_PATH, &lost_items, &found_items, &report));

    remove(EMPTY_PATH);
}

int main(void) {
    test_save_and_load_round_trip();
    test_malformed_and_duplicate_rows_are_skipped();
    test_empty_and_missing_files();
    return test_finish();
}
