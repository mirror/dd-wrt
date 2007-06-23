/* mpicoder.c  -  Coder for the external representation of MPIs
 * Copyright (C) 1998, 1999, 2000, 2001, 2002,
 *               2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "mpi-internal.h"
#include "g10lib.h"

#define MAX_EXTERN_MPI_BITS 16384


static gcry_mpi_t
mpi_read_from_buffer (const unsigned char *buffer, unsigned int *ret_nread,
                      int secure)
{
  int i, j;
  unsigned int nbits, nbytes, nlimbs, nread=0;
  mpi_limb_t a;
  gcry_mpi_t val = MPI_NULL;

  if (*ret_nread < 2)
    goto leave;
  nbits = buffer[0] << 8 | buffer[1];
  if (nbits > MAX_EXTERN_MPI_BITS) 
    {
      log_error ("mpi too large (%u bits)\n", nbits);
      goto leave;
    }
  else if (!nbits) 
    {
      log_error ("an mpi of size 0 is not allowed\n");
      goto leave;
    }
  buffer += 2;
  nread = 2;
  
  nbytes = (nbits+7) / 8;
  nlimbs = (nbytes+BYTES_PER_MPI_LIMB-1) / BYTES_PER_MPI_LIMB;
  val = secure? mpi_alloc_secure( nlimbs )
    : mpi_alloc( nlimbs );
  i = BYTES_PER_MPI_LIMB - nbytes % BYTES_PER_MPI_LIMB;
  i %= BYTES_PER_MPI_LIMB;
  j= val->nlimbs = nlimbs;
  val->sign = 0;
  for( ; j > 0; j-- ) 
    {
      a = 0;
      for (; i < BYTES_PER_MPI_LIMB; i++ )
        {
          if ( ++nread > *ret_nread )
            log_bug ("mpi larger than buffer");
          a <<= 8;
          a |= *buffer++;
        }
      i = 0;
      val->d[j-1] = a;
    }

 leave:
  *ret_nread = nread;
  return val;
}


/****************
 * Make an mpi from a hex character string.
 */
static int
mpi_fromstr(gcry_mpi_t val, const char *str)
{
    int sign=0, prepend_zero=0, i, j, c, c1, c2;
    unsigned nbits, nbytes, nlimbs;
    mpi_limb_t a;

    if( *str == '-' ) {
	sign = 1;
	str++;
    }

    /* skip optional hex prefix */
    if ( *str == '0' && str[1] == 'x' ) {
        str += 2;
    }

    nbits = strlen(str)*4;
    if( nbits % 8 )
	prepend_zero = 1;
    nbytes = (nbits+7) / 8;
    nlimbs = (nbytes+BYTES_PER_MPI_LIMB-1) / BYTES_PER_MPI_LIMB;
    if( val->alloced < nlimbs )
	mpi_resize(val, nlimbs );
    i = BYTES_PER_MPI_LIMB - nbytes % BYTES_PER_MPI_LIMB;
    i %= BYTES_PER_MPI_LIMB;
    j= val->nlimbs = nlimbs;
    val->sign = sign;
    for( ; j > 0; j-- ) {
	a = 0;
	for(; i < BYTES_PER_MPI_LIMB; i++ ) {
	    if( prepend_zero ) {
		c1 = '0';
		prepend_zero = 0;
	    }
	    else
		c1 = *str++;
	    assert(c1);
	    c2 = *str++;
	    assert(c2);
	    if( c1 >= '0' && c1 <= '9' )
		c = c1 - '0';
	    else if( c1 >= 'a' && c1 <= 'f' )
		c = c1 - 'a' + 10;
	    else if( c1 >= 'A' && c1 <= 'F' )
		c = c1 - 'A' + 10;
	    else {
		mpi_clear(val);
		return 1;
	    }
	    c <<= 4;
	    if( c2 >= '0' && c2 <= '9' )
		c |= c2 - '0';
	    else if( c2 >= 'a' && c2 <= 'f' )
		c |= c2 - 'a' + 10;
	    else if( c2 >= 'A' && c2 <= 'F' )
		c |= c2 - 'A' + 10;
	    else {
		mpi_clear(val);
		return 1;
	    }
	    a <<= 8;
	    a |= c;
	}
	i = 0;
	val->d[j-1] = a;
    }

    return 0;
}


