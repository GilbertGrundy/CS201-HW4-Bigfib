#include <assert.h>
#include <stdlib.h>

#include "bigint.h"

int main(int argc, char **argv) {
    assert(argc == 2);
    int n = atoi(argv[1]);
    assert(n > 0);
    struct bigint *b1 = bigint_read("1");
    assert(b1);
    struct bigint *b2 = bigint_read("1");
    assert(b2);
    for (int i = 2; i < n; i++) {
        struct bigint *b = bigint_add(b1, b2);
        bigint_free(b1);
        b1 = b2;
        b2 = b;
    }
    bigint_print(b2);
    bigint_free(b1);
    bigint_free(b2);
    return 0;
}
