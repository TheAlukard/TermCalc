#include "math.h"
#include "string.h"
#include "parser.h"
#include "list.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

void print_math(const Math math) 
{
    printf("Math:\n");

    for (size_t i = 0; i < math.num_list.count; i++) {
        printf("    %lf \n", math.num_list.items[i]);
        if (i < math.oper_list.count) {
            printf("    %c \n", math.oper_list.items[i]);
        }
    }

    printf("\n");
}

double perform_operation(double num1, double num2, char operation)
{
    switch (operation) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            return num1 / num2;
        case '%':
            return (int64_t)(num1) % (int64_t)(num2);
        case '^':
            return pow(num1, num2);
        default:
            fprintf(stderr, "%s:%d:1 Invalid operation: '%c'\n", __FILE__, __LINE__,
                    operation);
            exit(1);
    }
}

int get_func_param_count(MathFunc func) 
{
    switch (func) {
        case SQRT : // fall through
        case SIN  : // fall through
        case COS  : // fall through
        case SINH : // fall through
        case COSH : // fall through
        case TANH : // fall through
        case ASIN : // fall through
        case ACOS : // fall through
        case ATAN : // fall through
        case EXP  : // fall through
        case LOG  : // fall through
        case LOG10: // fall through
        case CEIL : // fall through
        case FLOOR: // fall through
        case TAN  : return 1;
        case MIN  : // fall through
        case ATAN2: // fall through
        case MAX  : return 2;
        default  : return -1;
    }
}

MathFunc chop_func(String *buffer) 
{
    if (buffer->str[0] != ESC_CHAR || buffer->len == 0) {
        return NOPE;
    }

    String new_buff = {
        .str = &buffer->str[1],
        .len = buffer->len - 1,
    };

    if (new_buff.len == 0) {
        return NOPE;
    }

    MathFunc func = NOPE;

    char *func_names[NOPE] = {
        "sqrt",
        "sin",
        "cos",
        "tan",
        "min",
        "max",
        "sinh",
        "cosh",
        "tanh",
        "asin",
        "acos",
        "atan",
        "atan2",
        "exp",
        "log",
        "log10",
        "ceil",
        "floor",
    };

    for (size_t i = 0; i < NOPE; i++) {
        if (chop_func_name(&new_buff, func_names[i])) {
            func = i;
            break;
        }
    }

    if (func != NOPE) {
        buffer->str = new_buff.str;
        buffer->len = new_buff.len;
    }

    return func;
}

bool chop_func_params(String *buffer, MathFunc func, Stackd *output) 
{
    if (buffer->str[0] != '(') {
        return false;
    }

    String args = chop_paren(buffer);
    if (args.str == NULL) {
        return false;
    }
    Parser parser;
    parser.expr = args;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    bool success = true;
    int arg_count = get_func_param_count(func);

    for (int i = 0; i < arg_count; i++) {
        String arg;
        arg = chop_expr(&args);
        if (arg.str == NULL) {
            success = false;
            break;
        } 
        else if (i < arg_count - 1 && *args.str != ',') {
            success = false;
            break;
        }
        chop_char_trim(&args);
        list_clear(parser.math.num_list);
        list_clear(parser.math.oper_list);
        if (!parse_input(arg, &parser.math)) {
            success = false;
            break;
        }

        parse_expression(&parser);
        if (parser.math.num_list.count <= 0) {
            success = false;
            break;
        }

        list_push(*output, do_the_math(parser.math));
    }

    list_free(parser.math.num_list);
    list_free(parser.math.oper_list);

    return success;
}

double do_the_math(const Math math) 
{
    double result = math.num_list.items[0];

    for (size_t i = 1; i < math.num_list.count; i++) {
        result = perform_operation(result, math.num_list.items[i],
                                   math.oper_list.items[i - 1]);
    }

    return result;
}