/* Dump the value of A in a format suitable for debugging to
   Libgcrypt's logging stream.  Note that one leading space but no
   trailing space or linefeed will be printed.  It is okay to pass
   NULL for A. */
void 
gcry_mpi_dump (const gcry_mpi_t a)
{
  int i;

  log_printf (" ");
  if (!a)
    log_printf ("[MPI_NULL]");
  else 
    {
      if (a->sign)
        log_printf ( "-");
#if BYTES_PER_MPI_LIMB == 2
# define X "4"
#elif BYTES_PER_MPI_LIMB == 4
# define X "8"
#elif BYTES_PER_MPI_LIMB == 8
# define X "16"
#elif BYTES_PER_MPI_LIMB == 16
# define X "32"
#else
# error please define the format here
#endif
      for (i=a->nlimbs; i > 0 ; i-- )
        {
          log_printf (i != a->nlimbs? "%0" X "lX":"%lX", (ulong)a->d[i-1]);
        }
#undef X
      if (!a->nlimbs)
        log_printf ("0");
    }
}

/* Convience function used internally. */
void
_gcry_log_mpidump (const char *text, gcry_mpi_t a)
{
  log_printf ("%s:", text);
  gcry_mpi_dump (a);
  log_printf ("\n");
}


/****************
 * Return an m_alloced buffer with the MPI (msb first).
 * NBYTES receives the length of this buffer. Caller must free the
 * return string (This function does return a 0 byte buffer with NBYTES
 * set to zero if the value of A is zero. If sign is not NULL, it will
 * be set to the sign of the A.
 */
static byte *
do_get_buffer( gcry_mpi_t a, unsigned *nbytes, int *sign, int force_secure )
{
    byte *p, *buffer;
    mpi_limb_t alimb;
    int i;
    size_t n;

    if( sign )
	*sign = a->sign;
    *nbytes = a->nlimbs * BYTES_PER_MPI_LIMB;
    n = *nbytes? *nbytes:1; /* allocate at least one byte */
    p = buffer = force_secure || mpi_is_secure(a) ? gcry_xmalloc_secure(n)
						  : gcry_xmalloc(n);

    for(i=a->nlimbs-1; i >= 0; i-- ) {
	alimb = a->d[i];
#if BYTES_PER_MPI_LIMB == 4
	*p++ = alimb >> 24;
	*p++ = alimb >> 16;
	*p++ = alimb >>  8;
	*p++ = alimb	  ;
#elif BYTES_PER_MPI_LIMB == 8
	*p++ = alimb >> 56;
	*p++ = alimb >> 48;
	*p++ = alimb >> 40;
	*p++ = alimb >> 32;
	*p++ = alimb >> 24;
	*p++ = alimb >> 16;
	*p++ = alimb >>  8;
	*p++ = alimb	  ;
#else
	#error please implement for this limb size.
#endif
    }

    /* this is sub-optimal but we need to do the shift oepration because
     * the caller has to free the returned buffer */
    for(p=buffer; !*p && *nbytes; p++, --*nbytes )
	;
    if( p != buffer )
	memmove(buffer,p, *nbytes);
    return buffer;
}


byte *
_gcry_mpi_get_buffer( gcry_mpi_t a, unsigned *nbytes, int *sign )
{
    return do_get_buffer( a, nbytes, sign, 0 );
}

byte *
_gcry_mpi_get_secure_buffer( gcry_mpi_t a, unsigned *nbytes, int *sign )
{
    return do_get_buffer( a, nbytes, sign, 1 );
}

/****************
 * Use BUFFER to update MPI.
 */
