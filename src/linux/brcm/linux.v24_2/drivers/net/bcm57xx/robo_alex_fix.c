/*
 * Broadcom 538x RoboSwitch device driver.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: robo_alex_fix.c,v 1.1 2006/06/14 09:33:06 michael Exp $
 */


#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <sbconfig.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#include <bcmenetmib.h>
#include <bcmnvram.h>
#include <bcmdevs.h>

#include <mm.h>
#include <robo.h>

#define	ET_ERROR(args)
#define	ET_MSG(args)


/*
 * Switch can be programmed through SPI interface, which
 * has a rreg and a wreg functions to read from and write to
 * registers.
 */

/* Global variables */
static robo_info_t gRobo;


/* MII access registers */
#define PSEUDO_PHYAD	0x1E	/* MII Pseudo PHY address */
#define REG_MII_PAGE	0x10	/* MII Page register */
#define REG_MII_ADDR	0x11	/* MII Address register */
#define REG_MII_DATA0	0x18	/* MII Data register 0 */
#define REG_MII_DATA1	0x19	/* MII Data register 1 */

/* Page numbers */
#define PAGE_CTRL	0x00	/* Control page */
#define PAGE_MMR	0x02	/* 5397 Management/Mirroring page */
#define PAGE_VTBL	0x05	/* ARL/VLAN Table access page */
#define PAGE_VLAN	0x34	/* VLAN page */

/* Control page registers */
#define REG_PORT0_CTRL	0x00	/* Port 0 traffic control register */
#define REG_PORT1_CTRL	0x01	/* Port 1 traffic control register */
#define REG_PORT2_CTRL	0x02	/* Port 2 traffic control register */
#define REG_PORT3_CTRL	0x03	/* Port 3 traffic control register */
#define REG_PORT4_CTRL	0x04	/* Port 4 traffic control register */
#define REG_PORT5_CTRL	0x05	/* Port 5 traffic control register */
#define REG_PORT6_CTRL	0x06	/* Port 6 traffic control register */
#define REG_PORT7_CTRL	0x07	/* Port 7 traffic control register */
#define REG_CTRL_MODE	0x0B	/* Switch Mode register */
#define REG_PORT4_STATE 0x5C    /* Port 4 RGMII state override register */

/* 5397 Managment/Mirroring page registers */
#define REG_DEVICE_ID	0x30

/* VLAN page registers */
#define REG_VLAN_CTRL0	0x00	/* VLAN Control 0 register */
#define REG_VLAN_CTRL1	0x01	/* VLAN Control 1 register */
#define REG_VLAN_PTAG0	0x10	/* VLAN Default Port Tag register - port 0 */
#define REG_VLAN_PTAG1	0x12	/* VLAN Default Port Tag register - port 1 */
#define REG_VLAN_PTAG2	0x14	/* VLAN Default Port Tag register - port 2 */
#define REG_VLAN_PTAG3	0x16	/* VLAN Default Port Tag register - port 3 */
#define REG_VLAN_PTAG4	0x18	/* VLAN Default Port Tag register - port 4 */
#define REG_VLAN_PTAG5	0x1a	/* VLAN Default Port Tag register - port 5 */
#define REG_VLAN_PTAG6	0x1c	/* VLAN Default Port Tag register - port 6 */
#define REG_VLAN_PTAG7	0x1e	/* VLAN Default Port Tag register - port 7 */
#define REG_VLAN_PTAG8	0x20	/* VLAN Default Port Tag register - IMP port */

#define VLAN_NUMVLANS	16	/* # of VLANs */


/* ARL/VLAN Table Access page registers */
#define REG_VTBL_ACCESS	0x60	/* VLAN table access register */
#define REG_VTBL_INDX	0x61	/* VLAN table address index register */
#define REG_VTBL_ENTRY	0x63	/* VLAN table entry register */

/* SPI registers */
#define REG_SPI_PAGE	0xff	/* SPI Page register */

/*
* Access switch registers through GPIO/SPI
*/
/* Minimum timing constants */
#define SCK_EDGE_TIME	2	/* clock edge duration - 2us */
#define MOSI_SETUP_TIME	1	/* input setup duration - 1us */
#define SS_SETUP_TIME	1 	/* select setup duration - 1us */

