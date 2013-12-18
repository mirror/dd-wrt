
/* This is an independent implementation of the encryption algorithm:   */
/*                                                                      */
/*         Twofish by Bruce Schneier and colleagues                     */
/*                                                                      */
/* which is a candidate algorithm in the Advanced Encryption Standard   */
/* programme of the US National Institute of Standards and Technology.  */
/*                                                                      */
/* Copyright in this implementation is held by Dr B R Gladman but I     */
/* hereby give permission for its free direct or derivative use subject */
/* to acknowledgment of its origin and compliance with any conditions   */
/* that the originators of t he algorithm place on its exploitation.    */
/*                                                                      */
/* My thanks to Doug Whiting and Niels Ferguson for comments that led   */
/* to improvements in this implementation.                              */
/*                                                                      */
/* Dr Brian Gladman (gladman@seven77.demon.co.uk) 14th January 1999     */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/* $Id: twofish.c,v 1.16 2003/01/19 17:48:27 nmav Exp $ */

/* Timing data for Twofish (twofish.c)

   128 bit key:
   Key Setup:    8414 cycles
   Encrypt:       376 cycles =    68.1 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          375 cycles =    68.3 mbits/sec

   192 bit key:
   Key Setup:   11628 cycles
   Encrypt:       376 cycles =    68.1 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          375 cycles =    68.3 mbits/sec

   256 bit key:
   Key Setup:   15457 cycles
   Encrypt:       381 cycles =    67.2 mbits/sec
   Decrypt:       374 cycles =    68.4 mbits/sec
   Mean:          378 cycles =    67.8 mbits/sec

 */

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "twofish.h"

#define _mcrypt_set_key twofish_LTX__mcrypt_set_key
#define _mcrypt_encrypt twofish_LTX__mcrypt_encrypt
#define _mcrypt_decrypt twofish_LTX__mcrypt_decrypt
#define _mcrypt_get_size twofish_LTX__mcrypt_get_size
#define _mcrypt_get_block_size twofish_LTX__mcrypt_get_block_size
#define _is_block_algorithm twofish_LTX__is_block_algorithm
#define _mcrypt_get_key_size twofish_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes twofish_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name twofish_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test twofish_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version twofish_LTX__mcrypt_algorithm_version

/* word32  k_len;
 * word32  l_key[40];
 * word32  s_key[4];
 */

/* Extract byte from a 32 bit quantity (little endian notation)     */
#define byte(x,n)   ((byte)((x) >> (8 * n)))

/* finite field arithmetic for GF(2**8) with the modular    */
/* polynomial x^8 + x^6 + x^5 + x^3 + 1 (0x169)             */

#define G_M 0x0169

byte tab_5b[4] = { 0, G_M >> 2, G_M >> 1, (G_M >> 1) ^ (G_M >> 2) };
byte tab_ef[4] = { 0, (G_M >> 1) ^ (G_M >> 2), G_M >> 1, G_M >> 2 };

#define ffm_01(x)    (x)
#define ffm_5b(x)   ((x) ^ ((x) >> 2) ^ tab_5b[(x) & 3])
#define ffm_ef(x)   ((x) ^ ((x) >> 1) ^ ((x) >> 2) ^ tab_ef[(x) & 3])

