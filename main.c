#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LIST_IMPLEMENTATION
#include "list.h"

const char operators[] = "+-/*%^";
#define operator_count ((sizeof(operators) / sizeof(operators[0])) - 1)

double ANS = 0;

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

typedef struct {
    char *str;
    size_t len;
} String;

typedef struct {
    String expr;
    Math math;
    double ans;
} Parser;

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

typedef enum {
    SQRT,
    SIN,
    COS,
    TAN,
    MIN,
    MAX,
    NOPE,
} MathFunc;

#define Max(one, two) ((one) > (two) ? (one) : (two))

#define Min(one, two) ((one) < (two) ? (one) : (two))

bool parse_input(String buffer, Math *output);
void parse_expression(Parser *parser);
double parse_math_func(MathFunc func, Stackd *args);
void parse_operations(Parser *parser, char *ops, size_t ops_count);
static inline double do_the_math(const Math math);
static inline void chop_char(String *buffer);
static inline void chop_char_trim(String *buffer);
static inline void trim_left(String *str);

#define INLINE __attribute__((always_inline)) static inline

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

INLINE void string_lower(String *string) 
{
    for (size_t i = 0; i < string->len; i++) {
        if (string->str[i] >= 65 && string->str[i] <= 90) {
            string->str[i] = string->str[i] + 32;
        }
    }
}

INLINE bool str_contains(const char *str, size_t count, char item) 
{
    for (size_t i = 0; i < count; i++) {
        if (str[i] == item)
            return true;
    }

    return false;
}

INLINE double perform_operation(double num1, double num2, char operation) 
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

INLINE void chop_char(String *buffer) 
{
    if (buffer->len == 0)
        return;

    buffer->str += 1;
    buffer->len -= 1;
}

INLINE void trim_left(String *str) 
{
    while (str->len > 0 && isspace(*str->str)) {
        chop_char(str);
    }
}

INLINE void chop_char_trim(String *buffer) 
{
    chop_char(buffer);
    trim_left(buffer);
}

