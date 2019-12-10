/**
 * bigint.c: Arbitrary-length integer library
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bigint.h"
#include "array.h"
#include "math.h"

// Free bigint
void bigint_delete(bigint_t *n)
{
    n->size = 0;
    free(n->val);
}

// Return zero of given size in words
bigint_t bigint_zero(size_t size)
{
    return (bigint_t) {
        .size = size,
        .val = calloc(size, sizeof(uword_t)),
    };
}

// Return -1 of given size in words
bigint_t bigint_minus1(size_t size)
{
    bigint_t out = {
        .size = size,
        .val = malloc(size * sizeof(uword_t)),
    };

    char fill = ~(char)0;
    memset(out.val, fill, size * sizeof(uword_t));

    return out;
}

// Return whether n is zero
bool is_zero(bigint_t n)
{
    for (size_t i = 0; i < n.size; i++)
        if (n.val[i]) return false;
    return true;
}

// Return whether n is negative (negative flag set)
bool is_neg(bigint_t n)
{
    if ( n.val[n.size-1] & ((uword_t)1 << (WORD_BITS-1)) )
        return true;
    return false;
}

// Return whether n is positive (not negative nor zero)
bool is_pos(bigint_t n)
{
    if ( is_neg(n) || is_zero(n))
        return false;
    return true;
}

// Return min(a, b)
bigint_t bigint_min(bigint_t a, bigint_t b)
{
    bigint_t diff = bigint_diff(a, b);
    bool neg = is_neg(diff);
    bigint_delete(&diff);

    if (neg)
        return bigint_copy(a);
    return bigint_copy(b);
}

// Return max(a, b)
bigint_t bigint_max(bigint_t a, bigint_t b)
{
    bigint_t diff = bigint_diff(a, b);
    bool neg = is_neg(diff);
    bigint_delete(&diff);

    if (neg)
        return bigint_copy(b);
    return bigint_copy(a);
}

// Return the logical negation of the input
bigint_t bigint_lneg(bigint_t n)
{
    bigint_t out = { .size = n.size, .val = malloc(n.size * sizeof(uword_t)) };

    // Take logical negation of n
    for (size_t i = 0; i < n.size; i++) {
        out.val[i] = ~n.val[i];
    }

    return out;
}

long bigint_to_long(bigint_t n)
{
    long out = 0;
    uint8_t *p = (uint8_t*)n.val;

    size_t n_bytes = smin(n.size * sizeof(uword_t), sizeof(long));

    if (is_neg(n)) {
        out = ~(((long)1 << n_bytes * BITS_PER_BYTE) - 1);
    }
    for (size_t i = 0; i < n_bytes; i++) {
        out |= (long)p[i] << i * BITS_PER_BYTE;
    }

    return out;
}

bigint_t long_to_bigint(long n)
{
    size_t size = sizeof(long) / sizeof(uword_t) + sizeof(long) % sizeof(uword_t);
    bigint_t out = bigint_zero(size);

    for (size_t i = 0; i < size; i++) {
        size_t shift = WORD_BITS * i;
        // Remember to split the shift!
        out.val[i] = (uword_t)(n >> shift / 2) >> shift / 2 + shift % 2;
    }

    return out;
}

// Return minimum number of words needed for n
size_t bigint_min_words(bigint_t n)
{
    size_t out;
    bigint_t temp = n;
    uword_t end = 0;
    // Whether n needs to be freed
    bool must_free = false;
    if (n.val[n.size - 1] & ((uword_t)1 << (WORD_BITS - 1))) {
        temp = bigint_lneg(n);
        must_free = true;
        end = ~(uword_t)0;
    }
    bool ret = false;
    for (size_t i = n.size - 1; i < n.size; i--) {
        if (temp.val[i] != end) {
            // Make sure most significant bit isn't set
            if (temp.val[i] & ((uword_t)1 << (WORD_BITS - 1))) {
                out = i + 2;
            } else {
                out = i + 1;
            }
            ret = true;
            break;
        }
    }
    if (must_free)
        bigint_delete(&temp);
    if (ret)
        return out;
    return 1;
}

// Resize bigint to specified number of words
bigint_t bigint_resize(bigint_t n, size_t size)
{
    bigint_t out;
    if (is_neg(n))
        out = bigint_minus1(size);
    else
        out = bigint_zero(size);

    size_t i;
    for (i = 0; i < smin(n.size, size); i++) {
        out.val[i] = n.val[i];
    }

    return out;
}

// Make copy of n
bigint_t bigint_copy(bigint_t n)
{
    bigint_t out = {
        .size = n.size,
        .val = malloc(n.size * sizeof(uword_t)),
    };

    for (size_t i = 0; i < out.size; i++)
        out.val[i] = n.val[i];

    return out;
}

// Add two words with overflow
static inline uword_t add_word(uword_t a, uword_t b, bool *overflow)
{
    asm (
        "add %2, %0\n\t"
        "setc %1\n\t"
        : "+r" (a), "=ro" (*overflow)
        : "ro" (b)
        : // No clobbers
    );
    return a;
}

// Increment n by k (in place)
static inline void ip_inc(bigint_t n, uword_t k)
{
    bool overflow = 0;

    n.val[0] = add_word(n.val[0], k, &overflow);

    for (size_t i = 1; i < n.size && overflow; i++) {
        n.val[i] = add_word(n.val[i], 1, &overflow);
    }
}

// Return the negative of the input
bigint_t bigint_neg(bigint_t n)
{
    bigint_t out = { .size = n.size, .val = malloc(n.size * sizeof(uword_t)) };

    // Take logical negation of n
    for (size_t i = 0; i < n.size; i++) {
        out.val[i] = ~n.val[i];
    }

    ip_inc(out, 1);

    // Shrink number
    out.size = bigint_min_words(out);

    return out;
}

// Sum of a and b
bigint_t bigint_sum(bigint_t a, bigint_t b)
{
    bigint_t sum;
    bigint_t term;

    // TODO make this cleaner?
    if (a.size < b.size) {
        term = bigint_resize(b, b.size);
        sum = bigint_resize(a, term.size + 1);
    } else {
        term = bigint_resize(a, a.size);
        sum = bigint_resize(b, term.size + 1);
    }

    bool overflow1;
    bool overflow2;
    uword_t next_overflow = 0;

    // Perform entrywise addition
    size_t i;
    for (i = 0; i < term.size; i++) {
        sum.val[i] = add_word(sum.val[i], next_overflow, &overflow1);
        next_overflow = 0;
        sum.val[i] = add_word(sum.val[i], term.val[i], &overflow2);
        if (overflow1 | overflow2)
            next_overflow++;
    }

    bool same_sign = (is_neg(a) == is_neg(b));

    if (next_overflow && same_sign) {
        // TODO is this right??
        sum.val[sum.size - 1] = is_neg(a) ? ~(uword_t)0 : 1;
    } else if (!same_sign) {
        sum.size--;
    }

    bigint_delete(&term);

    sum.size = bigint_min_words(sum);
    return sum;
}

// Return the difference a - b
bigint_t bigint_diff(bigint_t a, bigint_t b)
{
    bigint_t temp_neg;
    bigint_t out = bigint_sum(a, temp_neg = bigint_neg(b));
    bigint_delete(&temp_neg);

    return out;
}

// TODO is this function even necessary?
// n >> k (in place)
// NOTE It must be the case that k <= WORD_BITS
bigint_t ip_sr(bigint_t n, size_t k)
{
    // TODO handle sign extension for negative values

    // Shift the first word by k
    n.val[0] >>= k;

    // Shift the rest
    for (size_t i = 1; i < n.size; i++) {
        n.val[i-1] |= (n.val[i] & (( (uword_t)1 << k) - 1)) << (WORD_BITS - k);
        n.val[i] >> k;
    }
}

// n << k (in place)
// TODO support k > WORD_BITS
bigint_t ip_sl(bigint_t n, size_t k)
{
    // Shift the last word by k
    n.val[n.size-1] <<= k;

    // Shift the rest
    for (size_t i = n.size - 2; i < n.size; i--) {
        n.val[i] << k;
        n.val[i] |= (n.val[i-1] & ~( (uword_t)1 << (WORD_BITS - k) - 1)) >> (WORD_BITS - k);
    }
}

// Number of bits needed for number
static size_t bigint_bits(bigint_t n)
{
    // Calculate max shift
    for (ssize_t i = n.size * WORD_BITS - 1; i >= 0; i--) {
        if ( n.val[i / WORD_BITS] & ((uword_t)1 << i % WORD_BITS) )
            return i + 1;
    }
    return 0;
}

bigint_t bigint_sl(bigint_t n, size_t k)
{
    if (k == 0)
        return bigint_copy(n);

    bigint_t out;

    bool neg = is_neg(n);

    // Calculate new size in words
    size_t n_bits = bigint_bits(n);
    size_t new_size = (n_bits + k) / WORD_BITS + 1;
    new_size = smax(new_size, n.size + 1 + k / WORD_BITS);

    out = bigint_zero(new_size);

    /**
     * The procedure for shifting must go as follows. Say our number looks like
     * this (we order from most significant to least significant bit for
     * simplicity):
     *
     * [0---WORD----][----WORD----][----WORD----]
     *
     * After initially resizing the number to contain the shift, we may get
     * something like the following.
     *
     * [000000000000][0---WORD----][----WORD----][----WORD----]
     *
     * It is important to think of each word as structured as follows, since
     * these bit fields will be swapped between words to perform the shift.
     *
     * [ (WORD_BITS - (k % WORD_BITS)) bits  |  (k % WORD_BITS) bits ]
     */

    // Shift everything
    for (size_t i = 0; i < n.size; i++) {
        out.val[i + k / WORD_BITS] |= n.val[i] << (k % WORD_BITS);
        /**
         * Oh, Jesus. It turns out that x86 performs right shifts as follows:
         *      a >> k  =  a >> (k % word_size)
         * This means that the following code snippet would produce word
         * duplications whenever k % WORD_BITS == 0:
         *  out.val[i + 1 + k / WORD_BITS] = n.val[i] >> (WORD_BITS - (k % WORD_BITS));
         * You bet this was a pain to debug. We can instead break the shift into
         * halves as follows.
         */
        const size_t shift = WORD_BITS - (k % WORD_BITS);
        out.val[i + 1 + k / WORD_BITS] = (n.val[i] >> shift / 2) >> (shift / 2 + shift % 2);
    }

    if (neg) {
        uword_t val = ~(uword_t)0 >> (k % WORD_BITS) << (k % WORD_BITS);
        out.val[n.size + k / WORD_BITS] |= val;
    }

    out.size = bigint_min_words(out);
    return out;
}

