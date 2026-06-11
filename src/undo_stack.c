/* 撤销栈模块实现：用栈保存新增/删除快照，支持一步撤销。
 */

#include "undo_stack.h"

#include <stdlib.h>

static int undo_stack_push(
    UndoStack *stack,
    UndoOperationType operation,
    const Item *item
) {
    UndoNode *node;

    if (stack == NULL || item == NULL || !item_is_valid(item)) {
        return 0;
    }
    node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }
    node->operation = operation;
    node->item = *item;
    node->item.next = NULL;
    node->next = stack->top;
    stack->top = node;
    stack->size++;
    return 1;
}

/* 根据备份记录的类型选择失物链表或拾物链表。 */
static ItemList *target_list(
    ItemType type,
    ItemList *lost_items,
    ItemList *found_items
) {
    if (type == ITEM_LOST) {
        return lost_items;
    }
    if (type == ITEM_FOUND) {
        return found_items;
    }
    return NULL;
}

void undo_stack_init(UndoStack *stack) {
    if (stack == NULL) {
        return;
    }
    stack->top = NULL;
    stack->size = 0;
}

int undo_stack_push_add(UndoStack *stack, const Item *item) {
    return undo_stack_push(stack, UNDO_OPERATION_ADD, item);
}

int undo_stack_push_delete(UndoStack *stack, const Item *item) {
    return undo_stack_push(stack, UNDO_OPERATION_DELETE, item);
}

UndoResult undo_stack_apply(
    UndoStack *stack,
    ItemList *lost_items,
    ItemList *found_items
) {
    UndoNode *node;
    ItemList *list;
    ListResult result;

    if (stack == NULL || lost_items == NULL || found_items == NULL) {
        return UNDO_INVALID_ARGUMENT;
    }
    if (stack->top == NULL) {
        return UNDO_EMPTY;
    }

    node = stack->top;
    list = target_list(node->item.type, lost_items, found_items);
    if (list == NULL) {
        return UNDO_APPLY_FAILED;
    }
    if (node->operation == UNDO_OPERATION_ADD) {
        result = item_list_remove(list, node->item.id, NULL);
    } else {
        result = item_list_append(list, &node->item);
    }
    if (result != LIST_OK) {
        return UNDO_APPLY_FAILED;
    }

    stack->top = node->next;
    stack->size--;
    free(node);
    return UNDO_OK;
}

void undo_stack_clear(UndoStack *stack) {
    UndoNode *current;

    if (stack == NULL) {
        return;
    }
    current = stack->top;
    while (current != NULL) {
        UndoNode *next = current->next;

        free(current);
        current = next;
    }
    stack->top = NULL;
    stack->size = 0;
}
