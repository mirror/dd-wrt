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

#ifndef LIBDEFS_H
#define LIBDEFS_H
#include <libdefs.h>
#endif
#include <bzero.h>
#include <mcrypt_internal.h>
#include <xmemory.h>

#ifndef DEBUG
# define fputs(x, y) 
#endif

extern const mcrypt_preloaded mps[];

#define MAX_MOD_SIZE 1024

static int mcrypt_strcmp( const char* str1, const char* str2) {
int i;
int len;
	if (strlen(str1)!=strlen(str2)) return -1;
	len = strlen(str1);

	for (i=0;i<len;i++) {
		if (str1[i]=='_' && str2[i]=='-') continue;
		if (str2[i]=='_' && str1[i]=='-') continue;
		if (str1[i]!=str2[i]) return -1;
	}
	
	return 0;
}

lt_ptr _mcrypt_search_symlist_lib(const char* name) {
int i=0;

	while( mps[i].name!=0 || mps[i].address!=0) {
		if (mps[i].name!=NULL && mps[i].address==NULL) {
			if (mcrypt_strcmp(name, mps[i].name)==0) {
				 return (void*) -1;
			}
		}
		i++;
	}
	return NULL;
}

lt_ptr _mcrypt_search_symlist_sym(mcrypt_dlhandle handle, const char* _name) {
int i=0;
char name[MAX_MOD_SIZE];

	strcpy(name, handle.name);
	
	strcat( name, "_LTX_");
	strcat( name, _name);
	
	while( mps[i].name!=0 || mps[i].address!=0) {
		if (mps[i].name!=NULL) {
			if (mcrypt_strcmp(name, mps[i].name)==0) {
				 return mps[i].address;
			}
		}
		i++;
	}
	return NULL;
}

void mcrypt_dlclose( mcrypt_dlhandle handle) {
void* mod = handle.handle;

	if (mod!=MCRYPT_INTERNAL_HANDLER && mod!=NULL)
		lt_dlclose(mod);
	handle.handle = NULL;
}

lt_ptr mcrypt_dlsym( mcrypt_dlhandle handle, char* str) {
void* mod;

	mod = handle.handle;
	
	if (mod!=MCRYPT_INTERNAL_HANDLER)
		return lt_dlsym(mod, str);
	else
		return _mcrypt_search_symlist_sym(handle, str);

}

