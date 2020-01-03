/**
 * math.c: Mathematical operations
 */

#include "math.h"

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

// Return whether a == b, i.e. whether a - b == 0
bool bigint_equals(bigint_t a, bigint_t b)
{
    bigint_t diff = bigint_diff(a, b);
    bool out = is_zero(diff);
    bigint_delete(&diff);
    return out;
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
