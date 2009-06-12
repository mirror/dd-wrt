//==========================================================================
//
//        flash.c
//
//        ARM PID7 eval board FLASH program tool
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
// Date:          1998-11-18
// Description:   Tool used to program onboard FLASH image
//####DESCRIPTIONEND####

//
// This program will program the FLASH on the PID board
//

#include <pkgconf/libc.h>   // Configuration header

#include <cyg/kernel/kapi.h>
#include <stdlib.h>
#include <ctype.h>
#include <cyg/infra/testcase.h>
#include <sys/cstartup.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define SYNC_COUNT 63

extern void diag_printf(const char *, ...);
int identify_FLASH(void);
void write_sector(int, char *);
bool load_srecords(char (*readc)(), CYG_ADDRESS *start, int *size);

char dbuf[256];
char *raw = (char *)0x10000;
char *flash_buffer = (char *)0x30000;
int pos, len;

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

char nextch(void)
{
    return (raw[pos++]);
}

int
main( int argc, char *argv[] )
{
    int i, j, size;
    CYG_ADDRESS entry;
    char c;
    diag_printf("FLASH here!\n");
    while (identify_FLASH() == 0) {
        diag_printf("... Please change FLASH jumper - hit C/R to continue:");
        do {
            hal_diag_read_char(&c);
        } while ((c != '\r') && (c != '\n'));
        diag_printf("\n");
    }
 restart:
    diag_printf("Ready file - hit C/R to continue:");
    while (TRUE) {
        hal_diag_read_char(&c);
        if (c == '>') break;
    }
    i = 0;  j = 0;
    while (1) {
        hal_diag_read_char(&c);
        if (c == '!') {
            diag_printf("... Reset\n");
            goto restart;
        }
        raw[i++] = c;
        if (++j == SYNC_COUNT) {
            hal_diag_write_char(c);
            j = 0;
        }
        if (c == ':') break;
    }
    diag_printf("\n");
    pos = 0;  len = i;
    if (load_srecords(nextch, &entry, &size)) {
        diag_printf("Read %x bytes, entry: %x\n", size, entry);
        dump_buf(flash_buffer, 128);
        diag_printf("\nData loaded - hit '!' to continue:");
        while (TRUE) {
            hal_diag_read_char(&c);
            if (c == '!') break;
        }
        diag_printf("\n");
        diag_printf("...Programming FLASH\n");
        pos = 0;  i = 0;
        while (pos < size) {
            write_sector(i++, flash_buffer+pos);
            pos += 256;
        }
    } else {
        // Display buffer around failure        
        dump_buf(&raw[pos-32], 64);
    }
    diag_printf("All done!\n");
    while (1) ;
}

// Adapted from ARM sample code
#define SEQ_ADD1                0x5555
#define SEQ_ADD2                0xAAAA
#define START_CMD1              0xAA
#define START_CMD2              0x55
#define ID_CMD                  0x90
#define PROG_CMD                0xA0
#define STOP_CMD                0xF0

#define MAN_ATMEL               0x1F
#define ATMEL_AT29C040_ID       0X5B
#define ATMEL_AT29C040A_ID      0XA4
#define ATMEL_AT29C1024_ID      0X25
#define ATMEL_SECTOR_SIZE       256
#define ATMEL_MAX_SECTORS       2048

int manuf_code, device_code, sector_size, max_no_of_sectors, word_mode;
volatile char *FLASH = (volatile char *)0x04000000;

