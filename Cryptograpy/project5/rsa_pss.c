#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include "rsa_pss.h"

#if defined(SHA224)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha224;
#elif defined(SHA256)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha256;
#elif defined(SHA384)
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha384;
#else
void (*sha)(const unsigned char *, unsigned int, unsigned char *) = sha512;
#endif

/*
 * Copyright 2020, 2021. Heekuck Oh, all rights reserved
 * rsa_generate_key() - generates RSA keys e, d and n in octet strings.
 * If mode = 0, then e = 65537 is used. Otherwise e will be randomly selected.
 * Carmichael's totient function Lambda(n) is used.
 */
void rsa_generate_key(void *_e, void *_d, void *_n, int mode)
{
    mpz_t p, q, lambda, e, d, n, gcd;
    gmp_randstate_t state;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(p, q, lambda, e, d, n, gcd, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, arc4random());
    /*
     * Generate prime p and q such that 2^(RSAKEYSIZE-1) <= p*q < 2^RSAKEYSIZE
     */
    do {
        do {
            mpz_urandomb(p, state, RSAKEYSIZE/2);
            mpz_setbit(p, 0);
            mpz_setbit(p, RSAKEYSIZE/2-1);
       } while (mpz_probab_prime_p(p, 50) == 0);
        do {
            mpz_urandomb(q, state, RSAKEYSIZE/2);
            mpz_setbit(q, 0);
            mpz_setbit(q, RSAKEYSIZE/2-1);
        } while (mpz_probab_prime_p(q, 50) == 0);
        mpz_mul(n, p, q);
    } while (!mpz_tstbit(n, RSAKEYSIZE-1));
    /*
     * Generate e and d using Lambda(n)
     */
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_lcm(lambda, p, q);
    if (mode == 0)
        mpz_set_ui(e, 65537);
    else do {
        mpz_urandomb(e, state, RSAKEYSIZE);
        mpz_gcd(gcd, e, lambda);
    } while (mpz_cmp(e, lambda) >= 0 || mpz_cmp_ui(gcd, 1) != 0);
    mpz_invert(d, e, lambda);
    /*
     * Convert mpz_t values into octet strings
     */
    mpz_export(_e, NULL, 1, RSAKEYSIZE/8, 1, 0, e);
    mpz_export(_d, NULL, 1, RSAKEYSIZE/8, 1, 0, d);
    mpz_export(_n, NULL, 1, RSAKEYSIZE/8, 1, 0, n);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(p, q, lambda, e, d, n, gcd, NULL);
}

/*
 * Copyright 2020. Heekuck Oh, all rights reserved
 * rsa_cipher() - compute m^k mod n
 * If m >= n then returns EM_MSG_OUT_OF_RANGE, otherwise returns 0 for success.
 */
static int rsa_cipher(void *_m, const void *_k, const void *_n)
{
    mpz_t m, k, n;
    
    /*
     * Initialize mpz variables
     */
    mpz_inits(m, k, n, NULL);
    /*
     * Convert big-endian octets into mpz_t values
     */
    mpz_import(m, RSAKEYSIZE/8, 1, 1, 1, 0, _m);
    mpz_import(k, RSAKEYSIZE/8, 1, 1, 1, 0, _k);
    mpz_import(n, RSAKEYSIZE/8, 1, 1, 1, 0, _n);
    /*
     * Compute m^k mod n
     */
    if (mpz_cmp(m, n) >= 0) {
        mpz_clears(m, k, n, NULL);
        return EM_MSG_OUT_OF_RANGE;
    }
    mpz_powm(m, m, k, n);
    /*
     * Convert mpz_t m into the octet string _m
     */
    mpz_export(_m, NULL, 1, RSAKEYSIZE/8, 1, 0, m);
    /*
     * Free the space occupied by mpz variables
     */
    mpz_clears(m, k, n, NULL);
    return 0;
}

/*
 * Copyright 2020. Heekuck Oh, all rights reserved
 * A mask generation function based on a hash function
 */
