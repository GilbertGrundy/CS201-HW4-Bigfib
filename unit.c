#include <assert.h>
#include <stdio.h>

#include "bigint.h"

int unit_test_addc(void) {
    assert(sizeof(bucket_t) == 8);
    int status = 0;
    bucket_t ones = ~0UL;
    bucket_t half = 1UL << 63;

    struct test {
        int n;
        bucket_t cin, b1, b2;
        bucket_t sum, cout;
    } tests[] = {
        {1, 0,    0,    0,    0, 0},
        {2, 1,    0,    0,    1, 0},
        {3, 0, ones,    0, ones, 0},
        {4, 0, ones,    1,    0, 1},
        {5, 1, ones,    0,    0, 1},
        {6, 1, ones,    1,    1, 1},
        {7, 0, half, half,    0, 1},
        {8, 1, half, half,    1, 1},
        {0, 0,    0,    0,    0, 0},
    };

    for (struct test *t = tests; t->n; t++) {
        bucket_t carry = t->cin; 
        bucket_t sum = ADDC(&carry, t->b1, t->b2);
        if (sum != t->sum || carry != t->cout) {
            printf("test %d failed: sum %lx (%lx), carry %ld (%ld)\n",
                   t->n, sum, t->sum, carry, t->cout);
            status = -1;
        }
    }

    return status;
}

int main() {
    int status = unit_test_addc();
    if (status == 0)
        printf("all tests passed\n");
    return status;
}
