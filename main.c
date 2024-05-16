#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#define contains(list, count, item, result)                                    \
  do {                                                                         \
    result = false;                                                            \
    for (size_t i = 0; i < count; i++) {                                       \
      if (list[i] == item) {                                                   \
        result = true;                                                         \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  } while (0)

#define DEFAULT_LIST_CAP 16

#define list_alloc(list)                                                       \
  do {                                                                         \
    list.capacity = DEFAULT_LIST_CAP;                                          \
    list.count = 0;                                                            \
    list.items = malloc(list.capacity * sizeof(*list.items));                  \
  } while (0)

#define list_free(list)                                                        \
  do {                                                                         \
    if (list.items != NULL) {                                                  \
      free(list.items);                                                        \
      list.items = NULL;                                                       \
    }                                                                          \
    list.count = 0;                                                            \
    list.capacity = 0;                                                         \
  } while (0)

#define list_append(list, item)                                                \
  do {                                                                         \
    if (list.count >= list.capacity) {                                         \
      list.capacity *= 2;                                                      \
      list.items = realloc(list.items, list.capacity * sizeof(*list.items));   \
    }                                                                          \
    list.items[list.count] = item;                                             \
    list.count += 1;                                                           \
  } while (0)

#define list_clear(list) (list.count = 0)

const char operators[] = "+-/*%^"; 
#define operator_count ((sizeof(operators) / sizeof(operators[0])) - 1)

typedef struct {
    double *items;
    size_t count;
    size_t capacity;
} NumList;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} OperList; 

typedef struct {
    NumList num_list;
    OperList oper_list;
} Math;

typedef struct {
    char *expr;
    Math math;
} Parser;

Parser new_parser(char *expr)
{
    Parser parser;
    parser.expr = expr;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);

    return parser;
}

bool str_contains(const char* str, size_t count, char item)
{
    for (size_t i = 0; i < count; i++) {
        if (str[i] == item) return true;
    }

    return false;
}

void remove_white_spaces(char* str, size_t count)
{
    size_t i = 0;
    for (size_t j = 0; j < count - 1; j++) {
        while (j < count - 1 && isspace(str[j])) {
            j++; 
        }
        str[i] = str[j];
        if (i != j) str[j] = ' ';
        i++;
    }
    memset(str + i, 0, count - (i + 1));
}

double perform_operation(double num1, double num2, char operation)
{
    switch(operation) {
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
            fprintf(stderr, "%s:%d:1 Invalid operation: '%c'", __FILE__, __LINE__, operation);
            exit(1);
    }
}

bool parse_input(char *buffer, size_t buffer_count, Math *output)
{
    remove_white_spaces(buffer, buffer_count);

    size_t count = 0;
    char *position = buffer;

    while (*position != 0) {

        // printf("Buffer:\n%s\n", buffer);

        char current = *position;

        if (isdigit(current) || current == '.') {
            count += 1;
        }

        else if (str_contains(operators, operator_count, current)) {
            if (count == 0) {
                fprintf(stderr, "Error: Entered operator before entering any number.\n");
                exit(1);
            }

            char temp[count + 1];
            memcpy(temp, buffer, count * sizeof(char));
            temp[count] = '\0';
            char *end;
            double num = strtod(temp, &end);

            list_append(output->num_list, num);
            list_append(output->oper_list, current);

            // printf("{\n");
            // printf("    Buffer : %s\n", buffer);
            // printf("    Temp   : %s\n", temp);
            // printf("    Number : %lf\n", num);
            // printf("    Current: %c\n", current);
            // printf("    Count  : %zu\n", count);
            // printf("}\n");

            buffer += count + 1;
            count = 0;
        }

        position++;
    }

    char *end;
    double num = strtod(buffer, &end);
    list_append(output->num_list, num);

    if (output->num_list.count <= 0) {
        printf("Next time enter a number.");
    }

    if (output->oper_list.count != output->num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:", __FILE__, __LINE__);
        exit(1);
    }

    return true;
}

