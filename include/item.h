#ifndef ITEM_H
#define ITEM_H

#define ITEM_NAME_LENGTH 64
#define ITEM_CATEGORY_LENGTH 64
#define ITEM_COLOR_LENGTH 32
#define ITEM_LOCATION_LENGTH 128
#define ITEM_KEYWORDS_LENGTH 256
#define ITEM_CONTACT_LENGTH 64

typedef enum {
    ITEM_LOST = 0,
    ITEM_FOUND = 1
} ItemType;

typedef enum {
    STATUS_UNPROCESSED = 0,
    STATUS_MATCHED = 1,
    STATUS_COMPLETED = 2
} ItemStatus;

typedef struct Item {
    int id;
    ItemType type;
    char name[ITEM_NAME_LENGTH];
    char category[ITEM_CATEGORY_LENGTH];
    char color[ITEM_COLOR_LENGTH];
    char location[ITEM_LOCATION_LENGTH];
    char keywords[ITEM_KEYWORDS_LENGTH];
    char contact[ITEM_CONTACT_LENGTH];
    int year;
    int month;
    int day;
    int hour;
    int minute;
    ItemStatus status;
    struct Item *next;
} Item;

const char *item_type_to_string(ItemType type);
int item_type_from_string(const char *text, ItemType *type);
const char *item_status_to_string(ItemStatus status);
int item_status_from_string(const char *text, ItemStatus *status);
int item_is_valid(const Item *item);

#endif

