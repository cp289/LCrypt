/**
 * array.c: Arbitrary size array
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"

// Initialize new array
array_t array_new(size_t bytes)
{
    array_t out = {
        .capacity = ARRAY_DEFAULT_CAPACITY,
        .bytes = bytes,
        .size = 0,
        .data = malloc(ARRAY_DEFAULT_CAPACITY * bytes),
    };
    return out;
}

// Append element e to array
void array_append(array_t *array, void *e)
{
    // Double the capacity if necessary
    if (array->size == array->capacity) {
        // Double array size
        array->capacity <<= 1;
        array->data = realloc(array->data, array->capacity * array->bytes);
    }

    // Append element e
    for (size_t i = 0; i < array->bytes; i++) {
        ((char*)array->data)[array->bytes * array->size + i] = ((char*)e)[i];
    }
    array->size++;
}

void * array_get(array_t array, size_t i)
{
    return (void*)((uintptr_t)array.data + (uintptr_t)array.bytes * i);
}

// Delete array
void array_delete(array_t *array)
{
    free(array->data);

    array->capacity = 0;
    array->size = 0;
    array->data = NULL;
}
