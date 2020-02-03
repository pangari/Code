#include "crypt.h"
#include "Logging.h"

#include <blowfish.h>
#include <ice.h>
#include <tea.h>
#include <aes.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
* Copy an 8-byte block.
*/
void block_copy(unsigned char *dest, const unsigned char *src)
{
    int i;
    for(i=0; i<8; i++) dest[i] = src[i];
}
void block_copy16(unsigned char *dest, const unsigned char *src)
{
    int i;
    for(i=0; i<16; i++) dest[i] = src[i];
}

/*
* XOR an 8-byte block onto another one.
*/
void block_xor( unsigned char *dest, const unsigned char *src )
{
    int i;
    for(i=0; i<8; i++) dest[i] ^= src[i];
}
void block_xor16( unsigned char *dest, const unsigned char *src )
{
    int i;
    for(i=0; i<16; i++) dest[i] ^= src[i];
}

/*
* Pad an 8-byte block in positions n .. 7
* The lower 3 bits of the last byte will contain the value n.
* The padding will be an encrypted copy of the IV so as to
* thwart a ciphertext-only attack which might look for padding
* bytes.
*/
void block_pad(unsigned char *buf, size_t n, const unsigned char *padding)
{
    size_t i;
    for(i=n; i<8; i++) buf[i] = padding[i];
    buf[7] = (buf[7] & 0xf8) | (unsigned char)n;
}
void block_pad16(unsigned char *buf, size_t n, const unsigned char *padding)
{
    size_t i;
    for(i=n; i<16; i++) buf[i] = padding[i];
    buf[15] = (buf[15] & 0xf8) | (unsigned char)n;
}

/*
* Build the initial value string.
* If it isn't zero, set it from the time of day.
*/
INITIAL_VECTOR build_iv()
{
    int nTempo;
    char* p;
    char buf1[8];
    char buf2[8];
    long long time[2];
    INITIAL_VECTOR iv;

    time[0] = GetSimplePrecisionTime();

    srand((unsigned int)GetElapsedTime());
    nTempo = rand() % 256;

#ifdef WIN32
    Sleep(nTempo);
#else
    usleep(nTempo * 1000);
#endif

    time[1] = GetSimplePrecisionTime();

    p = (char*)&time[0];
    buf1[0] = p[0];
    buf1[1] = p[7];
    buf1[2] = p[1];
    buf1[3] = p[6];
    buf1[4] = p[2];
    buf1[5] = p[5];
    buf1[6] = p[3];
    buf1[7] = p[4];

    p = (char*)&time[1];
    buf2[0] = p[5];
    buf2[1] = p[2];
    buf2[2] = p[4];
    buf2[3] = p[3];
    buf2[4] = p[7];
    buf2[5] = p[0];
    buf2[6] = p[6];
    buf2[7] = p[1];

    block_xor(buf2, buf1);

    memcpy(&iv.iv, buf2, 8);

    return iv;
}

size_t key_build(const char *passwd, char *key) 
{
    size_t len = strlen(passwd);
    int i = 0;
    unsigned char buf[1024];

    memset(buf, 0, sizeof(buf));

    while(*passwd != '\0') 
    {
        unsigned char c = *passwd & 0x7f;
        int idx = i / 8;
        int bit = i & 7;

        if(bit == 0) 
        {
            buf[idx] = (c << 1);
        } 
        else if(bit == 1) 
        {
            buf[idx] |= c;
        } 
        else 
        {
            buf[idx] |= (c >> (bit - 1));
            buf[idx + 1] = (c << (9 - bit));
        }

        i += 7;
        passwd++;
    }

    len = ((len*7)/8) + (((len*7)%8) ? 1 : 0);
    memcpy(key, buf, len);

    return len;
}


