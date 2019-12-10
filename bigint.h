/**
 * bigint.h: Arbitrary-length integer library
 */

#ifndef BIGINT_H
#define BIGINT_H

#include <stdbool.h>
#include <stddef.h>

#include "math.h"

// Storage sizes
enum {
    BITS_PER_BYTE = 8,
    WORD_BITS = BITS_PER_BYTE * sizeof(uword_t),
};

typedef struct {
    size_t  size;
    uword_t  *val;
} bigint_t;

// Free bigint
void bigint_delete(bigint_t *n);

// Return zero of given size in words
bigint_t bigint_zero(size_t size);

// Return -1 of given size in words
bigint_t bigint_minus1(size_t size);

// Return whether n is zero
bool is_zero(bigint_t n);

// Return whether n is negative (negative flag set)
bool is_neg(bigint_t n);

// Return whether n is positive (not negative nor zero)
bool is_pos(bigint_t n);

bigint_t bigint_min(bigint_t a, bigint_t b);

bigint_t bigint_max(bigint_t a, bigint_t b);

// Return the logical negation of the input
bigint_t bigint_lneg(bigint_t n);

long bigint_to_long(bigint_t n);
bigint_t long_to_bigint(long n);

// Return minimum number of words needed for n
size_t bigint_min_words(bigint_t n);

// Resize bigint to specified number of words
bigint_t bigint_resize(bigint_t n, size_t size);

// Make copy of n
bigint_t bigint_copy(bigint_t n);

// Return the negative of the input
bigint_t bigint_neg(bigint_t n);

// Sum of a and b
bigint_t bigint_sum(bigint_t a, bigint_t b);

// Return the difference a - b
bigint_t bigint_diff(bigint_t a, bigint_t b);

// n >> k (in place)
// NOTE It must be the case that k <= WORD_BITS
bigint_t ip_sr(bigint_t n, size_t k);

// n << k (in place)
// TODO support k > WORD_BITS
bigint_t ip_sl(bigint_t n, size_t k);

// n << k
bigint_t bigint_sl(bigint_t n, size_t k);

// n >> k
bigint_t bigint_sr(bigint_t n, size_t k);

// Print each word in hex
void bigint_print_words(bigint_t n);

// Integer multiplication a * b
bigint_t bigint_prod(bigint_t a, bigint_t b);

// Integer division a/b
bigint_t bigint_div(bigint_t a, bigint_t b, bigint_t *rem);

// Return integer with value specified by decimal string
bigint_t bigint_new(char *string);

// Print n in base 10
char * bigint_print(bigint_t n);

// Print n in hexadecimal
char * bigint_print_hex(bigint_t n);

// Greatest Common Divisor
bigint_t bigint_gcd(bigint_t a, bigint_t b);

// Extended euclidean algorithm
bigint_t bigint_xgcd(bigint_t a, bigint_t b, bigint_t *x, bigint_t *y);

// Initialize bigint runtime data structures
void bigint_init(void);
// Free bigint runtime data structures
void bigint_exit(void);
void bigint_test(void);

#endif // BIGINT_H
