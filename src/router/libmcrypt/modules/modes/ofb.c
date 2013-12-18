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

#define _init_mcrypt ofb_LTX__init_mcrypt
#define _mcrypt_set_state ofb_LTX__mcrypt_set_state
#define _mcrypt_get_state ofb_LTX__mcrypt_get_state
#define _end_mcrypt ofb_LTX__end_mcrypt
#define _mcrypt ofb_LTX__mcrypt
#define _mdecrypt ofb_LTX__mdecrypt
#define _has_iv ofb_LTX__has_iv
#define _is_block_mode ofb_LTX__is_block_mode
#define _is_block_algorithm_mode ofb_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name ofb_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size ofb_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version ofb_LTX__mcrypt_mode_version

typedef struct ofb_buf {
	byte* s_register;
	byte* enc_s_register;
	int blocksize;
} OFB_BUFFER;


/* OFB MODE */

int _init_mcrypt( OFB_BUFFER* buf, void *key, int lenofkey, void *IV, int size)
{

    buf->s_register = buf->enc_s_register = NULL;

    buf->blocksize = size;
    
    /* For ofb */
	buf->s_register=malloc( size);
    if (buf->s_register==NULL) goto freeall;

	buf->enc_s_register=malloc( size);
    if (buf->enc_s_register==NULL) goto freeall;

	if (IV!=NULL) {
		memcpy(buf->s_register, IV, size);
	} else {
		memset(buf->s_register, 0, size);
	}
/* End ofb */

	return 0;

	freeall:
		free(buf->s_register);
		free(buf->enc_s_register);
		return -1;
}

int _mcrypt_get_state( OFB_BUFFER* buf, byte *IV, int *size)
{
	if (*size < buf->blocksize) {
		*size = buf->blocksize;
		return -1;
	}
	*size = buf->blocksize;

	memcpy( IV, buf->s_register, buf->blocksize);

	return 0;
}

int _mcrypt_set_state( OFB_BUFFER* buf, void *IV, int size)
{
	memcpy(buf->enc_s_register, IV, size);
	memcpy(buf->s_register, IV, size);

	return 0;
}


void _end_mcrypt( OFB_BUFFER* buf) {
	free(buf->s_register);
	free(buf->enc_s_register);
}


int _mcrypt( OFB_BUFFER* buf,void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*) )
{				/* plaintext is 1 byte (8bit ofb) */
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

		buf->s_register[blocksize - 1] = buf->enc_s_register[0];
	}

	return 0;

}


int _mdecrypt( OFB_BUFFER* buf, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
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

		buf->s_register[blocksize - 1] = buf->enc_s_register[0];

		plain[j] ^= buf->enc_s_register[0];

	}

	return 0;
}

int _is_block_mode() { return 0; }
int _has_iv() { return 1; }
int _is_block_algorithm_mode() { return 1; }
char *_mcrypt_get_modes_name() { return "OFB"; }
int _mcrypt_mode_get_size () {return sizeof(OFB_BUFFER);}

word32 _mcrypt_mode_version() {
	return 20010310;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
