/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/skpflash.c#6 $
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools, Drivers
 * Version:	$Revision: #6 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains parallel Flash EEPROM specific functions
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

#include "h/skdrv1st.h"
#ifndef SK_PFL_NO_UPDATE
#ifdef MRVL_UEFI
#include <stdlib.h>
#endif /* !MRVL_UEFI */
#endif	/* !SK_PFL_NO_UPDATE */
#include "h/skdrv2nd.h"

/* defines ******************************************************************/

#ifdef Core
#define fl_print pAC->Gui.WPrint
#elif defined(VCPU)
#define fl_print c_print
#else
extern void fl_print(char *str, ...);
#endif /* Core */

#define	SK_PFL_PART_INFO	0
#define	SK_PFL_PART_MAIN	1

#define SK_PFL_TIMER_SET	3	/* parallel flash timeout value (sec.) */
#define SK_PFL_TIMEOUT		0	/* Timeout check flag */

/* SPI EPROM BUSY CHECK */
#define SK_PFL_IS_BUSY_WR(w)	(((w) & BIT_31) != 0)
#define SK_PFL_IS_BUSY_RD(w)	(((w) & BIT_31) == 0)
#define SK_PFL_IS_BUSY_CMD(w)	(((w) & (BIT_31 | BIT_30 | BIT_29 | BIT_28)) != 0)

/* wait loop for parallel flash to finish a read (start spi_timer() before) */
#define SK_PFL_WAIT_LP_FINISH_RD(pAC, IoC, stat) {							\
	do {																	\
		if (spi_timer(pAC, SK_PFL_TIMEOUT)) {								\
			fl_print("\nRead timeout at parallel flash: %d sec.\n",			\
				SK_PFL_TIMER_SET);											\
			break;															\
		}																	\
		SK_IN32(IoC, FCU_MEM_CTRL, &stat);									\
	} while (SK_PFL_IS_BUSY_RD(stat));										\
}

/* wait loop for parallel flash to finish a write (start spi_timer() before) */
#define SK_PFL_WAIT_LP_FINISH_WR(pAC, IoC, stat) {							\
	do {																	\
		if (spi_timer(pAC, SK_PFL_TIMEOUT)) {								\
			fl_print("\nWrite timeout at parallel flash: %d sec.\n",		\
				SK_PFL_TIMER_SET);											\
			break;															\
		}																	\
		SK_IN32(IoC, FCU_MEM_CTRL, &stat);									\
	} while (SK_PFL_IS_BUSY_WR(stat));										\
}

/* wait for parallel flash to finish reading */
#define SK_PFL_WAIT_FINISH_RD(pAC, IoC) {									\
	volatile unsigned long stat;											\
	/* wait for command to finish */										\
	spi_timer(pAC, SK_PFL_TIMER_SET);										\
	SK_PFL_WAIT_LP_FINISH_RD(pAC, IoC, stat);								\
}

/* wait for parallel flash to finish writing */
#define SK_PFL_WAIT_FINISH_WR(pAC, IoC) {									\
	volatile unsigned long stat;											\
	/* wait for command to finish */										\
	spi_timer(pAC, SK_PFL_TIMER_SET);										\
	SK_PFL_WAIT_LP_FINISH_WR(pAC, IoC, stat);								\
}

/* wait for parallel flash to finish a command (write, erase, ...) */
#define SK_PFL_WAIT_FINISH_CMD(pAC, IoC) {									\
	volatile unsigned long stat2;											\
	SK_PFL_WAIT_FINISH_WR(pAC, IoC);										\
	/* wait for command to finish or timeout */								\
	spi_timer(pAC, SK_PFL_TIMER_SET);										\
	do {																	\
		if (spi_timer(pAC, SK_PFL_TIMEOUT)) {								\
			fl_print("\nCommand timeout at parallel flash: %d sec.\n",		\
				SK_PFL_TIMER_SET);											\
			break;															\
		}																	\
		/* bits 31..29 == 001: read access to parallel flash register. */	\
		/* bits 26..24 == 010: 32-bit access. */							\
		/* bits 7..0: register address */									\
		SK_OUT32(IoC, FCU_MEM_CTRL, BIT_29 | (2 << 24) | 0);				\
		SK_PFL_WAIT_LP_FINISH_RD(pAC, IoC, stat2);							\
		SK_IN32(IoC, FCU_RD_DATA, &stat2);									\
	} while (SK_PFL_IS_BUSY_CMD(stat2));									\
}