double chop_num(String *buffer) 
{
    size_t count = 0;
    bool isdot = false;
    String pos = *buffer;
    trim_left(&pos);
    while (pos.len > 0) {
        if (isdigit(*pos.str)) {
            while (pos.len > 0 && (isdigit(*pos.str) == true || *pos.str == '.')) {
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

    if (i != word_size) {
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

#define ESC_CHAR '\\'

int get_func_param_count(MathFunc func) 
{
    switch (func) {
        case SQRT: // fall through
        case SIN : // fall through
        case COS : // fall through
        case TAN : return 1;
        case MIN : // fall through
        case MAX : return 2;
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

    MathFunc func = NOPE;

    if (new_buff.len == 0) {
        return NOPE;
    }

    switch (new_buff.str[0]) {
        case 's':
            if (chop_word(&new_buff, "sqrt", 4)) {
                func = SQRT;
            } 
            else if (chop_word(&new_buff, "sin", 3)) {
                func = SIN;
            }
            break;
        case 'c':
            if (chop_word(&new_buff, "cos", 3)) {
                func = COS;
            }
            break;
        case 't':
            if (chop_word(&new_buff, "tan", 3)) {
                func = TAN;
            }
            break;
        case 'm':
            if (chop_word(&new_buff, "min", 3)) {
                func = MIN;
            } 
            else if (chop_word(&new_buff, "max", 3)) {
                func = MAX;
            }
            break;
    }

    if (func != NOPE) {
        buffer->str = new_buff.str;
        buffer->len = new_buff.len;
    }

    return func;
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

bool parse_input(String buffer, Math *output) 
{
    trim_left(&buffer);
    string_lower(&buffer);

    bool isnum = false;
    bool negate = false;
    if (buffer.str[0] == '-') {
        negate = true;
        chop_char_trim(&buffer);
    }

    if (buffer.str[0] == 'e') {
        bool _exit = chop_word(&buffer, "exit", 4);
        if (!_exit) {
            fprintf(stderr, "Error: Invalid input.\n");
            return false;
        }
        exit(0);
    }

    while (buffer.len > 0) {
        char c = *buffer.str;
        if (isdigit(c)) {
            double num = chop_num(&buffer);
            isnum = true;
            if (negate) {
                num *= -1;
                negate = false;
            }
            list_push(output->num_list, num);
        } 
        else if (str_contains(operators, operator_count, c)) {
            if (!isnum) {
                fprintf(stderr, "Error: Operator wasn't preceeded by a number\n");
                return false;
            }

            list_push(output->oper_list, c);
            isnum = false;
            chop_char_trim(&buffer);

            if (buffer.len > 0 && *buffer.str == '-') {
                negate = true;
                chop_char_trim(&buffer);
            }
        } 
        else if (c == '(') {
            Parser parser;
            parser.expr = chop_paren(&buffer);
            // printf("paren: %*.*s\n", (int)parser.expr.len, (int)parser.expr.len,
            // parser.expr.str);
            if (parser.expr.str == NULL) {
                fprintf(stderr, "Error: Invalid parenthesis.\n");
                return false;
            }
            list_alloc(parser.math.num_list);
            list_alloc(parser.math.oper_list);
            parse_input(parser.expr, &parser.math);
            parse_expression(&parser);
            double result = do_the_math(parser.math);
            if (negate) {
                result *= -1;
                negate = false;
            }
            list_push(output->num_list, result);
            list_free(parser.math.num_list);
            list_free(parser.math.oper_list);
            isnum = true;
        } 
        else if (c == 'a') {
            bool success = chop_word(&buffer, "ans", 3);
            if (!success) {
                fprintf(stderr, "Error: Invalid input.\n");
                return false;
            }
            isnum = true;
            list_push(output->num_list, ANS);
        } 
        else if (c == ESC_CHAR) {
            MathFunc func = chop_func(&buffer);
            if (func == NOPE) {
                fprintf(stderr, "Invalid function.\n");
                return false;
            }
            Stackd arg_list;
            list_alloc(arg_list);
            if (!chop_func_params(&buffer, func, &arg_list)) {
                fprintf(stderr, "Invalid function parameters.\n");
                list_free(arg_list);
                return false;
            }
            list_push(output->num_list, parse_math_func(func, &arg_list));
            list_free(arg_list);
            isnum = true;
        } 
        else {
            fprintf(stderr, "Error: Invalid input.\nBuffer: %s\nLen: %zu\n",
                    buffer.str, buffer.len);
            return false;
        }
    }

    return true;
}

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
            return Min(args->items[0], args->items[1]);
        case MAX:
            return Max(args->items[0], args->items[1]);
        default:
            return 0;
    }
}

void parse_expression(Parser *parser) 
{
    parse_operations(parser, "^", 1);
    parse_operations(parser, "*/%", 3);
}

INLINE double do_the_math(const Math math) 
{
    double result = math.num_list.items[0];

    for (size_t i = 1; i < math.num_list.count; i++) {
        result = perform_operation(result, math.num_list.items[i],
                                   math.oper_list.items[i - 1]);
    }

    return result;
}

bool expected(char *input, double expected_output) 
{
    Parser parser;
    parser.expr = (String){
        .str = input,
        .len = strlen(input),
    };
    parser.ans = 0;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    String str = {
        .str = input,
        .len = strlen(input),
    };
    parse_input(str, &parser.math);

#if 0
    for (size_t i = 0; i < parser.math.num_list.count; i++) {
        printf("%lf ",parser.math.num_list.items[i]);
        if (i < parser.math.oper_list.count) {
            printf("%c ", parser.math.oper_list.items[i]);
        }
    }
#endif

    parse_expression(&parser);
    double output = do_the_math(parser.math);
    ANS = output;

    list_free(parser.math.num_list);
    list_free(parser.math.oper_list);

    bool success;

    printf("{\n");
    printf("    Input   : %s\n", input);
    double eps = 1e-5;
    if (expected_output - output <= eps) {
        success = true;
    } 
    else {
        success = false;
    }
    printf("    Output  : %lf\n", output);
    printf("    Expected: %lf\n", expected_output);
    if (success) {
        printf("    PASSED!\n");
    } 
    else {
        printf("    FAILED!\n");
    }
    printf("}\n");

    return success;
}

#define EXPECTED(expr, EXPECTED_OUTPUT, success)                               \
  do {                                                                         \
    char EXPR[] = expr;                                                        \
    if (!expected(EXPR, EXPECTED_OUTPUT)) {                                    \
      success = false;                                                         \
    }                                                                          \
  } while (0)

void test() 
{
    bool success = true;
    long time1 = clock();

    EXPECTED(
        "3 + 3",
        6,
        success
    );
    EXPECTED(
        "3 - 3",
        0,
        success
    );
    EXPECTED(
        "3 * 3",
        9,
        success
    );
    EXPECTED(
        "3 / 3",
        1,
        success
    );
    EXPECTED(
        "3 ^ 3",
        27,
        success
    );
    EXPECTED(
        "5 % 3",
        2,
        success
    );
    EXPECTED(
        "5 - -3",
        8,
        success
    );
    EXPECTED(
        "5 + -(2 * 3)",
        -1,
        success
    );
    EXPECTED(
        "5 * -1",
        -5,
        success
    );
    EXPECTED(
        "3 * 7 + 46 % 4",
        23,
        success
    );
    EXPECTED(
        "5 * 2 ^ 3 / 4 + 1 * 5",
        15,
        success
    );
    EXPECTED(
        "5 * 2 ^ 3 ^ 4 + 1 +  79",
        20560,
        success
    );
    EXPECTED(
        "1 + 3 *  9",
        28,
        success
    );
    EXPECTED(
        "64 ^ 0 / 2 + 6",
        6.5f,
        success
    );
    EXPECTED(
        "4 ^ 2 / 8 + 1",
        3,
        success
    );
    EXPECTED(
        "1 / 0.5 + 6",
        8,
        success
    );
    EXPECTED(
        "3 + 5 / 3 ^ 4 * 9 - 2 * 1",
        1.55555555555556,
        success
    );
    EXPECTED(
        "3 + 3 + 3 + 3 + 3 ^ 0 + 3",
        16,
        success
    );
    EXPECTED(
        "1 / 0.3 - 0.1 * 3 ^ 2",
        2.43333333333333,
        success
    );
    EXPECTED(
        "0 / 2 * 1",
        0,
        success
    );
    EXPECTED(
        "0/1*2",
        0,
        success
    );
    EXPECTED(
        "2 ^ 3 + 2",
        10,
        success
    );
    EXPECTED(
        "2 ^ (3 + 2)",
        32,
        success
    );
    EXPECTED(
        "9 / 3 - 2",
        1,
        success
    );
    EXPECTED(
        "9 / (3 - 2)",
        9,
        success
    );
    EXPECTED(
        "9 + 2 ^ 2 + 9 * 3 - 1 - 1",
        9 + pow(2, 2) + 9 * 3 - 1 - 1, 
        success
    );
    EXPECTED(
        "9 + 2 ^ (2 + 9 * 3 - 1 - 1)",
        9 + pow(2, 2 + 9 * 3 - 1 - 1),
        success
    );
    EXPECTED(
        "9 + 2 ^ (2 + 9 * (3 - 1) - 1)",
        9 + pow(2, 2 + 9 * (3 - 1) - 1),
        success
    );

    EXPECTED(
        "0 / (3 - 1) + 9 * (2 ^ (1+1) + 3 * (3 * (1 - 1 + (3 / 1))))",
        0.f / (3 - 1) + 9 * (pow(2, 1 + 1) + 3 * (3 * (1 - 1 + (3.f / 1)))),
        success
    );
    EXPECTED(
        "90 / ( 32.32 * 32 ^ ( 3 / 2 ) ) + ( 3 * ( 32 % ( 4 ^ ( 2 * ( 1 + 1 ) ) ) ) )",
        90 / (32.32f * pow(32, 3.f / 2.f)) + (3 * (perform_operation(32.f, pow(4, 2 * (1 + 1)), '%'))),
        success
    );
    EXPECTED(
        "32 -(8 * -5 / 0.3 * (6 - 7) + -1) / 3.5 - -8",
        32 - (8 * -5 / 0.3f * (6 - 7) + -1) / 3.5 - -8,
        success
    );
    EXPECTED(
        "43 * 3",
        43 * 3,
        success
    );
    EXPECTED(
        "Ans + 1",
        (43 * 3) + 1,
        success
    );
    EXPECTED(
        "3 * ans * (ans ^ -2 + 1.3)",
        3 * ((43 * 3) + 1) * (pow((43 * 3) + 1, -2) + 1.3f),
        success
    );
    EXPECTED(
        "\\sqrt(42)",
        sqrt(42),
        success
    );
    EXPECTED(
        "\\sqrt(16) / 4 * (\\sqrt(4) + (27 - \\sqrt(9)))",
        sqrt(16) / 4 * (sqrt(4) + (27 - sqrt(9))),
        success
    );
    EXPECTED(
        "\\sqrt(30) / (\\tan(50.3) ^ (\\sin(43) / \\cos(44)))",
        sqrt(30) / pow(tan(50.3), sin(43) / cos(44)),
        success
    );
    EXPECTED(
        "\\sqrt(\\sin(30) / 4.6 * (\\tan(30) / 30)) + 20 ^ 2 / 30 * (3 ^ 2 + 2)",
        sqrt(sin(30) / 4.6 * (tan(30) / 30)) + pow(20, 2) / 30 * (pow(3, 2) + 2),
        success
    );
    EXPECTED(
        "\\max(3, 7)",
        Max(3, 7),
        success
    );
    EXPECTED(
        "\\max(43, \\sqrt(\\min(3 ^ 2, \\max(\\tan(43 * 2), 123 ) ) ) )",
        Max(43, sqrt(Min(pow(3, 3), Max(tan(43 * 2), 123)))),
        success
    );
    EXPECTED(
        "64 / 3 / (32 - 31.3) * \\max(\\sqrt(3), \\min(3, 0.5)) - 6^2",
        64.f / 3 / (32 - 31.3) * Max(sqrt(3), Min(3, 0.5)) - pow(6, 2),
        success
    );
    long time2 = clock();
    double delta = (double)(time2 - time1) / CLOCKS_PER_SEC;

    if (success) {
        printf("ALL TESTS WERE SUCCESSFUL!\n");
    } 
    else {
        printf("NOT ALL TESTS WERE SUCCESSFUL!\n");
    }

    printf("Executing all tests took %lf secs\n", delta); 
}

char *chop_arg(int *argc, char *(**argv)) 
{
    if (*argc <= 0)
        return NULL;

    char *arg = **argv;
    *argv += 1;
    argc -= 1;

    return arg;
}

int main(int argc, char *argv[]) 
{
    char *program = chop_arg(&argc, &argv);
    Unused(program);
    bool Test = false;

    if (argc > 0) {
        char *flag = chop_arg(&argc, &argv);
        if (flag != NULL) {
            if (strcmp("test", flag) == 0) {
                Test = true;
            }
        }
    }

    if (Test) {
        test();
        return 0;
    }

    char buffer[1000];
    const size_t buffer_count = array_len(buffer);

    Parser parser;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    parser.ans = 0;
    double result;

    while (true) {
        memset(buffer, 0, buffer_count * sizeof(char));
        fgets(buffer, buffer_count, stdin);

        String str = {
            .str = buffer,
            .len = strlen(buffer),
        };

        parser.expr = str;

        bool parsed = parse_input(str, &parser.math);

        if (parsed) {
            parse_expression(&parser);

            result = do_the_math(parser.math);
            ANS = result;

            printf("Result: %lf\n", result);
        }

        list_clear(parser.math.num_list);
        list_clear(parser.math.oper_list);
    }

    return 0;
}
