#pragma once

#ifndef STR_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KEY char* 
#define VALUE double

#define CMP(key, other) (strcmp(key, other) == 0) 

typedef struct {
    KEY key;
    VALUE value;
    bool valid;
} StrMap_Node;

typedef struct {
    StrMap_Node *items;
    size_t count;
    size_t capacity;
    uint32_t(*hash)(KEY);
} StrMap;

#define DEFAULT_STRMAP_CAP 150

StrMap strmap_new(void);
bool strmap_add(StrMap *map, KEY key, VALUE value);
VALUE strmap_get(StrMap *map, KEY key);
uint32_t strmap_get_hash(StrMap *map, char *key);
uint32_t strmap_get_i(StrMap *map, char *key);
bool strmap_has(StrMap *map, KEY key);
uint32_t strmap_get_hash(StrMap *map, KEY key);
uint32_t strmap_get_i(StrMap *map, KEY key);

#define STR_MAP_H

#endif // STR_MAP_H
