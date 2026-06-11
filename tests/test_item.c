#include "item.h"
#include "test.h"

#include <string.h>

static Item make_valid_item(void) {
    Item item = {0};

    item.id = 1;
    item.type = ITEM_LOST;
    strcpy(item.name, "Campus card");
    strcpy(item.category, "Card");
    strcpy(item.color, "Blue");
    strcpy(item.location, "Library");
    strcpy(item.keywords, "student card");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void test_type_conversion(void) {
    ItemType type = ITEM_LOST;

    ASSERT_STR_EQ("LOST", item_type_to_string(ITEM_LOST));
    ASSERT_STR_EQ("FOUND", item_type_to_string(ITEM_FOUND));
    ASSERT_TRUE(item_type_from_string("FOUND", &type));
    ASSERT_EQ(ITEM_FOUND, type);
    ASSERT_FALSE(item_type_from_string("OTHER", &type));
}

static void test_status_conversion(void) {
    ItemStatus status = STATUS_UNPROCESSED;

    ASSERT_STR_EQ("UNPROCESSED", item_status_to_string(STATUS_UNPROCESSED));
    ASSERT_STR_EQ("MATCHED", item_status_to_string(STATUS_MATCHED));
    ASSERT_STR_EQ("COMPLETED", item_status_to_string(STATUS_COMPLETED));
    ASSERT_TRUE(item_status_from_string("COMPLETED", &status));
    ASSERT_EQ(STATUS_COMPLETED, status);
    ASSERT_FALSE(item_status_from_string("OTHER", &status));
}

static void test_item_validation(void) {
    Item item = make_valid_item();

    ASSERT_TRUE(item_is_valid(&item));

    item.id = 0;
    ASSERT_FALSE(item_is_valid(&item));
    item = make_valid_item();

    item.name[0] = '\0';
    ASSERT_FALSE(item_is_valid(&item));
    item = make_valid_item();

    item.month = 13;
    ASSERT_FALSE(item_is_valid(&item));
    item = make_valid_item();

    item.hour = 24;
    ASSERT_FALSE(item_is_valid(&item));
}

int main(void) {
    test_type_conversion();
    test_status_conversion();
    test_item_validation();
    return test_finish();
}