static unsigned char *mgf(const unsigned char *mgfSeed, size_t seedLen, unsigned char *mask, size_t maskLen)
{
    uint32_t i, count, c;
    size_t hLen;
    unsigned char *mgfIn, *m;
    
    /*
     * Check if maskLen > 2^32*hLen
     */
    hLen = SHASIZE/8;
    if (maskLen > 0x0100000000*hLen)
        return NULL;
    /*
     * Generate octet string mask
     */
    if ((mgfIn = (unsigned char *)malloc(seedLen+4)) == NULL)
        return NULL;
    memcpy(mgfIn, mgfSeed, seedLen);
    count = maskLen/hLen + (maskLen%hLen ? 1 : 0);
    if ((m = (unsigned char *)malloc(count*hLen)) == NULL)
        return NULL;
    /*
     * Convert i to an octet string C of length 4 octets
     * Concatenate the hash of the seed mgfSeed and C to the octet string T:
     *       T = T || Hash(mgfSeed || C)
     */
    for (i = 0; i < count; i++) {
        c = i;
        mgfIn[seedLen+3] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen+2] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen+1] = c & 0x000000ff; c >>= 8;
        mgfIn[seedLen] = c & 0x000000ff;
        (*sha)(mgfIn, seedLen+4, m+i*hLen);
    }
    /*
     * Copy the mask and free memory
     */
    memcpy(mask, m, maskLen);
    free(mgfIn); free(m);
    return mask;
}

/*
 * rsassa_pss_sign - RSA Signature Scheme with Appendix
 */
int rsassa_pss_sign(const void *m, size_t mLen, const void *d, const void *n, void *s)
{
    uint64_t DB_size, p_size;
    unsigned char *m_prime, *salt, *DB, *H, *MGF, *EM;
    mpz_t salt_;
    gmp_randstate_t state;
    
    /* 
        EM size = RSAKEYSIZE ~= padding_size(dynamic size, more 8bytes) + SHASIZE(salt) + SHASIZE(H) + 1byte(0xbc)
        Error Detect : (SHASIZE*2 + 9 > RSAKEYSIZE) -> EM_HASH_TOO_LONG
    */
    if(SHASIZE*2 + 9 > RSAKEYSIZE)
        return EM_HASH_TOO_LONG;
    // EM_MSG_TOO_LONG : The limit to the SHA input is 0x2000000000000000
    if(mLen > 0x2000000000000000)
        return EM_MSG_TOO_LONG;
    
    // M' size = padding1[8bytes] + Hash(M)[SHASIZE] + salt[SHASIZE] = 8+SHASIZE/8*2 bytes
    m_prime = (unsigned char *)malloc(8+SHASIZE/8*2);
    memset(m_prime,0x00,8);//padding1
    (*sha)(m,mLen,m_prime+8);//Hash(M)
    // generate salt
    salt = (unsigned char *)malloc(SHASIZE/8);
    mpz_inits(salt_, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, arc4random());
    mpz_urandomb(salt_, state, SHASIZE);
    mpz_export(salt, NULL, 1, SHASIZE/8, 1, 0, salt_);//salt
    mpz_clears(salt_, NULL);
    // M' = padding1 + Hash(M) + salt
    memcpy(m_prime+8+SHASIZE/8,salt,SHASIZE/8);

    // DB = padding2 + salt
    DB_size = RSAKEYSIZE/8 - SHASIZE/8 - 1;
    p_size = DB_size - SHASIZE/8;// padding2 size
    DB = (unsigned char *)malloc(DB_size);
    memset(DB,0x00,p_size-1);
    memset(DB+p_size-1,0x01,1);
    memcpy(DB+p_size,salt,SHASIZE/8);

    // H = Hash(M')
    H = (unsigned char *)malloc(SHASIZE/8);
    (*sha)(m_prime,8+SHASIZE/8*2,H);

    // generate MGF
    MGF = (unsigned char *)malloc(DB_size);
    mgf(H,SHASIZE/8,MGF,DB_size);

    // masked DB = DB xor MGF
    for(int i=0;i<DB_size;i++)
        *(DB + i) ^= *(MGF + i);
    
    // EM = masked DB + H + 0xbc
    EM = (unsigned char *)malloc(RSAKEYSIZE/8);
    memcpy(EM,DB,DB_size);
    memcpy(EM+DB_size,H,SHASIZE/8);
    memset(EM+RSAKEYSIZE/8-1,0xbc,1);

    *EM &= 0x7f;// EM(1byte) & 0111 1111 = 0... .... 


    // cipher : EM = EM^d mod n
    rsa_cipher(EM,d,n);
    memcpy(s,EM,RSAKEYSIZE/8);
    free(m_prime); free(salt); free(DB); free(H); free(MGF); free(EM);
    return 0;
}

