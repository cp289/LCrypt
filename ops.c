/**
 * ops.c: Useful mathematical operations
 */

#include "math.h"

// Perform the extended euclidean algorithm
void extended_euclid(int64_t a, int64_t b, int64_t *gcd, int64_t *x, int64_t *y)
{
    int64_t r_2;
    int64_t r_1 = lmax(a, b);
    int64_t r = lmin(a, b) % r_1;

    int64_t x_2;
    int64_t x_1 = 0;
    *x = 1;

    int64_t y_2;
    int64_t y_1 = 1;
    *y = -(r/r_1);

    while (r) {
        r_2 = r_1;
        r_1 = r;

        x_2 = x_1;
        x_1 = *x;

        y_2 = y_1;
        y_1 = *y;

        *x = x_2 - x_1*(r_2 / r_1);
        *y = y_2 - y_1*(r_2 / r_1);

        r = r_2 % r_1;
    }

    if (a <= b) {
        *x = x_1;
        *y = y_1;
    } else {
        *x = y_1;
        *y = x_1;
    }

    *gcd = r_1;
}