/* misc. constants */
#define SPI_MAX_RETRY	100

/* Enable GPIO access to the chip */
static void
gpio_enable(robo_info_t *robo)
{
	void *regs;

	/* Save current core index */
	robo->coreidx = sb_coreidx(robo->sbh);

	/* Switch to GPIO core for faster access */
	regs = sb_gpiosetcore(robo->sbh);
	ASSERT(regs);

	/* Enable GPIO outputs with SCK and MOSI low, SS high */
	sb_gpioout(robo->sbh, robo->ss | robo->sck | robo->mosi, robo->ss, GPIO_DRV_PRIORITY);
	sb_gpioouten(robo->sbh, robo->ss | robo->sck | robo->mosi,
	             robo->ss | robo->sck | robo->mosi, GPIO_DRV_PRIORITY);
}

/* Disable GPIO access to the chip */
static void
gpio_disable(robo_info_t *robo)
{
	/* Disable GPIO outputs with all their current values */
	sb_gpioouten(robo->sbh, robo->ss | robo->sck | robo->mosi, 0, GPIO_DRV_PRIORITY);

	/* Switch back to original core */
	sb_setcoreidx(robo->sbh, robo->coreidx);
}

/* Write a byte stream to the chip thru SPI */
static int
spi_write(robo_info_t *robo, uint8 *buf, uint len)
{
	uint i;
	uint8 mask;

	/* Byte bang from LSB to MSB */
	for (i = 0; i < len; i++) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80; mask; mask >>= 1) {
			/* Clock low */
			sb_gpioout(robo->sbh, robo->sck, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on rising edge */
			if (mask & buf[i])
				sb_gpioout(robo->sbh, robo->mosi, robo->mosi, GPIO_DRV_PRIORITY);
			else
				sb_gpioout(robo->sbh, robo->mosi, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(MOSI_SETUP_TIME);

			/* Clock high */
			sb_gpioout(robo->sbh, robo->sck, robo->sck, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);
		}
	}

	return 0;
}

/* Read a byte stream from the chip thru SPI */
static int
spi_read(robo_info_t *robo, uint8 *buf, uint len)
{
	uint i, timeout;
	uint8 rack, mask, byte;

	/* Timeout after 100 tries without RACK */
	for (i = 0, rack = 0, timeout = SPI_MAX_RETRY; i < len && timeout;) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80, byte = 0; mask; mask >>= 1) {
			/* Clock low */
			sb_gpioout(robo->sbh, robo->sck, 0, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on falling edge */
			if (sb_gpioin(robo->sbh) & robo->miso)
				byte |= mask;

			/* Clock high */
			sb_gpioout(robo->sbh, robo->sck, robo->sck, GPIO_DRV_PRIORITY);
			OSL_DELAY(SCK_EDGE_TIME);
		}
		/* RACK when bit 0 is high */
		if (!rack) {
			rack = (byte & 1);
			timeout--;
			continue;
		}
		/* Byte bang from LSB to MSB */
		buf[i] = byte;
		i++;
	}

	if (timeout == 0) {
		ET_ERROR(("spi_read: timeout"));
		return -1;
	}

	return 0;
}

/* Enable/disable SPI access */
static void
spi_select(robo_info_t *robo, uint8 spi)
{
	if (spi) {
		/* Enable SPI access */
		sb_gpioout(robo->sbh, robo->ss, 0, GPIO_DRV_PRIORITY);
	} else {
		/* Disable SPI access */
		sb_gpioout(robo->sbh, robo->ss, robo->ss, GPIO_DRV_PRIORITY);
	}
	OSL_DELAY(SS_SETUP_TIME);
}


/* Select chip and page */
static void
spi_goto(robo_info_t *robo, uint8 page)
{
	uint8 reg8 = REG_SPI_PAGE;	/* page select register */
	uint8 cmd8;

	/* Issue the command only when we are on a different page */
	if (robo->page == page)
		return;
	robo->page = page;

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Select new page with CID 0 */
	cmd8 = ((6 << 4) |		/* normal SPI */
	        1);			/* write */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &reg8, 1);
	spi_write(robo, &page, 1);

	/* Disable SPI access */
	spi_select(robo, 0);
}

