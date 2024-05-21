#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef linux 
    char* strlwr(char *str)
    {
        unsigned char *p = (unsigned char *)str;

        while (*p) {
            *p = tolower((unsigned char)*p);
            p++;
        }

        return str;
    }
#endif

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#define DEFAULT_LIST_CAP 32

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

#define list_push(list, item)                                                  \
  do {                                                                         \
    if (list.count >= list.capacity) {                                         \
      list.capacity *= 2;                                                      \
      list.items = realloc(list.items, list.capacity * sizeof(*list.items));   \
    }                                                                          \
    list.items[list.count] = item;                                             \
    list.count += 1;                                                           \
  } while (0)

#define list_pop(list, type) ({                                                \
    type popped;                                                               \
    if (list.count > 0) {                                                      \
      list.count -= 1;                                                         \
      popped = list.items[list.count];                                         \
      memset(&(list.items[list.count]), 0, sizeof(type) * 1);                  \
      if (list.count < list.capacity / 3) {                                    \
        list.capacity /= 2;                                                    \
        list.items = realloc(list.items, list.capacity * sizeof(*list.items)); \
      }                                                                        \
    }                                                                          \
    popped;                                                                    \
})

#define list_copy(dest, src, start, count)                                     \
  do {                                                                         \
    if (start < 0) {                                                           \
      break;                                                                   \
    }                                                                          \
    size_t i = start;                                                          \
    size_t j = 0;                                                              \
    while (i < count && i < src.count && j < dest.count) {                     \
      dest.items[j] = src.items[i];                                            \
      i += 1;                                                                  \
      j += 1;                                                                  \
    }                                                                          \
  } while (0)

#define list_transfer(dest, src)                                               \
  do {                                                                         \
    if (dest.items != NULL) {                                                  \
      free(dest.items);                                                        \
      dest.items = NULL;                                                       \
    }                                                                          \
    dest.items = src.items;                                                    \
    dest.capacity = src.capacity;                                              \
    dest.count = src.count;                                                    \
  } while (0)

#define list_print(list, format)                                               \
  do {                                                                         \
    printf("[");                                                               \
    for (size_t i = 0; i < list.count; i++) {                                  \
      printf(format, list.items[i]);                                           \
      if (i < list.count - 1) {                                                \
        printf(", ");                                                          \
      }                                                                        \
    }                                                                          \
    printf("]\n");                                                             \
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
    double ans;
} Parser;

bool parse_input(char *buffer, size_t buffer_count, Math *output);
void parse_operations(Parser *parser, char *ops, size_t ops_count);
void parse_expression(Parser *parser);
double do_the_math(const Math math);

void print_math(const Math math)
{
    printf("Math:\n");

    for (size_t i = 0; i < math.num_list.count; i++) {
        printf("    %lf \n",math.num_list.items[i]);
        if (i < math.oper_list.count) {
            printf("    %c \n", math.oper_list.items[i]);
        }
    }
    printf("\n");
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
            fprintf(stderr, "%s:%d:1 Invalid operation: '%c'\n", __FILE__, __LINE__, operation);
            exit(1);
    }
}

double chop_num(char **buffer, size_t buffer_count)
{
    size_t count = 0;
    char *pos = *buffer;
    char *start = *buffer;
    bool isdot = false;
    while (*pos != 0) {
        if (isdigit(*pos)) {
            start = pos;
            while (*pos != 0 && (isdigit(*pos) == true || *pos == '.')) {
                if (*pos == '.') {
                    if (isdot) {
                        return 0;
                    }
                    else {
                        isdot = true;
                    }
                } 
                count++;
                pos++;
            }
            char temp[count + 1];
            memcpy(temp, start, count * sizeof(char));
            temp[count] = '\0';
            char *end;
            double result = strtod(temp, &end);
            *buffer = pos;
            return result;
        }

        pos++;
    }

    return 0;
}

bool chop_word(char **buffer, char* word, size_t word_size)
{
    if (**buffer != *word) {
        return false;
    }

    char *pos = *buffer;
    size_t i = 0;

    while (*pos != '\0' && i < word_size) {
        if (*pos != *word) {
            return false;
        }

        pos++;
        word++;
        i++;
    }

    if (i != word_size) {
        return false;
    }

    *buffer = pos;

    return true;
}

