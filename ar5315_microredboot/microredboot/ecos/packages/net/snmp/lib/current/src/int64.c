//==========================================================================
//
//      ./lib/current/src/int64.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/** file: test.c - test of 64-bit integer stuff
*
*
* 21-jan-1998: David Perkins <dperkins@dsperkins.com>
*
*/

#include <config.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#include "asn1.h"
#include "int64.h"

#define TRUE 1
#define FALSE 0

/** divBy10 - divide an unsigned 64-bit integer by 10
*
* call with:
*   u64 - number to be divided
*   pu64Q - location to store quotient
*   puR - location to store remainder
*
*/
void
divBy10(U64 u64,
	U64 *pu64Q,
	unsigned int *puR)  
{
    unsigned long ulT;
    unsigned long ulQ;
    unsigned long ulR;


    /* top 16 bits */
    ulT = (u64.high>>16) & 0x0ffff;
    ulQ = ulT/10;
    ulR = ulT%10;
    pu64Q->high = ulQ<<16;

    /* next 16 */
    ulT = (u64.high & 0x0ffff);
	ulT += (ulR<<16);
    ulQ = ulT/10;
    ulR = ulT%10;
    pu64Q->high = pu64Q->high | ulQ;

    /* next 16 */
    ulT = ((u64.low>>16) & 0x0ffff) + (ulR<<16);
    ulQ = ulT/10;
    ulR = ulT%10;
    pu64Q->low = ulQ<<16;

    /* final 16 */
    ulT = (u64.low & 0x0ffff);
	ulT += (ulR<<16);
    ulQ = ulT/10;
    ulR = ulT%10;
    pu64Q->low = pu64Q->low | ulQ;

    *puR = (unsigned int)(ulR);


} /* divBy10 */


/** multBy10 - multiply an unsigned 64-bit integer by 10
*
* call with:
*   u64 - number to be multiplied
*   pu64P - location to store product
*
*/
void
multBy10(U64 u64,
	 U64 *pu64P)
{
    unsigned long ulT;
    unsigned long ulP;
    unsigned long ulK;


    /* lower 16 bits */
    ulT = u64.low & 0x0ffff;
    ulP = ulT * 10;
    ulK = ulP>>16;
    pu64P->low = ulP & 0x0ffff;

    /* next 16 */
    ulT = (u64.low>>16) & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP>>16;
    pu64P->low = (ulP & 0x0ffff)<<16 | pu64P->low;

    /* next 16 bits */
    ulT = u64.high & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP>>16;
    pu64P->high = ulP & 0x0ffff;

    /* final 16 */
    ulT = (u64.high>>16) & 0x0ffff;
    ulP = (ulT * 10) + ulK;
    ulK = ulP>>16;
    pu64P->high = (ulP & 0x0ffff)<<16 | pu64P->high;


} /* multBy10 */


/** incrByU16 - add an unsigned 16-bit int to an unsigned 64-bit integer
*
* call with:
*   pu64 - number to be incremented
*   u16 - amount to add
*
*/
void
incrByU16(U64 *pu64,
	  unsigned int u16)
{
    unsigned long ulT1;
    unsigned long ulT2;
    unsigned long ulR;
    unsigned long ulK;


    /* lower 16 bits */
    ulT1 = pu64->low;
    ulT2 = ulT1 & 0x0ffff;
    ulR = ulT2 + u16;
    ulK = ulR>>16;
    if (ulK == 0) {
        pu64->low = ulT1 + u16;
        return;
    }

    /* next 16 bits */
    ulT2 = (ulT1>>16) & 0x0ffff;
    ulR = ulT2+1;
    ulK = ulR>>16;
    if (ulK == 0) {
        pu64->low = ulT1 + u16;
        return;
    }
   
    /* next 32 - ignore any overflow */
    pu64->low = (ulT1 + u16) & 0x0FFFFFFFFL;
    pu64->high++;

} /* incrByV16 */

void
incrByU32(U64 *pu64,
	  unsigned int u32)
{
  unsigned int tmp;
  tmp = pu64->low;
  pu64->low += u32;
  if (pu64->low < tmp)
    pu64->high++;
}

/* pu64out = pu64one - pu64two */
void
u64Subtract(U64 *pu64one,
            U64 *pu64two,
            U64 *pu64out)
{
  if (pu64one->low > pu64two->low) {
    pu64out->low = 0xffffffff - pu64two->low + pu64one->low + 1;
    pu64out->high = pu64one->high - pu64two->high - 1;
  } else {
    pu64out->low = pu64one->low - pu64two->low;
    pu64out->high = pu64one->high - pu64two->high;
  }
}

