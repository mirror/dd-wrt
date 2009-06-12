//=============================================================================
//
//      pci_serv.c - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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

/********************************************************************************/
/* PCI_SERV.C - PCI driver for IQ80310					        */
/*								    		*/
/* History:                                                                     */
/*  15sep00 ejb Ported to Cygmon on IQ80310				    	*/
/*  18dec00 snc                                                                 */
/********************************************************************************/
#include <redboot.h>
#include <cyg/hal/hal_iop310.h>        // Hardware definitions
#include "iq80310.h"
#include "pci_bios.h"

#undef  DEBUG_PCI

#define	IB_MA_ERROR	0x2000

/*==========================================================================*/
/* Globals                                                                  */
/*==========================================================================*/
ULONG	memspace_ptr[NUM_PCI_BUSES];
ULONG	iospace_ptr[NUM_PCI_BUSES];
ULONG	memspace_limit[NUM_PCI_BUSES];
ULONG	iospace_limit[NUM_PCI_BUSES];
UINT	nextbus;
UINT	secondary_busno = SECONDARY_BUS_NUM;
UINT	primary_busno = PRIMARY_BUS_NUM;
UINT    lastbus;
unsigned long dram_size; /* global storing the size of DRAM */	
int bus0_lastbus;        /* last secondary bus number behind bus 0 */
int bus1_lastbus;        /* last secondary bus number behind bus 1 */

int nmi_verbose;	/* global flag to indicate whether or not PCI Error messages should be
			   printed.  This flag is used to prevent a painful deluge of messages
			   when performing PCI configuration reads/writes to possibly non-existant
			   devices. */

int pci_config_error = FALSE; /* becomes TRUE if an NMI interrupt occurs due to a PCI config cycle */

#define PRINT_ON()  nmi_verbose = TRUE
#define PRINT_OFF() nmi_verbose = FALSE

/*==========================================================================*/
/* Function prototypes                                                      */
/*==========================================================================*/

typedef struct
{
    FUNCPTR	handler;
    int		arg;
    int		bus;
    int		device;
} INT_HANDLER;

#define NUM_PCI_XINTS		4		/* XINT0 - XINT3 */
#define MAX_PCI_HANDLERS	8		/* maximum handlers per PCI Xint */

extern void hexIn(void);
extern int pci_config_cycle;
extern void _enableFiqIrq(void);
extern void config_ints(void);	/* configure interrupts */

/*********************************************************************************
* pci_to_xint - convert a PCI device number and Interrupt line to an 80312 XINT
*
* This function converts a PCI slot number (0 - 7) and an Interrupt line
* (INTA - INTD) to a i960 processor XINT number (0 - 3)
*
* RETURNS: OK or ERROR if arguments are invalid
*
*/
STATUS pci_to_xint(int device, int intpin, int *xint)
{
    int device_base;	/* all devices mod 4 follow same interrupt mapping scheme */

    /* check validity of arguments */
    if ((intpin < INTA) || (intpin > INTD) || (device > 31))
	return (ERROR);

    device_base = device % 4;

    /* interrupt mapping scheme as per PCI-to-PCI Bridge Specification */
    switch (device_base) {
    case 0:
	switch (intpin) {
	case INTA:
	    *xint = XINT0;
	    break;
	case INTB:
	    *xint = XINT1;
	    break;
	case INTC:
	    *xint = XINT2;
	    break;
	case INTD:
	    *xint = XINT3;
	    break;
	}
	break;
    case 1:
	switch (intpin) {
	case INTA:
	    *xint = XINT1;
	    break;
	case INTB:
	    *xint = XINT2;
	    break;
	case INTC:
	    *xint = XINT3;
	    break;
	case INTD:
	    *xint = XINT0;
	    break;
	}
	break;
    case 2:
	switch (intpin) {
	case INTA:
	    *xint = XINT2;
	    break;
	case INTB:
	    *xint = XINT3;
	    break;
	case INTC:
	    *xint = XINT0;
	    break;
	case INTD:
	    *xint = XINT1;
	    break;
	}
	break;
    case 3:
	switch (intpin) {
	case INTA:
	    *xint = XINT3;
	    break;
	case INTB:
	    *xint = XINT0;
	    break;
	case INTC:
	    *xint = XINT1;
	    break;
	case INTD:
	    *xint = XINT2;
	    break;
	}
	break;
    }
    return (OK);
}


/******************************************************************************
*
* Checks to see if the "bus" argument identifies a PCI bus which is located 
* off of the Primary PCI bus of the board.
*/
int off_ppci_bus (int busno)
{
    if (busno == primary_busno) 
	return (TRUE);
    else if (busno == secondary_busno) 
	return (FALSE);
    else if (busno <= bus0_lastbus)
	return (TRUE);
    else
	return (FALSE);
}

/******************************************************************************
* sys_set_pci_irq - connect a PCI interrupt to a processor IRQ.
*
* The PCI Interrupt routing fabric on the Cyclone Hardware is not 
* reconfigurable (fixed mapping relationships) and therefore, this function
* is not supported.
*
*/
STATUS sys_set_pci_irq (int int_pin, int irq_num, int bus_dev)
{
    return (FUNC_NOT_SUPPORTED);
}

/* check if host of backplane */
int isHost(void)
{
    if (*BACKPLANE_DET_REG & BP_HOST_BIT)
	return TRUE;
    else
	return FALSE;
}




