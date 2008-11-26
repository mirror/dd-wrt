/*
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

#include <linux/autoconf.h>
#include "../arch/arm/mach-feroceon-orion/config/mvSysHwConfig.h"

#define MV_UART_LSR 	(*(volatile unsigned char *)(INTER_REGS_BASE + 0x12000 + 0x14))
#define MV_UART_THR	(*(volatile unsigned char *)(INTER_REGS_BASE + 0x12000 + 0x0 ))	 

#define LSR_THRE	0x20

/*
 * This does not append a newline
 */
static void putstr(const char *s)
{
        while (*s) {
		while ((MV_UART_LSR & LSR_THRE) == 0);
		MV_UART_THR = *s;
		
                if (*s == '\n') {
                        while ((MV_UART_LSR & LSR_THRE) == 0); 
                        MV_UART_THR = '\r';
                }
                s++;
        }
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()
