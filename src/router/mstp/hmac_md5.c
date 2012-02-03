/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include <string.h>
#include <sys/types.h>
#include <asm/types.h>

#include "mstp.h"

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef __u16 UINT2;

/* UINT4 defines a four byte word */
typedef __u32 UINT4;

/* MD5 context. */
typedef struct
{
    UINT4 state[4];           /* state (ABCD) */
    UINT4 count[2];           /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

#define MD5_memcpy(output, input, len) memcpy((output), (input), (len))
#define MD5_memset(output, value, len) memset((output), (value), (len))

/* Constants for MD5Transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static const unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode(output, input, len)
unsigned char *output;
UINT4 *input;
unsigned int len;
{
    unsigned int i, j;

    for(i = 0, j = 0; j < len; i++, j += 4)
    {
        output[j]     = (unsigned char)(input[i] & 0xff);
        output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode(output, input, len)
UINT4 *output;
unsigned char *input;
unsigned int len;
{
    unsigned int i, j;

    for(i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform(state, block)
UINT4 state[4];
unsigned char block[64];
{
    UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    /* Round 1 */
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information. */
        /* No need in MSTP ;) */
    /* MD5_memset((POINTER)x, 0, sizeof (x)); */
}

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
static void MD5Init(context)
MD5_CTX *context;                                        /* context */
{
    context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants. */
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
static void MD5Update(context, input, inputLen)
MD5_CTX *context;                                        /* context */
unsigned char *input;                                /* input block */
unsigned int inputLen;                     /* length of input block */
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((UINT4)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible. */
    if(inputLen >= partLen)
    {
        MD5_memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        MD5Transform(context->state, context->buffer);
        for(i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform(context->state, &input[i]);
        index = 0;
    }
    else
        i = 0;

    /* Buffer remaining input */
    MD5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i],
               inputLen - i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
static void MD5Final(digest, context)
unsigned char digest[16];                         /* message digest */
MD5_CTX *context;                                        /* context */
{
    unsigned char bits[8];
    unsigned int index, padLen;

    /* Save number of bits */
    Encode(bits, context->count, 8);

    /* Pad out to 56 mod 64. */
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen);

    /* Append length (before padding) */
    MD5Update(context, bits, 8);

    /* Store state in digest */
    Encode(digest, context->state, 16);

    /* Zeroize sensitive information. */
        /* No need in MSTP ;) */
    /* MD5_memset((POINTER)context, 0, sizeof(*context)); */
}

/*
** Function: hmac_md5 from RFC-2104
*/
void hmac_md5(text, text_len, key, key_len, digest)
unsigned char*  text;       /* pointer to data stream */
int             text_len;   /* length of data stream */
unsigned char*  key;        /* pointer to authentication key */
int             key_len;    /* length of authentication key */
caddr_t         digest;     /* caller digest to be filled in */
{
    MD5_CTX context;
    unsigned char k_ipad[65];    /* inner padding -
                                  * key XORd with ipad
                                  */
    unsigned char k_opad[65];    /* outer padding -
                                  * key XORd with opad
                                  */
    unsigned char tk[16];
    int i;
    /* if key is longer than 64 bytes reset it to key=MD5(key) */
    if(key_len > 64)
    {
        MD5_CTX tctx;

        MD5Init(&tctx);
        MD5Update(&tctx, key, key_len);
        MD5Final(tk, &tctx);

        key = tk;
        key_len = 16;
    }

    /*
     * the HMAC_MD5 transform looks like:
     *
     * MD5(K XOR opad, MD5(K XOR ipad, text))
     *
     * where K is an n byte key
     * ipad is the byte 0x36 repeated 64 times
     * opad is the byte 0x5c repeated 64 times
     * and text is the data being protected
     */

    /* start out by storing key in pads */
    bzero(k_ipad, sizeof k_ipad);
    bzero(k_opad, sizeof k_opad);
    bcopy(key, k_ipad, key_len);
    bcopy( key, k_opad, key_len);

    /* XOR key with ipad and opad values */
    for(i = 0; i < 64; ++i)
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
    /*
     * perform inner MD5
     */
    MD5Init(&context);                   /* init context for 1st
                                          * pass */
    MD5Update(&context, k_ipad, 64);     /* start with inner pad */
    MD5Update(&context, text, text_len); /* then text of datagram */
    MD5Final(digest, &context);          /* finish up 1st pass */
    /*
     * perform outer MD5
     */
    MD5Init(&context);                   /* init context for 2nd
                                          * pass */
    MD5Update(&context, k_opad, 64);     /* start with outer pad */
    MD5Update(&context, digest, 16);     /* then results of 1st
                                          * hash */
    MD5Final(digest, &context);          /* finish up 2nd pass */
}

#ifdef HMAC_MDS_TEST_FUNCTIONS
/* Digests a string */
static void MD5String(string, digest)
char *string;
caddr_t digest;     /* caller digest to be filled in */
{
    MD5_CTX context;
    unsigned int len = strlen(string);

    MD5Init(&context);
    MD5Update(&context, string, len);
    MD5Final(digest, &context);
}

/* Digests a reference suite of strings */
bool MD5TestSuite(void)
{
    unsigned char digest[16];
    unsigned char key[16];
    unsigned char mstp_key[16] = HMAC_KEY;
    unsigned char data[4096 * 2];
    int i;

    /* Tests from RFC-1231 */
    MD5String("", digest);
    {
        unsigned char expected_result[16] = {
            0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04,
            0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("a", digest);
    {
        unsigned char expected_result[16] = {
            0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8,
            0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("abc", digest);
    {
        unsigned char expected_result[16] = {
            0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
            0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("message digest", digest);
    {
        unsigned char expected_result[16] = {
            0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d,
            0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("abcdefghijklmnopqrstuvwxyz", digest);
    {
        unsigned char expected_result[16] = {
            0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00,
            0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
              digest);
    {
        unsigned char expected_result[16] = {
            0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5,
            0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    MD5String("1234567890123456789012345678901234567890"
              "1234567890123456789012345678901234567890", digest);
    {
        unsigned char expected_result[16] = {
            0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55,
            0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a};
        if(memcmp(expected_result, digest, 16))
            return false;
    }

    /* Tests from RFC-2104 */
    memset(key, 0x0B, 16);
    hmac_md5("Hi There", 8, key, 16, digest);
    {
        unsigned char expected_result[16] = {
            0x92, 0x94, 0x72, 0x7a, 0x36, 0x38, 0xbb, 0x1c,
            0x13, 0xf4, 0x8e, 0xf8, 0x15, 0x8b, 0xfc, 0x9d};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    hmac_md5("what do ya want for nothing?", 28, "Jefe", 4, digest);
    {
        unsigned char expected_result[16] = {
            0x75, 0x0c, 0x78, 0x3e, 0x6a, 0xb0, 0xb5, 0x03,
            0xea, 0xa8, 0x6e, 0x31, 0x0a, 0x5d, 0xb7, 0x38};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    memset(key, 0xAA, 16);
    memset(data, 0xDD, 50);
    hmac_md5(data, 50, key, 16, digest);
    {
        unsigned char expected_result[16] = {
            0x56, 0xbe, 0x34, 0x52, 0x1d, 0x14, 0x4c, 0x88,
            0xdb, 0xb8, 0xc7, 0x33, 0xf0, 0xe8, 0xb3, 0xf6};
        if(memcmp(expected_result, digest, 16))
            return false;
    }

    /* Tests from IEEE 802.1Q-2005 13.7 Table 13-2 */
    memset(data, 0, 4096 * 2);
    hmac_md5(data, 4096 * 2, mstp_key, 16, digest);
    {
        unsigned char expected_result[16] = {
            0xac, 0x36, 0x17, 0x7f, 0x50, 0x28, 0x3c, 0xd4,
            0xb8, 0x38, 0x21, 0xd8, 0xab, 0x26, 0xde, 0x62};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    for(i = 3; i < 4095 * 2; i+= 2)
        data[i] = 1;
    hmac_md5(data, 4096 * 2, mstp_key, 16, digest);
    {
        unsigned char expected_result[16] = {
            0xe1, 0x3a, 0x80, 0xf1, 0x1e, 0xd0, 0x85, 0x6a,
            0xcd, 0x4e, 0xe3, 0x47, 0x69, 0x41, 0xc7, 0x3b};
        if(memcmp(expected_result, digest, 16))
            return false;
    }
    for(i = 3; i < 4095 * 2; i+= 2)
        data[i] = (i / 2) % 32 + 1;
    hmac_md5(data, 4096 * 2, mstp_key, 16, digest);
    {
        unsigned char expected_result[16] = {
            0x9d, 0x14, 0x5c, 0x26, 0x7d, 0xbe, 0x9f, 0xb5,
            0xd8, 0x93, 0x44, 0x1b, 0xe3, 0xba, 0x08, 0xce};
        if(memcmp(expected_result, digest, 16))
            return false;
    }

    return true;
}
#endif /* HMAC_MDS_TEST_FUNCTIONS */
