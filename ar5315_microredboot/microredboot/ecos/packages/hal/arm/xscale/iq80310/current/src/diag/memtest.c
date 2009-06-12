//=============================================================================
//
//      memtest.c - Cyclone Diagnostics
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
// Contributors:
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
*/

#include <cyg/infra/diag.h>
#define printf diag_printf

#include "7_segment_displays.h"

#if 0
extern void store_double (unsigned long, unsigned long, unsigned long);
extern void read_double (unsigned long, unsigned long Data[]);
extern int quadtest(long startaddr);
#endif

extern void hex32out (unsigned int num);
#if 0
extern int printf(char*,...);
#endif

/* 02/02/01 jwf */
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif



#define FAILED          1
#define PASSED          0



/* Do walking one's test */
static int
onesTest(
	 long testAddr			/* address to test */
	 )
{
	long testData = 1; /* Current pattern being used */
	long dataRead;
	int	fail = 0;	   /* Test hasn't failed yet */
	int	loopCount = 0; /* To keep track of when to print CR */

	printf("\n");

	while(testData && !fail) 
	{	/* Loop until bit shifted out */
		*((long *) testAddr) = testData;	 /* Write test data */
		*((long *) (testAddr + 4)) = 0xFFFFFFFF; /* Drive d0-d31 hi */
		dataRead = *((long *) testAddr);	 /* Read back data */

		hex32out(dataRead);
		if (!(++loopCount % 8) && (loopCount != 32))
			printf("\n");
		else
			printf(" ");

		if (dataRead != testData) /* Verify data */
			return FAILED;	  /* Signal failure */
		else
			testData <<= 1;	  /* Shift data over one bit */
	}

	return PASSED;
}




#if 0
/*************************************************************************
*
* onesTest - perform a 64 bit walking one's test on a specified address
*
*
* RETURNS: PASSED if the test passes or FAILED otherwise
*
*/
static int onesTest(long testAddr)
{
	/* need to be arrays of sequential words in order to be
	   able to test a 64bit wide memory bus */
	unsigned long testData[2];	/* Current pattern being used */
	unsigned long dataRead[2];	/* Data read back from memory */
	int	bitsTested = 0;		/* To keep track of when to print CR and
							   when to switch words */

	printf("\n");

	/* test variable initialization */
	testData[0] = 0x00000001;	/* lower 32 bit word */
	testData[1] = 0x00000000;	/* upper 32 bit word */

	bitsTested = 0;


	/* Loop until all 64 data bits are tested */
	while (bitsTested < 64)
	{
		/* perform a double word write to cause a 64bit memory access */
		store_double (testAddr, testData[0], testData[1]);

		/* drive 64 bit data bus high and flush bus unit */
		store_double (testAddr + 8, 0xffffffff, 0xffffffff);

		/* perform a double word read to cause a 64bit memory access */
		read_double (testAddr, dataRead);

		hex32out((long)dataRead[1]);	/* print out MS word */
		hex32out((long)dataRead[0]);	/* print out LS word */

		if (!(++bitsTested % 4) && (bitsTested != 64))
			printf("\n");
		else
			printf(" ");

		/* verify the data */
		if ((dataRead[0] != testData[0]) || (dataRead[1] != testData[1]))
			return (FAILED);			/* Signal failure */
		else
		{
			if (bitsTested < 32)			/* data bits 0 - 31 */
			{
				testData[0] <<= 1;			/* shift data through LS word */
			}
			else if (bitsTested == 32)		/* start testing MS word */
			{
				testData[0] = 0x00000000;	/* clear LS word */
				testData[1] = 0x00000001;	/* shift into MS word */
			}
			else							/* data bits 32 - 63 */
			{
				testData[1] <<= 1;			/* shift data through MS word */
			}
		}
	}
		return (PASSED);
}

#endif



/* Do long word address test */

