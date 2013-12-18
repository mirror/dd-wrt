/**********************************************************
   TEA - Tiny Encryption Algorithm
   Feistel cipher by David Wheeler & Roger M. Needham
   (extended version)
 **********************************************************/

/* $Id: xtea.c,v 1.13 2003/01/19 17:48:27 nmav Exp $ */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

#include <libdefs.h>

#include <mcrypt_modules.h>

#define _mcrypt_set_key xtea_LTX__mcrypt_set_key
#define _mcrypt_encrypt xtea_LTX__mcrypt_encrypt
#define _mcrypt_decrypt xtea_LTX__mcrypt_decrypt
#define _mcrypt_get_size xtea_LTX__mcrypt_get_size
#define _mcrypt_get_block_size xtea_LTX__mcrypt_get_block_size
#define _is_block_algorithm xtea_LTX__is_block_algorithm
#define _mcrypt_get_key_size xtea_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes xtea_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name xtea_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test xtea_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version xtea_LTX__mcrypt_algorithm_version

#define ROUNDS 32
#define DELTA 0x9e3779b9	/* sqr(5)-1 * 2^31 */

/**********************************************************
   Input values: 	k[4]	128-bit key
			v[2]    64-bit plaintext block
   Output values:	v[2]    64-bit ciphertext block 
 **********************************************************/

WIN32DLL_DEFINE
    int _mcrypt_set_key(word32 * k, word32 * input_key, int len)
{
	k[0] = 0;
	k[2] = 0;
	k[1] = 0;
	k[3] = 0;
	memmove(k, input_key, len);
	return 0;
}

WIN32DLL_DEFINE void _mcrypt_encrypt(word32 * k, word32 * v)
{
#ifdef WORDS_BIGENDIAN
	word32 y = v[0], z = v[1];
#else
	word32 y = byteswap32(v[0]), z = byteswap32(v[1]);
#endif
	word32 limit, sum = 0;
	int N = ROUNDS;

	limit = DELTA * N;
#ifdef WORDS_BIGENDIAN
	while (sum != limit) {
		y += (((z << 4) ^ (z >> 5)) + z) ^ (sum + k[sum & 3]);
		sum += DELTA;
		z += (((y << 4) ^ (y >> 5)) + y) ^ (sum +
						    k[(sum >> 11) & 3]);
	}
#else
	while (sum != limit) {
		y += (((z << 4) ^ (z >> 5)) + z) ^ (sum +
						    byteswap32(k
							       [sum & 3]));
		sum += DELTA;
		z += (((y << 4) ^ (y >> 5)) + y) ^ (sum +
						    byteswap32(k
							       [(sum >>
								 11) &
								3]));
	}
#endif

#ifdef WORDS_BIGENDIAN
	v[0] = y;
	v[1] = z;
#else
	v[0] = byteswap32(y);
	v[1] = byteswap32(z);
#endif
}

WIN32DLL_DEFINE void _mcrypt_decrypt(word32 * k, word32 * v)
{
#ifdef WORDS_BIGENDIAN
	word32 y = v[0], z = v[1];
#else
	word32 y = byteswap32(v[0]), z = byteswap32(v[1]);
#endif
	word32 limit, sum = 0;
	int N = (-ROUNDS);

	limit = DELTA * N;
#ifdef WORDS_BIGENDIAN

	sum = DELTA * (-N);
	while (sum) {
		z -= (((y << 4) ^ (y >> 5)) + y) ^ (sum +
						    k[(sum >> 11) & 3]);
		sum -= DELTA;
		y -= (((z << 4) ^ (z >> 5)) + z) ^ (sum + k[sum & 3]);
#else
	sum = DELTA * (-N);
	while (sum) {
		z -= (((y << 4) ^ (y >> 5)) + y) ^ (sum +
						    byteswap32(k
							       [(sum >>
								 11) &
								3]));
		sum -= DELTA;
		y -= (((z << 4) ^ (z >> 5)) + z) ^ (sum +
						    byteswap32(k
							       [sum & 3]));
#endif
	}
#ifdef WORDS_BIGENDIAN
	v[0] = y;
	v[1] = z;
#else
	v[0] = byteswap32(y);
	v[1] = byteswap32(z);
#endif
}

/*
void _mcrypt_encrypt(word32 * k, word32 * v)
{
	_mcrypt_tean(k, v, ROUNDS);
}

void _mcrypt_decrypt(word32 * k, word32 * v)
{
	_mcrypt_tean(k, v, -ROUNDS);
}
*/

WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return 4 * sizeof(word32);
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
return "xTEA";
}

#define CIPHER "f61e7ff6da7cdb27"

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
