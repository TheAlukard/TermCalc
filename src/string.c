#include "string.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

double chop_num(String *buffer) 
{
    size_t count = 0;
    bool isdot = false;
    String pos = *buffer;
    trim_left(&pos);
    while (pos.len > 0) {
        if (is_digit(*pos.str)) {
            while (pos.len > 0 && (is_digit(*pos.str) == true || *pos.str == '.')) {
                if (*pos.str == '.') {
                    if (isdot) {
                        return 0;
                    } 
                    else {
                        isdot = true;
                    }
                }
                count++;
                chop_char_trim(&pos);
            }
            char temp[count + 1];
            memcpy(temp, buffer->str, count);
            temp[count] = '\0';
            char *end;
            double result = strtod(temp, &end);
            buffer->str = pos.str;
            buffer->len = pos.len;

            return result;
        }
        chop_char_trim(&pos);
    }

    return 0;
}

bool chop_word(String *buffer, char *word, size_t word_size) 
{
    size_t i = 0;
    String pos = *buffer;

    trim_left(&pos);
    while (pos.len > 0 && i < word_size) {
        if (*pos.str != *word) {
            return false;
        }

        word++;
        i++;
        chop_char_trim(&pos);
    }

    if (i != word_size || is_alnum(buffer->str[i])) {
        return false;
    }

    buffer->str = pos.str;
    buffer->len = pos.len;

    return true;
}

String chop_paren(String *buffer) 
{
    if (buffer->str[0] != '(') {
        return (String){0};
    }

    bool ended = false;
    int64_t countered = 0;
    String pos = *buffer;
    chop_char_trim(&pos);
    char *start = pos.str;
    char *location = pos.str;

    while (pos.len > 0) {
        if (*pos.str == ')') {
            location = pos.str;
            chop_char_trim(&pos);
            if (countered == 0) {
                ended = true;
                break;
            } 
            else {
                countered--;
            }
        } 
        else if (*pos.str == '(') {
            ended = false;
            countered++;
            chop_char_trim(&pos);
        } 
        else {
            chop_char_trim(&pos);
        }
    }

    if (!ended) {
        return (String){0};
    }

    String new_expr = {
        .str = start,
        .len = location - start,
    };

    buffer->str = pos.str;
    buffer->len = pos.len;

    return new_expr;
}

bool chop_func_name(String *buffer, char *word)
{
    size_t i = 0;
    String pos = *buffer;
    size_t word_size = strlen(word);

    trim_left(&pos);
    while (pos.len > 0 && i < word_size) {
        if (*pos.str != *word) {
            return false;
        }

        word++;
        i++;
        chop_char_trim(&pos);
    }

    if (i != word_size || buffer->str[i] != '(') {
        return false;
    }

    buffer->str = pos.str;
    buffer->len = pos.len;

    return true;
}

String chop_expr(String *buffer) 
{
    int64_t countered = 0;
    bool stop = true;
    size_t i;
    String expr = {0};

    for (i = 0; i < buffer->len; i++) {
        char c = buffer->str[i];
        if (c == '(') {
            countered++;
            stop = false;
        } 
        else if (c == ')') {
            countered--;
            if (countered == 0) {
                stop = true;
            } 
            else if (countered < 0) {
                return expr;
            }
        } 
        else if (stop == true && c == ',') {
            break;
        }
    }

    if (stop) {
        expr.str = buffer->str;
        expr.len = i;
        buffer->str = &buffer->str[i];
        buffer->len = buffer->len - i;
    }

    return expr;
}

String chop_name(String *buffer)
{
    String var = {0};

    String pos = *buffer;

    if (! is_alpha(*buffer->str)) return var;

    while (pos.len > 0 && is_alnum(*pos.str)) {
        chop_char(&pos);
    } 

    var.str = buffer->str;
    var.len = pos.str - buffer->str;

    buffer->str = pos.str;
    buffer->len = pos.len;

    return var;
}
