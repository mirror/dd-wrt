/* Rijndael Cipher

   Written by Mike Scott 21st April 1999
   Copyright (c) 1999 Mike Scott
   See rijndael documentation

   Permission for free direct or derivative use is granted subject 
   to compliance with any conditions that the originators of the 
   algorithm place on its exploitation.  

   Inspiration from Brian Gladman's implementation is acknowledged.

   Written for clarity, rather than speed.
   Full implementation. 
   Endian indifferent.
*/

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */


/* $Id: rijndael-192.c,v 1.15 2003/01/19 17:48:27 nmav Exp $ */

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "rijndael.h"

#define _mcrypt_set_key rijndael_192_LTX__mcrypt_set_key
#define _mcrypt_encrypt rijndael_192_LTX__mcrypt_encrypt
#define _mcrypt_decrypt rijndael_192_LTX__mcrypt_decrypt
#define _mcrypt_get_size rijndael_192_LTX__mcrypt_get_size
#define _mcrypt_get_block_size rijndael_192_LTX__mcrypt_get_block_size
#define _is_block_algorithm rijndael_192_LTX__is_block_algorithm
#define _mcrypt_get_key_size rijndael_192_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes rijndael_192_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name rijndael_192_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test rijndael_192_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version rijndael_192_LTX__mcrypt_algorithm_version

/* rotates x one bit to the left */

#define ROTL(x) (((x)>>7)|((x)<<1))

/* Rotates 32-bit word left by 1, 2 or 3 byte  */

#define ROTL8(x) (((x)<<8)|((x)>>24))
#define ROTL16(x) (((x)<<16)|((x)>>16))
#define ROTL24(x) (((x)<<24)|((x)>>8))

/* Fixed Data */

static byte InCo[4] = { 0xB, 0xD, 0x9, 0xE };	/* Inverse Coefficients */

static byte fbsub[256];
static byte rbsub[256];
static byte ptab[256], ltab[256];
static word32 ftable[256];
static word32 rtable[256];
static word32 rco[30];
static int tables_ok = 0;

/* Parameter-dependent data */

/* in "rijndael.h" */

static word32 pack(byte * b)
{				/* pack bytes into a 32-bit Word */
	return ((word32) b[3] << 24) | ((word32) b[2] << 16) | ((word32)
								b[1] << 8)
	    | (word32) b[0];
}

static void unpack(word32 a, byte * b)
{				/* unpack bytes from a word */
	b[0] = (byte) a;
	b[1] = (byte) (a >> 8);
	b[2] = (byte) (a >> 16);
	b[3] = (byte) (a >> 24);
}

static byte xtime(byte a)
{
	byte b;
	if (a & 0x80)
		b = 0x1B;
	else
		b = 0;
	a <<= 1;
	a ^= b;
	return a;
}

static byte bmul(byte x, byte y)
{				/* x.y= AntiLog(Log(x) + Log(y)) */
	if (x && y)
		return ptab[(ltab[x] + ltab[y]) % 255];
	else
		return 0;
}

static word32 SubByte(word32 a)
{
	byte b[4];
	unpack(a, b);
	b[0] = fbsub[b[0]];
	b[1] = fbsub[b[1]];
	b[2] = fbsub[b[2]];
	b[3] = fbsub[b[3]];
	return pack(b);
}

static byte product(word32 x, word32 y)
{				/* dot product of two 4-byte arrays */
	byte xb[4], yb[4];
	unpack(x, xb);
	unpack(y, yb);
	return bmul(xb[0], yb[0]) ^ bmul(xb[1], yb[1]) ^ bmul(xb[2],
							      yb[2]) ^
	    bmul(xb[3], yb[3]);
}

static word32 InvMixCol(word32 x)
{				/* matrix Multiplication */
	word32 y, m;
	byte b[4];

	m = pack(InCo);
	b[3] = product(m, x);
	m = ROTL24(m);
	b[2] = product(m, x);
	m = ROTL24(m);
	b[1] = product(m, x);
	m = ROTL24(m);
	b[0] = product(m, x);
	y = pack(b);
	return y;
}

static byte ByteSub(byte x)
{
	byte y = ptab[255 - ltab[x]];	/* multiplicative inverse */
	x = y;
	x = ROTL(x);
	y ^= x;
	x = ROTL(x);
	y ^= x;
	x = ROTL(x);
	y ^= x;
	x = ROTL(x);
	y ^= x;
	y ^= 0x63;
	return y;
}