byte ror4[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
byte ashx[16] = { 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12, 5, 14, 7 };

byte qt0[2][16] = {
	{8, 1, 7, 13, 6, 15, 3, 2, 0, 11, 5, 9, 14, 12, 10, 4}
	,
	{2, 8, 11, 13, 15, 7, 6, 14, 3, 1, 9, 4, 0, 10, 12, 5}
};

byte qt1[2][16] = {
	{14, 12, 11, 8, 1, 2, 3, 5, 15, 4, 10, 6, 7, 0, 9, 13}
	,
	{1, 14, 2, 11, 4, 12, 3, 7, 6, 13, 10, 5, 15, 9, 0, 8}
};

byte qt2[2][16] = {
	{11, 10, 5, 14, 6, 13, 9, 0, 12, 8, 15, 3, 2, 4, 7, 1}
	,
	{4, 12, 7, 5, 1, 6, 9, 10, 0, 14, 13, 8, 2, 11, 3, 15}
};

byte qt3[2][16] = {
	{13, 7, 15, 4, 1, 2, 6, 14, 9, 11, 3, 0, 8, 5, 12, 10}
	,
	{11, 9, 5, 1, 12, 3, 13, 14, 6, 4, 7, 15, 2, 0, 8, 10}
};

byte qp(const word32 n, const byte x)
{
	byte a0, a1, a2, a3, a4, b0, b1, b2, b3, b4;

	a0 = x >> 4;
	b0 = x & 15;
	a1 = a0 ^ b0;
	b1 = ror4[b0] ^ ashx[a0];
	a2 = qt0[n][a1];
	b2 = qt1[n][b1];
	a3 = a2 ^ b2;
	b3 = ror4[b2] ^ ashx[a2];
	a4 = qt2[n][a3];
	b4 = qt3[n][b3];
	return (b4 << 4) | a4;
}

#ifdef  Q_TABLES

#define q(n,x)  pkey->q_tab[n][x]

void gen_qtab(TWI * pkey)
{
	word32 i;

	for (i = 0; i < 256; ++i) {
		q(0, i) = qp(0, (byte) i);
		q(1, i) = qp(1, (byte) i);
	}
}

#else

#define q(n,x)  qp(n, x)

#endif

#ifdef  M_TABLE

void gen_mtab(TWI * pkey)
{
	word32 i, f01, f5b, fef;

	for (i = 0; i < 256; ++i) {
		f01 = q(1, i);
		f5b = ffm_5b(f01);
		fef = ffm_ef(f01);
		pkey->m_tab[0][i] =
		    f01 + (f5b << 8) + (fef << 16) + (fef << 24);
		pkey->m_tab[2][i] =
		    f5b + (fef << 8) + (f01 << 16) + (fef << 24);

		f01 = q(0, i);
		f5b = ffm_5b(f01);
		fef = ffm_ef(f01);
		pkey->m_tab[1][i] =
		    fef + (fef << 8) + (f5b << 16) + (f01 << 24);
		pkey->m_tab[3][i] =
		    f5b + (f01 << 8) + (fef << 16) + (f5b << 24);
	}
}

#define mds(n,x)    pkey->m_tab[n][x]

#else

#define fm_00   ffm_01
#define fm_10   ffm_5b
#define fm_20   ffm_ef
#define fm_30   ffm_ef
#define q_0(x)  q(1,x)

#define fm_01   ffm_ef
#define fm_11   ffm_ef
#define fm_21   ffm_5b
#define fm_31   ffm_01
#define q_1(x)  q(0,x)

#define fm_02   ffm_5b
#define fm_12   ffm_ef
#define fm_22   ffm_01
#define fm_32   ffm_ef
#define q_2(x)  q(1,x)

#define fm_03   ffm_5b
#define fm_13   ffm_01
#define fm_23   ffm_ef
#define fm_33   ffm_5b
#define q_3(x)  q(0,x)

#define f_0(n,x)    ((word32)fm_0##n(x))
#define f_1(n,x)    ((word32)fm_1##n(x) << 8)
#define f_2(n,x)    ((word32)fm_2##n(x) << 16)
#define f_3(n,x)    ((word32)fm_3##n(x) << 24)

#define mds(n,x)    f_0(n,q_##n(x)) ^ f_1(n,q_##n(x)) ^ f_2(n,q_##n(x)) ^ f_3(n,q_##n(x))

#endif

word32 h_fun(TWI * pkey, const word32 x, const word32 key[])
{
	word32 b0, b1, b2, b3;

#ifndef M_TABLE
	word32 m5b_b0, m5b_b1, m5b_b2, m5b_b3;
	word32 mef_b0, mef_b1, mef_b2, mef_b3;
#endif

	b0 = byte(x, 0);
	b1 = byte(x, 1);
	b2 = byte(x, 2);
	b3 = byte(x, 3);

	switch (pkey->k_len) {
	case 4:
		b0 = q(1, b0) ^ byte(key[3], 0);
		b1 = q(0, b1) ^ byte(key[3], 1);
		b2 = q(0, b2) ^ byte(key[3], 2);
		b3 = q(1, b3) ^ byte(key[3], 3);
	case 3:
		b0 = q(1, b0) ^ byte(key[2], 0);
		b1 = q(1, b1) ^ byte(key[2], 1);
		b2 = q(0, b2) ^ byte(key[2], 2);
		b3 = q(0, b3) ^ byte(key[2], 3);
	case 2:
		b0 = q(0, q(0, b0) ^ byte(key[1], 0)) ^ byte(key[0], 0);
		b1 = q(0, q(1, b1) ^ byte(key[1], 1)) ^ byte(key[0], 1);
		b2 = q(1, q(0, b2) ^ byte(key[1], 2)) ^ byte(key[0], 2);
		b3 = q(1, q(1, b3) ^ byte(key[1], 3)) ^ byte(key[0], 3);
	}
#ifdef  M_TABLE

	return mds(0, b0) ^ mds(1, b1) ^ mds(2, b2) ^ mds(3, b3);

#else

	b0 = q(1, b0);
	b1 = q(0, b1);
	b2 = q(1, b2);
	b3 = q(0, b3);
	m5b_b0 = ffm_5b(b0);
	m5b_b1 = ffm_5b(b1);
	m5b_b2 = ffm_5b(b2);
	m5b_b3 = ffm_5b(b3);
	mef_b0 = ffm_ef(b0);
	mef_b1 = ffm_ef(b1);
	mef_b2 = ffm_ef(b2);
	mef_b3 = ffm_ef(b3);
	b0 ^= mef_b1 ^ m5b_b2 ^ m5b_b3;
	b3 ^= m5b_b0 ^ mef_b1 ^ mef_b2;
	b2 ^= mef_b0 ^ m5b_b1 ^ mef_b3;
	b1 ^= mef_b0 ^ mef_b2 ^ m5b_b3;

	return b0 | (b3 << 8) | (b2 << 16) | (b1 << 24);

#endif
}

#ifdef  MK_TABLE

#define q20(x)  q(0,q(0,x) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q21(x)  q(0,q(1,x) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q22(x)  q(1,q(0,x) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q23(x)  q(1,q(1,x) ^ byte(key[1],3)) ^ byte(key[0],3)

#define q30(x)  q(0,q(0,q(1, x) ^ byte(key[2],0)) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q31(x)  q(0,q(1,q(1, x) ^ byte(key[2],1)) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q32(x)  q(1,q(0,q(0, x) ^ byte(key[2],2)) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q33(x)  q(1,q(1,q(0, x) ^ byte(key[2],3)) ^ byte(key[1],3)) ^ byte(key[0],3)

#define q40(x)  q(0,q(0,q(1, q(1, x) ^ byte(key[3],0)) ^ byte(key[2],0)) ^ byte(key[1],0)) ^ byte(key[0],0)
#define q41(x)  q(0,q(1,q(1, q(0, x) ^ byte(key[3],1)) ^ byte(key[2],1)) ^ byte(key[1],1)) ^ byte(key[0],1)
#define q42(x)  q(1,q(0,q(0, q(0, x) ^ byte(key[3],2)) ^ byte(key[2],2)) ^ byte(key[1],2)) ^ byte(key[0],2)
#define q43(x)  q(1,q(1,q(0, q(1, x) ^ byte(key[3],3)) ^ byte(key[2],3)) ^ byte(key[1],3)) ^ byte(key[0],3)

void gen_mk_tab(TWI * pkey, word32 key[])
{
	word32 i;
	byte by;

	switch (pkey->k_len) {
	case 2:
		for (i = 0; i < 256; ++i) {
			by = (byte) i;
#ifdef ONE_STEP
			pkey->mk_tab[0][i] = mds(0, q20(by));
			pkey->mk_tab[1][i] = mds(1, q21(by));
			pkey->mk_tab[2][i] = mds(2, q22(by));
			pkey->mk_tab[3][i] = mds(3, q23(by));
#else
			pkey->sb[0][i] = q20(by);
			pkey->sb[1][i] = q21(by);
			pkey->sb[2][i] = q22(by);
			pkey->sb[3][i] = q23(by);
#endif
		}
		break;

	case 3:
		for (i = 0; i < 256; ++i) {
			by = (byte) i;
#ifdef ONE_STEP
			pkey->mk_tab[0][i] = mds(0, q30(by));
			pkey->mk_tab[1][i] = mds(1, q31(by));
			pkey->mk_tab[2][i] = mds(2, q32(by));
			pkey->mk_tab[3][i] = mds(3, q33(by));
#else
			pkey->sb[0][i] = q30(by);
			pkey->sb[1][i] = q31(by);
			pkey->sb[2][i] = q32(by);
			pkey->sb[3][i] = q33(by);
#endif
		}
		break;

	case 4:
		for (i = 0; i < 256; ++i) {
			by = (byte) i;
#ifdef ONE_STEP
			pkey->mk_tab[0][i] = mds(0, q40(by));
			pkey->mk_tab[1][i] = mds(1, q41(by));
			pkey->mk_tab[2][i] = mds(2, q42(by));
			pkey->mk_tab[3][i] = mds(3, q43(by));
#else
			pkey->sb[0][i] = q40(by);
			pkey->sb[1][i] = q41(by);
			pkey->sb[2][i] = q42(by);
			pkey->sb[3][i] = q43(by);
#endif
		}
	}
}

#ifdef ONE_STEP
#define g0_fun(x) ( pkey->mk_tab[0][byte(x,0)] ^ pkey->mk_tab[1][byte(x,1)] \
                      ^ pkey->mk_tab[2][byte(x,2)] ^ pkey->mk_tab[3][byte(x,3)] )
