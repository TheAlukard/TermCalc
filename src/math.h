#pragma once

#ifndef MY_MATH_H_
#define MY_MATH_H_

#include "string.h"
#include "common.h"

typedef struct {
    double *items;
    size_t capacity;
    size_t count;
} Stackd;

typedef struct {
    char *items;
    size_t capacity;
    size_t count;
} Stackc;

typedef struct {
    double *items;
    size_t capacity;
    size_t count;
} NumList;

typedef struct {
    char *items;
    size_t capacity;
    size_t count;
} OperList;

typedef struct {
    NumList num_list;
    OperList oper_list;
} Math;

typedef enum {
    SQRT = 0,
    SIN,
    COS,
    TAN,
    MIN,
    MAX,
    SINH,
    COSH,
    TANH,
    ASIN,
    ACOS,
    ATAN,
    ATAN2,
    EXP,
    LOG,
    LOG10,
    CEIL,
    FLOOR,
    NOPE,
} MathFunc;

#define PI 3.14159265358979323846f

#define max(one, two) ((one) > (two) ? (one) : (two))
#define min(one, two) ((one) < (two) ? (one) : (two))

void print_math(const Math math);
double perform_operation(double num1, double num2, char operation);
int get_func_param_count(MathFunc func);
bool chop_func_params(String *buffer, MathFunc func, Stackd *output);
double do_the_math(const Math math);
MathFunc chop_func(String *buffer);

#endif // MY_MATH_H_
