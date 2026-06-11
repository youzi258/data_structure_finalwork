/* CSV 存储模块实现：解析和写入物品记录，过滤无效或重复数据。
 */

#include "storage.h"

#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STORAGE_LINE_LENGTH 1024
#define STORAGE_FIELD_COUNT 14

static int parse_int(const char *text, int *value) {
    char *end;
    long parsed;

    if (text == NULL || value == NULL || text[0] == '\0') {
        return 0;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

static int copy_field(char *destination, size_t size, const char *source) {
    size_t length;

    if (destination == NULL || source == NULL || size == 0) {
        return 0;
    }
    length = strlen(source);
    if (length >= size || strchr(source, ',') != NULL ||
        strchr(source, '\n') != NULL || strchr(source, '\r') != NULL) {
        return 0;
    }
    memcpy(destination, source, length + 1);
    return 1;
}

static size_t split_csv_line(char *line, char **fields, size_t capacity) {
    char *cursor;
    size_t count = 1;

    if (line == NULL || fields == NULL || capacity == 0) {
        return 0;
    }
    fields[0] = line;
    cursor = line;
    while (*cursor != '\0') {
        if (*cursor == ',') {
            if (count >= capacity) {
                return capacity + 1;
            }
            *cursor = '\0';
            fields[count++] = cursor + 1;
        }
        cursor++;
    }
    return count;
}

static void trim_line_end(char *line) {
    size_t length;

    if (line == NULL) {
        return;
    }
    length = strlen(line);
    while (length > 0 &&
           (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        line[--length] = '\0';
    }
}

static int is_blank_line(const char *line) {
    if (line == NULL) {
        return 1;
    }
    while (*line != '\0') {
        if (!isspace((unsigned char)*line)) {
            return 0;
        }
        line++;
    }
    return 1;
}

static int parse_item_row(char *line, Item *item) {
    char *fields[STORAGE_FIELD_COUNT];
    size_t count;

    memset(item, 0, sizeof(*item));
    trim_line_end(line);
    count = split_csv_line(line, fields, STORAGE_FIELD_COUNT);
    if (count != STORAGE_FIELD_COUNT) {
        return 0;
    }
    if (!parse_int(fields[0], &item->id) ||
        !item_type_from_string(fields[1], &item->type) ||
        !copy_field(item->name, sizeof(item->name), fields[2]) ||
        !copy_field(item->category, sizeof(item->category), fields[3]) ||
        !copy_field(item->color, sizeof(item->color), fields[4]) ||
        !copy_field(item->location, sizeof(item->location), fields[5]) ||
        !copy_field(item->keywords, sizeof(item->keywords), fields[6]) ||
        !copy_field(item->contact, sizeof(item->contact), fields[7]) ||
        !parse_int(fields[8], &item->year) ||
        !parse_int(fields[9], &item->month) ||
        !parse_int(fields[10], &item->day) ||
        !parse_int(fields[11], &item->hour) ||
        !parse_int(fields[12], &item->minute) ||
        !item_status_from_string(fields[13], &item->status)) {
        return 0;
    }
    return item_is_valid(item);
}

/* 保存前再次检查字段，防止写出当前简化 CSV 格式无法解析的数据。 */
static int item_fields_are_writable(const Item *item) {
    if (!item_is_valid(item)) {
        return 0;
    }
    return strchr(item->name, ',') == NULL &&
           strchr(item->category, ',') == NULL &&
           strchr(item->color, ',') == NULL &&
           strchr(item->location, ',') == NULL &&
           strchr(item->keywords, ',') == NULL &&
           strchr(item->contact, ',') == NULL &&
           strchr(item->name, '\n') == NULL &&
           strchr(item->category, '\n') == NULL &&
           strchr(item->color, '\n') == NULL &&
           strchr(item->location, '\n') == NULL &&
           strchr(item->keywords, '\n') == NULL &&
           strchr(item->contact, '\n') == NULL;
}

static int write_item(FILE *file, const Item *item) {
    return fprintf(
        file,
        "%d,%s,%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%s\n",
        item->id,
        item_type_to_string(item->type),
        item->name,
        item->category,
        item->color,
        item->location,
        item->keywords,
        item->contact,
        item->year,
        item->month,
        item->day,
        item->hour,
        item->minute,
        item_status_to_string(item->status)
    ) >= 0;
}

static StorageResult write_list(FILE *file, const ItemList *list) {
    const Item *current = list->head;

    while (current != NULL) {
        if (!item_fields_are_writable(current)) {
            return STORAGE_INVALID_DATA;
        }
        if (!write_item(file, current)) {
            return STORAGE_WRITE_FAILED;
        }
        current = current->next;
    }
    return STORAGE_OK;
}

StorageResult storage_save_items(
    const char *path,
    const ItemList *lost_items,
    const ItemList *found_items
) {
    FILE *file;
    StorageResult result;

    if (path == NULL || lost_items == NULL || found_items == NULL) {
        return STORAGE_INVALID_ARGUMENT;
    }
    file = fopen(path, "w");
    if (file == NULL) {
        return STORAGE_OPEN_FAILED;
    }
    if (fprintf(file, "%s\n", STORAGE_HEADER) < 0) {
        fclose(file);
        return STORAGE_WRITE_FAILED;
    }
    result = write_list(file, lost_items);
    if (result == STORAGE_OK) {
        result = write_list(file, found_items);
    }
    if (fclose(file) != 0 && result == STORAGE_OK) {
        result = STORAGE_WRITE_FAILED;
    }
    return result;
}

static int id_exists(
    const ItemList *lost_items,
    const ItemList *found_items,
    int id
) {
    return item_list_find_by_id_const(lost_items, id) != NULL ||
           item_list_find_by_id_const(found_items, id) != NULL;
}

static StorageResult add_loaded_item(
    ItemList *lost_items,
    ItemList *found_items,
    const Item *item
) {
    ItemList *target;
    ListResult result;

    if (id_exists(lost_items, found_items, item->id)) {
        return STORAGE_INVALID_DATA;
    }
    target = item->type == ITEM_LOST ? lost_items : found_items;
    result = item_list_append(target, item);
    if (result == LIST_OUT_OF_MEMORY) {
        return STORAGE_OUT_OF_MEMORY;
    }
    return result == LIST_OK ? STORAGE_OK : STORAGE_INVALID_DATA;
}

static void consume_line_remainder(FILE *file) {
    int character;

    do {
        character = fgetc(file);
    } while (character != '\n' && character != EOF);
}

StorageResult storage_load_items(
    const char *path,
    ItemList *lost_items,
    ItemList *found_items,
    StorageReport *report
) {
    FILE *file;
    char line[STORAGE_LINE_LENGTH];
    ItemList loaded_lost;
    ItemList loaded_found;
    StorageReport current_report = {0};
    int first_line = 1;

    if (path == NULL || lost_items == NULL || found_items == NULL) {
        return STORAGE_INVALID_ARGUMENT;
    }
    file = fopen(path, "r");
    if (file == NULL) {
        return STORAGE_OPEN_FAILED;
    }
    /* 先读入临时链表，全部处理完后再替换调用方列表，避免半加载状态。 */
    item_list_init(&loaded_lost, ITEM_LOST);
    item_list_init(&loaded_found, ITEM_FOUND);

    while (fgets(line, sizeof(line), file) != NULL) {
        Item item;
        StorageResult add_result;
        int complete_line = strchr(line, '\n') != NULL || feof(file);

        if (!complete_line) {
            consume_line_remainder(file);
            current_report.skipped_rows++;
            first_line = 0;
            continue;
        }
        trim_line_end(line);
        if (is_blank_line(line)) {
            first_line = 0;
            continue;
        }
        if (first_line && strcmp(line, STORAGE_HEADER) == 0) {
            first_line = 0;
            continue;
        }
        first_line = 0;
        if (!parse_item_row(line, &item)) {
            current_report.skipped_rows++;
            continue;
        }
        add_result = add_loaded_item(&loaded_lost, &loaded_found, &item);
        if (add_result == STORAGE_OUT_OF_MEMORY) {
            item_list_clear(&loaded_lost);
            item_list_clear(&loaded_found);
            fclose(file);
            return STORAGE_OUT_OF_MEMORY;
        }
        if (add_result != STORAGE_OK) {
            current_report.skipped_rows++;
            continue;
        }
        current_report.loaded_rows++;
    }
    if (ferror(file)) {
        item_list_clear(&loaded_lost);
        item_list_clear(&loaded_found);
        fclose(file);
        return STORAGE_READ_FAILED;
    }
    fclose(file);

    item_list_clear(lost_items);
    item_list_clear(found_items);
    *lost_items = loaded_lost;
    *found_items = loaded_found;
    if (report != NULL) {
        *report = current_report;
    }
    return STORAGE_OK;
}