void* ice_init(int level, const char *passwd)
{
    size_t len = strlen(passwd);
    unsigned char key[1024+1];
    unsigned char _passwd[1170+1];

    ICE_KEY *ik;

    memset(_passwd, 0, sizeof(_passwd));

    if(len > 1170)
    {
        memcpy(_passwd, passwd, 1170);
    }
    else
    {
        size_t idx, offset;
        for(idx = 0, offset = 0; idx < (1170 / len); idx++)
        {
            memcpy(&_passwd[offset], passwd, len);
            offset += len;
        }
    }

    memset(key, 0, sizeof(key));
    len = key_build(_passwd, key);

    if((ik = ice_key_create(level)) == NULL)
        return (NULL);

    ice_key_set(ik, key);

    return (void*)ik;
}

void ice_destroy(void* key)
{
    if(key) ice_key_destroy((ICE_KEY*)key);
}

size_t ice_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    unsigned char buf[8], inbuf[8], outbuf[8];

    memcpy(outbuf, &iv, 8); //block_copy

    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        block_xor(inbuf, outbuf);
        ice_key_encrypt((ICE_KEY*)key, inbuf, outbuf);

        memcpy(out, outbuf, 8);
        count += 8;
        out += 8;

        n -= 8;
    }

    memcpy(inbuf, in, n);
    in += n;

    ice_key_encrypt((ICE_KEY*)key, (unsigned char*)&iv, buf); /* Generate padding bytes */
    block_pad(inbuf, n, buf);
    block_xor(inbuf, outbuf);
    ice_key_encrypt((ICE_KEY*)key, inbuf, outbuf);

    memcpy(out, outbuf, 8);
    count += 8;

    return(count);
}

size_t ice_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    int block_loaded_flag;
    unsigned char buf[8], inbuf[8], outbuf[8];

    memcpy(buf, &iv, 8); //block_copy

    block_loaded_flag = 0;
    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        if(block_loaded_flag)
        {
            memcpy(out, outbuf, 8);
            count += 8;
            out += 8;
        }

        ice_key_decrypt((ICE_KEY*)key, inbuf, outbuf);
        block_xor(outbuf, buf);
        block_loaded_flag = 1;

        block_copy(buf, inbuf);

        n -= 8;
    }

    if(n != 0) return 0;

    /* The buffer should contain padding info in its last byte */
    n = outbuf[7] & 7;
    memcpy(out, outbuf, n);
    count += n;

    return(count);
}


void* fish_init(int level, const char *passwd)
{
    size_t len = strlen(passwd);
    unsigned char key[56+1];
    unsigned char _passwd[64+1];

    BLOWFISH_CTX* ctx = (BLOWFISH_CTX*)malloc(sizeof(BLOWFISH_CTX));

    if(!ctx) return NULL;

    memset(_passwd, 0, sizeof(_passwd));

    if(len > 64)
    {
        memcpy(_passwd, passwd, 64);
    }
    else
    {
        size_t idx, offset;
        for(idx = 0, offset = 0; idx < (64 / len); idx++)
        {
            memcpy(&_passwd[offset], passwd, len);
            offset += len;
        }
    }

    memset(key, 0, sizeof(key));
    len = key_build(_passwd, key);

    Blowfish_Init(ctx, key, (int)len);

    return ctx;
}

void fish_destroy(void* key)
{
    if(key)
    {
        memset(key, 0, sizeof(BLOWFISH_CTX));
        free(key);
    }
}

size_t fish_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    unsigned char buf[8], inbuf[8], outbuf[8];
    INITIAL_VECTOR* _buf = (INITIAL_VECTOR*)&buf[0];
    INITIAL_VECTOR* _inbuf = (INITIAL_VECTOR*)&inbuf[0];
    INITIAL_VECTOR* _outbuf = (INITIAL_VECTOR*)&outbuf[0];

    memcpy(outbuf, &iv, 8); //block_copy

    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        block_xor(inbuf, outbuf);
        _outbuf->iv = _inbuf->iv;
        Blowfish_Encrypt((BLOWFISH_CTX*)key, &_outbuf->u.low, &_outbuf->u.high);

        memcpy(out, outbuf, 8);
        count += 8;
        out += 8;

        n -= 8;
    }

    memcpy(inbuf, in, n);
    in += n;

    iv.iv = _buf->iv;
    Blowfish_Encrypt((BLOWFISH_CTX*)key, &_buf->u.low, &_buf->u.high);
    block_pad(inbuf, n, buf);
    block_xor(inbuf, outbuf);
    _outbuf->iv = _inbuf->iv;
    Blowfish_Encrypt((BLOWFISH_CTX*)key, &_outbuf->u.low, &_outbuf->u.high);

    memcpy(out, outbuf, 8);
    count += 8;

    return(count);
}


