//=============================================================================
//
//      i557_eep.c - Cyclone Diagnostics
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

#include "i557_eep.h"

/*****************************************************************************
*
* Serial EEPROM Access code for the i557/558
*
* Revision History:
* -----------------
*
* 
* 05jun98, snc Added setup time for eeprom CS.  Changed eeprom_delay to use the
*			   processor's internal timer.  Fixed programming algorithm to poll
*			   the eeprom's DO line to look for the transition from BUSY to READY
*			   which indicates that the programming operation has completed.
* 03jun98, snc Added setup time delay on data writes (delay before asserting
*			   a rising edge on the SK.  Fixed eeprom_get_word() to explicitly
*			   clear a bit position in the buffer after reading a low on the 
*			   data lines.
* 23oct96, snc Ported to the PCI914
*
*/

/*
 * Timing information.  According to the National Semiconductor manual,
 * the SK High Minimum time = SK Low Minimum time = 250 nsec.  However,
 * the minimum SK cycle time is 1 usec, so a 250 nsec high/750 nsec. low
 * sequence or equivalent would be required.
 */

/* Serial clock line */
#define SK_LOW_PERIOD		500	/* nsec, Time serial clock is low */
#define SK_HIGH_PERIOD		500	/* nsec, Time serial clock is high */

/* Serial data line */
#define DATA_IN_HOLD_TIME	20	/* nsec, SK low to EEDI change */
#define DATA_IN_SETUP_TIME	100	/* nsec, EEDI change to SK high */

/* Serial clock and data line states (assumes ports are non-inverting) */
#define HIGH			1
#define LOW			0

/* Select setup time to rising edge of SK */
#define SELECT_SETUP_TIME	50	/* nsec */

/* De-select time between consecutive commands */
#define DESELECT_TIME		100	/* nsec */


/* local functions */

static void set_sda_line (unsigned long pci_base,	/* PCI Base address */
			  int state);			/* HIGH or LOW */

static int get_sda_line (unsigned long pci_base);	/* PCI Base address */

static void set_scl_line (unsigned long pci_base,	/* PCI Base address */
			  int state);			/* HIGH or LOW */

void eeprom_delay (int nsec);
static int eeprom_send_start (unsigned long pci_base, int command);
static int eeprom_send_addr (unsigned long pci_base,
			     unsigned char eeprom_addr);
static int eeprom_get_word (unsigned long pci_base,
			    unsigned short *word_addr);
static int eeprom_put_word (unsigned long pci_base,
			    unsigned short data);
static int eeprom_write_enable(unsigned long pci_base);
static int eeprom_write_disable(unsigned long pci_base);

/* global variables */
int powerup_wait_done = 0;		/* set true after power-up wait done */


/*-------------------------------------------------------------
 * Function:	int eeprom_read ()
 *
 * Action:	Read data from the eeprom, place it at p_data
 *
 * Returns:	OK if read worked, EEPROM_NOT_RESPONDING if
 *		read fails.
 *-------------------------------------------------------------*/
int eeprom_read (unsigned long pci_base,/* PCI Base address */
		 int eeprom_addr,	/* word offset from start of eeprom */
		 unsigned short *p_data,/* where to put data in memory */
		 int nwords		/* number of 16bit words to read */
		 )
{
    int status;		/* result code */
    int i;		/* loop variable */

    /*
     * Make sure caller isn't requesting a read beyond the end of the 
     * eeprom.
     */
    if ((eeprom_addr + nwords) > EEPROM_WORD_SIZE)
	return (EEPROM_TO_SMALL);


    /* Read in desired number of words */
    for (i = 0; i < nwords; i++, eeprom_addr++) {
     	/* Select the serial EEPROM */
    	SELECT_557_EEP(pci_base);

	/* Wait CS setup time */
	eeprom_delay (SELECT_SETUP_TIME);

    	/* Send start/read command to begin the read */
    	if (((status = eeprom_send_start (pci_base, EEPROM_READ)) != OK) ||
    	/* Send address */
	    ((status = eeprom_send_addr (pci_base, eeprom_addr)) != OK))
	    return (status);

	if ((status = eeprom_get_word (pci_base, p_data++)) != OK)
	    return (status);

    	/* De-Select the serial EEPROM */
    	DESELECT_557_EEP(pci_base);

	/* wait the required de-select time between commands */
	eeprom_delay (DESELECT_TIME);
    }

    return (OK);
}

