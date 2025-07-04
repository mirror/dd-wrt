/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2012
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header file for crypto hashing.

  */

#ifndef GOT_HASH_H
#define GOT_HASH_H

/* length of hash values produced by SHA512 */
#define MAX_HASH_LENGTH 64

typedef enum {
  HSH_INVALID = 0,
  HSH_MD5 = 1,
  HSH_SHA1 = 2,
  HSH_SHA256 = 3,
  HSH_SHA384 = 4,
  HSH_SHA512 = 5,
  HSH_SHA3_224 = 6,
  HSH_SHA3_256 = 7,
  HSH_SHA3_384 = 8,
  HSH_SHA3_512 = 9,
  HSH_TIGER = 10,
  HSH_WHIRLPOOL = 11,
  HSH_MD5_NONCRYPTO = 10000, /* For NTPv4 reference ID */
} HSH_Algorithm;

extern int HSH_GetHashId(HSH_Algorithm algorithm);

extern int HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
                    unsigned char *out, int out_len);

extern void HSH_Finalise(void);

#endif
