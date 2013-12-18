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

/* EBC MODE */

#define _mcrypt_set_state ecb_LTX__mcrypt_set_state
#define _mcrypt_get_state ecb_LTX__mcrypt_get_state
#define _init_mcrypt ecb_LTX__init_mcrypt
#define _end_mcrypt ecb_LTX__end_mcrypt
#define _mcrypt ecb_LTX__mcrypt
#define _mdecrypt ecb_LTX__mdecrypt
#define _has_iv ecb_LTX__has_iv
#define _is_block_mode ecb_LTX__is_block_mode
#define _is_block_algorithm_mode ecb_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name ecb_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size ecb_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version ecb_LTX__mcrypt_mode_version

int _init_mcrypt( void* ign, void *key, int lenofkey, void *IV, int size)
{
	return 0;

}

int _mcrypt_set_state( void* buf, void *IV, int size) { return -1; }
int _mcrypt_get_state( void* buf, void *IV, int *size) { return -1; }

int _end_mcrypt (void* buf) {return 0;}

int _mcrypt( void* ign, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{
	int j;
	char *plain = plaintext;
	void (*_mcrypt_block_encrypt) (void *, void *);

	_mcrypt_block_encrypt = func;
	
	for (j = 0; j < len / blocksize; j++) {
		_mcrypt_block_encrypt(akey, &plain[j * blocksize]);
	}
	if (j==0 && len!=0) return -1; /* no blocks were encrypted */
	return 0;
}



int _mdecrypt( void* ign, void *ciphertext, int len, int blocksize, void* akey, void (*func)(void*,void*), void (*func2)(void*,void*))
{
	int j;
	char *cipher = ciphertext;
	void (*_mcrypt_block_decrypt) (void *, void *);

	_mcrypt_block_decrypt = func2;
	
	for (j = 0; j < len / blocksize; j++) {
		_mcrypt_block_decrypt(akey, &cipher[j * blocksize]);
	}
	if (j==0 && len!=0) return -1; /* no blocks were encrypted */
	return 0;
}

int _has_iv() { return 0; }
int _is_block_mode() { return 1; }
int _is_block_algorithm_mode() { return 1; }
const char *_mcrypt_get_modes_name() { return "ECB";}
int _mcrypt_mode_get_size () {return 0;}

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
