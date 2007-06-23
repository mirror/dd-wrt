/* global.c  -	global control functions
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
 *               2004, 2005, 2006  Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>

#include "g10lib.h"
#include "cipher.h"
#include "stdmem.h" /* our own memory allocator */
#include "secmem.h" /* our own secmem allocator */
#include "ath.h"

/****************
 * flag bits: 0 : general cipher debug
 *	      1 : general MPI debug
 */
static unsigned int debug_flags;

static gcry_handler_alloc_t alloc_func;
static gcry_handler_alloc_t alloc_secure_func;
static gcry_handler_secure_check_t is_secure_func;
static gcry_handler_realloc_t realloc_func;
static gcry_handler_free_t free_func;
static gcry_handler_no_mem_t outofcore_handler;

static void *outofcore_handler_value = NULL;
static int no_secure_memory = 0;
static int any_init_done;

/* This is our handmade constructor.  It gets called by any function
   likely to be called at startup.  The suggested way for an
   application to make sure that this has been called is by using
   gcry_check_version. */
static void
global_init (void)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;

  if (any_init_done)
    return;
  any_init_done = 1;

  err = ath_init ();
  if (! err)
    err = _gcry_cipher_init ();
  if (! err)
    err = _gcry_md_init ();
  if (! err)
    err = _gcry_pk_init ();
#if 0
  if (! err)
    err = _gcry_ac_init ();
#endif

  if (err)
    /* FIXME?  */
    BUG ();
}


static const char*
parse_version_number( const char *s, int *number )
{
    int val = 0;

    if( *s == '0' && isdigit(s[1]) )
	return NULL; /* leading zeros are not allowed */
    for ( ; isdigit(*s); s++ ) {
	val *= 10;
	val += *s - '0';
    }
    *number = val;
    return val < 0? NULL : s;
}


static const char *
parse_version_string( const char *s, int *major, int *minor, int *micro )
{
    s = parse_version_number( s, major );
    if( !s || *s != '.' )
	return NULL;
    s++;
    s = parse_version_number( s, minor );
    if( !s || *s != '.' )
	return NULL;
    s++;
    s = parse_version_number( s, micro );
    if( !s )
	return NULL;
    return s; /* patchlevel */
}

/****************
 * Check that the the version of the library is at minimum the requested one
 * and return the version string; return NULL if the condition is not
 * satisfied.  If a NULL is passed to this function, no check is done,
 * but the version string is simply returned.
 */
const char *
gcry_check_version( const char *req_version )
{
    const char *ver = VERSION;
    int my_major, my_minor, my_micro;
    int rq_major, rq_minor, rq_micro;
    const char *my_plvl, *rq_plvl;

    global_init ();
    if ( !req_version )
	return ver;

    my_plvl = parse_version_string( ver, &my_major, &my_minor, &my_micro );
    if ( !my_plvl )
	return NULL;  /* very strange our own version is bogus */
    rq_plvl = parse_version_string( req_version, &rq_major, &rq_minor,
								&rq_micro );
    if ( !rq_plvl )
	return NULL;  /* req version string is invalid */

    if ( my_major > rq_major
	|| (my_major == rq_major && my_minor > rq_minor)
	|| (my_major == rq_major && my_minor == rq_minor
				 && my_micro > rq_micro)
	|| (my_major == rq_major && my_minor == rq_minor
				 && my_micro == rq_micro
				 && strcmp( my_plvl, rq_plvl ) >= 0) ) {
	return ver;
    }
    return NULL;
}

