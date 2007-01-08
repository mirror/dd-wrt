/************************************************************************/
/*                                                                      */
/*  AMD CFI Enabled Flash Memory Drivers                                */
/*  File name: CFIFLASH.H                                               */
/*  Revision:  1.0  5/07/98                                             */
/*                                                                      */
/* Copyright (c) 1998 ADVANCED MICRO DEVICES, INC. All Rights Reserved. */
/* This software is unpublished and contains the trade secrets and      */
/* confidential proprietary information of AMD. Unless otherwise        */
/* provided in the Software Agreement associated herewith, it is        */
/* licensed in confidence "AS IS" and is not to be reproduced in whole  */
/* or part by any means except for backup. Use, duplication, or         */
/* disclosure by the Government is subject to the restrictions in       */
/* paragraph (b) (3) (B) of the Rights in Technical Data and Computer   */
/* Software clause in DFAR 52.227-7013 (a) (Oct 1988).                  */
/* Software owned by                                                    */
/* Advanced Micro Devices, Inc.,                                        */
/* One AMD Place,                                                       */
/* P.O. Box 3453                                                        */
/* Sunnyvale, CA 94088-3453.                                            */
/************************************************************************/
/*  This software constitutes a basic shell of source code for          */
/*  programming all AMD Flash components. AMD                           */
/*  will not be responsible for misuse or illegal use of this           */
/*  software for devices not supported herein. AMD is providing         */
/*  this source code "AS IS" and will not be responsible for            */
/*  issues arising from incorrect user implementation of the            */
/*  source code herein. It is the user's responsibility to              */
/*  properly design-in this source code.                                */
/*                                                                      */ 
/************************************************************************/
#ifndef _CFIFLASH_H
#define _CFIFLASH_H


/* include board/CPU specific definitions */
#include "bcmtypes.h"
#include "board.h"

#define FLASH_BASE_ADDR_REG FLASH_BASE

#ifndef NULL
#define NULL 0
#endif

#define MAXSECTORS  1024      /* maximum number of sectors supported */

/* A structure for identifying a flash part.  There is one for each
 * of the flash part definitions.  We need to keep track of the
 * sector organization, the address register used, and the size
 * of the sectors.
 */
struct flashinfo {
	 char *name;         /* "Am29DL800T", etc. */
	 unsigned long addr; /* physical address, once translated */
	 int areg;           /* Can be set to zero for all parts */
	 int nsect;          /* # of sectors -- 19 in LV, 22 in DL */
	 int bank1start;     /* first sector # in bank 1 */
	 int bank2start;     /* first sector # in bank 2, if DL part */
 struct {
	long size;           /* # of bytes in this sector */
	long base;           /* offset from beginning of device */
	int bank;            /* 1 or 2 for DL; 1 for LV */
	 } sec[MAXSECTORS];  /* per-sector info */
};

/*
 * This structure holds all CFI query information as defined
 * in the JEDEC standard. All information up to 
 * primary_extended_query is standard among all manufactures
 * with CFI enabled devices.
 */

struct cfi_query {
	int num_erase_blocks;		/* Number of sector defs. */
	struct {
	  unsigned long sector_size;	/* byte size of sector */
	  int num_sectors;		/* Num sectors of this size */
	} erase_block[8];		/* Max of 256, but 8 is good */
};

/* Standard Boolean declarations */
#define TRUE 				1
#define FALSE 				0

/* Define different type of flash */
#define FLASH_UNDEFINED 0
#define FLASH_AMD       1
#define FLASH_INTEL     2
#define FLASH_SST       3

/* Command codes for the flash_command routine */
#define FLASH_RESET     0       /* reset to read mode */
#define FLASH_READ_ID   1       /* read device ID */
#define FLASH_CFIQUERY  2       /* CFI query */
#define FLASH_UB        3       /* go into unlock bypass mode */
#define FLASH_PROG      4       /* program a word */
#define FLASH_UBRESET   5       /* reset to read mode from unlock bypass mode */
#define FLASH_SERASE    6       /* sector erase */

/* Return codes from flash_status */
#define STATUS_READY    0       /* ready for action */
#define STATUS_TIMEOUT  1       /* operation timed out */

/* A list of AMD compatible device ID's - add others as needed */
#define ID_AM29DL800T   0x224A
#define ID_AM29DL800B   0x22CB
#define ID_AM29LV800T   0x22DA
#define ID_AM29LV800B   0x225B
#define ID_AM29LV400B   0x22BA

#define ID_AM29LV160B   0x2249
#define ID_AM29LV160T   0x22C4

#define ID_AM29LV320T   0x22F6
#define ID_MX29LV320AT  0x22A7
#define ID_AM29LV320B   0x22F9
#define ID_MX29LV320AB  0x22A8

#define ID_AM29LV320M   0x227E
#define ID_AM29LV320MB  0x2200
#define ID_AM29LV320MT  0x2201

#define ID_SST39VF1601  0x234B
#define ID_SST39VF3201  0x235B

/* A list of Intel compatible device ID's - add others as needed */
#define ID_I28F160C3T   0x88C2
#define ID_I28F160C3B   0x88C3
#define ID_I28F320C3T   0x88C4
#define ID_I28F320C3B   0x88C5

extern byte flash_init(void);
extern int flash_write_buf(WORD sector, int offset, byte *buffer, int numbytes);
extern int flash_read_buf(WORD sector, int offset, byte *buffer, int numbytes);
extern byte flash_sector_erase_int(WORD sector);
extern int flash_get_numsectors(void);
extern int flash_get_sector_size(WORD sector);
extern int flash_get_total_size(void);
extern unsigned char *flash_get_memptr(WORD sector);
extern int flash_get_blk(int addr);


#endif