/* typedefs ******************************************************************/

/* global variables **********************************************************/

/* local variables **********************************************************/

/*
 * Yukon-II family parallel flash device table
 */
static SK_PFL_DEVICE SkPflDevTable[] = {
	{
		"SupremeInternal", 0x7F7F7FE9, "88E8075", 0x0291,
		{0x800, 0x8000}, {1, 12},
		{0x05, 0x15, 0x02, 0x07, 0x52, 0x82, 0x87, 0xD2}
	},
	{
		"SupremeInternal", 0x7F7F7FE9, "88E8075", 0x0290,
		{0x800, 0x8000}, {1, 12},
		{0x05, 0x15, 0x02, 0x07, 0x52, 0x82, 0x87, 0xD2}
	}
};

/* function prototypes *******************************************************/

/* low level SPI programming external interface */
extern int  spi_timer(SK_AC *pAC, unsigned int t);
#ifndef SK_PFL_NO_UPDATE
extern void *spi_malloc(unsigned long size);
extern void spi_free(void *memblock);
#endif	/* !SK_PFL_NO_UPDATE */

/* local functions ***********************************************************/

/*****************************************************************************
 *
 * SkPflCommand - Executes a command at the parallel flash
 *
 * Description:
 *	This function will send a command to the parallel flash
 *	and wait for it to complete.
 *
 * Returns:
 *	Nothing
 */
static void SkPflCommand(
SK_AC	*pAC,
SK_IOC	IoC,
int		Part,
SK_U8	Cmd,	/* comand code */
SK_U16	Data)	/* data for command (if needed) */
{
	SK_OUT32(IoC, FCU_WR_DATA, BIT_31 | (Cmd << 16) | Data);

	/* bits 31..29 == 101: write access to parallel flash register. */
	/* bits 26..24 == 010: 32-bit access. */
	/* bits 7..0: register address */
	SK_OUT32(IoC, FCU_MEM_CTRL, BIT_31 | BIT_29 | (2 << 24) | 0);

	/* wait for the parallel flash to finish the command */
	SK_PFL_WAIT_FINISH_CMD(pAC, IoC);
}	/* SkPflCommand */

/*****************************************************************************
 *
 * SkPflEraseSector - Erase the specified sector of the parallel flash
 *
 * Description:
 *	This function will erase the sector specified
 *	thus allowing data to be written.
 *
 * Returns:
 *	Nothing
 */
static void SkPflEraseSector(
SK_AC			*pAC,
SK_IOC			IoC,
int				Part,
unsigned long	sector_num)		/* sector to be erased */
{
	if (Part == SK_PFL_PART_MAIN) {
		SkPflCommand(pAC, IoC, Part, pAC->pfl.pPflDev->opcodes.OpPageErase,
			(SK_U16) sector_num);
	}
	else {
		SkPflCommand(pAC, IoC, Part, pAC->pfl.pPflDev->opcodes.OpInfoPageErase,
			(SK_U16) sector_num);
	}
}

/*****************************************************************************
 *
 * SkPflErase - Erases the specified sectors of the parallel flash
 *
 * Description:
 *	Erase all sectors of the flash prom affected by the address
 *	range denoted by parameters "off" (address offset) and "len"
 *	(length of address range).
 *
 * Returns:
 *	0	Success
 *	(1	Timeout)
 */