/*-------------------------------------------------------------
 * Function:	int eeprom_write ()
 *
 * Action:	Write data from p_data to the eeprom
 *
 * Returns:	OK if write worked, EEPROM_NOT_RESPONDING if
 *		write failed.
 *-------------------------------------------------------------*/
int eeprom_write (unsigned long pci_base,/* PCI Base address */
		 int eeprom_addr,	/* word offset from start of eeprom */
		 unsigned short *p_data,/* data source in memory */
		 int nwords		/* number of 16bit words to read */
		 )
{
    int status;			/* result code */
    int i;			/* loop variable */
    int check_cntr;
    unsigned short data;

    /*
     * Make sure caller isn't requesting a read beyond the end of the 
     * eeprom.
     */
    if ((eeprom_addr + nwords) > EEPROM_WORD_SIZE)
	return (EEPROM_TO_SMALL);
    
    /* enable eeprom writes */
    if ((status = eeprom_write_enable(pci_base)) != OK)
	return(status);

    /* Read in desired number of words */
    for (i = 0; i < nwords; i++, eeprom_addr++) {
     	/* Select the serial EEPROM */
    	SELECT_557_EEP(pci_base);

	/* Wait CS setup time */
	eeprom_delay (SELECT_SETUP_TIME);

    	/* Send start/write command to begin the read */
    	if (((status = eeprom_send_start (pci_base, EEPROM_WRITE)) != OK) ||
    	/* Send address */
	    ((status = eeprom_send_addr (pci_base, eeprom_addr)) != OK))
	    return (status);

	data = *p_data++;
	if ((status = eeprom_put_word (pci_base, data)) != OK)
	    return (status);

    	/* De-Select the serial EEPROM */
    	DESELECT_557_EEP(pci_base);

	/* wait the required de-select time between commands */
	eeprom_delay (DESELECT_TIME);

	/* Re-Select the serial EEPROM */
    	SELECT_557_EEP(pci_base);

	/* now that the write command/data have been clocked into the EEPROM
	   we must wait for the BUSY indicator (DO driven low) to indicate
	   READY (DO driven high) */
	check_cntr = 0;

	while (1) {
	    check_cntr++;

	    if (get_sda_line (pci_base) == HIGH) break;		/* programming complete */

	    if (check_cntr > 100000) {						/* timeout */
		/* De-Select the serial EEPROM */
		DESELECT_557_EEP(pci_base);
		/* wait the required de-select time between commands */
		eeprom_delay (DESELECT_TIME);
		return (EEPROM_ERROR);
	    }
	}

	/* De-Select the serial EEPROM */
    	DESELECT_557_EEP(pci_base);

	/* wait the required de-select time between commands */
	eeprom_delay (DESELECT_TIME);
    }

    /* disable eeprom writes */
    if ((status = eeprom_write_disable(pci_base)) != OK)
	return(status);

    return (OK);
}

/*-------------------------------------------------------------
 * Function:	int eeprom_write_enable ()
 *
 * Action:	Enable writes to the eeprom
 *
 * Returns:	OK if command sent, EEPROM_NOT_RESPONDING if not.
 *
 *-------------------------------------------------------------*/
int eeprom_write_enable (unsigned long pci_base)
{
    int status;				/* result code */

    /* Select the serial EEPROM */
    SELECT_557_EEP(pci_base);

    /* Wait CS setup time */
    eeprom_delay (SELECT_SETUP_TIME);

    /* Send start/write enable command */
    if (((status = eeprom_send_start (pci_base, EEPROM_EWEN)) != OK) ||
	/* Send address */
	((status = eeprom_send_addr (pci_base, EEPROM_EWEN_OP)) != OK))
	return (status);

    /* De-Select the serial EEPROM */
    DESELECT_557_EEP(pci_base);

    /* wait the required de-select time between commands */
    eeprom_delay (DESELECT_TIME);

    return (OK);
}

