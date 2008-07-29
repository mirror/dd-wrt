/*
 *  linux/include/asm-arm/arch-epxa10db/uncompress.h
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "asm/arch/platform.h"
#include "asm/arch/hardware.h"
#define UART_TYPE (volatile unsigned int*)
#ifndef CONFIG_SERIAL_IT8712
#include "asm/arch/uart.h"
#endif
extern unsigned int it8712_uart_base;

/*
 * This does not append a newline
 */

static inline void putc(int c)
{

#ifdef CONFIG_SERIAL_IT8712

	unsigned char *base,*status,stat;
	int i ;

	status = (unsigned char*)it8712_uart_base + 5;
	base = (unsigned char*)it8712_uart_base ;


		stat = *status;
		while (!(stat&0x20)) {				// check status
			for(i=0;i<0x10;i++)	;
			status = (unsigned char*)it8712_uart_base + 5;
			stat = *status ;
		}

		*base = c;
		barrier();


#else
		while (!(*UART_LSR(SL2312_UART_BASE) &
		         UART_LSR_THRE));
		       barrier();

		*UART_THR(SL2312_UART_BASE) = c;

#endif
}

/*
 * nothing to do
 */
#define arch_decomp_setup()

#define arch_decomp_wdog()