static int LWAddr (
	long	start,			/* Starting address of test */
	long	end,			/* Ending address */
	long	*badAddr		/* Failure address */
	)
{
	register long	currentAddr;	/* Current address being tested */
	register long   data;
	char	fail = 0;		/* Test hasn't failed yet */

	for(currentAddr = start; currentAddr < end; currentAddr += 4)
		*((long *) currentAddr) = currentAddr;

	for (currentAddr = start;
	     (currentAddr < end);
	     currentAddr += 4)
	{
		data = *(long *) currentAddr;
	    if (data != currentAddr)
		{
			fail = 1;
			printf ("\n\nLWAddr Bad Read, Address = 0x%08x, Data Read = 0x%08x\n\n", currentAddr, data);
			break;
		}
	}

	if (fail)
	{
	    *badAddr = currentAddr;
	    return FAILED;
	}
	else
	    return PASSED;
}

/* Do inverse long word address test */

static int LWBar (long	start,			/* Starting address of test */
       long	end,			/* Ending address */
       long	*badAddr		/* Failure address */
	   )
{
	register long	currentAddr;	/* Current address being tested */
	register long   data;
	int	fail = 0;		/* Test hasn't failed yet */

	for(currentAddr = start; currentAddr < end; currentAddr += 4)
		*((long *) currentAddr) = ~currentAddr;

	for (currentAddr = start;
	     (currentAddr < end); 
	     currentAddr += 4)
	{
		data = *(long *) currentAddr;
	    if (data != ~currentAddr)
		{
			fail = 1;		
			printf ("\n\nLWBar Bad Read, Address = 0x%08x, Data Read = 0x%08x\n\n", currentAddr, data);
			break;
		}
	}
	if (fail)
	{
	    *badAddr = currentAddr;
	    return FAILED;
	} 
	else
	    return PASSED;
}

/* Do byte address test */

static int
ByteAddr (
	  long	start,				/* Starting address of test */
	  long	end,				/* Ending address */
	  long	*badAddr			/* Failure address */
	  )
{
	long	currentAddr;	/* Current address being tested */
	int	fail = 0;		/* Test hasn't failed yet */

	for(currentAddr = start; currentAddr < end; currentAddr++)
		*((char *) currentAddr) = (char) currentAddr;

	for(currentAddr = start; (currentAddr < end) && (!fail); currentAddr++)
		if (*((char *) currentAddr) != (char) currentAddr)
			fail = 1;

	if (fail) 
	{
		*badAddr = currentAddr - 1;
		return FAILED;
	} 
	else
		return PASSED;
}

/* Do inverse byte address test */

static int ByteBar (
	 long	start,				/* Starting address of test */
	 long	end,				/* Ending address */
	 long	*badAddr			/* Failure address */
	 )
{
	long	currentAddr;		/* Current address being tested */
	int	fail = 0;		/* Test hasn't failed yet */

	for(currentAddr = start; currentAddr < end; currentAddr++)
		*((char *) currentAddr) = (char) ~currentAddr;

	for(currentAddr = start; (currentAddr < end) && (!fail); currentAddr++)
		if (*((char *) currentAddr) != (char) ~currentAddr)
			fail = 1;
	if (fail) {
		*badAddr = currentAddr - 1;
		return FAILED;
	} 
	else
		return PASSED;
}

/*
 * This routine is called if one of the memory tests fails.  It dumps
 * the 8 32-bit words before and the 8 after the failure address
 */

