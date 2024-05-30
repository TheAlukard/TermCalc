#include <stdio.h>
#include <stdlib.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#define DEFAULT_LIST_CAP 10

#define list_alloc(list)                                                       \
  do {                                                                         \
    (list).capacity = DEFAULT_LIST_CAP;                                        \
    (list).count = 0;                                                          \
    (list).items = malloc((list).capacity * sizeof(*(list).items));            \
  } while (0)

#define list_free(list)                                                        \
  do {                                                                         \
    if ((list).items != NULL) {                                                \
      free((list).items);                                                      \
      (list).items = NULL;                                                     \
    }                                                                          \
    (list).count = 0;                                                          \
    (list).capacity = 0;                                                       \
  } while (0)

#define list_push(list, item)                                                          \
  do {                                                                                 \
    if ((list).count >= (list).capacity) {                                             \
      (list).capacity *= 2;                                                            \
      (list).items = realloc((list).items, (list).capacity * sizeof(*(list).items));   \
    }                                                                                  \
    (list).items[(list).count] = item;                                                 \
    (list).count += 1;                                                                 \
  } while (0)

#define list_pop(list, type) ({                                                         \
    type popped;                                                                        \
    if ((list).count > 0) {                                                             \
      (list).count -= 1;                                                                \
      popped = (list).items[(list).count];                                              \
      memset(&((list).items[(list).count]), 0, sizeof(type) * 1);                       \
      if ((list).count < (list).capacity / 3) {                                         \
        (list).capacity /= 2;                                                           \
        (list).items = realloc((list).items, (list).capacity * sizeof(*(list).items));  \
      }                                                                                 \
    }                                                                                   \
    popped;                                                                             \
})

#define list_copy(dest, src, start, count)                                     \
  do {                                                                         \
    if ((start) < 0) {                                                         \
      break;                                                                   \
    }                                                                          \
    size_t i = (start);                                                        \
    size_t j = 0;                                                              \
    while (i < (count) && i < (src).count && j < (dest).count) {               \
      (dest).items[j] = (src).items[i];                                        \
      i += 1;                                                                  \
      j += 1;                                                                  \
    }                                                                          \
  } while (0)

#define list_transfer(dest, src)                                                 \
  do {                                                                           \
    if ((dest).items != NULL) {                                                  \
      free((dest).items);                                                        \
      (dest).items = NULL;                                                       \
    }                                                                            \
    (dest).items = (src).items;                                                  \
    (dest).capacity = (src).capacity;                                            \
    (dest).count = (src).count;                                                  \
  } while (0)

#define list_print(list, format)                                               \
  do {                                                                         \
    printf("[");                                                               \
    for (size_t i = 0; i < (list).count; i++) {                                \
      printf(format, (list).items[i]);                                         \
      if (i < (list).count - 1) {                                              \
        printf(", ");                                                          \
      }                                                                        \
    }                                                                          \
    printf("]\n");                                                             \
  } while (0)

#define list_clear(list) ((list).count = 0)

typedef struct {
    void *items;
    size_t capacity;
    size_t count;
} LIST_BLUE_PRINT;

typedef struct {
    LIST_BLUE_PRINT *items;
    size_t capacity;
    size_t count;
} List_Pool;

#define list_pool_alloc(list_pool)                                             \
  do {                                                                         \
    (list_pool).capacity = DEFAULT_LIST_CAP;                                   \
    (list_pool).items =                                                        \
        malloc((list_pool).capacity * sizeof(LIST_BLUE_PRINT));                \
    (list_pool).count = 0;                                                     \
  } while (0)

#define list_pool_free(list_pool)                                              \
  do {                                                                         \
    for (size_t i = 0; i < (list_pool).count; i++) {                           \
      list_free((list_pool).items[i]);                                         \
    }                                                                          \
    free((list_pool).items);                                                   \
    (list_pool).items = NULL;                                                  \
    (list_pool).capacity = 0;                                                  \
    (list_pool).count = 0;                                                     \
  } while (0)

#define list_pool_push(list_pool, item)                                        \
  do {                                                                         \
    if ((list_pool).count >= (list_pool).capacity) {                           \
      (list_pool).capacity *= 2;                                               \
      (list_pool).items = realloc(                                             \
          (list_pool).items, (list_pool).capacity * sizeof(LIST_BLUE_PRINT));  \
    }                                                                          \
    void *the_item = &item;                                                    \
    (list_pool).items[(list_pool).count] = *(LIST_BLUE_PRINT *)the_item;       \
    (list_pool).count += 1;                                                    \
  } while (0)

#define lp_at(list_pool, i, type) *(type*)((void*)(&list_pool.items[i]))

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

int main(void)
{
    List_Pool pool;
    list_pool_alloc(pool);

    Stackc _stackc;
    list_alloc(_stackc);
    Stackd _stackd;
    list_alloc(_stackd);

    list_pool_push(pool, _stackc);
    list_pool_push(pool, _stackd);

    Stackc *stackc = &lp_at(pool, 0, Stackc);
    Stackd *stackd = &lp_at(pool, 1, Stackd);

    for (char c = '0'; c < '0' + 20; c++) {
        list_push(*stackc, c); 
    }
    for (int i = 0; i < 20; i++) {
        list_push(*stackd, i);
    }

    list_print(*stackc, "%c");
    list_print(*stackd, "%lf");

    return 0;
}