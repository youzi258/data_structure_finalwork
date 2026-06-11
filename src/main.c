#include "hash_index.h"
#include "input.h"
#include "item_list.h"
#include "match.h"
#include "statistics.h"
#include "storage.h"
#include "undo_stack.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    ItemList lost_items;
    ItemList found_items;
    UndoStack undo_stack;
} Application;

static void print_item(const Item *item, void *context) {
    (void)context;
    printf(
        "ID: %d | Type: %s | Name: %s | Category: %s | Color: %s\n"
        "Location: %s | Keywords: %s | Contact: %s\n"
        "Time: %04d-%02d-%02d %02d:%02d | Status: %s\n",
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
    );
}

static void print_list(const ItemList *list, const char *title) {
    const Item *current;

    printf("\n=== %s (%zu) ===\n", title, list->size);
    if (list->head == NULL) {
        puts("No records.");
        return;
    }
    current = list->head;
    while (current != NULL) {
        print_item(current, NULL);
        current = current->next;
    }
}

static ItemList *find_item_list(Application *app, int id) {
    if (item_list_find_by_id(&app->lost_items, id) != NULL) {
        return &app->lost_items;
    }
    if (item_list_find_by_id(&app->found_items, id) != NULL) {
        return &app->found_items;
    }
    return NULL;
}

static const Item *find_item(const Application *app, int id) {
    const Item *item = item_list_find_by_id_const(&app->lost_items, id);

    if (item != NULL) {
        return item;
    }
    return item_list_find_by_id_const(&app->found_items, id);
}

static int read_item_fields(Item *item, int include_status) {
    int status_choice;

    if (!input_read_text("Name: ", item->name, sizeof(item->name), 0) ||
        !input_read_text(
            "Category: ", item->category, sizeof(item->category), 0) ||
        !input_read_text("Color (may be empty): ",
                         item->color, sizeof(item->color), 1) ||
        !input_read_text(
            "Location: ", item->location, sizeof(item->location), 0) ||
        !input_read_text("Keywords (may be empty): ",
                         item->keywords, sizeof(item->keywords), 1) ||
        !input_read_text(
            "Contact: ", item->contact, sizeof(item->contact), 0) ||
        !input_read_int("Year: ", 2000, 9999, &item->year) ||
        !input_read_int("Month: ", 1, 12, &item->month) ||
        !input_read_int("Day: ", 1, 31, &item->day) ||
        !input_read_int("Hour: ", 0, 23, &item->hour) ||
        !input_read_int("Minute: ", 0, 59, &item->minute)) {
        return 0;
    }
    if (include_status) {
        puts("Status: 1 UNPROCESSED, 2 MATCHED, 3 COMPLETED");
        if (!input_read_int("Status: ", 1, 3, &status_choice)) {
            return 0;
        }
        item->status = (ItemStatus)(status_choice - 1);
    } else {
        item->status = STATUS_UNPROCESSED;
    }
    if (!item_is_valid(item)) {
        puts("The item fields do not form a valid record.");
        return 0;
    }
    return 1;
}

static void add_item(Application *app, ItemType type) {
    Item item = {0};
    ItemList *list = type == ITEM_LOST
        ? &app->lost_items
        : &app->found_items;
    ListResult result;

    if (!input_read_int("ID: ", 1, INT_MAX, &item.id)) {
        return;
    }
    if (find_item(app, item.id) != NULL) {
        puts("That ID already exists.");
        return;
    }
    item.type = type;
    if (!read_item_fields(&item, 0)) {
        return;
    }
    result = item_list_append(list, &item);
    if (result != LIST_OK) {
        puts("Could not add the item.");
        return;
    }
    if (!undo_stack_push_add(&app->undo_stack, &item)) {
        item_list_remove(list, item.id, NULL);
        puts("Could not record undo; the add was rolled back.");
        return;
    }
    puts("Item added.");
}

static void update_item(Application *app) {
    int id;
    ItemList *list;
    Item replacement;

    if (!input_read_int("ID to update: ", 1, INT_MAX, &id)) {
        return;
    }
    list = find_item_list(app, id);
    if (list == NULL) {
        puts("Item not found.");
        return;
    }
    replacement = *item_list_find_by_id(list, id);
    replacement.next = NULL;
    puts("Enter the replacement fields.");
    if (!read_item_fields(&replacement, 1)) {
        return;
    }
    if (item_list_update(list, id, &replacement) == LIST_OK) {
        puts("Item updated.");
    } else {
        puts("Could not update the item.");
    }
}

