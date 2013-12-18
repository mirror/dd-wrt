/*
 *    "enigma.c" is in file cbw.tar from
 *      anonymous FTP host watmsg.waterloo.edu: pub/crypt/cbw.tar.Z
 *
 *      A one-rotor machine designed along the lines of Enigma
 *      but considerably trivialized.
 *
 *      A public-domain replacement for the UNIX "crypt" command.
 *      Changed to fit in mcrypt by nmav@hellug.gr
 */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

/* $Id: enigma.c,v 1.13 2003/01/19 17:48:27 nmav Exp $ */

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "enigma.h"

#define _mcrypt_set_key enigma_LTX__mcrypt_set_key
#define _mcrypt_encrypt enigma_LTX__mcrypt_encrypt
#define _mcrypt_decrypt enigma_LTX__mcrypt_decrypt
#define _mcrypt_get_size enigma_LTX__mcrypt_get_size
#define _mcrypt_get_block_size enigma_LTX__mcrypt_get_block_size
#define _is_block_algorithm enigma_LTX__is_block_algorithm
#define _mcrypt_get_key_size enigma_LTX__mcrypt_get_key_size
#define _mcrypt_get_algo_iv_size enigma_LTX__mcrypt_get_algo_iv_size
#define _mcrypt_get_supported_key_sizes enigma_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name enigma_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test enigma_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version enigma_LTX__mcrypt_algorithm_version

/* it needs to be linked against libufc or libcrypt */
/* it no longer needs that. It just needs the password to be
 * transformed by unix crypt() or the mhash SCRYPT 
 */

WIN32DLL_DEFINE
    int _mcrypt_set_key(CRYPT_KEY * ckey, char *password, int plen,
			void *u1, int u2)
{
	int ic, i, k, temp;
	unsigned random;
	sword32 seed;


	Bzero(ckey, sizeof(CRYPT_KEY));
	ckey->n1 = ckey->n2 = ckey->nr1 = ckey->nr2 = 0;

	if (plen > 13)
		plen = 13;

	memmove(ckey->cbuf, password, plen);

	seed = 123;
	for (i = 0; i < 13; i++)
		seed = seed * ckey->cbuf[i] + i;
	for (i = 0; i < ROTORSZ; i++) {
		ckey->t1[i] = i;
		ckey->deck[i] = i;
	}
	for (i = 0; i < ROTORSZ; i++) {
		seed = 5 * seed + ckey->cbuf[i % 13];
		random = seed % 65521;
		k = ROTORSZ - 1 - i;
		ic = (random & MASK) % (k + 1);
		random >>= 8;

		temp = ckey->t1[k];
		ckey->t1[k] = ckey->t1[ic];
		ckey->t1[ic] = temp;
		if (ckey->t3[k] != 0)
			continue;

		ic = (random & MASK) % k;
		while (ckey->t3[ic] != 0)
			ic = (ic + 1) % k;
		ckey->t3[k] = ic;
		ckey->t3[ic] = k;
	}

	for (i = 0; i < ROTORSZ; i++)
		ckey->t2[ckey->t1[i] & MASK] = i;

	return 0;
}


int shuffle(CRYPT_KEY * ckey)
{
	int i, ic, k, temp;
	unsigned random;
	static sword32 seed = 123;

	for (i = 0; i < ROTORSZ; i++) {
		seed = 5 * seed + ckey->cbuf[i % 13];
		random = seed % 65521;
		k = ROTORSZ - 1 - i;
		ic = (random & MASK) % (k + 1);
		temp = ckey->deck[k];
		ckey->deck[k] = ckey->deck[ic];
		ckey->deck[ic] = temp;
	}
	return 0;
}

WIN32DLL_DEFINE
    void _mcrypt_encrypt(CRYPT_KEY * ckey, void *gtext, int textlen)
{				/* 0 or 1 */
	int i, j;
	int secureflg = 0;
	char *text = gtext;

	for (j = 0; j < textlen; j++) {

		i = text[j];
		if (secureflg) {
			ckey->nr1 = ckey->deck[ckey->n1] & MASK;
			ckey->nr2 = ckey->deck[ckey->nr1] & MASK;
		} else {
			ckey->nr1 = ckey->n1;
		}
		i = ckey->
		    t2[(ckey->
			t3[(ckey->t1[(i + ckey->nr1) & MASK] +
			    ckey->nr2) & MASK] - ckey->nr2) & MASK] -
		    ckey->nr1;
		text[j] = i;
		ckey->n1++;
		if (ckey->n1 == ROTORSZ) {
			ckey->n1 = 0;
			ckey->n2++;
			if (ckey->n2 == ROTORSZ)
				ckey->n2 = 0;
			if (secureflg) {
				shuffle(ckey);
			} else {
				ckey->nr2 = ckey->n2;
			}
		}
	}

	return;
}

WIN32DLL_DEFINE
    void _mcrypt_decrypt(CRYPT_KEY * ckey, void *gtext, int textlen)
{				/* 0 or 1 */
	int i, j;
	int secureflg = 0;
	char *text = gtext;


	for (j = 0; j < textlen; j++) {

		i = text[j];
		if (secureflg) {
			ckey->nr1 = ckey->deck[ckey->n1] & MASK;
			ckey->nr2 = ckey->deck[ckey->nr1] & MASK;
		} else {
			ckey->nr1 = ckey->n1;
		}
		i = ckey->
		    t2[(ckey->
			t3[(ckey->t1[(i + ckey->nr1) & MASK] +
			    ckey->nr2) & MASK] - ckey->nr2) & MASK] -
		    ckey->nr1;
		text[j] = i;
		ckey->n1++;
		if (ckey->n1 == ROTORSZ) {
			ckey->n1 = 0;
			ckey->n2++;
			if (ckey->n2 == ROTORSZ)
				ckey->n2 = 0;
			if (secureflg) {
				shuffle(ckey);
			} else {
				ckey->nr2 = ckey->n2;
			}
		}
	}

	return;
}


WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(CRYPT_KEY);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_algo_iv_size()
{
	return 0;
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 0;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 13;
}
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = 0;
	return NULL;
}

WIN32DLL_DEFINE char *_mcrypt_get_algorithms_name()
{
return "enigma";
}

#define CIPHER "f3edda7da20f8975884600f014d32c7a08e59d7b"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	char *keyword;
	unsigned char plaintext[20];
	unsigned char ciphertext[20];
	int blocksize = 20, j;
	void *key;
	unsigned char cipher_tmp[200];

	keyword = calloc(1, _mcrypt_get_key_size());
	if (keyword == NULL)
		return -1;

	strcpy(keyword, "enadyotr");

	for (j = 0; j < blocksize; j++) {
		plaintext[j] = j % 256;
	}
	key = malloc(_mcrypt_get_size());
	if (key == NULL) {
		free(keyword);
		return -1;
	}
	memmove(ciphertext, plaintext, blocksize);

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size(),
			NULL, 0);
	_mcrypt_encrypt(key, (void *) ciphertext, blocksize);

	for (j = 0; j < blocksize; j++) {
		sprintf(&((char *) cipher_tmp)[2 * j], "%.2x",
			ciphertext[j]);
	}

	if (strcmp((char *) cipher_tmp, CIPHER) != 0) {
		printf("failed compatibility\n");
		printf("Expected: %s\nGot: %s\n", CIPHER,
		       (char *) cipher_tmp);
		free(keyword);
		free(key);
		return -1;
	}

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size(),
			NULL, 0);
	free(keyword);
	
	_mcrypt_decrypt(key, (void *) ciphertext, blocksize);
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
