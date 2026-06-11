/* 程序入口和控制台菜单：整合成员 A 基础功能与成员 B 扩展功能。
 */

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

#ifdef _WIN32
#include <windows.h>
#endif

typedef struct {
    ItemList lost_items;
    ItemList found_items;
    UndoStack undo_stack;
} Application;

static void configure_console_encoding(void) {
#ifdef _WIN32
    /* 中文界面使用 GBK 代码页，配合 Makefile 的 -fexec-charset=GBK。 */
    SetConsoleCP(936);
    SetConsoleOutputCP(936);
#endif
}

/* CSV 中仍使用 LOST/FOUND 等英文枚举，界面显示时单独转换为中文。 */
static const char *item_type_display(ItemType type) {
    return type == ITEM_LOST ? "失物" : "拾物";
}

static const char *item_status_display(ItemStatus status) {
    switch (status) {
        case STATUS_UNPROCESSED:
            return "未处理";
        case STATUS_MATCHED:
            return "已匹配";
        case STATUS_COMPLETED:
            return "已完成";
        default:
            return "未知";
    }
}

static void print_item(const Item *item, void *context) {
    (void)context;
    printf(
        "编号：%d | 类型：%s | 名称：%s | 类别：%s | 颜色：%s\n"
        "地点：%s | 关键词：%s | 联系方式：%s\n"
        "时间：%04d-%02d-%02d %02d:%02d | 状态：%s\n",
        item->id,
        item_type_display(item->type),
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
        item_status_display(item->status)
    );
}

