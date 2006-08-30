/************************************************************************/
/*                                                                      */
/*  AMD CFI Enabled Flash Memory Drivers                                */
/*  File name: CFIFLASH.C                                               */
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
#ifdef _CFE_                                                
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "cfe_timer.h"
#define printk  printf
#else       // linux
#include <linux/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#endif

#include "cfiflash.h"

static int flash_wait(WORD sector, int offset, UINT16 data);
static UINT16 flash_get_device_id(void);
static int flash_get_cfi(struct cfi_query *query, UINT16 *cfi_struct, int flashFamily);
static int flash_write(WORD sector, int offset, byte *buf, int nbytes);
static void flash_command(int command, WORD sector, int offset, UINT16 data);

/*********************************************************************/
/* 'meminfo' should be a pointer, but most C compilers will not      */
/* allocate static storage for a pointer without calling             */
/* non-portable functions such as 'new'.  We also want to avoid      */
/* the overhead of passing this pointer for every driver call.       */
/* Systems with limited heap space will need to do this.             */
/*********************************************************************/
struct flashinfo meminfo; /* Flash information structure */
static int flashFamily = FLASH_UNDEFINED;
static int totalSize = 0;
static struct cfi_query query;

static UINT16 cfi_data_struct_29W160[] = {
    0x0020, 0x0049, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0x0051, 0x0052, 0x0059, 0x0002, 0x0000, 0x0040, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0027, 0x0036, 0x0000, 0x0000, 0x0004,
    0x0000, 0x000a, 0x0000, 0x0004, 0x0000, 0x0003, 0x0000, 0x0015,
    0x0002, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0000, 0x0040,
    0x0000, 0x0001, 0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0080,
    0x0000, 0x001e, 0x0000, 0x0000, 0x0001, 0xffff, 0xffff, 0xffff,
    0x0050, 0x0052, 0x0049, 0x0031, 0x0030, 0x0000, 0x0002, 0x0001,
    0x0001, 0x0004, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0x0002,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0x0888, 0x252b, 0x8c84, 0x7dbc, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
    0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};


