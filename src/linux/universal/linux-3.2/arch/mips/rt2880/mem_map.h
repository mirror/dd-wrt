
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
*  File Name: mem_map.h
*     Author: Ian Thompson 
*
*    Purpose: Includes the correct memory map as defined in the makefile.
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
*    01/15/01  IST   Created.
*
*
*******************************************************************************/

#ifndef MEM_MAP_H
#define MEM_MAP_H

#include <asm/rt2880/rt_mmap.h>

/*=====================*
 *  Include Files      *
 *=====================*/

#include "mem_map_1fc0.h"
//#include "chip_reg_map.h"


/*=====================*
 *  Defines            *
 *=====================*/


#ifdef USE_CACHE
  // Overwrite some defines to specify virtual addresses instead of physical.
  // Also define uncached locations (*_NC).

  // Re-define MAC_ROM_BASE to KSEG0(MAC_ROM_BASE)
  #undef	MAC_ROM_BASE
  #define	MAC_ROM_BASE		(0x9fc00000)
  #define	MAC_ROM_BASE_NC		(0xbfc00000)

  // Re-define ROM_REMAPPED_BASE to KSEG0(ROM_REMAPPED_BASE)
  #undef	ROM_REMAPPED_BASE
  #define	ROM_REMAPPED_BASE	(0x9fc02000)
  #define	ROM_REMAPPED_BASE_NC	(0xbfc02000)

  // Re-define ISRAM_BOOT_BASE to KSEG1(ISRAM_BOOT_BASE)
  #undef	ISRAM_BOOT_BASE
  #define	ISRAM_BOOT_BASE		(0x80200000)
  #define	ISRAM_BOOT_BASE_NC	(0xa0200000)

  // Re-define ISRAM_REMAPPED_BASE to KSEG1(ISRAM_REMAPPED_BASE)
  #undef	ISRAM_REMAPPED_BASE
  #define	ISRAM_REMAPPED_BASE	(0x9fc00000)
  #define	ISRAM_REMAPPED_BASE_NC	(0xbfc00000)

  // Re-define PALMPAK_BASE to KSEG1(PALMPAK_BASE)
  #undef	PALMPAK_BASE
  #define	PALMPAK_BASE		(RALINK_SYSCTL_BASE)

  // Re-define MAC_SRAM_BASE to KSEG0(MAC_SRAM_BASE)
  #undef	MAC_SRAM_BASE
  #define	MAC_SRAM_BASE		(0x80000000)
  #define	MAC_SRAM_BASE_NC	(0xa0000000)

  // Re-define MAC_SDRAM_BASE to KSEG0(MAC_SDRAM_BASE)
  #undef	MAC_SDRAM_BASE
  #define	MAC_SDRAM_BASE		(0x81000000)
  #define	MAC_SDRAM_BASE_NC	(0xa1000000)

#endif /* USE_CACHE */


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/



#endif	// MEM_MAP_H
