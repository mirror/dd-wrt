/*
 * The GOST 28147-89 cipher
 *
 * This is based on the 25 Movember 1993 draft translation
 * by Aleksandr Malchik, with Whitfield Diffie, of the Government
 * Standard of the U.S.S.R. GOST 28149-89, "Cryptographic Transformation
 * Algorithm", effective 1 July 1990.  (Whitfield.Diffie@eng.sun.com)
 *
 * That is a draft, and may contain errors, which will be faithfully
 * reflected here, along with possible exciting new bugs.
 *
 * Some details have been cleared up by the paper "Soviet Encryption
 * Algorithm" by Josef Pieprzyk and Leonid Tombak of the University
 * of Wollongong, New South Wales.  (josef/leo@cs.adfa.oz.au)
 *
 * The standard is written by A. Zabotin (project leader), G.P. Glazkov,
 * and V.B. Isaeva.  It was accepted and introduced into use by the
 * action of the State Standards Committee of the USSR on 2 June 89 as
 * No. 1409.  It was to be reviewed in 1993, but whether anyone wishes
 * to take on this obligation from the USSR is questionable.
 *
 * This code is placed in the public domain.
 */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/* $Id: gost.c,v 1.13 2003/01/19 17:48:27 nmav Exp $ */

/*
 * If you read the standard, it belabors the point of copying corresponding
 * bits from point A to point B quite a bit.  It helps to understand that
 * the standard is uniformly little-endian, although it numbers bits from
 * 1 rather than 0, so bit n has value 2^(n-1).  The least significant bit
 * of the 32-bit words that are manipulated in the algorithm is the first,
 * lowest-numbered, in the bit string.
 */

#include <libdefs.h>

#include <mcrypt_modules.h>

#define _mcrypt_set_key gost_LTX__mcrypt_set_key
#define _mcrypt_encrypt gost_LTX__mcrypt_encrypt
#define _mcrypt_decrypt gost_LTX__mcrypt_decrypt
#define _mcrypt_get_size gost_LTX__mcrypt_get_size
#define _mcrypt_get_block_size gost_LTX__mcrypt_get_block_size
#define _is_block_algorithm gost_LTX__is_block_algorithm
#define _mcrypt_get_key_size gost_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes gost_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name gost_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test gost_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version gost_LTX__mcrypt_algorithm_version

void _mcrypt_kboxinit(void);

/*
 * The standard does not specify the contents of the 8 4 bit->4 bit
 * substitution boxes, saying they're a parameter of the network
 * being set up.  For illustration purposes here, I have used
 * the first rows of the 8 S-boxes from the DES.  (Note that the
 * DES S-boxes are numbered starting from 1 at the msb.  In keeping
 * with the rest of the GOST, I have used little-endian numbering.
 * Thus, k8 is S-box 1.
 *
 * Obviously, a careful look at the cryptographic properties of the cipher
 * must be undertaken before "production" substitution boxes are defined.
 *
 * The standard also does not specify a standard bit-string representation
 * for the contents of these blocks.
 */

/* These are NOT the original s-boxes. I replaced them with the ones
 * found in Applied Cryptography book by Bruce Schneier. These were
 * used in an application for the Central Bank of the Russian Federation
 * --Nikos
 */
static int init = 0;

static unsigned char const gost_k1[16] = {
	1, 15, 13, 0, 5, 7, 10, 4, 9, 2, 3, 14, 6, 11, 8, 2
};
static unsigned char const gost_k2[16] = {
	13, 11, 4, 1, 3, 15, 5, 9, 0, 10, 14, 7, 6, 8, 2, 12
};
static unsigned char const gost_k3[16] = {
	4, 11, 10, 0, 7, 2, 1, 13, 3, 6, 8, 5, 9, 12, 15, 14
};
static unsigned char const gost_k4[16] = {
	6, 12, 7, 1, 5, 15, 13, 8, 4, 10, 9, 14, 0, 3, 11, 2
};
static unsigned char const gost_k5[16] = {
	7, 13, 10, 1, 0, 8, 9, 15, 14, 4, 6, 12, 11, 2, 5, 3
};
static unsigned char const gost_k6[16] = {
	5, 8, 1, 13, 10, 3, 4, 2, 14, 15, 12, 7, 6, 0, 9, 11
};
static unsigned char const gost_k7[16] = {
	14, 11, 4, 12, 6, 13, 15, 10, 2, 3, 8, 1, 0, 7, 5, 9
};
static unsigned char const gost_k8[16] = {
	4, 10, 9, 2, 13, 8, 0, 14, 6, 11, 1, 12, 7, 15, 5, 3
};

