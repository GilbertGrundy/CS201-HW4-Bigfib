#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bigint.h"

/* Returns a new bigint structure with nbuckets
   buckets. Buckets are uninitialized. Code will panic on
   out-of-memory errors. */
struct bigint *bigint_new(int nbuckets) {
    struct bigint *b = malloc(sizeof *b);
    assert(b);
    b->buckets = malloc(nbuckets * sizeof(bucket_t));
    assert(b->buckets);
    b->nbuckets = nbuckets;
    return b;
}

/* Frees all memory associated with a bigint. */
void bigint_free(struct bigint *b) {
    free(b->buckets);
    free(b);
}

/* Convert a character representing a hex digit to its value. */
static bucket_t hex_digit_value(char d) {
    if (d >= '0' && d <= '9')
        return d - '0';
    d = tolower(d);
    assert(d >= 'a' && d <= 'f');
    return d - 'a' + 10;
}

/* Read a bigint from the given hex string (with no leading "0x").
   Returns null on incorrectly formatted input.*/
struct bigint *bigint_read(char *s) {
    /* Number of digits in input. */
    int ns = strlen(s);
    /* Empty string is invalid. */
    if (ns == 0)
        return 0;
    /* Number of digits per bucket. */
    int ds_per_b = 2 * sizeof(bucket_t);
    /* Number of buckets in result, computed from number of
       digits in input. 0..16 digits = 1 bucket; 17-32
       digits = 2 buckets ... */
    int nb = (ns + ds_per_b - 1) / ds_per_b;
    /* The result. */
    struct bigint *b = bigint_new(nb);

    /* Strategy: process digits of input from left-to-right,
       filling the buckets from largest index to
       smallest. */

    /* Index of current bucket in units of digits. */
    int di = 0;
    /* Build up all the buckets. */
    for (int i = nb - 1; i >= 0; --i) {
        /* Number of digits to process this iteration. The
           first iteration is "special", since it may not have
           enough digits. */
        int d_process = ds_per_b;
        if (i == nb - 1) {
            d_process = ns - ds_per_b * (nb - 1);
        }

        /* Fill a bucket. */
        bucket_t d = 0L;
        for (int j = di; j < di + d_process; j++) {
            /* Do not walk off the end of the input. */
            assert(j < ns);
            /* Convert current digit. */
            char c = s[j];
            if (!isxdigit(c)) {
                bigint_free(b);
                return 0;
            }
            bucket_t k = hex_digit_value(c);
            /* Store current digit at bottom of bucket,
               shifting the rest of the bucket contents
               left one digit. */
            d = (d << 4) | k;
        }
        /* Save the current bucket and set up for next
           iteration. */
        b->buckets[i] = d;
        di += d_process;
    }
    return b;
}

/* Print b as hexadecimal on stdout, with a trailing newline. */
void bigint_print(struct bigint *b) {
    /* Print the first bucket, without leading 0s.
       The funny cast is to allow "%lx" as the format
       for any bucket_t. */
    int n = b->nbuckets - 1;
    printf("%lx", (uint64_t) b->buckets[n]);
    /* Print the rest of the buckets. A number of leading
       zeros may be needed with each bucket, dependent on
       bucket_t. */
    for (int i = n - 1; i >= 0; --i)
        printf("%0*lx", 2 * (int) sizeof(bucket_t), (uint64_t) b->buckets[i]);
    printf("\n");
}

/* Add another uninitialized bucket to the bigint. Code will
   panic on out-of-memory errors. */
static void bigint_grow(struct bigint *b) {
    b->nbuckets++;
    b->buckets = realloc(b->buckets, b->nbuckets * sizeof(bucket_t));
    assert(b->buckets);
}

/* Add two buckets with carry. *carry should be either 0 or
   1 on input, representing the carry bit from a previous
   addition.  On output, *carry will be either 0 or 1
   depending on whether this addition had a carry. The
   result of the addition (up to carry) is returned. */


#ifdef ADDC_ASM_JMP

/* Assembly implementation of the C code, works only for
   bucket_t == uint64_t. */
extern bucket_t addc_asm_jmp(bucket_t *carry, bucket_t b1, bucket_t b2) {
    assert(sizeof(bucket_t) == 8);
    bucket_t sum;
    uint64_t cin = *carry;
    uint64_t cout;
    asm(
        "xorq	%1, %1\n\t"
        "movq	%2, %0\n\t"
        "addq	%4, %0\n\t"
        "jnb	.L%=0\n\t"
        "movq	$1, %1\n"
        ".L%=0:\n\t"
        "addq	%3, %0\n\t"
        "jnb	.L%=1\n\t"
        "movq	$1, %1\n"
        ".L%=1:\n"
        : "=&rm" (sum), "=&rm"(cout)
        : "rm" (b1), "rm" (b2), "r" (cin)
        : "cc"
    );

    /* Save output carry and return result. */
    *carry = cout;
    return sum;
}

#endif

#ifdef ADDC_C

/* Pure C implementation, works for any bucket size. */
extern bucket_t addc_c(bucket_t *carry, bucket_t b1, bucket_t b2) {
    /* Output carry flag value. */
    bucket_t cflag = 0;

    /* Output sum value. A carry out of sum will cause it to "wrap around",
       becoming smaller than b1. */
    bucket_t sum = b1 + *carry;
    if (sum < b1)
        cflag = 1;

    /* Now add in b2. Again, a carry will cause wrap. */
    sum += b2;
    if (sum < b2)
        cflag = 1;

#ifdef DEBUG_ADDC
    /* This tests that the sum and carry are being computed
       correctly by computing the 64-bit sum and
       carry. Requires that bucket_t be less than 64 bits to
       be correct. */
    assert(sizeof(bucket_t) < sizeof(uint64_t));
    uint64_t big_sum = b1 + b2 + *carry;
    assert((bucket_t) big_sum == sum);
    assert((big_sum >> (8 * sizeof(bucket_t))) == cflag);
#endif

    /* Save output carry and return result. */
    *carry = cflag;
    return sum;
}

#endif

/* Add b1 and b2, returning a newly-allocated bignum result.
   Neither b1 nor b2 are freed by this operation. */
struct bigint *bigint_add(struct bigint *b1, struct bigint *b2) {
    /* Arrange that b1 has at least as many buckets as b2
       by swapping the pointers if necessary. This is convenient
       later. */
    if (b1->nbuckets < b2->nbuckets) {
        struct bigint *tmp = b1;
        b1 = b2;
        b2 = tmp;
    }

    /* The result of the add will normally be the same
       number of buckets as b1. (It could be one bucket
       bigger if carry-out occurs. */
    struct bigint *b = bigint_new(b1->nbuckets);

    /* Carry-in from the previous bucket. There is no
       carry-in on the least significant bucket. */
    bucket_t carry = 0;


    /* Strategy: Add buckets that are in both b1 and b2
       using addc. When out of buckets in b2, propagate the
       carry and copy the buckets of b1 using addc (because
       it's easy). If there's a carry-out of the most
       significant bucket, grow the result to contain it. */
    int i;
    /* Add common buckets. */
    for (i = 0; i < b2->nbuckets; i++)
        b->buckets[i] =
            ADDC(&carry, b1->buckets[i], b2->buckets[i]);
    /* Propagate carry and copy the rest of b1. */
    for ( ; i < b1->nbuckets; i++)
        b->buckets[i] =
            ADDC(&carry, b1->buckets[i], 0);
    /* Handle carry-out. */
    if (carry) {
        bigint_grow(b);
        assert(i == b->nbuckets - 1);
        b->buckets[i] = carry;
    }
    return b;
}