/*********************************************************************/
/* Init_flash is used to build a sector table from the information   */
/* provided through the CFI query.  This information is translated   */
/* from erase_block information to base:offset information for each  */
/* individual sector. This information is then stored in the meminfo */
/* structure, and used throughout the driver to access sector        */
/* information.                                                      */
/*                                                                   */
/* This is more efficient than deriving the sector base:offset       */
/* information every time the memory map switches (since on the      */
/* development platform can only map 64k at a time).  If the entire  */
/* flash memory array can be mapped in, then the addition static     */
/* allocation for the meminfo structure can be eliminated, but the   */
/* drivers will have to be re-written.                               */
/*                                                                   */
/* The meminfo struct occupies 653 bytes of heap space, depending    */
/* on the value of the define MAXSECTORS.  Adjust to suit            */
/* application                                                       */ 
/*********************************************************************/
byte flash_init(void)
{
    int i=0, j=0, count=0;
    int basecount=0L;
    UINT16 device_id;
    int flipCFIGeometry = FALSE;

    /* First, assume
    * a single 8k sector for sector 0.  This is to allow
    * the system to perform memory mapping to the device,
    * even though the actual physical layout is unknown.
    * Once mapped in, the CFI query will produce all
    * relevant information.
    */
    meminfo.addr = 0L;
    meminfo.areg = 0;
    meminfo.nsect = 1;
    meminfo.bank1start = 0;
    meminfo.bank2start = 0;
    
    meminfo.sec[0].size = 8192;
    meminfo.sec[0].base = 0x00000;
    meminfo.sec[0].bank = 1;
        
    flash_command(FLASH_RESET, 0, 0, 0);

    device_id = flash_get_device_id();

    switch (device_id) {
        case ID_I28F160C3B:
        case ID_I28F320C3B:
        case ID_I28F160C3T:
        case ID_I28F320C3T:
            flashFamily = FLASH_INTEL;
            break;
        case ID_AM29DL800B:
        case ID_AM29LV800B:
        case ID_AM29LV400B:   
        case ID_AM29LV160B:
        case ID_AM29LV320B:
        case ID_MX29LV320AB:
        case ID_AM29LV320MB:
        case ID_AM29DL800T:
        case ID_AM29LV800T:
        case ID_AM29LV160T:
        case ID_AM29LV320T:
        case ID_MX29LV320AT:
        case ID_AM29LV320MT:
            flashFamily = FLASH_AMD;
            break;
        case ID_SST39VF1601:
		case ID_SST39VF3201:
            flashFamily = FLASH_SST;
            break;
        default:
            printk("Flash memory not supported!  Device id = %x\n", device_id);
            return -1;           
    }

    if (flash_get_cfi(&query, 0, flashFamily) == -1) {
        switch(device_id) {
        case ID_AM29LV160T:
        case ID_AM29LV160B:
            flash_get_cfi(&query, cfi_data_struct_29W160, flashFamily);
            break;
        default:
            printk("CFI data structure not found. Device id = %x\n", device_id);
            return -1;           
        }
    }

    // need to determine if it top or bottom boot here
    switch (device_id)
    {
        case ID_AM29DL800B:
        case ID_AM29LV800B:
        case ID_AM29LV400B:   
        case ID_AM29LV160B:
        case ID_AM29LV320B:
        case ID_MX29LV320AB:
        case ID_AM29LV320MB:
        case ID_I28F160C3B:
        case ID_I28F320C3B:
        case ID_I28F160C3T:
        case ID_I28F320C3T:
		case ID_SST39VF1601:
		case ID_SST39VF3201:
            flipCFIGeometry = FALSE;
            break;
        case ID_AM29DL800T:
        case ID_AM29LV800T:
        case ID_AM29LV160T:
        case ID_AM29LV320T:
        case ID_MX29LV320AT:
        case ID_AM29LV320MT:
            flipCFIGeometry = TRUE;
            break;
        default:
            printk("Flash memory not supported!  Device id = %x\n", device_id);
            return -1;           
    }

    count=0;basecount=0L;

    if (!flipCFIGeometry)
    {
       for (i=0; i<query.num_erase_blocks; i++) {
            for(j=0; j<query.erase_block[i].num_sectors; j++) {
                meminfo.sec[count].size = (int) query.erase_block[i].sector_size;
                meminfo.sec[count].base = (int) basecount;
                basecount += (int) query.erase_block[i].sector_size;
                count++;
            }
        }
    }
    else
    {
        for (i = (query.num_erase_blocks - 1); i >= 0; i--) {
            for(j=0; j<query.erase_block[i].num_sectors; j++) {
                meminfo.sec[count].size = (int) query.erase_block[i].sector_size;
                meminfo.sec[count].base = (int) basecount;
                basecount += (int) query.erase_block[i].sector_size;
				count++;
            }
        }
    }

    meminfo.nsect = count;
    totalSize = meminfo.sec[count-1].base + meminfo.sec[count-1].size;
    return (0);
}

/*********************************************************************/
/* Flash_sector_erase_int() is identical to flash_sector_erase(),    */
/* except it will wait until the erase is completed before returning */
/* control to the calling function.  This can be used in cases which */
/* require the program to hold until a sector is erased, without     */
/* adding the wait check external to this function.                  */
/*********************************************************************/
byte flash_sector_erase_int(WORD sector)
{
    int i;

    for( i = 0; i < 3; i++ ) {
        flash_command(FLASH_SERASE, sector, 0, 0);
        if (flash_wait(sector, 0, 0xffff) == STATUS_READY)
            break;
    }

    return(1);
}

/*********************************************************************/
/* flash_read_buf() reads buffer of data from the specified          */
/* offset from the sector parameter.                                 */
/*********************************************************************/
int flash_read_buf(WORD sector, int offset,
                        byte *buffer, int numbytes)
{
    byte *fwp;

    fwp = (byte *)flash_get_memptr(sector);

	while (numbytes) {
		*buffer++ = *(fwp + offset);
		numbytes--;
		fwp++;
    }

    return (1);
}