static void _mcrypt_rijndael_gentables(void)
{				/* generate tables */
	int i;
	byte y, b[4];

	/* use 3 as primitive root to generate power and log tables */

	ltab[0] = 0;
	ptab[0] = 1;
	ltab[1] = 0;
	ptab[1] = 3;
	ltab[3] = 1;
	for (i = 2; i < 256; i++) {
		ptab[i] = ptab[i - 1] ^ xtime(ptab[i - 1]);
		ltab[ptab[i]] = i;
	}

	/* affine transformation:- each bit is xored with itself shifted one bit */

	fbsub[0] = 0x63;
	rbsub[0x63] = 0;
	for (i = 1; i < 256; i++) {
		y = ByteSub((byte) i);
		fbsub[i] = y;
		rbsub[y] = i;
	}

	for (i = 0, y = 1; i < 30; i++) {
		rco[i] = y;
		y = xtime(y);
	}

	/* calculate forward and reverse tables */
	for (i = 0; i < 256; i++) {
		y = fbsub[i];
		b[3] = y ^ xtime(y);
		b[2] = y;
		b[1] = y;
		b[0] = xtime(y);
		ftable[i] = pack(b);

		y = rbsub[i];
		b[3] = bmul(InCo[0], y);
		b[2] = bmul(InCo[1], y);
		b[1] = bmul(InCo[2], y);
		b[0] = bmul(InCo[3], y);
		rtable[i] = pack(b);
	}
}

WIN32DLL_DEFINE int _mcrypt_set_key(RI * rinst, byte * key, int nk)
{				/* blocksize=32*nb bits. Key=32*nk bits */
	/* currently nb,bk = 4, 6 or 8          */
	/* key comes as 4*rinst->Nk bytes              */
	/* Key Scheduler. Create expanded encryption key */
	int nb = 6;		/* 192 block size */
	int i, j, k, m, N;
	int C1, C2, C3;
	word32 CipherKey[8];

	nk /= 4;

	if (tables_ok == 0) {
		_mcrypt_rijndael_gentables();
		tables_ok = 1;
	}

	rinst->Nb = nb;
	rinst->Nk = nk;

	/* rinst->Nr is number of rounds */
	if (rinst->Nb >= rinst->Nk)
		rinst->Nr = 6 + rinst->Nb;
	else
		rinst->Nr = 6 + rinst->Nk;

	C1 = 1;
	if (rinst->Nb < 8) {
		C2 = 2;
		C3 = 3;
	} else {
		C2 = 3;
		C3 = 4;
	}

	/* pre-calculate forward and reverse increments */
	for (m = j = 0; j < nb; j++, m += 3) {
		rinst->fi[m] = (j + C1) % nb;
		rinst->fi[m + 1] = (j + C2) % nb;
		rinst->fi[m + 2] = (j + C3) % nb;
		rinst->ri[m] = (nb + j - C1) % nb;
		rinst->ri[m + 1] = (nb + j - C2) % nb;
		rinst->ri[m + 2] = (nb + j - C3) % nb;
	}

	N = rinst->Nb * (rinst->Nr + 1);

	for (i = j = 0; i < rinst->Nk; i++, j += 4) {
		CipherKey[i] = pack(&key[j]);
	}
	for (i = 0; i < rinst->Nk; i++)
		rinst->fkey[i] = CipherKey[i];
	for (j = rinst->Nk, k = 0; j < N; j += rinst->Nk, k++) {
		rinst->fkey[j] =
		    rinst->fkey[j -
				rinst->Nk] ^ SubByte(ROTL24(rinst->
							    fkey[j -
								 1])) ^
		    rco[k];
		if (rinst->Nk <= 6) {
			for (i = 1; i < rinst->Nk && (i + j) < N; i++)
				rinst->fkey[i + j] =
				    rinst->fkey[i + j -
						rinst->Nk] ^ rinst->
				    fkey[i + j - 1];
		} else {
			for (i = 1; i < 4 && (i + j) < N; i++)
				rinst->fkey[i + j] =
				    rinst->fkey[i + j -
						rinst->Nk] ^ rinst->
				    fkey[i + j - 1];
			if ((j + 4) < N)
				rinst->fkey[j + 4] =
				    rinst->fkey[j + 4 -
						rinst->
						Nk] ^ SubByte(rinst->
							      fkey[j + 3]);
			for (i = 5; i < rinst->Nk && (i + j) < N; i++)
				rinst->fkey[i + j] =
				    rinst->fkey[i + j -
						rinst->Nk] ^ rinst->
				    fkey[i + j - 1];
		}

	}

	/* now for the expanded decrypt key in reverse order */

	for (j = 0; j < rinst->Nb; j++)
		rinst->rkey[j + N - rinst->Nb] = rinst->fkey[j];
	for (i = rinst->Nb; i < N - rinst->Nb; i += rinst->Nb) {
		k = N - rinst->Nb - i;
		for (j = 0; j < rinst->Nb; j++)
			rinst->rkey[k + j] = InvMixCol(rinst->fkey[i + j]);
	}
	for (j = N - rinst->Nb; j < N; j++)
		rinst->rkey[j - N + rinst->Nb] = rinst->fkey[j];
	return 0;
}