// TODO
// n >> k
bigint_t bigint_sr(bigint_t n, size_t k)
{
    //bigint_t out;

    //return out;
}

// TODO use buffer instead
// Print each word in hex
void bigint_print_words(bigint_t n)
{
    for (size_t i = n.size - 1; i < n.size; i--) {
        printf("%x ", n.val[i]);
    }
    printf("\n");
}

// TODO Make this more efficient (use asm multiplication instructions)
// Integer multiplication a * b
// NOTE: best efficiency is achieved when bits(a) < bits(b)
bigint_t bigint_prod(bigint_t a, bigint_t b)
{
    bigint_t out = bigint_zero(1);

    // Resize numbers
    size_t bits = bigint_bits(a) + bigint_bits(b);
    size_t size = bits / WORD_BITS + (bits % WORD_BITS > 0);
    a = bigint_resize(a, size);
    b = bigint_resize(b, size);

    for (size_t i = 0; i < size * WORD_BITS; i++) {
        if (a.val[i / WORD_BITS] & ((uword_t)1 << (i % WORD_BITS))) {
            bigint_t shift = bigint_sl(b, i);
            bigint_t temp_sum = bigint_sum(out, shift);
            bigint_delete(&out);
            out = temp_sum;

            bigint_delete(&shift);
        }
    }

    bigint_delete(&a);
    bigint_delete(&b);

    out.size = size;
    out.size = bigint_min_words(out);
    return out;
}

