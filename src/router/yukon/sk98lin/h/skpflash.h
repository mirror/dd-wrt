/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/h/skpflash.h#6 $
 * Project:	Flash Programmer, Manufacturing, Diagnostic Tools, and Drivers
 * Version:	$Revision: #6 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains SPI-Flash EEPROM specific definitions and constants
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

#ifndef	__INC_SKPFLASH_H
#define	__INC_SKPFLASH_H 

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ******************************************************************/

/* PFL constants */

#define SK_PFL_READ			1		/* Read action for SkPflManage*() */
#define SK_PFL_VERIFY		2		/* Verify action for SkPflManage*() */
#define SK_PFL_WRITE		3		/* Write action for SkPflManage*() */
#define SK_PFL_ERASE		4		/* Erase action for SkPflManage*() */

/* macros *******************************************************************/

#define FL_SWAP16(x)	\
			((((x) & 0xff00) >> 8) | \
			 (((x) & 0x00ff) << 8))

#define FL_SWAP32(x)	\
			((((x) & 0xff000000) >> 24) | \
			 (((x) & 0x00ff0000) >>  8) | \
			 (((x) & 0x0000ff00) <<  8) | \
			 (((x) & 0x000000ff) << 24))


/* typedefs ******************************************************************/

/*
 * Yukon-II family parallel flash device structure
 */
typedef struct s_PflDevTable {
	char	*pManName;
	SK_U32	ManId;
	char	*pDevName;
	SK_U16	DevId;
	SK_U32	SectorSize[2];
	SK_U32	NumSectors[2];

	struct {
		SK_U8	OpReadStatus;
		SK_U8	OpReadManDev;

		SK_U8	OpProgram;
		SK_U8	OpProgramCompare;
		SK_U8	OpPageErase;

		SK_U8	OpInfoProgram;
		SK_U8	OpInfoProgramCompare;
		SK_U8	OpInfoPageErase;
	} opcodes;
} SK_PFL_DEVICE;

/*
 *	Instruction/command code:
 *	8'h02   Program
 *	8'h07   Program and Compare
 *	8'h82   Information memory Program
 *	8'h87   Information memory Program and compare
 *	8'h05   Read Status register from Flash memory
 *	8'h15   Read manufacturer and device code
 *	8'h52   Page erase (to 1)
 *	8'hD2   Information memory erase (to 1)
 *	8'h62   Main memory erase (to 1)
 *	8'hE2   Both Information and Main memory erase (to 1)
 *	8'hA4   Shut down the flash
 *	8'hE5   Wake up the flash
 *	8'hA5   Set Slew
 *	8'hB0   Set config register1
 *	8'hB1   Set config register2
 *	8'hB2   Set config register3
 *	8'hB3   Set config register4
 *	8'hB4   Read config register1
 *	8'hB5   Read config register2
 *	8'hB6   Read config register3
 *	8'hB7   Read config register4
 */

typedef	struct s_Pfl {
	SK_PFL_DEVICE	*pPflDev;				/* Pointer to type of parallel flash */
	SK_U8			YkChipId;

/*	SK_U8			ModifiedPages[0x80];	Modified pages: 0x80000 / 0x1000 */
} SK_PFL;

/* function prototypes *******************************************************/

/*
 * Module usage:
 * - Call SkPflCheck() before calling any other SkPfl*() functions.
 * - Call SkPflManage() or SkPflManageInfo() to execute actions in the
 *   main or info part of the integrated parallel flash, respectively.
 *
 * NOTE: Writing contiguous 64-byte blocks during a single call to SkPflManage*()
 *		 minimizes the number of 'program' accesses to the parallel flash.
 *
 * skpflash needs spi_timer() as macro or function:
 *	extern int spi_timer(SK_AC *pAC, unsigned int t);
 * 't' is the time in seconds after that the timer should expire.
 * If 't' is 0, then the expiration status is returned (1 for 'expired').
 *
 * It also needs SK_IN*() and SK_OUT*() macros (or functions) for
 * hardware accesses.
 */

int SkPflCheck(
	SK_AC	*pAC,
	SK_IOC	IoC);

int SkPflManage(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U8	*pData,
	SK_U32	Offs,
	SK_U32	Len,
	int		Action);

int SkPflManageInfo(
	SK_AC	*pAC,
	SK_IOC	IoC,
	SK_U8	*pData,
	SK_U32	Offs,
	SK_U32	Len,
	int		Action);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __INC_SKPFLASH_H */

/* End of File */
