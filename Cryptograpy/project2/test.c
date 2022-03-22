#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "aes.h"
/*
   <128 bits AES verification data>
   plain text: 01 23 45 67 89 ab cd ef fe dc ba 98 76 54 32 10
   key: 0f 15 71 c9 47 d9 e8 59 0c b7 ad d6 af 7f 67 98
   cipher text : ff 0b 84 4a 08 53 bf 7c 69 34 ab 43 64 14 8f b9
 */
uint8_t in[BLOCKLEN] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
uint8_t key[KEYLEN] = {0x0f, 0x15, 0x71, 0xc9, 0x47, 0xd9, 0xe8, 0x59, 0x0c, 0xb7, 0xad, 0xd6, 0xaf, 0x7f, 0x67, 0x98};

int main(void)
{
    uint32_t roundKey[RNDKEYSIZE];
    uint8_t *p, buf[BLOCKLEN];
    int i, count;

    printf("<key>\n");
    for (i = 0; i < KEYLEN; ++i)
        printf("%02x ", key[i]);
    printf("\n<round key>\n");
    KeyExpansion(key, roundKey);
    for (i = 0; i < RNDKEYSIZE; ++i) {
        p = (uint8_t *)(roundKey+i);
        printf("%02x %02x %02x %02x ", p[0], p[1], p[2], p[3]);
        if ((i+1)%4 == 0)
            printf("\n");
    }
    printf("---\n<plain text>\n");
    for (i = 0; i < BLOCKLEN; ++i)
        printf("%02x ", in[i]);
    printf("\n<cipher text>\n");
    memcpy(buf, in, BLOCKLEN);
    Cipher(buf, roundKey, ENCRYPT);
    for (i = 0; i < BLOCKLEN; ++i)
        printf("%02x ", buf[i]);
    printf("\n<decrypt text>\n");
    Cipher(buf, roundKey, DECRYPT);
    for (i = 0; i < BLOCKLEN; ++i)
        printf("%02x ", buf[i]);
    printf("\n<inverse cipher text>\n");
    Cipher(buf, roundKey, DECRYPT);
    for (i = 0; i < BLOCKLEN; ++i)
        printf("%02x ", buf[i]);
    printf("\n<decrypt text>\n");
    Cipher(buf, roundKey, ENCRYPT);
    for (i = 0; i < BLOCKLEN; ++i)
        printf("%02x ", buf[i]);
    printf("\n");
    /*
     * Select random key & plain text, verify whether crypt * 100 -> decrypt * 200 -> crypt * 100 == plain text
     */
    printf("Random testing"); fflush(stdout);
    count = 0;
    do {
        arc4random_buf(key, KEYLEN);
        KeyExpansion(key, roundKey);
        arc4random_buf(in, BLOCKLEN);
        memcpy(buf, in, BLOCKLEN);
        for (i = 0; i < 100; ++i)
            Cipher(buf, roundKey, ENCRYPT);
        for (i = 0; i < 200; ++i)
            Cipher(buf, roundKey, DECRYPT);
        for (i = 0; i < 100; ++i)
            Cipher(buf, roundKey, ENCRYPT);
        for (i = 0; i < BLOCKLEN; ++i)
            if (buf[i] != in[i]) {
                    printf("Logic error\n");
                    exit(1);
            }
        if (++count % 0xff == 0) {
            printf(".");
            fflush(stdout);
        }
    } while (count < 0xfff);
    printf("No error found\n");
}
