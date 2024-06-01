#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#define DEFAULT_LIST_CAP 32

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

void* GET_POPPED(void* *list_items, size_t *list_count, size_t *list_cap, size_t type_size) 
{
    void *popped = NULL; 

    if (*list_count == 0) return popped;

    if (*list_count < (*list_cap) / 3) {
        *list_cap /= 2;
        *list_items = realloc(*list_items, (*list_cap) * type_size);
    }

    *list_count -= 1;

    popped = (uint8_t*)(*list_items) + ((*list_count) * type_size);

    return popped;
}

#define list_pop(list, type) (*(type*)GET_POPPED((void*)(&(list).items), &(list).count, &(list).capacity, sizeof(type)))

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

#define Unused(var) (void) (var)

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

#define list_pool_alloc(list_pool, pool_cap)                                   \
  do {                                                                         \
    (list_pool).capacity = (pool_cap);                                         \
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
    assert((list_pool).count < (list_pool).capacity);                          \
    void *the_item = &item;                                                    \
    (list_pool).items[(list_pool).count] = *(LIST_BLUE_PRINT *)the_item;       \
    (list_pool).count += 1;                                                    \
  } while (0)

#define lp_at(list_pool, i, type) (*(type*)((void*)(&list_pool.items[i])))

#endif // _UTILS_H_