/*-------------------------------------------------------------
 * Function:	int eeprom_write_disable ()
 *
 * Action:	Disable writes to the eeprom
 *
 * Returns:	OK if command sent, EEPROM_NOT_RESPONDING if not.
 *
 *-------------------------------------------------------------*/
int eeprom_write_disable (unsigned long pci_base)
{
    int status;				/* result code */

    /* Select the serial EEPROM */
    SELECT_557_EEP(pci_base);

    /* Wait CS setup time */
    eeprom_delay (SELECT_SETUP_TIME);

    /* Send start/write enable command */
    if (((status = eeprom_send_start (pci_base, EEPROM_EWDS)) != OK) ||
    	/* Send address */
	 ((status = eeprom_send_addr (pci_base, EEPROM_EWDS_OP)) != OK))
	return (status);

    /* De-Select the serial EEPROM */
    DESELECT_557_EEP(pci_base);

    /* wait the required de-select time between commands */
    eeprom_delay (DESELECT_TIME);

    return (OK);
}


/******************************************************************************
*
* eeprom_delay - delay for a specified number of nanoseconds
*
* Note: this routine is a generous approximation as delays for eeproms
*       are specified as minimums.
*/
void eeprom_delay (int nsec)
{
    extern void polled_delay (int usec);

    /* generously delay 1 usec. for each nsec. */
    polled_delay (nsec);
}

/******************************************************************************
*
* eeprom_send_start - send a start bit with a read opcode to the '557 serial
*		      eeprom
*
*/
static int eeprom_send_start (unsigned long pci_base, int command)
{
    int op_code[2];

    switch (command) {
    case EEPROM_WRITE:
	op_code[0] = LOW;
	op_code[1] = HIGH;
	break;

    case EEPROM_READ:
	op_code[0] = HIGH;
	op_code[1] = LOW;
	break;

    case EEPROM_ERASE:
	op_code[0] = HIGH;
	op_code[1] = HIGH;
	break;

    case EEPROM_EWEN:
    case EEPROM_EWDS:
	op_code[0] = LOW;
	op_code[1] = LOW;
	break;

    default:
	return(EEPROM_INVALID_CMD);
    }

    set_scl_line (pci_base, LOW);
    set_sda_line (pci_base, HIGH);	/* start bit */
    eeprom_delay (DATA_IN_SETUP_TIME);
    set_scl_line (pci_base, HIGH);	/* clock high */
    eeprom_delay (SK_HIGH_PERIOD);
    set_scl_line (pci_base, LOW);	/* clock low */
    eeprom_delay (SK_LOW_PERIOD);

    /* send the opcode */
    set_sda_line (pci_base, op_code[0]);	/* MSB of opcode */
    eeprom_delay (DATA_IN_SETUP_TIME);
    set_scl_line (pci_base, HIGH);	/* clock high */
    eeprom_delay (SK_HIGH_PERIOD);
    set_scl_line (pci_base, LOW);	/* clock low */
    eeprom_delay (SK_LOW_PERIOD);
    set_sda_line (pci_base, op_code[1]);	/* LSB of opcode */
    eeprom_delay (DATA_IN_SETUP_TIME);
    set_scl_line (pci_base, HIGH);	/* clock high */
    eeprom_delay (SK_HIGH_PERIOD);
    set_scl_line (pci_base, LOW);	/* clock low */
    eeprom_delay (SK_LOW_PERIOD);

    return (OK);
}

