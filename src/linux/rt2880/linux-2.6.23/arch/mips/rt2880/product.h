
/******************************************************************************
*
* (c) Copyright 1996-97, Palmchip Corporation
*
* This document is an unpublished work protected under the copyright laws
* of the United States containing the confidential, proprietary and trade
* secret information of Palmchip Corporation. This document may not be
* copied or reproduced in any form whatsoever without the express written
* permission of Palmchip Corporation.
*
*******************************************************************************
*
*  File Name: product.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/12/97  RWB   Created.
*    02/09/01  LRF   Removed UART Default definitions.
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains the product dependent values, so that the entire
//    library can be customised by changing this simple file. 
//
//    It also includes the product's memory map header file and a
//    header file containing common product-independent definitions.
//
// Sp. Notes:
//
 ******************************************************************************/

#ifndef PRODUCT_H
#define PRODUCT_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "mem_map.h"


/*=====================*
 *  Defines            *
 *=====================*/

#define SYS_CLK_MHZ			(14)
#define SYS_CLK_KHZ			(14746)

/* Audio sysclock must be less than sysclock */
#define AUD_SYS_CLK_HZ			(11289600)

#define SYS_CLK_NS			(1000 / SYS_CLK_MHZ)


/*
** This default value is chosen since the system clock freq is 66 MHz.
** So this will provide 2000 tics per sec, if the timer clock divider is
** set to 0. If it is set to 4, 8 or 16 then the timer will provide accurate tic
** every 2ms, 4ms or 8ms respectively since all numbers are intergers.
*/




/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/


#endif /* PRODUCT_H */


