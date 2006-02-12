/*
 * Common [OS-independent] header file for
 * Broadcom BCM44XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright 2005, Broadcom Corporation   
 * All Rights Reserved.                   
 *                                        
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;      
 * the contents of this file may not be disclosed to third parties, copied   
 * or duplicated in any form, in whole or in part, without the prior         
 * written permission of Broadcom Corporation.                               
 *
 * $Id$
 */

#ifndef _etc_h_
#define _etc_h_

#define	MAXMULTILIST	32

#ifndef ch_t
#define	ch_t void
#endif

struct etc_info;	/* forward declaration */

/* each chip type (4413, 47xx) supports a set of chip-type-specific ops */
struct chops {
	bool (*id)(uint vendor, uint device);		/* return true if match */
	void *(*attach)(struct etc_info *etc, void *dev, void *regs);
	void (*detach)(ch_t *ch);			/* free chip private state */
	void (*reset)(ch_t *ch);			/* chip reset */
	void (*init)(ch_t *ch, bool full);	/* chip init */
	void (*tx)(ch_t *ch, void *p);			/* transmit frame */
	void *(*rx)(ch_t *ch);				/* receive frame */
	void (*rxfill)(ch_t *ch);			/* post dma rx buffers */
	int (*getintrevents)(ch_t *ch);			/* return intr events */
	bool (*errors)(ch_t *ch);			/* handle chip errors */
	void (*intrson)(ch_t *ch);			/* enable chip interrupts */
	void (*intrsoff)(ch_t *ch);			/* disable chip interrupts */
	void (*txreclaim)(ch_t *ch, bool all);		/* reclaim transmit resources */
	void (*rxreclaim)(ch_t *ch);			/* reclaim receive resources */
	void (*statsupd)(ch_t *ch);			/* update sw stat counters */
	void (*enablepme)(ch_t *ch);			/* enable PME */
	void (*disablepme)(ch_t *ch);			/* disable PME */
	void (*phyreset)(ch_t *ch, uint phyaddr);	/* reset phy */
	uint16 (*phyrd)(ch_t *ch, uint phyaddr, uint reg);	/* read phy register */
	void (*phywr)(ch_t *ch, uint phyaddr, uint reg, uint16 val);	/* write phy register */
	void (*dump)(ch_t *ch, char *buf);		/* debugging output */
	void (*longname)(ch_t *ch, char *buf, uint bufsize);	/* return descriptive name */
	void (*duplexupd)(ch_t *ch);			/* keep mac duplex consistent */
};

/*
 * "Common" os-independent software state structure.
 */
typedef struct etc_info {
	void		*et;		/* pointer to os-specific private state */
	uint		unit;		/* device instance number */
	void 		*osh; 		/* pointer to os handler */
	bool		up;		/* interface up and running */
	bool		promisc;	/* promiscuous destination address */
	bool		qos;		/* QoS priority determination on rx */
	bool		loopbk;		/* loopback override mode */

	int		forcespeed;	/* disable autonegotiation and force speed/duplex */
	uint		advertise;	/* control speed/duplex advertised capability bits */
	bool		needautoneg;	/* request restart autonegotiation */
	int		speed;		/* current speed: 10, 100 */
	int		duplex;		/* current duplex: 0=half, 1=full */

	bool		piomode;	/* enable programmed io (!dma) */
	void		*pioactive;	/* points to pio packet being transmitted */
	volatile uint	*txavail;	/* dma: # tx descriptors available */

	uint16		vendorid;	/* pci function vendor id */
	uint16		deviceid;	/* pci function device id */
	uint		chip;		/* chip number */
	uint		chiprev;	/* chip revision */

	bool		nicmode;	/* is this core using its own pci i/f */

	struct chops	*chops;		/* pointer to chip-specific opsvec */
	void		*ch;		/* pointer to chip-specific state */

	uint		coreunit;	/* sb chips: chip enet instance # */
	uint		phyaddr;	/* sb chips: mdio 5-bit phy address */
	uint		mdcport;	/* sb chips: which mii to use (enet core #) to access our phy */

	struct ether_addr cur_etheraddr; /* our local ethernet address */
	struct ether_addr perm_etheraddr; /* original sprom local ethernet address */

	struct ether_addr multicast[MAXMULTILIST];
	uint		nmulticast;
	bool		allmulti;	/* enable all multicasts */

	bool		linkstate;	/* link integrity state */
	bool		pm_modechange;	/* true if mode change is to due pm */

	uint32		now;		/* elapsed seconds */

	uint32		boardflags;	/* board flags */

	/* chip-maintained plus a few sw-maintained stat counters */
	bcmenetmib_t	mib;		/* mib statistic counters */
	uint32		txframe;	/* transmitted frames */
	uint32		txbyte;		/* transmitted bytes */
	uint32		rxframe;	/* received frames */
	uint32		rxbyte;		/* received bytes */
	uint32		txerror;	/* total tx errors */
	uint32		txnobuf;	/* tx out-of-buffer errors */
	uint32		rxerror;	/* total rx errors */
	uint32		rxnobuf;	/* rx out-of-buffer errors */
	uint32		reset;		/* reset count */
	uint32		dmade;		/* pci descriptor errors */
	uint32		dmada;		/* pci data errors */
	uint32		dmape;		/* descriptor protocol error */
	uint32		rxdmauflo;	/* receive descriptor underflow */
	uint32		rxoflo;		/* receive fifo overflow */
	uint32		txuflo;		/* transmit fifo underflow */
	uint32		rxbadlen;	/* 802.3 len field != read length */
} etc_info_t;

/* interrupt event bitvec */
#define	INTR_TX		0x1
#define	INTR_RX		0x2
#define	INTR_ERROR	0x4
#define	INTR_TO		0x8
#define	INTR_NEW	0x10

/* forcespeed values */
#define	ET_AUTO		-1
#define	ET_10HALF	0
#define	ET_10FULL	1
#define	ET_100HALF	2
#define	ET_100FULL	3

/* common ioctl definitions */
#define	ETCUP		0
#define	ETCDOWN		1
#define ETCLOOP		2
#define ETCDUMP		3
#define ETCSETMSGLEVEL	4
#define	ETCPROMISC	5
#define	ETCRESV1	6
#define	ETCSPEED	7
#define ETCPHYRD	9
#define ETCPHYWR	10
#define	ETCQOS		11

/*
 * Least-common denominator rxbuf start-of-data offset:
 * Must be >= size of largest rxhdr
 * Must be 2-mod-4 aligned so IP is 0-mod-4
 */
#define	HWRXOFF		30

/* exported prototypes */
extern struct chops *etc_chipmatch(uint vendor, uint device);
extern void *etc_attach(void *et, uint vendor, uint device, uint unit, void *dev, void *regsva);
extern void etc_detach(etc_info_t *etc);
extern void etc_reset(etc_info_t *etc);
extern void etc_init(etc_info_t *etc);
extern void etc_up(etc_info_t *etc);
extern uint etc_down(etc_info_t *etc, int reset);
extern int etc_ioctl(etc_info_t *etc, int cmd, void *arg);
extern void etc_promisc(etc_info_t *etc, uint on);
extern void etc_setprio (void *ethpkt, bool update_vtag);
extern void etc_qos(etc_info_t *etc, uint on);
extern void etc_dump(etc_info_t *etc, uchar *buf, int size);
extern void etc_watchdog(etc_info_t *etc);
extern uint etc_totlen(etc_info_t *etc, void *p);

#endif	/* _etc_h_ */