static void delete_item(Application *app) {
    int id;
    ItemList *list;
    Item removed = {0};

    if (!input_read_int("ID to delete: ", 1, INT_MAX, &id)) {
        return;
    }
    list = find_item_list(app, id);
    if (list == NULL) {
        puts("Item not found.");
        return;
    }
    if (item_list_remove(list, id, &removed) != LIST_OK) {
        puts("Could not delete the item.");
        return;
    }
    if (!undo_stack_push_delete(&app->undo_stack, &removed)) {
        item_list_append(list, &removed);
        puts("Could not record undo; the delete was rolled back.");
        return;
    }
    puts("Item deleted.");
}

static void find_item_by_id(const Application *app) {
    int id;
    const Item *item;

    if (!input_read_int("ID to find: ", 1, INT_MAX, &id)) {
        return;
    }
    item = find_item(app, id);
    if (item == NULL) {
        puts("Item not found.");
        return;
    }
    print_item(item, NULL);
}

static void search_items(const Application *app) {
    int choice;
    ItemField field;
    char query[ITEM_KEYWORDS_LENGTH];
    size_t count;

    puts("Search field: 1 name, 2 category, 3 location, 4 keywords");
    if (!input_read_int("Field: ", 1, 4, &choice) ||
        !input_read_text("Query: ", query, sizeof(query), 0)) {
        return;
    }
    field = (ItemField)(choice - 1);
    count = item_list_visit_matching(
        &app->lost_items, field, query, print_item, NULL);
    count += item_list_visit_matching(
        &app->found_items, field, query, print_item, NULL);
    printf("Matched records: %zu\n", count);
}

static void undo_last_operation(Application *app) {
    UndoResult result = undo_stack_apply(
        &app->undo_stack, &app->lost_items, &app->found_items);

    if (result == UNDO_OK) {
        puts("Last add/delete operation undone.");
    } else if (result == UNDO_EMPTY) {
        puts("There is no operation to undo.");
    } else {
        puts("Could not undo the operation.");
    }
}

static void load_items(Application *app) {
    char path[260];
    StorageReport report = {0};
    StorageResult result;

    if (!input_read_text("CSV path: ", path, sizeof(path), 0)) {
        return;
    }
    result = storage_load_items(
        path, &app->lost_items, &app->found_items, &report);
    if (result != STORAGE_OK) {
        puts("Could not load the CSV file.");
        return;
    }
    undo_stack_clear(&app->undo_stack);
    printf("Loaded %zu row(s); skipped %zu invalid row(s).\n",
           report.loaded_rows, report.skipped_rows);
}

static int save_items(const Application *app) {
    char path[260];

    if (!input_read_text("CSV path: ", path, sizeof(path), 0)) {
        return 0;
    }
    if (storage_save_items(path, &app->lost_items, &app->found_items)
        != STORAGE_OK) {
        puts("Could not save the CSV file.");
        return 0;
    }
    puts("Items saved.");
    return 1;
}

static void show_match_results(const Application *app) {
    MatchResultList results;
    const MatchResult *current;

    match_result_list_init(&results);
    if (!match_generate_results(
            &app->lost_items,
            &app->found_items,
            MATCH_MINIMUM_SCORE,
            &results)) {
        puts("Could not generate match results.");
        return;
    }
    printf("\n=== Smart Match Results (%zu) ===\n", results.size);
    if (results.head == NULL) {
        puts("No match results reached the display threshold.");
    }
    current = results.head;
    while (current != NULL) {
        const Item *lost = item_list_find_by_id_const(
            &app->lost_items, current->lost_id);
        const Item *found = item_list_find_by_id_const(
            &app->found_items, current->found_id);

        printf(
            "Lost #%d%s%s <-> Found #%d%s%s | Score: %d | %s\n"
            "Reason: %s\n",
            current->lost_id,
            lost == NULL ? "" : " ",
            lost == NULL ? "" : lost->name,
            current->found_id,
            found == NULL ? "" : " ",
            found == NULL ? "" : found->name,
            current->score,
            match_level_to_string(current->score),
            current->reason
        );
        current = current->next;
    }
    match_result_list_clear(&results);
}

static const ItemList *select_item_list(const Application *app) {
    int choice;

    puts("List: 1 lost items, 2 found items");
    if (!input_read_int("List: ", 1, 2, &choice)) {
        return NULL;
    }
    return choice == 1 ? &app->lost_items : &app->found_items;
}

