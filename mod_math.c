/**
 * mod_math.c: Modular arithmetic
 */

#include <stdio.h>

#include "math.h"

// TODO move to a header file
bigint_t mod(bigint_t a, bigint_t n);
bigint_t mod_sum(bigint_t a, bigint_t b, bigint_t n);
bigint_t mod_diff(bigint_t a, bigint_t b, bigint_t n);
bigint_t mod_prod(bigint_t a, bigint_t b, bigint_t n);
bigint_t mod_exp(bigint_t a, bigint_t exp, bigint_t n);
bigint_t mod_inv(bigint_t a, bigint_t n);
bigint_t mod_neg(bigint_t a, bigint_t n);

bigint_t mod(bigint_t a, bigint_t n)
{
    bigint_t out;
    bigint_t div = div = bigint_div(a, n, &out);
    bigint_delete(&div);
    return out;
}

// TODO test
bigint_t mod_sum(bigint_t a, bigint_t b, bigint_t n)
{
    bigint_t sum;
    a = mod(a, n);
    b = mod(b, n);
    bigint_t tmp_sum = bigint_sum(a, b);
    bigint_delete(&a);
    bigint_delete(&b);
    sum = mod(tmp_sum, n);
    bigint_delete(&tmp_sum);
    return sum;
}

// TODO test
bigint_t mod_diff(bigint_t a, bigint_t b, bigint_t n)
{
    a = mod(a, n);
    b = mod(b, n);
    bigint_t neg_b = bigint_neg(b);
    bigint_delete(&b);
    bigint_t diff_ab = bigint_sum(a, neg_b);
    bigint_delete(&a);
    bigint_delete(&neg_b);
    bigint_t diff = mod(diff_ab, n);
    bigint_delete(&diff_ab);
    return diff;
}

// TODO re-implement bigint_prod and take mods after each iteration
// TODO test
bigint_t mod_prod(bigint_t a, bigint_t b, bigint_t n)
{
    a = mod(a, n);
    b = mod(b, n);
    bigint_t prod_ab = bigint_prod(a, b);
    bigint_delete(&a);
    bigint_delete(&b);
    bigint_t prod = mod(prod_ab, n);
    bigint_delete(&prod_ab);
    return prod;
}

// TODO
// TODO test
// Calculate a^exp mod n
bigint_t mod_exp(bigint_t a, bigint_t exp, bigint_t n)
{

}

// TODO test
// Calculate multiplicative inverse of a mod n, return 0 if it doesn't exist
bigint_t mod_inv(bigint_t a, bigint_t n)
{
    a = mod(a, n);
    bigint_t inv_a, y;
    bigint_t gcd = bigint_xgcd(a, n, &inv_a, &y);
    bigint_delete(&a);
    bigint_delete(&y);
    bigint_t one = bigint_new("1");
    bigint_t inv;
    if (bigint_equals(gcd, one)) {
        bigint_delete(&one);
        inv = mod(inv_a, n);
        bigint_delete(&inv_a);
    } else {
        bigint_delete(&one);
        bigint_delete(&inv_a);
        inv = bigint_new("0");
    }
    return inv;
}

// TODO test
// Calculate the additive inverse of a mod n
bigint_t mod_neg(bigint_t a, bigint_t n)
{
    a = mod(a, n);
    bigint_t neg_a = bigint_neg(a);
    bigint_delete(&a);
    bigint_t neg = mod(neg_a, n);
    bigint_delete(&neg_a);
    return neg;
}

// Testing
int mod_test(void)
{
    char *p1, *p2, *p3;
    bigint_t tmp1, tmp2;
    bool test;
    int total_errors = 0;

    // Test 28374717842836 % 3 == 1

    bigint_t a = bigint_new("28374717842836");
    bigint_t n = bigint_new("3");

    tmp1 = mod(a, n);
    test = bigint_equals(tmp1, tmp2 = bigint_new("1"));
    printf("%s: %s %% %s == %s\n",
        test ? "TRUE" : "FALSE",
        p1 = bigint_print(a),
        p2 = bigint_print(n),
        p3 = bigint_print(tmp1)
    );
    free(p1); free(p2); free(p3);
    bigint_delete(&tmp1); bigint_delete(&tmp2);

    bigint_delete(&a);
    bigint_delete(&n);

    total_errors += !test;

    return total_errors;
}
