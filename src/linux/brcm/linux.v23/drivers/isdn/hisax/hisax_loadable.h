/* $Id: hisax_loadable.h,v 2.1 2001/06/08 22:19:16 werner Exp $
 *
 *
 * Author       (C) 2001 Werner Cornelius (werner@isdn-development.de)
 *              modular driver for Colognechip HFC-USB chip
 *              as plugin for HiSax isdn driver
 *
 * Copyright 2001  by Werner Cornelius (werner@isdn4linux.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/types.h>
#include <linux/skbuff.h>

/***************************************/
/* additional defines for l1 constants */
/***************************************/
#define B1_DATA       0x1f0
#define B1_SETMODE    0x1f4
#define B2_DATA       0x1f8
#define B2_SETMODE    0x1fc


/********************************************************/
/* structure used for register and release of L1 driver */
/********************************************************/
struct hisax_drvreg {
	int version;		/* actual version for check */
	int cmd;		/* command code */

	/* function pointers set by hisax during register call */
	void (*release_driver) (void *arg_hisax);	/* set by hisax, release function for driver */
	void (*dch_l1l2) (void *arg_hisax, int pr, void *arg);	/* set by hisax, notify dch+l1 events */
	void (*bch_l1l2) (void *arg_hisax, int chan, int pr, void *arg);	/* set by hisax, notify bch events */
	void *arg_hisax;	/* argument when calling hisax main */
	struct sk_buff_head *drq;	/* pointer to D-receive queue */
	struct sk_buff_head *dsq;	/* pointer to D-send queue */
	struct sk_buff_head erq;	/* E-receive queue */
	struct sk_buff_head *brq[2];	/* pointer to B-receive queues */
	struct sk_buff **bsk[2];	/* pointer to B-transmit buffer */

	/* function pointers set by l1 driver before calling the register function */
	void (*dch_l2l1) (void *argl1, int pr, void *arg);	/* function dch+l1 from hisax -> l1 */
	void (*bch_l2l1) (void *argl1, int chan, int pr, void *arg);	/* function bch from hisax -> l1 */
	void *argl1;		/* pointer to l1 data structure when calling l1 */

	char *drvname;		/* driver name for hisax usage */
};

/**************************/
/* constants and commands */
/**************************/
#define HISAX_LOAD_VERSION  4	/* change when interface changes */
#define HISAX_LOAD_CHKVER   0	/* check version command (returns 0 on success) */
#define HISAX_LOAD_REGISTER 1	/* register the L1 driver and return 0 on success */

/***************************************/
/* definition of the register function */
/***************************************/
extern int hisax_register_hfcusb(struct hisax_drvreg *l1drv);
