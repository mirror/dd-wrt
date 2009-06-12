/****************************************************************************
*
*	Name:			hstif.h
*
*	Description:	Host Interface header file
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
*  $Modtime: 2/28/02 8:04a $
****************************************************************************/

/*  Host Control Reg    */
#define HST_CTRL        0x002D0000  /* Register used to select host brd options*/

/*  Host Control bit definitions */
#define HSTCTRL_4MEG    BIT2
#define HSTCTRL_8MEG    BIT3
#define HSTCTRL_RUN_MAP BIT4  /* This bit remaps SRAM to blk 0 when set  */

#define pHST_CtrlReg		      ( volatile UINT32* )0x002D0000
#define pHST_RwstReg		      ( volatile UINT32* )0x002D0004
#define pHST_WwstReg		      ( volatile UINT32* )0x002D0008
#define pHST_Xfer_CntlReg	   ( volatile UINT32* )0x002D000C
#define pHST_Read_Cntl1Reg	   ( volatile UINT32* )0x002D0010
#define pHST_Read_Cntl2Reg	   ( volatile UINT32* )0x002D0014
#define pHST_Write_Cntl1Reg	( volatile UINT32* )0x002D0018
#define pHST_Write_Cntl2Reg	( volatile UINT32* )0x002D001C
#define pMSTR_Intf_WidthReg	( volatile UINT32* )0x002D0020
#define pMSTR_HandshakeReg	   ( volatile UINT32* )0x002D0024
#define pHDMA_Src_AddrReg	   ( volatile UINT32* )0x002D0028
#define pHDMA_Dst_AddrReg	   ( volatile UINT32* )0x002D002C
#define pHDMA_BcntReg		   ( volatile UINT32* )0x002D0030
#define pHDMA_TimersReg		   ( volatile UINT32* )0x002D0034

#define MODE_RW_DS		1	   /* R/W and a DS signal */
#define MODE_R_W		   0	   /* Seperate Read and Write signals */

/* How far to shift up the wait states for the wait state control registers */
#define FLASH_WS			0
#define HCS1_WS			5
#define FALCON_WS			10
#define V90_WS				15
#define UART_WS			20

/* How far to shift up the chip select hold times */
#define HCS1_TCH			0
#define FALCON_TCH		4
#define V90_TCH			8
#define UART_TCH			12

/* How far to shift up the chip select setup times */
#define HCS1_TCS			16
#define FALCON_TCS		20
#define V90_TCS			24
#define UART_TCS			28

/* How far to shift up the address hold times */
#define HCS1_TAH			0
#define FALCON_TAH		4
#define V90_TAH			8
#define UART_TAH			12

/* How far to shift up the address setup times */
#define HCS1_TAS			16
#define FALCON_TAS		20
#define V90_TAS			24
#define UART_TAS			28


#define HST_ASB_ADDR_START	( UINT32* )0x00200000
#define HST_ASB_ADDR_FINISH	( UINT32* )0x00200FFF

/* Host Interface UART Section */
/*taken straight from datasheet*/

#define pUreg_RHR	( volatile UINT8* )0x002C0000	/*Receive Holding Register*/
#define pUreg_THR	( volatile UINT8* )0x002C0000 	/*Receive Holding Register*/
#define pUreg_IER	( volatile UINT8* )0x002C0002	/*Interrupt Enable Register*/
#define pUreg_FCR	( volatile UINT8* )0x002C0004	/*FIFO control Register*/
#define pUreg_ISR	( volatile UINT8* )0x002C0004	/*Interrupt Status Register*/
#define pUreg_LCR	( volatile UINT8* )0x002C0006	/*Line control Register*/
#define pUreg_MCR   ( volatile UINT8* )0x002C0008	/*Modem Control Register*/
#define pUreg_LSR	( volatile UINT8* )0x002C000A	/*Line Status Register*/
#define pUreg_MSR	( volatile UINT8* )0x002C000C   /*Modem Status Register*/
#define pUreg_SCR	( volatile UINT8* )0x002C000E	/*Scratch pad Register*/

/* Two offsets used for defining the baud rate*/
#define DIVLSB		( volatile UINT8* )0x002C0000	/*Divisor LSB latch address */
#define DIVMSB		( volatile UINT8* )0x002C0002	/*Divisor MSB latch address */

/* Program table for baud rate */


/*Baud Rates */
#define	_COM_300_			0
#define _COM_1200_			1
#define	_COM_2400_			2
#define _COM_9600_			3
#define	_COM_19K_			4
#define _COM_38K_			   5
#define	_COM_56K_			6
#define _COM_115K_			7

/*Parity*/
#define	_COM_NOPARITY_		0
#define _COM_ODDPARITY_		1
#define _COM_EVENPARITY_	2

/*Stopbits*/
#define	_COM_STOP1_			0
#define _COM_STOP2_			1
#define _COM_STOP1_5_		1

/*word length*/
#define	_COM_CHR5_			0
#define _COM_CHR6_			1
#define _COM_CHR7_			2
#define _COM_CHR8_			3

/*fifo length*/
#define _COM_FIFO1_			0
#define _COM_FIFO4_			1
#define _COM_FIFO8_			2
#define _COM_FIFO14_		   3


/* Host Interface Flash Section */
/*taken straight from datasheet*/
#define pFlashBaseAddr		0x400000	 

#ifndef _ASMLANGUAGE
void HSTInit(void);
#endif
  
/****************************************************************************
 *
 *  $Log: hstif.h,v $
 *  Revision 1.1  2003/06/29 14:28:18  gerg
 *  Import of the Conexant 82100 code (from Conexant).
 *  Compiles, needs to be tested on real hardware.
 *
 * 
 * 2     2/28/02 8:26a Richarjc
 * Added Conexant ownership and GPL notices.
 * 
 * 1     2/22/02 11:47a Palazzjd
 * Lineo Beta
 *  Revision 1.1  2001/12/12 14:26:32  oleks
 *  OZH: Added Conexant "Golden Gate" support.
 *
 * 
 * 1     6/13/01 8:56a Richarjc
 * Initial checkin of the Linux kernel and BSP source code for the Golden
 * Gate development board based upon the CX82100 Home Networking
 * Processor.
 * 
 *    Rev 1.4   15 Nov 2000 18:56:54   dinhtm
 *  
 * 
 *    Rev 1.2   28 Mar 2000 15:33:16   sneedgc
 * Chris S.: Document cache code, enable
 * async mode, shink IRQ stack to 0x800/2,
 * and add code to change to run map.
 * 
 *    Rev 1.1   Jan 20 2000 16:32:56   nguyenpa
 * Add CX82100 support and clean up.
 * 
 *    Rev 1.0   31 Oct 1999 16:03:18   nguyenpa
 *  
 * 
 *    Rev 1.3   27 Oct 1999 14:33:04   blackldk
 * Added symbolic definition for interface timing bits
 * 
 *    Rev 1.2   01 Oct 1999 08:06:50   chen2p
 *  
 * 
 *    Rev 1.1   28 Sep 1999 08:16:16   chen2p
 *  
 * 
 *    Rev 1.0   22 Sep 1999 16:59:04   chen2p
 *  First merge of Host IF, GPIO,Memory, and
 *  Timer tests.
 * 
 *    Rev 1.0   17 Aug 1999 15:00:38   blackldk
 * DMA control routines
 * 
 *
 ***************************************************************************/