void
_gcry_mpi_set_buffer( gcry_mpi_t a, const byte *buffer, unsigned nbytes, int sign )
{
    const byte *p;
    mpi_limb_t alimb;
    int nlimbs;
    int i;

    nlimbs = (nbytes + BYTES_PER_MPI_LIMB - 1) / BYTES_PER_MPI_LIMB;
    RESIZE_IF_NEEDED(a, nlimbs);
    a->sign = sign;

    for(i=0, p = buffer+nbytes-1; p >= buffer+BYTES_PER_MPI_LIMB; ) {
#if BYTES_PER_MPI_LIMB == 4
	alimb  = *p--	    ;
	alimb |= *p-- <<  8 ;
	alimb |= *p-- << 16 ;
	alimb |= *p-- << 24 ;
#elif BYTES_PER_MPI_LIMB == 8
	alimb  = (mpi_limb_t)*p--	;
	alimb |= (mpi_limb_t)*p-- <<  8 ;
	alimb |= (mpi_limb_t)*p-- << 16 ;
	alimb |= (mpi_limb_t)*p-- << 24 ;
	alimb |= (mpi_limb_t)*p-- << 32 ;
	alimb |= (mpi_limb_t)*p-- << 40 ;
	alimb |= (mpi_limb_t)*p-- << 48 ;
	alimb |= (mpi_limb_t)*p-- << 56 ;
#else
	#error please implement for this limb size.
#endif
	a->d[i++] = alimb;
    }
    if( p >= buffer ) {
#if BYTES_PER_MPI_LIMB == 4
	alimb  = *p--	    ;
	if( p >= buffer ) alimb |= *p-- <<  8 ;
	if( p >= buffer ) alimb |= *p-- << 16 ;
	if( p >= buffer ) alimb |= *p-- << 24 ;
#elif BYTES_PER_MPI_LIMB == 8
	alimb  = (mpi_limb_t)*p-- ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- <<	8 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 16 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 24 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 32 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 40 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 48 ;
	if( p >= buffer ) alimb |= (mpi_limb_t)*p-- << 56 ;
#else
	#error please implement for this limb size.
#endif
	a->d[i++] = alimb;
    }
    a->nlimbs = i;
    assert( i == nlimbs );
}



/* Convert the external representation of an integer stored in BUFFER
   with a length of BUFLEN into a newly create MPI returned in
   RET_MPI.  If NBYTES is not NULL, it will receive the number of
   bytes actually scanned after a successful operation. */
