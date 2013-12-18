/* 
 * Copyright (C) 1998,1999,2000,2001 Nikos Mavroyanopoulos
 *
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <lib/libdefs.h>
#include <lib/mcrypt_modules.h>
#include <stdlib.h>
#include "arcfour.h"

#define _mcrypt_set_key arcfour_LTX__mcrypt_set_key
#define _mcrypt_encrypt arcfour_LTX__mcrypt_encrypt
#define _mcrypt_decrypt arcfour_LTX__mcrypt_decrypt
#define _mcrypt_get_size arcfour_LTX__mcrypt_get_size
#define _mcrypt_get_block_size arcfour_LTX__mcrypt_get_block_size
#define _is_block_algorithm arcfour_LTX__is_block_algorithm
#define _mcrypt_get_key_size arcfour_LTX__mcrypt_get_key_size
#define _mcrypt_get_algo_iv_size arcfour_LTX__mcrypt_get_algo_iv_size
#define _mcrypt_get_supported_key_sizes arcfour_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name arcfour_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test arcfour_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version arcfour_LTX__mcrypt_algorithm_version

#define swap_byte(x,y) tmp = (x); (x) = (y); (y) = tmp

/* #define USE_IV */
/* IV was an mcrypt extension */

void _mcrypt_encrypt(arcfour_key * key, byte * buffer_ptr, int buffer_len);

/* key is the state that should be allocated by the caller */
#define STATE key->state
#define I key->i
#define J key->j
WIN32DLL_DEFINE
    int _mcrypt_set_key(arcfour_key * key, byte * key_data, int key_len,
			char *IV, int iv_len)
{
	register int tmp, j, i;
	byte *state = STATE;
	int ivindex;

	for (i = 0; i < 256; i++)
		state[i] = i;

	I = 0;
	J = 0;
	ivindex = 0;

	for (j = i = 0; i < 256; i++) {
		j += state[i] + key_data[i % key_len];
#ifdef USE_IV
		if (iv_len > 0 && IV != NULL)
			j += IV[ivindex];
#endif
		j &= 0xff;
		swap_byte(state[i], state[j]);
#ifdef USE_IV
		if (iv_len > 0)
			ivindex = (ivindex + 1) % iv_len;
#endif
	}

	return 0;
}

WIN32DLL_DEFINE
    void _mcrypt_encrypt(arcfour_key * key, byte * buffer_ptr,
			 int buffer_len)
{
	register int tmp;
	register int counter;
	register int i = I, j = J;
	byte *state = STATE;


	for (counter = 0; counter < buffer_len; counter++) {
		i++;
		i &= 0xff;
		j += state[i];
		j &= 0xff;

		/* swap state[i] and state[j] */
		swap_byte(state[i], state[j]);
		buffer_ptr[counter] ^= state[(state[i] + tmp) & 0xff];
	}
	J = j;
	I = i;
}

/* this is the same with mcrypt_encrypt */
WIN32DLL_DEFINE
    void _mcrypt_decrypt(arcfour_key * key, byte * buffer_ptr,
			 int buffer_len)
{
	_mcrypt_encrypt(key, buffer_ptr, buffer_len);
}

WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(arcfour_key);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_algo_iv_size()
{
#ifdef USE_IV
	return 32;
#else
	return 0;
#endif
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 0;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 256;
}
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = 0;
	return NULL;
}
WIN32DLL_DEFINE const char *_mcrypt_get_algorithms_name()
{
return "RC4";
}

#define CIPHER "3abaa03a286e24c4196d292ab72934d6854c3eee"

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
	_mcrypt_decrypt(key, (void *) ciphertext, blocksize);

	free(keyword);
	free(key);
	if (strcmp(ciphertext, plaintext) != 0) {
		printf("failed internally\n");
		return -1;
	}

	return 0;
}

WIN32DLL_DEFINE word32 _mcrypt_algorithm_version()
{
	return 20020610;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