#define g1_fun(x) ( pkey->mk_tab[0][byte(x,3)] ^ pkey->mk_tab[1][byte(x,0)] \
                      ^ pkey->mk_tab[2][byte(x,1)] ^ pkey->mk_tab[3][byte(x,2)] )
#else
#define g0_fun(x) ( mds(0, pkey->sb[0][byte(x,0)]) ^ mds(1, pkey->sb[1][byte(x,1)]) \
                      ^ mds(2, pkey->sb[2][byte(x,2)]) ^ mds(3, pkey->sb[3][byte(x,3)]) )
#define g1_fun(x) ( mds(0, pkey->sb[0][byte(x,3)]) ^ mds(1, pkey->sb[1][byte(x,0)]) \
                      ^ mds(2, pkey->sb[2][byte(x,1)]) ^ mds(3, pkey->sb[3][byte(x,2)]) )
#endif

#else

#define g0_fun(x)   h_fun(pkey, x,pkey->s_key)
#define g1_fun(x)   h_fun(pkey, rotl32(x,8),pkey->s_key)

#endif

/* The (12,8) Reed Soloman code has the generator polynomial

   g(x) = x^4 + (a + 1/a) * x^3 + a * x^2 + (a + 1/a) * x + 1

   where the coefficients are in the finite field GF(2^8) with a
   modular polynomial a^8 + a^6 + a^3 + a^2 + 1. To generate the
   remainder we have to start with a 12th order polynomial with our
   eight input bytes as the coefficients of the 4th to 11th terms. 
   That is:

   m[7] * x^11 + m[6] * x^10 ... + m[0] * x^4 + 0 * x^3 +... + 0

   We then multiply the generator polynomial by m[7] * x^7 and subtract
   it - xor in GF(2^8) - from the above to eliminate the x^7 term (the 
   artihmetic on the coefficients is done in GF(2^8). We then multiply 
   the generator polynomial by x^6 * coeff(x^10) and use this to remove
   the x^10 term. We carry on in this way until the x^4 term is removed
   so that we are left with:

   r[3] * x^3 + r[2] * x^2 + r[1] 8 x^1 + r[0]

   which give the resulting 4 bytes of the remainder. This is equivalent 
   to the matrix multiplication in the Twofish description but much faster 
   to implement.

 */

