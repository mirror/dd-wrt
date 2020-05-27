/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/h/flash.h#5 $
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: #5 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains Flash-PROM interface function prototypes
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#ifndef __INC_FLASH_H
#define __INC_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ******************************************************************/

#define FT_AMD		0
#define FT_INTEL	1
#define FT_29		2
/*
 * Next flash type defined in skspi.h
 * #define FT_SPI		3
 */

/* typedefs ******************************************************************/

/* function prototypes *******************************************************/

int calibrate(
	SK_AC		*pAC);

void init_flash(
	SK_AC		*pAC,
	SK_IOC		IoC,
	unsigned	rombase,
	long		pages,
	long		faddr);

int flash_man_code(
	SK_AC		*pAC,
	SK_IOC		IoC);

int program_flash(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*prom_data);

int flash_verify(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data);

int erase_flash(
	SK_AC			*pAC,
	SK_IOC			IoC);

int read_from_flash(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	offs,
	unsigned long	size);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __INC_FLASH_H */

/* End of File */