gcry_error_t
gcry_control (enum gcry_ctl_cmds cmd, ...)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  static int init_finished = 0;
  va_list arg_ptr;
  
  va_start (arg_ptr, cmd);
  switch (cmd)
    {
    case GCRYCTL_ENABLE_M_GUARD:
      _gcry_private_enable_m_guard ();
      break;

    case GCRYCTL_ENABLE_QUICK_RANDOM:
      _gcry_quick_random_gen (1);
      break;

    case 51:  /* Should be GCRYCTL_FAKED_RANDOM_P but we want to sneak
                 that into the API for the sake of GnuPG 1.9 - there
                 we check the error code and print a warning message
                 if that call is not supported.  The literal number is
                 used there as well. */
      /* Return an error if the RNG is faked one (i.e. enabled by
         ENABLE_QUICK_RANDOM. */
      if (_gcry_random_is_faked ())
        err = GPG_ERR_GENERAL;
      break;


    case GCRYCTL_DUMP_RANDOM_STATS:
      _gcry_random_dump_stats ();
      break;

    case GCRYCTL_DUMP_MEMORY_STATS:
      /*m_print_stats("[fixme: prefix]");*/
      break;

    case GCRYCTL_DUMP_SECMEM_STATS:
      _gcry_secmem_dump_stats ();
      break;

    case GCRYCTL_DROP_PRIVS:
      global_init ();
      _gcry_secmem_init (0);
      break;

    case GCRYCTL_DISABLE_SECMEM:
      global_init ();
      no_secure_memory = 1;
      break;    

    case GCRYCTL_INIT_SECMEM:
      global_init ();
      _gcry_secmem_init (va_arg (arg_ptr, unsigned int));
      if ((_gcry_secmem_get_flags () & GCRY_SECMEM_FLAG_NOT_LOCKED))
        err = GPG_ERR_GENERAL;
      break;

    case GCRYCTL_TERM_SECMEM:
      global_init ();
      _gcry_secmem_term ();
      break;

    case GCRYCTL_DISABLE_SECMEM_WARN:
      _gcry_secmem_set_flags ((_gcry_secmem_get_flags ()
			       | GCRY_SECMEM_FLAG_NO_WARNING));
      break;

    case GCRYCTL_SUSPEND_SECMEM_WARN:
      _gcry_secmem_set_flags ((_gcry_secmem_get_flags ()
			       | GCRY_SECMEM_FLAG_SUSPEND_WARNING));
      break;

    case GCRYCTL_RESUME_SECMEM_WARN:
      _gcry_secmem_set_flags ((_gcry_secmem_get_flags ()
			       & ~GCRY_SECMEM_FLAG_SUSPEND_WARNING));
      break;

    case GCRYCTL_USE_SECURE_RNDPOOL:
      global_init ();
      _gcry_secure_random_alloc (); /* put random number into secure memory */
      break;

    case GCRYCTL_SET_RANDOM_SEED_FILE:
      _gcry_set_random_seed_file (va_arg (arg_ptr, const char *));
      break;

    case GCRYCTL_UPDATE_RANDOM_SEED_FILE:
      _gcry_update_random_seed_file ();
      break;

    case GCRYCTL_SET_VERBOSITY:
      _gcry_set_log_verbosity (va_arg (arg_ptr, int));
      break;

    case GCRYCTL_SET_DEBUG_FLAGS:
      debug_flags |= va_arg (arg_ptr, unsigned int);
      break;

    case GCRYCTL_CLEAR_DEBUG_FLAGS:
      debug_flags &= ~va_arg (arg_ptr, unsigned int);
      break;

    case GCRYCTL_DISABLE_INTERNAL_LOCKING:
      global_init ();
      break;

    case GCRYCTL_ANY_INITIALIZATION_P:
      if (any_init_done)
	err = GPG_ERR_GENERAL;
      break;

    case GCRYCTL_INITIALIZATION_FINISHED_P:
      if (init_finished)
	err = GPG_ERR_GENERAL;
      break;

    case GCRYCTL_INITIALIZATION_FINISHED:
      /* This is a hook which should be used by an application after
	 all initialization has been done and right before any threads
	 are started.  It is not really needed but the only way to be
	 really sure that all initialization for thread-safety has
	 been done. */
        if (! init_finished)
	  {
            global_init ();
            /* Do only a basic random initialization, i.e. init the
               mutexes. */
            _gcry_random_initialize (0);
            init_finished = 1;
	  }
        break;

    case GCRYCTL_SET_THREAD_CBS:
      err = ath_install (va_arg (arg_ptr, void *), any_init_done);
      break;

    case GCRYCTL_FAST_POLL:
      /* We need to do make sure that the random pool is really
         initialized so that the poll fucntion is not a NOP. */
      _gcry_random_initialize (1);
      _gcry_fast_random_poll (); 
      break;

    default:
      err = GPG_ERR_INV_OP;
    }

  va_end(arg_ptr);
  return gcry_error (err);
}

/* Return a pointer to a string containing a description of the error
   code in the error value ERR.  */
const char *
gcry_strerror (gcry_error_t err)
{
  return gpg_strerror (err);
}

/* Return a pointer to a string containing a description of the error
   source in the error value ERR.  */
const char *
gcry_strsource (gcry_error_t err)
{
  return gpg_strsource (err);
}

/* Retrieve the error code for the system error ERR.  This returns
   GPG_ERR_UNKNOWN_ERRNO if the system error is not mapped (report
   this).  */
gcry_err_code_t
gcry_err_code_from_errno (int err)
{
  return gpg_err_code_from_errno (err);
}


/* Retrieve the system error for the error code CODE.  This returns 0
   if CODE is not a system error code.  */