#define G_MOD   0x0000014d

word32 mds_rem(word32 p0, word32 p1)
{
	word32 i, t, u;

	for (i = 0; i < 8; ++i) {
		t = p1 >> 24;	/* get most significant coefficient */

		p1 = (p1 << 8) | (p0 >> 24);
		p0 <<= 8;	/* shift others up */

		/* multiply t by a (the primitive element - i.e. left shift) */

		u = (t << 1);

		if (t & 0x80)
			/* subtract modular polynomial on overflow */
			u ^= G_MOD;

		p1 ^= t ^ (u << 16);	/* remove t * (a * x^2 + 1)  */

		u ^= (t >> 1);	/* form u = a * t + t / a = t * (a + 1 / a); */

		if (t & 0x01)
			/* add the modular polynomial on underflow */
			u ^= G_MOD >> 1;

		p1 ^= (u << 24) | (u << 8);	/* remove t * (a + 1/a) * (x^3 + x) */

	}

	return p1;
}

/* initialise the key schedule from the user supplied key   */
WIN32DLL_DEFINE
    int _mcrypt_set_key(TWI * pkey, const word32 in_key[],
			const word32 key_len)
{
	word32 i, a, b, me_key[4], mo_key[4];

#ifdef Q_TABLES
	pkey->qt_gen = 0;

	if (!pkey->qt_gen) {
		gen_qtab(pkey);
		pkey->qt_gen = 1;
	}
#endif

#ifdef M_TABLE
	pkey->mt_gen = 0;
	if (!pkey->mt_gen) {
		gen_mtab(pkey);
		pkey->mt_gen = 1;
	}
#endif

	pkey->k_len = (key_len * 8) / 64;	/* 2, 3 or 4 */

	for (i = 0; i < pkey->k_len; ++i) {
#ifdef WORDS_BIGENDIAN
		a = byteswap32(in_key[i + i]);
		me_key[i] = a;
		b = byteswap32(in_key[i + i + 1]);
#else
		a = in_key[i + i];
		me_key[i] = a;
		b = in_key[i + i + 1];
#endif
		mo_key[i] = b;
		pkey->s_key[pkey->k_len - i - 1] = mds_rem(a, b);
	}

	for (i = 0; i < 40; i += 2) {
		a = 0x01010101 * i;
		b = a + 0x01010101;
		a = h_fun(pkey, a, me_key);
		b = rotl32(h_fun(pkey, b, mo_key), 8);
		pkey->l_key[i] = a + b;
		pkey->l_key[i + 1] = rotl32(a + 2 * b, 9);
	}

#ifdef MK_TABLE
	gen_mk_tab(pkey, pkey->s_key);
#endif
	return 0;
}

