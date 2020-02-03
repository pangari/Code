#ifndef __CRYPT_H__
#define __CRYPT_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(1)
typedef union _InitialVector
{
    long long iv;
    struct
    {
        long low;
        long high;
    } u;

} INITIAL_VECTOR, *PINITIAL_VECTOR;
#pragma pack()

void block_copy(unsigned char *dest, const unsigned char *src);
void block_xor( unsigned char *dest, const unsigned char *src );
void block_pad(unsigned char *buf, size_t n, const unsigned char *padding);

void block_copy16(unsigned char *dest, const unsigned char *src);
void block_xor16( unsigned char *dest, const unsigned char *src );
void block_pad16(unsigned char *buf, size_t n, const unsigned char *padding);

INITIAL_VECTOR build_iv();
size_t key_build (const char *passwd, char *key);


void* ice_init(int level, const char *passwd);
void ice_destroy(void* key);
size_t ice_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);
size_t ice_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);


void* fish_init(int level, const char *passwd);
void fish_destroy(void* key);
size_t fish_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);
size_t fish_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);


void* tea_init(int level, const char *passwd);
void tea_destroy(void* key);
size_t tea_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);
size_t tea_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);


void* aes_init(int level, const char *passwd);
void aes_destroy(void* key);
size_t aes_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);
size_t aes_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len);


#endif /* __CRYPT_H__ */
