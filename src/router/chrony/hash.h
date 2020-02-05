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

extern int HSH_GetHashId(const char *name);

extern unsigned int HSH_Hash(int id,
    const unsigned char *in1, unsigned int in1_len,
    const unsigned char *in2, unsigned int in2_len,
    unsigned char *out, unsigned int out_len);

extern void HSH_Finalise(void);

#endif
