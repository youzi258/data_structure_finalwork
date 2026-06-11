/* 轻量级测试框架：提供 C 项目单元测试常用断言和汇总输出。
 */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

static int test_failures = 0;

#define ASSERT_TRUE(condition)                                                   \
    do {                                                                         \
        if (!(condition)) {                                                       \
            fprintf(stderr, "%s:%d: assertion failed: %s\n",                    \
                    __FILE__, __LINE__, #condition);                              \
            test_failures++;                                                      \
        }                                                                         \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_EQ(expected, actual)                                               \
    do {                                                                         \
        long test_expected = (long)(expected);                                    \
        long test_actual = (long)(actual);                                        \
        if (test_expected != test_actual) {                                       \
            fprintf(stderr, "%s:%d: expected %ld, got %ld\n",                   \
                    __FILE__, __LINE__, test_expected, test_actual);               \
            test_failures++;                                                      \
        }                                                                         \
    } while (0)

#define ASSERT_STR_EQ(expected, actual)                                           \
    do {                                                                         \
        const char *test_expected = (expected);                                   \
        const char *test_actual = (actual);                                       \
        if (strcmp(test_expected, test_actual) != 0) {                            \
            fprintf(stderr, "%s:%d: expected \"%s\", got \"%s\"\n",            \
                    __FILE__, __LINE__, test_expected, test_actual);               \
            test_failures++;                                                      \
        }                                                                         \
    } while (0)

static int test_finish(void) {
    if (test_failures == 0) {
        puts("All tests passed.");
        return 0;
    }

    fprintf(stderr, "%d test assertion(s) failed.\n", test_failures);
    return 1;
}

#endif
