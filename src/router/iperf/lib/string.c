/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002,2003                              
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 *
 * string.c
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * various string utilities
 * ------------------------------------------------------------------- */

#include "headers.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------
 * pattern
 *
 * Initialize the buffer with a pattern of (index mod 10).
 * ------------------------------------------------------------------- */

void pattern( char *outBuf, int inBytes ) {
    assert( outBuf != NULL );

    while ( inBytes-- > 0 ) {
        outBuf[ inBytes ] = (inBytes % 10) + '0';
    }
} /* end pattern */

/* -------------------------------------------------------------------
 * replace( text, length, replacement )
 *
 * replaces text[ 0..length-1 ] with replacement, shifting
 * text[ length.. ] so it is not lost in any way.
 * ------------------------------------------------------------------- */

void replace( char *position, int poslen, const char *replacement ) {
    int orig_len = strlen( position    );
    int repl_len = strlen( replacement );

    /* move memory from (position + poslen) down to (position + repl_len).
     * remember the null terminating byte! */
    memmove( position + repl_len, position + poslen, orig_len - poslen + 1 );

    /* Put in replacement string */
    memcpy( position, replacement, repl_len );
}

/* -------------------------------------------------------------------
 * concat( destination, length, source )
 *
 * Similar to strcat, but will not overwrite the bounds of dest
 * and will *always* terminate dest (unlike strncat).
 * ------------------------------------------------------------------- */

char *concat( char *dest, int len, const char *src ) {
    char *s = dest;
    char *end = dest + len;

    /* make s point to the end (terminating null) of s1 */
    while ( *s != '\0' )
        s++;

    /* copy characters until end (before terminating null) of src,
     * or end of dest buffer */
    while ( *src != '\0' ) {
        *s++ = *src++;

        if ( s >= end ) {
            s--;   /* back up one for terminating null */
            break;
        }
    }

    /* null terminate */
    *s = '\0';

    return dest;
}

/* -------------------------------------------------------------------
 * copy( destination, length, source )
 *
 * Similar to strcpy, but will not overwrite the bounds of dest
 * and will *always* terminate dest (unlike strncpy).
 * ------------------------------------------------------------------- */

char *copy( char *dest, int len, const char *src ) {
    char *s = dest;
    char *end = dest + len;

    /* copy characters until end (before terminating null) of src,
     * or end of dest buffer */
    while ( *src != '\0' ) {
        *s++ = *src++;

        if ( s >= end ) {
            s--;   /* back up one for terminating null */
            break;
        }
    }

    /* null terminate */
    *s = '\0';

    return dest;
}

#ifdef __cplusplus
} /* end extern "C" */
#endif
