CC := D:/mingw/mingw64/bin/gcc.exe
CFLAGS := -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude
BIN_DIR := bin
SHELL := D:/Git/bin/bash.exe

.PHONY: all test test-item test-list clean

all: test

test: test-item test-list

test-item: $(BIN_DIR)/test_item.exe
	./$(BIN_DIR)/test_item.exe

test-list: $(BIN_DIR)/test_item_list.exe
	./$(BIN_DIR)/test_item_list.exe

$(BIN_DIR)/test_item.exe: tests/test_item.c src/item.c include/item.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_item.c src/item.c -o $@

$(BIN_DIR)/test_item_list.exe: tests/test_item_list.c src/item.c src/item_list.c include/item.h include/item_list.h tests/test.h | $(BIN_DIR)
	$(CC) $(CFLAGS) tests/test_item_list.c src/item.c src/item_list.c -o $@

$(BIN_DIR):
	mkdir -p "$(BIN_DIR)"

clean:
	rm -rf "$(BIN_DIR)"