static int SkPflErase(
SK_AC			*pAC,
SK_IOC			IoC,
int				Part,
unsigned long	off,	/* start offset in parallel flash for erase */
unsigned long	len)	/* length in parallel flash for erase */
{
	unsigned long	i;
	unsigned long	StartSect;
	unsigned long	EndSect;

	if (len == 0) {
		return (0);
	}

	/* Erase all affected sectors. */
	StartSect = off / pAC->pfl.pPflDev->SectorSize[Part];
	EndSect   = (off + len - 1) / pAC->pfl.pPflDev->SectorSize[Part];
#ifdef SK_LINUX
	SK_DBG_PRINTF("%s: Erase flash ", DRV_NAME);
#endif /* SK_LINUX */
	for (i = StartSect; i <= EndSect; i++) {
		SkPflEraseSector(pAC, IoC, Part, i);
#ifdef SK_LINUX
		SK_DBG_PRINTF(".");
#else /* !SK_LINUX */
		fl_print(".");
#endif /* !SK_LINUX */
	}

#ifdef SK_LINUX
	SK_DBG_PRINTF(" finished!\n");
#endif /* SK_LINUX */
	return (0);
}

/*****************************************************************************
 *
 * SkPflReadDword - Reads a dword from the parallel flash
 *
 * Description:
 *	The function reads a dword from the specified address in the parallel flash.
 *
 * Returns:
 *	The DWord read
 */
static SK_U32 SkPflReadDword(
SK_AC	*pAC,
SK_IOC	IoC,
int		Part,
SK_U32	Addr)		/* address in the parallel flash to read from */
{
	SK_U32	Val;

	Addr &= (1 << 20) - 1;
	Addr |= (2 << 24);
	/* bits 31..29 == 000: read access to information memory. */
	/* bits 26..24 == 010: 32-bit access. */
	Addr |= (Part << 30);

	SK_OUT32(IoC, FCU_MEM_CTRL, Addr);
	SK_PFL_WAIT_FINISH_RD(pAC, IoC);
	SK_IN32(IoC, FCU_RD_DATA, &Val);

	return (Val);
}

/*****************************************************************************
 *
 * SkPflWriteDword - Writes a dword into the parallel flash
 *
 * Description:
 *	This function writes the specified dword into the specified parallel flash
 *	location. The parallel flash should have been put previously in write enable
 *	mode and the target sector erased for the access to succeed.
 *
 * Returns:
 *	Nothing
 */
static void SkPflWriteDword(
SK_AC	*pAC,
SK_IOC	IoC,
int		Part,		/* Info or main part */
SK_U32	Addr,		/* Address to write to */
SK_U32	Val)		/* New value to be written */
{
	Addr &= (1 << 20) - 1;
	Addr |= BIT_31;
	Addr |= (2 << 24);
	/* bits 31..29 == 100: write access to information memory. */
	/* bits 26..24 == 010: 32-bit access. */
	Addr |= (Part << 30);

	SK_OUT32(IoC, FCU_WR_DATA, Val);
	SK_OUT32(IoC, FCU_MEM_CTRL, Addr);
	SK_PFL_WAIT_FINISH_WR(pAC, IoC);
}

/*****************************************************************************
 *
 * SkPflReadChipId - Reads parallel flash IDs
 *
 * Description:
 *	This function reads the IDs of the parallel flash.
 *
 * Returns:
 *	Nothing (parallel flash manufacturer and device ID in parameters)
 */
static void SkPflReadChipId(
SK_AC	*pAC,
SK_IOC	IoC,
SK_U32	*pManId,
SK_U16	*pDevId)
{
	/* Put manufacturer and device ID into proper registers. */
	SkPflCommand(pAC, IoC, -1, pAC->pfl.pPflDev->opcodes.OpReadManDev, 0);

	/* bits 31..29 == 001: read access to parallel flash register. */
	/* bits 26..24 == 010: 32-bit access. */
	/* bits 7..0: register address */
	SK_OUT32(IoC, FCU_MEM_CTRL, BIT_29 | (2 << 24) | 0x18);
	SK_PFL_WAIT_FINISH_RD(pAC, IoC);
	SK_IN32(IoC, FCU_RD_DATA, pManId);
	*pManId = FL_SWAP32(*pManId);

	SK_OUT32(IoC, FCU_MEM_CTRL, BIT_29 | (2 << 24) | 0x1C);
	SK_PFL_WAIT_FINISH_RD(pAC, IoC);
	SK_IN16(IoC, FCU_RD_DATA, pDevId);
	*pDevId = FL_SWAP16(*pDevId);

	return;
}

