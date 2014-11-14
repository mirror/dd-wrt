
/******************************************************************************
*
* (c) Copyright 1996-2000, Palmchip Corporation
*
* This document is an unpublished work protected under the copyright laws
* of the United States containing the confidential, proprietary and trade
* secret information of Palmchip Corporation. This document may not be
* copied or reproduced in any form whatsoever without the express written
* permission of Palmchip Corporation.
*
*******************************************************************************
*
*  File Name: pubdefs.h
*     Author: Robin Bhagat 
*
*    Purpose: To define the CPU architecture and compiler independent typedef
*             for portability.
*
*  Sp. Notes:
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    07/30/96  RWB   Created
*
*
*
*******************************************************************************/

#ifndef PUBDEFS_H
#define PUBDEFS_H

/*=====================*
 *  Include Files      *
 *=====================*/
/* Get INLINE definition. */
#include "compiler.h"            // CPU-specific defs


/*=====================*
 *  Defines            *
 *=====================*/

typedef char              int8;
typedef short             int16;
typedef long              int32;

typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef unsigned long     uint32;
typedef volatile unsigned long     asicreg;

//typedef unsigned long     bool;

typedef void (*voidFuncPtr)(void);

#define PUBLIC            extern
#define PRIVATE           static
#define FAST              register
#define REG               register

#ifndef TRUE
#define TRUE              (1)
#define FALSE             (0)
#endif

#define MIN( x, y )	( (x) < (y) ? (x) : (y) )
#define MAX( x, y )	( (x) > (y) ? (x) : (y) )

/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* PUBDEFS_H */
