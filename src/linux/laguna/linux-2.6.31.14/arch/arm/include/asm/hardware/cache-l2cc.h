/*******************************************************************************
 *
 *  arch/arm/include/asm/hardware/cache-l2cc.h
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#ifndef __ASM_ARM_HARDWARE_L2_H
#define __ASM_ARM_HARDWARE_L2_H

#define L2CC_CACHE_ID			0x000
#define L2CC_CACHE_TYPE			0x004
#define L2CC_CTRL			0x100
#define L2CC_AUX_CTRL			0x104
#define L2CC_TAG_RAM_LATENCY_CTRL	0x108
#define L2CC_DATA_RAM_LATENCY_CTRL	0x10C
#define L2CC_EVENT_CNT_CTRL		0x200
#define L2CC_EVENT_CNT1_CFG		0x204
#define L2CC_EVENT_CNT0_CFG		0x208
#define L2CC_EVENT_CNT1_VAL		0x20C
#define L2CC_EVENT_CNT0_VAL		0x210
#define L2CC_INTR_MASK			0x214
#define L2CC_MASKED_INTR_STAT		0x218
#define L2CC_RAW_INTR_STAT		0x21C
#define L2CC_INTR_CLEAR			0x220
#define L2CC_CACHE_SYNC			0x730
#define L2CC_INV_LINE_PA		0x770
#define L2CC_INV_WAY			0x77C
#define L2CC_CLEAN_LINE_PA		0x7B0
#define L2CC_CLEAN_LINE_IDX		0x7B8
#define L2CC_CLEAN_WAY			0x7BC
#define L2CC_CLEAN_INV_LINE_PA		0x7F0
#define L2CC_CLEAN_INV_LINE_IDX		0x7F8
#define L2CC_CLEAN_INV_WAY		0x7FC
#define L2CC_LOCKDOWN_0_WAY_D		0x900
#define L2CC_LOCKDOWN_0_WAY_I		0x904
#define L2CC_LOCKDOWN_1_WAY_D		0x908
#define L2CC_LOCKDOWN_1_WAY_I		0x90C
#define L2CC_LOCKDOWN_2_WAY_D		0x910
#define L2CC_LOCKDOWN_2_WAY_I		0x914
#define L2CC_LOCKDOWN_3_WAY_D		0x918
#define L2CC_LOCKDOWN_3_WAY_I		0x91C
#define L2CC_LOCKDOWN_4_WAY_D		0x920
#define L2CC_LOCKDOWN_4_WAY_I		0x924
#define L2CC_LOCKDOWN_5_WAY_D		0x928
#define L2CC_LOCKDOWN_5_WAY_I		0x92C
#define L2CC_LOCKDOWN_6_WAY_D		0x930
#define L2CC_LOCKDOWN_6_WAY_I		0x934
#define L2CC_LOCKDOWN_7_WAY_D		0x938
#define L2CC_LOCKDOWN_7_WAY_I		0x93C
#define L2CC_LOCKDOWN_LINE_EN		0x950
#define L2CC_UNLOCK_ALL_LINE_WAY	0x954
#define L2CC_ADDR_FILTER_START		0xC00
#define L2CC_ADDR_FILTER_END		0xC04
#define L2CC_DEBUG_CTRL			0xF40

#ifndef __ASSEMBLY__
extern void __init l2cc_init(void __iomem *base);
#endif

#endif
