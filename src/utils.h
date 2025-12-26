#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char *read_text(const char *path);
void __unreachable(const char *file, int line, const char *func);

#define DYNAMIC_ARRAY_ADD(array, value)                                        \
{                                                                              \
    size_t index = array->len;                                                 \
    if (index >= array->capacity)                                              \
    {                                                                          \
        if (array->capacity == 0) array->capacity = 8;                         \
        else array->capacity *= 2;                                             \
        array->items = realloc(array->items, sizeof(value) * array->capacity); \
    }                                                                          \
    array->items[index] = value;                                               \
    array->len++;                                                              \
}

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))
#define UNREACHABLE __unreachable(__FILE__, __LINE__, __func__)
#define UNREACHABLE_RETURN(r) __unreachable(__FILE__, __LINE__, __func__); return r

#endif
