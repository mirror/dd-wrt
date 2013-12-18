/*
 * $Id: cast-128.c,v 1.12 2003/01/19 17:48:27 nmav Exp $
 *
 *	CAST-128 in C
 *	Written by Steve Reid <sreid@sea-to-sky.net>
 *	100% Public Domain - no warranty
 *	Released 1997.10.11
 */

/* Adapted to the pike cryptographic toolkit by Niels Möller */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/* $Id: cast-128.c,v 1.12 2003/01/19 17:48:27 nmav Exp $ */

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "cast-128.h"

#define _mcrypt_set_key cast_128_LTX__mcrypt_set_key
#define _mcrypt_encrypt cast_128_LTX__mcrypt_encrypt
#define _mcrypt_decrypt cast_128_LTX__mcrypt_decrypt
#define _mcrypt_get_size cast_128_LTX__mcrypt_get_size
#define _mcrypt_get_block_size cast_128_LTX__mcrypt_get_block_size
#define _is_block_algorithm cast_128_LTX__is_block_algorithm
#define _mcrypt_get_key_size cast_128_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes cast_128_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name cast_128_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test cast_128_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version cast_128_LTX__mcrypt_algorithm_version

#define u8 byte
#define u32 word32

#include "cast-128_sboxes.h"

/* Macros to access 8-bit bytes out of a 32-bit word */
#define U8a(x) ( (u8) (x>>24) )
#define U8b(x) ( (u8) ((x>>16)&255) )
#define U8c(x) ( (u8) ((x>>8)&255) )
#define U8d(x) ( (u8) ((x)&255) )

/* Circular left shift */
#define ROL(x, n) ( ((x)<<(n)) | ((x)>>(32-(n))) )

/* CAST-128 uses three different round functions */
#define F1(l, r, i) \
	t = ROL(key->xkey[i] + r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] ^ cast_sbox2[U8b(t)]) \
	 - cast_sbox3[U8c(t)]) + cast_sbox4[U8d(t)];
#define F2(l, r, i) \
	t = ROL(key->xkey[i] ^ r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] - cast_sbox2[U8b(t)]) \
	 + cast_sbox3[U8c(t)]) ^ cast_sbox4[U8d(t)];
#define F3(l, r, i) \
	t = ROL(key->xkey[i] - r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] + cast_sbox2[U8b(t)]) \
	 ^ cast_sbox3[U8c(t)]) - cast_sbox4[U8d(t)];


/***** Encryption Function *****/

WIN32DLL_DEFINE void _mcrypt_encrypt(CAST_KEY * key, u8 * block)
{
	u32 t, l, r;

	/* Get inblock into l,r */
	l = ((u32) block[0] << 24) | ((u32) block[1] << 16)
	    | ((u32) block[2] << 8) | (u32) block[3];
	r = ((u32) block[4] << 24) | ((u32) block[5] << 16)
	    | ((u32) block[6] << 8) | (u32) block[7];
	/* Do the work */
	F1(l, r, 0);
	F2(r, l, 1);
	F3(l, r, 2);
	F1(r, l, 3);
	F2(l, r, 4);
	F3(r, l, 5);
	F1(l, r, 6);
	F2(r, l, 7);
	F3(l, r, 8);
	F1(r, l, 9);
	F2(l, r, 10);
	F3(r, l, 11);
	/* Only do full 16 rounds if key length > 80 bits */
	if (key->rounds > 12) {
		F1(l, r, 12);
		F2(r, l, 13);
		F3(l, r, 14);
		F1(r, l, 15);
	}
	/* Put l,r into outblock */
	block[0] = U8a(r);
	block[1] = U8b(r);
	block[2] = U8c(r);
	block[3] = U8d(r);
	block[4] = U8a(l);
	block[5] = U8b(l);
	block[6] = U8c(l);
	block[7] = U8d(l);
	/* Wipe clean */
	t = l = r = 0;
}


/***** Decryption Function *****/

