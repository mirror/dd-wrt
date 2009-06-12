//=============================================================================
//
//      memtest.c
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors: Mark Salter
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/*************************************************************************
* Memtest.c - this file performs an address/address bar memory test.
*
*  Modification History
*  --------------------
*  01sep00 ejb Ported to StrongARM2
*  18dec00 snc
*  02feb01 jwf for snc
*  25jan02 Rewritten for RedBoot by Mark Salter
*/

#include <redboot.h>

#define FAILED          1
#define PASSED          0

// Do walking one's test
//
static int
onesTest(CYG_ADDRWORD *testAddr)
{
    CYG_ADDRWORD theOne, dataRead;
    int	fail, loopCount = 0; // To keep track of when to print CR

    diag_printf("\n");

    // Loop for all bits in an address
    for (theOne = 1, fail = 0; theOne && !fail; theOne <<= 1) {

	testAddr[0] = theOne;     // Write test data
	testAddr[1] = ~0L;        // Drive d0-dn hi
	dataRead = *testAddr;     // Read back data

	diag_printf("%08x%s", dataRead, (++loopCount % 8) ? "" : "\n");

	if (dataRead != theOne)  // Verify data
	    return FAILED;       // Signal failure
    }
    return PASSED;
}


// Do 32-bit word address test
//
static int
Addr32 (cyg_uint32 *start, cyg_uint32 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint32 *currentAddr; /* Current address being tested */
    cyg_uint32 data;

    for(currentAddr = start; currentAddr <= end; currentAddr++)
	*currentAddr = (cyg_uint32)(CYG_ADDRWORD)currentAddr;

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
	data = *currentAddr;
	if (data != (cyg_uint32)(CYG_ADDRWORD)currentAddr) {
	    diag_printf ("\n\nBad Read, Address = %p, Data Read = 0x%08x\n\n",
			 currentAddr, data);
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}


// Do inverse long word address test
//
static int
Bar32 (cyg_uint32 *start, cyg_uint32 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint32 *currentAddr, data;

    for(currentAddr = start; currentAddr <= end; currentAddr++)
	*currentAddr = ~(CYG_ADDRWORD)currentAddr;

    for (currentAddr = start; currentAddr <= end; currentAddr++) {
	data = *currentAddr;
	if (data != (~(CYG_ADDRWORD)currentAddr) & 0xffffffff) {
	    diag_printf ("\n\nBad Read, Address = %p, Data Read = 0x%08x\n\n",
			 currentAddr, data);
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    }
    return PASSED;
}

// Do byte address test
//
static int
Addr8 (cyg_uint8 *start, cyg_uint8 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint8 *currentAddr; // Current address being tested

    for (currentAddr = start; currentAddr <= end; currentAddr++)
	*currentAddr = (cyg_uint8)(CYG_ADDRWORD)currentAddr;

    for (currentAddr = start; currentAddr <= end; currentAddr++)
	if (*currentAddr != (cyg_uint8)(CYG_ADDRWORD)currentAddr) {
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    return PASSED;
}

// Do inverse byte address test
//
static int
Bar8 (cyg_uint8 *start, cyg_uint8 *end, CYG_ADDRWORD *badAddr)
{
    cyg_uint8 *currentAddr;  // Current address being tested

    for(currentAddr = start; currentAddr <= end; currentAddr++)
	*currentAddr = ~(CYG_ADDRWORD)currentAddr;

    for(currentAddr = start; currentAddr <= end; currentAddr++)
	if (*currentAddr != (~(CYG_ADDRWORD)currentAddr & 0xff)) {
	    *badAddr = (CYG_ADDRWORD)currentAddr;
	    return FAILED;
	}
    return PASSED;
}

// This routine is called if one of the memory tests fails.  It dumps
// the 8 32-bit words before and the 8 after the failure address
//
void
dumpMem (CYG_ADDRWORD badAddr)
{
    cyg_uint32 *addr;
    cyg_uint16 *saddr;
    int i;

    // Print out first line of mem dump
    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr - 32,
		*(cyg_uint32 *)(badAddr - 32),
		*(cyg_uint32 *)(badAddr - 28),
		*(cyg_uint32 *)(badAddr - 24),
		*(cyg_uint32 *)(badAddr - 20));

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr - 16,
		*(cyg_uint32 *)(badAddr - 16),
		*(cyg_uint32 *)(badAddr - 12),
		*(cyg_uint32 *)(badAddr - 8),
		*(cyg_uint32 *)(badAddr - 4));

    // Print out contents of fault addr
    diag_printf("\n%p: %08x",
		(char *)badAddr, *(cyg_uint32 *)badAddr);

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr + 4,
		*(cyg_uint32 *)(badAddr + 4),
		*(cyg_uint32 *)(badAddr + 8),
		*(cyg_uint32 *)(badAddr + 12),
		*(cyg_uint32 *)(badAddr + 16));

    diag_printf("\n%p: %08x %08x %08x %08x",
		(char *)badAddr + 20,
		*(cyg_uint32 *)(badAddr + 20),
		*(cyg_uint32 *)(badAddr + 24),
		*(cyg_uint32 *)(badAddr + 28),
		*(cyg_uint32 *)(badAddr + 32));

    /* DEBUG */
    diag_printf ("\n\nReading back data in 32bit chunks:\n");
    for (addr = (cyg_uint32 *)(badAddr - 16), i = 0; i <= 8; i++, addr++)
	diag_printf ("Address = %p, Data = 0x%08x\n", addr, *addr);
    diag_printf ("\n");

    diag_printf ("Reading back data in 16bit chunks:\n");
    for (saddr = (cyg_uint16 *)(badAddr - 16), i = 0; i <= 16; i++, saddr++)
	diag_printf ("Address = %p, Data = 0x%08x\n", saddr, *saddr);
    diag_printf ("\n");
}

