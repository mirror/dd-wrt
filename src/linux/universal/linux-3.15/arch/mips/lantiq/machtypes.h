/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LANTIQ_MACH_H__
#define _LANTIQ_MACH_H__

#include <asm/mips_machine.h>

enum lantiq_mach_type {
	LTQ_MACH_GENERIC = 0,
	LTQ_MACH_EASY50712,	/* Danube evaluation board */
	LTQ_MACH_EASY50601,	/* Amazon SE evaluation board */

	/* FALCON */
	LANTIQ_MACH_EASY98000,		/* Falcon Eval Board, NOR Flash */
	LANTIQ_MACH_EASY98000SF,	/* Falcon Eval Board, Serial Flash */
	LANTIQ_MACH_EASY98000NAND,	/* Falcon Eval Board, NAND Flash */
	LANTIQ_MACH_EASY98020,		/* EASY98020 Eval Board */
	LANTIQ_MACH_EASY98020_1LAN,	/* EASY98020 Eval Board (1 LAN port) */
	LANTIQ_MACH_EASY98020_2LAN,	/* EASY98020 Eval Board (2 LAN port) */
	LANTIQ_MACH_95C3AM1,		/* 95C3AM1 Eval Board */

	/* FRITZ!BOX */
	LANTIQ_MACH_FRITZ3370,		/* FRITZ!BOX 3370 vdsl cpe */

	/* Arcadyan */
	LANTIQ_MACH_ARV3527P,		/* Arcor easybox a401 */
	LANTIQ_MACH_ARV4510PW,		/* Wippies Homebox */
	LANTIQ_MACH_ARV4518PW,		/* Airties WAV-221, SMC-7908A-ISP */
	LANTIQ_MACH_ARV4520PW,		/* Airties WAV-281, Arcor EasyboxA800 */
	LANTIQ_MACH_ARV452CPW,		/* Arcor EasyboxA801 */
	LANTIQ_MACH_ARV4525PW,		/* Speedport W502V */
	LANTIQ_MACH_ARV7525PW,		/* Speedport W303V */
	LANTIQ_MACH_ARV752DPW,		/* Arcor easybox a802 */
	LANTIQ_MACH_ARV752DPW22,	/* Arcor easybox a803 */
	LANTIQ_MACH_ARV7518PW,		/* ASTORIA */

	/* Netgear */
	LANTIQ_MACH_DGN3500B,		/* Netgear DGN3500 */

	/* Gigaset */
	LANTIQ_MACH_GIGASX76X,		/* Gigaset SX76x */

	/* Buffalo */
	LANTIQ_MACH_WBMR,		/* WBMR-HP-G300H */
};

#endif
