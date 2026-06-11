CC := D:/mingw/mingw64/bin/gcc.exe
# 源文件统一保存为 UTF-8，中文字符串在编译后转换为 GBK，适配 Windows 控制台。
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -finput-charset=UTF-8 -fexec-charset=GBK -Iinclude
BIN_DIR := bin
SHELL := D:/Git/bin/bash.exe
MKDIR_P := mkdir -p
RM_RF := rm -rf

.PHONY: all app test test-item test-list test-undo test-storage test-input test-match test-hash test-statistics clean

all: test

test: test-item test-list test-undo test-storage test-input test-match test-hash test-statistics

app: $(BIN_DIR)/lost_found.exe

test-item: $(BIN_DIR)/test_item.exe
	./$(BIN_DIR)/test_item.exe

test-list: $(BIN_DIR)/test_item_list.exe
	./$(BIN_DIR)/test_item_list.exe

test-undo: $(BIN_DIR)/test_undo_stack.exe
	./$(BIN_DIR)/test_undo_stack.exe

test-storage: $(BIN_DIR)/test_storage.exe
	$(MKDIR_P) tests/tmp
	./$(BIN_DIR)/test_storage.exe

test-input: $(BIN_DIR)/test_input.exe
	./$(BIN_DIR)/test_input.exe

test-match: $(BIN_DIR)/test_match.exe
	./$(BIN_DIR)/test_match.exe

test-hash: $(BIN_DIR)/test_hash_index.exe
	./$(BIN_DIR)/test_hash_index.exe

test-statistics: $(BIN_DIR)/test_statistics.exe
	./$(BIN_DIR)/test_statistics.exe

$(BIN_DIR)/test_item.exe: tests/test_item.c src/item.c include/item.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_item.c src/item.c -o $@

$(BIN_DIR)/test_item_list.exe: tests/test_item_list.c src/item.c src/item_list.c include/item.h include/item_list.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_item_list.c src/item.c src/item_list.c -o $@

$(BIN_DIR)/test_undo_stack.exe: tests/test_undo_stack.c src/item.c src/item_list.c src/undo_stack.c include/item.h include/item_list.h include/undo_stack.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_undo_stack.c src/item.c src/item_list.c src/undo_stack.c -o $@

$(BIN_DIR)/test_storage.exe: tests/test_storage.c src/item.c src/item_list.c src/storage.c include/item.h include/item_list.h include/storage.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_storage.c src/item.c src/item_list.c src/storage.c -o $@

$(BIN_DIR)/test_input.exe: tests/test_input.c src/input.c include/input.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_input.c src/input.c -o $@

$(BIN_DIR)/test_match.exe: tests/test_match.c src/item.c src/item_list.c src/match.c include/item.h include/item_list.h include/match.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_match.c src/item.c src/item_list.c src/match.c -o $@

$(BIN_DIR)/test_hash_index.exe: tests/test_hash_index.c src/item.c src/item_list.c src/hash_index.c include/item.h include/item_list.h include/hash_index.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_hash_index.c src/item.c src/item_list.c src/hash_index.c -o $@

$(BIN_DIR)/test_statistics.exe: tests/test_statistics.c src/item.c src/item_list.c src/statistics.c include/item.h include/item_list.h include/statistics.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_statistics.c src/item.c src/item_list.c src/statistics.c -o $@

$(BIN_DIR)/lost_found.exe: src/main.c src/item.c src/item_list.c src/undo_stack.c src/storage.c src/input.c src/match.c src/hash_index.c src/statistics.c include/item.h include/item_list.h include/undo_stack.h include/storage.h include/input.h include/match.h include/hash_index.h include/statistics.h | $(BIN_DIR)
	$(CC) $(CFLAGS) src/main.c src/item.c src/item_list.c src/undo_stack.c src/storage.c src/input.c src/match.c src/hash_index.c src/statistics.c -o $@

$(BIN_DIR):
	$(MKDIR_P) "$(BIN_DIR)"

clean:
	$(RM_RF) "$(BIN_DIR)"
	$(RM_RF) tests/tmp
