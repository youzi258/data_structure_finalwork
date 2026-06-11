/* 哈希索引测试：验证类别查询、关键词 token 查询和空索引行为。
 */

#include "hash_index.h"
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
    strcpy(item.keywords, "campus card student");
    strcpy(item.contact, "13800000001");
    item.year = 2026;
    item.month = 6;
    item.day = 1;
    item.hour = 14;
    item.minute = 30;
    item.status = STATUS_UNPROCESSED;
    return item;
}

static void collect_id(const Item *item, void *context) {
    int *sum = context;

    ASSERT_TRUE(item != NULL);
    *sum += item->id;
}

static void test_category_lookup_is_case_insensitive(void) {
    ItemList list;
    HashIndex index;
    Item first = make_item(1, ITEM_FOUND, "Student card");
    Item second = make_item(2, ITEM_FOUND, "Door key");
    int id_sum = 0;

    strcpy(second.category, "Key");
    item_list_init(&list, ITEM_FOUND);
    ASSERT_EQ(LIST_OK, item_list_append(&list, &first));
    ASSERT_EQ(LIST_OK, item_list_append(&list, &second));
    ASSERT_TRUE(hash_index_init(&index, 5, HASH_INDEX_CATEGORY));
    ASSERT_TRUE(hash_index_build(&index, &list));

    ASSERT_EQ(1, hash_index_visit(&index, "card", collect_id, &id_sum));
    ASSERT_EQ(1, id_sum);

    hash_index_clear(&index);
    item_list_clear(&list);
}

static void test_keyword_lookup_handles_tokens_and_collisions(void) {
    ItemList list;
    HashIndex index;
    Item first = make_item(1, ITEM_LOST, "Campus card");
    Item second = make_item(2, ITEM_LOST, "Water bottle");
    int id_sum = 0;

    strcpy(second.keywords, "blue bottle campus");
    item_list_init(&list, ITEM_LOST);
    ASSERT_EQ(LIST_OK, item_list_append(&list, &first));
    ASSERT_EQ(LIST_OK, item_list_append(&list, &second));
    ASSERT_TRUE(hash_index_init(&index, 1, HASH_INDEX_KEYWORDS));
    ASSERT_TRUE(hash_index_build(&index, &list));

    ASSERT_EQ(2, hash_index_visit(&index, "CAMPUS", collect_id, &id_sum));
    ASSERT_EQ(3, id_sum);
    id_sum = 0;
    ASSERT_EQ(1, hash_index_visit(&index, "bottle", collect_id, &id_sum));
    ASSERT_EQ(2, id_sum);

    hash_index_clear(&index);
    item_list_clear(&list);
}

int main(void) {
    test_category_lookup_is_case_insensitive();
    test_keyword_lookup_handles_tokens_and_collisions();
    return test_finish();
}