void parse_operations(Parser *parser, char *ops, size_t ops_count)
{
    if (parser->math.oper_list.count != parser->math.num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:\n", __FILE__, __LINE__);

        for (size_t i = 0; i < parser->math.num_list.count; i++) {
            printf("%lf ",parser->math.num_list.items[i]);
            if (i < parser->math.oper_list.count) {
                printf("%c ", parser->math.oper_list.items[i]);
            }
        }

        printf("\n");
        exit(1);
    }

    Math new;
    list_alloc(new.num_list);
    list_alloc(new.oper_list);

    double result;
    bool checked = false;

    for (size_t i = 0; i < parser->math.oper_list.count; i++) {
        if (str_contains(ops, ops_count, parser->math.oper_list.items[i])) {
            double num;
            if (! checked) {
                num = parser->math.num_list.items[i];
                checked = true;
            }
            else {
                num = result;
            }
            result = perform_operation(num, parser->math.num_list.items[i + 1], parser->math.oper_list.items[i]);

            if (i == parser->math.oper_list.count - 1) {
                list_append(new.num_list, result);
            }
        }
        else {
            if (checked) {
                list_append(new.num_list, result);
                checked = false;
            }
            else {
                list_append(new.num_list, parser->math.num_list.items[i]);
            }
            list_append(new.oper_list, parser->math.oper_list.items[i]);
            if (i == parser->math.oper_list.count - 1) {
                list_append(new.num_list, parser->math.num_list.items[i + 1]);
            }
        }
    }

    list_free(parser->math.num_list);
    list_free(parser->math.oper_list);

    parser->math = new;
}


void parse_expression(Parser *parser)
{
    remove_white_spaces(parser->expr, strlen(parser->expr));

    // printf("%s\n", parser->expr);
    
    parse_operations(parser, "^", 1);
    parse_operations(parser, "*/%", 3);
}

double do_the_math(const Math math)
{
    double result = math.num_list.items[0];

    for (size_t i = 1; i < math.num_list.count; i++) {
        result = perform_operation(result, math.num_list.items[i], math.oper_list.items[i - 1]);
    }

    return result;
}

bool expected(char* input, double expected_output)
{
    Parser parser;
    parser.expr = input;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    parse_input(input, strlen(input) + 1, &parser.math);

    // for (size_t i = 0; i < parser.math.num_list.count; i++) {
    //     printf("%lf ",parser.math.num_list.items[i]);
    //     if (i < parser.math.oper_list.count) {
    //         printf("%c ", parser.math.oper_list.items[i]);
    //     }
    // }

    parse_expression(&parser);
    double output = do_the_math(parser.math);

    list_free(parser.math.num_list);
    list_free(parser.math.oper_list);

    printf("{\n");
    printf("    Input   : %s\n", input);
    printf("    Output  : %lf\n", output);
    printf("    Expected: %lf\n", expected_output);
    printf("}\n");

    return output == expected_output; 
}


void test()
{
    char ex_0[] = "3 * 7 + 46 % 4";
    char ex_1[] = "5 * 2 ^ 3 / 4 + 1 * 5";
    char ex_2[] = "5 * 2 ^ 3 ^ 4 + 1 +  79";
    char ex_3[] = "1 + 3 *  9";
    char ex_4[] = "64 ^ 0 / 2 + 6";
    char ex_5[] = "4 ^ 2 / 8 + 1";
    char ex_6[] = "1 / 0.5 + 6";
    char ex_7[] = "3 + 5 / 3 ^ 4 * 9 - 2 * 1";
    char ex_8[] = "3 + 3 + 3 + 3 + 3 ^ 0 + 3";
    char ex_9[] = "1 / 0.3 - 0.1 * 3 ^ 2";
    char ex_10[] = "0 / 2 * 1";
    char ex_11[] = "0/1*2";

    expected(ex_0, 23);
    expected(ex_1, 15);
    expected(ex_2, 20560);
    expected(ex_3, 28);
    expected(ex_4, 6.5f); 
    expected(ex_5, 3);
    expected(ex_6, 8);
    expected(ex_7, 1.55555555555556);
    expected(ex_8, 16);
    expected(ex_9, 2.43333333333333);
    expected(ex_10, 0);
    expected(ex_11, 0);
}

int main(void)
{
#if 0
    test(); 
    return 0;
#endif

    char buffer[1000];
    const size_t buffer_count = array_len(buffer);

    Parser parser;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    
    while (true)
    {
        memset(buffer, 0, buffer_count * sizeof(char));
        fgets(buffer, buffer_count, stdin);
        parser.expr = buffer;

        parse_input(buffer, buffer_count, &parser.math); 
        parse_expression(&parser);

        double result = do_the_math(parser.math);

        printf("Result: %lf\n", result);

        list_clear(parser.math.num_list);
        list_clear(parser.math.oper_list);
    }

    return 0;
}