/* 物品数据模型实现：负责枚举转换、日期校验和记录合法性检查。
 */

#include "item.h"

#include <string.h>

static int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int days_in_month(int year, int month) {
    static const int days[] = {
        0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month];
}

const char *item_type_to_string(ItemType type) {
    switch (type) {
        case ITEM_LOST:
            return "LOST";
        case ITEM_FOUND:
            return "FOUND";
        default:
            return "UNKNOWN";
    }
}

int item_type_from_string(const char *text, ItemType *type) {
    if (text == NULL || type == NULL) {
        return 0;
    }
    if (strcmp(text, "LOST") == 0) {
        *type = ITEM_LOST;
        return 1;
    }
    if (strcmp(text, "FOUND") == 0) {
        *type = ITEM_FOUND;
        return 1;
    }
    return 0;
}

const char *item_status_to_string(ItemStatus status) {
    switch (status) {
        case STATUS_UNPROCESSED:
            return "UNPROCESSED";
        case STATUS_MATCHED:
            return "MATCHED";
        case STATUS_COMPLETED:
            return "COMPLETED";
        default:
            return "UNKNOWN";
    }
}

int item_status_from_string(const char *text, ItemStatus *status) {
    if (text == NULL || status == NULL) {
        return 0;
    }
    if (strcmp(text, "UNPROCESSED") == 0) {
        *status = STATUS_UNPROCESSED;
        return 1;
    }
    if (strcmp(text, "MATCHED") == 0) {
        *status = STATUS_MATCHED;
        return 1;
    }
    if (strcmp(text, "COMPLETED") == 0) {
        *status = STATUS_COMPLETED;
        return 1;
    }
    return 0;
}

int item_is_valid(const Item *item) {
    int max_day;

    if (item == NULL || item->id <= 0) {
        return 0;
    }
    if (item->type != ITEM_LOST && item->type != ITEM_FOUND) {
        return 0;
    }
    if (item->status < STATUS_UNPROCESSED || item->status > STATUS_COMPLETED) {
        return 0;
    }
    if (item->name[0] == '\0' || item->category[0] == '\0' ||
        item->location[0] == '\0' || item->contact[0] == '\0') {
        return 0;
    }
    if (item->year < 2000 || item->year > 9999) {
        return 0;
    }
    max_day = days_in_month(item->year, item->month);
    if (item->day < 1 || item->day > max_day) {
        return 0;
    }
    if (item->hour < 0 || item->hour > 23 ||
        item->minute < 0 || item->minute > 59) {
        return 0;
    }
    return 1;
}