/*****************************************************************************
 *
 * SkPflSetDevPtr - Sets a pointer to the correct device table entry
 *
 * Description:
 *	This function identifies the parallel flash device and sets
 *	the pointer to its device table entry.
 *
 * Returns:
 *	Nothing
 */
static void SkPflSetDevPtr(
SK_AC	*pAC,
SK_IOC	IoC)
{
	SK_U32	i;
	SK_U32	ManId;
	SK_U16	DevId;

	/* search for parallel flash in device table */
	for (i = 0; i < (sizeof(SkPflDevTable) / sizeof(SkPflDevTable[0])); i++) {

		/* Used by e.g. SkPflReadChipId(). */
		pAC->pfl.pPflDev = &SkPflDevTable[i];

		SkPflReadChipId(pAC, IoC, &ManId, &DevId);
		if ((SkPflDevTable[i].ManId == ManId) &&
			(SkPflDevTable[i].DevId == DevId)) {
			fl_print("\nFlash Device: %s ", SkPflDevTable[i].pDevName);
			fl_print("(VID 0x%08x, ", ManId);
			fl_print("DID 0x%04x)\n", DevId);
			return;
		}
	}

	pAC->pfl.pPflDev = NULL;
	return;
}

/*****************************************************************************
 *
 * SkPflCheck - Determines whether a parallel flash is present
 *
 * Description:
 *	This function determines whether a parallel flash is present.
 *
 * Returns:
 *	0	No parallel flash
 *	1	Parallel flash detected
 */
int SkPflCheck(
SK_AC	*pAC,
SK_IOC	IoC)
{
#ifdef XXX
	unsigned long opcodes;
#endif	/* XXX */

	SK_IN8(IoC, B2_CHIP_ID, &pAC->pfl.YkChipId);
	if (pAC->pfl.YkChipId == CHIP_ID_YUKON_SUPR) {

		SkPflSetDevPtr(pAC, IoC);
		if (pAC->pfl.pPflDev == NULL) {
			/* unknown or no flash */
			fl_print("\nFlash device\t: none\n");
			return (0);
		}

#ifdef XXX
		/*
		 * set the opcodes for the SPI flash found
		 */
		SK_IN32(IoC, SPI_Y2_OPCODE_REG1, &opcodes);

		opcodes &= 0x000000ffL;
		opcodes |= ((((unsigned long)(pAC->pfl.pPflDev->opcodes.op_read)) << 8) |
					(((unsigned long)(pAC->pfl.pPflDev->opcodes.op_read_id)) << 16) |
					(((unsigned long)(pAC->pfl.pPflDev->opcodes.op_read_status)) << 24));

		SK_OUT32(IoC, SPI_Y2_OPCODE_REG1, opcodes);

		opcodes = (((unsigned long)(pAC->pfl.pPflDev->opcodes.op_write_enable)) |
				   (((unsigned long)(pAC->pfl.pPflDev->opcodes.op_write)) << 8) |
				   (((unsigned long)(pAC->pfl.pPflDev->opcodes.op_sector_erase)) << 16) |
				   (((unsigned long)(pAC->pfl.pPflDev->opcodes.op_chip_erase)) << 24));

		SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, opcodes);
#endif	/* XXX */

		return (1);
	}

	return (0);
}

