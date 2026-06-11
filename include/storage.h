/* CSV 存储模块接口：负责记录的文件加载、保存和坏行统计。
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "item_list.h"

#include <stddef.h>

#define STORAGE_HEADER \
    "id,type,name,category,color,location,keywords,contact," \
    "year,month,day,hour,minute,status"

typedef enum {
    STORAGE_OK = 0,
    STORAGE_INVALID_ARGUMENT,
    STORAGE_OPEN_FAILED,
    STORAGE_READ_FAILED,
    STORAGE_WRITE_FAILED,
    STORAGE_INVALID_DATA,
    STORAGE_OUT_OF_MEMORY
} StorageResult;

typedef struct {
    size_t loaded_rows;
    size_t skipped_rows;
} StorageReport;

StorageResult storage_save_items(
    const char *path,
    const ItemList *lost_items,
    const ItemList *found_items
);
StorageResult storage_load_items(
    const char *path,
    ItemList *lost_items,
    ItemList *found_items,
    StorageReport *report
);

#endif
