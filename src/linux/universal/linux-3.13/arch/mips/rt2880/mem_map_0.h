
/******************************************************************************
*
* (c) Copyright 1996-2001, Palmchip Corporation
*
* This document is an unpublished work protected under the copyright laws
* of the United States containing the confidential, proprietary and trade
* secret information of Palmchip Corporation. This document may not be
* copied or reproduced in any form whatsoever without the express written
* permission of Palmchip Corporation.
*
*******************************************************************************
*
*  File Name: mem_map_0.h
*     Author: Robin Bhagat 
*
*    Purpose: Contains the entire memory map of the PalmPak chip.
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
*    08/11/97  RWB   Created
*    03/20/00  JGU   Renamed from palmpak.h and moved processor-specific
*                    stack info to proc_stack.h
*    11/02/00  IST   Removed blocks not included in PalmPak 2.0
*    01/15/01  IST   Renamed to differentiate memory maps.
*    02/09/01  IST   Pulled out PalmPak chip regs into chip_reg_map.h
*
*
*******************************************************************************/

#ifndef MEM_MAP_0_H
#define MEM_MAP_0_H

/*=====================*
 *  Include Files      *
 *=====================*/

/*=====================*
 *  Defines            *
 *=====================*/


/*
**-------------------------------------------------------------------------- 
** High-level memory map definitions.
** NOTE: If you change the memory addresses, then change softpak.mak too.
**-------------------------------------------------------------------------- 
*/

/* ROM */
#define MAC_ROM_BASE	(0x00000000)
#define MAC_ROM_SIZE	(0x00200000)		/* 2MB reserved */
#define MAC_ROM_END	(MAC_ROM_BASE + MAC_ROM_SIZE)

#define ROM_BASE	(MAC_ROM_BASE)
#define ROM_SIZE	(0x00200000)		/* 2MB actual */
#define ROM_END		(ROM_BASE + ROM_SIZE)

/* When the REMAP is set in the system control block,
** 1st ISRAM-sized section of ROM is remapped to boot location of ISRAM.
** The following defines refer to the section of ROM which remains un-remapped.
*/
#define ROM_REMAPPED_BASE       (ISRAM_REMAPPED_END)
#define ROM_REMAPPED_SIZE       (ROM_SIZE - ISRAM_SIZE)
#define ROM_REMAPPED_END        (ROM_END)

/* May need a better way to (not) set this for testing hardware! */
#define ROM_BANK_IS_FLASH


/* ISRAM */
#define ISRAM_BOOT_BASE	(0x00200000)	        /* Internal SRAM at boot */
#define ISRAM_SIZE	(0x00002000)		/* 8KB actual */
#define ISRAM_BOOT_END	(ISRAM_BOOT_BASE + ISRAM_SIZE)

/* When the REMAP is set in the system control block,
** Int SRAM is remapped to the boot location of ROM
*/
#define ISRAM_REMAPPED_BASE	(ROM_BASE)
#define ISRAM_REMAPPED_END	(ISRAM_REMAPPED_BASE + ISRAM_SIZE)


/* PALMPAK */
#define PALMPAK_BASE	(0x00300000)		/* PalmPak ASIC regs */
#define PALMPAK_SIZE    (0x00010000)		/* 64KB */
#define PALMPAK_END	(PALMPAK_BASE + PALMPAK_SIZE)


/* Main SRAM */
/* Note: The palmpak memory map reserves 2M of the address space for SRAM,
   and this is reflected in the memory controller configuration, but the
   development board contains 512K. */
#define MAC_SRAM_BASE   (0x00400000)
#define MAC_SRAM_SIZE	(0x00200000)		/* 2MB reserved */
#define MAC_SRAM_END	(MAC_SRAM_BASE + MAC_SRAM_SIZE)

#define SRAM_BASE	MAC_SRAM_BASE
#ifdef DEV_BOARD_SRAM_SIZE
/* WARNING: Must be careful not to use SRAM_SIZE in HAL code,
   as that would cause the HAL to be configuration-specific
   due to the configuration-specific use of this #ifdef! */
#define SRAM_SIZE	DEV_BOARD_SRAM_SIZE
#else
#define SRAM_SIZE	(0x00200000)		/* 2MB actual */
#endif
#define SRAM_END	(SRAM_BASE + SRAM_SIZE)


/* SDRAM */
#ifdef USE_SDRAM
#define MAC_SDRAM_BASE  (0x01000000)
#define MAC_SDRAM_SIZE  (0x01000000)            /* 16MB reserved */
#define MAC_SDRAM_END   (MAC_SDRAM_BASE + MAC_SDRAM_SIZE)

#define SDRAM_BASE      (MAC_SDRAM_BASE)
#define SDRAM_SIZE      (MAC_SDRAM_SIZE)        /* 16MB actual */
#define SDRAM_END       (SDRAM_BASE + SDRAM_SIZE)
#endif


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* MEM_MAP_0_H */


/****************************************************************
 *
 * PalmPak 2.0 Chip Address Map
 * -----------------------------
 *
 * Normal mapping:
 * ---------------
 * 0000_0000 - 001F_FFFF : 2M ROM/Flash
 * 0020_0000 - 0020_1FFF : 8K Embedded SRAM (ISRAM)
 * 0020_2000 - 002F_FFFF : Reserved
 * 0030_0000 - 0030_FFFF : 64K Chip Registers**
 * 0031_0000 - 003F_FFFF : Reserved
 * 0040_0000 - 005F_FFFF : 2M SRAM
 * 0060_0000 - 00FF_FFFF : Reserved
 * 0100_0000 - 01FF_FFFF : 16M SDRAM
 * 0200_0000 - FFFF_FFFF : Reserved
 *
 * With remap set
 * --------------
 * 0000_0000 - 0000_1FFF : 8K Embedded SRAM (ISRAM)
 * 0000_2000 - 001F_FFFF : Remaining (2M minus 8K) ROM/Flash
 * 0020_0000 - 0020_1FFF : First 8K ROM/Flash
 * 0020_2000 - 002F_FFFF : Reserved
 * 0030_0000 - 0030_FFFF : 64K Chip Registers**
 * 0031_0000 - 003F_FFFF : Reserved
 * 0040_0000 - 005F_FFFF : 2M SRAM
 * 0060_0000 - 00FF_FFFF : Reserved
 * 0100_0000 - 01FF_FFFF : 16M SDRAM
 * 0200_0000 - FFFF_FFFF : Reserved
 *
 *
 * ** The Internal ASIC register space is broken up as follows:
 *    256 blocks x 256 regs/block x 4 bytes/reg = 64KB
 *
 ****************************************************************/