// Integer division a/b
bigint_t bigint_div(bigint_t a, bigint_t b, bigint_t *rem)
{
    //bigint_t temp_diff;
    bool neg_a = is_neg(a);
    bool neg_b = is_neg(b);
    bool neg_out = neg_a != neg_b;

    if (neg_a)
        a = bigint_neg(a);
    if (neg_b)
        b = bigint_neg(b);

    *rem = bigint_copy(a);

    // TODO is this necessary?
    bigint_t temp_diff = bigint_diff(b, a);
    if (is_pos(temp_diff)) {
        bigint_delete(&temp_diff);
        return bigint_zero(1);
    }
    bigint_delete(&temp_diff);

    // Calculate the max left shift i so that (b << i) <= a
    size_t i;
    temp_diff = bigint_zero(1);
    for (i = 0; ! is_neg(temp_diff); i++) {
        bigint_delete(&temp_diff);
        bigint_t temp_shift;
        temp_diff = bigint_diff(a, temp_shift = bigint_sl(b, i));

        bigint_delete(&temp_shift);
    }
    bigint_delete(&temp_diff);
    bigint_t out = bigint_zero(i / WORD_BITS + (i % WORD_BITS > 0));

    i -= 2;

    // Perform long division algorithm
    for ( ; i < SIZE_MAX; i--) {
        bigint_t temp_shift = bigint_sl(b, i);
        temp_diff = bigint_diff(*rem, temp_shift);

        bigint_delete(&temp_shift);
        if (! is_neg(temp_diff)) {
            out.val[i / WORD_BITS] |= (uword_t)1 << (i % WORD_BITS);
            bigint_delete(rem);
            *rem = temp_diff;
        } else {
            bigint_delete(&temp_diff);
        }
    }

    // Take care of sign
    // TODO adjust `rem` based on sign?
    if (neg_a)
        bigint_delete(&a);
    if (neg_b)
        bigint_delete(&b);
    if (neg_out) {
        bigint_t temp = bigint_neg(out);
        bigint_delete(&out);
        out = temp;
    }

    return out;
}

