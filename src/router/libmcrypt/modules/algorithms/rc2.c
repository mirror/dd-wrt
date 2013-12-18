/**********************************************************************\
* To commemorate the 1996 RSA Data Security Conference, the following  *
* code is released into the public domain by its author.  Prost!       *
*                                                                      *
* This cipher uses 16-bit words and little-endian byte ordering.       *
* I wonder which processor it was optimized for?                       *
*                                                                      *
* Thanks to CodeView, SoftIce, and D86 for helping bring this code to  *
* the public.                                                          *
\**********************************************************************/

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/* $Id: rc2.c,v 1.14 2003/01/19 17:48:27 nmav Exp $ */

#include <libdefs.h>

#include <mcrypt_modules.h>
/* #include <assert.h> */
#include "rc2.h"

#define _mcrypt_set_key rc2_LTX__mcrypt_set_key
#define _mcrypt_encrypt rc2_LTX__mcrypt_encrypt
#define _mcrypt_decrypt rc2_LTX__mcrypt_decrypt
#define _mcrypt_get_size rc2_LTX__mcrypt_get_size
#define _mcrypt_get_block_size rc2_LTX__mcrypt_get_block_size
#define _is_block_algorithm rc2_LTX__is_block_algorithm
#define _mcrypt_get_key_size rc2_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes rc2_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name rc2_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test rc2_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version rc2_LTX__mcrypt_algorithm_version

/**********************************************************************\
* Expand a variable-length user key (between 1 and 128 bytes) to a     *
* 64-short working rc2 key, of at most "bits" effective key bits.      *
* The effective key bits parameter looks like an export control hack.  *
* For normal use, it should always be set to 1024.  For convenience,   *
* zero is accepted as an alias for 1024.                               *
\**********************************************************************/

WIN32DLL_DEFINE
    int _mcrypt_set_key(word16 * xkey, const byte * key, unsigned int len)
{
	unsigned int j;
	byte *xkey_p = (void *) xkey;
	int i;

	/* 256-entry permutation table, probably derived somehow from pi */
	static const byte permute[256] = {
		217, 120, 249, 196, 25, 221, 181, 237, 40, 233, 253, 121,
		74, 160, 216, 157,
		198, 126, 55, 131, 43, 118, 83, 142, 98, 76, 100, 136,
		68, 139, 251, 162,
		23, 154, 89, 245, 135, 179, 79, 19, 97, 69, 109, 141, 9,
		129, 125, 50,
		189, 143, 64, 235, 134, 183, 123, 11, 240, 149, 33, 34,
		92, 107, 78, 130,
		84, 214, 101, 147, 206, 96, 178, 28, 115, 86, 192, 20,
		167, 140, 241, 220,
		18, 117, 202, 31, 59, 190, 228, 209, 66, 61, 212, 48,
		163, 60, 182, 38,
		111, 191, 14, 218, 70, 105, 7, 87, 39, 242, 29, 155, 188,
		148, 67, 3,
		248, 17, 199, 246, 144, 239, 62, 231, 6, 195, 213, 47,
		200, 102, 30, 215,
		8, 232, 234, 222, 128, 82, 238, 247, 132, 170, 114, 172,
		53, 77, 106, 42,
		150, 26, 210, 113, 90, 21, 73, 116, 75, 159, 208, 94, 4,
		24, 164, 236,
		194, 224, 65, 110, 15, 81, 203, 204, 36, 145, 175, 80,
		161, 244, 112, 57,
		153, 124, 58, 133, 35, 184, 180, 122, 252, 2, 54, 91, 37,
		85, 151, 49,
		45, 93, 250, 152, 227, 138, 146, 174, 5, 223, 41, 16,
		103, 108, 186, 201,
		211, 0, 230, 207, 225, 158, 168, 44, 99, 22, 1, 63, 88,
		226, 137, 169,
		13, 56, 52, 27, 171, 51, 255, 176, 187, 72, 12, 95, 185,
		177, 205, 46,
		197, 243, 219, 71, 229, 165, 156, 119, 10, 166, 32, 104,
		254, 127, 193, 173
	};

/*	assert(len > 0 && len <= 128); */

	memmove(xkey, key, len);

	/* Phase 1: Expand input key to 128 bytes */

	for (j = len; j < 128; j++) {
		xkey_p[j] =
		    permute[(xkey_p[j - len] + xkey_p[j - 1]) % 256];
	}

	xkey_p[0] = permute[xkey_p[0]];

	/* Phase 2 - reduce effective key size to "bits" */
	/* stripped */

	/* Phase 3 - copy to xkey in little-endian order */
	i = 63;
	do {
		xkey[i] = xkey_p[2 * i] + (xkey_p[2 * i + 1] << 8);
	} while (i--);

	return 0;
}


