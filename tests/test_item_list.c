/* 物品链表测试：验证追加、查找、更新、删除和遍历逻辑。
 */

#include "item_list.h"
#include "test.h"

#include <string.h>

static Item make_item(int id, ItemType type, const char *name) {
    Item item = {0};

    item.id = id;
    item.type = type;
    strcpy(item.name, name);
    strcpy(item.category, "Card");
    strcpy(item.color, "Blue");
    strcpy(item.location, "Library");
    strcpy(item.keywords, "student campus");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void count_visit(const Item *item, void *context) {
    size_t *count = context;

    ASSERT_TRUE(item != NULL);
    (*count)++;
}

static void test_init_append_and_find(void) {
    ItemList list;
    Item first = make_item(1, ITEM_LOST, "Campus card");
    Item duplicate = make_item(1, ITEM_LOST, "Other");
    Item wrong_type = make_item(2, ITEM_FOUND, "Key");

    item_list_init(&list, ITEM_LOST);
    ASSERT_EQ(0, list.size);
    ASSERT_TRUE(list.head == NULL);

    ASSERT_EQ(LIST_OK, item_list_append(&list, &first));
    ASSERT_EQ(1, list.size);
    ASSERT_STR_EQ("Campus card", item_list_find_by_id(&list, 1)->name);
    ASSERT_EQ(LIST_DUPLICATE_ID, item_list_append(&list, &duplicate));
    ASSERT_EQ(LIST_WRONG_TYPE, item_list_append(&list, &wrong_type));
    ASSERT_TRUE(item_list_find_by_id(&list, 99) == NULL);

    item_list_clear(&list);
}

static void test_update_preserves_identity(void) {
    ItemList list;
    Item original = make_item(1, ITEM_LOST, "Campus card");
    Item replacement = make_item(99, ITEM_FOUND, "Student card");
    Item *updated;

    item_list_init(&list, ITEM_LOST);
    ASSERT_EQ(LIST_OK, item_list_append(&list, &original));
    ASSERT_EQ(LIST_OK, item_list_update(&list, 1, &replacement));

    updated = item_list_find_by_id(&list, 1);
    ASSERT_TRUE(updated != NULL);
    ASSERT_EQ(1, updated->id);
    ASSERT_EQ(ITEM_LOST, updated->type);
    ASSERT_STR_EQ("Student card", updated->name);
    ASSERT_EQ(LIST_NOT_FOUND, item_list_update(&list, 2, &replacement));

    item_list_clear(&list);
}

static void test_remove_head_middle_and_tail(void) {
    ItemList list;
    Item removed = {0};
    Item first = make_item(1, ITEM_FOUND, "First");
    Item second = make_item(2, ITEM_FOUND, "Second");
    Item third = make_item(3, ITEM_FOUND, "Third");

    item_list_init(&list, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&list, &first));
    ASSERT_EQ(LIST_OK, item_list_append(&list, &second));
    ASSERT_EQ(LIST_OK, item_list_append(&list, &third));

    ASSERT_EQ(LIST_OK, item_list_remove(&list, 2, &removed));
    ASSERT_EQ(2, removed.id);
    ASSERT_EQ(2, list.size);
    ASSERT_EQ(LIST_OK, item_list_remove(&list, 1, NULL));
    ASSERT_EQ(LIST_OK, item_list_remove(&list, 3, NULL));
    ASSERT_EQ(0, list.size);
    ASSERT_TRUE(list.head == NULL);
    ASSERT_EQ(LIST_NOT_FOUND, item_list_remove(&list, 99, NULL));
}

static void test_matching_queries(void) {
    ItemList list;
    Item first = make_item(1, ITEM_LOST, "Campus Card");
    Item second = make_item(2, ITEM_LOST, "Door Key");
    size_t visited = 0;

    strcpy(second.category, "Key");
    strcpy(second.location, "Dormitory");
    strcpy(second.keywords, "silver door");

    item_list_init(&list, ITEM_LOST);
    ASSERT_EQ(LIST_OK, item_list_append(&list, &first));
    ASSERT_EQ(LIST_OK, item_list_append(&list, &second));

    ASSERT_EQ(1, item_list_visit_matching(
        &list, ITEM_FIELD_NAME, "campus", count_visit, &visited));
    ASSERT_EQ(1, visited);
    visited = 0;
    ASSERT_EQ(1, item_list_visit_matching(
        &list, ITEM_FIELD_CATEGORY, "key", count_visit, &visited));
    ASSERT_EQ(1, visited);
    visited = 0;
    ASSERT_EQ(1, item_list_visit_matching(
        &list, ITEM_FIELD_LOCATION, "DORM", count_visit, &visited));
    ASSERT_EQ(1, visited);
    visited = 0;
    ASSERT_EQ(1, item_list_visit_matching(
        &list, ITEM_FIELD_KEYWORDS, "silver", count_visit, &visited));
    ASSERT_EQ(1, visited);

    item_list_clear(&list);
    ASSERT_EQ(0, list.size);
    ASSERT_TRUE(list.head == NULL);
}

int main(void) {
    test_init_append_and_find();
    test_update_preserves_identity();
    test_remove_head_middle_and_tail();
    test_matching_queries();
    return test_finish();
}
