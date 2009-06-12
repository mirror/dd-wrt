//==========================================================================
//
//        prog_flash.c
//
//        Cirrus CL7211 eval board FLASH program tool
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
// Contributors:  gthomas
// Date:          1999-05-12
// Description:   Tool used to program onboard FLASH image
//####DESCRIPTIONEND####

//
// This program will program the FLASH on the CL7211 board from a fixed buffer
//

#include <pkgconf/kernel.h>   // Configuration header
#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_cache.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define STACK_SIZE 4096
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

void identify_FLASH(void);
int  write_flash(long *data, volatile long *mem);
int  erase_sector(volatile long *mem);

// Note: these are mapped addresses
//  0xE0000000 = ROM Bank 0
//  0xF0000000 = ROM Bank 1
volatile unsigned long *FLASH = (volatile unsigned long *)0xE0000000;

long *flash_buffer =     (long *)0x60000;
long *flash_buffer_end = (long *)0x64000;
long *ROM_address =      (long *)0xE07E0000;

// Adapted from Cirrus sample code
#define ATMEL_SEQ_ADD1                0x00015554>>2
#define ATMEL_SEQ_ADD2                0x0000AAA8>>2
#define ATMEL_START_CMD1              0xAAAAAAAA
#define ATMEL_START_CMD2              0x55555555
#define ATMEL_ID_CMD                  0x90909090
#define ATMEL_PROG_CMD                0xA0A0A0A0
#define ATMEL_ERASE_CMD               0x80808080
#define ATMEL_SECTOR_ERASE_CMD        0x30303030
#define ATMEL_STOP_CMD                0xF0F0F0F0
#define ATMEL_BUSY_TOGGLE             0x00400040
#define ATMEL_ERASE_TOGGLE            0x00440044

#define ATMEL_MANUF                   0x1F
#define ATMEL_AT29C040_ID             0X5B
#define ATMEL_AT29C040A_ID            0XA4
#define ATMEL_AT29C1024_ID            0X25
#define ATMEL_SECTOR_SIZE             256
#define ATMEL_MAX_SECTORS             2048

#define INTEL_MANUF                   0x0089
#define INTEL_STOP_CMD                0x00FF00FF
#define INTEL_PROG_CMD                0x00400040
#define INTEL_ERASE_CMD               0x00200020
#define INTEL_ERASE_CONFIRM           0x00D000D0
#define INTEL_READ_STATUS             0x00700070
#define INTEL_CLEAR_STATUS            0x00500050
#define INTEL_SB_WSMS                 0x00800080  // Write state machine = ready
#define INTEL_SB_ERASE_ERROR          0x00200020  // Erase failure
#define INTEL_SB_PROG_ERROR           0x00100010  // Programming failure

#define ERASE_TIMEOUT                 10          // seconds

int manuf_code, device_code, sector_size, max_no_of_sectors, word_mode;

// FUNCTIONS

static void
cyg_test_exit(void)
{
    while (TRUE) ;
}

static void
program_flash(cyg_addrword_t arg)
{
    diag_printf("PROGRAM FLASH here!\n");
    HAL_UCACHE_SYNC();     // ROM space is marked cacheable which causes problems!
    HAL_UCACHE_DISABLE();  // So, just disable caches.
    identify_FLASH();
    diag_printf("About to program FLASH using data at %x..%x\n", flash_buffer, flash_buffer_end);
    diag_printf("*** Press RESET now to abort!\n");
    cyg_thread_delay(5*100);
    diag_printf("\n");
    diag_printf("... Erase sector\n");
    if (erase_sector(ROM_address)) {
        diag_printf("... Programming FLASH\n");
        while (flash_buffer < flash_buffer_end) {
            if (!write_flash(flash_buffer++, ROM_address++)) break;
        }
    }
    
    // Exit Program Mode
    switch (manuf_code) {
    case ATMEL_MANUF:
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
        FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_STOP_CMD;
        break;
    case INTEL_MANUF:
        FLASH[0] = INTEL_STOP_CMD;
        break;
    }
    diag_printf("All done!\n");
    cyg_test_exit();
}

