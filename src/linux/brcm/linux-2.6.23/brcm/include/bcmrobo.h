/*
 * RoboSwitch setup functions
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmrobo.h,v 13.7.54.1 2008/11/11 04:13:29 Exp $
 */

#ifndef _bcm_robo_h_
#define _bcm_robo_h_

#define DEVID5325   0x25    /* 5325 (Not really be we fake it) */
#define	DEVID5395	0x95	/* 5395 */
#define	DEVID5397	0x97	/* 5397 */
#define	DEVID5398	0x98	/* 5398 */
#define	DEVID53115	0x3115	/* 53115 */

/* Forward declaration */
typedef struct robo_info_s robo_info_t;

/* Device access/config oprands */
typedef struct {
	/* low level routines */
	void (*enable_mgmtif)(robo_info_t *robo);	/* enable mgmt i/f, optional */
	void (*disable_mgmtif)(robo_info_t *robo);	/* disable mgmt i/f, optional */
	int (*write_reg)(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len);
	int (*read_reg)(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len);
	/* description */
	char *desc;
} dev_ops_t;


typedef	uint16 (*miird_f)(void *h, int add, int off);
typedef	void (*miiwr_f)(void *h, int add, int off, uint16 val);

/* Private state per RoboSwitch */
struct robo_info_s {
	si_t	*sih;			/* SiliconBackplane handle */
	char	*vars;			/* nvram variables handle */
	void	*h;			/* dev handle */
	uint16	devid;			/* Device id for the switch */
	uint32	corerev;		/* Core rev of internal switch */

	dev_ops_t *ops;			/* device ops */
	uint8	page;			/* current page */

	/* SPI */
	uint32	ss, sck, mosi, miso;	/* GPIO mapping */

	/* MII */
	miird_f	miird;
	miiwr_f	miiwr;
};

extern robo_info_t *bcm_robo_attach(si_t *sih, void *h, char *vars, miird_f miird, miiwr_f miiwr);
extern void bcm_robo_detach(robo_info_t *robo);
extern int bcm_robo_enable_device(robo_info_t *robo);
extern int bcm_robo_config_vlan(robo_info_t *robo, uint8 *mac_addr);
extern int bcm_robo_enable_switch(robo_info_t *robo);


#endif /* _bcm_robo_h_ */
