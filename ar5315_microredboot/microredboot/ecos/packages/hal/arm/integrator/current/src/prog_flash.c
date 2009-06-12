//==========================================================================
//
//        prog_flash.c
//
//        ARM INTEGRATOR eval board FLASH program tool
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     gthomas
// Contributors:  Philippe Robin
// Date:          November 7, 2000
// Description:   Tool used to program onboard FLASH image
//####DESCRIPTIONEND####

//
// This program will program the FLASH on the PID board
// It is similar to 'flash' (which also downloads S records) but it always
// programs from a fixed buffer.  This is sufficient to load/update the GDB
// stubs on the board.
//

#include <pkgconf/libc.h>   // Configuration header

#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include <ctype.h>
#include <cyg/infra/testcase.h>
#include <sys/cstartup.h>
#include <cyg/hal/hal_integrator.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif


#define INVALID_FTYPE		0x00000000
#define UNKNOWN_FTYPE		0xFFFFFFFF
#define ATMEL_FTYPE		0x00000001
#define INTEL_FTYPE		0x00000002

#define FLASH_TYPE_MASK		(ATMEL_FTYPE | INTEL_FTYPE)

// On Some platforms Boot and program flash may be part of the same device
#define INTEGRATED_FTYPE	0x80000000
#define BOOT_FTYPE		0x40000000
#define APP_FTYPE		0x20000000

#define FLASH_USAGE_MASK	(BOOT_FTYPE | APP_FTYPE)

#define DEFAULT_FLASH_MASK 0xFFFFFFF8
#define FLASH_BLOCK_SIZE	0x00020000	// 128Kb
#define EPROM_BASE		0x20000000
#define EPROM_SIZE		0x00080000	// 512Kb
#define FLASH_BASE		0x24000000
#define FLASH_SIZE		0x02000000	// 32Mb

typedef struct flashType {
    char *base;			// Base Address of flash
    char *physicalBase;		// before mem initialisation
    unsigned int size;		// Size of flash, in bytes
    unsigned int type;		// Atmel / Intel (CFI) / Unknown
    unsigned int writeSize;	// Size of physical block
    unsigned int eraseSize;	// Size of block erase
    unsigned int logicalSize;	// Size of logical block
    char *ident;		// identification string
} tFlash;

tFlash Integrator_Flash[2] =
{
    {
	(char *)EPROM_BASE,		// Base Address of flash
	(char *)EPROM_BASE,		// Physical Address of flash
	EPROM_SIZE,			// Size of flash, in bytes (512K)
	BOOT_FTYPE | ATMEL_FTYPE,	// Flash type
	FLASH_BLOCK_SIZE,	// Size of physical block
	FLASH_BLOCK_SIZE,	// Size of block erase
	FLASH_BLOCK_SIZE,	// Size of logical block
	"Atmel",		// Null terminated Info string
    },
    {
	(char *)FLASH_BASE,	// Base Address of flash
	(char *)FLASH_BASE,	// Physical Address of flash
	FLASH_SIZE,		// Size of flash, in bytes
	APP_FTYPE | INTEL_FTYPE,	// Flash type
	FLASH_BLOCK_SIZE,	// Size of physical block
	FLASH_BLOCK_SIZE,	// Size of block erase
	FLASH_BLOCK_SIZE,	// Size of logical block
	"Intel 28F320S3",       // Null terminated Info string
   }
};


extern void diag_printf(const char *, ...);
int identify_FLASH(void);
void write_sector(int, char *);

char *flash_buffer =     (char *)0x60000;
char *flash_buffer_end = (char *)0x80000;

#ifdef BE_IMAGE
#define BUF(x) buf[x^3]
#else
#define BUF(x) buf[x]
#endif

// FUNCTIONS

externC void
cyg_package_start( void )
{
#ifdef CYGPKG_LIBC
    cyg_iso_c_start();
#else
    (void)main(0, NULL);
#endif
} // cyg_package_start()

int
main( int argc, char *argv[] )
{
    int i;

    diag_printf("FLASH here!\n");
    while (identify_FLASH() == 0) {
        diag_printf("... Please change FLASH jumper\n");
        cyg_thread_delay(5*100);
    }
    diag_printf("About to program FLASH using data at %x..%x\n", flash_buffer, flash_buffer_end);
    diag_printf("*** Press RESET now to abort!\n");
    cyg_thread_delay(5*100);
    diag_printf("\n");
    diag_printf("...Programming FLASH\n");

    i = 0;
    while (flash_buffer < flash_buffer_end) {
        write_sector(i++, flash_buffer);
        flash_buffer += 256;
    }
    diag_printf("All done!\n");
    while (1) ;
}

