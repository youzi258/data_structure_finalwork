#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>

int input_parse_int(const char *text, int minimum, int maximum, int *value);
int input_copy_text(
    const char *text,
    char *destination,
    size_t destination_size,
    int allow_empty
);
int input_read_int(const char *prompt, int minimum, int maximum, int *value);
int input_read_text(
    const char *prompt,
    char *destination,
    size_t destination_size,
    int allow_empty
);
int input_read_yes_no(const char *prompt, int *answer);

#endif

