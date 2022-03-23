#include <stdio.h>
#include <stdint.h>
// mod_add() - computes a+b mod m
uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{
    a = a%m;
    b = b%m;
    if(a >= m-b)
        return a-(m-b);
    else
        return a + b;
}

// mod_sub() - computes a-b mod m
uint64_t mod_sub(uint64_t a, uint64_t b, uint64_t m)
{
    a = a%m;
    b = b%m;
    if(a<b)
        return a+(m-b);
    else
        return a-b;
}

/*
 * mod_mul() - computes a*b mod m
 *     r = 0;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_add(r, a, m);
 *         b = b >> 1;
 *         a = mod_add(a, a, m);
 *     }
 */
uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
{
    uint64_t r = 0;
    while (b > 0) {
        if (b & 1)
            r = mod_add(r, a, m);
        b = b >> 1;
        a = mod_add(a, a, m);
    }
    return r;
}

/*
 * mod_pow() - computes a^b mod m
 *     r = 1;
 *     while (b > 0) {
 *         if (b & 1)
 *             r = mod_mul(r, a, m);
 *         b = b >> 1;
 *         a = mod_mul(a, a, m);
 *     }
 */
uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
{
    uint64_t r = 1;
    while (b > 0) {
        if (b & 1)
            r = mod_mul(r, a, m);
        b = b >> 1;
        a = mod_mul(a, a, m);
    }
    return r;
}