static void hash_search_items(const Application *app, HashIndexMode mode) {
    const ItemList *list = select_item_list(app);
    HashIndex index;
    char query[ITEM_KEYWORDS_LENGTH];
    size_t matches;

    if (list == NULL) {
        return;
    }
    if (!input_read_text("Query: ", query, sizeof(query), 0)) {
        return;
    }
    if (!hash_index_init(&index, 97, mode)) {
        puts("Could not create hash index.");
        return;
    }
    if (!hash_index_build(&index, list)) {
        hash_index_clear(&index);
        puts("Could not build hash index.");
        return;
    }
    matches = hash_index_visit(&index, query, print_item, NULL);
    printf("Hash matched records: %zu\n", matches);
    hash_index_clear(&index);
}

static void show_location_statistics(const Application *app) {
    LocationStatsList stats;
    const LocationStat *current;

    location_stats_list_init(&stats);
    if (!location_stats_build(&app->lost_items, &stats)) {
        puts("Could not build location statistics.");
        return;
    }
    printf("\n=== Lost Item Location Statistics (%zu) ===\n", stats.size);
    if (stats.head == NULL) {
        puts("No lost item records.");
    }
    current = stats.head;
    while (current != NULL) {
        printf("%s: %zu\n", current->location, current->count);
        current = current->next;
    }
    location_stats_list_clear(&stats);
}

static void show_member_b_menu(void) {
    puts(
        "\n=== Member B Features ===\n"
        "1  Generate smart match results\n"
        "2  Hash search by category\n"
        "3  Hash search by keyword\n"
        "4  Show lost item location statistics\n"
        "0  Back"
    );
}

static void member_b_menu(const Application *app) {
    int running = 1;

    while (running) {
        int choice;

        show_member_b_menu();
        if (!input_read_int("Choice: ", 0, 4, &choice)) {
            break;
        }
        switch (choice) {
            case 1:
                show_match_results(app);
                break;
            case 2:
                hash_search_items(app, HASH_INDEX_CATEGORY);
                break;
            case 3:
                hash_search_items(app, HASH_INDEX_KEYWORDS);
                break;
            case 4:
                show_location_statistics(app);
                break;
            case 0:
                running = 0;
                break;
            default:
                break;
        }
    }
}

static void show_menu(void) {
    puts(
        "\n=== Campus Lost And Found ===\n"
        "1  Add lost item\n"
        "2  Add found item\n"
        "3  Show lost items\n"
        "4  Show found items\n"
        "5  Update item\n"
        "6  Delete item\n"
        "7  Find item by ID\n"
        "8  Search items\n"
        "9  Undo last add/delete\n"
        "10 Load items from CSV\n"
        "11 Save items to CSV\n"
        "12 Member B feature menu\n"
        "0  Exit"
    );
}

static int handle_choice(Application *app, int choice) {
    int save_before_exit;

    switch (choice) {
        case 1:
            add_item(app, ITEM_LOST);
            break;
        case 2:
            add_item(app, ITEM_FOUND);
            break;
        case 3:
            print_list(&app->lost_items, "Lost Items");
            break;
        case 4:
            print_list(&app->found_items, "Found Items");
            break;
        case 5:
            update_item(app);
            break;
        case 6:
            delete_item(app);
            break;
        case 7:
            find_item_by_id(app);
            break;
        case 8:
            search_items(app);
            break;
        case 9:
            undo_last_operation(app);
            break;
        case 10:
            load_items(app);
            break;
        case 11:
            save_items(app);
            break;
        case 12:
            member_b_menu(app);
            break;
        case 0:
            if (!input_read_yes_no("Save before exit? (y/n): ",
                                   &save_before_exit)) {
                return 0;
            }
            if (save_before_exit && !save_items(app)) {
                puts("Exit cancelled because save did not complete.");
                break;
            }
            return 0;
        default:
            break;
    }
    return 1;
}

int main(void) {
    Application app;
    int running = 1;

    item_list_init(&app.lost_items, ITEM_LOST);
    item_list_init(&app.found_items, ITEM_FOUND);
    undo_stack_init(&app.undo_stack);

    while (running) {
        int choice;

        show_menu();
        if (!input_read_int("Choice: ", 0, 12, &choice)) {
            break;
        }
        running = handle_choice(&app, choice);
    }

    undo_stack_clear(&app.undo_stack);
    item_list_clear(&app.lost_items);
    item_list_clear(&app.found_items);
    puts("Goodbye.");
    return 0;
}
