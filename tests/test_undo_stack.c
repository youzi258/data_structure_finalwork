#include "undo_stack.h"
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
    strcpy(item.keywords, "campus");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void test_empty_stack(void) {
    UndoStack stack;
    ItemList lost_items;
    ItemList found_items;

    undo_stack_init(&stack);
    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);

    ASSERT_EQ(UNDO_EMPTY, undo_stack_apply(&stack, &lost_items, &found_items));
    ASSERT_EQ(0, stack.size);
}

static void test_undo_add_removes_item(void) {
    UndoStack stack;
    ItemList lost_items;
    ItemList found_items;
    Item item = make_item(1, ITEM_LOST, "Campus card");

    undo_stack_init(&stack);
    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &item));
    ASSERT_TRUE(undo_stack_push_add(&stack, &item));

    ASSERT_EQ(UNDO_OK, undo_stack_apply(&stack, &lost_items, &found_items));
    ASSERT_TRUE(item_list_find_by_id(&lost_items, 1) == NULL);
    ASSERT_EQ(0, stack.size);
}

static void test_undo_delete_restores_item(void) {
    UndoStack stack;
    ItemList lost_items;
    ItemList found_items;
    Item item = make_item(2, ITEM_FOUND, "Key");
    Item removed = {0};

    undo_stack_init(&stack);
    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&found_items, &item));
    ASSERT_EQ(LIST_OK, item_list_remove(&found_items, 2, &removed));
    ASSERT_TRUE(undo_stack_push_delete(&stack, &removed));

    ASSERT_EQ(UNDO_OK, undo_stack_apply(&stack, &lost_items, &found_items));
    ASSERT_STR_EQ("Key", item_list_find_by_id(&found_items, 2)->name);
    item_list_clear(&found_items);
}

static void test_lifo_order(void) {
    UndoStack stack;
    ItemList lost_items;
    ItemList found_items;
    Item first = make_item(1, ITEM_LOST, "First");
    Item second = make_item(2, ITEM_LOST, "Second");

    undo_stack_init(&stack);
    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &first));
    ASSERT_TRUE(undo_stack_push_add(&stack, &first));
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &second));
    ASSERT_TRUE(undo_stack_push_add(&stack, &second));

    ASSERT_EQ(UNDO_OK, undo_stack_apply(&stack, &lost_items, &found_items));
    ASSERT_TRUE(item_list_find_by_id(&lost_items, 2) == NULL);
    ASSERT_TRUE(item_list_find_by_id(&lost_items, 1) != NULL);
    ASSERT_EQ(UNDO_OK, undo_stack_apply(&stack, &lost_items, &found_items));
    ASSERT_TRUE(item_list_find_by_id(&lost_items, 1) == NULL);

    undo_stack_clear(&stack);
}

int main(void) {
    test_empty_stack();
    test_undo_add_removes_item();
    test_undo_delete_restores_item();
    test_lifo_order();
    return test_finish();
}
