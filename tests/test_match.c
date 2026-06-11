#include "item_list.h"
#include "match.h"
#include "test.h"

#include <string.h>

static Item make_item(int id, ItemType type, const char *name) {
    Item item = {0};

    item.id = id;
    item.type = type;
    strcpy(item.name, name);
    strcpy(item.category, "Card");
    strcpy(item.color, "Blue");
    strcpy(item.location, "Library second floor");
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

static void test_scores_full_match_with_reasons(void) {
    Item lost = make_item(1, ITEM_LOST, "Campus card");
    Item found = make_item(2, ITEM_FOUND, "Student card");
    char reason[MATCH_REASON_LENGTH];
    int score;

    strcpy(found.location, "Library gate");
    found.hour = 15;
    found.minute = 0;

    score = match_score_items(&lost, &found, reason, sizeof(reason));

    ASSERT_EQ(100, score);
    ASSERT_TRUE(strstr(reason, "category +25") != NULL);
    ASSERT_TRUE(strstr(reason, "name +25") != NULL);
    ASSERT_TRUE(strstr(reason, "color +20") != NULL);
    ASSERT_TRUE(strstr(reason, "location +15") != NULL);
    ASSERT_TRUE(strstr(reason, "time +10") != NULL);
    ASSERT_TRUE(strstr(reason, "keywords +5") != NULL);
    ASSERT_STR_EQ("High similarity", match_level_to_string(score));
}

static void test_generates_thresholded_sorted_results(void) {
    ItemList lost_items;
    ItemList found_items;
    MatchResultList results;
    Item lost = make_item(1, ITEM_LOST, "Campus card");
    Item strong = make_item(2, ITEM_FOUND, "Student card");
    Item weak = make_item(3, ITEM_FOUND, "Black umbrella");
    Item medium = make_item(4, ITEM_FOUND, "Blue card");

    strcpy(strong.location, "Library gate");
    strong.hour = 15;
    strcpy(weak.category, "Umbrella");
    strcpy(weak.color, "Black");
    strcpy(weak.location, "Canteen");
    strcpy(weak.keywords, "rain umbrella");
    weak.hour = 18;
    strcpy(medium.location, "Teaching building");
    strcpy(medium.keywords, "campus");
    medium.hour = 18;

    item_list_init(&lost_items, ITEM_LOST);
    item_list_init(&found_items, ITEM_FOUND);
    match_result_list_init(&results);
    ASSERT_EQ(LIST_OK, item_list_append(&lost_items, &lost));
    ASSERT_EQ(LIST_OK, item_list_append(&found_items, &weak));
    ASSERT_EQ(LIST_OK, item_list_append(&found_items, &medium));
    ASSERT_EQ(LIST_OK, item_list_append(&found_items, &strong));

    ASSERT_TRUE(match_generate_results(
        &lost_items, &found_items, MATCH_MINIMUM_SCORE, &results));
    ASSERT_EQ(2, results.size);
    ASSERT_TRUE(results.head != NULL);
    ASSERT_EQ(2, results.head->found_id);
    ASSERT_TRUE(results.head->score >= results.head->next->score);
    ASSERT_EQ(4, results.head->next->found_id);

    match_result_list_clear(&results);
    item_list_clear(&lost_items);
    item_list_clear(&found_items);
}

int main(void) {
    test_scores_full_match_with_reasons();
    test_generates_thresholded_sorted_results();
    return test_finish();
}