size_t fish_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    int block_loaded_flag;
    unsigned char buf[8], inbuf[8], outbuf[8];
    INITIAL_VECTOR* _buf = (INITIAL_VECTOR*)&buf[0];
    INITIAL_VECTOR* _inbuf = (INITIAL_VECTOR*)&inbuf[0];
    INITIAL_VECTOR* _outbuf = (INITIAL_VECTOR*)&outbuf[0];

    memcpy(buf, &iv, 8); //block_copy

    block_loaded_flag = 0;
    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        if(block_loaded_flag)
        {
            memcpy(out, outbuf, 8);
            count += 8;
            out += 8;
        }

        _outbuf->iv = _inbuf->iv;
        Blowfish_Decrypt((BLOWFISH_CTX*)key, &_outbuf->u.low, &_outbuf->u.high);
        block_xor(outbuf, buf);
        block_loaded_flag = 1;

        block_copy(buf, inbuf);

        n -= 8;
    }

    if(n != 0) return 0;

    /* The buffer should contain padding info in its last byte */
    n = outbuf[7] & 7;
    memcpy(out, outbuf, n);
    count += n;

    return(count);
}


void* tea_init(int level, const char *passwd)
{
    size_t len = strlen(passwd);
    unsigned char _passwd[18+1];

    long* key = (long*)malloc(20);

    if(!key) return NULL;

    if(len > 18)
    {
        memcpy(_passwd, passwd, 18);
        _passwd[18] = 0;
    }
    else
    {
        memcpy(_passwd, passwd, len);
        _passwd[len] = 0;
    }

    memset(key, 0, 20);
    key[0] = (long)level;
    len = key_build(_passwd, (char*)&key[1]);

    return key;
}
void tea_destroy(void* key)
{
    if(key)
    {
        memset(key, 0, 20);
        free(key);
    }
}

size_t tea_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    unsigned char buf[8], inbuf[8], outbuf[8];
    INITIAL_VECTOR* _buf = (INITIAL_VECTOR*)&buf[0];
    INITIAL_VECTOR* _inbuf = (INITIAL_VECTOR*)&inbuf[0];
    INITIAL_VECTOR* _outbuf = (INITIAL_VECTOR*)&outbuf[0];
    long* _key = (long*)key;

    memcpy(outbuf, &iv, 8); //block_copy

    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        block_xor(inbuf, outbuf);
        _outbuf->iv = _inbuf->iv;
        encipher(&_outbuf->u.low, &_outbuf->u.high, &_key[1], _key[0]);

        memcpy(out, outbuf, 8);
        count += 8;
        out += 8;

        n -= 8;
    }

    memcpy(inbuf, in, n);
    in += n;

    iv.iv = _buf->iv;
    encipher(&_buf->u.low, &_buf->u.high, &_key[1], _key[0]);
    block_pad(inbuf, n, buf);
    block_xor(inbuf, outbuf);
    _outbuf->iv = _inbuf->iv;
    encipher(&_outbuf->u.low, &_outbuf->u.high, &_key[1], _key[0]);

    memcpy(out, outbuf, 8);
    count += 8;

    return(count);
}


