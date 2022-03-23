#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "mRSA.h"

#define ALEN 12
const uint64_t a[ALEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

static uint64_t gcd(uint64_t a, uint64_t b)
{
    uint64_t k;
    while(b!=0){
        k = a % b;
        a = b;
        b = k;
    }
    return a;
}

static uint64_t mul_inv(uint64_t a, uint64_t m){
    uint64_t d0=a, d1=m;
    uint64_t x0=1, x1=0;
    uint64_t q,temp;
    int sign = -1;
    while(d1>1){
        q=d0/d1;
        temp = d0-q*d1; d0=d1; d1=temp;
        temp = x0+q*x1; x0=x1; x1=temp;
        sign = ~sign;
    }
    if(d1==1)
        return (sign?m-x1:x1);
    else
        return 0;
}

static uint64_t mod_add(uint64_t a, uint64_t b, uint64_t m)
{
    a = a%m;
    b = b%m;
    if(a >= m-b)
        return a-(m-b);
    else
        return a + b;
}

static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m)
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

static uint64_t mod_pow(uint64_t a, uint64_t b, uint64_t m)
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

static int miller_rabin(uint64_t n)
{
    uint64_t q, k=0;
    q=n-1;
    while((q&1)==0){ // q%2==0
        k++;
        q=q>>1;// q = q/2
    }

    if(n!=2 && k==0)// k > 0;
       return 0;
    
    for(int i=0;i<ALEN;i++){
        int incon = 0;

        if(a[i]>=n-1) return 1; // 1 < a < n-1
        
        uint64_t t = mod_pow(a[i],q,n);
        if(t==1 || t == n-1) continue; //inconclusive
        for(int j=1;j<k;j++){
            t = mod_mul(t,t,n);// t^(2^j) == (t*t)^j 
            if(t==n-1){ // inconclusive
                incon = 1;
                break;
            }
        }
        if(incon) continue;
        return 0;//composite
    }
    return 1;
}


/*
 * mRSA_generate_key() - generates mini RSA keys e, d and n
 * Carmichael's totient function Lambda(n) is used.
 */
void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n)
{
    uint64_t p,q,lcm,min;
    //create prime p
    while(1){
        arc4random_buf(&p,sizeof(uint32_t));
        /*
            fermat theorem( 2^p mod p != 2 -> p is not prime)
            (p&1) == 1 is odd, (p&1) == 0 is even
         */
        if(p < 3 || (p&1)==0 || mod_pow(2,p,p)!=2)
            continue;
        if(miller_rabin(p))
            break;
    }
    /* 
        create prime q
        2^63 <= p*q < 2^64, 2^63/p <= q < 2^64/p
        0 <= q_range < 2^64/p - 2^63/p == 2^63/p
     */
    min = (MINIMUM_N-1)/p;//(2^63-1)/p 
    while(1){
        q = arc4random_uniform(min); // 0 <= q_range < (2^63-1)/p
        q += min + 1; // (2^63-1)/p < q < (2^64)/p
        if(p==q)
            continue;
        if((q&1)==0 || mod_pow(2,q,q)!=2)
            continue;
        if(miller_rabin(q))
            break;
    }
    
    *n = p*q;
    lcm = (p-1)*(q-1)/gcd(p-1,q-1);
    if(gcd(65537,lcm)==1)
        *e = 65537; // for fast encryption
    else{           // gcd(65537,LCM) != 1;
        while(1){
            *e = arc4random_uniform(lcm);
            if(*e < 3 || (*e&1)==0)
                continue;
            if(gcd(*e,lcm)==1)
                break;
        }
    }
    *d = mul_inv(*e,lcm); // ed = 1 mod lcm, d = e^(-1) mod lcm;
}

/*
 * mRSA_cipher() - compute m^k mod n
 * If data >= n then returns 1 (error), otherwise 0 (success).
 */
int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n)
{
    if(*m>=n)
        return 1;
    *m = mod_pow(*m,k,n);
    return 0;
}
