/* This is an independent implementation of the encryption algorithm:
 *
 * Serpent by Ross Anderson, Eli Biham and Lars Knudsen 
 *
 * which is a candidate algorithm in the Advanced Encryption Standard
 * programme of the US National Institute of Standards and Technology
 *
 * Copyright in this implementation is held by Dr B R Gladman but I
 * hereby give permission for its free direct or derivative use subject
 * to acknowledgment of its origin and compliance with any conditions
 * that the originators of the algorithm place on its exploitation.
 *
 * Dr Brian Gladman (gladman@seven77.demon.co.uk) 14th January 1999
 */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/*
Algorithm serpent (serpent.c)

128 bit key:
Key Setup:    2366 cycles
Encrypt:       954 cycles =    26.8 mbits/sec
Decrypt:       907 cycles =    28.2 mbits/sec
Mean:          931 cycles =    27.5 mbits/sec

192 bit key:
Key Setup:    2382 cycles
Encrypt:       967 cycles =    26.5 mbits/sec
Decrypt:       915 cycles =    28.0 mbits/sec
Mean:          941 cycles =    27.2 mbits/sec

256 bit key:
Key Setup:    2360 cycles
Encrypt:       967 cycles =    26.5 mbits/sec
Decrypt:       915 cycles =    28.0 mbits/sec
Mean:          941 cycles =    27.2 mbits/sec
*/

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "serpent.h"

#define _mcrypt_set_key serpent_LTX__mcrypt_set_key
#define _mcrypt_encrypt serpent_LTX__mcrypt_encrypt
#define _mcrypt_decrypt serpent_LTX__mcrypt_decrypt
#define _mcrypt_get_size serpent_LTX__mcrypt_get_size
#define _mcrypt_get_block_size serpent_LTX__mcrypt_get_block_size
#define _is_block_algorithm serpent_LTX__is_block_algorithm
#define _mcrypt_get_key_size serpent_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes serpent_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name serpent_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test serpent_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version serpent_LTX__mcrypt_algorithm_version

/* Partially optimised Serpent S Box boolean functions derived  */
/* using a recursive descent analyser but without a full search */
/* of all subtrees. This set of S boxes is the result of work   */
/* by Sam Simpson and Brian Gladman using the spare time on a   */
/* cluster of high capacity servers to search for S boxes with  */
/* this customised search engine.                               */
/*                                                              */
/* Copyright:   Dr B. R Gladman (gladman@seven77.demon.co.uk)   */
/*              and Sam Simpson (s.simpson@mia.co.uk)           */
/*              17th December 1998                              */
/*                                                              */
/* We hereby give permission for information in this file to be */
/* used freely subject only to acknowledgement of its origin    */

/* 15 terms */

#define sb0(a,b,c,d,e,f,g,h)    \
    t1 = a ^ d;     \
    t2 = a & d;     \
    t3 = c ^ t1;    \
    t6 = b & t1;    \
    t4 = b ^ t3;    \
    t10 = ~t3;      \
    h = t2 ^ t4;    \
    t7 = a ^ t6;    \
    t14 = ~t7;      \
    t8 = c | t7;    \
    t11 = t3 ^ t7;  \
    g = t4 ^ t8;    \
    t12 = h & t11;  \
    f = t10 ^ t12;  \
    e = t12 ^ t14

/* 15 terms */

#define ib0(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = a ^ b;     \
    t3 = t1 | t2;   \
    t4 = d ^ t3;    \
    t7 = d & t2;    \
    t5 = c ^ t4;    \
    t8 = t1 ^ t7;   \
    g = t2 ^ t5;    \
    t11 = a & t4;   \
    t9 = g & t8;    \
    t14 = t5 ^ t8;  \
    f = t4 ^ t9;    \
    t12 = t5 | f;   \
    h = t11 ^ t12;  \
    e = h ^ t14

/* 14 terms!  */

#define sb1(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = b ^ t1;    \
    t3 = a | t2;    \
    t4 = d | t2;    \
    t5 = c ^ t3;    \
    g = d ^ t5;     \
    t7 = b ^ t4;    \
    t8 = t2 ^ g;    \
    t9 = t5 & t7;   \
    h = t8 ^ t9;    \
    t11 = t5 ^ t7;  \
    f = h ^ t11;    \
    t13 = t8 & t11; \
    e = t5 ^ t13

