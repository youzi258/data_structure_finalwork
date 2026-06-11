# Member A Core Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the member A portion of the campus lost-and-found system: shared model, linked-list CRUD, undo, CSV persistence, and a usable console menu.

**Architecture:** Keep the shared `Item` model and each behavior in focused C modules under `include/` and `src/`. Lost and found records use separate `ItemList` instances with the same API; member B reads those lists through the public headers without depending on the menu.

**Tech Stack:** C11, MinGW GCC at `D:\mingw\mingw64\bin\gcc.exe`, MinGW Make at `D:\mingw\mingw64\bin\mingw32-make.exe`, standard C library only.

---

### Task 1: Build Scaffold And Shared Model

**Files:**
- Create: `include/item.h`
- Create: `src/item.c`
- Create: `tests/test_item.c`
- Create: `tests/test.h`
- Create: `Makefile`
- Create: `.gitignore`

- [ ] **Step 1: Write the failing item conversion and validation tests**

Create tests that assert:

```c
ASSERT_STR_EQ("LOST", item_type_to_string(ITEM_LOST));
ASSERT_EQ(ITEM_FOUND, item_type_from_string("FOUND", &type));
ASSERT_TRUE(item_is_valid(&valid_item));
ASSERT_FALSE(item_is_valid(&invalid_item));
```

- [ ] **Step 2: Run the test and verify it fails**

Run:

```powershell
& 'D:\mingw\mingw64\bin\mingw32-make.exe' test-item
```

Expected: compilation fails because `item.h` and its functions do not exist.

- [ ] **Step 3: Implement the shared model**

Define `ItemType`, `ItemStatus`, `Item`, field limits, and these functions:

```c
const char *item_type_to_string(ItemType type);
int item_type_from_string(const char *text, ItemType *type);
const char *item_status_to_string(ItemStatus status);
int item_status_from_string(const char *text, ItemStatus *status);
int item_is_valid(const Item *item);
```

Validation requires a positive ID, recognized type/status, non-empty name/category/location/contact, and valid calendar/time ranges.

- [ ] **Step 4: Run the test and verify it passes**

Run `mingw32-make test-item`.

Expected: all item tests pass with no compiler warnings.

- [ ] **Step 5: Commit**

```powershell
git add .gitignore Makefile include/item.h src/item.c tests/test.h tests/test_item.c
git commit -m "feat: add shared item model"
```

### Task 2: Implement Linked-List CRUD And Queries

**Files:**
- Create: `include/item_list.h`
- Create: `src/item_list.c`
- Create: `tests/test_item_list.c`
- Modify: `Makefile`

- [ ] **Step 1: Write failing insertion and lookup tests**

Test an empty list, successful append, size tracking, duplicate-ID rejection, wrong-type rejection, and lookup by ID.

- [ ] **Step 2: Run the tests and verify they fail**

Run `mingw32-make test-list`.

Expected: compilation fails because the list API does not exist.

- [ ] **Step 3: Implement initialization, append, and lookup**

Expose:

```c
void item_list_init(ItemList *list, ItemType accepted_type);
ListResult item_list_append(ItemList *list, const Item *item);
Item *item_list_find_by_id(ItemList *list, int id);
const Item *item_list_find_by_id_const(const ItemList *list, int id);
```

- [ ] **Step 4: Run tests and verify they pass**

Run `mingw32-make test-list`.

- [ ] **Step 5: Write failing update, delete, query, and clear tests**

Cover head/middle/tail deletion, missing IDs, preserving `id` and `type` during update, case-insensitive substring query, and freeing an entire list.

- [ ] **Step 6: Implement the remaining list operations**

Expose:

```c
ListResult item_list_update(ItemList *list, int id, const Item *replacement);
ListResult item_list_remove(ItemList *list, int id, Item *removed);
size_t item_list_visit_matching(
    const ItemList *list,
    ItemField field,
    const char *query,
    ItemVisitor visitor,
    void *context
);
void item_list_clear(ItemList *list);
```

- [ ] **Step 7: Run all list tests**

Run `mingw32-make test-list`.

Expected: all list tests pass with no warnings.

- [ ] **Step 8: Commit**

```powershell
git add Makefile include/item_list.h src/item_list.c tests/test_item_list.c
git commit -m "feat: implement item list management"
```

### Task 3: Implement Undo Stack

**Files:**
- Create: `include/undo_stack.h`
- Create: `src/undo_stack.c`
- Create: `tests/test_undo_stack.c`
- Modify: `Makefile`

- [ ] **Step 1: Write failing stack behavior tests**

Cover empty-stack undo, undoing an add, undoing a delete, and LIFO order across multiple operations.

- [ ] **Step 2: Run the test and verify it fails**

Run `mingw32-make test-undo`.

Expected: compilation fails because the undo API does not exist.

- [ ] **Step 3: Implement the undo stack**

Expose:

```c
void undo_stack_init(UndoStack *stack);
int undo_stack_push_add(UndoStack *stack, const Item *item);
int undo_stack_push_delete(UndoStack *stack, const Item *item);
UndoResult undo_stack_apply(
    UndoStack *stack,
    ItemList *lost_items,
    ItemList *found_items
);
void undo_stack_clear(UndoStack *stack);
```

The stack stores copies, chooses the target list from `Item.type`, and does not create a new undo entry while applying an undo.

- [ ] **Step 4: Run undo and regression tests**