WIN32DLL_DEFINE
int mcrypt_module_close(MCRYPT td)
{
	if (td==NULL) return MCRYPT_UNKNOWN_ERROR;
	
	mcrypt_dlclose(td->algorithm_handle);
	mcrypt_dlclose(td->mode_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	td->m_encrypt = NULL;
	td->a_encrypt = NULL;
	td->a_decrypt = NULL;
	td->m_decrypt = NULL;

	free(td);
	
	return 0;
}


WIN32DLL_DEFINE
void* mcrypt_dlopen ( mcrypt_dlhandle* handle, const char* a_directory, const char *m_directory, const char *filename) {
	
	char paths[1539];

	if (!filename || !(*filename)) {
		return MCRYPT_FAILED;
	}

	if (strlen(filename) >= sizeof( handle->name))
		return MCRYPT_FAILED;
	else
		strcpy( handle->name, filename);
			
	if (_mcrypt_search_symlist_lib(filename)!=NULL) {
		handle->handle = MCRYPT_INTERNAL_HANDLER;
		return handle->handle;
	}

	*paths = '\0';
	if (a_directory != NULL) {
		strncat( paths, a_directory, 512);
		strcat( paths, ":");
	}
	if (m_directory != NULL) {
		strncat( paths, m_directory, 512);
		strcat( paths, ":");
	}
	strncat( paths, LIBDIR, 512);

	lt_dlsetsearchpath(paths);

	handle->handle = lt_dlopenext(filename);

	return handle->handle;

}

WIN32DLL_DEFINE
MCRYPT mcrypt_module_open(const char *algorithm,
			  const char *a_directory, const char *mode, 
			  const char *m_directory)
{
	MCRYPT td;
	const char* err;
	void *ret;
	
	td = calloc(1, sizeof(CRYPT_STREAM));
	if (td==NULL) return MCRYPT_FAILED;

	if (lt_dlinit() != 0) {
		return MCRYPT_FAILED;
	}

	ret = mcrypt_dlopen( &td->algorithm_handle, a_directory, m_directory, algorithm);
	if (ret == NULL) {
		err=lt_dlerror();
		if (err!=NULL) {
			fputs( err, stderr);
			fputs("\n", stderr);
		}
		free(td);
		lt_dlexit();
		return MCRYPT_FAILED;
	}

	ret = mcrypt_dlopen( &td->mode_handle, a_directory, m_directory, mode);

	if (ret == NULL) {
		err=lt_dlerror();
		if (err!=NULL) {
			fputs( err, stderr);
			fputs( "\n", stderr);
		}
		mcrypt_dlclose(td->algorithm_handle);
		free(td);
		lt_dlexit();
		return MCRYPT_FAILED;
	}

	td->a_encrypt = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_encrypt");
	td->a_decrypt = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_decrypt");
	td->m_encrypt = mcrypt_dlsym(td->mode_handle, "_mcrypt");
	td->m_decrypt = mcrypt_dlsym(td->mode_handle, "_mdecrypt");
	td->a_block_size =
	    mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_block_size");

	if (td->a_encrypt == NULL || td->a_decrypt == NULL|| td->m_encrypt == NULL||
		td->m_decrypt == NULL|| td->a_block_size == NULL) {
		err=lt_dlerror();
		if (err!=NULL) {
			fputs( err, stderr);
			fputs( "\n", stderr);
		}
		mcrypt_dlclose(td->algorithm_handle);
		free(td);
		lt_dlexit();
		return MCRYPT_FAILED;
	}

	if (mcrypt_enc_is_block_algorithm_mode(td) !=
	    mcrypt_enc_is_block_algorithm(td)) {
		mcrypt_module_close(td);
		return MCRYPT_FAILED;
	}

	return td;
}



/* Modules' frontends */

WIN32DLL_DEFINE
int mcrypt_get_size(MCRYPT td)
{
	int (*_mcrypt_get_size) (void);

	const char *error;

	_mcrypt_get_size = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_size");
	if (_mcrypt_get_size == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return _mcrypt_get_size();
}

WIN32DLL_DEFINE
int mcrypt_mode_get_size(MCRYPT td)
{
	int (*_mcrypt_get_size) (void);

	const char *error;

	_mcrypt_get_size = mcrypt_dlsym(td->mode_handle, "_mcrypt_mode_get_size");
	if (_mcrypt_get_size == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return _mcrypt_get_size();
}

WIN32DLL_DEFINE
int mcrypt_set_key(MCRYPT td, void *a, const void *key, int keysize, const void *iv, int e)
{
	int (*__mcrypt_set_key_stream) (void *, const void *, int, const void *, int);
	int (*__mcrypt_set_key_block) (void *, const void *, int);
	const char *error;

	if (mcrypt_enc_is_block_algorithm(td) == 0) {
		/* stream */
		__mcrypt_set_key_stream = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_set_key");
		if (__mcrypt_set_key_stream == NULL) {
			error = lt_dlerror();
			if (error!=NULL) {
				fputs(error, stderr);
				fputs("\n", stderr);
			}
			return -2;
		}
		return __mcrypt_set_key_stream(a, key, keysize, iv, e);
	} else {
		__mcrypt_set_key_block = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_set_key");
		if (__mcrypt_set_key_block == NULL) {
		 	error = lt_dlerror();
			if (error!=NULL) {
				fputs(error, stderr);
				fputs("\n", stderr);
			}
			return -2;
		}
		return __mcrypt_set_key_block(a, key, keysize);
	}
}

WIN32DLL_DEFINE
int mcrypt_enc_set_state(MCRYPT td, const void *iv, int size)
{
	int (*__mcrypt_set_state) (void *, const void *, int);
	const char *error;

	__mcrypt_set_state = mcrypt_dlsym(td->mode_handle, "_mcrypt_set_state");
	if (__mcrypt_set_state==NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return __mcrypt_set_state(td->abuf, iv, size);
}

WIN32DLL_DEFINE
int mcrypt_enc_get_state(MCRYPT td, void *iv, int *size)
{
	int (*__mcrypt_get_state) (void *, void *, int*);
	const char *error;

	__mcrypt_get_state = mcrypt_dlsym(td->mode_handle, "_mcrypt_get_state");
	if (__mcrypt_get_state==NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return __mcrypt_get_state(td->abuf, iv, size);
}


WIN32DLL_DEFINE
int mcrypt_enc_get_block_size(MCRYPT td)
{
	int (*_mcrypt_get_block_size) (void);

	_mcrypt_get_block_size = td->a_block_size;
	return _mcrypt_get_block_size();
}

WIN32DLL_DEFINE
int mcrypt_get_algo_iv_size(MCRYPT td)
{
	const char* error;
	int (*_mcrypt_get_algo_iv_size) (void);

	_mcrypt_get_algo_iv_size = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_algo_iv_size");
	if (_mcrypt_get_algo_iv_size == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return _mcrypt_get_algo_iv_size();
}

WIN32DLL_DEFINE
int mcrypt_enc_get_iv_size(MCRYPT td)
{
	if (mcrypt_enc_is_block_algorithm_mode(td) == 1) {
		return mcrypt_enc_get_block_size(td);
	} else {
		return mcrypt_get_algo_iv_size(td);
	}
}

WIN32DLL_DEFINE
int mcrypt_enc_get_key_size(MCRYPT td)
{
	const char* error;
	int (*_mcrypt_get_key_size) (void);

	_mcrypt_get_key_size = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_key_size");
	if (_mcrypt_get_key_size == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}
	return _mcrypt_get_key_size();
}

WIN32DLL_DEFINE
int *mcrypt_enc_get_supported_key_sizes(MCRYPT td, int *len)
{
	int *(*_mcrypt_get_key_sizes) (int *);
	int *size, *ret;
	const char* error;

	_mcrypt_get_key_sizes =
	    mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_supported_key_sizes");
	if (_mcrypt_get_key_sizes == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		*len = 0;
		return NULL;
	}

	size = _mcrypt_get_key_sizes(len);
	
	ret = NULL;
	if (size!=NULL && (*len) != 0) {
		ret = malloc( sizeof(int)*(*len));
		if (ret==NULL) return NULL;
		memcpy( ret, size, sizeof(int)*(*len));
	}
	return ret;
}

WIN32DLL_DEFINE
int mcrypt_enc_is_block_algorithm(MCRYPT td)
{
	int (*_is_block_algorithm) (void);
	const char* error;

	_is_block_algorithm = mcrypt_dlsym(td->algorithm_handle, "_is_block_algorithm");
	if (_is_block_algorithm == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _is_block_algorithm();
}

WIN32DLL_DEFINE
char *mcrypt_enc_get_algorithms_name(MCRYPT td)
{
	const char *(*_mcrypt_get_algorithms_name) (void);
	const char* error;

	_mcrypt_get_algorithms_name =
	    mcrypt_dlsym(td->algorithm_handle, "_mcrypt_get_algorithms_name");
	if (_mcrypt_get_algorithms_name == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return NULL;
	}

	return strdup(_mcrypt_get_algorithms_name());
}

WIN32DLL_DEFINE
int init_mcrypt(MCRYPT td, void *buf, const void *key, int keysize, const void *iv)
{
	int (*_init_mcrypt) (void *, const void *, int, const void *, int);
	const char* error;

	_init_mcrypt = mcrypt_dlsym(td->mode_handle, "_init_mcrypt");
	if (_init_mcrypt == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _init_mcrypt(buf, key, keysize, iv, mcrypt_enc_get_block_size(td));
}

WIN32DLL_DEFINE
int end_mcrypt(MCRYPT td, void *buf)
{
	int (*_end_mcrypt) (void *);
	const char* error;

	_end_mcrypt = mcrypt_dlsym(td->mode_handle, "_end_mcrypt");
	if (_end_mcrypt == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _end_mcrypt(buf);
}

WIN32DLL_DEFINE
int mcrypt(MCRYPT td, void *buf, void *a, int b)
{
	int (*_mcrypt) (void *, void *, int, int, void *, void *, void*);

	_mcrypt = td->m_encrypt;

	return _mcrypt(buf, a, b, mcrypt_enc_get_block_size(td), td->akey,
		       td->a_encrypt, td->a_decrypt);
}

WIN32DLL_DEFINE
int mdecrypt(MCRYPT td, void *buf, void *a, int b)
{
	int (*_mdecrypt) (void *, void *, int, int, void *, void *, void*);

	_mdecrypt = td->m_decrypt;
	return _mdecrypt(buf, a, b, mcrypt_enc_get_block_size(td),
			 td->akey, td->a_encrypt, td->a_decrypt);
}

WIN32DLL_DEFINE
char *mcrypt_enc_get_modes_name(MCRYPT td)
{
	const char *(*_mcrypt_get_modes_name) (void);
	const char* error;

	_mcrypt_get_modes_name = mcrypt_dlsym(td->mode_handle, "_mcrypt_get_modes_name");
	if (_mcrypt_get_modes_name == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return NULL;
	}

	return strdup(_mcrypt_get_modes_name());
}

WIN32DLL_DEFINE
int mcrypt_enc_is_block_mode(MCRYPT td)
{
	int (*_is_block_mode) (void);
	const char* error;
	
	_is_block_mode = mcrypt_dlsym(td->mode_handle, "_is_block_mode");
	if (_is_block_mode == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _is_block_mode();
}

WIN32DLL_DEFINE
int mcrypt_enc_mode_has_iv(MCRYPT td)
{
	int (*_has_iv) (void);
	const char* error;

	_has_iv = mcrypt_dlsym(td->mode_handle, "_has_iv");
	if (_has_iv == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _has_iv();
}

WIN32DLL_DEFINE
int mcrypt_enc_is_block_algorithm_mode(MCRYPT td)
{
	int (*_is_a_block_mode) (void);
	const char* error;

	_is_a_block_mode = mcrypt_dlsym(td->mode_handle, "_is_block_algorithm_mode");
	if (_is_a_block_mode == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _is_a_block_mode();
}

WIN32DLL_DEFINE
int mcrypt_enc_self_test(MCRYPT td)
{
	int (*_self_test) (void);
	const char* error;

	_self_test = mcrypt_dlsym(td->algorithm_handle, "_mcrypt_self_test");
	if (_self_test == NULL) {
		error = lt_dlerror();
		if (error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		return MCRYPT_UNKNOWN_ERROR;
	}

	return _self_test();
}

WIN32DLL_DEFINE
int mcrypt_module_self_test(const char *algorithm, const char *a_directory)
{
	int i;
	void* rr;
	mcrypt_dlhandle _handle;
	int (*_self_test) (void);
	const char* error;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen(&_handle, a_directory, NULL, algorithm);

	if (!rr) {
		error=lt_dlerror();
		if (error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_self_test = mcrypt_dlsym(_handle, "_mcrypt_self_test");
	if (_self_test == NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}
	
	i = _self_test();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_algorithm_version(const char *algorithm, const char *a_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_version) (void);
	const char* error;
	void* rr;
	
	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen(&_handle, a_directory, NULL, algorithm);
	if (!rr) {
		error=lt_dlerror();
		if (error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_version = mcrypt_dlsym(_handle, "_mcrypt_algorithm_version");
	if (_version==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _version();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_mode_version(const char *mode, const char *m_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_version) (void);
	const char* error;
	void * rr;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, m_directory, NULL, mode);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs(error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_version = mcrypt_dlsym( _handle, "_mcrypt_mode_version");
	if (_version==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _version();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_is_block_algorithm(const char *algorithm, const char *a_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_is_block_algorithm) (void);
	const char* error;
	void * rr;
	
	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, a_directory, NULL, algorithm);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_is_block_algorithm = mcrypt_dlsym(_handle, "_is_block_algorithm");
	if (_is_block_algorithm==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _is_block_algorithm();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_is_block_algorithm_mode(const char *mode, const char *m_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_is_a_block_mode) (void);
	const char* error;
	void * rr;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, m_directory, NULL, mode);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_is_a_block_mode = mcrypt_dlsym(_handle, "_is_block_algorithm_mode");
	if (_is_a_block_mode==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _is_a_block_mode();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_is_block_mode(const char *mode, const char *m_directory)
{
	void * rr;
	int i;
	mcrypt_dlhandle _handle;
	int (*_is_block_mode) (void);
	const char* error;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, m_directory, NULL, mode);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_is_block_mode = mcrypt_dlsym(_handle, "_is_block_mode");
	if (_is_block_mode==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _is_block_mode();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_get_algo_block_size(const char *algorithm, const char *a_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_get_block_size) (void);
	const char* error;
	void * rr;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, a_directory, NULL, algorithm);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_get_block_size = mcrypt_dlsym(_handle, "_mcrypt_get_block_size");
	if (_get_block_size==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _get_block_size();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int mcrypt_module_get_algo_key_size(const char *algorithm, const char *a_directory)
{
	int i;
	mcrypt_dlhandle _handle;
	int (*_get_key_size) (void);
	const char* error;
	void * rr;

	if (lt_dlinit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	rr = mcrypt_dlopen( &_handle, a_directory, NULL, algorithm);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	_get_key_size = mcrypt_dlsym(_handle, "_mcrypt_get_key_size");
	if (_get_key_size==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		return MCRYPT_UNKNOWN_ERROR;
	}

	i = _get_key_size();

	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return MCRYPT_UNKNOWN_ERROR;
	}

	return i;
}

WIN32DLL_DEFINE
int *mcrypt_module_get_algo_supported_key_sizes(const char *algorithm,
						const char *a_directory,
						int *len)
{
	mcrypt_dlhandle _handle;
	int *(*_mcrypt_get_key_sizes) (int *);
	int *size;
	int * ret_size;
	const char* error;
	void * rr;

	
	if (lt_dlinit() != 0) {
		*len = 0;
		return NULL;
	}

	rr = mcrypt_dlopen( &_handle, a_directory, NULL, algorithm);
	if (!rr) {
		error=lt_dlerror();
		if(error!=NULL) {
			fputs( error, stderr);
			fputs("\n", stderr);
		}
		lt_dlexit();
		*len = 0;
		return NULL;
	}

	_mcrypt_get_key_sizes =
	    mcrypt_dlsym(_handle, "_mcrypt_get_supported_key_sizes");
	if (_mcrypt_get_key_sizes==NULL) {
		mcrypt_dlclose(_handle);
		lt_dlexit();
		*len = 0;
		return NULL;
	}

	ret_size = NULL;
	size = _mcrypt_get_key_sizes(len);
	if (*len!=0 && size!=NULL) {
		ret_size = malloc( (*len)*sizeof(int));
		if (ret_size!=NULL) {
			memcpy( ret_size, size, (*len)*sizeof(int));
		}
	} else *len = 0;
	
	mcrypt_dlclose(_handle);
	if (lt_dlexit() != 0) {
		return NULL;
	}

	return ret_size;
}

/* Returns false(0) if the library has not been compiled
 * with dynamic module support.
 */
int mcrypt_module_support_dynamic(void) {
#ifdef USE_LTDL
	return 1;
#else
	return 0;
#endif

}