/* This macro was moved to an inline function, because it
 * was breaking some compilers.
 */
inline 
static void f_rnd(int i, word32* blk, TWI* pkey, word32 t0, word32 t1)
{
    t1 = g1_fun(blk[1]); 
    t0 = g0_fun(blk[0]);
    
    blk[2] = rotr32(blk[2] ^ (t0 + t1 + pkey->l_key[4 * (i) + 8]), 1);

    blk[3] = rotl32(blk[3], 1) ^ (t0 + 2 * t1 + pkey->l_key[4 * (i) + 9]);
    t1 = g1_fun(blk[3]); 
    t0 = g0_fun(blk[2]);
    
    blk[0] = rotr32(blk[0] ^ (t0 + t1 + pkey->l_key[4 * (i) + 10]), 1);
    blk[1] = rotl32(blk[1], 1) ^ (t0 + 2 * t1 + pkey->l_key[4 * (i) + 11]);
}

/* encrypt a block of text  */
WIN32DLL_DEFINE void _mcrypt_encrypt(TWI * pkey, word32 * in_blk)
{
	word32 t0, t1, blk[4];
#ifdef WORDS_BIGENDIAN
	blk[0] = byteswap32(in_blk[0]) ^ pkey->l_key[0];
	blk[1] = byteswap32(in_blk[1]) ^ pkey->l_key[1];
	blk[2] = byteswap32(in_blk[2]) ^ pkey->l_key[2];
	blk[3] = byteswap32(in_blk[3]) ^ pkey->l_key[3];
#else
	blk[0] = in_blk[0] ^ pkey->l_key[0];
	blk[1] = in_blk[1] ^ pkey->l_key[1];
	blk[2] = in_blk[2] ^ pkey->l_key[2];
	blk[3] = in_blk[3] ^ pkey->l_key[3];
#endif

	f_rnd(0, blk, pkey, t0, t1);
	f_rnd(1, blk, pkey, t0, t1);
	f_rnd(2, blk, pkey, t0, t1);
	f_rnd(3, blk, pkey, t0, t1);
	f_rnd(4, blk, pkey, t0, t1);
	f_rnd(5, blk, pkey, t0, t1);
	f_rnd(6, blk, pkey, t0, t1);
	f_rnd(7, blk, pkey, t0, t1);

#ifdef WORDS_BIGENDIAN
	in_blk[0] = byteswap32(blk[2] ^ pkey->l_key[4]);
	in_blk[1] = byteswap32(blk[3] ^ pkey->l_key[5]);
	in_blk[2] = byteswap32(blk[0] ^ pkey->l_key[6]);
	in_blk[3] = byteswap32(blk[1] ^ pkey->l_key[7]);
#else
	in_blk[0] = blk[2] ^ pkey->l_key[4];
	in_blk[1] = blk[3] ^ pkey->l_key[5];
	in_blk[2] = blk[0] ^ pkey->l_key[6];
	in_blk[3] = blk[1] ^ pkey->l_key[7];
#endif
}

