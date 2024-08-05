#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "list.h"
#include "strmap.h"
#include "string.h"
#include "utils.h"
#include "math.h"
#include "parser.h"

const char operators[] = "+-/*%^";
#define operator_count ((sizeof(operators) / sizeof(operators[0])) - 1)

bool is_let = false;
#define var_buff_cap 1000
char var_buffer[var_buff_cap];
char *var_ptr = var_buffer;
StrMap var_map;

double ANS = 0;

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
        if (is_digit(c)) {
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
        else if (c == 'p') {
            bool success = chop_word(&buffer, "pi", 2);
            if (!success) {
                fprintf(stderr, "Error: Invalid input.\n");
                return false;
            }
            isnum = true;
            list_push(output->num_list, PI);
        }
        else if (c == 'l') {
            if (! chop_word(&buffer, "let", 3)) {
                fprintf(stderr, "Error: Invalid input.\n");
                return false;
            }

            if (is_let) {
                fprintf(stderr, "Error: Can declare variable only in beginning of the expression.\n");
                return false;
            }

            String var = chop_name(&buffer);

            if (var.str == NULL) {
                fprintf(stderr, "Error: must put variable name after 'let'.\n");
                return false;
            }

            if (var_ptr + var.len > &var_buffer[var_buff_cap]) {
                fprintf(stderr, "Error: variable memory is full, can't delcare more variables.\n");
                return false;
            }

            trim_left(&buffer);

            if (*buffer.str != '=') {
                fprintf(stderr, "Error: Must put '=' after variable name.\n");
                return false;
            }

            chop_char_trim(&buffer);

            Parser parser;
            parser.expr = buffer;
            list_alloc(parser.math.num_list);
            list_alloc(parser.math.oper_list);
            parse_input(parser.expr, &parser.math);

            parse_expression(&parser);

            double result = do_the_math(parser.math);

            if (negate) {
                result *= -1;
                negate = false;
            }

            list_free(parser.math.num_list);
            list_free(parser.math.oper_list);

            char temp[var.len + 1];
            memcpy(temp, var.str, sizeof(char) * var.len);
            temp[var.len] = '\0';

            if (strmap_has(&var_map, temp)) {
                var_map.items[strmap_get_i(&var_map, temp)].value = result;                
            }
            else {
                strmap_add(&var_map, var_ptr, result);

                for (size_t i = 0; i < var.len; i++, var_ptr++) {
                    *var_ptr = var.str[i];
                }

                *var_ptr = '\0';
                var_ptr++;
            }

            is_let = true;
        }
        else if (c == '$') {
            chop_char(&buffer);

            String var = chop_name(&buffer);

            if (var.str == NULL) {
                fprintf(stderr, "Error: Invalid input.\n");
                return false;
            }

            trim_left(&buffer);
            
            char temp[var.len + 1];
            memcpy(temp, var.str, sizeof(char) * var.len);
            temp[var.len] = '\0';

            if (! strmap_has(&var_map, temp)) {
                fprintf(stderr, "Error: Variable '%s' doesn't exist.\n", temp);
                return false;
            }

            double result = strmap_get(&var_map, temp);

            isnum = true;
            list_push(output->num_list, result);
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
    is_let = false;
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
    double eps = 1e-3;
    double diff = expected_output - output;
    if (diff < 0) diff *= -1;
    if (diff <= eps) {
        success = true;
    } 
    else {
        success = false;
    }
    printf("    Output  : %0.15lf\n", output);
    printf("    Expected: %0.15lf\n", expected_output);
    if (success) {
        printf("    PASSED!\n");
    } 
    else {
        printf("    FAILED!\n");
    }
    printf("}\n");

    return success;
}

bool success = true;

#define EXPECTED(expr, EXPECTED_OUTPUT)                                        \
  do {                                                                         \
    char EXPR[] = expr;                                                        \
    if (!expected(EXPR, EXPECTED_OUTPUT)) {                                    \
      success = false;                                                         \
    }                                                                          \
  } while (0)

void test() 
{
    long time1 = clock();

    EXPECTED(
        "3 + 3",
        6
    );
    EXPECTED(
        "3 - 3",
        0
    );
    EXPECTED(
        "3 * 3",
        9
    );
    EXPECTED(
        "3 / 3",
        1
    );
    EXPECTED(
        "3 ^ 3",
        27
    );
    EXPECTED(
        "5 % 3",
        2
    );
    EXPECTED(
        "5 - -3",
        8
    );
    EXPECTED(
        "5 + -(2 * 3)",
        -1
    );
    EXPECTED(
        "5 * -1",
        -5
    );
    EXPECTED(
        "3 * 7 + 46 % 4",
        23
    );
    EXPECTED(
        "5 * 2 ^ 3 / 4 + 1 * 5",
        15
    );
    EXPECTED(
        "5 * 2 ^ 3 ^ 4 + 1 +  79",
        20560
    );
    EXPECTED(
        "1 + 3 *  9",
        28
    );
    EXPECTED(
        "64 ^ 0 / 2 + 6",
        6.5f
    );
    EXPECTED(
        "4 ^ 2 / 8 + 1",
        3
    );
    EXPECTED(
        "1 / 0.5 + 6",
        8
    );
    EXPECTED(
        "3 + 5 / 3 ^ 4 * 9 - 2 * 1",
        1.55555555555556
    );
    EXPECTED(
        "3 + 3 + 3 + 3 + 3 ^ 0 + 3",
        16
    );
    EXPECTED(
        "1 / 0.3 - 0.1 * 3 ^ 2",
        2.43333333333333
    );
    EXPECTED(
        "0 / 2 * 1",
        0
    );
    EXPECTED(
        "0/1*2",
        0
    );
    EXPECTED(
        "2 ^ 3 + 2",
        10
    );
    EXPECTED(
        "2 ^ (3 + 2)",
        32
    );
    EXPECTED(
        "9 / 3 - 2",
        1
    );
    EXPECTED(
        "9 / (3 - 2)",
        9
    );
    EXPECTED(
        "9 + 2 ^ 2 + 9 * 3 - 1 - 1",
        9 + pow(2, 2) + 9 * 3 - 1 - 1
    );
    EXPECTED(
        "9 + 2 ^ (2 + 9 * 3 - 1 - 1)",
        9 + pow(2, 2 + 9 * 3 - 1 - 1)
    );
    EXPECTED(
        "9 + 2 ^ (2 + 9 * (3 - 1) - 1)",
        9 + pow(2, 2 + 9 * (3 - 1) - 1)
    );
    EXPECTED(
        "0 / (3 - 1) + 9 * (2 ^ (1+1) + 3 * (3 * (1 - 1 + (3 / 1))))",
        0.f / (3 - 1) + 9 * (pow(2, 1 + 1) + 3 * (3 * (1 - 1 + (3.f / 1))))
    );
    EXPECTED(
        "90 / ( 32.32 * 32 ^ ( 3 / 2 ) ) + ( 3 * ( 32 % ( 4 ^ ( 2 * ( 1 + 1 ) ) ) ) )",
        90 / (32.32f * pow(32, 3.f / 2.f)) + (3 * (perform_operation(32.f, pow(4, 2 * (1 + 1)), '%')))
    );
    EXPECTED(
        "32 -(8 * -5 / 0.3 * (6 - 7) + -1) / 3.5 - -8",
        32 - (8 * -5 / 0.3f * (6 - 7) + -1) / 3.5 - -8
    );
    EXPECTED(
        "43 * 3",
        43 * 3
    );
    EXPECTED(
        "Ans + 1",
        (43 * 3) + 1
    );
    EXPECTED(
        "3 * ans * (ans ^ -2 + 1.3)",
        3 * ((43 * 3) + 1) * (pow((43 * 3) + 1, -2) + 1.3f)
    );
    EXPECTED(
        "\\sqrt(42)",
        sqrt(42)
    );
    EXPECTED(
        "\\sqrt(16) / 4 * (\\sqrt(4) + (27 - \\sqrt(9)))",
        sqrt(16) / 4 * (sqrt(4) + (27 - sqrt(9)))
    );
    EXPECTED(
        "\\sqrt(30) / (\\tan(50.3) ^ (\\sin(43) / \\cos(44)))",
        sqrt(30) / pow(tan(50.3), sin(43) / cos(44))
    );
    EXPECTED(
        "\\sqrt(\\sin(30) / 4.6 * (\\tan(30) / 30)) + 20 ^ 2 / 30 * (3 ^ 2 + 2)",
        sqrt(sin(30) / 4.6 * (tan(30) / 30)) + pow(20, 2) / 30 * (pow(3, 2) + 2)
    );
    EXPECTED(
        "\\max(3, 7)",
        max(3, 7)
    );
    EXPECTED(
        "\\max(43, \\sqrt(\\min(3 ^ 2, \\max(\\tan(43 * 2), 123 ) ) ) )",
        max(43, sqrt(min(pow(3, 3), max(tan(43 * 2), 123))))
    );
    EXPECTED(
        "64 / 3 / (32 - 31.3) * \\max(\\sqrt(3), \\min(3, 0.5)) - 6^2",
        64.f / 3 / (32 - 31.3) * max(sqrt(3), min(3, 0.5)) - pow(6, 2)
    );
    EXPECTED(
        "\\log(100)",
        log(100)
    );
    EXPECTED(
        "\\sinh(193)",
        sinh(193)
    );
    EXPECTED(
        "\\log10(100)",
        log10(100)
    );
    EXPECTED(
        "\\asin(1) + \\acos(1)",
        asin(1) + acos(1)
    );
    EXPECTED(
        "let var1 = \\cos(390) - (3 * 3) ^ 1.4",
        cos(390) - pow(3 * 3, 1.4)
    );
    EXPECTED(
        "$var1 / 39 + ($var1 - 30)",
        (cos(390) - pow(3 * 3, 1.4)) / 39 + ((cos(390) - pow(3 * 3, 1.4)) - 30)
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
    *argc -= 1;

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

    var_map = strmap_new();

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
        printf("> ");
        fgets(buffer, buffer_count, stdin);

        String str = {
            .str = buffer,
            .len = strlen(buffer),
        };

        parser.expr = str;

        is_let = false;
        bool parsed = parse_input(str, &parser.math);

        if (parsed) {
            parse_expression(&parser);

            result = do_the_math(parser.math);
            ANS = result;

            printf("= %0.15lf\n", result);
        }

        list_clear(parser.math.num_list);
        list_clear(parser.math.oper_list);
    }

    return 0;
}
