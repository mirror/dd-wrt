/*
 * Copyright (C) 1998,1999,2000,2001,2002 Nikos Mavroyanopoulos
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

#define _init_mcrypt cfb_LTX__init_mcrypt
#define _mcrypt_set_state cfb_LTX__mcrypt_set_state
#define _mcrypt_get_state cfb_LTX__mcrypt_get_state
#define _end_mcrypt cfb_LTX__end_mcrypt
#define _mcrypt cfb_LTX__mcrypt
#define _mdecrypt cfb_LTX__mdecrypt
#define _has_iv cfb_LTX__has_iv
#define _is_block_mode cfb_LTX__is_block_mode
#define _is_block_algorithm_mode cfb_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name cfb_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size cfb_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version cfb_LTX__mcrypt_mode_version

typedef struct cfb_buf {
	byte* s_register;
	byte* enc_s_register;
	int blocksize;
} CFB_BUFFER;

/* CFB MODE */

int _init_mcrypt( CFB_BUFFER* buf, void *key, int lenofkey, void *IV, int size)
{

    buf->s_register = buf->enc_s_register = NULL;
    
    buf->blocksize = size;
/* For cfb */
	buf->s_register=malloc( size);
    if (buf->s_register==NULL) goto freeall;

	buf->enc_s_register=malloc( size);
    if (buf->enc_s_register==NULL) goto freeall;

	if (IV!=NULL) {
		memcpy(buf->s_register, IV, size);
	} else {
		memset(buf->s_register, 0, size);	
	}
/* End cfb */
	return 0;

	freeall:
		free(buf->s_register);
		free(buf->enc_s_register);
		return -1;
}


int _mcrypt_set_state( CFB_BUFFER* buf, void *IV, int size)
{
	memcpy(buf->enc_s_register, IV, size);
	memcpy(buf->s_register, IV, size);

	return 0;
}

int _mcrypt_get_state( CFB_BUFFER* buf, byte *IV, int *size)
{
	if (*size < buf->blocksize) {
		*size = buf->blocksize;
		return -1;
	}
	*size = buf->blocksize;

	memcpy( IV, buf->s_register, buf->blocksize);

	return 0;
}


void _end_mcrypt( CFB_BUFFER* buf) {
	free(buf->s_register);
	free(buf->enc_s_register);
}

int _mcrypt( CFB_BUFFER* buf, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext is 1 byte (8bit cfb) */
	char *plain = plaintext;
	int i, j;
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;

	for (j = 0; j < len; j++) {

		memcpy(buf->enc_s_register, buf->s_register, blocksize);

		_mcrypt_block_encrypt(akey, buf->enc_s_register);

		plain[j] ^= buf->enc_s_register[0];

/* Shift the register */
		for (i = 0; i < (blocksize - 1); i++)
			buf->s_register[i] = buf->s_register[i + 1];

		buf->s_register[blocksize - 1] = plain[j];
	}

	return 0;

}


int _mdecrypt( CFB_BUFFER* buf, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext is 1 byte (8bit ofb) */
	char *plain = plaintext;
	int i, j;
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;

	for (j = 0; j < len; j++) {

		memcpy(buf->enc_s_register, buf->s_register, blocksize);

		_mcrypt_block_encrypt(akey, buf->enc_s_register);

/* Shift the register */
		for (i = 0; i < (blocksize - 1); i++)
			buf->s_register[i] = buf->s_register[i + 1];

		buf->s_register[blocksize - 1] = plain[j];

		plain[j] ^= buf->enc_s_register[0];

	}

	return 0;
}

int _has_iv() { return 1; }
int _is_block_mode() { return 0; }
int _is_block_algorithm_mode() { return 1; }
const char *_mcrypt_get_modes_name() { return "CFB"; }
int _mcrypt_mode_get_size () {return sizeof(CFB_BUFFER);}

word32 _mcrypt_mode_version() {
	return 20020310;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