char* chop_paren(char **buffer, char *endPtr)
{
    if (**buffer != '(') {
        return NULL;
    }

    char *start = *buffer + 1;
    char *pos = start;
    char *location = start;
    bool ended = false;
    size_t countered = 0;

    while (pos < endPtr && *pos != 0) {
        if (*pos == ')') {
            ended = true;
            location = pos;
            if (countered == 0) {
                break;
            }
            else {
                countered--;
            }
        }
        else if (*pos == '(') {
            countered++;
        }

        pos++;
    }

    if (! ended) {
        return NULL;
    }

    size_t size = location - start;
    char *new_expr = (char*)malloc((size + 1) * sizeof(char));
    memcpy(new_expr, start, size);
    new_expr[size] = '\0';
    *buffer = location + 1;

    return new_expr;
}

double ANS = 0;

bool parse_input(char *buffer, size_t buffer_count, Math *output)
{
    remove_white_spaces(buffer, buffer_count);
    strlwr(buffer);

    char *pos = buffer;
    size_t i = 0;
    bool isnum = false;
    bool negate = false;
    if (*pos == '-') {
        negate = true;
    }

    if (*pos == 'e') {
        bool _exit = chop_word(&pos, "exit", 4);
        if (! _exit) {
            fprintf(stderr, "Error: Invalid input.\n");
            return false;
        }
        exit(0);
    }

    while (i < buffer_count && *pos != 0) {
        if (isdigit(*pos)) {
            double num = chop_num(&pos, buffer_count - i);
            i = pos - buffer;
            isnum = true;
            if (negate) {
                num *= -1;
                negate = false;
            }
            list_push(output->num_list, num);
        }
        else if (str_contains(operators, operator_count, *pos)) {
            if (! isnum) {
                fprintf(stderr, "Error: Operator wasn't preceeded by a number\n");
                return false;
            }

            list_push(output->oper_list, *pos);
            isnum = false;
            i++;
            pos++;

            if (i < buffer_count && *pos == '-') {
                negate = true;
                pos++;
                i++;
            }
        }
        else if (*pos == '(') {
            char *current = pos;
            Parser parser;
            parser.expr = chop_paren(&pos, buffer + buffer_count);
            if (parser.expr == NULL) {
                fprintf(stderr, "Error: Invalid parenthesis.\n");
                return false;
            }
            i = pos - current;
            list_alloc(parser.math.num_list);
            list_alloc(parser.math.oper_list);
            parse_input(parser.expr, strlen(parser.expr), &parser.math);
            parse_expression(&parser);
            double result = do_the_math(parser.math);
            if (negate) {
                result *= -1;
                negate = false;
            }
            list_push(output->num_list, result);
            free(parser.expr);
            list_free(parser.math.num_list);
            list_free(parser.math.oper_list);
            isnum = true;
        }
        else if (*pos == 'a') {
            char *current = pos;
            bool success = chop_word(&pos, "ans", 3);
            if (! success) {
                fprintf(stderr, "Error: Invalid input.\n");
                return false;
            }
            isnum = true;
            i = pos - current;
            list_push(output->num_list, ANS);
        }
        else {
            fprintf(stderr, "Error: Invalid input.\n");
            return false;
        }
    }  

    return true;
}

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