/* Byte-at-a-time substitution boxes */
static unsigned char gost_k87[256];
static unsigned char gost_k65[256];
static unsigned char gost_k43[256];
static unsigned char gost_k21[256];

/*
 * Build byte-at-a-time subtitution tables.
 * This must be called once for global setup.
 */

WIN32DLL_DEFINE int _mcrypt_set_key(word32 * inst, word32 * key, int len)
{

	_mcrypt_kboxinit();
	inst[0] = 0;
	inst[1] = 0;
	inst[2] = 0;
	inst[3] = 0;
	inst[4] = 0;
	inst[5] = 0;
	inst[6] = 0;
	inst[7] = 0;
	memmove(inst, key, len);
#ifdef WORDS_BIGENDIAN
	inst[0] = byteswap32(inst[0]);
	inst[1] = byteswap32(inst[1]);
	inst[2] = byteswap32(inst[2]);
	inst[3] = byteswap32(inst[3]);
	inst[4] = byteswap32(inst[4]);
	inst[5] = byteswap32(inst[5]);
	inst[6] = byteswap32(inst[6]);
	inst[7] = byteswap32(inst[7]);
#endif
	return 0;
}

void _mcrypt_kboxinit(void)
{
	int i;

	if (init == 0) {
		init = 1;
		for (i = 0; i < 256; i++) {
			gost_k87[i] = gost_k8[i >> 4] << 4 | gost_k7[i & 15];
			gost_k65[i] = gost_k6[i >> 4] << 4 | gost_k5[i & 15];
			gost_k43[i] = gost_k4[i >> 4] << 4 | gost_k3[i & 15];
			gost_k21[i] = gost_k2[i >> 4] << 4 | gost_k1[i & 15];
		}
	}

}

/*
 * Do the substitution and rotation that are the core of the operation,
 * like the expansion, substitution and permutation of the DES.
 * It would be possible to perform DES-like optimisations and store
 * the table entries as 32-bit words, already rotated, but the
 * efficiency gain is questionable.
 *
 * This should be inlined for maximum speed
 */
static word32 f(word32 x)
{
	/* Do substitutions */
# if 0
	/* This is annoyingly slow */
	x = k8[x >> 28 & 15] << 28 | k7[x >> 24 & 15] << 24 |
	    k6[x >> 20 & 15] << 20 | k5[x >> 16 & 15] << 16 |
	    k4[x >> 12 & 15] << 12 | k3[x >> 8 & 15] << 8 |
	    k2[x >> 4 & 15] << 4 | k1[x & 15];
# else
	/* This is faster */
	x = gost_k87[x >> 24 & 255] << 24 | gost_k65[x >> 16 & 255] << 16 |
	    gost_k43[x >> 8 & 255] << 8 | gost_k21[x & 255];
# endif

	/* Rotate left 11 bits */
	return x << 11 | x >> (32 - 11);

}

/*
 * The GOST standard defines the input in terms of bits 1..64, with
 * bit 1 being the lsb of in[0] and bit 64 being the msb of in[1].
 *
 * The keys are defined similarly, with bit 256 being the msb of key[7].
 */
