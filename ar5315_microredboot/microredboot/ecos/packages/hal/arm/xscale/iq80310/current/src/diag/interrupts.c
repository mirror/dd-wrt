//=============================================================================
//
//      interrupts.c - Cyclone Diagnostics
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

/******************************************************************************/
/* interrupts.c - Interrupt dispatcher routines for IQ80310 Board			  */
/*																			  */
/* modification history														  */
/* --------------------														  */
/* 07sep00, ejb, Written for IQ80310 Cygmon diagnostics						  */
/* 11oct00, ejb, Switched FIQ and IRQ interrupt handlers					  */
/* 18dec00  snc and jwf                                                       */
/* 02feb01  jwf for snc                                                       */
/******************************************************************************/

#include "iq80310.h"
#include "pci_bios.h"
#include "7_segment_displays.h"

extern int(*board_fiq_handler)(void);
extern int(*board_irq_handler)(void);
extern long _cspr_enable_fiq_int(void);
extern long _cspr_enable_irq_int(void);
extern long _read_cpsr(void);
extern long _scrub_ecc(unsigned);
extern void _flushICache(void);


#define AND_WORD(addr,val)   *addr = *addr & val


void error_print(char *fmt, int arg0, int arg1, int arg2, int arg3);


extern int nmi_verbose;	/* for NMI, only print NMI info if this is TRUE */
extern int pci_config_cycle; /* don't handle NMI if in a config cycle */
extern int pci_config_error;

typedef struct
{
	INTFUNCPTR	handler;
	int		arg;
	int		bus;
	int		device;
} INT_HANDLER;

extern UINT	secondary_busno;
extern UINT	primary_busno;

extern STATUS pci_to_xint(int device, int intpin, int *xint);
extern int isHost(void);
extern int off_ppci_bus (int busno);

#define MAX_SPURIOUS_CNT	5
#define NUM_PCI_XINTS		4		/* SINTA - SINTD */
#define MAX_PCI_HANDLERS	8		/* maximum handlers per PCI Xint */

/* 02/02/01 jwf */
int ecc_error_reported = FALSE;

static int isr_xint0_spurious = 0;
static int isr_xint1_spurious = 0;
static int isr_xint2_spurious = 0;
static int isr_xint3_spurious = 0;

/* Table where the interrupt handler addresses are stored. */
INT_HANDLER pci_int_handlers[4][MAX_PCI_HANDLERS];

/* Other User Interrupt Service Routines */

void (*usr_timer_isr)(int) = NULL;
int usr_timer_arg = 0;
void (*usr_enet_isr)(int) = NULL;
int usr_enet_arg = 0;
void (*usr_uart1_isr)(int) = NULL;
int usr_uart1_arg = 0;
void (*usr_uart2_isr)(int) = NULL;
int usr_uart2_arg = 0;
void (*usr_dma0_isr)(int) = NULL;
int usr_dma0_arg = 0;
void (*usr_dma1_isr)(int) = NULL;
int usr_dma1_arg = 0;
void (*usr_dma2_isr)(int) = NULL;
int usr_dma2_arg = 0;
void (*usr_pm_isr)(int) = NULL;
int usr_pm_arg = 0;
void (*usr_aa_isr)(int) = NULL;
int usr_aa_arg = 0;
void (*usr_i2c_isr)(int) = NULL;
int usr_i2c_arg = 0;
void (*usr_mu_isr)(int) = NULL;
int usr_mu_arg = 0;
void (*usr_patu_isr)(int) = NULL;
int usr_patu_arg = 0;

int ecc_int_handler(void);



/*********************************
* PCI interrupt wrappers 
*/
int sinta_handler(void)
{
    int x, serviced = 0;
	/* cycle through connected interrupt handlers to determine which caused int */
	for (x = 0; x < MAX_PCI_HANDLERS; x++) 
	{
		if (pci_int_handlers[0][x].handler != NULL)	/* Is a routine installed */
			if ((*pci_int_handlers[0][x].handler)(pci_int_handlers[0][x].arg) == 1)	
			{
				serviced = 1;
				break;
			}
	}
	if (serviced == 0)
	{
		isr_xint0_spurious++;

		if (isr_xint0_spurious > MAX_SPURIOUS_CNT)
			; 
	}
	else
		isr_xint0_spurious = 0;

	return (serviced);

}

