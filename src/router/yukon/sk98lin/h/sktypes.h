/******************************************************************************
 *
 * Name:	sktypes.h
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Purpose:	Define data types for Linux
 *
 ******************************************************************************/

/******************************************************************************
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 ******************************************************************************/

#ifndef __INC_SKTYPES_H
#define __INC_SKTYPES_H

#define SK_I8    s8    /* 8 bits (1 byte) signed       */
#define SK_U8    u8    /* 8 bits (1 byte) unsigned     */
#define SK_I16  s16    /* 16 bits (2 bytes) signed     */
#define SK_U16  u16    /* 16 bits (2 bytes) unsigned   */
#define SK_I32  s32    /* 32 bits (4 bytes) signed     */
#define SK_U32  u32    /* 32 bits (4 bytes) unsigned   */
#define SK_I64  s64    /* 64 bits (8 bytes) signed     */
#define SK_U64  u64    /* 64 bits (8 bytes) unsigned   */

#define SK_UPTR	ulong  /* casting pointer <-> integral */

#define SK_BOOL   SK_U8
#define SK_FALSE  0
#define SK_TRUE   (!SK_FALSE)

#endif	/* __INC_SKTYPES_H */

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
