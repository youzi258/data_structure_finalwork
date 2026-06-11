/* 安全输入模块实现：所有控制台输入都先用 fgets 读取整行再校验。
 */

#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_BUFFER_LENGTH 512

static void discard_line_remainder(void) {
    int character;

    do {
        character = getchar();
    } while (character != '\n' && character != EOF);
}

int input_parse_int(const char *text, int minimum, int maximum, int *value) {
    const char *start;
    char *end;
    long parsed;

    if (text == NULL || value == NULL || minimum > maximum) {
        return 0;
    }
    start = text;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    if (*start == '\0') {
        return 0;
    }

    errno = 0;
    parsed = strtol(start, &end, 10);
    if (errno != 0 || end == start || parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }
    while (isspace((unsigned char)*end)) {
        end++;
    }
    if (*end != '\0' || parsed < minimum || parsed > maximum) {
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

int input_copy_text(
    const char *text,
    char *destination,
    size_t destination_size,
    int allow_empty
) {
    size_t length;

    if (text == NULL || destination == NULL || destination_size == 0) {
        return 0;
    }
    length = strlen(text);
    while (length > 0 &&
           (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        length--;
    }
    if ((!allow_empty && length == 0) || length >= destination_size) {
        return 0;
    }
    if (memchr(text, ',', length) != NULL ||
        memchr(text, '\n', length) != NULL ||
        memchr(text, '\r', length) != NULL) {
        return 0;
    }
    memcpy(destination, text, length);
    destination[length] = '\0';
    return 1;
}

int input_read_int(const char *prompt, int minimum, int maximum, int *value) {
    char buffer[INPUT_BUFFER_LENGTH];

    while (1) {
        if (prompt != NULL) {
            fputs(prompt, stdout);
            fflush(stdout);
        }
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0;
        }
        if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
            discard_line_remainder();
            puts("输入内容过长，请重新输入。");
            continue;
        }
        if (input_parse_int(buffer, minimum, maximum, value)) {
            return 1;
        }
        printf("请输入 %d 到 %d 之间的整数。\n", minimum, maximum);
    }
}

int input_read_text(
    const char *prompt,
    char *destination,
    size_t destination_size,
    int allow_empty
) {
    char buffer[INPUT_BUFFER_LENGTH];

    while (1) {
        if (prompt != NULL) {
            fputs(prompt, stdout);
            fflush(stdout);
        }
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return 0;
        }
        if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
            discard_line_remainder();
            puts("输入内容过长，请重新输入。");
            continue;
        }
        if (input_copy_text(
                buffer, destination, destination_size, allow_empty)) {
            return 1;
        }
        puts("文本输入无效：请勿使用英文逗号，并确保长度不超过限制。");
    }
}

int input_read_yes_no(const char *prompt, int *answer) {
    char buffer[8];

    if (answer == NULL) {
        return 0;
    }
    while (1) {
        if (!input_read_text(prompt, buffer, sizeof(buffer), 0)) {
            return 0;
        }
        if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0) {
            *answer = 1;
            return 1;
        }
        if (strcmp(buffer, "n") == 0 || strcmp(buffer, "N") == 0) {
            *answer = 0;
            return 1;
        }
        puts("请输入 y 或 n。");
    }
}
