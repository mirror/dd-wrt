/* $Id: isdn_dwabc.h,v 1.9 2001/09/26 20:32:08 detabc Exp $
 *
 * Header for the Linux ISDN abc-extension.
 *
 * Copyright           by abc GmbH
 *                     written by Detlef Wengorz <detlefw@isdn4linux.de>
 * 
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#ifndef ISDN_DWABC_H
#define ISDN_DWABC_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/errno.h>


typedef struct ISDN_DWSPINLOCK {

	spinlock_t 	spin;
	short  		owner;
	short		my_flags;
	ulong 		irq_flags;

} ISDN_DWSPINLOCK;

#define ISDN_DWSPIN_UNLOCKED				\
	(ISDN_DWSPINLOCK) {						\
		spin: 		SPIN_LOCK_UNLOCKED,		\
		owner:		-1,						\
		my_flags:	0,						\
		irq_flags:	0,						\
	}

#define ISDN_DWSPIN_INIT(x)			\
			do { *(x) = ISDN_DWSPIN_UNLOCKED; } while(0);

static __inline__ int isdn_dwspin_trylock(ISDN_DWSPINLOCK *spin)
{
	if(!spin_trylock(&spin->spin)) {

		if(spin->owner == smp_processor_id())
			return(-EAGAIN);

		spin_lock(&spin->spin);
	}

	spin->owner = smp_processor_id();
	return(0);
}

static __inline__ void isdn_dwspin_unlock(ISDN_DWSPINLOCK *spin)
{
	spin->owner = -1;
	spin_unlock(&spin->spin);
}


#else
#include <sys/types.h>
#endif

#define DWABC_LCR_FLG_NEWNUMBER		0x00000001L
#define DWABC_LCR_FLG_DISABLE		0x00000002L
#define DWABC_LCR_FLG_NEWHUPTIME	0x00000004L


struct ISDN_DWABC_LCR_IOCTL {

	int 	lcr_ioctl_sizeof;	/* mustbe sizeof(ISDN_DWABC_LCR_IOCTL)	*/
	u_short lcr_ioctl_onhtime;	/* new hanguptime						*/
	u_long 	lcr_ioctl_callid;	/* callid from lcr-subsystem			*/
	u_long 	lcr_ioctl_flags;	/* see above							*/
	char 	lcr_ioctl_nr[32];	/* new destination phonenumber			*/
};

#endif