int
gcry_err_code_to_errno (gcry_err_code_t code)
{
  return gpg_err_code_from_errno (code);
}

  
/* Return an error value with the error source SOURCE and the system
   error ERR.  */
gcry_error_t
gcry_err_make_from_errno (gpg_err_source_t source, int err)
{
  return gpg_err_make_from_errno (source, err);
}


/* Return an error value with the system error ERR.  */
gcry_err_code_t
gcry_error_from_errno (int err)
{
  return gcry_error (gpg_err_code_from_errno (err));
}

/****************
 * NOTE: All 5 functions should be set.  */
void
gcry_set_allocation_handler (gcry_handler_alloc_t new_alloc_func,
			     gcry_handler_alloc_t new_alloc_secure_func,
			     gcry_handler_secure_check_t new_is_secure_func,
			     gcry_handler_realloc_t new_realloc_func,
			     gcry_handler_free_t new_free_func)
{
  global_init ();

  alloc_func = new_alloc_func;
  alloc_secure_func = new_alloc_secure_func;
  is_secure_func = new_is_secure_func;
  realloc_func = new_realloc_func;
  free_func = new_free_func;
}



/****************
 * Set an optional handler which is called in case the xmalloc functions
 * ran out of memory.  This handler may do one of these things:
 *   o free some memory and return true, so that the xmalloc function
 *     tries again.
 *   o Do whatever it like and return false, so that the xmalloc functions
 *     use the default fatal error handler.
 *   o Terminate the program and don't return.
 *
 * The handler function is called with 3 arguments:  The opaque value set with
 * this function, the requested memory size, and a flag with these bits
 * currently defined:
 *	bit 0 set = secure memory has been requested.
 */
void
gcry_set_outofcore_handler( int (*f)( void*, size_t, unsigned int ),
							void *value )
{
    global_init ();

    outofcore_handler = f;
    outofcore_handler_value = value;
}

gcry_err_code_t
_gcry_malloc (size_t n, unsigned int flags, void **mem)
{
  gcry_err_code_t err = 0;
  void *m;

  if ((flags & GCRY_ALLOC_FLAG_SECURE) && !no_secure_memory)
    {
      if (alloc_secure_func)
	m = (*alloc_secure_func) (n);
      else
	m = _gcry_private_malloc_secure (n);
    }
  else
    {
      if (alloc_func)
	m = (*alloc_func) (n);
      else
	m = _gcry_private_malloc (n);
    }

  if (!m)
    {
      if (!errno)
        errno = ENOMEM;
      err = gpg_err_code_from_errno (errno);
    }
  else
    *mem = m;

  return err;
}
  
void *
gcry_malloc (size_t n)
{
  void *mem = NULL;

  _gcry_malloc (n, 0, &mem);

  return mem;
}

void *
gcry_malloc_secure (size_t n)
{
  void *mem = NULL;

  _gcry_malloc (n, GCRY_ALLOC_FLAG_SECURE, &mem);

  return mem;
}

int
gcry_is_secure (const void *a)
{
  if (no_secure_memory)
    return 0;
  if (is_secure_func)
    return is_secure_func (a) ;
  return _gcry_private_is_secure (a);
}

void
_gcry_check_heap( const void *a )
{
    /* FIXME: implement this*/
#if 0
    if( some_handler )
	some_handler(a)
    else
	_gcry_private_check_heap(a)
#endif
}

void *
gcry_realloc (void *a, size_t n)
{
  void *p;

  if (realloc_func)
    p = realloc_func (a, n);
  else
    p = _gcry_private_realloc (a, n);
  if (!p && !errno)
    errno = ENOMEM;
  return p;
}

void
gcry_free( void *p )
{
  if( !p )
    return;

  if (free_func)
    free_func (p);
  else
    _gcry_private_free (p);
}

void *
gcry_calloc (size_t n, size_t m)
{
  size_t bytes;
  void *p;

  bytes = n * m; /* size_t is unsigned so the behavior on overflow is
                    defined. */
  if (m && bytes / m != n) 
    {
      errno = ENOMEM;
      return NULL;
    }

  p = gcry_malloc (bytes);
  if (p)
    memset (p, 0, bytes);
  return p;
}

void *
gcry_calloc_secure (size_t n, size_t m)
{
  size_t bytes;
  void *p;

  bytes = n * m; /* size_t is unsigned so the behavior on overflow is
                    defined. */
  if (m && bytes / m != n) 
    {
      errno = ENOMEM;
      return NULL;
    }
  
  p = gcry_malloc_secure (bytes);
  if (p)
    memset (p, 0, bytes);
  return p;
}


