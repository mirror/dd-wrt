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
 * endian.c
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * convert endianness from/to network-byte order
 * on Big Endian machines these are null macros
 * ------------------------------------------------------------------- */

#define HEADERS()

#include "headers.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------
 * ntoh
 *
 * Given a buffer of len elements, each of size inSizeof, convert
 * the endian-ness of each element from network to host byte order.
 * This function is necesary for Little Endian machines (Intel) but
 * not for Big Endian machines (most RISC), where it is a null macro.
 * ------------------------------------------------------------------- */

#ifdef WORDS_BIGENDIAN

/* nothing to do; network byte order is big endian */

#else /* not defined WORDS_BIGENDIAN */

void ntoh( void *buffer, int len, int inSizeof ) {
    switch ( inSizeof ) {
        case 2: {
                /* note that htons works on 2 bytes, regardless of sizeof(short) */
                u_int16_t* shorts = (u_int16_t*) buffer;

                while ( len-- > 0 ) {
                    *shorts = htons( *shorts );
                    shorts++;
                }
                break;
            }

        case 4: {
                /* note that htonl works on 4 bytes, regardless of sizeof(long) */
                u_int32_t* longs = (u_int32_t*) buffer;

                while ( len-- > 0 ) {
                    *longs = htonl( *longs );
                    longs++;
                }
                break;
            }

        default: {
                /* there are probably more efficient ways to do byte swapping */
                int i;
                int half_i = inSizeof / 2;
                int last_i = inSizeof - 1;
                char tmp;
                char* bytes = (char*) buffer;

                while ( len-- > 0 ) {
                    for ( i = 0; i < half_i; i++ ) {
                        /* swap bytes i and n-i, where n is the index
                         * of the last byte of each element */
                        tmp                 = bytes[ i ];
                        bytes[ i ]          = bytes[ last_i - i ];
                        bytes[ last_i - i ] = tmp;
                    }
                    bytes += inSizeof;
                }
                break;
            }
    }
}

#endif /* not defined WORDS_BIGENDIAN */

#ifdef __cplusplus
} /* end extern "C" */
#endif
