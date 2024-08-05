#pragma once

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>

#define INLINE __attribute__((always_inline)) static inline

INLINE bool is_alpha(char c) 
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_');
}

INLINE bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

INLINE bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

INLINE bool is_space(char c)
{
    return c == ' '  ||
           c == '\r' ||
           c == '\t' ||
           c == '\n';
}

#endif // _UTILS_H_