int sintb_handler(void)
{
    int x, serviced = 0;
	
	/* cycle through connected interrupt handlers to determine which caused int */
	for (x = 0; x < MAX_PCI_HANDLERS; x++) 
	{
		if (pci_int_handlers[1][x].handler != NULL)	/* Is a routine installed */
			if ((*pci_int_handlers[1][x].handler)(pci_int_handlers[1][x].arg) == 1)	
			{
				serviced = 1;
				break;
			}
	}
	if (serviced == 0)
	{
		isr_xint1_spurious++;

		if (isr_xint1_spurious > MAX_SPURIOUS_CNT)
			;
	}
	else
		isr_xint1_spurious = 0;

	return (serviced);

}

int sintc_handler(void)
{

    int x, serviced = 0;

	/* cycle through connected interrupt handlers to determine which caused int */
	for (x = 0; x < MAX_PCI_HANDLERS; x++) 
	{
		if (pci_int_handlers[2][x].handler != NULL)	/* Is a routine installed */
			if ((*pci_int_handlers[2][x].handler)(pci_int_handlers[2][x].arg) == 1)	
			{
				serviced = 1;
				break;
			}
	}
	if (serviced == 0)
	{
		isr_xint2_spurious++;

		if (isr_xint2_spurious > MAX_SPURIOUS_CNT)
			;
	}
	else
		isr_xint2_spurious = 0;

	return (serviced);

}

int sintd_handler(void)
{

    int x, serviced = 0;

	/* cycle through connected interrupt handlers to determine which caused int */
	for (x = 0; x < MAX_PCI_HANDLERS; x++) 
	{
		if (pci_int_handlers[3][x].handler != NULL)	/* Is a routine installed */
			if ((*pci_int_handlers[3][x].handler)(pci_int_handlers[3][x].arg) == 1)	
			{
				serviced = 1;
				break;
			}
	}
	if (serviced == 0)
	{
		isr_xint3_spurious++;

		if (isr_xint3_spurious > MAX_SPURIOUS_CNT)
			;
	}
	else
		isr_xint3_spurious = 0;

	return (serviced);


}


/******************************************************************************
*
* Installs an interrupt handler in the PCI dispatch table, to be called
* by the appropriate PCI isr (above) when an interrupt occurs.
*
* Note: the intline parameter refers to which PCI interrupt INT A - INT D
*
*       device identifies the PCI device number
*
* Note: isrs connected with this function must return 1 if an interrupt is 
*	serviced in order to support the PCI interrupt sharing mechanism
*                                                   
*/
STATUS pci_isr_connect (int intline, int bus, int device, int (*handler)(int), int arg)
{
	int which_xint;
	int handler_index;

	/* check to see if we are attempting to connect to a PPCI interrupt and we are not
	   a host card */
	if ((isHost() == FALSE) && (off_ppci_bus(bus) == TRUE))
		return (ERROR);

	if ((intline < INTA) || (intline > INTD))
		return (ERROR);

    (void)pci_to_xint(device, intline, &which_xint);

	for (handler_index = 0; handler_index < MAX_PCI_HANDLERS; handler_index++)
	{
		if (pci_int_handlers[which_xint][handler_index].handler == NULL)
		{
			pci_int_handlers[which_xint][handler_index].handler = handler;
			pci_int_handlers[which_xint][handler_index].arg		= arg;
			pci_int_handlers[which_xint][handler_index].bus		= bus;
			pci_int_handlers[which_xint][handler_index].device	= device;
			break;
		}
	}

	/* if there is no more room in the table return an error */
	if (handler_index == MAX_PCI_HANDLERS)
		return (ERROR);

    return (OK);
}


/******************************************************************************
*
* Uninstalls an interrupt handler in the PCI dispatch table
*
* Note: the intline parameter refers to which PCI interrupt INTA - INTD
*
*       the device parameter refers to which SPCI device number is sourcing the
*       interrupt
*
*/
STATUS pci_isr_disconnect (int intline, int bus, int device)
{
	int which_xint;
	int handler_index;

	/* check to see if we are attempting to disconnect a PPCI interrupt and we are not
	   a host card */
	if ((isHost() == FALSE) && (off_ppci_bus(bus) == TRUE))
		return (ERROR);

	if ((intline < INTA) || (intline > INTD))
		return (ERROR);

    (void)pci_to_xint(device, intline, &which_xint);

	for (handler_index = 0; handler_index < MAX_PCI_HANDLERS; handler_index++)
	{
		if ((pci_int_handlers[which_xint][handler_index].bus == bus) &&
			(pci_int_handlers[which_xint][handler_index].device == device))
		{
			pci_int_handlers[which_xint][handler_index].handler = NULL;
			pci_int_handlers[which_xint][handler_index].arg		= (int)NULL;
			pci_int_handlers[which_xint][handler_index].bus		= (int)NULL;
			pci_int_handlers[which_xint][handler_index].device	= (int)NULL;
		}
	}

	/* if the handler was not found in the table return an error */
	if (handler_index == MAX_PCI_HANDLERS)
		return (ERROR);

    return (OK);
}


