#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"

uint circular_shift(uint word, int bits)
{
    return ((word << bits)) | (word) >> (32 - (bits));
}

uint *align(const char* text, int *size)
{
    int i;
    int text_len;
    int data_len;
    int zeros;
    unsigned char *data;
    uint *data_tmp;
    
    text_len = strlen(text);
    data_len = (text_len + LEN_SPACE + 0x3F) & 0xFFFFFFC0;
    zeros = data_len - LEN_SPACE - text_len;
    if (zeros == 0) {
        data_len += 64;
    }

    data = (unsigned char *) malloc(data_len);
    memset(data, 0, data_len);
    strcpy(data, text);
    data[text_len] = 0x80;

    data_tmp = (uint *) data;
    data_tmp[data_len/4-1] = 8 * text_len;

    for (i = 0; i < data_len-4; i+=4) {
        uint tmp = 0;
        tmp |= data[i] << 24;
        tmp |= data[i+1] << 16;
        tmp |= data[i+2] << 8;
        tmp |= data[i+3] << 0;
        data_tmp[i/4] = tmp;
    }
    
    *size = data_len / 4;
    return data_tmp;
}

void sha1(uint *data, int size, uint *hash)
{
    int i, t;
    uint tmp;
    const uint K[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
    uint H[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint W[80] = {0};

    for (i = 0; i < size; i += 16) {

        for (t = 0; t < 16; t++) {
            W[t] = data[i+t];
        }

        for (t = 16; t < 80; t++) {
            W[t] = circular_shift(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);
        }

        memcpy(hash, H, 20);

        for (t = 0; t < 80; t++) {

            if (t < 20) {
                tmp = circular_shift(hash[0], 5) + ((hash[1] & hash[2]) | ((~hash[1]) & hash[3])) + hash[4] + W[t] + K[0];
            } else if (t < 40) {
                tmp = circular_shift(hash[0], 5) + (hash[1] ^ hash[2] ^ hash[3]) + hash[4] + W[t] + K[1];
            } else if (t < 60) {
                tmp = circular_shift(hash[0], 5) + ((hash[1] & hash[2]) | (hash[1] & hash[3]) | (hash[2] & hash[3])) + hash[4] + W[t] + K[2];
            } else if (t < 80) {
                tmp = circular_shift(hash[0], 5) + (hash[1] ^ hash[2] ^ hash[3]) + hash[4] + W[t] + K[3];
            }

            hash[4] = hash[3];
            hash[3] = hash[2];
            hash[2] = circular_shift(hash[1], 30);
            hash[1] = hash[0];
            hash[0] = tmp;
        }  
        
        for (t = 0; t < 5; t++) {
            H[t] = hash[t] + H[t];
        }
    }

    memcpy(hash, H, 20);
}

int main(int argc, char **argv)
{
    int i, size;
    uint *data;
    uint hash[5];

    if (argc != 2) {
        fprintf(stderr, "must enter text you want to hash\n");
        exit(1);
    }

    data = align(argv[1], &size);
    sha1(data, size, hash);

    for (i = 0; i < 5; i++) {
        printf("%x", hash[i]);
    }
    printf("\n");

    free(data);
    return 0;
}
