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

#define _init_mcrypt stream_LTX__init_mcrypt
#define _mcrypt_set_state stream_LTX__mcrypt_set_state
#define _mcrypt_get_state stream_LTX__mcrypt_get_state
#define _end_mcrypt stream_LTX__end_mcrypt
#define _mcrypt stream_LTX__mcrypt
#define _mdecrypt stream_LTX__mdecrypt
#define _has_iv stream_LTX__has_iv
#define _is_block_mode stream_LTX__is_block_mode
#define _is_block_algorithm_mode stream_LTX__is_block_algorithm_mode
#define _mcrypt_get_modes_name stream_LTX__mcrypt_get_modes_name
#define _mcrypt_mode_get_size stream_LTX__mcrypt_mode_get_size
#define _mcrypt_mode_version stream_LTX__mcrypt_mode_version

/* STREAM MODE */

int _init_mcrypt( void* ign, void *key, int lenofkey, void *IV, int size) { return 0; }

int _mcrypt_set_state( void* buf, void *IV, int size) { return -1; }
int _mcrypt_get_state( void* buf, void *IV, int *size) { return -1; }

int _end_mcrypt(void* ign) {return 0;}

int _mcrypt( void* ign, void *plaintext, int len, int blocksize, void* akey, void (*func)(void*,void*, int), void (*func2)(void*,void*, int))
{
	void (*_mcrypt_stream_encrypt) (void *, void *, int);

	_mcrypt_stream_encrypt = func;

	_mcrypt_stream_encrypt(akey, plaintext, len);
	return 0;
}



int _mdecrypt( void* ign, void *ciphertext, int len, int blocksize, void* akey, void (*func)(void*,void*, int), void (*func2)(void*,void*, int))
{
	void (*_mcrypt_stream_decrypt) (void *, void *, int);

	_mcrypt_stream_decrypt = func2;

	_mcrypt_stream_decrypt(akey, ciphertext, len);
	return 0;
}

int _has_iv() { return 1; }
int _is_block_mode() { return 0; }
int _is_block_algorithm_mode() { return 0; }
const char *_mcrypt_get_modes_name() { return "STREAM";}
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
