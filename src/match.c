#include "match.h"

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

static size_t copy_lower_token(char *dest, size_t dest_size, const char **cursor) {
    size_t length = 0;

    while (**cursor != '\0' && !isalnum((unsigned char)**cursor)) {
        (*cursor)++;
    }
    while (**cursor != '\0' && isalnum((unsigned char)**cursor)) {
        if (length + 1 < dest_size) {
            dest[length++] = (char)ascii_tolower(**cursor);
        }
        (*cursor)++;
    }
    if (dest_size > 0) {
        dest[length] = '\0';
    }
    return length;
}

static int text_has_token(const char *text, const char *token) {
    char current[64];
    const char *cursor = text;

    if (text == NULL || token == NULL || token[0] == '\0') {
        return 0;
    }
    while (*cursor != '\0') {
        if (copy_lower_token(current, sizeof(current), &cursor) == 0) {
            break;
        }
        if (strcmp(current, token) == 0) {
            return 1;
        }
    }
    return 0;
}

static int texts_share_token(const char *left, const char *right) {
    char token[64];
    const char *cursor = left;

    if (left == NULL || right == NULL) {
        return 0;
    }
    while (*cursor != '\0') {
        size_t length = copy_lower_token(token, sizeof(token), &cursor);

        if (length == 0) {
            break;
        }
        if (length >= 2 && text_has_token(right, token)) {
            return 1;
        }
    }
    return 0;
}

static long item_minutes(const Item *item) {
    long days = (((long)item->year * 12L + item->month) * 31L) + item->day;

    return ((days * 24L + item->hour) * 60L) + item->minute;
}

static int times_are_close(const Item *lost, const Item *found) {
    long difference = item_minutes(lost) - item_minutes(found);

    if (difference < 0) {
        difference = -difference;
    }
    return difference <= 24L * 60L;
}

static void append_reason(char *reason, size_t reason_size, const char *text) {
    size_t used;
    size_t remaining;

    if (reason == NULL || reason_size == 0 || text == NULL) {
        return;
    }
    used = strlen(reason);
    if (used + 1 >= reason_size) {
        return;
    }
    if (used > 0) {
        remaining = reason_size - used - 1;
        strncat(reason, "; ", remaining);
        used = strlen(reason);
        if (used + 1 >= reason_size) {
            return;
        }
    }
    remaining = reason_size - used - 1;
    strncat(reason, text, remaining);
}

static MatchResult *match_result_new(
    int lost_id,
    int found_id,
    int score,
    const char *reason
) {
    MatchResult *result = malloc(sizeof(*result));

    if (result == NULL) {
        return NULL;
    }
    result->lost_id = lost_id;
    result->found_id = found_id;
    result->score = score;
    result->reason[0] = '\0';
    if (reason != NULL) {
        strncpy(result->reason, reason, sizeof(result->reason) - 1);
        result->reason[sizeof(result->reason) - 1] = '\0';
    }
    result->next = NULL;
    return result;
}

static void match_result_insert_sorted(MatchResultList *list, MatchResult *node) {
    MatchResult *current;

    if (list->head == NULL || node->score > list->head->score) {
        node->next = list->head;
        list->head = node;
        list->size++;
        return;
    }
    current = list->head;
    while (current->next != NULL && current->next->score >= node->score) {
        current = current->next;
    }
    node->next = current->next;
    current->next = node;
    list->size++;
}

void match_result_list_init(MatchResultList *list) {
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->size = 0;
}

int match_score_items(
    const Item *lost,
    const Item *found,
    char *reason,
    size_t reason_size
) {
    int score = 0;

    if (reason != NULL && reason_size > 0) {
        reason[0] = '\0';
    }
    if (lost == NULL || found == NULL ||
        lost->type != ITEM_LOST || found->type != ITEM_FOUND) {
        return 0;
    }
    if (strings_equal_case_insensitive(lost->category, found->category)) {
        score += 25;
        append_reason(reason, reason_size, "category +25");
    }
    if (texts_share_token(lost->name, found->name)) {
        score += 25;
        append_reason(reason, reason_size, "name +25");
    }
    if (lost->color[0] != '\0' &&
        strings_equal_case_insensitive(lost->color, found->color)) {
        score += 20;
        append_reason(reason, reason_size, "color +20");
    }
    if (strings_equal_case_insensitive(lost->location, found->location) ||
        texts_share_token(lost->location, found->location)) {
        score += 15;
        append_reason(reason, reason_size, "location +15");
    }
    if (times_are_close(lost, found)) {
        score += 10;
        append_reason(reason, reason_size, "time +10");
    }
    if (texts_share_token(lost->keywords, found->keywords)) {
        score += 5;
        append_reason(reason, reason_size, "keywords +5");
    }
    return score;
}

const char *match_level_to_string(int score) {
    if (score >= 80) {
        return "High similarity";
    }
    if (score >= 60) {
        return "Possible match";
    }
    if (score >= 40) {
        return "Low confidence";
    }
    return "Hidden";
}

int match_generate_results(
    const ItemList *lost_items,
    const ItemList *found_items,
    int minimum_score,
    MatchResultList *results
) {
    const Item *lost;

    if (lost_items == NULL || found_items == NULL || results == NULL) {
        return 0;
    }
    match_result_list_clear(results);
    lost = lost_items->head;
    while (lost != NULL) {
        const Item *found = found_items->head;

        while (found != NULL) {
            char reason[MATCH_REASON_LENGTH];
            int score = match_score_items(lost, found, reason, sizeof(reason));

            if (score >= minimum_score) {
                MatchResult *node = match_result_new(
                    lost->id, found->id, score, reason);

                if (node == NULL) {
                    match_result_list_clear(results);
                    return 0;
                }
                match_result_insert_sorted(results, node);
            }
            found = found->next;
        }
        lost = lost->next;
    }
    return 1;
}

void match_result_list_clear(MatchResultList *list) {
    MatchResult *current;

    if (list == NULL) {
        return;
    }
    current = list->head;
    while (current != NULL) {
        MatchResult *next = current->next;

        free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}