/*****************************************************************************
 *
 * SkPflDoManage - Reads, verifies, writes, or erases parallel flash
 *
 * Description:
 *	This function reads from parallel flash, verifies data, writes to
 *	parallel flash and verifies data, or erases parts of the parallel flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int SkPflDoManage(
SK_AC	*pAC,
SK_IOC	IoC,
int		Part,	/* SK_PFL_PART_INFO or SK_PFL_PART_MAIN */
SK_U8	*pData,	/* data buffer - might be NULL for action SK_PFL_ERASE only */
SK_U32	Offs,	/* start offset in flash for action - must be a multiple of 4 */
SK_U32	Len,	/* length in parallel flash - must be a multiple of 4 */
int		Action)	/* SK_PFL_READ / SK_PFL_VERIFY / SK_PFL_WRITE / SK_PFL_ERASE */
{
	SK_U32	Addr;
	SK_U32	Addr2;
	SK_U32	FplData;
	SK_U32	*pFplData;
	SK_U32	Progress;
	SK_U32	LastProgress;
	int		RetVal;
	SK_BOOL	Fill64ByteBlock;

	RetVal = 0;
	Fill64ByteBlock = SK_FALSE;

	Progress = LastProgress = 0;

#ifdef VCPU
	fl_print("SkPflDoManage() action: %d, pData: %08lx, Part: %d, Offs: %08lx, "
		"Len: %d.\n", Action, pData, Part, Offs, Len);
#endif	/* VCPU */

	if (Action == SK_PFL_ERASE) {
		/* erase the affected sectors */
		fl_print("Erasing: ");
		RetVal = SkPflErase(pAC, IoC, Part, Offs, Len);
	}
	else {
		if (Action == SK_PFL_WRITE) {
#ifdef SK_LINUX
			SK_FLASH_RESCHED();
#else /* !SK_LINUX */
			fl_print("Writing: ");
#endif /* !SK_LINUX */

			if ((Offs & 63) != 0) {
				/*
				 * Fill the start of the on-chip 64-byte buffer
				 * with data from flash if writing starts later.
				 */
				for (Addr2 = Offs & ~63; Addr2 < Offs; Addr2 += 4) {
					/* Read a dword from parallel flash. */
					FplData = SkPflReadDword(pAC, IoC, Part, Addr2);
				}
			}

			/*
			 * If the data does not fill a single 64-byte block
			 * to the end, remember to fill the end of the
			 * on-chip 64-byte buffer with data from flash.
			 */
			Fill64ByteBlock = (Offs + Len - (Offs & ~63) < 64);
		}
		else if (Action == SK_PFL_READ) {
			fl_print("Reading: ");
		}
		else if (Action == SK_PFL_VERIFY) {
			fl_print("Verify : ");
		}
		for (Addr = Offs, pFplData = (SK_U32 *) pData;
			Addr < Offs + Len; Addr += 4, pFplData++) {

			Progress = ((Addr - Offs) * 100) / Len;

			if ((Progress - LastProgress) >= 10) {
				fl_print(".");
				LastProgress += 10;
			}

			switch (Action) {
			case SK_PFL_READ:
				/* Read a dword from parallel flash. */
				FplData = SkPflReadDword(pAC, IoC, Part, Addr);
				*pFplData = FplData;
				break;

			case SK_PFL_VERIFY:
				/* Read and verify dword from parallel flash. */
				FplData = SkPflReadDword(pAC, IoC, Part, Addr);
				if (FplData != *pFplData) {
					fl_print("\n*** Verify error in parallel flash at address "
						"0x%08lx, is %08lx, should be %08lx\n",
						Addr, FplData, *pFplData);
					RetVal = 1;
				}
				break;

			case SK_PFL_WRITE:
				if (Fill64ByteBlock) {
					/*
					 * Fill the end of the on-chip
					 * 64-byte buffer with data from flash.
					 */
					for (Addr2 = Offs + Len; Addr2 < ((Offs + Len + 63) & ~63);
						Addr2 += 4) {
						/* Read a dword from parallel flash. */
						FplData = SkPflReadDword(pAC, IoC, Part, Addr2);
					}
					Fill64ByteBlock = SK_FALSE;
				}

				/* Write a dword to the parallel flash. */
				FplData = *pFplData;
				SkPflWriteDword(pAC, IoC, Part, Addr, FplData);
				if ((Addr + 4 >= Offs + Len) || (((Addr + 4) & 63) == 0)) {
					/*
					 * Last DWord of buffer or last DWord
					 * of a 64-byte block was written.
					 */
					if (Part == SK_PFL_PART_MAIN) {
						SkPflCommand(pAC, IoC, Part,
							pAC->pfl.pPflDev->opcodes.OpProgram, 0);
					}
					else {
						SkPflCommand(pAC, IoC, Part,
							pAC->pfl.pPflDev->opcodes.OpInfoProgram, 0);
					}

					if ((Addr + 4 < Offs + Len) &&
						(Offs + Len - (Addr + 4) < 64)) {
						/*
						 * The last 64-byte block is started now -
						 * and it is incomplete. So remember to fill
						 * the end of the on-chip 64-byte buffer with
						 * data from flash.
						 */
						Fill64ByteBlock = SK_TRUE;
					}
				}
				break;
			}	/* switch */

			if (RetVal != 0) {
				break;
			}
		}	/* for */
	}

	fl_print("\tDone\n");

	if (Action == SK_PFL_WRITE) {
		RetVal = SkPflDoManage(pAC, IoC, Part, pData, Offs, Len, SK_PFL_VERIFY);
	}

	return (RetVal);
}