/*********************************************************************/
/* flash_write_buf() utilizes                                        */
/* the unlock bypass mode of the flash device.  This can remove      */
/* significant overhead from the bulk programming operation, and     */
/* when programming bulk data a sizeable performance increase can be */
/* observed.                                                         */
/*********************************************************************/
int flash_write_buf(WORD sector, int offset, byte *buffer, int numbytes)
{
    int ret = -1;
    int i;
    unsigned char *p = flash_get_memptr(sector) + offset;

    /* After writing the flash block, compare the contents to the source
     * buffer.  Try to write the sector successfully up to three times.
     */
    for( i = 0; i < 3; i++ ) {
        ret = flash_write(sector, offset, buffer, numbytes);
        if( !memcmp( p, buffer, numbytes ) )
            break;
        /* Erase and try again */
        flash_sector_erase_int(sector);
        ret = -1;
    }

    if( ret == -1 )
        printk( "Flash write error.  Verify failed\n" );

    return( ret );
}

/*********************************************************************/
/* Usefull funtion to return the number of sectors in the device.    */
/* Can be used for functions which need to loop among all the        */
/* sectors, or wish to know the number of the last sector.           */
/*********************************************************************/
int flash_get_numsectors(void)
{
    return meminfo.nsect;
}

/*********************************************************************/
/* flash_get_sector_size() is provided for cases in which the size   */
/* of a sector is required by a host application.  The sector size   */
/* (in bytes) is returned in the data location pointed to by the     */
/* 'size' parameter.                                                 */
/*********************************************************************/
int flash_get_sector_size(WORD sector)
{
    return meminfo.sec[sector].size;
}

/*********************************************************************/
/* The purpose of flash_get_memptr() is to return a memory pointer   */
/* which points to the beginning of memory space allocated for the   */
/* flash.  All function pointers are then referenced from this       */
/* pointer. 							     */
/*                                                                   */
/* Different systems will implement this in different ways:          */
/* possibilities include:                                            */
/*  - A direct memory pointer                                        */
/*  - A pointer to a memory map                                      */
/*  - A pointer to a hardware port from which the linear             */
/*    address is translated                                          */
/*  - Output of an MMU function / service                            */
/*                                                                   */
/* Also note that this function expects the pointer to a specific    */
/* sector of the device.  This can be provided by dereferencing      */
/* the pointer from a translated offset of the sector from a         */
/* global base pointer (e.g. flashptr = base_pointer + sector_offset)*/
/*                                                                   */
/* Important: Many AMD flash devices need both bank and or sector    */
/* address bits to be correctly set (bank address bits are A18-A16,  */
/* and sector address bits are A18-A12, or A12-A15).  Flash parts    */
/* which do not need these bits will ignore them, so it is safe to   */
/* assume that every part will require these bits to be set.         */
/*********************************************************************/
unsigned char *flash_get_memptr(WORD sector)
{
	unsigned char *memptr = (unsigned char*)(FLASH_BASE_ADDR_REG + meminfo.sec[sector].base);

	return (memptr);
}

/*********************************************************************/
/* The purpose of flash_get_blk() is to return a the block number    */
/* for a given memory address.                                       */
/*********************************************************************/
int flash_get_blk(int addr)
{
    int blk_start, i;
    int last_blk = flash_get_numsectors();
    int relative_addr = addr - (int) FLASH_BASE_ADDR_REG;

    for(blk_start=0, i=0; i < relative_addr && blk_start < last_blk; blk_start++)
        i += flash_get_sector_size(blk_start);

    if( i > relative_addr )
    {
        blk_start--;        // last blk, dec by 1
    }
    else
        if( blk_start == last_blk )
        {
            printk("Address is too big.\n");
            blk_start = -1;
        }

    return( blk_start );
}

/************************************************************************/
/* The purpose of flash_get_total_size() is to return the total size of */
/* the flash                                                            */
/************************************************************************/
int flash_get_total_size()
{
    return totalSize;
}