/**********************************************************************************
* iq80310_irq_handler - Interrupt dispatcher for IQ80310 IRQ Interrupts
*
* This function determines the source of the IRQ Interrupt, and calls the 
* corresponding interrupt service routine.  If multiple sources are interrupting
* the dispatcher will call all interrupt handles.  Users must clear the interrupt
* within the interrupt service routine before exiting.
*
* IRQ Interrupts are multiplexed from SPCI INTA - INTD, External Device Interrupts,
* and XINT6 and XINT7 Internal device interrupts.
*/
int iq80310_irq_handler(void)
{
UINT8* int_status_reg;
UINT8  int_status;	
int num_sources = 0;


/* 12/18/00 jwf */
unsigned char ri_state;
unsigned char board_rev;
unsigned char sint_status;						/* holds interrupt status for SINTA-SINTC */
	ri_state = *( unsigned char * ) 0xfe810006;	/* access uart u2 msr reg at addr fe810006 */
	ri_state &= RI_MASK;
	if(ri_state == RI_MASK)						/* RI# pin on UART2 is grounded */
	{
		board_rev = *BOARD_REV_REG_ADDR;		/* read Board Revision register */
		board_rev &= BOARD_REV_MASK;			/* isolate LSN */
		if (board_rev >= BOARD_REV_E)			/* Board Rev is at E or higher */
		{
			sint_status = *SINT_REG_ADDR;		/* read current secondary pci interrupt status */
			sint_status &= SINT_MASK;			/* isolate SINTA, SINTB, and SINTC */
			switch(sint_status)
			{
				case SINTA_INT:
					num_sources += sinta_handler();	/* IRQ0 = SINTA? */
/*					printf(" sinta status = %#x\n", sint_status); */
					break;
				case SINTB_INT:
					num_sources += sintb_handler();	/* IRQ1 = SINTB? */
/*					printf(" sintb status = %#x\n", sint_status); */
					break;			
				case SINTC_INT:
					num_sources += sintc_handler();	/* IRQ2 = SINTC? */
/*					printf(" sintc status = %#x\n", sint_status); */
					break;
				default:
/*					printf(" sint? status = %#x\n", sint_status); */
					break;						/* probably should test for more conditions: 011b, 101b, 110b, 111b */
			}
		}
	}
	else										/* RI# pin on UART2 is pulled up to 3.3V. Cannot read board revision register, not implemented */
	{
		num_sources += sinta_handler();			/* IRQ0 = SINTA? */
		num_sources += sintb_handler();			/* IRQ1 = SINTB? */
		num_sources += sintc_handler();			/* IRQ2 = SINTC? */
	}


	/* 12/18/00 jwf */
	/* Original code */
	/* No S_INTA - S_INTC status register, call handlers always */
	/* This may change in next revision of board */
	/*num_sources += sinta_handler();*/	/* IRQ0 = SINTA? */
	/*num_sources += sintb_handler();*/	/* IRQ1 = SINTB? */
	/*num_sources += sintc_handler();*/	/* IRQ2 = SINTC? */


	/* Read IRQ3 Status Register, and if any of the multiple sources are 
	   interrupting, call corresponding handler */
	int_status_reg = (UINT8 *)X3ISR_ADDR;
	int_status = *int_status_reg;
	{ 
		if (int_status & TIMER_INT) /* timer interrupt? */
		{
			/* call user ISR, if connected */
			if (usr_timer_isr != NULL)
				(*usr_timer_isr)(usr_timer_arg);
			else
				printf ("\nUnhandled Timer Interrupt Detected!\n");
			
			num_sources++;
		}

		if (int_status & ENET_INT)	/* ethernet interrupt? */
		{
			/* call user ISR, if connected */
			if (usr_enet_isr != NULL)
				(*usr_enet_isr)(usr_enet_arg);
			else
				printf ("\nUnhandled Ethernet Interrupt Detected!\n");

			num_sources++;
		}

		if (int_status & UART1_INT) /* uart1 interrupt? */
		{
			/* call user ISR, if connected */
			if (usr_uart1_isr != NULL)
				(*usr_uart1_isr)(usr_uart1_arg);
			else
				printf ("\nUnhandled UART1 Interrupt Detected!\n");

			num_sources++;
		}

		if (int_status & UART2_INT) /* uart2 interrupt? */
		{
			/* call user ISR, if connected */
			if (usr_uart2_isr != NULL)
				(*usr_uart2_isr)(usr_uart2_arg);
			else
				printf ("\nUnhandled UART2 Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & SINTD_INT)	/* SPCI_INTD? */
		{
			num_sources += sintd_handler();
		}
	}


	/* Read XINT6 Status Register, and if any of the multiple sources are 
	   interrupting, call corresponding handler */
	int_status_reg = (UINT8 *)X6ISR_ADDR;
	int_status = *int_status_reg;
	{
		if (int_status & DMA0_INT) /* dma0 interrupt? */
		{
			if (usr_dma0_isr != NULL)
				(*usr_dma0_isr)(usr_dma0_arg);
			else
				printf ("\nUnhandled DMA Channel 0 Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & DMA1_INT) /* dma1 interrupt? */
		{
			if (usr_dma1_isr != NULL)
				(*usr_dma1_isr)(usr_dma1_arg);
			else
				printf ("\nUnhandled DMA Channel 1 Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & DMA2_INT) /* dma2 interrupt? */
		{
			if (usr_dma2_isr != NULL)
				(*usr_dma2_isr)(usr_dma2_arg);
			else
				printf ("\nUnhandled DMA Channel 2 Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & PM_INT) /* performance monitoring interrupt? */
		{
			if (usr_pm_isr != NULL)
				(*usr_pm_isr)(usr_pm_arg);
			else
				printf ("\nUnhandled Performance Monitoring Unit Interrupt Detected!\n");
			num_sources++;
		}
		
		if (int_status & AA_INT) /* application accelerator interrupt? */
		{
			if (usr_aa_isr != NULL)
				(*usr_aa_isr)(usr_aa_arg);
			else
				printf ("\nUnhandled Application Accelerating Unit Interrupt Detected!\n");
			num_sources++;
		}
	}


	/* Read XINT7 Status Register, and if any of the multiple sources are 
	   interrupting, call corresponding handler */
	int_status_reg = (UINT8 *)X7ISR_ADDR;
	int_status = *int_status_reg;
	{
		if (int_status & I2C_INT) /* i2c interrupt? */
		{
			if (usr_i2c_isr != NULL)
				(*usr_i2c_isr)(usr_i2c_arg);
			else
				printf ("\nUnhandled I2C Unit Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & MU_INT) /* messaging unit interrupt? */
		{	
			if (usr_mu_isr != NULL)
				(*usr_mu_isr)(usr_mu_arg);
			else
				printf ("\nUnhandled Messaging Unit Interrupt Detected!\n");
			num_sources++;
		}

		if (int_status & PATU_INT) /* primary ATU / BIST start interrupt? */
		{
			if (usr_patu_isr != NULL)
				(*usr_patu_isr)(usr_patu_arg);
			else
				printf ("\nUnhandled Primary ATU Interrupt Detected!\n");
			num_sources++;
		}
	}

	/* return the number of interrupt sources found */
	return (num_sources);
}





/****************************************************************
*  nmi_ecc_isr - ECC NMI Interrupt Handler
*
*  This module handles the NMI caused by an ECC error.
*  For a Single-bit error it does a read-nodify-write
*  to correct the error in memory. For a multi-bit or
*  nibble error it does absolutely nothing.
*/
void nmi_ecc_isr(void)
{
	UINT32 eccr_register;
	UINT32* reg32;

	/* Read current state of ECC register */
	eccr_register = *(UINT32 *)ECCR_ADDR;

	/* Turn off all ecc error reporting */
	*(UINT32 *)ECCR_ADDR = 0x4;

	/* Check for ECC Error 0 */
	if(*(UINT32 *)MCISR_ADDR & 0x1)
	{
        reg32 = (UINT32*)ELOG0_ADDR;
		error_print("ELOG0 = 0x%X\n",*reg32,0,0,0);
		
		reg32 = (UINT32*)ECAR0_ADDR;
		error_print("ECC Error Detected at Address 0x%X\n",*reg32,0,0,0);		
	
		/* Check for single-bit error */
        if(!(*(UINT32 *)ELOG0_ADDR & 0x00000100))
        {
			/* call ECC restoration function */
			_scrub_ecc(*reg32);

			/* Clear the MCISR */
		    *(UINT32 *)MCISR_ADDR = 0x1;
        }
        else
            error_print("Multi-bit or nibble error\n",0,0,0,0);
	}

	/* Check for ECC Error 1 */
	if(*(UINT32 *)MCISR_ADDR & 0x2)
	{
		reg32 = (UINT32*)ELOG1_ADDR;
		error_print("ELOG0 = 0x%X\n",*reg32,0,0,0);
		
		reg32 = (UINT32*)ECAR1_ADDR;
		error_print("ECC Error Detected at Address 0x%X\n",*reg32,0,0,0);	
        
		/* Check for single-bit error */
        if(!(*(UINT32 *)ELOG1_ADDR & 0x00000100))
        {
			/* call ECC restoration function */
			_scrub_ecc(*reg32);
 
			/* Clear the MCISR */
			*(UINT32 *)MCISR_ADDR = 0x2;
       }
       else
            error_print("Multi-bit or nibble error\n",0,0,0,0);
	}

	/* Check for ECC Error N */
	if(*(UINT32 *)MCISR_ADDR & 0x4)
	{
		/* Clear the MCISR */
		*(UINT32 *)MCISR_ADDR = 0x4;
		error_print("Uncorrectable error during RMW\n",0,0,0,0);
	}
    
	/* Turn on  ecc error reporting */
	*(UINT32 *)ECCR_ADDR = eccr_register;
}




/******************************************************************************
* iq80310_fiq_handler - Interrupt dispatcher for IQ80310 FIQ Interrupts
*
*
*/
int iq80310_fiq_handler(void)
{
	
unsigned long nmi_status = *(volatile unsigned long *)NISR_ADDR;
unsigned long status;
int srcs_found = 0;

	if (nmi_status & MCU_ERROR)
	{
		status = *(volatile unsigned long *)MCISR_ADDR;
		*MSB_DISPLAY_REG = LETTER_E;
		if (status & 0x001)
			*LSB_DISPLAY_REG = ONE;
		if (status & 0x002)
			*LSB_DISPLAY_REG = TWO;
		if (status & 0x004)
			*LSB_DISPLAY_REG = FOUR;
		srcs_found++;
#if 0
		error_print ("**** 80312 Memory Controller Error ****\n",0,0,0,0);
		if (status & 0x001) error_print ("One ECC Error Detected and Recorded in ELOG0\n",0,0,0,0);
		if (status & 0x002) error_print ("Second ECC Error Detected and Recorded in ELOG1\n",0,0,0,0);
		if (status & 0x004) error_print ("Multiple ECC Errors Detected\n",0,0,0,0);
#endif

		/* call ecc interrupt handler */
		nmi_ecc_isr(); 

		/* clear the interrupt condition*/
		AND_WORD((volatile unsigned long *)MCISR_ADDR, 0x07);

/* 02/02/01 jwf */
		ecc_error_reported = TRUE;

	}


	if (nmi_status & PATU_ERROR)
	{
		srcs_found++;
		error_print ("**** Primary ATU Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)PATUISR_ADDR;
		if (status & 0x001) error_print ("PPCI Master Parity Error\n",0,0,0,0);
		if (status & 0x002) error_print ("PPCI Target Abort (target)\n",0,0,0,0);
		if (status & 0x004) error_print ("PPCI Target Abort (master)\n",0,0,0,0);
		if (status & 0x008) error_print ("PPCI Master Abort\n",0,0,0,0);
		if (status & 0x010) error_print ("Primary P_SERR# Detected\n",0,0,0,0);
		if (status & 0x080) error_print ("Internal Bus Master Abort\n",0,0,0,0);
		if (status & 0x100) error_print ("PATU BIST Interrupt\n",0,0,0,0);
		if (status & 0x200) error_print ("PPCI Parity Error Detected\n",0,0,0,0);
		if (status & 0x400) error_print ("Primary P_SERR# Asserted\n",0,0,0,0);

		/* clear the interrupt conditions */
		AND_WORD((volatile unsigned long *)PATUISR_ADDR, 0x79f);
		CLEAR_PATU_STATUS();

		/* tell the config cleanup code about error */
		if (pci_config_cycle == 1) 
			pci_config_error = TRUE;
	}

	if (nmi_status & SATU_ERROR)
	{
		srcs_found++;
		error_print ("**** Secondary ATU Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)SATUISR_ADDR;
		if (status & 0x001) error_print ("SPCI Master Parity Error\n",0,0,0,0);
		if (status & 0x002) error_print ("SPCI Target Abort (target)\n",0,0,0,0);
		if (status & 0x004) error_print ("SPCI Target Abort (master)\n",0,0,0,0);
		if (status & 0x008) error_print ("SPCI Master Abort\n",0,0,0,0);
		if (status & 0x010) error_print ("Secondary P_SERR# Detected\n",0,0,0,0);
		if (status & 0x080) error_print ("Internal Bus Master Abort\n",0,0,0,0);
		if (status & 0x200) error_print ("SPCI Parity Error Detected\n",0,0,0,0);
		if (status & 0x400) error_print ("Secondary S_SERR# Asserted\n",0,0,0,0);

		/* clear the interrupt conditions */
		AND_WORD((volatile unsigned long *)SATUISR_ADDR, 0x69f);
		CLEAR_SATU_STATUS();

		/* tell the config cleanup code about error */
		if (pci_config_cycle == 1) 
			pci_config_error = TRUE;
	}

	if (nmi_status & PBRIDGE_ERROR)
	{
		srcs_found++;
		error_print ("**** Primary Bridge Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)PBISR_ADDR;
		if (status & 0x001) error_print ("PPCI Master Parity Error\n",0,0,0,0);
		if (status & 0x002) error_print ("PPCI Target Abort (Target)\n",0,0,0,0);
		if (status & 0x004) error_print ("PPCI Target Abort (Master)\n",0,0,0,0);
		if (status & 0x008) error_print ("PPCI Master Abort\n",0,0,0,0);
		if (status & 0x010) error_print ("Primary P_SERR# Asserted\n",0,0,0,0);
		if (status & 0x020) error_print ("PPCI Parity Error Detected\n",0,0,0,0);

		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)PBISR_ADDR, 0x3f);
		CLEAR_PBRIDGE_STATUS();

		/* tell the config cleanup code about error */
		if (pci_config_cycle == 1) 
			pci_config_error = TRUE;
	}

	if (nmi_status & SBRIDGE_ERROR)
	{
		srcs_found++;

		/* don't print configuration secondary bridge errors */

		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)SBISR_ADDR, 0x7f);
		CLEAR_SBRIDGE_STATUS();

		/* tell the config cleanup code about error */
		if (pci_config_cycle == 1) 
			pci_config_error = TRUE;
	}

	if (nmi_status & DMA_0_ERROR)
	{
		srcs_found++;
		error_print ("**** DMA Channel 0 Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)CSR0_ADDR;
		if (status & 0x001) error_print ("DMA Channel 0 PCI Parity Error\n",0,0,0,0);
		if (status & 0x004) error_print ("DMA Channel 0 PCI Target Abort\n",0,0,0,0);
		if (status & 0x008) error_print ("DMA Channel 0 PCI Master Abort\n",0,0,0,0);
		if (status & 0x020) error_print ("Internal PCI Master Abort\n",0,0,0,0);    
		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)CSR0_ADDR, 0x2D);
	}

	if (nmi_status & DMA_1_ERROR)
	{
		srcs_found++;
		error_print ("**** DMA Channel 1 Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)CSR1_ADDR;
		if (status & 0x001) error_print ("DMA Channel 1 PCI Parity Error\n",0,0,0,0);
		if (status & 0x004) error_print ("DMA Channel 1 PCI Target Abort\n",0,0,0,0);
		if (status & 0x008) error_print ("DMA Channel 1 PCI Master Abort\n",0,0,0,0);
		if (status & 0x020) error_print ("Internal PCI Master Abort\n",0,0,0,0);  
	    
		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)CSR1_ADDR, 0x2D);
	}

	if (nmi_status & DMA_2_ERROR)
	{
		srcs_found++;
		error_print ("**** DMA Channel 2 Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)CSR2_ADDR;
		if (status & 0x001) error_print ("DMA Channel 2 PCI Parity Error\n",0,0,0,0);
		if (status & 0x004) error_print ("DMA Channel 2 PCI Target Abort\n",0,0,0,0);
		if (status & 0x008) error_print ("DMA Channel 2 PCI Master Abort\n",0,0,0,0);
		if (status & 0x020) error_print ("Internal PCI Master Abort\n",0,0,0,0);  
		
		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)CSR2_ADDR, 0x2D);
	}

	if (nmi_status & MU_ERROR)
	{	
		status = *(volatile unsigned long *)IISR_ADDR;
		if (status & 0x20)
		{
			srcs_found++;
			error_print ("Messaging Unit Outbound Free Queue Overflow\n",0,0,0,0);

			/* clear the interrupt condition; note that the clearing of the NMI doorbell
			is handled by the PCI comms code */
		}	AND_WORD((volatile unsigned long *)IISR_ADDR, 0x20);	
	}

	if (nmi_status & AAU_ERROR)
	{
		srcs_found++;
		error_print ("**** Application Accelerator Unit Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)ASR_ADDR;
		if (status & 0x020) error_print ("Internal PCI Master Abort\n",0,0,0,0);

		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)ASR_ADDR, 0x20);
	}

	if (nmi_status & BIU_ERROR)
	{
		srcs_found++;
		error_print ("**** Bus Interface Unit Error ****\n",0,0,0,0);
		status = *(volatile unsigned long *)BIUISR_ADDR;
		if (status & 0x004) error_print ("Internal PCI Master Abort\n",0,0,0,0);

		/* clear the interrupt condition */
		AND_WORD((volatile unsigned long *)BIUISR_ADDR, 0x04);
	}

	return (srcs_found);

}


/**********************************************************************
* isr_connect - Disconnect a user Interrupt Service Routine
*
* NOT TO BE USED FOR SPCI INTERRUPTS! - use pci_isr_connect instead
*
*/
int isr_connect(int int_num, void (*handler)(int), int arg)
{
    switch (int_num)
    {

	case DMA0_INT_ID:
	    usr_dma0_isr = handler;
	    usr_dma0_arg = arg;
	    break;
	case DMA1_INT_ID:
	    usr_dma1_isr = handler;
	    usr_dma1_arg = arg;
	    break;
	case DMA2_INT_ID:
	    usr_dma2_isr = handler;
	    usr_dma2_arg = arg;
	    break;
	case PM_INT_ID:
		usr_pm_isr = handler;
		usr_pm_arg = arg;
		break;
	case AA_INT_ID:
		usr_aa_isr = handler;
		usr_aa_arg = arg;
		break;
	case I2C_INT_ID: 
		usr_i2c_isr = handler;
		usr_i2c_arg = arg;
		break;
	case MU_INT_ID:
		usr_mu_isr = handler;
		usr_mu_arg = arg;
		break;
	case PATU_INT_ID:
		usr_patu_isr = handler;
		usr_patu_arg = arg;
		break;
	case TIMER_INT_ID:
		usr_timer_isr = handler;
		usr_timer_arg = arg;
		break;
	case ENET_INT_ID:
		usr_enet_isr = handler;
		usr_enet_arg = arg;
		break;
	case UART1_INT_ID:
		usr_uart1_isr = handler;
		usr_uart1_arg = arg;
		break;
	case UART2_INT_ID:
		usr_uart2_isr = handler;
		usr_uart2_arg = arg;
		break;
	default:
		return (ERROR);
		break;
	}

	return (OK);
}

/**********************************************************************
* isr_disconnect - Disconnect a user Interrupt Service Routine
*
* NOT TO BE USED FOR SPCI INTERRUPTS! - use pci_isr_disconnect instead
*
*/
int isr_disconnect(int int_num)
{
    switch (int_num)
    {

	case DMA0_INT_ID:
	    usr_dma0_isr = NULL;
	    usr_dma0_arg = 0;
	    break;
	case DMA1_INT_ID:
	    usr_dma1_isr = NULL;
	    usr_dma1_arg = 0;
	    break;
	case DMA2_INT_ID:
	    usr_dma2_isr = NULL;
	    usr_dma2_arg = 0;
	    break;
	case PM_INT_ID:
		usr_pm_isr = NULL;
		usr_pm_arg = 0;
		break;
	case AA_INT_ID:
		usr_aa_isr = NULL;
		usr_aa_arg = 0;
		break;
	case I2C_INT_ID: 
		usr_i2c_isr = NULL;
		usr_i2c_arg = 0;
		break;
	case MU_INT_ID:
		usr_mu_isr = NULL;
		usr_mu_arg = 0;
		break;
	case PATU_INT_ID:
		usr_patu_isr = NULL;
		usr_patu_arg = 0;
		break;
	case TIMER_INT_ID:
		usr_timer_isr = NULL;
		usr_timer_arg = 0;
		break;
	case ENET_INT_ID:
		usr_enet_isr = NULL;
		usr_enet_arg = 0;
		break;
	case UART1_INT_ID:
		usr_uart1_isr = NULL;
		usr_uart1_arg = 0;
		break;
	case UART2_INT_ID:
		usr_uart2_isr = NULL;
		usr_uart2_arg = 0;
		break;
	default:
		return (ERROR);
		break;
	}

	/* i960 disabled interrupt here - should we? */

	return (OK);
}

/********************************************************************
* disable_external_interrupt - Mask an external interrupt 
*
*/
int disable_external_interrupt(int int_id)
{

unsigned char* ext_mask_reg = (unsigned char*) X3MASK_ADDR;
unsigned char  new_mask_value;

	/* make sure interrupt to enable is an external interrupt */
	if ((int_id < TIMER_INT_ID) || (int_id > SINTD_INT_ID))
		return (ERROR);

	new_mask_value = *ext_mask_reg; /* read current mask status */
	
	switch (int_id)
	{
	case TIMER_INT_ID:
		new_mask_value |= TIMER_INT;
		break;
	case ENET_INT_ID:
		new_mask_value |= ENET_INT;
		break;
	case UART1_INT_ID:
		new_mask_value |= UART1_INT;
		break;
	case UART2_INT_ID:
		new_mask_value |= UART2_INT;
		break;
	case SINTD_INT_ID:
		new_mask_value |= SINTD_INT;
		break;
	default:
		break; /* leave mask register as it was */
	}

	*ext_mask_reg = new_mask_value; /* set new mask value */

	return (OK);

}



/********************************************************************
* enable_external_interrupt - Unmask an external interrupt 
*
*/
int enable_external_interrupt(int int_id)
{

unsigned char* ext_mask_reg = (unsigned char*) X3MASK_ADDR;
unsigned char  new_mask_value;

	/* make sure interrupt to enable is an external interrupt */
	if ((int_id < TIMER_INT_ID) || (int_id > SINTD_INT_ID))
		return (ERROR);

	
	new_mask_value = *ext_mask_reg; /* read current mask status */
	
	switch (int_id)
	{
	case TIMER_INT_ID:
		new_mask_value &= ~(TIMER_INT);
		break;
	case ENET_INT_ID:
		new_mask_value &= ~(ENET_INT);
		break;
	case UART1_INT_ID:
		new_mask_value &= ~(UART1_INT);
		break;
	case UART2_INT_ID:
		new_mask_value &= ~(UART2_INT);
		break;
	case SINTD_INT_ID:
		new_mask_value &= ~(SINTD_INT);
		break;
	default:
		break; /* leave mask register as it was */
	}

	*ext_mask_reg = new_mask_value; /* set new mask value */

	return (OK);
}


void error_print (
		char *fmt,
		int arg0,
		int arg1,
		int arg2,
		int arg3
		)
{
	/* Wait until host configures the boards to start printing NMI errors */
	UINT32* atu_reg = (UINT32*)PIABAR_ADDR;
	if ((*atu_reg & 0xfffffff0) == 0)
		return;
	if (nmi_verbose) printf (fmt, arg0, arg1, arg2, arg3);
		return;
}

extern void __diag_IRQ(void);
extern void __diag_FIQ(void);

void config_ints(void)
{
int xint, x;
	
	unsigned int* pirsr_ptr = (unsigned int*)PIRSR_ADDR;
	*pirsr_ptr = 0xf; /* this is an errata in the original Yavapai manual.
					     The interrupt steering bits are reversed, so a '1'
						 routes XINT interrupts to FIQ
						*/

	/* install diag IRQ handlers */
	((volatile unsigned *)0x20)[6] = (unsigned)__diag_IRQ;
	((volatile unsigned *)0x20)[7] = (unsigned)__diag_FIQ;
	_flushICache();	

	/* make sure interrupts are enabled in CSPR */

	_cspr_enable_irq_int(); 

	_cspr_enable_fiq_int(); 

	/* initialize the PCI interrupt table */
	for (xint = 0; xint < NUM_PCI_XINTS; xint++)
	{
		for (x = 0; x < MAX_PCI_HANDLERS; x++) 
		{
			pci_int_handlers[xint][x].handler	= NULL;
			pci_int_handlers[xint][x].arg		= (int)NULL;
			pci_int_handlers[xint][x].bus		= (int)NULL;
			pci_int_handlers[xint][x].device	= (int)NULL;
		}
	}


}



