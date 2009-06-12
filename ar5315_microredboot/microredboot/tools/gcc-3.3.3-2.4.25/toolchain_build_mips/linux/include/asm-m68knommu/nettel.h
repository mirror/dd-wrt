/****************************************************************************/

/*
 *	nettel.h -- SnapGear (formerly Moreton Bay) NETtel support.
 *
 *	(C) Copyright 1999-2000, Moreton Bay (www.moretonbay.com)
 * 	(C) Copyright 2000-2001, Lineo Inc. (www.lineo.com) 
 * 	(C) Copyright 2001-2003, SnapGear Inc., (www.snapgear.com) 
 */

/****************************************************************************/
#ifndef	nettel_h
#define	nettel_h
/****************************************************************************/

#include <linux/config.h>

/****************************************************************************/
#ifdef CONFIG_NETtel
/****************************************************************************/

#ifdef CONFIG_COLDFIRE
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#endif

/*---------------------------------------------------------------------------*/
#if defined(CONFIG_M5307)
/*
 *	NETtel/5307 based hardware first. DTR/DCD lines are wired to
 *	GPIO lines. Most of the LED's are driven through a latch
 *	connected to CS2.
 */
#define	MCF_HAVEDCD0
#define	MCF_HAVEDCD1
#define	MCF_HAVEDTR0
#define	MCF_HAVEDTR1

#define	NETtel_LEDADDR	0x30400000

#ifndef __ASSEMBLY__
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

/*---------------------------------------------------------------------------*/
#elif defined(CONFIG_M5206e)
/*
 *	NETtel/5206e based hardware has leds on latch on CS3.
 *	No support modem for lines??
 */
#define	NETtel_LEDADDR	0x50000000

/*---------------------------------------------------------------------------*/
#elif defined(CONFIG_M5272)
/*
 *	NETtel/5272 based hardware. DTR/DCD lines are wired to GPB lines.
 *	No DCD/DTR on port 1, only on port 0.
 */
#define	MCF_HAVEDCD0
#define	MCF_HAVEDTR0

#ifndef __ASSEMBLY__
static __inline__ unsigned int mcf_getppdcd(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr == 0) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PBDAT);
		return((*pp & 0x0080) ? 0 : 1);
	}
	return(0);
}

static __inline__ unsigned int mcf_getppdtr(unsigned int portnr)
{
	volatile unsigned short *pp;
	if (portnr == 0) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PBDAT);
		return((*pp & 0x0040) ? 0 : 1);
	}
	return(0);
}

static __inline__ void mcf_setppdtr(unsigned int portnr, unsigned int dtr)
{
	volatile unsigned short *pp;
	if (portnr == 0) {
		pp = (volatile unsigned short *) (MCF_MBAR + MCFSIM_PBDAT);
		*pp = (*pp & ~0x0040) | (dtr ? 0 : 0x0040);
	}
}
#endif /* __ASSEMBLY */

#endif
/*---------------------------------------------------------------------------*/

/****************************************************************************/
#endif /* CONFIG_NETtel */
/****************************************************************************/
#endif	/* nettel_h */