/** zeroU64 - set an unsigned 64-bit number to zero
*
* call with:
*   pu64 - number to be zero'ed
*
*/
void
zeroU64(U64 *pu64)
{

    pu64->low = 0;
    pu64->high = 0;
} /* zeroU64 */


/** isZeroU64 - check if an unsigned 64-bit number is
*
* call with:
*   pu64 - number to be zero'ed
*
*/
int
isZeroU64(U64 *pu64)
{

    if ((pu64->low == 0) && (pu64->high == 0))
        return(TRUE);
    else
        return(FALSE);

} /* isZeroU64 */

void
printU64(char * buf, /* char [I64CHARSZ+1]; */
	 U64 *pu64)
{
  U64 u64a;
  U64 u64b;

  char aRes [I64CHARSZ+1];
  unsigned int u;
  int j;

  u64a.high = pu64->high;
  u64a.low = pu64->low;
  aRes[I64CHARSZ] = 0;
  for (j = 0; j < I64CHARSZ; j++) {
    divBy10(u64a, &u64b, &u);
    aRes[(I64CHARSZ-1)-j] = (char)('0' + u);
    u64a.high = u64b.high;
    u64a.low = u64b.low;
    if (isZeroU64(&u64a))
      break;
  }
  strcpy(buf, &aRes[(I64CHARSZ-1)-j]);
}

void
printI64(char * buf, /* char [I64CHARSZ+1]; */
	 U64 *pu64)
{
  U64 u64a;
  U64 u64b;

  char aRes [I64CHARSZ+1];
  unsigned int u;
  int j, sign=0;

  if (pu64->high & 0x80000000) {
    u64a.high = ~pu64->high;
    u64a.low = ~pu64->low;
    sign = 1;
    incrByU32(&u64a, 1);  /* bit invert and incr by 1 to print 2s complement */
  } else {
    u64a.high = pu64->high;
    u64a.low = pu64->low;
  }
  
  aRes[I64CHARSZ] = 0;
  for (j = 0; j < I64CHARSZ; j++) {
    divBy10(u64a, &u64b, &u);
    aRes[(I64CHARSZ-1)-j] = (char)('0' + u);
    u64a.high = u64b.high;
    u64a.low = u64b.low;
    if (isZeroU64(&u64a))
      break;
  }
  if (sign == 1) {
    aRes[(I64CHARSZ-1)-j-1] = '-';
    strcpy(buf, &aRes[(I64CHARSZ-1)-j-1]);
    return;
  }
  strcpy(buf, &aRes[(I64CHARSZ-1)-j]);
}

int
read64(U64 *i64,
       const char *string)
{
  U64 i64p;
  unsigned int u;
  int sign = 0;
  int ok = 0;
  
  zeroU64(i64);
  if (*string == '-') {
    sign = 1;
    string++;
  }
  
  while (*string && isdigit(*string)) {
    ok = 1;
    u = *string - '0';
    multBy10(*i64, &i64p);
    memcpy(i64, &i64p, sizeof(i64p));
    incrByU16(i64, u);
    string++;
  }
  if (sign) {
    i64->high = ~i64->high;
    i64->low = ~i64->low;
    incrByU16(i64,1);
  }
  return ok;
}


  

#ifdef TESTING
void
main(int argc, char *argv[])
{
    int i;
    int j;
    int l;
    unsigned int u;
    U64 u64a;
    U64 u64b;
#define MXSZ 20
    char aRes[MXSZ+1];


    if (argc < 2) {
        printf("This program takes numbers from the command line\n"
               "and prints them out.\n"
               "Usage: test <unsignedInt>...\n");
        exit(1);
    }

    aRes[MXSZ] = 0;

    for (i = 1; i < argc; i++) {
        l = strlen(argv[i]);
        zeroU64(&u64a);
        for (j = 0; j < l; j++) {
            if (!isdigit(argv[i][j])) {
                printf("Argument is not a number \"%s\"\n", argv[i]);
                exit(1);
            }
            u = argv[i][j] - '0';
            multBy10(u64a, &u64b);
            u64a = u64b;
            incrByU16(&u64a, u);
        }

        printf("number \"%s\" in hex is '%08x%08x'h\n",
                argv[i], u64a.high, u64a.low);

        printf("number is \"%s\"\n", printU64(&u64a));
        for (j = 0; j < MXSZ; j++) {
            divBy10(u64a, &u64b, &u);
            aRes[(MXSZ-1)-j] = (char)('0' + u);
            u64a = u64b;
            if (isZeroU64(&u64a))
                break;
        }

        printf("number is \"%s\"\n", &aRes[(MXSZ-1)-j]);
    }
	exit(0);
} /* main */
#endif /* TESTING */

/* file: test.c */