// Returns 1 if passed, 0 if failed.
//
int
memTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr)
{
    CYG_ADDRWORD badAddr;  // Addr test failed at

    diag_printf("\nWalking 1's test: ");
    if (onesTest((CYG_ADDRWORD *)startAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n32-bit address test: ");
    if (Addr32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n32-bit address bar test: ");
    if (Bar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\n8-bit address test: ");
    if (Addr8((cyg_uint8 *)startAddr, (cyg_uint8 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    diag_printf("\nByte address bar test: ");
    if (Bar8((cyg_uint8 *)startAddr, (cyg_uint8 *)endAddr, &badAddr) == FAILED)
	goto failed;
    diag_printf("passed");

    return 1;

 failed:
    diag_printf("failed");
    dumpMem(badAddr);
    return 0;
}


/* 02/02/01 jwf */
/* Do alternating inverse long word address test */
static int
ABar32(cyg_uint32 *start,		/* Starting address of test */
       cyg_uint32 *end,		/* Ending address */
       CYG_ADDRWORD *badAddr)		/* Failure address */
{
    register cyg_uint32 *currentAddr;	/* Current address being tested */
    int fail = 0;		/* Test hasn't failed yet */
    cyg_uint32 data;

    /* In this test, the contents of each longword address toggles 
       between the Address and the Address BAR */
    for(currentAddr = start; currentAddr <= end; currentAddr++) {
	
	if ((CYG_ADDRWORD)currentAddr & 4) /* Address ending in 0x4 or 0xc */
	    *currentAddr = ~(cyg_uint32)(CYG_ADDRWORD)currentAddr;

	else /* Address ending in 0x0 or 0x8 */ 
	    *currentAddr = (cyg_uint32)(CYG_ADDRWORD)currentAddr;
    }

    for (currentAddr = start; currentAddr <= end && !fail; currentAddr++) {
	data = *currentAddr;

	switch ((CYG_ADDRWORD)currentAddr & 0xf) {
	  case 0x0:
	  case 0x8:
	    if (data != (cyg_uint32)(CYG_ADDRWORD)currentAddr) {
		fail = 1;
		diag_printf ("\nFailed at Address %p, Expected 0x%08X, Read 0x%08X\n",
			     currentAddr, (cyg_uint32)(CYG_ADDRWORD)currentAddr, data);
	    }
	    break;

	  case 0x4:
	  case 0xc:
	    if (data != ~(cyg_uint32)(CYG_ADDRWORD)currentAddr) {
		fail = 1;
		diag_printf ("\nFailed at Address %p, Expected 0x%08X, Read 0x%08X\n",
			     currentAddr, ~(cyg_uint32)(CYG_ADDRWORD)currentAddr, data);
	    }
	    break;

	  default:
	    fail = 1;
	    diag_printf ("\nFailed at Address %p, Unaligned address\n", currentAddr);
	    break;
	}
    }

    if (fail) {
	*badAddr = (CYG_ADDRWORD)(--currentAddr);
	return FAILED;
    } else
	return PASSED;
}


/* 02/02/01 jwf */
/*
 * Returns 1 if passed, 0 if failed.
 */
int
LoopMemTest (CYG_ADDRWORD startAddr, CYG_ADDRWORD endAddr)
{
    CYG_ADDRWORD badAddr;  /* Addr test failed at */
    volatile int junk;

    while (1) {
	diag_printf("\n");
			
	diag_printf("\n32-bit address test: ");
	if (Addr32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");

	diag_printf("\n32-bit address bar test: ");
	if (Bar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");

	diag_printf("\nAlternating Long word, Long word address bar test: ");
	if (ABar32((cyg_uint32 *)startAddr, (cyg_uint32 *)endAddr, &badAddr) == FAILED)
	    break;
	diag_printf("passed");
    }

    diag_printf("failed at Address %p\n", badAddr);
    diag_printf("Performing Continuous Write/Read/!Write/Read...\n\n");
    while (1) {
	*(volatile int *)badAddr = badAddr;
	junk = *(volatile int *)badAddr;
	*(volatile int *)badAddr = ~badAddr;
	junk = *(volatile int *)badAddr;
    }

    return 1;	// not reached
}


