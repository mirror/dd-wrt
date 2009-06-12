/****************************************************************************
*
*	Name:			bspcfg.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 10/25/02 10:55a $
****************************************************************************/

#ifndef BSPCFG_H
#define BSPCFG_H

#include <linux/autoconf.h>

#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE // Set in autoconf.h
	#define DSL_DMA	1	
#else
	#define DSL_DMA	0	
#endif

/***************************************************************
 * The main defines below will configure the BSP default builds
 * appropriately.  If you need to further customize the build per
 * your own need th1n venture below.
 **************************************************************/

#define	ZIGGURAT	1

/*#define  ETHERNET_ADSL_MULTIVC 1*/ 	/* Enable Multi PVCs */

// Set to 1 if the sysInit function is to set the
// RUN_MAP bit and the SDRAM size in the HST_CTRL register.
#define	HOSTINTERFACE_CONTROL	0

// Set to 1 to have the sysInit function automatically set
// the PLL_F and PLL_B registers to the maximum frequency 
// setting of the device as determined by the function
// reading the device id.
#define	PLL_CONTROL		1

#define INCLUDE_AUX_CLK	1

// Something needed to build the Voice stuff
#define RAM53FMOD	1

/***************************************************************
 * CX821XX DEVICE SPECIFIC
 **************************************************************/

/*------------------------------------*/
// Enable level interrupts for ADSL and
// CX821XX critical section
/*------------------------------------*/

/* Indicate whether to use GPIO edge or level interrupt. */
#define LEVEL_MODE_ADSL		1

/***************************************************************
 * SYSTEM FUNCTION SPECIFIC
 **************************************************************/

/*------------------------------------*/
// System Memory Map
/*------------------------------------*/
/*
** Warning!
** SDRAM_SIZE is either 2,4 or 8. Do not choose other values and
** do not comment SDRAM_SIZE out.
*/
#define SDRAM_SIZE 8

/*------------------------------------*/
// DMA Memory Map
/*------------------------------------*/

#define  DSL_DMA_IN_SRAM   1


/*------------------------------------*/
// Showtime Defines
/*------------------------------------*/

#if DSL_DMA
	/* This assumes that Showtime is linked into the BSP */
	#define SHOWTIME_AUTO_START		1
#else
	#define SHOWTIME_AUTO_START		0
#endif


/*------------------------------------*/
// Power Management
/*------------------------------------*/

#define   POWER_CTRL		1

/***************************************************************
 * MISC DEFINES
 **************************************************************/

#if DSL_DMA
	#ifdef   ETHERNET_ADSL_MULTIVC 
		#define  MAX_VC_PER_LINK 8
	#endif

	// NOTE: The following two defines need to be set
	//  to one for the DL10_D325_003 Hasbani board.
	#define DL10_D325_003	0
#endif

/*------------------------------------*/
// Enable EEPROM Write Function
/*------------------------------------*/


/*#define EEPROM_WRITE       1*/

/*------------------------------------*/
// LED Pattern
/*------------------------------------*/

#define COMMON_TX_RX_INDICATION   0      /* ADSL TX and RX share same INDICATION */


// Use to change console interface hardware from
// uart 1 to uart 2 for hardware sanity check purposes.

#define DEBUG_CONSOLE_ON_UART_2		0


// Used to enable/disable the file operations used in
// the HAL drivers of the VOP.

#define OS_FILE_SYSTEM_SUPPORTED	0

// This needs to be set to 1 if you are building the
// code to run on an Oakland board that has this version
// of the PLD code loaded into the XC95144 device.
// If you do not the UART will not be addressed properly
// See the cnxtserial.h file for where the UART is mapped.
#define PLD_CS4_UART_v100	0

#endif