array_t powers10;

// Return k-th power of 10
static bigint_t bigint_power10(size_t k)
{
    // Generate more powers of 10 if necessary
    if (powers10.size == 0) {
        bigint_t one = long_to_bigint(1);
        array_append(&powers10, &one);
    }
    if (k >= powers10.size) {
        for (size_t i = powers10.size; i <= k; i++) {
            bigint_t temp2 = bigint_sl( *(bigint_t*)array_get(powers10, i-1), 1);
            bigint_t temp8 = bigint_sl( *(bigint_t*)array_get(powers10, i-1), 3);
            bigint_t pow10 = bigint_sum(temp2, temp8);

            bigint_delete(&temp2);
            bigint_delete(&temp8);

            array_append(&powers10, &pow10);
        }
    }
    return *(bigint_t*)array_get(powers10, k);
}

// Return integer with value specified by decimal string
bigint_t bigint_new(char *string)
{
    bigint_t out = long_to_bigint(0);
    bool neg = false;

    size_t len = strlen(string);
    if (len == 0) {
        fprintf(stderr, "bigint_new: WARNING: input has length zero\n");
        return bigint_zero(1);
    }

    if (string[0] == '-') {
        neg = true;
        if (--len == 0) {
            fprintf(stderr, "bigint_new: WARNING: negative input has length zero\n");
            return bigint_zero(1);
        }
    }

    // Initialize all values from 0 to 9 for digit computations
    bigint_t digit[10];
    for (long i = 0; i < 10; i++) {
        digit[i] = long_to_bigint(i);
    }

    for (size_t i = 0; i < len; i++) {
        // Here, read each digit and additively construct the output
        const size_t idx = neg + i;
        bigint_t temp_prod = bigint_prod(digit[string[idx] - '0'], bigint_power10(len - i - 1));
        out = bigint_sum(out, temp_prod);
        bigint_delete(&temp_prod);
    }

    // Free digits
    for (int i = 0; i < 10; i++)
        bigint_delete(digit + i);

    return out;
}