/*********************************************************************/
/* Flash_command() is the main driver function.  It performs         */
/* every possible command available to AMD B revision                */
/* flash parts. Note that this command is not used directly, but     */
/* rather called through the API wrapper functions provided below.   */
/*********************************************************************/
static void flash_command(int command, WORD sector, int offset, UINT16 data)
{
    volatile UINT16 *flashptr;
    volatile UINT16 *flashbase;

    flashptr = (UINT16 *) flash_get_memptr(sector);
    flashbase = (UINT16 *) flash_get_memptr(0);
    
    switch (flashFamily) {
    case FLASH_UNDEFINED:
        /* These commands should work for AMD, Intel and SST flashes */
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xF0;
            flashptr[0] = 0xFF;
            break;
        case FLASH_READ_ID:
			flashptr[0x5555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AAA] = 0x55;       /* unlock 2 */
            flashptr[0x5555] = 0x90;
            break;
        default:
            break;
        }
        break;
    case FLASH_AMD:
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xF0;
            break;
        case FLASH_READ_ID:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashptr[0x55] = 0x98;
            break;
        case FLASH_UB:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x20;
            break;
        case FLASH_PROG:
            flashptr[0] = 0xA0;
            flashptr[offset/2] = data;
            break;
        case FLASH_UBRESET:
            flashptr[0] = 0x90;
            flashptr[0] = 0x00;
            break;
        case FLASH_SERASE:
            flashptr[0x555] = 0xAA;       /* unlock 1 */
            flashptr[0x2AA] = 0x55;       /* unlock 2 */
            flashptr[0x555] = 0x80;
            flashptr[0x555] = 0xAA;
            flashptr[0x2AA] = 0x55;
            flashptr[0] = 0x30;
            break;
        default:
            break;
        }
        break;
    case FLASH_INTEL:
        switch (command) {
        case FLASH_RESET:
            flashptr[0] = 0xFF;
            break;
        case FLASH_READ_ID:
            flashptr[0] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashptr[0] = 0x98;
            break;
        case FLASH_PROG:
            flashptr[0] = 0x40;
            flashptr[offset/2] = data;
            break;
        case FLASH_SERASE:
            flashptr[0] = 0x60;
            flashptr[0] = 0xD0;
            flashptr[0] = 0x20;
            flashptr[0] = 0xD0;
            break;
        default:
            break;
        }
        break;
    case FLASH_SST:
        switch (command) {
        case FLASH_RESET:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0xf0;
            break;
        case FLASH_READ_ID:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x90;
            break;
        case FLASH_CFIQUERY:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x98;
            break;
        case FLASH_UB:
            break;
        case FLASH_PROG:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0xa0;
            flashptr[offset/2] = data;
            break;
        case FLASH_UBRESET:
            break;
        case FLASH_SERASE:
            flashbase[0x5555] = 0xAA;       /* unlock 1 */
            flashbase[0x2AAA] = 0x55;       /* unlock 2 */
            flashbase[0x5555] = 0x80;
            flashbase[0x5555] = 0xAA;
            flashbase[0x2AAA] = 0x55;
            flashptr[0] = 0x30;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************/
/* flash_write extends the functionality of flash_program() by       */
/* providing an faster way to program multiple data words, without   */
/* needing the function overhead of looping algorithms which         */
/* program word by word.  This function utilizes fast pointers       */
/* to quickly loop through bulk data.                                */
/*********************************************************************/
static int flash_write(WORD sector, int offset, byte *buf, int nbytes)
{
    UINT16 *src;
    src = (UINT16 *)buf;

    if ((nbytes | offset) & 1) {
        return -1;
    }

    flash_command(FLASH_UB, 0, 0, 0);
    while (nbytes > 0) {
        flash_command(FLASH_PROG, sector, offset, *src);
        if (flash_wait(sector, offset, *src) != STATUS_READY)
            break;
        offset +=2;
        nbytes -=2;
        src++;
    }
    flash_command(FLASH_UBRESET, 0, 0, 0);
    
    return (byte*)src - buf;
}

/*********************************************************************/
/* flash_wait utilizes the DQ6, DQ5, and DQ2 polling algorithms      */
/* described in the flash data book.  It can quickly ascertain the   */
/* operational status of the flash device, and return an             */
/* appropriate status code (defined in flash.h)                      */
/*********************************************************************/
static int flash_wait(WORD sector, int offset, UINT16 data)
{
    volatile UINT16 *flashptr; /* flash window */
    UINT16 d1;

    flashptr = (UINT16 *) flash_get_memptr(sector);

    if (flashFamily == FLASH_AMD || flashFamily == FLASH_SST) {
#if defined(_BCM96338_) || defined(CONFIG_BCM96338)
        do {
            d1 = flashptr[offset/2];
            if (d1 == data)
                return STATUS_READY;
        } while (!(d1 & 0x20));

        d1 = flashptr[offset/2];

        if (d1 != data) {
            flash_command(FLASH_RESET, 0, 0, 0);
            return STATUS_TIMEOUT;
        }
#else
        do {
            d1 = *flashptr;    /* read data */
            d1 ^= *flashptr;   /* read it again and see what toggled */
            if (d1 == 0)       /* no toggles, nothing's happening */
                return STATUS_READY;
        } while (!(d1 & 0x20));

        d1 = *flashptr;        /* read data */
        d1 ^= *flashptr;   /* read it again and see what toggled */

        if (d1 != 0) {
            flash_command(FLASH_RESET, 0, 0, 0);
            return STATUS_TIMEOUT;
        }
#endif
    } else if (flashFamily == FLASH_INTEL) {
        flashptr[0] = 0x70;
        /* Wait for completion */
        while(!(*flashptr & 0x80));
        if (*flashptr & 0x30) {
            flashptr[0] = 0x50;
            flash_command(FLASH_RESET, 0, 0, 0);
            return STATUS_TIMEOUT;
        }
        flashptr[0] = 0x50;
        flash_command(FLASH_RESET, 0, 0, 0);
    }
    
    return STATUS_READY;
}

/*********************************************************************/
/* flash_get_device_id() will perform an autoselect sequence on the  */
/* flash device, and return the device id of the component.          */
/* This function automatically resets to read mode.                  */
/*********************************************************************/
static UINT16 flash_get_device_id()
{
    volatile UINT16 *fwp; /* flash window */
    UINT16 answer;
    
    fwp = (UINT16 *)flash_get_memptr(0);
    
    flash_command(FLASH_READ_ID, 0, 0, 0);
    answer = *(fwp + 1);
    if (answer == ID_AM29LV320M) {
        answer = *(fwp + 0xe);
        answer = *(fwp + 0xf);
    }
    
    flash_command(FLASH_RESET, 0, 0, 0);
    return( (UINT16) answer );
}

/*********************************************************************/
/* flash_get_cfi() is the main CFI workhorse function.  Due to it's  */
/* complexity and size it need only be called once upon              */
/* initializing the flash system.  Once it is called, all operations */
/* are performed by looking at the meminfo structure.                */
/* All possible care was made to make this algorithm as efficient as */
/* possible.  90% of all operations are memory reads, and all        */
/* calculations are done using bit-shifts when possible              */
/*********************************************************************/
static int flash_get_cfi(struct cfi_query *query, UINT16 *cfi_struct, int flashFamily)
{
    volatile UINT16 *fwp; /* flash window */
    int i=0;

    flash_command(FLASH_CFIQUERY, 0, 0, 0);
    
    if (cfi_struct == 0)
        fwp = (UINT16 *)flash_get_memptr(0);
    else
        fwp = cfi_struct;
    
    /* Initial house-cleaning */
    for(i=0; i < 8; i++) {
        query->erase_block[i].sector_size = 0;
        query->erase_block[i].num_sectors = 0;
    }
    
    /* If not 'QRY', then we dont have a CFI enabled device in the socket */
    if( fwp[0x10] != 'Q' &&
        fwp[0x11] != 'R' &&
        fwp[0x12] != 'Y') {
        flash_command(FLASH_RESET, 0, 0, 0);
        return(-1);
    }
    
	query->num_erase_blocks = fwp[0x2C];
	if(flashFamily == FLASH_SST)
		query->num_erase_blocks = 1;
	
    for(i=0; i < query->num_erase_blocks; i++) {
			query->erase_block[i].num_sectors = fwp[(0x2D+(4*i))] + (fwp[0x2E + (4*i)] << 8);
			query->erase_block[i].num_sectors++;
			query->erase_block[i].sector_size = 256 * (256 * fwp[(0x30+(4*i))] + fwp[(0x2F+(4*i))]);
    }
    
    flash_command(FLASH_RESET, 0, 0, 0);
    return(1);
}