gcry_error_t
gcry_mpi_scan( struct gcry_mpi **ret_mpi, enum gcry_mpi_format format,
	       const void *buffer_arg, size_t buflen, size_t *nscanned )
{
    const unsigned char *buffer = (const unsigned char*)buffer_arg;
    struct gcry_mpi *a = NULL;
    unsigned int len;
    int secure = (buffer && gcry_is_secure (buffer));

    if (format == GCRYMPI_FMT_SSH)
      len = 0;
    else
      len = buflen;

    if( format == GCRYMPI_FMT_STD ) {
	const byte *s = buffer;

	a = secure? mpi_alloc_secure ((len+BYTES_PER_MPI_LIMB-1)
                                      /BYTES_PER_MPI_LIMB)
                  : mpi_alloc ((len+BYTES_PER_MPI_LIMB-1)/BYTES_PER_MPI_LIMB);
	if( len ) { /* not zero */
	    a->sign = *s & 0x80;
	    if( a->sign ) {
		/* FIXME: we have to convert from 2compl to magnitude format */
		mpi_free(a);
		return gcry_error (GPG_ERR_INTERNAL);
	    }
	    else
		_gcry_mpi_set_buffer( a, s, len, 0 );
	}
	if( ret_mpi ) {
	    mpi_normalize ( a );
	    *ret_mpi = a;
	}
	else
	    mpi_free(a);
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_USG ) {
	a = secure? mpi_alloc_secure ((len+BYTES_PER_MPI_LIMB-1)
                                      /BYTES_PER_MPI_LIMB)
                  : mpi_alloc ((len+BYTES_PER_MPI_LIMB-1)/BYTES_PER_MPI_LIMB);

	if( len )  /* not zero */
	    _gcry_mpi_set_buffer( a, buffer, len, 0 );
	if( ret_mpi ) {
	    mpi_normalize ( a );
	    *ret_mpi = a;
	}
	else
	    mpi_free(a);
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_PGP ) {
	a = mpi_read_from_buffer (buffer, &len, secure);
	if( nscanned )
	    *nscanned = len;
	if( ret_mpi && a ) {
	    mpi_normalize ( a );
	    *ret_mpi = a;
	}
	else
	    mpi_free(a);
	return gcry_error (a ? GPG_ERR_NO_ERROR : GPG_ERR_INV_OBJ);
    }
    else if( format == GCRYMPI_FMT_SSH ) {
	const unsigned char *s = buffer;
	size_t n;

	if( len && len < 4 )
	    return gcry_error (GPG_ERR_TOO_SHORT);
	n = s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
	s += 4; 
        if (len)
          len -= 4;
	if( len && n > len )
	    return gcry_error (GPG_ERR_TOO_LARGE); /* or should it be
                                                      too_short */

	a = secure? mpi_alloc_secure ((n+BYTES_PER_MPI_LIMB-1)
                                      /BYTES_PER_MPI_LIMB)
                  : mpi_alloc ((n+BYTES_PER_MPI_LIMB-1)/BYTES_PER_MPI_LIMB);
	if( n ) { /* not zero */
	    a->sign = *s & 0x80;
	    if( a->sign ) {
		/* FIXME: we have to convert from 2compl to magnitude format */
		mpi_free(a);
		return gcry_error (GPG_ERR_INTERNAL);
	    }
	    else
		_gcry_mpi_set_buffer( a, s, n, 0 );
	}
	if( nscanned )
	    *nscanned = n+4;
	if( ret_mpi ) {
	    mpi_normalize ( a );
	    *ret_mpi = a;
	}
	else
	    mpi_free(a);
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_HEX ) {
	if( buflen )
	    return gcry_error (GPG_ERR_INV_ARG); /* can only handle C
                                                    strings for now */
	a = secure? mpi_alloc_secure (0) : mpi_alloc(0);
	if( mpi_fromstr ( a, (const char *)buffer ) )
	    return gcry_error (GPG_ERR_INV_OBJ);
	if( ret_mpi ) {
	    mpi_normalize ( a );
	    *ret_mpi = a;
	}
	else
	    mpi_free(a);
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else
	return gcry_error (GPG_ERR_INV_ARG);
}

/* Convert the big integer A into the external representation
   described by FORMAT and store it in the provided BUFFER which has
   been allocated by the user with a size of BUFLEN bytes.  NWRITTEN
   receives the actual length of the external representation unless it
   has been passed as NULL.  BUFFER may be NULL to query the required
   length.*/
gcry_error_t
gcry_mpi_print( enum gcry_mpi_format format,
                unsigned char *buffer, size_t buflen,
                size_t *nwritten, struct gcry_mpi *a)
{
    unsigned int nbits = mpi_get_nbits(a);
    size_t len;
    size_t dummy_nwritten;

    if (!nwritten)
      nwritten = &dummy_nwritten;

    len = buflen;
    *nwritten = 0;
    if( format == GCRYMPI_FMT_STD ) {
	unsigned char *tmp;
	int extra = 0;
	unsigned int n;

	if( a->sign )
	    return gcry_error (GPG_ERR_INTERNAL); /* can't handle it yet */

	tmp = _gcry_mpi_get_buffer( a, &n, NULL );
	if( n && (*tmp & 0x80) ) {
	    n++;
	    extra=1;
	}

	if (buffer && n > len) {
            /* The provided buffer is too short. */
	    gcry_free(tmp);
	    return gcry_error (GPG_ERR_TOO_SHORT);  
	}
	if( buffer ) {
	    byte *s = buffer;
	    if( extra )
		*s++ = 0;

	    memcpy( s, tmp, n-extra );
	}
	gcry_free(tmp);
	*nwritten = n;
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_USG ) {
	unsigned int n = (nbits + 7)/8;

	/* we ignore the sign for this format */
	/* FIXME: for performance reasons we should put this into
	 * mpi_aprint becuase we can then use the buffer directly */
	if (buffer && n > len)
	    return gcry_error (GPG_ERR_TOO_SHORT);  /* the provided buffer is too short */
	if( buffer ) {
	    unsigned char *tmp;
	    tmp = _gcry_mpi_get_buffer( a, &n, NULL );
	    memcpy( buffer, tmp, n );
	    gcry_free(tmp);
	}
	*nwritten = n;
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_PGP ) {
	unsigned int n = (nbits + 7)/8;

	if( a->sign )
	    return gcry_error (GPG_ERR_INV_ARG); /* pgp format can only handle unsigned */

	if (buffer && n+2 > len)
	    return gcry_error (GPG_ERR_TOO_SHORT);  /* the provided buffer is too short */
	if( buffer ) {
	    unsigned char *tmp;
	    unsigned char *s = buffer;
	    s[0] = nbits >> 8;
	    s[1] = nbits;

	    tmp = _gcry_mpi_get_buffer( a, &n, NULL );
	    memcpy( s+2, tmp, n );
	    gcry_free(tmp);
	}
	*nwritten = n+2;
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_SSH ) {
	unsigned char *tmp;
	int extra = 0;
	unsigned int n;

	if( a->sign )
	    return gcry_error (GPG_ERR_INTERNAL); /* can't handle it yet */

	tmp = _gcry_mpi_get_buffer( a, &n, NULL );
	if( n && (*tmp & 0x80) ) {
	    n++;
	    extra=1;
	}

	if (buffer && n+4 > len) {
	    gcry_free(tmp);
	    return gcry_error (GPG_ERR_TOO_SHORT);  /* the provided buffer is too short */
	}
	if( buffer ) {
	    byte *s = buffer;
	    *s++ = n >> 24;
	    *s++ = n >> 16;
	    *s++ = n >> 8;
	    *s++ = n;
	    if( extra )
		*s++ = 0;

	    memcpy( s, tmp, n-extra );
	}
	gcry_free(tmp);
	*nwritten = 4+n;
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else if( format == GCRYMPI_FMT_HEX ) {
	byte *tmp;
	int i;
	int extra = 0;
	unsigned int n=0;

	tmp = _gcry_mpi_get_buffer( a, &n, NULL );
	if( !n || (*tmp & 0x80) )
	    extra=2;

	if(buffer && 2*n + extra + !!a->sign + 1 > len) {
	    gcry_free(tmp);
	    return gcry_error (GPG_ERR_TOO_SHORT);  /* the provided buffer is too short */
	}
	if( buffer ) {
	    byte *s = buffer;
	    if( a->sign )
		*s++ = '-';
	    if( extra ) {
		*s++ = '0';
		*s++ = '0';
	    }

	    for(i=0; i < n; i++ ) {
		unsigned int c = tmp[i];
		*s++ = (c >> 4) < 10? '0'+(c>>4) : 'A'+(c>>4)-10 ;
		c &= 15;
		*s++ = c < 10? '0'+c : 'A'+c-10 ;
	    }
	    *s++ = 0;
	    *nwritten = s - buffer;
	}
	else {
	    *nwritten = 2*n + extra + !!a->sign + 1;
	}
	gcry_free(tmp);
	return gcry_error (GPG_ERR_NO_ERROR);
    }
    else
	return gcry_error (GPG_ERR_INV_ARG);
}

/****************
 * Like gcry_mpi_print but this function allocates the buffer itself.
 * The caller has to supply the address of a pointer. NWRITTEN may be
 * NULL.
 */
gcry_error_t
gcry_mpi_aprint( enum gcry_mpi_format format,
                 unsigned char **buffer, size_t *nwritten,
		 struct gcry_mpi *a )
{
    size_t n;
    gcry_error_t rc;

    *buffer = NULL;
    rc = gcry_mpi_print( format, NULL, 0, &n, a );
    if( rc )
	return rc;
    *buffer = mpi_is_secure(a) ? gcry_xmalloc_secure( n ) : gcry_xmalloc( n );
    rc = gcry_mpi_print( format, *buffer, n, &n, a );
    if( rc ) {
	gcry_free(*buffer);
	*buffer = NULL;
    }
    else if( nwritten )
	*nwritten = n;
    return rc;
}