void parse_operations(Parser *parser, char *ops, size_t ops_count)
{
    if (parser->math.oper_list.count != parser->math.num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:\n", __FILE__, __LINE__);
        print_math(parser->math);
        exit(1);
    }

    Stackd num_stack;
    Stackc oper_stack;
    list_alloc(num_stack);
    list_alloc(oper_stack);
    list_push(num_stack, parser->math.num_list.items[0]);

    size_t j = 1;
    for (size_t i = 0; i < parser->math.oper_list.count; i++) {
        char operator = parser->math.oper_list.items[i];
        if (str_contains(ops, ops_count, operator)) {
            double num = list_pop(num_stack, double);
            double result = perform_operation(num, parser->math.num_list.items[i + 1], operator);
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


void parse_expression(Parser *parser)
{
    remove_white_spaces(parser->expr, strlen(parser->expr));

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
    parser.ans = 0;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    parse_input(input, strlen(input) + 1, &parser.math);

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
    if (((int64_t)output ^ (int64_t)expected_output) != 0) {
        printf("    \033[31mOutput  : %lf\n", output);
        success= false;
    }
    else {
        printf("    Output  : %lf\n", output);
        success = true;
    }
    printf("    \033[0mExpected: %lf\n", expected_output);
    printf("}\n");

    return success; 
}

#define EXPECTED(expr, EXPECTED_OUTPUT, success)                               \
  do {                                                                         \
    char EXPR[] = expr;                                                        \
    if (! expected(EXPR, EXPECTED_OUTPUT)) {                                   \
      success = false;                                                          \
    }                                                                          \
  } while (0)

void test()
{
    bool success = true;
    EXPECTED("3 + 3", 6, success);
    EXPECTED("3 - 3", 0, success);
    EXPECTED("3 * 3", 9, success);
    EXPECTED("3 / 3", 1, success);
    EXPECTED("3 ^ 3", 27, success);
    EXPECTED("5 % 3", 2, success);
    EXPECTED("5 - -3", 8, success);
    EXPECTED("5 + -(2 * 3)", -1, success);
    EXPECTED("5 * -1", -5, success);
    EXPECTED("3 * 7 + 46 % 4", 23, success);
    EXPECTED("5 * 2 ^ 3 / 4 + 1 * 5", 15, success);
    EXPECTED("5 * 2 ^ 3 ^ 4 + 1 +  79", 20560, success);
    EXPECTED("1 + 3 *  9", 28, success);
    EXPECTED("64 ^ 0 / 2 + 6", 6.5f, success);
    EXPECTED("4 ^ 2 / 8 + 1", 3, success);
    EXPECTED("1 / 0.5 + 6", 8, success);
    EXPECTED("3 + 5 / 3 ^ 4 * 9 - 2 * 1", 1.55555555555556, success);
    EXPECTED("3 + 3 + 3 + 3 + 3 ^ 0 + 3", 16, success);
    EXPECTED("1 / 0.3 - 0.1 * 3 ^ 2", 2.43333333333333, success);
    EXPECTED("0 / 2 * 1", 0, success);
    EXPECTED("0/1*2", 0, success);
    EXPECTED("2 ^ 3 + 2", 10, success);
    EXPECTED("2 ^ (3 + 2)", 32, success);
    EXPECTED("9 / 3 - 2", 1, success);
    EXPECTED("9 / (3 - 2)", 9, success);
    EXPECTED("9 + 2 ^ 2 + 9 * 3 - 1 - 1", 9 + pow(2, 2) + 9 * 3 -1 -1, success);
    EXPECTED("9 + 2 ^ (2 + 9 * 3 - 1 - 1)", 9 + pow(2, 2 + 9 * 3 - 1 - 1), success);
    EXPECTED("9 + 2 ^ (2 + 9 * (3 - 1) - 1)", 9 + pow(2, 2 + 9 * (3 - 1) - 1), success);
    EXPECTED(
        "0 / (3 - 1) + 9 * (2 ^ (1+1) + 3 * (3 * (1 - 1 + (3 / 1))))", 
        0.f / (3 - 1) + 9 * (pow(2, 1+1) + 3 * (3 * (1 - 1 + (3.f / 1)))), 
        success
    );
    EXPECTED(
        "90 / ( 32.32 * 32 ^ ( 3 / 2 ) ) + ( 3 * ( 32 % ( 4 ^ ( 2 * ( 1 + 1 ) ) ) ) )", 
        90 / (32.32f * pow(32, 3.f / 2.f)) + (3 * (perform_operation(32.f, pow(4, 2 * (1 + 1)), '%'))), 
        success
    );
    EXPECTED(
        "32 -(8 * -5 / 0.3 * (6 - 7) + -1) / 3.5 - -8",
        32 -(8 * -5 / 0.3f * (6 - 7) + -1) / 3.5 - -8,
        success
    );
    EXPECTED("43 * 3", 43 * 3, success);
    EXPECTED("Ans + 1", (43 * 3) + 1, success);
    EXPECTED("3 * ans * (ans ^ -2 + 1.3)", 3 * ((43 * 3) + 1) * (pow((43 * 3) + 1, -2) + 1.3f), success);

    if (success) {
        printf("\033[32mALL TESTS WERE SUCCESSFUL!\n");
    }
    else {
        printf("\033[31mNOT ALL TESTS WERE SUCCESSFUL!\n");
    }
}

char* chop_arg(int *argc, char *(**argv))
{
    if (argc <= 0) return NULL;


    char* arg = **argv;
    *argv += 1;
    argc -= 1;

    return arg;
}

int main(int argc, char* argv[])
{
    char *program = chop_arg(&argc, &argv);
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

    while (true)
    {
        memset(buffer, 0, buffer_count * sizeof(char));
        fgets(buffer, buffer_count, stdin);
        parser.expr = buffer;

        bool parsed = parse_input(buffer, buffer_count, &parser.math); 

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
