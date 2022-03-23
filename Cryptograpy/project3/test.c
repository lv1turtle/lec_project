#include <stdio.h>
#include "miller_rabin.h"

/*
 * test program
 */
int main(void)
{
    uint64_t a, b, m, x;
    int i;
    
    a = 1234; b = 5678; m = 3456;
    printf("<덧셈> ");
    printf("%llu + %llu mod %llu = %llu\n", a, b, m, mod_add(a,b,m));
    printf("<뺄셈> ");
    printf("%llu - %llu mod %llu = %llu\n", a, b, m, mod_sub(a,b,m));
    printf("<곱셈> ");
    printf("%llu * %llu mod %llu = %llu\n", a, b, m, mod_mul(a,b,m));
    printf("<지수> ");
    printf("%llu ^ %llu mod %llu = %llu\n", a, b, m, mod_pow(a,b,m));
    printf("---\n");
    a = 3684901700; b = 3904801120; m = 4294901760;
    printf("<덧셈> ");
    printf("%llu + %llu mod %llu = %llu\n", a, b, m, mod_add(a,b,m));
    printf("<뺄셈> ");
    printf("%llu - %llu mod %llu = %llu\n", a, b, m, mod_sub(a,b,m));
    printf("<곱셈> ");
    printf("%llu * %llu mod %llu = %llu\n", a, b, m, mod_mul(a,b,m));
    printf("<지수> ");
    printf("%llu ^ %llu mod %llu = %llu\n", a, b, m, mod_pow(a,b,m));
    printf("---\n");
    a = 18446744073709551360u;
    b = 18446744073709551598u;
    m = 18441921395520346504u;
    printf("<덧셈> ");
    printf("%llu + %llu mod %llu = %llu\n", a, b, m, mod_add(a,b,m));
    printf("<뺄셈> ");
    printf("%llu - %llu mod %llu = %llu\n", a, b, m, mod_sub(a,b,m));
    printf("<곱셈> ");
    printf("%llu * %llu mod %llu = %llu\n", a, b, m, mod_mul(a,b,m));
    printf("<지수> ");
    printf("%llu ^ %llu mod %llu = %llu\n", a, b, m, mod_pow(a,b,m));

    /*
     * Print 10000 primes from beginning
     */
    x = 2; i = 0;
    while (1) {
        if (miller_rabin(x)) {
            ++i;
            printf("%llu ", x);
            if (i % 10 == 0)
                printf("\n");
            if (i == 10000)
                break;
        }
        ++x;
    }
    /*
     * x = 0x8000000000000000
     */
    x = 0x8000000000000000; i = 0;
    while (1) {
        if (miller_rabin(x)) {
            ++i;
            printf("%llu ", x);
            if (i % 4 == 0)
                printf("\n");
            if (i == 100)
                break;
        }
        ++x;
    }
}
