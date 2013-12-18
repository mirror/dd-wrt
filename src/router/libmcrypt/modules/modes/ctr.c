/*
 * Copyright (C) 2002 Nikos Mavroyanopoulos
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

#define _init_mcrypt ctr_LTX__init_mcrypt
#define _mcrypt_set_state ctr_LTX__mcrypt_set_state
#define _mcrypt_get_state ctr_LTX__mcrypt_get_state
#define _end_mcrypt ctr_LTX__end_mcrypt
#define _mcrypt ctr_LTX__mcrypt
#define _mdecrypt ctr_LTX__mdecrypt
#define _has_iv ctr_LTX__has_iv
#define _is_block_mode ctr_LTX__is_block_mode
#define _is_block_algorithm_mode ctr_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name ctr_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size ctr_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version ctr_LTX__mcrypt_mode_version

typedef struct ctr_buf {
	byte* enc_counter;
	byte* c_counter;
	int c_counter_pos;
	int blocksize;
} CTR_BUFFER;

/* CTR MODE */

/* This function will add one to the given number (as a byte string).
 * has been reached.
 */
static void increase_counter( byte *x, int x_size) {
register int i, y=0;

	for (i=x_size-1;i>=0;i--) {
		y = 0;
		if ( x[i] == 0xff) {
			x[i] = 0;
			y = 1;
		} else x[i]++;

		if (y==0) break;
	}

	return;
}


/* size holds the size of the IV (counter in this mode).
 * This is the block size.
 */
int _init_mcrypt( CTR_BUFFER* buf, void *key, int lenofkey, void *IV, int size)
{
    buf->c_counter = buf->enc_counter = NULL;
    
/* For ctr */
    buf->c_counter_pos = 0;
    buf->blocksize = size;

	buf->c_counter=calloc( 1, size);
    if (buf->c_counter==NULL) goto freeall;

	buf->enc_counter=calloc( 1, size);
    if (buf->enc_counter==NULL) goto freeall;
    
    if (IV!=NULL) {
	memcpy(buf->enc_counter, IV, size);
	memcpy(buf->c_counter, IV, size);
    }

/* End ctr */

	return 0;
	freeall:
		free(buf->c_counter);
		free(buf->enc_counter);
		return -1;
}

int _mcrypt_set_state( CTR_BUFFER* buf, byte *IV, int size)
{
	buf->c_counter_pos = IV[0];
	memcpy(buf->c_counter, &IV[1], size-1);
	memcpy(buf->enc_counter, &IV[1], size-1);

	return 0;
}

int _mcrypt_get_state( CTR_BUFFER* buf, byte *IV, int *size)
{
	if (*size < buf->blocksize + 1) {
		*size = buf->blocksize + 1;
		return -1;
	}
	*size = buf->blocksize + 1;

	IV[0] = buf->c_counter_pos;
	memcpy( &IV[1], buf->c_counter, buf->blocksize);

	return 0;
}


void _end_mcrypt( CTR_BUFFER* buf) {
	free(buf->c_counter);
	free(buf->enc_counter);
}

inline static
void xor_stuff( CTR_BUFFER *buf, void* akey, void (*func)(void*,void*), byte* plain,  int blocksize, int xor_size) 
{
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;

	if (xor_size == blocksize) {
		if (buf->c_counter_pos == 0) {

			memcpy( buf->enc_counter, buf->c_counter, blocksize);
			_mcrypt_block_encrypt(akey, buf->enc_counter);

			memxor( plain, buf->enc_counter, blocksize);

			increase_counter( buf->c_counter, blocksize);


		} else {
			int size = blocksize - buf->c_counter_pos;

			memxor( plain, &buf->enc_counter[buf->c_counter_pos],
				size);
		
			increase_counter( buf->c_counter, blocksize);

			memcpy( buf->enc_counter, buf->c_counter, blocksize);
			_mcrypt_block_encrypt(akey, buf->enc_counter);

			memxor( &plain[size], buf->enc_counter,
				buf->c_counter_pos);

			/* buf->c_counter_pos remains the same */

		}
	} else { /* xor_size != blocksize */
		if (buf->c_counter_pos == 0) {
			memcpy( buf->enc_counter, buf->c_counter, blocksize);
			_mcrypt_block_encrypt(akey, buf->enc_counter);

			memxor( plain, buf->enc_counter, xor_size);
			buf->c_counter_pos = xor_size;
		} else {
			int size = blocksize - buf->c_counter_pos;
			int min_size =  size < xor_size ? size: xor_size;

			memxor( plain, &buf->enc_counter[buf->c_counter_pos],
				min_size); 

			buf->c_counter_pos += min_size;

			if (min_size >= xor_size)
				return;

			increase_counter( buf->c_counter, blocksize);

			memcpy( buf->enc_counter, buf->c_counter, blocksize);
			_mcrypt_block_encrypt(akey, buf->enc_counter);

			memxor( &plain[min_size], buf->enc_counter,
				xor_size - min_size);

			buf->c_counter_pos = xor_size - min_size;

		}
	
	}
	return;
}

int _mcrypt( CTR_BUFFER* buf,void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext can be any size */
	byte *plain;
	word32 *fplain = plaintext;
	int i, j=0;
	int modlen;

	plain = plaintext;
	for (j = 0; j < len / blocksize; j++) {

		xor_stuff( buf, akey, func, plain, blocksize, blocksize);

		plain += blocksize;

/* Put the new register */

	}
	modlen = len % blocksize;
	if (modlen > 0) {
		/* This is only usefull if encrypting the
		 * final block. Otherwise you'll not be
		 * able to decrypt it.
		 */

		xor_stuff( buf, akey, func, plain, blocksize, modlen);

	}
	
	return 0;
}


int _mdecrypt( CTR_BUFFER* buf,void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{				/* plaintext can be any size */
	return _mcrypt( buf, plaintext, len, blocksize, akey, func, func2);
}

int _has_iv() { return 1; }
int _is_block_mode() { return 0; }
int _is_block_algorithm_mode() { return 1; }
const char *_mcrypt_get_modes_name() { return "CTR";}
int _mcrypt_mode_get_size () {return sizeof(CTR_BUFFER);}


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