void dumpMem (
	 long	badAddr			/* Failure address */
	 )
{
	unsigned long *addr;
	unsigned short *saddr;

	printf("\n");					/* Print out first line of mem dump */
	hex32out(badAddr - 32);		/* Starting address */
 	printf(": ");
	hex32out(*((long *) (badAddr - 32)));	/* First longword */
 	printf(" ");
	hex32out(*((long *) (badAddr - 28)));
 	printf(" ");
	hex32out(*((long *) (badAddr - 24)));
 	printf(" ");
	hex32out(*((long *) (badAddr - 20)));

 	printf("\n");
	hex32out(badAddr - 16);	
 	printf(": ");
	hex32out(*((long *) (badAddr - 16)));
 	printf(" ");
	hex32out(*((long *) (badAddr - 12)));
 	printf(" ");
	hex32out(*((long *) (badAddr - 8)));
 	printf(" ");
	hex32out(*((long *) (badAddr - 4)));

	printf("\n");		/* Print out contents of fault addr */
	hex32out(badAddr);
 	printf(": ");
	hex32out(*((long *) badAddr));


	printf("\n");		/* Print out next line of mem dump */
	hex32out(badAddr + 4);		/* Starting address */
 	printf(": ");
	hex32out(*((long *) (badAddr + 4)));	/* First longword */
 	printf(" ");
	hex32out(*((long *) (badAddr + 8)));
 	printf(" ");
	hex32out(*((long *) (badAddr + 12)));
 	printf(" ");
	hex32out(*((long *) (badAddr + 16)));

 	printf("\n");
	hex32out(badAddr + 20);	
 	printf(": ");
	hex32out(*((long *) (badAddr + 20)));
 	printf(" ");
	hex32out(*((long *) (badAddr + 24)));
 	printf(" ");
	hex32out(*((long *) (badAddr + 28)));
 	printf(" ");
	hex32out(*((long *) (badAddr + 32)));

	/* DEBUG */
	printf ("\n\nReading back data in 32bit chunks:\n");
	addr = (unsigned long *)(badAddr - 16);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr - 12);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr - 8);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr - 4);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr + 4);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr + 8);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr + 12);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	addr = (unsigned long *)(badAddr + 16);
	printf ("Address = 0x%08x, Data = 0x%08x\n", addr, *addr);
	printf ("\n");

	printf ("Reading back data in 16bit chunks:\n");
	saddr = (unsigned short *)(badAddr - 16);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 14);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 12);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 10);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 8);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 6);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 4);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr - 2);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 2);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 4);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 6);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 8);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 10);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 12);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 14);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	saddr = (unsigned short *)(badAddr + 16);
	printf ("Address = 0x%08x, Data = 0x%04x\n", saddr, *saddr);
	printf ("\n");

}

/*
 * Returns 1 if passed, 0 if failed.
 */

int
memTest (
	 long	startAddr,		/* Start address of test */
	 long	endAddr			/* End address + 1 */
	 )
{
	long	badAddr;		/* Addr test failed at */

	printf("\n");
	    
	if (onesTest(startAddr) == FAILED)
	{
		printf("\nWalking 1's test: failed");
		return 0;
	}
	printf("\nWalking 1's test: passed\n");

	/* rval = quadtest(startAddr); 

	switch (rval)
	{
		case 0:
			printf("\nQuadword test passed\n");
			break;

		case 1:
			printf("\nQuadword test failed: Quadword Write, Longword Read\n");
			dumpMem(startAddr);
			return 0;
			
		case 2:
			printf("\nQuadword test failed: Longword Write, Quadword Read\n");
			dumpMem(startAddr);
			return 0;			

		default:
			printf("\nQuadword test: Unknown return value 0x%X\n", rval);
			return 0;		
	}
	*/

	printf("\nLong word address test: ");
	if (LWAddr(startAddr, endAddr, &badAddr) == FAILED)
	{
		printf("failed");
		dumpMem(badAddr);
		return 0;
	}
	printf("passed");

	printf("\nLong word address bar test: ");
	if (LWBar(startAddr, endAddr, &badAddr) == FAILED)
	{
		printf("failed");
		dumpMem(badAddr);
		return 0;
	}
	printf("passed");

	printf("\nByte address test: ");
	if (ByteAddr(startAddr, endAddr, &badAddr) == FAILED)
	{
		printf("failed");
		dumpMem(badAddr);
		return 0;
	}
	printf("passed");

	printf("\nByte address bar test: ");
	if (ByteBar(startAddr, endAddr, &badAddr) == FAILED)
	{
		printf("failed");
		dumpMem(badAddr);
		return 0;
	}
	printf("passed");

	return 1;
}


