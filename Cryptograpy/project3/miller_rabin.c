#include "miller_rabin.h"

/*
 * Miller-Rabin Primality Testing against small sets of bases
 *
 * if n < 2^64,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, and 37.
 *
 * if n < 3,317,044,064,679,887,385,961,981,
 * it is enough to test a = 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, and 41.
 */
const uint64_t a[ALEN] = {2,3,5,7,11,13,17,19,23,29,31,37};

/*
 * miller_rabin() - Miller-Rabin Primality Test (deterministic version)
 *
 * n > 3, an odd integer to be tested for primality
 * It returns 1 if n is prime, 0 otherwise.
 */
int miller_rabin(uint64_t n)
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
