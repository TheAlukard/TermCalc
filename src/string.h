#pragma once

#ifndef MY_STRING_H_
#define MY_STRING_H_

#include "utils.h"
#include "common.h"

typedef struct {
    char *str;
    size_t len;
} String;

INLINE bool str_contains(const char *str, size_t count, char item) 
{
    for (size_t i = 0; i < count; i++) {
        if (str[i] == item)
            return true;
    }

    return false;
}

INLINE void chop_char(String *buffer) 
{
    if (buffer->len == 0)
        return;

    buffer->str += 1;
    buffer->len -= 1;
}

INLINE void trim_left(String *str) 
{
    while (str->len > 0 && is_space(*str->str)) {
        chop_char(str);
    }
}

INLINE void chop_char_trim(String *buffer) 
{
    chop_char(buffer);
    trim_left(buffer);
}

INLINE bool str_equal(String one, String two)
{
    if (one.len != two.len) return false;

    for (size_t i = 0; i < one.len; i++) {
        if (*one.str != *two.str) return false;
    }

    return true;
}

INLINE void string_lower(String *string) 
{
    for (size_t i = 0; i < string->len; i++) {
        if (string->str[i] >= 65 && string->str[i] <= 90) {
            string->str[i] = string->str[i] + 32;
        }
    }
}

String chop_name(String *buffer);
double chop_num(String *buffer);
bool chop_word(String *buffer, char *word, size_t word_size);
String chop_paren(String *buffer);
bool chop_func_name(String *buffer, char *word);
String chop_expr(String *buffer);

#endif // MY_STRING_H_