/*****************************************************************************
 *
 * SkPflManage - Reads, verifies, writes, or erases main part
 *
 * Description:
 *	This function reads from the main part of the parallel flash, verifies data,
 *	writes to the main part of the parallel flash and verifies data, or erases
 *	parts of the main part of parallel flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int SkPflManage(
SK_AC	*pAC,
SK_IOC	IoC,
SK_U8	*pData,	/* data buffer - might be NULL for action SK_PFL_ERASE only */
SK_U32	Offs,	/* start offset in flash for action - must be a multiple of 4 */
SK_U32	Len,	/* length in parallel flash - must be a multiple of 4 */
int		Action)	/* SK_PFL_READ / SK_PFL_VERIFY / SK_PFL_WRITE / SK_PFL_ERASE */
{
	SK_U32	FSize;

	if ((pAC->pfl.pPflDev == NULL) || ((Offs & 3) != 0) || ((Len & 3) != 0)) {
		return (1);
	}

	if ((pData == NULL) && (Action != SK_PFL_ERASE)) {
		return (1);
	}

	FSize = pAC->pfl.pPflDev->NumSectors[SK_PFL_PART_MAIN] *
		pAC->pfl.pPflDev->SectorSize[SK_PFL_PART_MAIN];
	if (Offs + Len > FSize) {
		return (1);
	}

	return (SkPflDoManage(pAC, IoC, SK_PFL_PART_MAIN, pData, Offs, Len, Action));
}	/* SkPflManage */

/*****************************************************************************
 *
 * SkPflManage - Reads, verifies, writes, or erases info part
 *
 * Description:
 *	This function reads from the info part of the parallel flash, verifies data,
 *	writes to the info part of the parallel flash and verifies data, or erases
 *	parts of the info part of parallel flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int SkPflManageInfo(
SK_AC	*pAC,
SK_IOC	IoC,
SK_U8	*pData,	/* data buffer - might be NULL for action SK_PFL_ERASE only */
SK_U32	Offs,	/* start offset in flash for action - must be a multiple of 4 */
SK_U32	Len,	/* length in parallel flash - must be a multiple of 4 */
int		Action)	/* SK_PFL_READ / SK_PFL_VERIFY / SK_PFL_WRITE / SK_PFL_ERASE */
{
	SK_U32	FSize;

	if ((pAC->pfl.pPflDev == NULL) || ((Offs & 3) != 0) || ((Len & 3) != 0)) {
		return (1);
	}

	if ((pData == NULL) && (Action != SK_PFL_ERASE)) {
		return (1);
	}

	FSize = pAC->pfl.pPflDev->NumSectors[SK_PFL_PART_INFO] *
		pAC->pfl.pPflDev->SectorSize[SK_PFL_PART_INFO];
	if (Offs + Len > FSize) {
		return (1);
	}

	return (SkPflDoManage(pAC, IoC, SK_PFL_PART_INFO, pData, Offs, Len, Action));
}	/* SkPflManageInfo */