WIN32DLL_DEFINE void _mcrypt_encrypt(word32 const key[8], word32 * in)
{
	register word32 n1, n2;	/* As named in the GOST */

/* Added to make it compatible with bigendian machines 
 * --nikos 
 */

#ifndef WORDS_BIGENDIAN
	n1 = byteswap32(in[0]);
	n2 = byteswap32(in[1]);
#else
	n1 = in[0];
	n2 = in[1];
#endif

	/* Instead of swapping halves, swap names each round */
	n2 ^= f(n1 + key[0]);
	n1 ^= f(n2 + key[1]);
	n2 ^= f(n1 + key[2]);
	n1 ^= f(n2 + key[3]);
	n2 ^= f(n1 + key[4]);
	n1 ^= f(n2 + key[5]);
	n2 ^= f(n1 + key[6]);
	n1 ^= f(n2 + key[7]);

	n2 ^= f(n1 + key[0]);
	n1 ^= f(n2 + key[1]);
	n2 ^= f(n1 + key[2]);
	n1 ^= f(n2 + key[3]);
	n2 ^= f(n1 + key[4]);
	n1 ^= f(n2 + key[5]);
	n2 ^= f(n1 + key[6]);
	n1 ^= f(n2 + key[7]);

	n2 ^= f(n1 + key[0]);
	n1 ^= f(n2 + key[1]);
	n2 ^= f(n1 + key[2]);
	n1 ^= f(n2 + key[3]);
	n2 ^= f(n1 + key[4]);
	n1 ^= f(n2 + key[5]);
	n2 ^= f(n1 + key[6]);
	n1 ^= f(n2 + key[7]);

	n2 ^= f(n1 + key[7]);
	n1 ^= f(n2 + key[6]);
	n2 ^= f(n1 + key[5]);
	n1 ^= f(n2 + key[4]);
	n2 ^= f(n1 + key[3]);
	n1 ^= f(n2 + key[2]);
	n2 ^= f(n1 + key[1]);
	n1 ^= f(n2 + key[0]);

	/* There is no swap after the last round */

#ifndef WORDS_BIGENDIAN
	in[0] = byteswap32(n2);
	in[1] = byteswap32(n1);
#else
	in[0] = n2;
	in[1] = n1;
#endif
}


/*
 * The key schedule is somewhat different for decryption.
 * (The key table is used once forward and three times backward.)
 * You could define an expanded key, or just write the code twice,
 * as done here.
 */
WIN32DLL_DEFINE void _mcrypt_decrypt(word32 const key[8], word32 * in)
{
	register word32 n1, n2;	/* As named in the GOST */

#ifndef WORDS_BIGENDIAN
	n1 = byteswap32(in[0]);
	n2 = byteswap32(in[1]);
#else
	n1 = in[0];
	n2 = in[1];
#endif

	n2 ^= f(n1 + key[0]);
	n1 ^= f(n2 + key[1]);
	n2 ^= f(n1 + key[2]);
	n1 ^= f(n2 + key[3]);
	n2 ^= f(n1 + key[4]);
	n1 ^= f(n2 + key[5]);
	n2 ^= f(n1 + key[6]);
	n1 ^= f(n2 + key[7]);

	n2 ^= f(n1 + key[7]);
	n1 ^= f(n2 + key[6]);
	n2 ^= f(n1 + key[5]);
	n1 ^= f(n2 + key[4]);
	n2 ^= f(n1 + key[3]);
	n1 ^= f(n2 + key[2]);
	n2 ^= f(n1 + key[1]);
	n1 ^= f(n2 + key[0]);

	n2 ^= f(n1 + key[7]);
	n1 ^= f(n2 + key[6]);
	n2 ^= f(n1 + key[5]);
	n1 ^= f(n2 + key[4]);
	n2 ^= f(n1 + key[3]);
	n1 ^= f(n2 + key[2]);
	n2 ^= f(n1 + key[1]);
	n1 ^= f(n2 + key[0]);

	n2 ^= f(n1 + key[7]);
	n1 ^= f(n2 + key[6]);
	n2 ^= f(n1 + key[5]);
	n1 ^= f(n2 + key[4]);
	n2 ^= f(n1 + key[3]);
	n1 ^= f(n2 + key[2]);
	n2 ^= f(n1 + key[1]);
	n1 ^= f(n2 + key[0]);

#ifndef WORDS_BIGENDIAN
	in[0] = byteswap32(n2);
	in[1] = byteswap32(n1);
#else
	in[0] = n2;
	in[1] = n1;
#endif

}

WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return 8 * sizeof(word32);
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
	return 32;
}

static const int key_sizes[] = { 32 };
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = sizeof(key_sizes)/sizeof(int);
	return key_sizes;

}
WIN32DLL_DEFINE const char *_mcrypt_get_algorithms_name()
{
return "GOST";
}

#define CIPHER "e498cf78cdf1d4a5"

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
