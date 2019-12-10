/**
 * array.h: Arbitrary size array
 */

#ifndef ARRAY_H
#define ARRAY_H

typedef struct {
    size_t capacity;// Current capacity
    size_t bytes;   // Size of each elements
    size_t size;    // Number of elements
    void *data;     // Data
} array_t;

enum { ARRAY_DEFAULT_CAPACITY = 20 };

void array_append(array_t *array, void *e);
void array_delete(array_t *array);
void * array_get(array_t array, size_t i);
array_t array_new(size_t bytes);

#endif // ARRAY_H