/*
 * rsassa_pss_verify - RSA Signature Scheme with Appendix
 */
int rsassa_pss_verify(const void *m, size_t mLen, const void *e, const void *n, const void *s)
{
    uint64_t DB_size, p_size, success;
    unsigned char *m_prime, *salt, *DB, *H, *MGF, *EM, *h_prime, *testP;
    DB_size = RSAKEYSIZE/8 - SHASIZE/8 - 1;
    p_size = DB_size - SHASIZE/8;

    //generate EM
    EM = (unsigned char *)malloc(RSAKEYSIZE/8);
    memcpy(EM,s,RSAKEYSIZE/8);
    rsa_cipher(EM,e,n);//encrypt

    /*
        Error Detect : EM_INVALID_LAST, EM_INVALID_INIT
    */
    if(*(EM+RSAKEYSIZE/8-1) != 0xbc)
        return EM_INVALID_LAST;
    if(*EM & 0x80)// EM(1byte) & 1000 0000 = 1(error) or 0
        return EM_INVALID_INIT;

    // generate H
    H = (unsigned char *)malloc(SHASIZE/8);
    memcpy(H,EM+DB_size,SHASIZE/8);

    // generate MGF
    MGF = (unsigned char *)malloc(DB_size);
    mgf(H,SHASIZE/8,MGF,DB_size);

    // generate masked DB
    DB = (unsigned char *)malloc(DB_size);
    memcpy(DB,EM,DB_size);

    // DB = masked DB xor MGF
    for(int i=0;i<DB_size;i++)
        *(DB + i) ^= *(MGF + i);

    if(*DB & 0x80)// DB(1byte) & 1000 0000 = 1
        *DB &= 0x7f;// DB(1byte) & 0111 1111 = 0... .... 

    /*
        Error Detect : EM_INVALID_PD2
        generate testP = 0x00...01
        compare DB with testP
    */
    testP = (unsigned char *)malloc(p_size);
    memset(testP,0x00,p_size-1);
    memset(testP+p_size-1,0x01,1);
    if(memcmp(DB,testP,p_size)!=0)
        return EM_INVALID_PD2;

    // generate salt
    salt = (unsigned char *)malloc(SHASIZE/8);
    memcpy(salt,DB+p_size,SHASIZE/8);

    // generate M'
    m_prime = (unsigned char *)malloc(8+SHASIZE/8*2);
    memset(m_prime,0x00,8);//padding1
    (*sha)(m,mLen,m_prime+8);//Hash(M)
    memcpy(m_prime+8+SHASIZE/8,salt,SHASIZE/8);

    // H' = Hash(M')
    h_prime = (unsigned char *)malloc(SHASIZE/8);
    (*sha)(m_prime,8+SHASIZE/8*2,h_prime);//Hash(M')

    // Compare H' with H
    success = 0;
    if(memcmp(H,h_prime,SHASIZE/8)==0)
        success = 1;
    free(m_prime); free(salt); free(DB); free(H); free(MGF); free(EM); free(testP); free(h_prime);
    if(success)
        return 0;//success
    else
        return EM_HASH_MISMATCH;//error
}
