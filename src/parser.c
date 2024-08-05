#include <stdio.h>
#include <math.h>
#include "parser.h"
#include "list.h"
#include "strmap.h"

void parse_operations(Parser *parser, char *ops, size_t ops_count) 
{
    if (parser->math.oper_list.count != parser->math.num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:\n", __FILE__, __LINE__);
        exit(1);
    }

    Stackd num_stack;
    Stackc oper_stack;
    list_alloc(num_stack);
    list_alloc(oper_stack);

    list_push(num_stack, parser->math.num_list.items[0]);

    size_t j = 1;
    for (size_t i = 0; i < parser->math.oper_list.count; i++) {
        char operator= parser->math.oper_list.items[i];
        if (str_contains(ops, ops_count, operator)) {
            double num = list_pop(num_stack, double);
            double result =
                perform_operation(num, parser->math.num_list.items[i + 1], operator);
            list_push(num_stack, result);
            j = i + 2;
        } 
        else {
            list_push(oper_stack, operator);
            list_push(num_stack, parser->math.num_list.items[j]);
            j++;
        }
    }

    if (num_stack.count != oper_stack.count + 1) {
        printf("Something's wrong, I can feel it\n");
        printf("new.oper_list.count: %zu\n", oper_stack.count);
        printf("num_stack.count: %zu\n", num_stack.count);
    }

    list_transfer(parser->math.num_list, num_stack);
    list_transfer(parser->math.oper_list, oper_stack);
}

double parse_math_func(MathFunc func, Stackd *args) 
{
    if (args->count == 0) {
        return 0;
    }

    switch (func) {
        case SQRT:
            return sqrt(args->items[0]);
        case SIN:
            return sin(args->items[0]);
        case COS:
            return cos(args->items[0]);
        case TAN:
            return tan(args->items[0]);
        case MIN:
            return min(args->items[0], args->items[1]);
        case MAX:
            return max(args->items[0], args->items[1]);
        case SINH: 
            return sinh(args->items[0]);
        case COSH:
            return cosh(args->items[0]);
        case TANH:
            return tanh(args->items[0]);
        case ASIN:
            return asin(args->items[0]);
        case ACOS:
            return acos(args->items[0]);
        case ATAN:
            return atan(args->items[0]);
        case ATAN2:
            return atan2(args->items[0], args->items[1]);
        case EXP:
            return exp(args->items[0]);
        case LOG:
            return log(args->items[0]);
        case LOG10:
            return log10(args->items[0]);
        case CEIL:
            return ceil(args->items[0]);
        case FLOOR:
            return floor(args->items[0]);
        default:
            return 0;
    }
}

void parse_expression(Parser *parser) 
{
    parse_operations(parser, "^", 1);
    parse_operations(parser, "*/%", 3);
}