/* 17 terms */

#define ib1(a,b,c,d,e,f,g,h)    \
    t1 = a ^ d;     \
    t2 = a & b;     \
    t3 = b ^ c;     \
    t4 = a ^ t3;    \
    t5 = b | d;     \
    t7 = c | t1;    \
    h = t4 ^ t5;    \
    t8 = b ^ t7;    \
    t11 = ~t2;      \
    t9 = t4 & t8;   \
    f = t1 ^ t9;    \
    t13 = t9 ^ t11; \
    t12 = h & f;    \
    g = t12 ^ t13;  \
    t15 = a & d;    \
    t16 = c ^ t13;  \
    e = t15 ^ t16

/* 16 terms */

#define sb2(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = b ^ d;     \
    t3 = c & t1;    \
    t13 = d | t1;   \
    e = t2 ^ t3;    \
    t5 = c ^ t1;    \
    t6 = c ^ e;     \
    t7 = b & t6;    \
    t10 = e | t5;   \
    h = t5 ^ t7;    \
    t9 = d | t7;    \
    t11 = t9 & t10; \
    t14 = t2 ^ h;   \
    g = a ^ t11;    \
    t15 = g ^ t13;  \
    f = t14 ^ t15

/* 16 terms */

#define ib2(a,b,c,d,e,f,g,h)    \
    t1 = b ^ d;     \
    t2 = ~t1;       \
    t3 = a ^ c;     \
    t4 = c ^ t1;    \
    t7 = a | t2;    \
    t5 = b & t4;    \
    t8 = d ^ t7;    \
    t11 = ~t4;      \
    e = t3 ^ t5;    \
    t9 = t3 | t8;   \
    t14 = d & t11;  \
    h = t1 ^ t9;    \
    t12 = e | h;    \
    f = t11 ^ t12;  \
    t15 = t3 ^ t12; \
    g = t14 ^ t15

/* 17 terms */

#define sb3(a,b,c,d,e,f,g,h)    \
    t1 = a ^ c;     \
    t2 = d ^ t1;    \
    t3 = a & t2;    \
    t4 = d ^ t3;    \
    t5 = b & t4;    \
    g = t2 ^ t5;    \
    t7 = a | g;     \
    t8 = b | d;     \
    t11 = a | d;    \
    t9 = t4 & t7;   \
    f = t8 ^ t9;    \
    t12 = b ^ t11;  \
    t13 = g ^ t9;   \
    t15 = t3 ^ t8;  \
    h = t12 ^ t13;  \
    t16 = c & t15;  \
    e = t12 ^ t16

/* 16 term solution that performs less well than 17 term one
   in my environment (PPro/PII)                                  

#define sb3(a,b,c,d,e,f,g,h)    \
    t1 = a ^ b;     \
    t2 = a & c;     \
    t3 = a | d;     \
    t4 = c ^ d;     \
    t5 = t1 & t3;   \
    t6 = t2 | t5;   \
    g = t4 ^ t6;    \
    t8 = b ^ t3;    \
    t9 = t6 ^ t8;   \
    t10 = t4 & t9;  \
    e = t1 ^ t10;   \
    t12 = g & e;    \
    f = t9 ^ t12;   \
    t14 = b | d;    \
    t15 = t4 ^ t12; \
    h = t14 ^ t15
*/

/* 17 terms */

#define ib3(a,b,c,d,e,f,g,h)    \
    t1 = b ^ c;     \
    t2 = b | c;     \
    t3 = a ^ c;     \
    t7 = a ^ d;     \
    t4 = t2 ^ t3;   \
    t5 = d | t4;    \
    t9 = t2 ^ t7;   \
    e = t1 ^ t5;    \
    t8 = t1 | t5;   \
    t11 = a & t4;   \
    g = t8 ^ t9;    \
    t12 = e | t9;   \
    f = t11 ^ t12;  \
    t14 = a & g;    \
    t15 = t2 ^ t14; \
    t16 = e & t15;  \
    h = t4 ^ t16