WIN32DLL_DEFINE void _mcrypt_decrypt(CAST_KEY * key, u8 * block)
{
	u32 t, l, r;

	/* Get inblock into l,r */
	r = ((u32) block[0] << 24) | ((u32) block[1] << 16)
	    | ((u32) block[2] << 8) | (u32) block[3];
	l = ((u32) block[4] << 24) | ((u32) block[5] << 16)
	    | ((u32) block[6] << 8) | (u32) block[7];
	/* Do the work */
	/* Only do full 16 rounds if key length > 80 bits */
	if (key->rounds > 12) {
		F1(r, l, 15);
		F3(l, r, 14);
		F2(r, l, 13);
		F1(l, r, 12);
	}
	F3(r, l, 11);
	F2(l, r, 10);
	F1(r, l, 9);
	F3(l, r, 8);
	F2(r, l, 7);
	F1(l, r, 6);
	F3(r, l, 5);
	F2(l, r, 4);
	F1(r, l, 3);
	F3(l, r, 2);
	F2(r, l, 1);
	F1(l, r, 0);
	/* Put l,r into outblock */
	block[0] = U8a(l);
	block[1] = U8b(l);
	block[2] = U8c(l);
	block[3] = U8d(l);
	block[4] = U8a(r);
	block[5] = U8b(r);
	block[6] = U8c(r);
	block[7] = U8d(r);
	/* Wipe clean */
	t = l = r = 0;
}


/***** Key Schedual *****/

