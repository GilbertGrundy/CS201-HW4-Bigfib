#include <stdint.h>

/* Type of a bucket. Each bucket contains a piece of the
   integer. This type can be any unsigned integer type. */
typedef uint64_t bucket_t;

/* Array of buckets in least-significant to most-significant
   order. */
struct bigint {
    int nbuckets;
    bucket_t *buckets;
};

/* Standard interface. See the implementation for usage
   instructions. */

extern struct bigint *bigint_new(int nbuckets);
extern void bigint_free(struct bigint *b);
extern struct bigint *bigint_read(char *s);
extern void bigint_print(struct bigint *b);
struct bigint *bigint_add(struct bigint *b1, struct bigint *b2);

/* This part is an implementation detail that is exposed for
   unit-testing purposes. */

#ifdef ADDC_C
#define ADDC addc_c
#endif
#ifdef ADDC_ASM_JMP
#define ADDC addc_asm_jmp
#endif

extern bucket_t ADDC(bucket_t *carry, bucket_t b1, bucket_t b2);