/* 15 terms */

#define sb4(a,b,c,d,e,f,g,h)    \
    t1 = a ^ d;     \
    t2 = d & t1;    \
    t3 = c ^ t2;    \
    t4 = b | t3;    \
    h = t1 ^ t4;    \
    t6 = ~b;        \
    t7 = t1 | t6;   \
    e = t3 ^ t7;    \
    t9 = a & e;     \
    t10 = t1 ^ t6;  \
    t11 = t4 & t10; \
    g = t9 ^ t11;   \
    t13 = a ^ t3;   \
    t14 = t10 & g;  \
    f = t13 ^ t14

/* 17 terms */

#define ib4(a,b,c,d,e,f,g,h)    \
    t1 = c ^ d;     \
    t2 = c | d;     \
    t3 = b ^ t2;    \
    t4 = a & t3;    \
    f = t1 ^ t4;    \
    t6 = a ^ d;     \
    t7 = b | d;     \
    t8 = t6 & t7;   \
    h = t3 ^ t8;    \
    t10 = ~a;       \
    t11 = c ^ h;    \
    t12 = t10 | t11;\
    e = t3 ^ t12;   \
    t14 = c | t4;   \
    t15 = t7 ^ t14; \
    t16 = h | t10;  \
    g = t15 ^ t16

/* 16 terms */

#define sb5(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = a ^ b;     \
    t3 = a ^ d;     \
    t4 = c ^ t1;    \
    t5 = t2 | t3;   \
    e = t4 ^ t5;    \
    t7 = d & e;     \
    t8 = t2 ^ e;    \
    t10 = t1 | e;   \
    f = t7 ^ t8;    \
    t11 = t2 | t7;  \
    t12 = t3 ^ t10; \
    t14 = b ^ t7;   \
    g = t11 ^ t12;  \
    t15 = f & t12;  \
    h = t14 ^ t15

/* 16 terms */

#define ib5(a,b,c,d,e,f,g,h)    \
    t1 = ~c;        \
    t2 = b & t1;    \
    t3 = d ^ t2;    \
    t4 = a & t3;    \
    t5 = b ^ t1;    \
    h = t4 ^ t5;    \
    t7 = b | h;     \
    t8 = a & t7;    \
    f = t3 ^ t8;    \
    t10 = a | d;    \
    t11 = t1 ^ t7;  \
    e = t10 ^ t11;  \
    t13 = a ^ c;    \
    t14 = b & t10;  \
    t15 = t4 | t13; \
    g = t14 ^ t15

/* 15 terms */

#define sb6(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = a ^ d;     \
    t3 = b ^ t2;    \
    t4 = t1 | t2;   \
    t5 = c ^ t4;    \
    f = b ^ t5;     \
    t13 = ~t5;      \
    t7 = t2 | f;    \
    t8 = d ^ t7;    \
    t9 = t5 & t8;   \
    g = t3 ^ t9;    \
    t11 = t5 ^ t8;  \
    e = g ^ t11;    \
    t14 = t3 & t11; \
    h = t13 ^ t14

/* 15 terms */

#define ib6(a,b,c,d,e,f,g,h)    \
    t1 = ~a;        \
    t2 = a ^ b;     \
    t3 = c ^ t2;    \
    t4 = c | t1;    \
    t5 = d ^ t4;    \
    t13 = d & t1;   \
    f = t3 ^ t5;    \
    t7 = t3 & t5;   \
    t8 = t2 ^ t7;   \
    t9 = b | t8;    \
    h = t5 ^ t9;    \
    t11 = b | h;    \
    e = t8 ^ t11;   \
    t14 = t3 ^ t11; \
    g = t13 ^ t14

/* 17 terms */

#define sb7(a,b,c,d,e,f,g,h)    \
    t1 = ~c;        \
    t2 = b ^ c;     \
    t3 = b | t1;    \
    t4 = d ^ t3;    \
    t5 = a & t4;    \
    t7 = a ^ d;     \
    h = t2 ^ t5;    \
    t8 = b ^ t5;    \
    t9 = t2 | t8;   \
    t11 = d & t3;   \
    f = t7 ^ t9;    \
    t12 = t5 ^ f;   \
    t15 = t1 | t4;  \
    t13 = h & t12;  \
    g = t11 ^ t13;  \
    t16 = t12 ^ g;  \
    e = t15 ^ t16

