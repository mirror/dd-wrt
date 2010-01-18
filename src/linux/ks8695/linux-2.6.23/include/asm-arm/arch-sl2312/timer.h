/*
 *
 *  This file contains the register definitions for the Excalibur
 *  Timer TIMER00.
 *
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
#ifndef __TIMER_H
#define __TIMER_H

/*
 * Register definitions for the timers
 */

#define TIMER_COUNT(BASE_ADDR)          (TIMER_TYPE (BASE_ADDR  + 0x00 ))
#define TIMER_LOAD(BASE_ADDR)           (TIMER_TYPE (BASE_ADDR  + 0x04 ))
#define TIMER_MATCH1(BASE_ADDR)         (TIMER_TYPE (BASE_ADDR  + 0x08 ))
#define TIMER_MATCH2(BASE_ADDR)         (TIMER_TYPE (BASE_ADDR  + 0x0C ))
#define TIMER_CR(BASE_ADDR)             (TIMER_TYPE (BASE_ADDR  + 0x30 ))
#define TIMER_1_CR_ENABLE_MSK 		    (0x00000001)
#define TIMER_1_CR_ENABLE_OFST 		    (0)
#define TIMER_1_CR_CLOCK_MSK  		    (0x00000002)
#define TIMER_1_CR_CLOCK_OFST 		    (1)
#define TIMER_1_CR_INT_MSK 		        (0x00000004)
#define TIMER_1_CR_INT_OFST 		    (2)
#define TIMER_2_CR_ENABLE_MSK 		    (0x00000008)
#define TIMER_2_CR_ENABLE_OFST 		    (3)
#define TIMER_2_CR_CLOCK_MSK  		    (0x00000010)
#define TIMER_2_CR_CLOCK_OFST 		    (4)
#define TIMER_2_CR_INT_MSK 		        (0x00000020)
#define TIMER_2_CR_INT_OFST 		    (5)
#define TIMER_3_CR_ENABLE_MSK 		    (0x00000040)
#define TIMER_3_CR_ENABLE_OFST 		    (6)
#define TIMER_3_CR_CLOCK_MSK  		    (0x00000080)
#define TIMER_3_CR_CLOCK_OFST 		    (7)
#define TIMER_3_CR_INT_MSK 		        (0x00000100)
#define TIMER_3_CR_INT_OFST 		    (8)

#endif /* __TIMER00_H */
