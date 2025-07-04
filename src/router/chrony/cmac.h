/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019
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

  Header file for CMAC.

  */

#ifndef GOT_CMAC_H
#define GOT_CMAC_H

/* Avoid overlapping with the hash enumeration */
typedef enum {
  CMC_INVALID = 0,
  CMC_AES128 = 13,
  CMC_AES256 = 14,
} CMC_Algorithm;

typedef struct CMC_Instance_Record *CMC_Instance;

extern int CMC_GetKeyLength(CMC_Algorithm algorithm);
extern CMC_Instance CMC_CreateInstance(CMC_Algorithm algorithm, const unsigned char *key,
                                       int length);
extern int CMC_Hash(CMC_Instance inst, const void *in, int in_len,
                    unsigned char *out, int out_len);
extern void CMC_DestroyInstance(CMC_Instance inst);

#endif

