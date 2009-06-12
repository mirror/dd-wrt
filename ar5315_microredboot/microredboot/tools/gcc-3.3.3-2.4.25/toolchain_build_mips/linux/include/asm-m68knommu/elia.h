/****************************************************************************/

/*
 *	elia.h -- Lineo (formerly Moreton Bay) eLIA platform support.
 *
 *	(C) Copyright 1999-2000, Moreton Bay (www.moreton.com.au)
 *	(C) Copyright 1999-2000, Lineo (www.lineo.com)
 */

/****************************************************************************/
#ifndef	elia_h
#define	elia_h
/****************************************************************************/

#include <linux/config.h>
#include <asm/coldfire.h>

#ifdef CONFIG_eLIA

/*
 *	The serial port DTR and DCD lines are also on the Parallel I/O
 *	as well, so define those too.
 */
#define	MCF_HAVEDCD0
#define	MCF_HAVEDCD1
#define	MCF_HAVEDTR0
#define	MCF_HAVEDTR1

#define	eLIA_PCIRESET		0x0020

/*
 *	Kernel functions to set and unset DTR/DCD and the LEDs.
 */
#ifndef __ASSEMBLY__

#define	MCFPP_DCD1	0x0001
#define	MCFPP_DCD0	0x0002
#define	MCFPP_DTR1	0x0004
#define	MCFPP_DTR0	0x0008

extern volatile unsigned short ppdata;

static __inline__ unsigned int mcf_getppdcd(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr < 2) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		return((*pp & (portnr ? 0x0001 : 0x0002)) ? 0 : 1);
	}
	return(0);
}

static __inline__ unsigned int mcf_getppdtr(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr < 2) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		return((*pp & (portnr ? 0x0004 : 0x0008)) ? 0 : 1);
	}
	return(0);
}

static __inline__ void mcf_setppdtr(unsigned int portnr, unsigned int dtr)
{
	volatile unsigned short *pp;
	unsigned short bit;
	if (portnr < 2) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		bit = (portnr ? 0x0004 : 0x0008);
		*pp = (*pp & ~bit) | (dtr ? 0 : bit);
	}
}

/*
 *	The power and heartbeat LED's are connected to PIO bits of
 *	the 5307 based boards.
 */
static __inline__ void mcf_setppleds(unsigned int mask, unsigned int bits)
{
	volatile unsigned short *pp;
	pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
	*pp = (*pp & ~mask) | bits;
}
#endif /* __ASSEMBLY__ */
#endif	/* CONFIG_eLIA */

/****************************************************************************/
#endif	/* elia_h */
