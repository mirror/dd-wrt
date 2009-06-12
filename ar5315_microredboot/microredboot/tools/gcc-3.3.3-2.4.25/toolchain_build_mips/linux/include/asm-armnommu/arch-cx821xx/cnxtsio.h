/****************************************************************************
*
*	Name:			cnxtsio.h
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
*  $Modtime: 9/13/02 3:17p $
****************************************************************************/
#ifndef CNXT_SIO_H
#define CNXT_SIO_H

#include "bspcfg.h"

#if defined (CONFIG_BD_HASBANI)
	#define SERIALPORT1 0x002C0010
#elif defined (CONFIG_BD_MACKINAC)
	#define SERIALPORT1 0x00290000
#elif defined (CONFIG_BD_RUSHMORE)
	#define SERIALPORT1 0x002C0010
#else
	#define 0x00000000
#endif


#define SIZE_OF_UART_FIFO	512

typedef struct
{

	UINT16	get_idx;
	UINT16	put_idx;
	UINT16	entry_cnt;
	UINT16	fill;		// Dword align the structure
	BYTE	fifo[ SIZE_OF_UART_FIFO ];
} UART_FIFO_T;





typedef struct CnxtUart
{
	UINT32		  regAddr;
	UINT8		* regs;
	UART_FIFO_T	  xmtr_uart_fifo;
	UART_FIFO_T	  rcvr_uart_fifo;
} CNXT_UART;

eUARTType spUartDevInit( CNXT_UART * pUARTChan);

#endif

