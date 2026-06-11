#ifndef MATCH_H
#define MATCH_H

#include "item_list.h"

#include <stddef.h>

#define MATCH_REASON_LENGTH 256
#define MATCH_MINIMUM_SCORE 40

typedef struct MatchResult {
    int lost_id;
    int found_id;
    int score;
    char reason[MATCH_REASON_LENGTH];
    struct MatchResult *next;
} MatchResult;

typedef struct {
    MatchResult *head;
    size_t size;
} MatchResultList;

void match_result_list_init(MatchResultList *list);
int match_score_items(
    const Item *lost,
    const Item *found,
    char *reason,
    size_t reason_size
);
const char *match_level_to_string(int score);
int match_generate_results(
    const ItemList *lost_items,
    const ItemList *found_items,
    int minimum_score,
    MatchResultList *results
);
void match_result_list_clear(MatchResultList *list);

#endif
