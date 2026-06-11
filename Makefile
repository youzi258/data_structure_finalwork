CC := D:/mingw/mingw64/bin/gcc.exe
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude
BIN_DIR := bin
SHELL := D:/Git/bin/bash.exe

.PHONY: all app test test-item test-list test-undo test-storage test-input clean

all: test

test: test-item test-list test-undo test-storage test-input

app: $(BIN_DIR)/lost_found.exe

test-item: $(BIN_DIR)/test_item.exe
	./$(BIN_DIR)/test_item.exe

test-list: $(BIN_DIR)/test_item_list.exe
	./$(BIN_DIR)/test_item_list.exe

test-undo: $(BIN_DIR)/test_undo_stack.exe
	./$(BIN_DIR)/test_undo_stack.exe

test-storage: $(BIN_DIR)/test_storage.exe
	mkdir -p tests/tmp
	./$(BIN_DIR)/test_storage.exe

test-input: $(BIN_DIR)/test_input.exe
	./$(BIN_DIR)/test_input.exe

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

$(BIN_DIR)/lost_found.exe: src/main.c src/item.c src/item_list.c src/undo_stack.c src/storage.c src/input.c include/item.h include/item_list.h include/undo_stack.h include/storage.h include/input.h | $(BIN_DIR)
	$(CC) $(CFLAGS) src/main.c src/item.c src/item_list.c src/undo_stack.c src/storage.c src/input.c -o $@

$(BIN_DIR):
	mkdir -p "$(BIN_DIR)"

clean:
	rm -rf "$(BIN_DIR)"
	rm -rf tests/tmp
