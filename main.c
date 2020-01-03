/**
 * main.c: Main program file
 */

#include <stdint.h>
#include <stdio.h>

#include "math.h"

// TODO place all test code into file-specific testing methods
static int main_test(void)
{
    bigint_t a = bigint_zero(5);
    bigint_t b = bigint_zero(5);

    char *p1, *p2, *p3; 

    a.val[2] = 590;
    a.size = bigint_min_words(a);

    b.val[3] = 555;
    b.size = bigint_min_words(b);

    printf("a: %s\n", p1 = bigint_print_hex(a));
    free(p1);

    printf("a = %s\n", p1 = bigint_print(a));
    free(p1);

    printf("b = %s\n", p1 = bigint_print(b));
    free(p1);

    bigint_t temp_gcd;
    printf("math.gcd(%s, %s) == %s\n",
            p1 = bigint_print(a),
            p2 = bigint_print(b),
            p3 = bigint_print(temp_gcd = bigint_gcd(a,b)));
    free(p1); free(p2); free(p3);
    bigint_delete(&temp_gcd);
    printf("math.gcd(0x%s, 0x%s) == 0x%s\n",
            p1 = bigint_print_hex(a),
            p2 = bigint_print_hex(b),
            p3 = bigint_print_hex(temp_gcd = bigint_gcd(a,b)));
    free(p1); free(p2); free(p3);
    bigint_delete(&temp_gcd);

    bigint_t one = bigint_zero(1);
    one.val[0] = 1;
    bigint_t temp;
    size_t shift = 300;
    printf("1 << %lu == %s\n",
        shift,
        p3 = bigint_print(temp = bigint_sl(one, shift))
    );
    free(p3);
    bigint_delete(&one);
    bigint_delete(&temp);

    bigint_t temp_prod;
    printf("%s * %s == %s\n",
        p1 = bigint_print(a),
        p2 = bigint_print(b),
        p3 = bigint_print(temp_prod = bigint_prod(a, b))
    );
    free(p1); free(p2); free(p3);
    bigint_delete(&temp_prod);

    bigint_delete(&a);
    a = bigint_minus1(1);
    printf("%s * %s == %s\n",
        p1 = bigint_print(a),
        p2 = bigint_print(b),
        p3 = bigint_print(temp_prod = bigint_prod(a, b))
    );
    free(p1); free(p2); free(p3);

    bigint_delete(&temp_prod);
    bigint_delete(&a);
    bigint_delete(&b);

    {
        char *a_s = "182735418273654813947182735872";
        char *b_s = "98373624897834762873723426734";

        bigint_t a = bigint_new(a_s);
        bigint_t b = bigint_new(b_s);

        char *p1, *p2, *p3, *p4, *p5;
        printf("new: %s == %s\n", a_s, p1 = bigint_print(a));
        printf("new: %s == %s\n", b_s, p2 = bigint_print(b));
        free(p1); free(p2);

        bigint_t x, y;
        bigint_t gcd = bigint_xgcd(a, b, &x, &y);

        printf("xgcd: %s * %s + %s * %s == %s\n",
            p1 = bigint_print(x),
            p2 = bigint_print(a),
            p3 = bigint_print(y),
            p4 = bigint_print(b),
            p5 = bigint_print(gcd)
        );
        free(p1); free(p2); free(p3); free(p4); free(p5);
        bigint_delete(&a);
        bigint_delete(&b);
        bigint_delete(&x);
        bigint_delete(&y);
        bigint_delete(&gcd);
    }

    // Test: 42 as the sum of cubes
    bigint_t x = bigint_new("-80538738812075974");
    bigint_t y = bigint_new("80435758145817515");
    bigint_t z = bigint_new("12602123297335631");

    printf("x: %s (neg: %d), y: %s, z: %s\n",
        p1 = bigint_print(x),
        is_neg(x),
        p2 = bigint_print(y),
        p3 = bigint_print(z));
    free(p1); free(p2); free(p3);

    bigint_t tmp1, tmp2, tmp3;
    bigint_t prod1 = bigint_prod(x, tmp1 = bigint_prod(x,x));
    bigint_t prod2 = bigint_prod(y, tmp2 = bigint_prod(y,y));
    bigint_t prod3 = bigint_prod(z, tmp3 = bigint_prod(z,z));
    bigint_delete(&tmp1); bigint_delete(&tmp2); bigint_delete(&tmp3);

    printf("Test 42: %s\n", p1 = bigint_print(tmp1 = bigint_sum(prod1, tmp2 = bigint_sum(prod2, prod3))));
    free(p1);
    bigint_delete(&tmp1);
    bigint_delete(&tmp2);

    bigint_delete(&prod1);
    bigint_delete(&prod2);
    bigint_delete(&prod3);
    bigint_delete(&x);
    bigint_delete(&y);
    bigint_delete(&z);

    mod_test();
}

static void main_init(void)
{
    bigint_init();
}

static void main_exit(void)
{
    bigint_exit();
    printf("EXIT\n");
}

int main(int argc, char *argv[])
{
    main_init();
    main_test();
    main_exit();

    return 0;
}
