#include <assert.h>
#include "bigint.h"

int main(int argc, char **argv) {
    assert(argc == 3);
    struct bigint *b1 = bigint_read(argv[1]);
    assert(b1);
    struct bigint *b2 = bigint_read(argv[2]);
    assert(b2);
    struct bigint *b = bigint_add(b1, b2);
    bigint_free(b1);
    bigint_free(b2);
    bigint_print(b);
    bigint_free(b);
    return 0;
}