/* Write register thru SPI */
static int
spi_wreg(robo_info_t *robo, uint8 page, uint8 addr, void *val, int len)
{
	int status = 0;
	uint8 cmd8;

	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Write with CID 0 */
	cmd8 = ((6 << 4) |		/* normal SPI */
	        1);			/* write */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);
	spi_write(robo, (uint8 *)val, len);

	ET_MSG(("%s: [0x%x-0x%x] := 0x%x (len %d)\n", __FUNCTION__, page, addr,
	        *(uint16 *)val, len))
;
	/* Disable SPI access */
	spi_select(robo, 0);
	return status;
}

/* Read register thru SPI in fast SPI mode */
static int
spi_rreg(robo_info_t *robo, uint8 page, uint8 addr, void *val, int len)
{
	int status = 0;
	uint8 cmd8;

	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Fast SPI read with CID 0 and byte offset 0 */
	cmd8 = (1 << 4);		/* fast SPI */
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);

	status = spi_read(robo, (uint8 *)val, len);

	ET_MSG(("%s: [0x%x-0x%x] => 0x%x (len %d)\n", __FUNCTION__, page, addr,
	        *(uint16 *)val, len));

	/* Disable SPI access */
	spi_select(robo, 0);
	return status;
}

/* High level switch configuration functions. */

/* SPI/gpio interface functions */
static dev_ops_t spigpio = {
	gpio_enable,
	gpio_disable,
	spi_wreg,
	spi_rreg,
	"SPI (GPIO)"
};


/* Access switch registers through MII (MDC/MDIO) */

#define MII_MAX_RETRY	100

/* Write register thru MDC/MDIO */
/* static */ int
mii_wreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint32 flags, savephyaddr, cmd32, val32;
	PUM_DEVICE_BLOCK pudev = robo->pudev;
	PLM_DEVICE_BLOCK pdev = &pudev->lm_dev;
	int i;

	/* validate value length and buffer address */
	ASSERT(len == 1 || (len == 2 && !((int)val & 1)) ||
	       (len == 4 && !((int)val & 3)));

	ET_MSG(("Alex:mii_Wreg: [0x%x-0x%x] := 0x%x (len %d)\n",  page, reg,
	          *(uint16 *)val, len));

	BCM5700_PHY_LOCK(pudev, flags);
	savephyaddr = pdev->PhyAddr;
	pdev->PhyAddr = PSEUDO_PHYAD;

	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd32 = ((page << 8) |		/* page number */
		         1);			/* mdc/mdio access enable */
		LM_WritePhy(pdev, REG_MII_PAGE, cmd32);
		robo->page = page;
	}

	switch (len) {
	case 1:
	case 2:
		val32 = (len == 1) ? *(uint8 *)val : *(uint16 *)val;
		LM_WritePhy(pdev, REG_MII_DATA0, val32); 
		break;
	case 4:
		val32 = *(uint32 *)val;
		LM_WritePhy(pdev, REG_MII_DATA0, val32);
		val32 = (*(uint32 *)val >> 16);
		LM_WritePhy(pdev, REG_MII_DATA1, val32);
		break;
	}
	
	/* set register address - MII register 0x11 */
	cmd32 = ((reg << 8) |		/* register address */
	         1);		/* opcode write */
	LM_WritePhy(pdev, REG_MII_ADDR, cmd32);

	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		LM_ReadPhy(pdev, REG_MII_ADDR, &val32);
		if ((val32 & 3) == 0)
			break;
	}

	pdev->PhyAddr = savephyaddr;
	BCM5700_PHY_UNLOCK(pudev, flags);

	/* timed out */
	if (!i) {
		ET_ERROR(("mii_wreg: timeout"));
		return -1;
	}
	ET_MSG(("Alex:end of mii_wreg\n"));
	return 0;
}

