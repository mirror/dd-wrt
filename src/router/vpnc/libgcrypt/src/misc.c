/* misc.c
 *	Copyright (C) 1999, 2001, 2002, 2003 Free Software Foundation, Inc.
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
#include <assert.h>
#include <unistd.h>

#include "g10lib.h"

static int verbosity_level = 0;

static void (*fatal_error_handler)(void*,int, const char*) = NULL;
static void *fatal_error_handler_value = 0;
static void (*log_handler)(void*,int, const char*, va_list) = NULL;
static void *log_handler_value = 0;

static const char *(*user_gettext_handler)( const char * ) = NULL;

void
gcry_set_gettext_handler( const char *(*f)(const char*) )
{
    user_gettext_handler = f;
}


const char *
_gcry_gettext( const char *key )
{
    if( user_gettext_handler )
	return user_gettext_handler( key );
    /* FIXME: switch the domain to gnupg and restore later */
    return key;
}

void
gcry_set_fatalerror_handler( void (*fnc)(void*,int, const char*), void *value)
{
    fatal_error_handler_value = value;
    fatal_error_handler = fnc;
}

static void
write2stderr( const char *s )
{
    write( 2, s, strlen(s) );
}

/*
 * This function is called for fatal errors.  A caller might want to
 * set his own handler because this function simply calls abort().
 */
void
_gcry_fatal_error (int rc, const char *text)
{
  if ( !text ) /* get a default text */
    text = gpg_strerror (rc);

  if (fatal_error_handler)
    fatal_error_handler (fatal_error_handler_value, rc, text);

  write2stderr("\nFatal error: ");
  write2stderr(text);
  write2stderr("\n");
  abort ();
}

void
gcry_set_log_handler( void (*f)(void*,int, const char*, va_list ),
							    void *opaque )
{
    log_handler = f;
    log_handler_value = opaque;
}

void
_gcry_set_log_verbosity( int level )
{
    verbosity_level = level;
}

int
_gcry_log_verbosity( int level )
{
    return verbosity_level >= level;
}

/****************
 * This is our log function which prints all log messages to stderr or
 * using the function defined with gcry_set_log_handler().
 */
static void
_gcry_logv( int level, const char *fmt, va_list arg_ptr )
{
    if( log_handler )
	log_handler( log_handler_value, level, fmt, arg_ptr );
    else {
	switch ( level ) {
	  case GCRY_LOG_CONT: break;
	  case GCRY_LOG_INFO: break;
	  case GCRY_LOG_WARN: break;
	  case GCRY_LOG_ERROR: break;
	  case GCRY_LOG_FATAL: fputs("Fatal: ",stderr ); break;
	  case GCRY_LOG_BUG: fputs("Ohhhh jeeee: ", stderr); break;
	  case GCRY_LOG_DEBUG: fputs("DBG: ", stderr ); break;
	  default: fprintf(stderr,"[Unknown log level %d]: ", level ); break;
	}
	vfprintf(stderr,fmt,arg_ptr) ;
    }

    if( level == GCRY_LOG_FATAL )
	exit(2);
    else if( level == GCRY_LOG_BUG )
	abort();
}

void
_gcry_log( int level, const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( level, fmt, arg_ptr );
    va_end(arg_ptr);
}


#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5 )
void
_gcry_bug( const char *file, int line, const char *func )
{
    _gcry_log( GCRY_LOG_BUG,
	     ("... this is a bug (%s:%d:%s)\n"), file, line, func );
    abort(); /* never called, but it makes the compiler happy */
}
#else
void
_gcry_bug( const char *file, int line )
{
    _gcry_log( GCRY_LOG_BUG,
	     _("you found a bug ... (%s:%d)\n"), file, line);
    abort(); /* never called, but it makes the compiler happy */
}
#endif

void
_gcry_log_info( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_INFO, fmt, arg_ptr );
    va_end(arg_ptr);
}

void
_gcry_log_error( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_ERROR, fmt, arg_ptr );
    va_end(arg_ptr);
}


void
_gcry_log_fatal( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_FATAL, fmt, arg_ptr );
    va_end(arg_ptr);
    abort(); /* never called, but it makes the compiler happy */
}

void
_gcry_log_bug( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_BUG, fmt, arg_ptr );
    va_end(arg_ptr);
    abort(); /* never called, but it makes the compiler happy */
}

void
_gcry_log_debug( const char *fmt, ... )
{
    va_list arg_ptr ;

    va_start( arg_ptr, fmt ) ;
    _gcry_logv( GCRY_LOG_DEBUG, fmt, arg_ptr );
    va_end(arg_ptr);
}

void
_gcry_log_printf (const char *fmt, ...)
{
  va_list arg_ptr;
  
  if (fmt) 
    {
      va_start( arg_ptr, fmt ) ;
      _gcry_logv (GCRY_LOG_CONT, fmt, arg_ptr);
      va_end(arg_ptr);
    }
}

void
_gcry_burn_stack (int bytes)
{
    char buf[64];
    
    wipememory (buf, sizeof buf);
    bytes -= sizeof buf;
    if (bytes > 0)
        _gcry_burn_stack (bytes);
}