WIN32DLL_DEFINE
    int _mcrypt_set_key(CAST_KEY * key, u8 * rawkey, unsigned keybytes)
{
	u32 t[4], z[4], x[4];
	unsigned i;

	/* Set number of rounds to 12 or 16, depending on key length */
	key->rounds = (keybytes <= CAST_SMALL_KEY)
	    ? CAST_SMALL_ROUNDS : CAST_FULL_ROUNDS;


	/* Copy key to workspace x */
	for (i = 0; i < 4; i++) {
		x[i] = 0;
		if ((i * 4 + 0) < keybytes)
			x[i] = (u32) rawkey[i * 4 + 0] << 24;
		if ((i * 4 + 1) < keybytes)
			x[i] |= (u32) rawkey[i * 4 + 1] << 16;
		if ((i * 4 + 2) < keybytes)
			x[i] |= (u32) rawkey[i * 4 + 2] << 8;
		if ((i * 4 + 3) < keybytes)
			x[i] |= (u32) rawkey[i * 4 + 3];
	}
	/* Generate 32 subkeys, four at a time */
	for (i = 0; i < 32; i += 4) {
		switch (i & 4) {
		case 0:
			t[0] = z[0] = x[0] ^ cast_sbox5[U8b(x[3])]
			    ^ cast_sbox6[U8d(x[3])] ^ cast_sbox7[U8a(x[3])]
			    ^ cast_sbox8[U8c(x[3])] ^
			    cast_sbox7[U8a(x[2])];
			t[1] = z[1] = x[2] ^ cast_sbox5[U8a(z[0])]
			    ^ cast_sbox6[U8c(z[0])] ^ cast_sbox7[U8b(z[0])]
			    ^ cast_sbox8[U8d(z[0])] ^
			    cast_sbox8[U8c(x[2])];
			t[2] = z[2] = x[3] ^ cast_sbox5[U8d(z[1])]
			    ^ cast_sbox6[U8c(z[1])] ^ cast_sbox7[U8b(z[1])]
			    ^ cast_sbox8[U8a(z[1])] ^
			    cast_sbox5[U8b(x[2])];
			t[3] = z[3] =
			    x[1] ^ cast_sbox5[U8c(z[2])] ^
			    cast_sbox6[U8b(z[2])] ^ cast_sbox7[U8d(z[2])]
			    ^ cast_sbox8[U8a(z[2])] ^
			    cast_sbox6[U8d(x[2])];
			break;
		case 4:
			t[0] = x[0] = z[2] ^ cast_sbox5[U8b(z[1])]
			    ^ cast_sbox6[U8d(z[1])] ^ cast_sbox7[U8a(z[1])]
			    ^ cast_sbox8[U8c(z[1])] ^
			    cast_sbox7[U8a(z[0])];
			t[1] = x[1] = z[0] ^ cast_sbox5[U8a(x[0])]
			    ^ cast_sbox6[U8c(x[0])] ^ cast_sbox7[U8b(x[0])]
			    ^ cast_sbox8[U8d(x[0])] ^
			    cast_sbox8[U8c(z[0])];
			t[2] = x[2] = z[1] ^ cast_sbox5[U8d(x[1])]
			    ^ cast_sbox6[U8c(x[1])] ^ cast_sbox7[U8b(x[1])]
			    ^ cast_sbox8[U8a(x[1])] ^
			    cast_sbox5[U8b(z[0])];
			t[3] = x[3] = z[3] ^ cast_sbox5[U8c(x[2])]
			    ^ cast_sbox6[U8b(x[2])] ^ cast_sbox7[U8d(x[2])]
			    ^ cast_sbox8[U8a(x[2])] ^
			    cast_sbox6[U8d(z[0])];
			break;
		}
		switch (i & 12) {
		case 0:
		case 12:
			key->xkey[i + 0] =
			    cast_sbox5[U8a(t[2])] ^ cast_sbox6[U8b(t[2])]
			    ^ cast_sbox7[U8d(t[1])] ^
			    cast_sbox8[U8c(t[1])];
			key->xkey[i + 1] =
			    cast_sbox5[U8c(t[2])] ^ cast_sbox6[U8d(t[2])]
			    ^ cast_sbox7[U8b(t[1])] ^
			    cast_sbox8[U8a(t[1])];
			key->xkey[i + 2] =
			    cast_sbox5[U8a(t[3])] ^ cast_sbox6[U8b(t[3])]
			    ^ cast_sbox7[U8d(t[0])] ^
			    cast_sbox8[U8c(t[0])];
			key->xkey[i + 3] =
			    cast_sbox5[U8c(t[3])] ^ cast_sbox6[U8d(t[3])]
			    ^ cast_sbox7[U8b(t[0])] ^
			    cast_sbox8[U8a(t[0])];
			break;
		case 4:
		case 8:
			key->xkey[i + 0] =
			    cast_sbox5[U8d(t[0])] ^ cast_sbox6[U8c(t[0])]
			    ^ cast_sbox7[U8a(t[3])] ^
			    cast_sbox8[U8b(t[3])];
			key->xkey[i + 1] =
			    cast_sbox5[U8b(t[0])] ^ cast_sbox6[U8a(t[0])]
			    ^ cast_sbox7[U8c(t[3])] ^
			    cast_sbox8[U8d(t[3])];
			key->xkey[i + 2] =
			    cast_sbox5[U8d(t[1])] ^ cast_sbox6[U8c(t[1])]
			    ^ cast_sbox7[U8a(t[2])] ^
			    cast_sbox8[U8b(t[2])];
			key->xkey[i + 3] =
			    cast_sbox5[U8b(t[1])] ^ cast_sbox6[U8a(t[1])]
			    ^ cast_sbox7[U8c(t[2])] ^
			    cast_sbox8[U8d(t[2])];
			break;
		}
		switch (i & 12) {
		case 0:
			key->xkey[i + 0] ^= cast_sbox5[U8c(z[0])];
			key->xkey[i + 1] ^= cast_sbox6[U8c(z[1])];
			key->xkey[i + 2] ^= cast_sbox7[U8b(z[2])];
			key->xkey[i + 3] ^= cast_sbox8[U8a(z[3])];
			break;
		case 4:
			key->xkey[i + 0] ^= cast_sbox5[U8a(x[2])];
			key->xkey[i + 1] ^= cast_sbox6[U8b(x[3])];
			key->xkey[i + 2] ^= cast_sbox7[U8d(x[0])];
			key->xkey[i + 3] ^= cast_sbox8[U8d(x[1])];
			break;
		case 8:
			key->xkey[i + 0] ^= cast_sbox5[U8b(z[2])];
			key->xkey[i + 1] ^= cast_sbox6[U8a(z[3])];
			key->xkey[i + 2] ^= cast_sbox7[U8c(z[0])];
			key->xkey[i + 3] ^= cast_sbox8[U8c(z[1])];
			break;
		case 12:
			key->xkey[i + 0] ^= cast_sbox5[U8d(x[0])];
			key->xkey[i + 1] ^= cast_sbox6[U8d(x[1])];
			key->xkey[i + 2] ^= cast_sbox7[U8a(x[2])];
			key->xkey[i + 3] ^= cast_sbox8[U8b(x[3])];
			break;
		}
		if (i >= 16) {
			key->xkey[i + 0] &= 31;
			key->xkey[i + 1] &= 31;
			key->xkey[i + 2] &= 31;
			key->xkey[i + 3] &= 31;
		}
	}
	/* Wipe clean */
	for (i = 0; i < 4; i++) {
		t[i] = x[i] = z[i] = 0;
	}
	return 0;
}

/* Made in Canada */


WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(CAST_KEY);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 8;
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 16;
}

static const int key_sizes[] = { 16 };
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = sizeof(key_sizes)/sizeof(int);
	return key_sizes;

}

WIN32DLL_DEFINE const char *_mcrypt_get_algorithms_name()
{
return "CAST-128";
}

#define CIPHER "434e25460c8c9525"

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
