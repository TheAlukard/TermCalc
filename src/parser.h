#pragma once

#ifndef _PARSER_H_
#define _PARSER_H_

#include "string.h"
#include "math.h"
#include "common.h"

typedef struct {
    String expr;
    Math math;
    double ans;
} Parser;

#define ESC_CHAR '\\'

bool parse_input(String buffer, Math *output);
void parse_expression(Parser *parser);
double parse_math_func(MathFunc func, Stackd *args);
void parse_operations(Parser *parser, char *ops, size_t ops_count);

#endif // _PARSER_H_