size_t tea_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    int block_loaded_flag;
    unsigned char buf[8], inbuf[8], outbuf[8];
    INITIAL_VECTOR* _buf = (INITIAL_VECTOR*)&buf[0];
    INITIAL_VECTOR* _inbuf = (INITIAL_VECTOR*)&inbuf[0];
    INITIAL_VECTOR* _outbuf = (INITIAL_VECTOR*)&outbuf[0];
    long* _key = (long*)key;

    memcpy(buf, &iv, 8); //block_copy

    block_loaded_flag = 0;
    while(n >= 8)
    {
        memcpy(inbuf, in, 8);
        in += 8;

        if(block_loaded_flag)
        {
            memcpy(out, outbuf, 8);
            count += 8;
            out += 8;
        }

        _outbuf->iv = _inbuf->iv;
        decipher(&_outbuf->u.low, &_outbuf->u.high, &_key[1], _key[0]);
        block_xor(outbuf, buf);
        block_loaded_flag = 1;

        block_copy(buf, inbuf);

        n -= 8;
    }

    if(n != 0) return 0;

    /* The buffer should contain padding info in its last byte */
    n = outbuf[7] & 7;
    memcpy(out, outbuf, n);
    count += n;

    return(count);
}


void* aes_init(int level, const char *passwd)
{
    size_t len = strlen(passwd);
    unsigned char key[32+1];
    unsigned char _passwd[36+1];

    aes_context *ctx = (aes_context*)malloc(sizeof(aes_context));

    if(!ctx) return NULL;

    memset(_passwd, 0, sizeof(_passwd));

    if(len > 36)
    {
        memcpy(_passwd, passwd, 36);
    }
    else
    {
        size_t idx, offset;
        for(idx = 0, offset = 0; idx < (36 / len); idx++)
        {
            memcpy(&_passwd[offset], passwd, len);
            offset += len;
        }
    }

    memset(key, 0, sizeof(key));
    len = key_build(_passwd, key);

    if(len > 24)
        aes_set_key(ctx, key, 256);
    else if(len > 16)
        aes_set_key(ctx, key, 192);
    else
        aes_set_key(ctx, key, 128);

    return ctx;
}
void aes_destroy(void* key)
{
    if(key)
    {
        memset(key, 0, sizeof(aes_context));
        free(key);
    }
}
size_t aes_encrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    unsigned char buf[16], inbuf[16], outbuf[16], ivbuf[16];

    memcpy(outbuf, &iv, 8);     //block_copy
    memcpy(&outbuf[8], &iv, 8); //block_copy

    while(n >= 16)
    {
        memcpy(inbuf, in, 16);
        in += 16;

        block_xor16(inbuf, outbuf);
        aes_ecb_encrypt((aes_context*)key, inbuf, outbuf);

        memcpy(out, outbuf, 16);
        count += 16;
        out += 16;

        n -= 16;
    }

    memcpy(inbuf, in, n);
    in += n;

    memcpy(ivbuf, &iv, 8);     //block_copy
    memcpy(&ivbuf[8], &iv, 8); //block_copy

    aes_ecb_encrypt((aes_context*)key, ivbuf, buf); /* Generate padding bytes */
    block_pad16(inbuf, n, buf);
    block_xor16(inbuf, outbuf);
    aes_ecb_encrypt((aes_context*)key, inbuf, outbuf);

    memcpy(out, outbuf, 16);
    count += 16;

    return(count);
}
size_t aes_decrypt(void *key, INITIAL_VECTOR iv, unsigned char* in, unsigned char* out, size_t len)
{
    size_t count = 0;
    size_t n = len;
    int block_loaded_flag;
    unsigned char buf[16], inbuf[16], outbuf[16];

    memcpy(buf, &iv, 8);     //block_copy
    memcpy(&buf[8], &iv, 8); //block_copy

    block_loaded_flag = 0;
    while(n >= 16)
    {
        memcpy(inbuf, in, 16);
        in += 16;

        if(block_loaded_flag)
        {
            memcpy(out, outbuf, 16);
            count += 16;
            out += 16;
        }

        aes_ecb_decrypt((aes_context*)key, inbuf, outbuf);
        block_xor16(outbuf, buf);
        block_loaded_flag = 1;

        block_copy16(buf, inbuf);

        n -= 16;
    }

    if(n != 0) return 0;

    /* The buffer should contain padding info in its last byte */
    n = outbuf[15] & 15;
    memcpy(out, outbuf, n);
    count += n;

    return(count);
}
