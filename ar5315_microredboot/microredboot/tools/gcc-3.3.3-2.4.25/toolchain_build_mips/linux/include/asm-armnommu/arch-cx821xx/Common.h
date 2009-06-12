/****************************************************************************
*
*	Name:			Common.h
*
*	Description:	Common types and structures used in the system
*
*	Copyright:		(c) 1997 - 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
** $Archive: /Projects/Hasbani and Derivatives Linux/Reference Designs/Mackinac/Lineo 2.4.6 Beta/linux/include/asm-armnommu/arch-cx821xx/Common.h $
** $Revision: 1.1 $
** $Date: 2003/06/29 14:28:18 $
*******************************************************************************
******************************************************************************/
#ifndef _COMMON_H_		//	File Wrapper,
#define _COMMON_H_		//	prevents multiple inclusions


/*******************************************************************************
**                              Keywords
*******************************************************************************/
#define LOCAL static					/* Used only in the current file */
#define MODULAR							/* Used only in the current module */
#define GLOBAL							/* Used inter module */

#define IN								/* Used to denote input arguments */
#define OUT								/* Used to denote output arguments */
#define I_O								/* Used to denote input/output arguments */


/*******************************************************************************
**                              Macros
*******************************************************************************/

#define NOT_USED(x)  (x) = (x)



////////////////////////////
//	
////////////////////////////

//define OFF						0
//define ON						1
#ifdef OS_NONE
	#include "OsDefs.h"	// OsTools.h includes linux files - we only need OsTools.h types
	#include <stddef.h>	//size_t
#else
	#include <OsTools.h>
#endif

#if 0
#ifndef __MODEM_H_
#if !defined( BOOL)
typedef BOOLEAN BOOL;
#endif

typedef enum 
{
OFF,
ON
} BINARY;


typedef unsigned char			BYTE;		/* 8-bit unsigned integer  */
typedef unsigned short			WORD;		/* 16-bit unsigned integer */
typedef unsigned long			DWORD;		/* 32-bit unsigned integer */

typedef signed char				CHAR;	    /* 8-bit signed integer  */
typedef signed short int		SHORT;	    /* 16-bit signed integer */
typedef signed long int			LONG;	    /* 32-bit signed integer */

#endif

//
// Bit mask
//
#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

#else
typedef int BINARY;
#endif
/* 	The va_names type match the type name after promotion, These can be used with 
**	the va functions to get the correct size variable off the stack. 
*/

typedef BOOLEAN VA_BOOL;

typedef BINARY VA_BINARY;

typedef unsigned char			VA_BYTE;		  /* 8-bit unsigned integer  */
typedef unsigned short			VA_WORD;		  /* 16-bit unsigned integer */
typedef unsigned long			VA_DWORD;		  /* 32-bit unsigned integer */

typedef signed char				VA_CHAR;		  /* 8-bit signed integer  */
typedef signed short int		VA_SHORT;		  /* 16-bit signed integer */
typedef signed long int			VA_LONG;		  /* 32-bit signed integer */


//
// Bit clear mask
//
#define BIT0_CLEAR		0xFE
#define BIT1_CLEAR		0xFD
#define BIT2_CLEAR		0xFB
#define BIT3_CLEAR		0xF7
#define BIT4_CLEAR		0xEF
#define BIT5_CLEAR		0xDF
#define BIT6_CLEAR		0xBF
#define BIT7_CLEAR		0x7F



// counters to be used for statistics  
// each counter has a flag to track if the counter has overflowed.

typedef struct
{
	BOOL	Overflow ;
	BYTE	Cnt ;
} BYTE_CTR_T ;

typedef struct
{
	BOOL Overflow ;
	WORD Cnt ;
} WORD_CTR_T ;

typedef struct
{
	BOOL Overflow ;
	LONG Cnt ;
} LONG_CTR_T;

typedef struct
{
	BOOL Overflow ;
	LONGLONG Cnt ;
} LONGLONG_CTR_T;


// Macros used to update statistics counters
// These macors also check for overflow.

#define ADD_TO_CTR( Ctr, Delta ) \
	if ( Ctr.Cnt + (Delta) < Ctr.Cnt ) \
	{ \
		Ctr.Overflow = TRUE ; \
	} \
	Ctr.Cnt += Delta ; 

#define ADD_TO_CTR_IND( CtrPtr, Delta ) \
	if ( (CtrPtr)->Cnt + (Delta) < (CtrPtr)->Cnt ) \
	{ \
		(CtrPtr)->Overflow = TRUE ; \
	} \
	(CtrPtr)->Cnt += Delta ;

// Returns size in bytes rounded up to the nearest DWORD
#define SIZEOF_DWORD_ALIGN(n)	((sizeof(n) + 3) >> 2 << 2)
//#pragma intrinsic(memcmp)
#endif		//#ifndef _COMMON_H_
