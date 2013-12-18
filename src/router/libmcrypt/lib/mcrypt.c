/*
 * Copyright (C) 1998,1999,2000 Nikos Mavroyanopoulos
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

/* $Id: mcrypt.c,v 1.19 2002/07/06 10:18:18 nmav Exp $ */

/* Changed by Steve Underwood 1999/12/10 to allow an arbitrary number of 
 * streams of encryption. Currently the resulting code is probably not
 * thread safe, but as far as I could tell the previous code wasn't
 * either. This version has brute force locking in a lot of places, but
 * it has not been tested in a multi-threaded manner.
 * The key locking issue is that the table of encryption streams could
 * be moved when it is extended. Any address pre-calculated, or in
 * calculation at the time of the reallocation would be screwed.
 * This won't happen often, but requires lots of locks - PITA! 
 */

/* Changed again at 1999/12/15 to correct the thread safeness. Now it
 * seems to be thread safe. Brute force locking was removed and
 * locks per thread were introduced. 
 *						--nikos
 */

/* The above comments are too old! */

#ifndef LIBDEFS_H
#define LIBDEFS_H
#include <libdefs.h>
#endif
#include <bzero.h>
#include <xmemory.h>
#include <mcrypt_internal.h>

#if 0
static int preloaded_symbols = 0;
#endif

static int internal_end_mcrypt(MCRYPT td);

static int internal_init_mcrypt(MCRYPT td, const void *key, int lenofkey, const void *IV)
{
	int *sizes = NULL;
	int num_of_sizes, i, ok = 0;
	int key_size = mcrypt_enc_get_key_size(td);

	if (lenofkey > key_size || lenofkey==0) {
		return MCRYPT_KEY_LEN_ERROR;	/* error */	
	}
	
	sizes = mcrypt_enc_get_supported_key_sizes(td, &num_of_sizes);
	if (sizes != NULL) {
		for (i = 0; i < num_of_sizes; i++) {
			if (lenofkey == sizes[i]) {
				ok = 1;
				break;
			}
		}
	} else {		/* sizes==NULL */
		if (num_of_sizes == 0
		    && lenofkey <= mcrypt_enc_get_key_size(td))
			ok = 1;
	}


	if (ok == 0) { /* not supported key size */
		key_size = mcrypt_enc_get_key_size(td);
		if (sizes != NULL) {
			for (i = 0; i < num_of_sizes; i++) {
				if (lenofkey <= sizes[i]) {
					key_size = sizes[i];
					break;
				}
			}
		} else { /* well every key size is supported! */
			key_size = lenofkey;
		}
	} else {
		key_size = lenofkey;
	}
	free(sizes);

	td->keyword_given = mxcalloc(1, mcrypt_enc_get_key_size(td));
	if (td->keyword_given==NULL) return MCRYPT_MEMORY_ALLOCATION_ERROR; 
	
	memmove(td->keyword_given, key, lenofkey);
	i = mcrypt_get_size(td);
	td->akey = mxcalloc(1, i);
	if (td->akey==NULL) {
		free(td->keyword_given);
		return MCRYPT_MEMORY_ALLOCATION_ERROR;
	}
	i = mcrypt_mode_get_size(td);
	if (i > 0) {
		td->abuf = mxcalloc(1, i);
		if (td->abuf==NULL) {
			free(td->keyword_given);
			free(td->akey);
			return MCRYPT_MEMORY_ALLOCATION_ERROR;
		}
	}
	ok = init_mcrypt(td, td->abuf, key, key_size, IV);
	if (ok!=0) {
		free(td->keyword_given);
		free(td->akey);
		free(td->abuf);
		return MCRYPT_UNKNOWN_ERROR; /* algorithm error */
	}

	ok = mcrypt_set_key(td,
		       (void *) td->akey,
		       (void *) td->keyword_given,
		       key_size, IV, IV!=NULL ? mcrypt_enc_get_iv_size(td) : 0);

	if (ok!=0) {
		internal_end_mcrypt(td);
		return MCRYPT_UNKNOWN_ERROR; /* algorithm error */
	}

	return 0;
}

static int internal_end_mcrypt(MCRYPT td)
{
	mxfree(td->keyword_given, mcrypt_enc_get_key_size(td));
	td->keyword_given = NULL;

	mxfree(td->akey, mcrypt_get_size(td));
	td->akey = NULL;

	end_mcrypt(td, td->abuf);
	if (td->abuf!=NULL) mxfree(td->abuf, mcrypt_mode_get_size(td));
	td->abuf = NULL;

	return 0;
}

/* Generic - High level functions */

WIN32DLL_DEFINE
int mcrypt_generic_init(const MCRYPT td, const void *key, int lenofkey, const void *IV)
{
	return internal_init_mcrypt(td, key, lenofkey, IV);
}

WIN32DLL_DEFINE
int mcrypt_generic(MCRYPT td, void *plaintext, int len)
{
	int x;

	x = mcrypt(td, td->abuf, plaintext, len);
	return x;
}

WIN32DLL_DEFINE
int mdecrypt_generic(MCRYPT td, void *ciphertext, int len)
{
	int x;
	x = mdecrypt(td, td->abuf, ciphertext, len);
	return x;
}

WIN32DLL_DEFINE
int mcrypt_generic_end( MCRYPT td)
{
	if (td==NULL) return MCRYPT_UNKNOWN_ERROR;

	if (td->keyword_given!=NULL)
		internal_end_mcrypt(td);
	mcrypt_module_close(td);
	return 0;
}

WIN32DLL_DEFINE
int mcrypt_generic_deinit( MCRYPT td)
{
	if (td==NULL || td->keyword_given==NULL) return MCRYPT_UNKNOWN_ERROR;
	
	internal_end_mcrypt(td);
	return 0;
}

WIN32DLL_DEFINE
void mcrypt_perror(int err)
{

	switch (err) {
	case MCRYPT_UNKNOWN_ERROR:
		fprintf(stderr, "Unknown error.\n");
		break;
	case MCRYPT_ALGORITHM_MODE_INCOMPATIBILITY:
		fprintf(stderr,
			"Algorithm incompatible with this mode.\n");
		break;
	case MCRYPT_KEY_LEN_ERROR:
		fprintf(stderr, "Key length is not legal.\n");
		break;
	case MCRYPT_MEMORY_ALLOCATION_ERROR:
		fprintf(stderr, "Memory allocation failed.\n");
		break;
	case MCRYPT_UNKNOWN_MODE:
		fprintf(stderr, "Unknown mode.\n");
		break;
	case MCRYPT_UNKNOWN_ALGORITHM:
		fprintf(stderr, "Unknown algorithm.\n");
		break;

	}
	return;
}

WIN32DLL_DEFINE
const char* mcrypt_strerror(int err)
{

	switch (err) {
	case MCRYPT_UNKNOWN_ERROR:
		return "Unknown error.\n";
		break;
	case MCRYPT_ALGORITHM_MODE_INCOMPATIBILITY:
		return	"Algorithm incompatible with this mode.\n";
		break;
	case MCRYPT_KEY_LEN_ERROR:
		return "Key length is not legal.\n";
		break;
	case MCRYPT_MEMORY_ALLOCATION_ERROR:
		return "Memory allocation failed.\n";
		break;
	case MCRYPT_UNKNOWN_MODE:
		return "Unknown mode.\n";
		break;
	case MCRYPT_UNKNOWN_ALGORITHM:
		return "Unknown algorithm.\n";
		break;

	}
	return NULL;
}

WIN32DLL_DEFINE
void mcrypt_free(void *ptr)
{
	free(ptr);
}
