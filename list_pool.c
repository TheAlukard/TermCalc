#include "utils.h"

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