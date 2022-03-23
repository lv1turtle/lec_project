#ifndef mRSA_H
#define mRSA_H

#include <stdint.h>

#define PRIME 1
#define COMPOSITE 0
#define MINIMUM_N 0x8000000000000000

void mRSA_generate_key(uint64_t *e, uint64_t *d, uint64_t *n);
int mRSA_cipher(uint64_t *m, uint64_t k, uint64_t n);

#endif
