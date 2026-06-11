#ifndef UNDO_STACK_H
#define UNDO_STACK_H

#include "item_list.h"

#include <stddef.h>

typedef enum {
    UNDO_OPERATION_ADD,
    UNDO_OPERATION_DELETE
} UndoOperationType;

typedef struct UndoNode {
    UndoOperationType operation;
    Item item;
    struct UndoNode *next;
} UndoNode;

typedef struct {
    UndoNode *top;
    size_t size;
} UndoStack;

typedef enum {
    UNDO_OK = 0,
    UNDO_EMPTY,
    UNDO_INVALID_ARGUMENT,
    UNDO_APPLY_FAILED
} UndoResult;

void undo_stack_init(UndoStack *stack);
int undo_stack_push_add(UndoStack *stack, const Item *item);
int undo_stack_push_delete(UndoStack *stack, const Item *item);
UndoResult undo_stack_apply(
    UndoStack *stack,
    ItemList *lost_items,
    ItemList *found_items
);
void undo_stack_clear(UndoStack *stack);

#endif
