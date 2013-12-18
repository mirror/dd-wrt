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

#include <libdefs.h>
#include <mcrypt_modules.h>

#define _init_mcrypt cbc_LTX__init_mcrypt
#define _mcrypt_set_state cbc_LTX__mcrypt_set_state
#define _mcrypt_get_state cbc_LTX__mcrypt_get_state
#define _end_mcrypt cbc_LTX__end_mcrypt
#define _mcrypt cbc_LTX__mcrypt
#define _mdecrypt cbc_LTX__mdecrypt
#define _has_iv cbc_LTX__has_iv
#define _is_block_mode cbc_LTX__is_block_mode
#define _is_block_algorithm_mode cbc_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name cbc_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size cbc_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version cbc_LTX__mcrypt_mode_version

typedef struct cbc_buf {
	word32 *previous_ciphertext;
	word32 *previous_cipher;
	int blocksize;
} CBC_BUFFER;

/* CBC MODE */


int _init_mcrypt( CBC_BUFFER* buf,void *key, int lenofkey, void *IV, int size)
{
/* For cbc */
	buf->previous_ciphertext =
	buf->previous_cipher = NULL;

	buf->blocksize = size;
		
	buf->previous_ciphertext = malloc( size);
	buf->previous_cipher = malloc( size);
	
	if (buf->previous_ciphertext==NULL ||
		buf->previous_cipher==NULL) goto freeall;
	
	if (IV!=NULL) {
		memcpy(buf->previous_ciphertext, IV, size);
	} else {
	        memset(buf->previous_ciphertext, 0, size);
	}

	return 0;

	freeall:
		free(buf->previous_ciphertext);
		free(buf->previous_cipher);
		return -1;
}

int _mcrypt_set_state( CBC_BUFFER* buf, void *IV, int size)
{
/* For cbc */

	memcpy(buf->previous_ciphertext, IV, size);
	memcpy(buf->previous_cipher, IV, size);
 
	return 0;
}

int _mcrypt_get_state( CBC_BUFFER* buf, void *IV, int *size)
{
	if (*size < buf->blocksize) {
		*size = buf->blocksize;
		return -1;
	}
	*size = buf->blocksize;

	memcpy( IV, buf->previous_ciphertext, buf->blocksize);

	return 0;
}


void _end_mcrypt( CBC_BUFFER* buf) {
	free(buf->previous_ciphertext);
	free(buf->previous_cipher);
}

int _mcrypt( CBC_BUFFER* buf, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{
	word32 *fplain = plaintext;
	word32 *plain;
	int i, j; 
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;
	
	for (j = 0; j < len / blocksize; j++) {

		plain = &fplain[j * blocksize / sizeof(word32)];

		for (i = 0; i < blocksize / sizeof(word32); i++) {
			plain[i] ^= buf->previous_ciphertext[i];
		}

		_mcrypt_block_encrypt(akey, plain);

		/* Copy the ciphertext to prev_ciphertext */
		memcpy(buf->previous_ciphertext, plain, blocksize);
	}
	if (j==0 && len!=0) return -1;
	return 0;
}



int _mdecrypt( CBC_BUFFER* buf, void *ciphertext, int len, int blocksize,void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{
	word32 *cipher;
	word32 *fcipher = ciphertext;
	int i, j; 
	void (*_mcrypt_block_decrypt) (void *, void *);

	_mcrypt_block_decrypt = func2;


	for (j = 0; j < len / blocksize; j++) {

		cipher = &fcipher[j * blocksize / sizeof(word32)];
		memcpy(buf->previous_cipher, cipher, blocksize);

		_mcrypt_block_decrypt(akey, cipher);
		for (i = 0; i < blocksize / sizeof(word32); i++) {
			cipher[i] ^= buf->previous_ciphertext[i];
		}

		/* Copy the ciphertext to prev_cipher */
		memcpy(buf->previous_ciphertext, buf->previous_cipher, blocksize);

	}
	if (j==0 && len!=0) return -1;
	return 0;
}

int _has_iv() { return 1; }
int _is_block_mode() { return 1; }
int _is_block_algorithm_mode() { return 1; }
const char *_mcrypt_get_modes_name() { return "CBC";}
int _mcrypt_mode_get_size () {return sizeof(CBC_BUFFER);}

word32 _mcrypt_mode_version() {
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
