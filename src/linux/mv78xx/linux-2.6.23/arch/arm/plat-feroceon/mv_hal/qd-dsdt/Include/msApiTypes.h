/* 
 * Copyright 2002, Marvell International Ltd.
 * 
 * THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 * NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 * OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 * DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 * THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED
 * OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
 */

/*
 * FILENAME:    $Workfile: mv_types.h $
 * REVISION:    $Revision: 3 $
 * LAST UPDATE: $Modtime: 12/24/02 5:37p $
 *
 * DESCRIPTION:
 *     This file defines common data types used on Host and NetGX sides.
 */


#ifndef MV_TYPES_H
#define MV_TYPES_H


/* general */

#undef IN
#define IN
#undef OUT
#define OUT
#undef INOUT
#define INOUT


#ifndef NULL
#define NULL ((void*)0)
#endif

typedef void  GT_VOID;
typedef char  GT_8;
typedef short GT_16; 
typedef long  GT_32;

typedef unsigned char  GT_U8;
typedef unsigned short GT_U16;
typedef unsigned long  GT_U32;
typedef unsigned int   GT_UINT; 

typedef union {
	GT_U8	c[8];
	GT_U16	s[4];
	GT_U32	l[2];
} GT_U64;


typedef enum {
    GT_FALSE = 0,
    GT_TRUE  = 1
} GT_BOOL;

typedef void          (*GT_VOIDFUNCPTR) (void); /* ptr to function returning void */
typedef unsigned int  (*GT_INTFUNCPTR)  (void); /* ptr to function returning int  */


/* module state */
typedef enum {
	GT_STATE_NONE = 0,	/* Uninitialized */
	GT_STATE_IDLE,		/* Initialized, but not started (or stopped) */
	GT_STATE_ACTIVE		/* Started */
} GT_STATE;


#define	GT_ETHERNET_HEADER_SIZE		(6)

typedef struct
{
    GT_U8       arEther[GT_ETHERNET_HEADER_SIZE];
}GT_ETHERADDR;

/* This macro checks for a multicast mac address    */
#define GT_IS_MULTICAST_MAC(mac)  ((mac.arEther[0] & 0x1) == 1)


/* This macro checks for an broadcast mac address     */
#define GT_IS_BROADCAST_MAC(mac) (((mac).arEther[0] == 0xFF) && ((mac).arEther[1] == 0xFF) && ((mac).arEther[2] == 0xFF) && ((mac).arEther[3] == 0xFF) && ((mac).arEther[4] == 0xFF) && ((mac).arEther[5] == 0xFF))


/* status / error codes */
typedef int GT_STATUS;

#define GT_ERROR		   (-1)
#define GT_OK			   (0x00)	/* Operation succeeded                   */
#define GT_FAIL			   (0x01)	/* Operation failed                      */
#define GT_BAD_VALUE       (0x02)   /* Illegal value (general)               */
#define GT_BAD_PARAM       (0x04)   /* Illegal parameter in function called  */
#define GT_NOT_FOUND       (0x0B)   /* Item not found                        */
#define GT_NO_MORE         (0x0C)   /* No more items found                   */
#define GT_NO_SUCH         (0x0D)   /* No such item                          */
#define GT_TIMEOUT         (0x0E)   /* Time Out                              */
#define GT_NOT_SUPPORTED   (0x10)   /* This request is not support           */
#define GT_ALREADY_EXIST   (0x1B)   /* Tried to create existing item         */

#endif /* MV_TYPES_H */