/* Create and return a copy of the null-terminated string STRING.  If
   it is contained in secure memory, the copy will be contained in
   secure memory as well.  In an out-of-memory condition, NULL is
   returned.  */
char *
gcry_strdup (const char *string)
{
  char *string_cp = NULL;
  size_t string_n = 0;

  string_n = strlen (string);

  if (gcry_is_secure (string))
    string_cp = gcry_malloc_secure (string_n + 1);
  else
    string_cp = gcry_malloc (string_n + 1);
  
  if (string_cp)
    strcpy (string_cp, string);

  return string_cp;
}


void *
gcry_xmalloc( size_t n )
{
    void *p;

    while ( !(p = gcry_malloc( n )) ) {
	if( !outofcore_handler
	    || !outofcore_handler( outofcore_handler_value, n, 0 ) ) {
	    _gcry_fatal_error(gpg_err_code_from_errno (errno), NULL );
	}
    }
    return p;
}

void *
gcry_xrealloc( void *a, size_t n )
{
    void *p;

    while ( !(p = gcry_realloc( a, n )) ) {
	if( !outofcore_handler
	    || !outofcore_handler( outofcore_handler_value, n, 
                                   gcry_is_secure(a)? 3:2 ) ) {
	    _gcry_fatal_error(gpg_err_code_from_errno (errno), NULL );
	}
    }
    return p;
}

void *
gcry_xmalloc_secure( size_t n )
{
    void *p;

    while ( !(p = gcry_malloc_secure( n )) ) {
	if( !outofcore_handler
	    || !outofcore_handler( outofcore_handler_value, n, 1 ) ) {
	    _gcry_fatal_error(gpg_err_code_from_errno (errno),
			     _("out of core in secure memory"));
	}
    }
    return p;
}

void *
gcry_xcalloc( size_t n, size_t m )
{
  size_t nbytes;
  void *p;

  nbytes = n * m; 
  if (m && nbytes / m != n) 
    {
      errno = ENOMEM;
      _gcry_fatal_error(gpg_err_code_from_errno (errno), NULL );
    }

  p = gcry_xmalloc ( nbytes );
  memset ( p, 0, nbytes );
  return p;
}

void *
gcry_xcalloc_secure( size_t n, size_t m )
{
  size_t nbytes;
  void *p;

  nbytes = n * m; 
  if (m && nbytes / m != n) 
    {
      errno = ENOMEM;
      _gcry_fatal_error(gpg_err_code_from_errno (errno), NULL );
    }

  p = gcry_xmalloc_secure ( nbytes );
  memset ( p, 0, nbytes );
  return p;
}

char *
gcry_xstrdup (const char *string)
{
  char *p;

  while ( !(p = gcry_strdup (string)) ) 
    {
      size_t n = strlen (string);
      int is_sec = !!gcry_is_secure (string);

      if (!outofcore_handler
          || !outofcore_handler (outofcore_handler_value, n, is_sec) ) 
        {
          _gcry_fatal_error (gpg_err_code_from_errno (errno),
                             is_sec? _("out of core in secure memory"):NULL);
	}
    }

  return p;
}


int
_gcry_get_debug_flag( unsigned int mask )
{
    return debug_flags & mask;
}



/* It is often useful to get some feedback of long running operations.
   This function may be used to register a handler for this. 
   The callback function CB is used as:

   void cb (void *opaque, const char *what, int printchar,
           int current, int total);

   Where WHAT is a string identifying the the type of the progress
   output, PRINTCHAR the character usually printed, CURRENT the amount
   of progress currently done and TOTAL the expected amount of
   progress.  A value of 0 for TOTAL indicates that there is no
   estimation available.

   Defined values for WHAT:

   "need_entropy"  X    0  number-of-bytes-required
            When running low on entropy
   "primegen"      '\n'  0 0
           Prime generated
                   '!'
           Need to refresh the prime pool
                   '<','>'
           Number of bits adjusted
                   '^'
           Looking for a generator
                   '.'
           Fermat tests on 10 candidates failed
                  ':'
           Restart with a new random value
                  '+'
           Rabin Miller test passed          
   "pk_elg"        '+','-','.','\n'   0  0
            Only used in debugging mode.
   "pk_dsa"       
            Only used in debugging mode.
*/
void
gcry_set_progress_handler (void (*cb)(void *,const char*,int, int, int),
                           void *cb_data)
{
#if USE_DSA
  _gcry_register_pk_dsa_progress (cb, cb_data);
#endif
#if USE_ELGAMAL
  _gcry_register_pk_elg_progress (cb, cb_data);
#endif
  _gcry_register_primegen_progress (cb, cb_data);
  _gcry_register_random_progress (cb, cb_data);
}