Run `mingw32-make test`.

Expected: all tests pass.

- [ ] **Step 5: Commit**

```powershell
git add Makefile include/undo_stack.h src/undo_stack.c tests/test_undo_stack.c
git commit -m "feat: add undo stack"
```

### Task 4: Implement CSV Persistence

**Files:**
- Create: `include/storage.h`
- Create: `src/storage.c`
- Create: `tests/test_storage.c`
- Modify: `Makefile`

- [ ] **Step 1: Write failing save-and-load round-trip test**

Create one lost and one found item, save them to a temporary CSV, load into fresh lists, and compare every persisted field.

- [ ] **Step 2: Run the test and verify it fails**

Run `mingw32-make test-storage`.

Expected: compilation fails because the storage API does not exist.

- [ ] **Step 3: Implement CSV save and basic load**

Expose:

```c
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
```

Save the documented header and fields. Load into temporary lists and replace the caller lists only after file processing completes.

- [ ] **Step 4: Run round-trip test and verify it passes**

Run `mingw32-make test-storage`.

- [ ] **Step 5: Write failing malformed-file tests**

Cover nonexistent file, header-only file, malformed row, invalid enum/date, and duplicate ID. Assert invalid rows are skipped and counted.

- [ ] **Step 6: Implement defensive CSV loading**

Reject fields containing commas when saving, cap line length, validate every parsed item, and populate `StorageReport` with loaded/skipped counts.

- [ ] **Step 7: Run storage and regression tests**

Run `mingw32-make test`.

Expected: all tests pass and temporary files are removed by the test binary.

- [ ] **Step 8: Commit**

```powershell
git add Makefile include/storage.h src/storage.c tests/test_storage.c
git commit -m "feat: add csv persistence"
```

### Task 5: Build Safe Console Input And Menu

**Files:**
- Create: `include/input.h`
- Create: `src/input.c`
- Create: `src/main.c`
- Modify: `Makefile`

- [ ] **Step 1: Write failing input parser tests**

Test valid integer parsing, trailing garbage, empty input, range rejection, and overlong text rejection using parser functions that do not require an interactive terminal.

- [ ] **Step 2: Run the test and verify it fails**

Run `mingw32-make test-input`.

- [ ] **Step 3: Implement safe input helpers**

Expose parsing helpers and interactive wrappers based on `fgets`; never use unbounded `%s`.

- [ ] **Step 4: Run input tests and verify they pass**

Run `mingw32-make test-input`.

- [ ] **Step 5: Implement the main menu**

Connect member A operations:

```text
1 Add lost item
2 Add found item
3 Show lost items
4 Show found items
5 Update item
6 Delete item
7 Find item by ID
8 Search by keyword/location/category
9 Undo last add/delete
10 Load items from CSV
11 Save items to CSV
12 Member B feature menu
0 Exit
```

The member B entry prints a clear placeholder and does not fail the build.

- [ ] **Step 6: Build and run scripted smoke input**

Run:

```powershell
& 'D:\mingw\mingw64\bin\mingw32-make.exe' app
@("3","4","12","0","n") | .\bin\lost_found.exe
```

Expected: both empty lists display, member B placeholder displays, and the program exits normally.

- [ ] **Step 7: Commit**

```powershell
git add Makefile include/input.h src/input.c src/main.c tests/test_input.c
git commit -m "feat: add console menu"
```

### Task 6: Add Demonstration Data And Integration Verification

**Files:**
- Create: `data/items.csv`
- Modify: `README.md`
- Modify: `Makefile`

- [ ] **Step 1: Add representative sample data**

Add at least five valid lost records and five valid found records that cover common categories and locations. Keep the full 30-record dataset for the later joint integration phase.

- [ ] **Step 2: Document exact MinGW build and run commands**

Document:

```powershell
& 'D:\mingw\mingw64\bin\mingw32-make.exe' test
& 'D:\mingw\mingw64\bin\mingw32-make.exe' app
.\bin\lost_found.exe
```

- [ ] **Step 3: Run complete verification**

Run `mingw32-make clean`, `mingw32-make test`, `mingw32-make app`, and a scripted load/list/save/undo smoke scenario.

Expected: all tests and the smoke scenario pass with no compiler warnings.

- [ ] **Step 4: Review member B integration surface**

Confirm that `include/item.h` and `include/item_list.h` are sufficient for member B to read both lists and that no member B implementation was added.

- [ ] **Step 5: Commit**

```powershell
git add Makefile README.md data/items.csv
git commit -m "docs: add build guide and sample data"
```

### Task 7: Review And Publish Draft PR

**Files:**
- Modify only files required by review findings.

- [ ] **Step 1: Run final verification from a clean build**

Run:

```powershell
& 'D:\mingw\mingw64\bin\mingw32-make.exe' clean
& 'D:\mingw\mingw64\bin\mingw32-make.exe' test
& 'D:\mingw\mingw64\bin\mingw32-make.exe' app
git diff --check origin/main...HEAD
```

- [ ] **Step 2: Perform code review**

Review the full diff against the design and member A requirements. Fix all critical and important findings, rerun verification, and commit fixes.

- [ ] **Step 3: Push the branch over SSH**

```powershell
git push -u origin feature/member-a-core
```

- [ ] **Step 4: Create a Draft PR targeting `main`**

The PR description must summarize the member A scope, public interface for member B, exact MinGW checks, and deferred member B/report work.