/* 02/02/01 jwf */
/* Do alternating inverse long word address test */
static int
LWABar(long	start,			/* Starting address of test */
       long	end,			/* Ending address */
       long	*badAddr)		/* Failure address */
{
	register long	currentAddr;	/* Current address being tested */
	int				fail = 0;		/* Test hasn't failed yet */
	register long	data;

	/* In this test, the contents of each longword address toggles 
	   between the Address and the Address BAR */
	for(currentAddr = start; currentAddr < end; currentAddr += 4)
	{
		/* Address ending in 0x4 or 0xc */
		if (currentAddr & 4)
			*((long *) currentAddr) = ~currentAddr;

		/* Address ending in 0x0 or 0x8 */ 
		else
			*((long *) currentAddr) = currentAddr;
	}

	for (currentAddr = start; (currentAddr < end) && (!fail); currentAddr += 4)
	{
		data = *(long *) currentAddr;

		switch (currentAddr & 0xf)
		{
			case 0x0:
			case 0x8:
				if (data != currentAddr)
				{
					fail = 1;
					printf ("\nFailed at Address 0x%08X, Expected 0x%08X, Read 0x%08X\n",
						currentAddr, currentAddr, data);
				}
				break;

			case 0x4:
			case 0xc:
				if (data != ~currentAddr)
				{
					fail = 1;
					printf ("\nFailed at Address 0x%08X, Expected 0x%08X, Read 0x%08X\n",
						currentAddr, ~currentAddr, data);
				}
				break;

			default:
				fail = 1;
				printf ("\nFailed at Address 0x%08X, Unaligned address\n", currentAddr);
				break;
		}
	}

	if (fail) {
	    *badAddr = currentAddr - 4;
	    return FAILED;
	} else
	    return PASSED;
}


/* 02/02/01 jwf */
/*
 * Returns 1 if passed, 0 if failed.
 */
int
LoopMemTest (
	 long	startAddr,		/* Start address of test */
	 long	endAddr			/* End address + 1 */
	 )
{
	long	badAddr;		/* Addr test failed at */
	volatile int junk;
	extern int ecc_error_reported;

	/* indicate no ECC errors recorded */
	*MSB_DISPLAY_REG = DISPLAY_OFF;

	/* indicate passing test */
	*LSB_DISPLAY_REG = LETTER_P;

	while (1)
	{
		printf("\n");
			
		printf("\nLong word address test: ");
		if (LWAddr(startAddr, endAddr, &badAddr) == FAILED)
		{
			/* indicate failing test */
			*LSB_DISPLAY_REG = LETTER_F;

			printf("failed at Address 0x%08x\n", badAddr);
			printf("Performing Continuous Write/Read/!Write/Read...\n\n");
			while (1)
			{
				*(volatile int *)badAddr = badAddr;
				junk = *(volatile int *)badAddr;
				*(volatile int *)badAddr = ~badAddr;
				junk = *(volatile int *)badAddr;

				if (ecc_error_reported)
				{
					printf ("Disabling ECC reporting\n");
					/* disable single and multi-bit reporting */
					*(volatile unsigned long *)0x1534 = 0x4;
					ecc_error_reported = FALSE;
				}
			}
			return 0;	/* not reached */
		}
		printf("passed");

		printf("\nLong word address bar test: ");
		if (LWBar(startAddr, endAddr, &badAddr) == FAILED)
		{
			/* indicate failing test */
			*LSB_DISPLAY_REG = LETTER_F;

			printf("failed at Address 0x%08x\n", badAddr);
			printf("Performing Continuous Write/Read/!Write/Read...\n\n");
			while (1)
			{
				*(volatile int *)badAddr = badAddr;
				junk = *(volatile int *)badAddr;
				*(volatile int *)badAddr = ~badAddr;
				junk = *(volatile int *)badAddr;

				if (ecc_error_reported) 
				{
					printf ("Disabling ECC reporting\n");
					/* disable single and multi-bit reporting */
					*(volatile unsigned long *)0x1534 = 0x4;
					ecc_error_reported = FALSE;
				}
			}
			return 0;	/* not reached */
		}
		printf("passed");

		printf("\nAlternating Long word, Long word address bar test: ");
		if (LWABar(startAddr, endAddr, &badAddr) == FAILED)
		{
			/* indicate failing test */
			*LSB_DISPLAY_REG = LETTER_F;

			printf("failed at Address 0x%08x\n", badAddr);
			printf("Performing Continuous Write/Read/!Write/Read...\n\n");
			while (1)
			{
				*(volatile int *)badAddr = badAddr;
				junk = *(volatile int *)badAddr;
				*(volatile int *)badAddr = ~badAddr;
				junk = *(volatile int *)badAddr;

				if (ecc_error_reported) 
				{
					printf ("Disabling ECC reporting\n");
					/* disable single and multi-bit reporting */
					*(volatile unsigned long *)0x1534 = 0x4;
					ecc_error_reported = FALSE;
				}
			}
			return 0;	/* not reached */
		}
		printf("passed");
	}

	return 1;
}