/* Read register thru MDC/MDIO */
/* static */ int
mii_rreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint32 flags, savephyaddr, cmd32, val32, val32hi;
	PUM_DEVICE_BLOCK pudev = robo->pudev;
	PLM_DEVICE_BLOCK pdev = &pudev->lm_dev;
	int i;

	/* validate value length and buffer address */
	ASSERT(len == 1 || (len == 2 && !((int)val & 1)) ||
	       (len == 4 && !((int)val & 3)));

	BCM5700_PHY_LOCK(pudev, flags);
	savephyaddr = pdev->PhyAddr;
	pdev->PhyAddr = PSEUDO_PHYAD;

	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd32 = ((page << 8) |		/* page number */
		         1);			/* mdc/mdio access enable */
		LM_WritePhy(pdev, REG_MII_PAGE, cmd32);
		robo->page = page;
	}

	/* set register address - MII register 0x11 */
	cmd32 = ((reg << 8) |		/* register address */
	         2);			/* opcode read */
	LM_WritePhy(pdev, REG_MII_ADDR, cmd32);

	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		LM_ReadPhy(pdev, REG_MII_ADDR, &val32);
		if ((val32 & 3) == 0)
			break;
	}
	/* timed out */
	if (!i) {
		ET_ERROR(("mii_rreg: timeout"));
		pdev->PhyAddr = savephyaddr;
		BCM5700_PHY_UNLOCK(pudev, flags);
		return -1;
	}
	/* read data - MII register 0x18-0x1B */
	LM_ReadPhy(pdev, REG_MII_DATA0, &val32);

	ET_MSG(("Alex:mii_Rreg: [0x%x-0x%x] => 0x%x (len %d)\n", page, reg, val32, len));

	switch (len) {
	case 1:
		*(uint8 *)val = (uint8)(val32 & 0xff);
		break;
	case 2:
		*(uint16 *)val = (uint16)val32;
		break;
	case 4:
		LM_ReadPhy(pdev, REG_MII_DATA0, &val32hi);
		*(uint32 *)val = (val32hi << 16) | (val32 & 0xffff);
		break;
	}

	pdev->PhyAddr = savephyaddr;
	BCM5700_PHY_UNLOCK(pudev, flags);

	ET_MSG(("Alex: end of mii_rreg\n"));
	return 0;
}

static dev_ops_t mdcmdio = {
	NULL,
	NULL,
	mii_wreg,
	mii_rreg,
	"MII (MDC/MDIO)"
};

/* Get access to the RoboSwitch */
robo_info_t *
robo_attach(PUM_DEVICE_BLOCK pudev, sb_t *sbh)
{
	PLM_DEVICE_BLOCK pdev = &pudev->lm_dev;
	uint32 savephyaddr, flags, val32;
	int mosi, miso, ss, sck;
	robo_info_t *robo = &gRobo;


	/* Init robo structure */
	bzero(robo, sizeof(robo_info_t));

	mosi = miso = ss = sck = 0;

	robo->pudev = pudev;
	robo->sbh = sbh;
	robo->page = -1;

	BCM5700_PHY_LOCK(pudev, flags);
	savephyaddr = pdev->PhyAddr;
	pdev->PhyAddr = PSEUDO_PHYAD;
	LM_ReadPhy(pdev, 2, &val32);
	robo->devid = val32 & 0xffff;
	ET_MSG(("Alex: 2 devid=%x\n",robo->devid));
	LM_ReadPhy(pdev, 3, &val32);
	robo->devid |= val32 << 16;
	ET_MSG(("Alex: 3 devid=%x\n",robo->devid));
	pdev->PhyAddr = savephyaddr;

	mii_rreg(robo, PAGE_MMR, REG_DEVICE_ID, &robo->phyid, 2);
	ET_MSG(("Alex: phyid=%x\n",robo->phyid));

	BCM5700_PHY_UNLOCK(pudev, flags);
#if 0
	if ((robo->devid == 0) &&
	    (mii_rreg(robo, PAGE_MMR, REG_DEVICE_ID, &robo->phyid, 2) == 0) &&
	    (robo->phyid == 0x97)) {
#else
	  if  (mii_rreg(robo, PAGE_MMR, REG_DEVICE_ID, &robo->phyid, 2) == 0){
#endif
		ET_MSG(("%s: phyid read via mii: 0x%x\n", __FUNCTION__, robo->phyid));
		robo->ops = &mdcmdio;
	} else {
		/* Init GPIO mapping. Default 12, 13, 14, 15 */
		mosi = getgpiopin(NULL, "robo_mosi", 12);
		robo->mosi = 1 << mosi;
		miso = getgpiopin(NULL, "robo_miso", 13);
		robo->miso = 1 << miso;
		ss = getgpiopin(NULL, "robo_ss", 14);
		robo->ss = 1 << ss;
		sck = getgpiopin(NULL, "robo_sck", 15);
		robo->sck = 1 << sck;
		robo->ops = &spigpio;
		ET_MSG(("%s: mosi %d miso %d ss %d sck %d\n", __FUNCTION__,
		        mosi, miso, ss, sck));
	}

	/* sanity check */
	ASSERT(robo->ops);
	ASSERT(robo->ops->write_reg);
	ASSERT(robo->ops->read_reg);

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

	return robo;
}

/* Release access to the RoboSwitch */
void
robo_detach(robo_info_t *robo)
{
	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);
}