/**********************************************************************\
* Encrypt an 8-byte block of plaintext using the given key.            *
\**********************************************************************/

WIN32DLL_DEFINE void _mcrypt_encrypt(const word16 * xkey, word16 * plain)
{
	word16 x3, x2, x1, x0, i;

#ifdef WORDS_BIGENDIAN
	x3 = byteswap16(plain[3]);
	x2 = byteswap16(plain[2]);
	x1 = byteswap16(plain[1]);
	x0 = byteswap16(plain[0]);
#else
	x3 = plain[3];
	x2 = plain[2];
	x1 = plain[1];
	x0 = plain[0];
#endif

	for (i = 0; i < 16; i++) {
		x0 += (x1 & ~x3) + (x2 & x3) + xkey[4 * i + 0];
		x0 = rotl16(x0, 1);

		x1 += (x2 & ~x0) + (x3 & x0) + xkey[4 * i + 1];
		x1 = rotl16(x1, 2);

		x2 += (x3 & ~x1) + (x0 & x1) + xkey[4 * i + 2];
		x2 = rotl16(x2, 3);

		x3 += (x0 & ~x2) + (x1 & x2) + xkey[4 * i + 3];
		x3 = rotl16(x3, 5);

		if (i == 4 || i == 10) {
			x0 += xkey[x3 & 63];
			x1 += xkey[x0 & 63];
			x2 += xkey[x1 & 63];
			x3 += xkey[x2 & 63];
		}
	}

#ifdef WORDS_BIGENDIAN
	plain[0] = byteswap16(x0);
	plain[1] = byteswap16(x1);
	plain[2] = byteswap16(x2);
	plain[3] = byteswap16(x3);
#else
	plain[0] = (x0);
	plain[1] = (x1);
	plain[2] = (x2);
	plain[3] = (x3);

#endif

}

/**********************************************************************\
* Decrypt an 8-byte block of ciphertext using the given key.           *
\**********************************************************************/

WIN32DLL_DEFINE void _mcrypt_decrypt(const word16 * xkey, word16 * plain)
{
	word16 x3, x2, x1, x0, i;

#ifndef WORDS_BIGENDIAN
	x3 = plain[3];
	x2 = plain[2];
	x1 = plain[1];
	x0 = plain[0];
#else
	x3 = byteswap16(plain[3]);
	x2 = byteswap16(plain[2]);
	x1 = byteswap16(plain[1]);
	x0 = byteswap16(plain[0]);
#endif


	i = 15;
	do {
		x3 = rotr16(x3, 5);
		x3 -= (x0 & ~x2) + (x1 & x2) + xkey[4 * i + 3];

		x2 = rotr16(x2, 3);
		x2 -= (x3 & ~x1) + (x0 & x1) + xkey[4 * i + 2];

		x1 = rotr16(x1, 2);
		x1 -= (x2 & ~x0) + (x3 & x0) + xkey[4 * i + 1];

		x0 = rotr16(x0, 1);
		x0 -= (x1 & ~x3) + (x2 & x3) + xkey[4 * i + 0];

		if (i == 5 || i == 11) {
			x3 -= xkey[x2 & 63];
			x2 -= xkey[x1 & 63];
			x1 -= xkey[x0 & 63];
			x0 -= xkey[x3 & 63];
		}
	} while (i--);

#ifdef WORDS_BIGENDIAN
	plain[0] = byteswap16(x0);
	plain[1] = byteswap16(x1);
	plain[2] = byteswap16(x2);
	plain[3] = byteswap16(x3);
#else
	plain[0] = x0;
	plain[1] = x1;
	plain[2] = x2;
	plain[3] = x3;
#endif
}

WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return 64 * sizeof(word16);
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
	return 128;
}

WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = 0;
	return NULL;
}
WIN32DLL_DEFINE char *_mcrypt_get_algorithms_name()
{
return "RC2";
}

#define CIPHER "becbe4c8e6237a14"

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
