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

#define _init_mcrypt ncfb_LTX__init_mcrypt
#define _mcrypt_set_state ncfb_LTX__mcrypt_set_state
#define _mcrypt_get_state ncfb_LTX__mcrypt_get_state
#define _end_mcrypt ncfb_LTX__end_mcrypt
#define _mcrypt ncfb_LTX__mcrypt
#define _mdecrypt ncfb_LTX__mdecrypt
#define _has_iv ncfb_LTX__has_iv
#define _is_block_mode ncfb_LTX__is_block_mode
#define _is_block_algorithm_mode ncfb_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name ncfb_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size ncfb_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version ncfb_LTX__mcrypt_mode_version

typedef struct ncfb_buf {
	byte*   enc_s_register;
	byte* s_register;
	int   s_register_pos;
	int   blocksize;
} nCFB_BUFFER;

/* nCFB MODE */

int _init_mcrypt( nCFB_BUFFER* buf, void *key, int lenofkey, void *IV, int size)
{
    buf->enc_s_register = buf->s_register = NULL;
    buf->s_register_pos = 0;
    
    buf->blocksize = size;

/* For cfb */
	buf->enc_s_register=malloc( size);
    if (buf->enc_s_register==NULL) goto freeall;
    
	buf->s_register=malloc( size);
    if (buf->s_register==NULL) goto freeall;

	if (IV!=NULL) {
		memcpy(buf->enc_s_register, IV, size);
		memcpy(buf->s_register, IV, size);
	} else {
		memset(buf->enc_s_register, 0, size);
		memset(buf->s_register, 0, size);
	}

/* End ncfb */

	return 0;
	freeall:
		free(buf->enc_s_register);
		free(buf->s_register);
		return -1;
}

int _mcrypt_set_state( nCFB_BUFFER* buf, byte *IV, int size)
{
	buf->s_register_pos = IV[0];
	memcpy(buf->enc_s_register, &IV[1], size-1);
	memcpy(buf->s_register, &IV[1], size-1);

	return 0;
}

int _mcrypt_get_state( nCFB_BUFFER* buf, byte *IV, int *size)
{
	if (*size < buf->blocksize + 1) {
		*size = buf->blocksize + 1;
		return -1;
	}
	*size = buf->blocksize + 1;

	IV[0] = buf->s_register_pos;
	memcpy( &IV[1], buf->s_register, buf->blocksize);

	return 0;
}

void _end_mcrypt( nCFB_BUFFER* buf) {
	free(buf->enc_s_register);
	free(buf->s_register);
}

inline static
void xor_stuff_en( nCFB_BUFFER *buf, void* akey, void (*func)(void*,void*), byte* plain,  int blocksize, int xor_size)
{
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;

	if (xor_size == blocksize) {
		if (buf->s_register_pos == 0) {

			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);
			
			memxor( plain, buf->enc_s_register, blocksize);

			memcpy(buf->s_register, plain, blocksize);

		} else {
			int size = blocksize - buf->s_register_pos;

			memxor( plain, &buf->enc_s_register[buf->s_register_pos],
				size); 
		
			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memxor( &plain[size], buf->enc_s_register,
				buf->s_register_pos);

			memcpy( &buf->s_register[size], 
				plain, buf->s_register_pos);

			/* buf->s_register_pos remains the same */
		}
	} else { /* xor_size != blocksize */
		if (buf->s_register_pos == 0) {
			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);
			
			memxor( plain, buf->enc_s_register, xor_size);

			memcpy(buf->s_register, plain, xor_size);

			buf->s_register_pos = xor_size;
		} else {
			int size = blocksize - buf->s_register_pos;
			int min_size =  size < xor_size ? size: xor_size;

			memxor( plain, &buf->enc_s_register[buf->s_register_pos],
				min_size);

			memcpy( &buf->s_register[buf->s_register_pos], plain, min_size);

			buf->s_register_pos += min_size;

			if (min_size >= xor_size)
				return;

			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memxor( &plain[min_size], buf->s_register,
				xor_size - min_size);

			buf->s_register_pos = xor_size - min_size;

			memcpy(buf->s_register, plain, xor_size - min_size);
		}
	
	}
	return;
}

inline static
void xor_stuff_de( nCFB_BUFFER *buf, void* akey, void (*func)(void*,void*), byte* cipher,  int blocksize, int xor_size)
{
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;

	if (xor_size == blocksize) {
		if (buf->s_register_pos == 0) {

			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memcpy(buf->s_register, cipher, blocksize);
			
			memxor( cipher, buf->enc_s_register, blocksize);


		} else {
			int size = blocksize - buf->s_register_pos;

			memxor( cipher, &buf->enc_s_register[buf->s_register_pos],
				size); 
		
			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memcpy( &buf->s_register[size], 
				cipher, buf->s_register_pos);

			memxor( &cipher[size], buf->enc_s_register,
				buf->s_register_pos);


			/* buf->s_register_pos remains the same */
		}
	} else { /* xor_size != blocksize */
		if (buf->s_register_pos == 0) {
			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memcpy(buf->s_register, cipher, xor_size);
			
			memxor( cipher, buf->enc_s_register, xor_size);


			buf->s_register_pos = xor_size;
		} else {
			int size = blocksize - buf->s_register_pos;
			int min_size =  size < xor_size ? size: xor_size;

			memxor( cipher, &buf->enc_s_register[buf->s_register_pos],
				min_size);

			memcpy( &buf->s_register[buf->s_register_pos], cipher, min_size);

			buf->s_register_pos += min_size;

			if (min_size >= xor_size)
				return;

			memcpy(buf->enc_s_register, buf->s_register, blocksize);

			_mcrypt_block_encrypt(akey, buf->enc_s_register);

			memcpy(buf->s_register, cipher, xor_size - min_size);

			memxor( &cipher[min_size], buf->s_register,
				xor_size - min_size);

			buf->s_register_pos = xor_size - min_size;

		}
	
	}
	return;
}


int _mcrypt( nCFB_BUFFER* buf,void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext is n*blocksize bytes (nbit cfb) */
	byte* plain;
	int i, j=0;
	void (*_mcrypt_block_encrypt) (void *, void *);
	int modlen;
	
	_mcrypt_block_encrypt = func;

	plain = plaintext;
	for (j = 0; j < len / blocksize; j++) {
		xor_stuff_en( buf, akey, func, plain, blocksize, blocksize); 
		
		plain += blocksize;

	}
	modlen = len % blocksize;
	if (modlen > 0) {
		xor_stuff_en( buf, akey, func, plain, blocksize, modlen); 
	}
	
	return 0;
}


int _mdecrypt( nCFB_BUFFER* buf,void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext is n*blocksize bytes (nbit cfb) */
	byte* plain;
	int i, j=0;
	void (*_mcrypt_block_encrypt) (void *, void *);
	int modlen;
	
	_mcrypt_block_encrypt = func;

	plain = plaintext;
	for (j = 0; j < len / blocksize; j++) {
		xor_stuff_de( buf, akey, func, plain, blocksize, blocksize); 
		
		plain += blocksize;

	}
	modlen = len % blocksize;
	if (modlen > 0) {
		xor_stuff_de( buf, akey, func, plain, blocksize, modlen); 
	}
	
	return 0;
}


int _has_iv() { return 1; }
int _is_block_mode() { return 0; }
int _is_block_algorithm_mode() { return 1; }
const char *_mcrypt_get_modes_name() { return "nCFB";}
int _mcrypt_mode_get_size () {return sizeof(nCFB_BUFFER);}


word32 _mcrypt_mode_version() {
	return 20020307;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