// Adapted from ARM sample code
#define SEQ_ADD1                0x5555
#define SEQ_ADD2                0x2aaa
#define START_CMD1              0xaaaaaaaa
#define START_CMD2              0x55555555
#define ID_CMD                  0x90909090
#define PROG_CMD                0xA0
#define STOP_CMD                0xf0f0f0f0

#define MAN_ATMEL               0x1F
#define ATMEL_AT29C040_ID       0X5B
#define ATMEL_AT29C040A_ID      0XA4
#define ATMEL_AT29C1024_ID      0X25
#define ATMEL_SECTOR_SIZE       256
#define ATMEL_MAX_SECTORS       2048

#define MAN_INTEL		0xB0
#define INTEL_28F320S3_ID	0xD4

int manuf_code, device_code;
int sector_size, max_no_of_sectors, word_mode;
volatile unsigned int *FLASH = (volatile unsigned int *)0x24000000;

int
identify_FLASH(void )
{
    unsigned int *ptr = (unsigned int *)FLASH;

    HAL_FLASH_WRITE_ENABLE();

    // Enter Software Product Identification Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = ID_CMD;

    // Wait at least 10ms
    cyg_thread_delay(4);

    // Read Manufacturer and device code from the device
    manuf_code   = *ptr++ & 0xff;
    device_code  = *ptr & 0xff;
    
    diag_printf("manuf: 0x%x, device: 0x%x\n", manuf_code, device_code);

    // Exit Software Product Identification Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = STOP_CMD;

    // Wait at least 10ms
    cyg_thread_delay(5);

    HAL_FLASH_WRITE_DISABLE();

    if (manuf_code != MAN_ATMEL || manuf_code != MAN_INTEL) {
        diag_printf ( "Error: Wrong Manufaturer: %02x\n", manuf_code );
        return (0);
    }

    switch (device_code) {
    case  ATMEL_AT29C040A_ID:
        diag_printf ("AT29C040A recognised\n");
        sector_size = ATMEL_SECTOR_SIZE;
        max_no_of_sectors = ATMEL_MAX_SECTORS;
        word_mode = FALSE;
        break;

    case  ATMEL_AT29C1024_ID:
        diag_printf ("AT29C1024 recognised\n");
        sector_size = ATMEL_SECTOR_SIZE;
        max_no_of_sectors = ATMEL_MAX_SECTORS;
        word_mode = TRUE;
        break;

    case  INTEL_28F320S3_ID:
        diag_printf ("INTEL_28F320S3 recognised\n");
        sector_size = FLASH_BLOCK_SIZE;
	//        max_no_of_sectors = ATMEL_MAX_SECTORS;
        word_mode = TRUE;
        break;

    default :
        diag_printf ( "Error: Unsupported device: %02x\n", device_code);
        return (0);
    }
    return (1);
}

void
write_sector(int num, char *buf)
{
    int i, cnt;
    volatile char *wrt = (volatile char *)&FLASH[num*sector_size];

//    diag_printf("Writing to %08x\n", wrt);
    // Enter Program Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = PROG_CMD;

    // Note: write bytes as longs regardless of bus width
    for (i = 0;  i < sector_size;  i++) {
        wrt[i] = BUF(i);
    }

    // Wait for sector to program
    cnt = 0;
    i = sector_size - 1;
    while (wrt[i] != BUF(i)) {
        if (cnt++ > 0x01000000) break;
    }
//    diag_printf("Out - i: %d, wrt[i] = %08X.%08X, BUF(i) = %08X, count = %x\n", i, &wrt[i], wrt[i], BUF(i), cnt);

    // Verify
    for (i = 0;  i < sector_size;  i++) {
        for (cnt = 0;  cnt < 10;  cnt++) {
            if (wrt[i] == BUF(i)) break;
            cyg_thread_delay(1);
        }
        if (cnt == 10) {
            diag_printf("Can't program at 0x%08X: %02X not %02X\n", wrt, *wrt, BUF(0));
        }
    }
}