static void print_list(const ItemList *list, const char *title) {
    const Item *current;

    printf("\n=== %s (%zu) ===\n", title, list->size);
    if (list->head == NULL) {
        puts("暂无记录。");
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

/* 失物和拾物共用同一套字段读取逻辑，避免两个录入流程重复。 */
static int read_item_fields(Item *item, int include_status) {
    int status_choice;

    if (!input_read_text("名称：", item->name, sizeof(item->name), 0) ||
        !input_read_text(
            "类别：", item->category, sizeof(item->category), 0) ||
        !input_read_text("颜色（可留空）：",
                         item->color, sizeof(item->color), 1) ||
        !input_read_text(
            "地点：", item->location, sizeof(item->location), 0) ||
        !input_read_text("关键词（可留空）：",
                         item->keywords, sizeof(item->keywords), 1) ||
        !input_read_text(
            "联系方式：", item->contact, sizeof(item->contact), 0) ||
        !input_read_int("年份：", 2000, 9999, &item->year) ||
        !input_read_int("月份：", 1, 12, &item->month) ||
        !input_read_int("日期：", 1, 31, &item->day) ||
        !input_read_int("小时：", 0, 23, &item->hour) ||
        !input_read_int("分钟：", 0, 59, &item->minute)) {
        return 0;
    }
    if (include_status) {
        puts("状态：1 未处理，2 已匹配，3 已完成");
        if (!input_read_int("请选择状态：", 1, 3, &status_choice)) {
            return 0;
        }
        item->status = (ItemStatus)(status_choice - 1);
    } else {
        item->status = STATUS_UNPROCESSED;
    }
    if (!item_is_valid(item)) {
        puts("物品字段不完整或日期时间无效。");
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

    if (!input_read_int("编号：", 1, INT_MAX, &item.id)) {
        return;
    }
    if (find_item(app, item.id) != NULL) {
        puts("该编号已经存在。");
        return;
    }
    item.type = type;
    if (!read_item_fields(&item, 0)) {
        return;
    }
    result = item_list_append(list, &item);
    if (result != LIST_OK) {
        puts("添加记录失败。");
        return;
    }
    /* 数据写入成功后再压栈；压栈失败时回滚，保持数据和撤销栈一致。 */
    if (!undo_stack_push_add(&app->undo_stack, &item)) {
        item_list_remove(list, item.id, NULL);
        puts("无法记录撤销操作，本次添加已回滚。");
        return;
    }
    puts("记录添加成功。");
}

static void update_item(Application *app) {
    int id;
    ItemList *list;
    Item replacement;

    if (!input_read_int("请输入要修改的编号：", 1, INT_MAX, &id)) {
        return;
    }
    list = find_item_list(app, id);
    if (list == NULL) {
        puts("未找到该记录。");
        return;
    }
    replacement = *item_list_find_by_id(list, id);
    replacement.next = NULL;
    puts("请输入修改后的字段：");
    if (!read_item_fields(&replacement, 1)) {
        return;
    }
    if (item_list_update(list, id, &replacement) == LIST_OK) {
        puts("记录修改成功。");
    } else {
        puts("记录修改失败。");
    }
}

static void delete_item(Application *app) {
    int id;
    ItemList *list;
    Item removed = {0};

    if (!input_read_int("请输入要删除的编号：", 1, INT_MAX, &id)) {
        return;
    }
    list = find_item_list(app, id);
    if (list == NULL) {
        puts("未找到该记录。");
        return;
    }
    if (item_list_remove(list, id, &removed) != LIST_OK) {
        puts("记录删除失败。");
        return;
    }
    if (!undo_stack_push_delete(&app->undo_stack, &removed)) {
        item_list_append(list, &removed);
        puts("无法记录撤销操作，本次删除已回滚。");
        return;
    }
    puts("记录删除成功。");
}

static void find_item_by_id(const Application *app) {
    int id;
    const Item *item;

    if (!input_read_int("请输入要查询的编号：", 1, INT_MAX, &id)) {
        return;
    }
    item = find_item(app, id);
    if (item == NULL) {
        puts("未找到该记录。");
        return;
    }
    print_item(item, NULL);
}

static void search_items(const Application *app) {
    int choice;
    ItemField field;
    char query[ITEM_KEYWORDS_LENGTH];
    size_t count;

    puts("查询字段：1 名称，2 类别，3 地点，4 关键词");
    if (!input_read_int("请选择字段：", 1, 4, &choice) ||
        !input_read_text("请输入查询内容：", query, sizeof(query), 0)) {
        return;
    }
    field = (ItemField)(choice - 1);
    count = item_list_visit_matching(
        &app->lost_items, field, query, print_item, NULL);
    count += item_list_visit_matching(
        &app->found_items, field, query, print_item, NULL);
    printf("匹配记录数：%zu\n", count);
}

static void undo_last_operation(Application *app) {
    UndoResult result = undo_stack_apply(
        &app->undo_stack, &app->lost_items, &app->found_items);

    if (result == UNDO_OK) {
        puts("已撤销最近一次新增或删除操作。");
    } else if (result == UNDO_EMPTY) {
        puts("当前没有可撤销的操作。");
    } else {
        puts("撤销操作失败。");
    }
}

static void load_items(Application *app) {
    char path[260];
    StorageReport report = {0};
    StorageResult result;

    if (!input_read_text("CSV 文件路径：", path, sizeof(path), 0)) {
        return;
    }
    result = storage_load_items(
        path, &app->lost_items, &app->found_items, &report);
    if (result != STORAGE_OK) {
        puts("无法加载 CSV 文件。");
        return;
    }
    undo_stack_clear(&app->undo_stack);
    printf("已加载 %zu 条记录；跳过 %zu 条无效记录。\n",
           report.loaded_rows, report.skipped_rows);
}

static int save_items(const Application *app) {
    char path[260];

    if (!input_read_text("CSV 文件路径：", path, sizeof(path), 0)) {
        return 0;
    }
    if (storage_save_items(path, &app->lost_items, &app->found_items)
        != STORAGE_OK) {
        puts("无法保存 CSV 文件。");
        return 0;
    }
    puts("数据保存成功。");
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
        puts("无法生成智能匹配结果。");
        return;
    }
    printf("\n=== 智能匹配结果（%zu 条）===\n", results.size);
    if (results.head == NULL) {
        puts("没有达到显示阈值的匹配结果。");
    }
    current = results.head;
    while (current != NULL) {
        const Item *lost = item_list_find_by_id_const(
            &app->lost_items, current->lost_id);
        const Item *found = item_list_find_by_id_const(
            &app->found_items, current->found_id);

        printf(
            "失物 #%d%s%s <-> 拾物 #%d%s%s | 得分：%d | %s\n"
            "原因：%s\n",
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

/* 成员 B 的功能只读取成员 A 的链表，不接管链表节点所有权。 */
static const ItemList *select_item_list(const Application *app) {
    int choice;

    puts("请选择列表：1 失物，2 拾物");
    if (!input_read_int("列表：", 1, 2, &choice)) {
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
    if (!input_read_text("请输入查询内容：", query, sizeof(query), 0)) {
        return;
    }
    if (!hash_index_init(&index, 97, mode)) {
        puts("无法创建哈希索引。");
        return;
    }
    if (!hash_index_build(&index, list)) {
        hash_index_clear(&index);
        puts("无法构建哈希索引。");
        return;
    }
    matches = hash_index_visit(&index, query, print_item, NULL);
    printf("哈希查询命中记录数：%zu\n", matches);
    hash_index_clear(&index);
}

static void show_location_statistics(const Application *app) {
    LocationStatsList stats;
    const LocationStat *current;

    location_stats_list_init(&stats);
    if (!location_stats_build(&app->lost_items, &stats)) {
        puts("无法生成地点统计。");
        return;
    }
    printf("\n=== 失物高发地点统计（%zu 个地点）===\n", stats.size);
    if (stats.head == NULL) {
        puts("暂无失物记录。");
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
        "\n=== 智能匹配与扩展功能 ===\n"
        "1  生成智能匹配结果\n"
        "2  按类别进行哈希查询\n"
        "3  按关键词进行哈希查询\n"
        "4  查看失物高发地点统计\n"
        "0  返回主菜单"
    );
}

static void member_b_menu(const Application *app) {
    int running = 1;

    while (running) {
        int choice;

        show_member_b_menu();
        if (!input_read_int("请选择：", 0, 4, &choice)) {
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
        "\n=== 校园失物招领智能匹配系统 ===\n"
        "1  录入失物信息\n"
        "2  录入拾物信息\n"
        "3  查看失物列表\n"
        "4  查看拾物列表\n"
        "5  修改记录\n"
        "6  删除记录\n"
        "7  按编号查询\n"
        "8  按字段查询\n"
        "9  撤销最近一次新增或删除\n"
        "10 从 CSV 文件加载数据\n"
        "11 保存数据到 CSV 文件\n"
        "12 智能匹配与扩展功能\n"
        "0  退出系统"
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
            print_list(&app->lost_items, "失物列表");
            break;
        case 4:
            print_list(&app->found_items, "拾物列表");
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
            if (!input_read_yes_no("退出前是否保存？(y/n)：",
                                   &save_before_exit)) {
                return 0;
            }
            if (save_before_exit && !save_items(app)) {
                puts("保存未完成，已取消退出。");
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

    configure_console_encoding();
    item_list_init(&app.lost_items, ITEM_LOST);
    item_list_init(&app.found_items, ITEM_FOUND);
    undo_stack_init(&app.undo_stack);

    while (running) {
        int choice;

        show_menu();
        if (!input_read_int("请选择：", 0, 12, &choice)) {
            break;
        }
        running = handle_choice(&app, choice);
    }

    undo_stack_clear(&app.undo_stack);
    item_list_clear(&app.lost_items);
    item_list_clear(&app.found_items);
    puts("感谢使用，再见。");
    return 0;
}
