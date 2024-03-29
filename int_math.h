/**
 * int_math.h: Mathematical operations
 */

#ifndef INT_MATH_H
#define INT_MATH_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Word definitions
typedef  int64_t word_t;
typedef uint64_t uword_t;

// Word operations
static inline word_t wmin(word_t a, word_t b)
{
    return a > b ? b : a;
}

static inline word_t wmax(word_t a, word_t b)
{
    return a > b ? a : b;
}

static inline uword_t uwmin(uword_t a, uword_t b)
{
    return a > b ? b : a;
}

static inline uword_t uwmax(uword_t a, uword_t b)
{
    return a > b ? a : b;
}

static inline word_t wabs(word_t a)
{
    return a < 0 ? -a : a;
}

static inline size_t smin(size_t a, size_t b)
{
    return a < b ? a : b;
}

static inline size_t smax(size_t a, size_t b)
{
    return a < b ? b : a;
}

static inline ssize_t ssmin(ssize_t a, ssize_t b)
{
    return a < b ? a : b;
}

static inline ssize_t ssmax(ssize_t a, ssize_t b)
{
    return a < b ? b : a;
}

static inline long lmin(long a, long b)
{
    return a < b ? a : b;
}

static inline long lmax(long a, long b)
{
    return a < b ? b : a;
}

#endif // INT_MATH_H