const char const hex_digit[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

// Print n in base 10
char * bigint_print(bigint_t n)
{
    bool neg = is_neg(n);
    if (neg) {
        n = bigint_neg(n);
        n.size = bigint_min_words(n);
    }

    // Find first power of 10 greater than n
    ssize_t i;
    for (i = 1; true; i++) {
        bigint_t temp_diff = bigint_diff(bigint_power10(i-1), n);
        if (is_pos(temp_diff)) {
            bigint_delete(&temp_diff);
            i = ssmax(i - 2, 0);
            break;
        } else {
            bigint_delete(&temp_diff);
        }
    }

    ssize_t buffer_size = i + 1 + neg;
    char *out = malloc(buffer_size + 1);

    if (neg)
        out[0] = '-';

    // Compute each digit
    bigint_t running = bigint_copy(n);
    bigint_t rem;
    for ( ; i >= 0; i--) {
        bigint_t temp_digit = bigint_div(running, *(bigint_t*)array_get(powers10, i), &rem);
        bigint_delete(&running);
        running = rem;
        long digit = bigint_to_long(temp_digit);
        bigint_delete(&temp_digit);
        // TODO
        if (digit < 0 || digit > 10) {
            printf("ERROR: digit offset: %d\n", digit);
        }
        out[buffer_size - i - 1] = hex_digit[digit];
    }
    out[buffer_size] = '\0';

    bigint_delete(&running);

    // Free n if necessary
    if (neg)
        bigint_delete(&n);

    return out;
}

// Print n in hexadecimal
char * bigint_print_hex(bigint_t n)
{
    const size_t buffer_size = (WORD_BITS >> 2) * n.size + 1;
    char *buffer = malloc(buffer_size);

    for (size_t i = n.size - 1; i < n.size; i--) {
        for (size_t j = WORD_BITS - 4; j < WORD_BITS; j -= 4) {
            const size_t idx = (WORD_BITS >> 2) * (n.size - 1 - i) + ((WORD_BITS - 4 - j) >> 2);
            const size_t hex_idx = (n.val[i] >> j) & 0xf;
            buffer[idx] = hex_digit[hex_idx];
        }
    }
    buffer[buffer_size - 1] = 0;

    return buffer;
}

// Greatest Common Divisor
bigint_t bigint_gcd(bigint_t a, bigint_t b)
{
    bigint_t M = bigint_copy(a);
    bigint_t m = bigint_copy(b);
    bigint_t rem;

    do {
        bigint_t temp = bigint_div(M, m, &rem);

        bigint_delete(&temp);
        bigint_delete(&M);

        M = m;
        m = rem;
    } while (!is_zero(rem));

    bigint_delete(&m);
    return M;
}

// Perform the extended euclidean algorithm
bigint_t bigint_xgcd(bigint_t a, bigint_t b, bigint_t *x, bigint_t *y)
{
    bigint_t r_2;
    bigint_t r_1 = bigint_max(a, b);
    bigint_t r = bigint_min(a, b);

    bigint_t x_2;
    bigint_t x_1 = long_to_bigint(0);
    *x = long_to_bigint(1);

    bigint_t y_2;
    bigint_t y_1 = long_to_bigint(1);

    bigint_t temp_rem;
    bigint_t temp_div;
    *y = bigint_neg(temp_div = bigint_div(r, r_1, &temp_rem));
    bigint_delete(&temp_div);
    bigint_delete(&temp_rem);

    while (!is_zero(r)) {
        r_2 = r_1;
        r_1 = r;

        x_2 = x_1;
        x_1 = *x;

        y_2 = y_1;
        y_1 = *y;

        bigint_t prod;
        bigint_t div = bigint_div(r_2, r_1, &r);
        *x = bigint_diff(x_2, prod = bigint_prod(x_1, div));

        bigint_delete(&prod);

        *y = bigint_diff(y_2, prod = bigint_prod(y_1, div));

        bigint_delete(&prod);
        bigint_delete(&div);
        bigint_delete(&r_2);
        bigint_delete(&x_2);
        bigint_delete(&y_2);
    }
    bigint_delete(&r);
    bigint_delete(x);
    bigint_delete(y);

    bigint_t temp_diff;
    if (!is_neg(temp_diff = bigint_diff(b, a))) {
        *x = x_1;
        *y = y_1;
    } else {
        *x = y_1;
        *y = x_1;
    }

    bigint_delete(&temp_diff);

    return r_1;
}

// Initialize bigint runtime data structures
void bigint_init(void)
{
    powers10 = array_new(sizeof(bigint_t));
}

// Free bigint runtime data structures
void bigint_exit(void)
{
    // Free powers10
    for (size_t i = 0; i < powers10.size; i++) {
        bigint_delete((bigint_t*)array_get(powers10, i));
    }
    array_delete(&powers10);
}
