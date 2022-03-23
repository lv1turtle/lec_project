#include <stdio.h>
#include <stdlib.h>
#include "mRSA.h"

int main(void)
{
    uint64_t e, d, n, m, c;
    int i, count;

    /*
     * test 1 : m = 0 ~ 19
     */
    mRSA_generate_key(&e, &d, &n);
    if (n < MINIMUM_N) {
        printf("Error: RSA key is not 64 bits: n = %016llx\n", n);
        exit(1);
    }
    printf("e = %016llx\nd = %016llx\nn = %016llx\n", e, d, n);
    for (i = 0; i < 20; ++i) {
        m = i;
        printf("m = %llu, ", m);
        mRSA_cipher(&m, e, n);
        printf("c = %llu, ", m);
        mRSA_cipher(&m, d, n);
        printf("v = %llu\n", m);
    }
    /*
     * test 2: Generate random m
     */
    mRSA_generate_key(&e, &d, &n);
    printf("e = %016llx\nd = %016llx\nn = %016llx\n", e, d, n);
    for (i = 0; i < 20; ++i) {
        arc4random_buf(&m, sizeof(uint64_t));
        printf("m = %016llx, ", m);
        if (mRSA_cipher(&m, d, n))
            printf("m may be too big\n");
        else {
            printf("c = %016llx, ", m);
            mRSA_cipher(&m, e, n);
            printf("v = %016llx\n", m);
        }
    }

    printf("Random testing"); fflush(stdout);
    count = 0;
    do {
        printf("");
        mRSA_generate_key(&e, &d, &n);
        arc4random_buf(&m, sizeof(uint64_t)); m &= 0x7fffffffffffffff;
        c = m;
        if (mRSA_cipher(&c, e, n)) {
            printf("Error: RSA key is not 64 bits: %016llx\n", n);
            exit(1);
        };
        if (mRSA_cipher(&c, d, n)) {
            printf("Error: check your modular calculations.\n");
            exit(1);
        };
        if (m != c) {
            printf("Error: your RSA key generation may be wrong.\n");
            exit(1);
        }
        if (++count % 0xff == 0) {
            printf(".");
            fflush(stdout);
        }
    } while (count < 0xfff);
    printf("No error found!\n");
    fflush(stdout);
}