#ifdef XXX

#ifndef SK_PFL_NO_UPDATE

/*****************************************************************************
 *
 * SkPflUpdateConfig - Updates part of the config area in the parallel flash
 *
 * Description:
 *	This function updates part of configuration area in the parallel flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int SkPflUpdateConfig(
SK_AC	*pAC,
SK_IOC	IoC,
SK_U8	*pData,	/* data buffer */
SK_U32	Offs,	/* start offset in parallel flash (config area) for operation */
SK_U32	Len)	/* length of changing data */
{
	SK_U8	*pSpiBuf;
	SK_U32	SectorSize;
	SK_U32	StartSector;
	SK_U32	EndSector;
	int		i;

	/* RARAsafe */
	return (1);

	/* RARA or SK_PFL_PART_MAIN? */
	SectorSize = pAC->pfl.pPflDev->SectorSize[SK_PFL_PART_INFO];

	/* Determine the affected sectors. */
	StartSector = Offs / SectorSize;
	EndSector   = (Offs + Len - 1) / SectorSize;

	/* Allocate the necessary memory to temporary save the affected sectors. */
	pSpiBuf = spi_malloc(((EndSector - StartSector) + 1) * SectorSize);

	if (pSpiBuf == NULL) {
		return (51);
	}

	/* Read out the affected sectors. */
	if (SkPflManage(pAC, IoC, pSpiBuf, StartSector * SectorSize,
		((EndSector - StartSector) + 1) * SectorSize, SPI_READ)) {

		spi_free(pSpiBuf);
		return (1);
	}

	/* Update the just read out data. */
	for (i = 0; i < Len; i++) {
		pSpiBuf[Offs + i - SPI_LSECT_OFF] = pData[i];
	}

	/* Erase the affected sectors. */
	if (SkPflErase(pAC, IoC, StartSector * SectorSize,
		((EndSector - StartSector) + 1) * SectorSize)) {

		spi_free(pSpiBuf);
		return (7);
	}

	/* Write the updated data back to the flash. */
	if (SkPflManage(pAC, IoC, pSpiBuf, StartSector * SectorSize,
		((EndSector - StartSector) + 1) * SectorSize, SPI_WRITE)) {

		spi_free(pSpiBuf);
		return (8);
	}

	spi_free(pSpiBuf);
	return (0);
}

#endif	/* !SK_PFL_NO_UPDATE */

/*****************************************************************************
 *
 * SkPflVpdTransfer - Reads or updates data in the VPD area in the parallel flash
 *
 * Description:
 *	This function reads or updates data in the VPD area in the parallel flash.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int SkPflVpdTransfer(
SK_AC	*pAC,
SK_IOC	IoC,
SK_U8	*pData,	/* data buffer */
SK_U32	Addr,	/* VPD start address */
SK_U32	Len,	/* number of bytes to read or write */
int		Dir)	/* transfer direction may be VPD_READ or VPD_WRITE */
{
	if (Dir == VPD_READ) {
		return (SkPflManage/SkPflManageInfo(pAC, IoC, pBuf,
			SPI_VPD_OFF + Addr, Len, SPI_READ));
	}
	else {
#ifndef SK_PFL_NO_UPDATE
		return (SkPflUpdateConfig(pAC, IoC, pBuf, SPI_VPD_OFF + Addr, Len));
#else	/* SK_PFL_NO_UPDATE */
		return (1);
#endif	/* SK_PFL_NO_UPDATE */
	}
}

#endif	/* XXX */

/* End of File */
