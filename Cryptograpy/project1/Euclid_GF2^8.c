#include <stdio.h>
#include <stdlib.h>

// gcd() - Euclidean algorithm
int gcd(int a, int b)
{
    int k;
    while(b!=0){
        k = a % b;
        a = b;
        b = k;
    }
    return a;
}

// xgcd() - Extended Euclidean algorithm
int xgcd(int a, int b, int *x, int *y)
{
    int x0 = 1, y0 = 0, x1= 0, y1=1;
    int d0 = a, d1 = b, q, swap;
    while(d1 != 0){
        q = d0/d1;
        swap = d0 - q*d1;
        d0 = d1;
        d1 = swap;
        
        swap = x0 - q*x1;
        x0 = x1;
        x1 = swap;

        swap = y0 - q*y1;
        y0 = y1;
        y1 = swap;
    }  
    *x = x0, *y = y0;
    return d0;
}

// mul_inv() - computes multiplicative inverse a^-1 mod m
int mul_inv(int a, int m)
{
    int x0 = 1, x1= 0;
    int d0 = a, d1 = m, q, swap;
    while(d1 > 1){
        q = d0/d1;
        swap = d0 - q*d1;
        d0 = d1;
        d1 = swap;
        
        swap = x0 - q*x1;
        x0 = x1;
        x1 = swap;
    }  
    if(d1==1)
        return (x1>0)?x1:(x1+m);
    else
        return 0;
}

// umul_inv() - computes multiplicative inverse a^-1 mod m
uint64_t umul_inv(uint64_t a, uint64_t m)
{
    unsigned long long int x0 = 1, x1= 0, t;
    unsigned long long int d0 = a, d1 = m, q, swap;
    while(d1 > 1){
        q = d0 / d1;
        swap = d0 - q*d1;
        d0 = d1;
        d1 = swap;
        
        t = q*x1;
        if(x0 < t){
            while(m < t){
                t = t - m;
            }
            swap = m - t + x0; 
        }
        else
            swap = x0 - t;
        x0 = x1;
        x1 = swap;
    }
    uint64_t r = x1;
    if(d1==1)
        return r;
    else
        return 0;
}

// gf8_mul(a, b) - a * b mod x^8+x^4+x^3+x+1.
uint8_t gf8_mul(uint8_t a, uint8_t b)
{
    uint8_t r = 0;

    while(b>0){
        if(b&1)
            r = r^a;
        b = b>>1;
        a = (a<<1) ^ ((a>>7)&1 ? 0x1B : 0);// xtime(a) 
    }
    return r;
}

// gf8_pow(a,b) - a^b mod x^8+x^4+x^3+x+1
uint8_t gf8_pow(uint8_t a, uint8_t b)
{
    uint8_t r = 1;
    while(b>0){
        if(b&1)
            r = gf8_mul(r, a);
        b = b >> 1;
        a = gf8_mul(a, a);
    }
    return r;
}

// gf8_inv(a) - a^-1 mod x^8+x^4+x^3+x+1
uint8_t gf8_inv(uint8_t a)
{
    return gf8_pow(a, 0xfe);
}

int main(void)
{
    int a, b, x, y, d, count;
    uint64_t m, ai;
    
    // gcd test
    printf("--- gcd test ---\n");
    a = 28; b = 0;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 0; b = 32;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 41370; b = 22386;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    a = 22386; b = 41371;
    printf("gcd(%d,%d) = %d\n", a, b, gcd(a,b));
    
    // xgcd, mul_inv test
    printf("--- xgcd, mul_inv test ---\n");
    a = 41370; b = 22386;
    d = xgcd(a, b, &x, &y);
    printf("%d = %d * %d + %d * %d\n", d, a, x, b, y);
    printf("%d^-1 mod %d = %d, %d^-1 mod %d = %d\n", a, b, mul_inv(a,b), b, a, mul_inv(b,a));
    a = 41371; b = 22386;
    d = xgcd(a, b, &x, &y);
    printf("%d = %d * %d + %d * %d\n", d, a, x, b, y);
    printf("%d^-1 mod %d = %d, %d^-1 mod %d = %d\n", a, b, mul_inv(a,b), b, a, mul_inv(b,a));
    
    /*
     * generate random number a, b & compute xgcd
     * if xgcd == 1, check a^-1 mod b & b^-1 mod a
     * repeat a lot
     */
    printf("--- random mul_inv test ---\n"); fflush(stdout);
    count = 0;
    do {
        arc4random_buf(&a, sizeof(int)); a &= 0x7fffffff;
        arc4random_buf(&b, sizeof(int)); b &= 0x7fffffff;
        d = xgcd(a, b, &x, &y);
        if (d == 1) {
            if (x < 0)
                x = x + b;
            else
                y = y + a;
            if (x != mul_inv(a, b) || y != mul_inv(b, a)) {
                printf("Inversion error\n");
                exit(1);
            }
        }
        if (++count % 0xffff == 0) {
            printf(".");
            fflush(stdout);
        }
    } while (count < 0xfffff);
    printf("No error found\n");
    
    printf("--- a*b for GF(2^8)  ---\n");
    a = 28; b = 7;
    printf("%d * %d = %d\n", a, b, gf8_mul(a,b));
    a = 127; b = 68;
    printf("%d * %d = %d\n", a, b, gf8_mul(a,b));


    printf("--- all a*b test for GF(2^8) ---\n");
    for (a = 1; a < 256; ++a) {
        if (a == 0) continue;
        b = gf8_inv(a);
        if (gf8_mul(a,b) != 1) {
            printf("Logic error\n");
            exit(1);
        }
        else {
            printf(".");
            fflush(stdout);
        }
    }
    printf("No error found\n");

    printf("--- umul_inv test ---\n");
    a = 5; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %llu, a^-1 mod m = %llu", a, m, ai);
    if (ai != 5534023222112865485u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");
    a = 17; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %llu, a^-1 mod m = %llu", a, m, ai);
    if (ai != 8138269444283625713u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");
    a = 85; m = 9223372036854775808u;
    ai = umul_inv(a, m);
    printf("a = %d, m = %llu, a^-1 mod m = %llu", a, m, ai);
    if (ai != 9006351518340545789u) {
        printf(" <- inversion error\n");
        exit(1);
    }
    else
        printf(" OK\n");

    printf("Congratulations!\n");
    return 0;
}
