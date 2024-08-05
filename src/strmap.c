#include "strmap.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

uint32_t hash(char *str)
{
    uint32_t hash = 5381;
    int c = 1;

    while (c != 0) {
        c = *str;
        hash = ((hash << 5) + hash) + c;
        str++;
    }

    return hash;
}

StrMap strmap_new()
{
    StrMap map;
    map.capacity = DEFAULT_STRMAP_CAP;
    map.count = 0;
    map.items = (StrMap_Node*)calloc(map.capacity, sizeof(StrMap_Node));
    map.hash = hash;

    return map;
}

uint32_t strmap_get_hash(StrMap *map, KEY key)
{
    return map->hash(key) % map->capacity;
}

uint32_t strmap_get_i(StrMap *map, KEY key)
{
    uint32_t hash_value = strmap_get_hash(map, key);

    for (uint32_t i = 0; i < map->capacity; i++) {
        uint32_t index = (hash_value + i) % map->capacity;

        if (map->items[index].valid && CMP(key, map->items[index].key)) {
            return index;
        }
    }

    return 0;
}

bool strmap_add(StrMap *map, KEY key, VALUE value)
{
    uint32_t hash_value = strmap_get_hash(map, key);
    
    for (uint32_t i = 0; i < map->capacity; i++) {
        uint32_t index = (hash_value + i) % map->capacity;

        if (! map->items[index].valid) {
            map->items[index].key = key;
            map->items[index].value = value;
            map->items[index].valid = true;
            map->count++;

            return true;
        }
        else if (CMP(key, map->items[index].key)) { 
            map->items[index].value = value;

            return true;
        }
    }
    
    return false;
}

VALUE strmap_get(StrMap *map, KEY key)
{
    uint32_t hash_value = strmap_get_hash(map, key);
    
    for (uint32_t i = 0; i < map->capacity; i++) {
        uint32_t index = (hash_value + i) % map->capacity;

        if (map->items[index].valid && CMP(key, map->items[index].key)) {
            return map->items[index].value;
        }
    }
    
    return (VALUE){0};
}

bool strmap_has(StrMap *map, KEY key)
{
    uint32_t hash_value = strmap_get_hash(map, key);
    
    for (uint32_t i = 0; i < map->capacity; i++) {
        uint32_t index = (hash_value + i) % map->capacity;

        if (map->items[index].valid && CMP(key, map->items[index].key)) {
            return true;
        }
    }
    
    return false;
}