/* 17 terms */

#define ib7(a,b,c,d,e,f,g,h)    \
    t1 = a & b;     \
    t2 = a | b;     \
    t3 = c | t1;    \
    t4 = d & t2;    \
    h = t3 ^ t4;    \
    t6 = ~d;        \
    t7 = b ^ t4;    \
    t8 = h ^ t6;    \
    t11 = c ^ t7;   \
    t9 = t7 | t8;   \
    f = a ^ t9;     \
    t12 = d | f;    \
    e = t11 ^ t12;  \
    t14 = a & h;    \
    t15 = t3 ^ f;   \
    t16 = e ^ t14;  \
    g = t15 ^ t16

#define k_xor(r,a,b,c,d)    \
    a ^= spkey->l_key[4 * r +  8]; \
    b ^= spkey->l_key[4 * r +  9]; \
    c ^= spkey->l_key[4 * r + 10]; \
    d ^= spkey->l_key[4 * r + 11]

#define k_set(r,a,b,c,d)    \
    a = spkey->l_key[4 * r +  8];  \
    b = spkey->l_key[4 * r +  9];  \
    c = spkey->l_key[4 * r + 10];  \
    d = spkey->l_key[4 * r + 11]

#define k_get(r,a,b,c,d)    \
    spkey->l_key[4 * r +  8] = a;  \
    spkey->l_key[4 * r +  9] = b;  \
    spkey->l_key[4 * r + 10] = c;  \
    spkey->l_key[4 * r + 11] = d

/* the linear transformation and its inverse    */

#define rot(a,b,c,d)    \
    a = rotl32(a, 13);    \
    c = rotl32(c, 3);     \
    d ^= c ^ (a << 3);  \
    b ^= a ^ c;         \
    d = rotl32(d, 7);     \
    b = rotl32(b, 1);     \
    a ^= b ^ d;         \
    c ^= d ^ (b << 7);  \
    a = rotl32(a, 5);     \
    c = rotl32(c, 22)

#define irot(a,b,c,d)   \
    c = rotr32(c, 22);    \
    a = rotr32(a, 5);     \
    c ^= d ^ (b << 7);  \
    a ^= b ^ d;         \
    d = rotr32(d, 7);     \
    b = rotr32(b, 1);     \
    d ^= c ^ (a << 3);  \
    b ^= a ^ c;         \
    c = rotr32(c, 3);     \
    a = rotr32(a, 13)

/* initialise the key schedule from the user supplied key   */