/* decrypt a block of text  */

#define i_rnd(i)                                                        \
        t1 = g1_fun(blk[1]); t0 = g0_fun(blk[0]);                       \
        blk[2] = rotl32(blk[2], 1) ^ (t0 + t1 + pkey->l_key[4 * (i) + 10]);     \
        blk[3] = rotr32(blk[3] ^ (t0 + 2 * t1 + pkey->l_key[4 * (i) + 11]), 1); \
        t1 = g1_fun(blk[3]); t0 = g0_fun(blk[2]);                       \
        blk[0] = rotl32(blk[0], 1) ^ (t0 + t1 + pkey->l_key[4 * (i) +  8]);     \
        blk[1] = rotr32(blk[1] ^ (t0 + 2 * t1 + pkey->l_key[4 * (i) +  9]), 1)

WIN32DLL_DEFINE void _mcrypt_decrypt(TWI * pkey, word32 * in_blk)
{
	word32 t0, t1, blk[4];

#ifdef WORDS_BIGENDIAN
	blk[0] = byteswap32(in_blk[0]) ^ pkey->l_key[4];
	blk[1] = byteswap32(in_blk[1]) ^ pkey->l_key[5];
	blk[2] = byteswap32(in_blk[2]) ^ pkey->l_key[6];
	blk[3] = byteswap32(in_blk[3]) ^ pkey->l_key[7];
#else
	blk[0] = in_blk[0] ^ pkey->l_key[4];
	blk[1] = in_blk[1] ^ pkey->l_key[5];
	blk[2] = in_blk[2] ^ pkey->l_key[6];
	blk[3] = in_blk[3] ^ pkey->l_key[7];
#endif

	i_rnd(7);
	i_rnd(6);
	i_rnd(5);
	i_rnd(4);
	i_rnd(3);
	i_rnd(2);
	i_rnd(1);
	i_rnd(0);

#ifdef WORDS_BIGENDIAN
	in_blk[0] = byteswap32(blk[2] ^ pkey->l_key[0]);
	in_blk[1] = byteswap32(blk[3] ^ pkey->l_key[1]);
	in_blk[2] = byteswap32(blk[0] ^ pkey->l_key[2]);
	in_blk[3] = byteswap32(blk[1] ^ pkey->l_key[3]);
#else
	in_blk[0] = blk[2] ^ pkey->l_key[0];
	in_blk[1] = blk[3] ^ pkey->l_key[1];
	in_blk[2] = blk[0] ^ pkey->l_key[2];
	in_blk[3] = blk[1] ^ pkey->l_key[3];
#endif

}

WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(TWI);
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
return "Twofish";
}

#define CIPHER "019f9809de1711858faac3a3ba20fbc3"
#define PT "\xD4\x91\xDB\x16\xE7\xB1\xC3\x9E\x86\xCB\x08\x6B\x78\x9F\x54\x19"
#define KEY "\x9F\x58\x9F\x5C\xF6\x12\x2C\x32\xB6\xBF\xEC\x2F\x2A\xE8\xC3\x5A"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	unsigned char keyword[16];
	unsigned char plaintext[16];
	unsigned char ciphertext[16];
	int blocksize = _mcrypt_get_block_size(), j;
	void* key;
	unsigned char cipher_tmp[200];

	memcpy( keyword, KEY, 16);
	memcpy( plaintext, PT, 16);

	memcpy(ciphertext, plaintext, 16);
	
	key = malloc(_mcrypt_get_size());
	if (key==NULL) return -1;
	
	_mcrypt_set_key(key, (void *) keyword, 16);

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

	if (memcmp(ciphertext, plaintext, 16) != 0) {
		printf("failed internally\n");
		return -1;
	}

	return 0;
}

WIN32DLL_DEFINE word32 _mcrypt_algorithm_version()
{
	return 19991129;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
