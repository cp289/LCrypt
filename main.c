/**
 * main.c: Main program file
 */

#include <stdio.h>
#include <stddef.h>

#include "math.h"
#include "bigint.h"

int main(int argc, char *argv[])
{
    bigint_init();

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

    bigint_exit();
    printf("EXIT\n");

    return 0;
}