int
identify_FLASH(void )
{
    // Enter Software Product Identification Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = ID_CMD;

    // Wait at least 10ms
    cyg_thread_delay(2);

    // Read Manufacturer and device code from the device
    manuf_code = FLASH[0];
    device_code = FLASH[1];

    diag_printf("manuf: %x, device: %x\n", manuf_code, device_code);

    // Exit Software Product Identification Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = STOP_CMD;

    // Wait at least 10ms
    cyg_thread_delay(5);

    if (manuf_code != MAN_ATMEL) {
        diag_printf ( "Error: Wrong Manufaturer: %02x\n",manuf_code );
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
    volatile char *wrt = (volatile int *)&FLASH[num*sector_size];

//    diag_printf("Writing to %08x\n", wrt);
    // Enter Program Mode
    FLASH[SEQ_ADD1] = START_CMD1;
    FLASH[SEQ_ADD2] = START_CMD2;
    FLASH[SEQ_ADD1] = PROG_CMD;

    // Note: write bytes as longs regardless of bus width
    for (i = 0;  i < sector_size;  i++) {
        wrt[i] = buf[i];
    }

    // Wait for sector to program
    cnt = 0;
    i = sector_size - 1;
    while (wrt[i] != buf[i]) {
        if (cnt++ > 0x01000000) break;
    }
//    diag_printf("Out - i: %d, wrt[i] = %08X.%08X, buf[i] = %08X, count = %x\n", i, &wrt[i], wrt[i], buf[i], cnt);

    // Verify
    for (i = 0;  i < sector_size;  i++) {
        for (cnt = 0;  cnt < 10;  cnt++) {
            if (*wrt == *buf) break;
            cyg_thread_delay(1);
        }
        if (cnt == 10) {
            diag_printf("Can't program at 0x%08X: %02X not %02X\n", wrt, *wrt, *buf);
        }
        wrt++;  buf++;
    }
}

// S-record download code - viciously 'adapted' from "kernel/src/sload/sload.c"

/*---------------------------------------------------------------------------*/
/*
//
//      An srecord looks like this:
//
// byte count-+     address
// start ---+ |        |       data        +- checksum
//          | |        |                   |
//        S01000006F6B692D746573742E73726563E4
//        S315000448600000000000000000FC00005900000000E9
//        S31A0004000023C1400037DE00F023604000377B009020825000348D
//        S30B0004485A0000000000004E
//        S70500040000F6
//
//      S<type><length><address><data><checksum>
//
//      Where 
//      - length (2 characters)
//        is the number of bytes following upto the checksum. Note that
//        this is not the number of chars following, since it takes two
//        chars to represent a byte.
//      - type (2 characters)
//        is one of:
//        0) header record
//        1) two byte address data record
//        2) three byte address data record
//        3) four byte address data record
//        5) record containing the number of S1, S2, or S3 records
//        7) four byte address termination record
//        8) three byte address termination record
//        9) two byte address termination record
//       
//      - address (4, 6, or 8 characters)
//        is the start address of the data following, or in the case of
//        a termination record, the start address of the image
//      - data (0-2n characters)
//        is the data.
//      - checksum (2 characters)
//        is the sum of all the raw byte data in the record, from the length
//        upwards, modulo 256 and subtracted from 255.
//
// Useful S-records for testing purposes:
//   Start record:
//      S00B0000737461303030447563
//   This sets the default address to be 0x02005000:
//      S31A020050002700801481C4E0B0A15000000100000091D02000018F
//      S31A0200501500000001000000010000002700801881C4E2E4A150C1
//      S311020080A42407070A090B0A0781050000E1
//   Termination record:
//      S70502005000A8
//
*/

#define S0      0
#define S1      1
#define S2      2
#define S3      3
#define S5      5
#define S7      7
#define S8      8
#define S9      9

/*---------------------------------------------------------------------------*/

int hex2digit(char c)
{
    if( c & 0x40 ) c += 9;;
    return c &0x0f;
    
//    return ( c <= '9' ? c - '0' :
//             c <= 'Z' ? c - 'A' + 10 :
//             c - 'a' + 10);
}

/*---------------------------------------------------------------------------*/

bool load_srecords(char (*readc)(), 
                   CYG_ADDRESS *start,
                   int *size)
{
    CYG_ADDRESS addr, load_addr;
    int addrsize;
    int length;
    int i;
    cyg_uint8 chksum, ochksum;
    cyg_uint8 val;
    cyg_uint8 *tdata;    
    char s;
    char type;
    char len0;
    char len1;
    bool first = true;

    
    do {
        // Skip whitespace characters until we find something that
        // might be an 'S'.
        do {
            s = readc();
        } while( s == '\r' || s == '\n' || s == ' ');

        // Check that this is an S record
        if( s != 'S' ) {
            diag_printf("Invalid 'S' record\n");
            return false;
        }

        // First 4 bytes are standard S + type + len
        type = readc();
        len0 = readc();
        len1 = readc();
        
        // decode the type
        type = hex2digit(type);

        // determine address size
        switch (type) {
        case S0:                        // start records have no address
            addrsize = 0;
            break;
        case S1:                        // two byte address
        case S9:
            addrsize = 4;
            break;
        case S2:                        // 3 byte address
        case S8:
            addrsize = 6;
            break;   
        case S3:                        // 4 byte address
        case S7:
            addrsize = 8;
            break;
        }

        length  = hex2digit (len0) << 4;
        length |= hex2digit (len1);
        chksum = length;

        // read the address
        addr = 0;
        for (i = 0; i < addrsize; i++) {
            val = hex2digit(readc());
            addr = (addr << 4) | val;
        }

        // calculate the checksum, which is done by the byte, not the digit
        for (i = 0; i < addrsize*4; i += 8) {
            chksum += ((addr >>  i) & 0xff);
        }

        // decide where to load this data
        if (first && (type != S0)) {
            load_addr = addr;
            first = false;
        }

        // read the data and put it directly into memory where it belongs
        tdata = (cyg_uint8 *)((addr - load_addr) + flash_buffer);
        if (type < S7) {
            *size = (addr - load_addr);
        }
        val = 0;
        for (i = 0; i < ((length - 1) * 2) - addrsize; i += 2 ) {
            val  = hex2digit (readc()) << 4;
            val |= hex2digit (readc());
            chksum += val;
            if( type != S0 ) *tdata++ = val;
            if (type < S7) *size = *size + 1;
        }

        // now get the old checksum
        ochksum = hex2digit(readc()) << 4;
        ochksum |= hex2digit(readc());
        chksum = ~chksum;
        if (chksum != ochksum) {
            diag_printf("Bad checksum - addr: %x\n", addr);
            return false;
        }
        
    } while( type < S7 );

    *start = addr;
    return true;
}
