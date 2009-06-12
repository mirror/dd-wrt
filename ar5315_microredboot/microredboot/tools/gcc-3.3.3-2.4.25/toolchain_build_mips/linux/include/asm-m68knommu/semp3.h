/****************************************************************************/

/*
 *	semp.h -- SecureEdge MP3 hardware platform support.
 *
 *	(C) Copyright 2001-2003, Greg Ungerer (gerg@snapgear.com).
 */

/****************************************************************************/
#ifndef	semp3_h
#define	semp3_h
/****************************************************************************/

#include <linux/config.h>

/****************************************************************************/
#ifdef CONFIG_SECUREEDGEMP3
/****************************************************************************/

#include <asm/coldfire.h>
#include <asm/mcfsim.h>

/*
 *	The ColdFire UARTs do not have any support for DTR/DCD lines.
 *	We have wired them onto some of the parallel IO lines.
 */
#define	MCF_HAVEDCD1
#define	MCF_HAVEDTR1

#ifndef __ASSEMBLY__
static __inline__ unsigned int mcf_getppdcd(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr == 1) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		return((*pp & 0x0004) ? 0 : 1);
	}
	return(0);
}

static __inline__ unsigned int mcf_getppdtr(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr == 1) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		return((*pp & 0x0080) ? 0 : 1);
	}
	return(0);
}

static __inline__ void mcf_setppdtr(unsigned int portnr, unsigned int dtr)
{
	volatile unsigned short *pp;
	if (portnr == 1) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT);
		*pp = (*pp & ~0x0080) | (dtr ? 0 : 0x0080);
	}
}
#endif /* __ASSEMBLY__ */

/****************************************************************************/
#endif /* CONFIG_SECUREEDGEMP3 */
/****************************************************************************/
#endif	/* semp3_h */