/* Enable the device and set it to a known good state */
int
robo_enable_device(robo_info_t *robo)
{

	return 0;
}

/* Port flags */
#define FLAG_TAGGED	't'	/* output tagged (external ports only) */
#define FLAG_UNTAG	'u'	/* input & output untagged (CPU port only, for OS (linux, ...) */
#define FLAG_LAN	'*'	/* input & output untagged (CPU port only, for CFE */

/* Configure the VLANs */
int
robo_config_vlan(robo_info_t *robo)
{
	uint8 val8;
	uint16 val16;
	uint32 val32;
	/* port descriptor */
	struct {
		uint32 untag;	/* untag enable bit (Page 0x05 Address 0x63-0x66 Bit[17:9]) */
		uint32 member;	/* vlan member bit (Page 0x05 Address 0x63-0x66 Bit[7:0]) */
		uint8 ptagr;	/* port tag register address (Page 0x34 Address 0x10-0x1F) */
		uint8 cpu;	/* is this cpu port? */
	} pdesc[] = {
		/* 5397/5398 is 0 ~ 7.  port 8 is IMP port.*/
		/* port 0 */ {1 << 9, 1 << 0, REG_VLAN_PTAG0, 0},
		/* port 1 */ {1 << 10, 1 << 1, REG_VLAN_PTAG1, 0},
		/* port 2 */ {1 << 11, 1 << 2, REG_VLAN_PTAG2, 0},
		/* port 3 */ {1 << 12, 1 << 3, REG_VLAN_PTAG3, 0},
		/* port 4 */ {1 << 13, 1 << 4, REG_VLAN_PTAG4, 0},
		/* port 5 */ {1 << 14, 1 << 5, REG_VLAN_PTAG5, 0},
		/* port 6 */ {1 << 15, 1 << 6, REG_VLAN_PTAG6, 0},
		/* port 7 */ {1 << 16, 1 << 7, REG_VLAN_PTAG7, 0},
		/* mii port */ {1 << 17, 1 << 8, REG_VLAN_PTAG8, 1},
	};
	uint16 vid;

	/* setup global vlan configuration */
	/* VLAN Control 0 Register (Page 0x34, Address 0) */
	val8 = ((1 << 7) |		/* enable 802.1Q VLAN */
	        (3 << 5));		/* individual VLAN learning mode */
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL0, &val8, sizeof(val8));

	/* VLAN Control 1 Register (Page 0x34, Address 1) */
	val8 = ((1 << 2) |		/* enable RSV multicast V Fwdmap */
		(1 << 3));		/* enable RSV multicast V Untagmap */
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL1, &val8, sizeof(val8));

	/* setup each vlan. max. 16 vlans. */
	/* force vlan id to be equal to vlan number */
	for (vid = 0; vid < VLAN_NUMVLANS; vid ++) {
		char vlanports[] = "vlanXXXXports";
		char port[] = "XXXX", *ports, *next, *cur;
		uint32 untag = 0;
		uint32 member = 0;
		int pid, len;

		/* no members if VLAN id is out of limitation */
		if (vid > VLAN_MAXVID)
			goto vlan_setup;

		/* get vlan member ports from nvram */
		sprintf(vlanports, "vlan%dports", vid);
		ports = getvar(NULL, vlanports);

		/* In 5397 vid == 0 us invalid?? */
		if (vid == 0) {
			if (ports)
				ET_ERROR(("VID 0 is set in nvram, Ignoring\n"));
			continue;
		}

		/* disable this vlan if not defined */
		if (!ports)
			goto vlan_setup;

		ET_MSG(("Alex: getvar vlan%dports=%s\n",vid,ports));
		/*
		 * setup each port in the vlan. cpu port needs special handing
		 * (with or without output tagging) to support linux/pmon/cfe.
		 */
		for (cur = ports; cur; cur = next) {
			/* tokenize the port list */
			while (*cur == ' ')
				cur ++;
			next = bcmstrstr(cur, " ");
			len = next ? next - cur : strlen(cur);
			if (!len)
				break;
			if (len > sizeof(port) - 1)
				len = sizeof(port) - 1;
			strncpy(port, cur, len);
			port[len] = 0;

			/* make sure port # is within the range */
			pid = bcm_atoi(port);
			if (pid >= sizeof(pdesc) / sizeof(pdesc[0])) {
				ET_ERROR(("robo_config_vlan: port %d in vlan%dports is out "
				          "of range\n", pid, vid));
				continue;
			}

			/* build VLAN registers values */
#ifndef	_CFE_
			if ((!pdesc[pid].cpu && !strchr(port, FLAG_TAGGED)) ||
			    (pdesc[pid].cpu && strchr(port, FLAG_UNTAG)))
#endif
				untag |= pdesc[pid].untag;

			member |= pdesc[pid].member;

			/* set port tag - applies to untagged ingress frames */
			/* Default Port Tag Register (Page 0x34, Address 0x10-0x1D) */
			if (!pdesc[pid].cpu ||
#ifdef	_CFE_
                            strchr(port, FLAG_LAN)
#else
                            strchr(port, FLAG_UNTAG)
#endif /* _CFE_ */
				) {
				val16 = ((0 << 13) |		/* priority - always 0 */
				         vid);			/* vlan id */
				robo->ops->write_reg(robo, PAGE_VLAN, pdesc[pid].ptagr,
				                     &val16, sizeof(val16));
			}
		}

		/* setup VLAN ID and VLAN memberships */
vlan_setup:

		/* VLAN Table Entry Register (Page 0x05, Address 0x63-0x66) */
		val32 = (untag |			/* untag enable */
		         member);			/* vlan members */
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_ENTRY, &val32, sizeof(val32));

		/* VLAN Table Address Index Register (Page 0x05, Address 0x61-0x62) */
		val16 = vid;		/* vlan id */
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_INDX, &val16, sizeof(val16));

		/* VLAN Table Access Register (Page 0x34, Address 0x60) */
		val8 = ((1 << 7) | 	/* start command */
			0);		/* write */
		robo->ops->write_reg(robo, PAGE_VTBL, REG_VTBL_ACCESS, &val8, sizeof(val8));
	}

	return 0;
}

/* Enable switching/forwarding */
int
robo_enable_switch(robo_info_t *robo)
{
	int status = 0;
	uint8 mode8;
	uint8 i;

	/* Switch Mode register (Page 0, Address 0x0B) */
	robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &mode8, sizeof(mode8));

	/* Bit 1 enables switching/forwarding */
	if (!(mode8 & (1 << 1))) {
		/* Enable forwarding */
		mode8 |= (1 << 1);
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &mode8, sizeof(mode8));

		/* Read back */
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MODE, &mode8, sizeof(mode8));
		if (!(mode8 & (1 << 1))) {
			ET_ERROR(("robo_enable_switch: enabling forwarding failed\n"));
			status = -1;
		}
	}
	
	/*Make STP_STATE to 0, which is necessary for fwg working*/
	for(i=0; i<5; i++)
	{
	 uint8 mode8;

		robo->ops->read_reg(robo, PAGE_CTRL, i, &mode8, sizeof(mode8));
		mode8 &= 0x1f;	//make bit[5:7] is 0.
		robo->ops->write_reg(robo, PAGE_CTRL, i, &mode8, sizeof(mode8));	}

	return status;
}