WIN32DLL_DEFINE
    int _mcrypt_set_key(SERPENT_KEY * spkey, const word32 * in_key,
			word32 key_len)
{
	word32 i, lk, a, b, c, d, e, f, g, h;
	word32 t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14,
	    t15, t16;

	key_len *= 8;
	if (key_len > 256)

		return -1;

	i = 0;
	lk = (key_len + 31) / 32;

	while (i < lk) {
#ifdef WORDS_BIGENDIAN
		spkey->l_key[i] = byteswap32(in_key[i]);
#else
		spkey->l_key[i] = (in_key[i]);
#endif
		i++;
	}

	if (key_len < 256) {
		while (i < 8)

			spkey->l_key[i++] = 0;

		i = key_len / 32;
		lk = 1 << key_len % 32;
		spkey->l_key[i] = (spkey->l_key[i] & (lk - 1)) | lk;
	}

	for (i = 0; i < 132; ++i) {
		lk = spkey->l_key[i] ^ spkey->l_key[i +
						    3] ^ spkey->l_key[i +
								      5] ^
		    spkey->l_key[i + 7] ^ 0x9e3779b9 ^ i;

		spkey->l_key[i + 8] = (lk << 11) | (lk >> 21);
	}

	k_set(0, a, b, c, d);
	sb3(a, b, c, d, e, f, g, h);
	k_get(0, e, f, g, h);
	k_set(1, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	k_get(1, e, f, g, h);
	k_set(2, a, b, c, d);
	sb1(a, b, c, d, e, f, g, h);
	k_get(2, e, f, g, h);
	k_set(3, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	k_get(3, e, f, g, h);
	k_set(4, a, b, c, d);
	sb7(a, b, c, d, e, f, g, h);
	k_get(4, e, f, g, h);
	k_set(5, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	k_get(5, e, f, g, h);
	k_set(6, a, b, c, d);
	sb5(a, b, c, d, e, f, g, h);
	k_get(6, e, f, g, h);
	k_set(7, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	k_get(7, e, f, g, h);
	k_set(8, a, b, c, d);
	sb3(a, b, c, d, e, f, g, h);
	k_get(8, e, f, g, h);
	k_set(9, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	k_get(9, e, f, g, h);
	k_set(10, a, b, c, d);
	sb1(a, b, c, d, e, f, g, h);
	k_get(10, e, f, g, h);
	k_set(11, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	k_get(11, e, f, g, h);
	k_set(12, a, b, c, d);
	sb7(a, b, c, d, e, f, g, h);
	k_get(12, e, f, g, h);
	k_set(13, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	k_get(13, e, f, g, h);
	k_set(14, a, b, c, d);
	sb5(a, b, c, d, e, f, g, h);
	k_get(14, e, f, g, h);
	k_set(15, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	k_get(15, e, f, g, h);
	k_set(16, a, b, c, d);
	sb3(a, b, c, d, e, f, g, h);
	k_get(16, e, f, g, h);
	k_set(17, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	k_get(17, e, f, g, h);
	k_set(18, a, b, c, d);
	sb1(a, b, c, d, e, f, g, h);
	k_get(18, e, f, g, h);
	k_set(19, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	k_get(19, e, f, g, h);
	k_set(20, a, b, c, d);
	sb7(a, b, c, d, e, f, g, h);
	k_get(20, e, f, g, h);
	k_set(21, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	k_get(21, e, f, g, h);
	k_set(22, a, b, c, d);
	sb5(a, b, c, d, e, f, g, h);
	k_get(22, e, f, g, h);
	k_set(23, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	k_get(23, e, f, g, h);
	k_set(24, a, b, c, d);
	sb3(a, b, c, d, e, f, g, h);
	k_get(24, e, f, g, h);
	k_set(25, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	k_get(25, e, f, g, h);
	k_set(26, a, b, c, d);
	sb1(a, b, c, d, e, f, g, h);
	k_get(26, e, f, g, h);
	k_set(27, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	k_get(27, e, f, g, h);
	k_set(28, a, b, c, d);
	sb7(a, b, c, d, e, f, g, h);
	k_get(28, e, f, g, h);
	k_set(29, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	k_get(29, e, f, g, h);
	k_set(30, a, b, c, d);
	sb5(a, b, c, d, e, f, g, h);
	k_get(30, e, f, g, h);
	k_set(31, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	k_get(31, e, f, g, h);
	k_set(32, a, b, c, d);
	sb3(a, b, c, d, e, f, g, h);
	k_get(32, e, f, g, h);

	return 0;
}

/* encrypt a block of text  */

WIN32DLL_DEFINE void _mcrypt_encrypt(SERPENT_KEY * spkey, word32 * in_blk)
{
	word32 a, b, c, d, e, f, g, h;
	word32 t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14,
	    t15, t16;

#ifdef WORDS_BIGENDIAN
	a = byteswap32(in_blk[0]);
	b = byteswap32(in_blk[1]);
	c = byteswap32(in_blk[2]);
	d = byteswap32(in_blk[3]);
#else
	a = in_blk[0];
	b = in_blk[1];
	c = in_blk[2];
	d = in_blk[3];
#endif

	k_xor(0, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(1, e, f, g, h);
	sb1(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(2, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(3, e, f, g, h);
	sb3(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(4, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(5, e, f, g, h);
	sb5(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(6, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(7, e, f, g, h);
	sb7(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(8, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(9, e, f, g, h);
	sb1(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(10, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(11, e, f, g, h);
	sb3(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(12, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(13, e, f, g, h);
	sb5(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(14, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(15, e, f, g, h);
	sb7(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(16, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(17, e, f, g, h);
	sb1(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(18, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(19, e, f, g, h);
	sb3(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(20, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(21, e, f, g, h);
	sb5(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(22, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(23, e, f, g, h);
	sb7(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(24, a, b, c, d);
	sb0(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(25, e, f, g, h);
	sb1(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(26, a, b, c, d);
	sb2(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(27, e, f, g, h);
	sb3(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(28, a, b, c, d);
	sb4(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(29, e, f, g, h);
	sb5(e, f, g, h, a, b, c, d);
	rot(a, b, c, d);
	k_xor(30, a, b, c, d);
	sb6(a, b, c, d, e, f, g, h);
	rot(e, f, g, h);
	k_xor(31, e, f, g, h);
	sb7(e, f, g, h, a, b, c, d);
	k_xor(32, a, b, c, d);

#ifdef WORDS_BIGENDIAN
	in_blk[0] = byteswap32(a);
	in_blk[1] = byteswap32(b);
	in_blk[2] = byteswap32(c);
	in_blk[3] = byteswap32(d);
#else
	in_blk[0] = a;
	in_blk[1] = b;
	in_blk[2] = c;
	in_blk[3] = d;
#endif
}

/* decrypt a block of text  */

WIN32DLL_DEFINE void _mcrypt_decrypt(SERPENT_KEY * spkey, word32 * in_blk)
{
	word32 a, b, c, d, e, f, g, h;
	word32 t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14,
	    t15, t16;

#ifdef WORDS_BIGENDIAN
	a = byteswap32(in_blk[0]);
	b = byteswap32(in_blk[1]);
	c = byteswap32(in_blk[2]);
	d = byteswap32(in_blk[3]);
#else
	a = in_blk[0];
	b = in_blk[1];
	c = in_blk[2];
	d = in_blk[3];
#endif

	k_xor(32, a, b, c, d);
	ib7(a, b, c, d, e, f, g, h);
	k_xor(31, e, f, g, h);
	irot(e, f, g, h);
	ib6(e, f, g, h, a, b, c, d);
	k_xor(30, a, b, c, d);
	irot(a, b, c, d);
	ib5(a, b, c, d, e, f, g, h);
	k_xor(29, e, f, g, h);
	irot(e, f, g, h);
	ib4(e, f, g, h, a, b, c, d);
	k_xor(28, a, b, c, d);
	irot(a, b, c, d);
	ib3(a, b, c, d, e, f, g, h);
	k_xor(27, e, f, g, h);
	irot(e, f, g, h);
	ib2(e, f, g, h, a, b, c, d);
	k_xor(26, a, b, c, d);
	irot(a, b, c, d);
	ib1(a, b, c, d, e, f, g, h);
	k_xor(25, e, f, g, h);
	irot(e, f, g, h);
	ib0(e, f, g, h, a, b, c, d);
	k_xor(24, a, b, c, d);
	irot(a, b, c, d);
	ib7(a, b, c, d, e, f, g, h);
	k_xor(23, e, f, g, h);
	irot(e, f, g, h);
	ib6(e, f, g, h, a, b, c, d);
	k_xor(22, a, b, c, d);
	irot(a, b, c, d);
	ib5(a, b, c, d, e, f, g, h);
	k_xor(21, e, f, g, h);
	irot(e, f, g, h);
	ib4(e, f, g, h, a, b, c, d);
	k_xor(20, a, b, c, d);
	irot(a, b, c, d);
	ib3(a, b, c, d, e, f, g, h);
	k_xor(19, e, f, g, h);
	irot(e, f, g, h);
	ib2(e, f, g, h, a, b, c, d);
	k_xor(18, a, b, c, d);
	irot(a, b, c, d);
	ib1(a, b, c, d, e, f, g, h);
	k_xor(17, e, f, g, h);
	irot(e, f, g, h);
	ib0(e, f, g, h, a, b, c, d);
	k_xor(16, a, b, c, d);
	irot(a, b, c, d);
	ib7(a, b, c, d, e, f, g, h);
	k_xor(15, e, f, g, h);
	irot(e, f, g, h);
	ib6(e, f, g, h, a, b, c, d);
	k_xor(14, a, b, c, d);
	irot(a, b, c, d);
	ib5(a, b, c, d, e, f, g, h);
	k_xor(13, e, f, g, h);
	irot(e, f, g, h);
	ib4(e, f, g, h, a, b, c, d);
	k_xor(12, a, b, c, d);
	irot(a, b, c, d);
	ib3(a, b, c, d, e, f, g, h);
	k_xor(11, e, f, g, h);
	irot(e, f, g, h);
	ib2(e, f, g, h, a, b, c, d);
	k_xor(10, a, b, c, d);
	irot(a, b, c, d);
	ib1(a, b, c, d, e, f, g, h);
	k_xor(9, e, f, g, h);
	irot(e, f, g, h);
	ib0(e, f, g, h, a, b, c, d);
	k_xor(8, a, b, c, d);
	irot(a, b, c, d);
	ib7(a, b, c, d, e, f, g, h);
	k_xor(7, e, f, g, h);
	irot(e, f, g, h);
	ib6(e, f, g, h, a, b, c, d);
	k_xor(6, a, b, c, d);
	irot(a, b, c, d);
	ib5(a, b, c, d, e, f, g, h);
	k_xor(5, e, f, g, h);
	irot(e, f, g, h);
	ib4(e, f, g, h, a, b, c, d);
	k_xor(4, a, b, c, d);
	irot(a, b, c, d);
	ib3(a, b, c, d, e, f, g, h);
	k_xor(3, e, f, g, h);
	irot(e, f, g, h);
	ib2(e, f, g, h, a, b, c, d);
	k_xor(2, a, b, c, d);
	irot(a, b, c, d);
	ib1(a, b, c, d, e, f, g, h);
	k_xor(1, e, f, g, h);
	irot(e, f, g, h);
	ib0(e, f, g, h, a, b, c, d);
	k_xor(0, a, b, c, d);

#ifdef WORDS_BIGENDIAN
	in_blk[0] = byteswap32(a);
	in_blk[1] = byteswap32(b);
	in_blk[2] = byteswap32(c);
	in_blk[3] = byteswap32(d);
#else
	in_blk[0] = a;
	in_blk[1] = b;
	in_blk[2] = c;
	in_blk[3] = d;
#endif
}


WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(SERPENT_KEY);
}

WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 16;
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 32;
}

static const int key_sizes[] = { 16, 24, 32 };
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = sizeof(key_sizes)/sizeof(int);
	return key_sizes;

}
WIN32DLL_DEFINE const char *_mcrypt_get_algorithms_name()
{
return "Serpent";
}

#define CIPHER "9a99455df5080bfccadf049b5aaf7d61"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	char *keyword;
	unsigned char plaintext[16];
	unsigned char ciphertext[16];
	int blocksize = _mcrypt_get_block_size(), j;
	void *key;
	unsigned char cipher_tmp[200];

	keyword = calloc(1, _mcrypt_get_key_size());
	if (keyword == NULL)
		return -1;

	for (j = 0; j < _mcrypt_get_key_size(); j++) {
		keyword[j] = ((j * 2 + 10) % 256);
	}


	for (j = 0; j < blocksize; j++) {
		plaintext[j] = j % 256;
	}
	key = malloc(_mcrypt_get_size());
	if (key == NULL)
		return -1;

	memcpy(ciphertext, plaintext, blocksize);

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size());
	free(keyword);

	_mcrypt_encrypt(key, (void *) ciphertext);

	for (j = 0; j < blocksize; j++) {
		sprintf(&((char *) cipher_tmp)[2 * j], "%.2x",
			ciphertext[j]);
	}

	if (strcmp((char *) cipher_tmp, CIPHER) != 0) {
		printf("failed compatibility\n");
		printf("Expected: %s\nGot: %s\n", CIPHER,
		       (char *) cipher_tmp);
		free(key);
		return -1;
	}
	_mcrypt_decrypt(key, (void *) ciphertext);
	free(key);

	if (strcmp(ciphertext, plaintext) != 0) {
		printf("failed internally\n");
		return -1;
	}

	return 0;
}

WIN32DLL_DEFINE word32 _mcrypt_algorithm_version()
{
	return 20010801;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