/******************************************************************************
*
* eeprom_send_addr - send the read address to the '557 serial eeprom
*
*/
static int eeprom_send_addr (unsigned long pci_base,
			     unsigned char eeprom_addr)
{
    register int i;

    /* Do each address bit, MSB => LSB - after each address bit is
       sent, read the EEDO bit on the '557 to check for the "dummy 0 bit"
       which when set to 0, indicates that the address field is complete */
    for (i = 5; i >= 0; i--) {
        /* If this bit is a 1, set SDA high.  If 0, set it low */
        if (eeprom_addr & (1 << i))
            set_sda_line (pci_base, HIGH);
        else
            set_sda_line (pci_base, LOW);
 
        eeprom_delay (DATA_IN_SETUP_TIME); /* Data setup before raising clock */
        set_scl_line (pci_base, HIGH);     /* Clock in this data bit */
        eeprom_delay (SK_HIGH_PERIOD);
        set_scl_line (pci_base, LOW);	   /* Prepare for next bit */
        eeprom_delay (SK_LOW_PERIOD);

	/* check to see if "dummy 0 bit" is set to 0 indicating address
	   complete */
	if (get_sda_line (pci_base) == LOW)
	    break;			   /* address complete */
    }
    return (OK);
}

/******************************************************************************
*
* eeprom_get_word - read a 16 bit word from the '557 serial eeprom
*
* Note: this routine assumes that the start/opcode/address have already
*       been set up
*/
static int eeprom_get_word (unsigned long pci_base,
			    unsigned short *word_addr)
{
    register int i;
 
    /* Do each data bit, MSB => LSB */
    for (i = 15; i >= 0; i--) {
	set_scl_line (pci_base, HIGH);
        eeprom_delay (SK_HIGH_PERIOD);

	if (get_sda_line (pci_base) == HIGH)
	    *word_addr |=  (1 << i);			/* store bit as a '1' */
	else
	    *word_addr &= ~(1 << i);			/* store bit as a '0' */

	set_scl_line (pci_base, LOW);
        eeprom_delay (SK_LOW_PERIOD);
    }
    return (OK);
}

/******************************************************************************
*
* eeprom_put_word - write a 16 bit word to the '557 serial eeprom
*
* Note: this routine assumes that the start/opcode/address have already
*       been set up
*/
static int eeprom_put_word (unsigned long pci_base,
			    unsigned short data)
{
    register int i;
 
    /* Do each data bit, MSB => LSB */
    for (i = 15; i >= 0; i--) {
	if (data & (1 << i))
	    set_sda_line(pci_base, HIGH);
	else
	    set_sda_line(pci_base, LOW);

	eeprom_delay (DATA_IN_SETUP_TIME);
	set_scl_line (pci_base, HIGH);
        eeprom_delay (SK_HIGH_PERIOD);
	set_scl_line (pci_base, LOW);
        eeprom_delay (SK_LOW_PERIOD);
    }
    return (OK);
}

/*-------------------------------------------------------------
 * Function:	void set_scl_line ()
 *
 * Action:	Sets the value of the eeprom's serial clock line
 *		to the value HIGH or LOW.
 *
 * Returns:	N/A.
 *-------------------------------------------------------------*/
static void set_scl_line (unsigned long pci_base, /* PCI address */
			  int state)		  /* HIGH or LOW */
{
    if (state == HIGH)
	SK_HIGH_557_EEP (pci_base);
    else if (state == LOW)
	SK_LOW_557_EEP (pci_base);
}

/*-------------------------------------------------------------
 * Function:	void set_sda_line ()
 *
 * Action:	Sets the value of the eeprom's serial data line
 *		to the value HIGH or LOW.
 *
 * Returns:	N/A.
 *-------------------------------------------------------------*/
static void set_sda_line (unsigned long pci_base, /* PCI address */
			  int state)		  /* HIGH or LOW */
{
    if (state == HIGH)
	EEDI_HIGH_557_EEP (pci_base);
    else if (state == LOW)
	EEDI_LOW_557_EEP (pci_base);
}

/*-------------------------------------------------------------
 * Function:	int get_sda_line ()
 *
 * Action:	Returns the value of the eeprom's serial data line
 *
 * Returns:	HIGH or LOW.
 *-------------------------------------------------------------*/
static int get_sda_line (unsigned long pci_base) /* PCI address */
{
    int ret_val;				/* result code */

    if (EEDO_557_EEP (pci_base))
	ret_val = HIGH;
    else
	ret_val = LOW;

    return (ret_val);
}