void
identify_FLASH(void )
{
    // Enter Software Product Identification Mode
    FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
    FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
    FLASH[ATMEL_SEQ_ADD1] = ATMEL_ID_CMD;

    // Wait at least 10ms
    cyg_thread_delay(2);

    // Read Manufacturer and device code from the device
    manuf_code = FLASH[0] >> 16;
    device_code = FLASH[1] >> 16;

    diag_printf("manufacturer: 0x%04x, device: 0x%04x\n", manuf_code, device_code);

    // Exit Software Product Identification Mode
    switch (manuf_code) {
    case ATMEL_MANUF:
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
        FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_STOP_CMD;
        break;
    case INTEL_MANUF:
        FLASH[0] = INTEL_STOP_CMD;
        break;
    default:
        diag_printf("Unrecognized FLASH manufacturer - I give up!\n");
        cyg_test_exit();
    }
}

void
spin(void)
{
    volatile int i;
    for (i = 0;  i < 100;  i++) ;
}

int
write_flash(long *data, volatile long *mem)
{
    long data1, data2;
    long status;
    int timer;
    switch (manuf_code) {
    case ATMEL_MANUF:
        // Enter Program Mode
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
        FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_PROG_CMD;
        // Send data to selected address
        *mem = *data;
        while (TRUE) {
            data1 = *mem;  // Read cell back twice
            data2 = *mem;
            // Bit 6 toggles between reads while programming in progress
            if ((data1 & ATMEL_BUSY_TOGGLE) == (data2 & ATMEL_BUSY_TOGGLE)) break;
        }
        break;
    case INTEL_MANUF:
        // Clear current errors
        FLASH[0] = INTEL_CLEAR_STATUS;
        // Issue program command
        FLASH[0] = INTEL_PROG_CMD;
        // Send data to selected address
        *mem = *data;
        timer = 10000;
        // Read the status register to wait for programming complete
        do {
            spin();
            status = FLASH[0];
        } while (((status & INTEL_SB_WSMS) != INTEL_SB_WSMS) &&
                 (--timer > 0));
        // Check for errors
        if (timer == 0) {
            diag_printf("Programming at 0x%08x timed out - status: 0x%08x\n", mem, status);
        }
        if ((status & INTEL_SB_PROG_ERROR) != 0) {
            diag_printf("Device reports programming error at 0x%08x - status: 0x%08x\n", mem, status);
        }
        // Exit program mode
        FLASH[0] = INTEL_STOP_CMD;
        break;
    }
    if (*mem != *data) {
        diag_printf("Programming failed at 0x%08x - write: 0x%08x, read: 0x%08x\n",
                    mem, *data, *mem);
        return (FALSE);
    }
    return (TRUE);
}

int
erase_sector(volatile long *mem)
{
    long data1, data2;
    long status;
    int timer;
    switch (manuf_code) {
    case ATMEL_MANUF:
        // Erase sector command
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
        FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_ERASE_CMD;
        FLASH[ATMEL_SEQ_ADD1] = ATMEL_START_CMD1;
        FLASH[ATMEL_SEQ_ADD2] = ATMEL_START_CMD2;
        *mem = ATMEL_SECTOR_ERASE_CMD;
        while (TRUE) {
            data1 = *mem;  // Read cell back twice
            data2 = *mem;
            // Bits 6+2 toggle between reads while programming in progress
            if ((data1 & ATMEL_ERASE_TOGGLE) == (data2 & ATMEL_ERASE_TOGGLE)) break;
        }
        break;
    case INTEL_MANUF:
        // Clear current errors
        FLASH[0] = INTEL_CLEAR_STATUS;
        // Issue erase block command
        FLASH[0] = INTEL_ERASE_CMD;
        *mem = INTEL_ERASE_CONFIRM;
        timer = ERASE_TIMEOUT*50;
        // Read the status register while erase in progress
        do {
            cyg_thread_delay(2);
            status = FLASH[0];
        } while (((status & INTEL_SB_WSMS) != INTEL_SB_WSMS) &&
                 (--timer > 0));
        // Check for errors
        if (timer == 0) {
            diag_printf("Erase not complete after %d seconds - status: 0x%08x\n", 
                        ERASE_TIMEOUT, status);
        }
        if ((status & INTEL_SB_ERASE_ERROR) != 0) {
            diag_printf("Device reports erase error - status: 0x%08x\n", status);
        }
        // Exit erase mode
        FLASH[0] = INTEL_STOP_CMD;
        break;
    }
    if (*mem != 0xFFFFFFFF) {
        diag_printf("Erase failed at 0x%08x - read: 0x%08x\n",
                    mem, *mem);
        return (FALSE);
    }
    return (TRUE);
}

externC void
cyg_start( void )
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      program_flash,     // entry
                      1,                 // index
                      "program_thread",  // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
} // cyg_package_start()

