/**
 * math.h: Mathematical operations
 */

#ifndef MATH_H
#define MATH_H

// NOTE: bigint.h includes int_math.h
#include "bigint.h"

// Return the negative of the input
bigint_t bigint_neg(bigint_t n);

// Sum of a and b
bigint_t bigint_sum(bigint_t a, bigint_t b);

// Return the difference a - b
bigint_t bigint_diff(bigint_t a, bigint_t b);

// Return whether a == b, i.e. whether a - b == 0
bool bigint_equals(bigint_t a, bigint_t b);

// Integer multiplication a * b
bigint_t bigint_prod(bigint_t a, bigint_t b);

// Integer division a/b
bigint_t bigint_div(bigint_t a, bigint_t b, bigint_t *rem);

// Greatest Common Divisor
bigint_t bigint_gcd(bigint_t a, bigint_t b);

// Extended euclidean algorithm
bigint_t bigint_xgcd(bigint_t a, bigint_t b, bigint_t *x, bigint_t *y);

// Testing methods
int mod_test(void);

#endif // MATH_H
