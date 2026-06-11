#include "input.h"
#include "test.h"

static void test_parse_int(void) {
    int value = 0;

    ASSERT_TRUE(input_parse_int(" 42 \n", 1, 100, &value));
    ASSERT_EQ(42, value);
    ASSERT_FALSE(input_parse_int("", 1, 100, &value));
    ASSERT_FALSE(input_parse_int("12x", 1, 100, &value));
    ASSERT_FALSE(input_parse_int("0", 1, 100, &value));
    ASSERT_FALSE(input_parse_int("101", 1, 100, &value));
}

static void test_copy_text(void) {
    char output[8];

    ASSERT_TRUE(input_copy_text("hello\n", output, sizeof(output), 0));
    ASSERT_STR_EQ("hello", output);
    ASSERT_FALSE(input_copy_text("\n", output, sizeof(output), 0));
    ASSERT_TRUE(input_copy_text("\n", output, sizeof(output), 1));
    ASSERT_STR_EQ("", output);
    ASSERT_FALSE(input_copy_text("12345678", output, sizeof(output), 0));
    ASSERT_FALSE(input_copy_text("a,b", output, sizeof(output), 0));
}

int main(void) {
    test_parse_int();
    test_copy_text();
    return test_finish();
}
