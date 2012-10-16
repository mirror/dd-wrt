
/******************************************************************************
 *
 * (c) Copyright 2000-2001, Palmchip Corporation
 *
 * This document is an unpublished work protected under the copyright laws
 * of the United States containing the confidential, proprietary and trade
 * secret information of Palmchip Corporation. This document may not be
 * copied or reproduced in any form whatsoever without the express written
 * permission of Palmchip Corporation.
 *
 ******************************************************************************
 *
 *  File Name: compiler.h
 *     Author: Linda Yang
 *
 ******************************************************************************
 *
 * Revision History:
 *
 *      Date    Name  Comments
 *    --------  ---   ------------------------------------
 *    12/22/00  LYT   Created.
 *
 *****************************************************************************/


/* FILE_DESC ******************************************************************
//
// Purpose:
//    To define all compiler-specific macros.
//
// Sp. Notes:
//
******************************************************************************/


#ifndef COMPILER_H
#define COMPILER_H


/*=====================*
 *  Include Files      *
 *=====================*/


/*=====================*
 *  Defines            *
 *=====================*/

/* 
** __inline is special function declaration in ARM compiler for 
** inline function.
*/
#define INLINE            __inline

/* 
** __irq is special function declaration in ARM compiler for 
** IRQ and FIRQ interrupt handlers.
*/
#define ISR               __irq          


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* COMPILER_H */