/* There is an obvious time/space trade-off possible here.     *
 * Instead of just one ftable[], I could have 4, the other     *
 * 3 pre-rotated to save the ROTL8, ROTL16 and ROTL24 overhead */

WIN32DLL_DEFINE void _mcrypt_encrypt(RI * rinst, byte * buff)
{
	int i, j, k, m;
	word32 a[8], b[8], *x, *y, *t;

	for (i = j = 0; i < rinst->Nb; i++, j += 4) {
		a[i] = pack(&buff[j]);
		a[i] ^= rinst->fkey[i];
	}
	k = rinst->Nb;
	x = a;
	y = b;

/* State alternates between a and b */
	for (i = 1; i < rinst->Nr; i++) {	/* rinst->Nr is number of rounds. May be odd. */

/* if rinst->Nb is fixed - unroll this next 
   loop and hard-code in the values of fi[]  */

		for (m = j = 0; j < rinst->Nb; j++, m += 3) {	/* deal with each 32-bit element of the State */
			/* This is the time-critical bit */
			y[j] = rinst->fkey[k++] ^ ftable[(byte) x[j]] ^
			    ROTL8(ftable[(byte) (x[rinst->fi[m]] >> 8)]) ^
			    ROTL16(ftable
				   [(byte) (x[rinst->fi[m + 1]] >> 16)]) ^
			    ROTL24(ftable[x[rinst->fi[m + 2]] >> 24]);
		}
		t = x;
		x = y;
		y = t;		/* swap pointers */
	}

/* Last Round - unroll if possible */
	for (m = j = 0; j < rinst->Nb; j++, m += 3) {
		y[j] = rinst->fkey[k++] ^ (word32) fbsub[(byte) x[j]] ^
		    ROTL8((word32) fbsub[(byte) (x[rinst->fi[m]] >> 8)]) ^
		    ROTL16((word32)
			   fbsub[(byte) (x[rinst->fi[m + 1]] >> 16)]) ^
		    ROTL24((word32) fbsub[x[rinst->fi[m + 2]] >> 24]);
	}
	for (i = j = 0; i < rinst->Nb; i++, j += 4) {
		unpack(y[i], &buff[j]);
		x[i] = y[i] = 0;	/* clean up stack */
	}
	return;
}

WIN32DLL_DEFINE void _mcrypt_decrypt(RI * rinst, byte * buff)
{
	int i, j, k, m;
	word32 a[8], b[8], *x, *y, *t;

	for (i = j = 0; i < rinst->Nb; i++, j += 4) {
		a[i] = pack(&buff[j]);
		a[i] ^= rinst->rkey[i];
	}
	k = rinst->Nb;
	x = a;
	y = b;

/* State alternates between a and b */
	for (i = 1; i < rinst->Nr; i++) {	/* rinst->Nr is number of rounds. May be odd. */

/* if rinst->Nb is fixed - unroll this next 
   loop and hard-code in the values of ri[]  */

		for (m = j = 0; j < rinst->Nb; j++, m += 3) {	/* This is the time-critical bit */
			y[j] = rinst->rkey[k++] ^ rtable[(byte) x[j]] ^
			    ROTL8(rtable[(byte) (x[rinst->ri[m]] >> 8)]) ^
			    ROTL16(rtable
				   [(byte) (x[rinst->ri[m + 1]] >> 16)]) ^
			    ROTL24(rtable[x[rinst->ri[m + 2]] >> 24]);
		}
		t = x;
		x = y;
		y = t;		/* swap pointers */
	}

/* Last Round - unroll if possible */
	for (m = j = 0; j < rinst->Nb; j++, m += 3) {
		y[j] = rinst->rkey[k++] ^ (word32) rbsub[(byte) x[j]] ^
		    ROTL8((word32) rbsub[(byte) (x[rinst->ri[m]] >> 8)]) ^
		    ROTL16((word32)
			   rbsub[(byte) (x[rinst->ri[m + 1]] >> 16)]) ^
		    ROTL24((word32) rbsub[x[rinst->ri[m + 2]] >> 24]);
	}
	for (i = j = 0; i < rinst->Nb; i++, j += 4) {
		unpack(y[i], &buff[j]);
		x[i] = y[i] = 0;	/* clean up stack */
	}
	return;
}


WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(RI);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 24;
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
WIN32DLL_DEFINE char *_mcrypt_get_algorithms_name()
{
return "Rijndael-192";
}

#define CIPHER "380ee49a5de1dbd4b9cc11af60b8c8ff669e367af8948a8a"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	char *keyword;
	unsigned char plaintext[32];
	unsigned char ciphertext[32];
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
	if (key == NULL) {
		free(keyword);
		return -1;
	}

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
