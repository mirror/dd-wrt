/*
 * RoboSwitch setup functions
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <sbutils.h>
#include <sbconfig.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmparams.h>
#include <proto/ethernet.h>
#include <bcmenetmib.h>
#include <bcmdevs.h>
#include <etc.h>
#include <etc_robo.h>
#include <et_dbg.h>

/*
* Switch can be programmed through SPI, MII, or OCP/SB. Each
* interface has a rreg and a wreg functions to read from and 
* write to registers.
*/
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

/* Private state per RoboSwitch */
struct robo_info_s {
	void *sbh;			/* SiliconBackplane handle */
	void *vars;			/* nvram variables handle */
	dev_ops_t *ops;			/* device ops */
	uint32 devid;			/* device ID (phyid) */
	uint8 page;			/* current page */

	/* SPI */
	uint32 ss, sck, mosi, miso;	/* GPIO mapping */

	/* MII */
	void *ch;			/* etc */
	struct chops *chop;		/* etc chop */
};

/* Constants */
#define PSEUDO_PHYAD	0x1E	/* MII Pseudo PHY address */

/* Page numbers */
#define PAGE_CTRL	0x00	/* Control page */
#define PAGE_VLAN	0x34	/* VLAN page */

/* Control page registers */
#define REG_CTRL_PORT0	0x00	/* Port 0 Mode register */
#define REG_CTRL_MODE	0x0B	/* Switch Mode register */
#define REG_CTRL_MIIPO	0x0E	/* MII Port Override register */

/* VLAN page registers */
#define REG_VLAN_CTRL0	0x00	/* VLAN Control 0 register */
#define REG_VLAN_CTRL1	0x01	/* VLAN Control 1 register */
#define REG_VLAN_CTRL4	0x04	/* VLAN Control 4 register */
#define REG_VLAN_CTRL5	0x05	/* VLAN Control 5 register */
#define REG_VLAN_ACCESS	0x06	/* VLAN Table Access register */
#define REG_VLAN_ACCESS_5365	0x08	/* 5365 VLAN Table Access register */
#define REG_VLAN_WRITE	0x08	/* VLAN Write register */
#define REG_VLAN_WRITE_5365	0x0A	/* 5365 VLAN Write register */
#define REG_VLAN_READ	0x0C	/* VLAN Read register */
#define REG_VLAN_PTAG0	0x10	/* VLAN Default Port Tag register - port 0 */
#define REG_VLAN_PTAG1	0x12	/* VLAN Default Port Tag register - port 1 */
#define REG_VLAN_PTAG2	0x14	/* VLAN Default Port Tag register - port 2 */
#define REG_VLAN_PTAG3	0x16	/* VLAN Default Port Tag register - port 3 */
#define REG_VLAN_PTAG4	0x18	/* VLAN Default Port Tag register - port 4 */
#define REG_VLAN_PTAG5	0x1A	/* VLAN Default Port Tag register - MII port */
#define REG_VLAN_PMAP	0x20	/* VLAN Priority Re-map register */

#define VLAN_NUMVLANS	16	/* # of VLANs */

/* MII registers */
#define REG_MII_PAGE	0x10	/* MII Page register */
#define REG_MII_ADDR	0x11	/* MII Address register */
#define REG_MII_DATA0	0x18	/* MII Data register 0 */
#define REG_MII_DATA1	0x19	/* MII Data register 1 */

/* SPI registers */
#define REG_SPI_PAGE	0xff	/* SPI Page register */

/* Misc. */
#define INVAL_DEVID	-1	/* invalid PHY id */

/*
* Access switch registers through GPIO/SPI
*/
/* Minimum timing constants */
#define SCK_EDGE_TIME	2	/* clock edge duration - 2us */
#define MOSI_SETUP_TIME	1	/* input setup duration - 1us */
#define SS_SETUP_TIME	1	/* select setup duration - 1us */

/* misc. constants */
#define SPI_MAX_RETRY	100

/* Enable GPIO access to the chip */
static void
gpio_enable(robo_info_t *robo)
{
	/* Enable GPIO outputs with SCK and MOSI low, SS high */
	sb_gpioout(robo->sbh, robo->ss | robo->sck | robo->mosi, robo->ss);
	sb_gpioouten(robo->sbh, robo->ss | robo->sck | robo->mosi, 
		robo->ss | robo->sck | robo->mosi);
}

/* Disable GPIO access to the chip */
static void
gpio_disable(robo_info_t *robo)
{
	/* Disable GPIO outputs with all their current values */
	sb_gpioouten(robo->sbh, robo->ss | robo->sck | robo->mosi, 0);
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
			sb_gpioout(robo->sbh, robo->sck, 0);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on rising edge */
			if (mask & buf[i])
				sb_gpioout(robo->sbh, robo->mosi, robo->mosi);
			else
				sb_gpioout(robo->sbh, robo->mosi, 0);
			OSL_DELAY(MOSI_SETUP_TIME);
		
			/* Clock high */
			sb_gpioout(robo->sbh, robo->sck, robo->sck);
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
			sb_gpioout(robo->sbh, robo->sck, 0);
			OSL_DELAY(SCK_EDGE_TIME);

			/* Sample on falling edge */
			if (sb_gpioin(robo->sbh) & robo->miso)
				byte |= mask;

			/* Clock high */
			sb_gpioout(robo->sbh, robo->sck, robo->sck);
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
	/* Enable SPI access */
	if (spi)
		sb_gpioout(robo->sbh, robo->ss, 0);
	/* Disable SPI access */
	else
		sb_gpioout(robo->sbh, robo->ss, robo->ss);
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
	cmd8 = (6 << 4)		/* normal SPI */
		| 1		/* write */
		;
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
	union {
		uint8 val8;
		uint16 val16;
		uint32 val32;
	} bytes;

	/* validate value length and buffer address */
	ASSERT (len == 1 || (len == 2 && !((int)val & 1)) || 
		(len == 4 && !((int)val & 3)));
	
	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Write with CID 0 */
	cmd8 = (6 << 4)		/* normal SPI */
		| 1		/* write */
		;
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);
	switch (len) {
	case 1:
		bytes.val8 = *(uint8 *)val;
		break;
	case 2:
		bytes.val16 = htol16(*(uint16 *)val);
		break;
	case 4:
		bytes.val32 = htol32(*(uint32 *)val);
		break;
	}
	spi_write(robo, (uint8 *)&bytes, len);

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
	union {
		uint8 val8;
		uint16 val16;
		uint32 val32;
	} bytes;

	/* validate value length and buffer address */
	ASSERT (len == 1 || (len == 2 && !((int)val & 1)) || 
		(len == 4 && !((int)val & 3)));
	
	/* Select chip and page */
	spi_goto(robo, page);

	/* Enable SPI access */
	spi_select(robo, 1);

	/* Fast SPI read with CID 0 and byte offset 0 */
	cmd8 = (1 << 4)		/* fast SPI */
		;
	spi_write(robo, &cmd8, 1);
	spi_write(robo, &addr, 1);
	status = spi_read(robo, (uint8 *)&bytes, len);
	switch (len) {
	case 1:
		*(uint8 *)val = bytes.val8;
		break;
	case 2:
		*(uint16 *)val = ltoh16(bytes.val16);
		break;
	case 4:
		*(uint32 *)val = ltoh32(bytes.val32);
		break;
	}

	/* Disable SPI access */
	spi_select(robo, 0);
	return status;
}

/*
* Access switch registers through MII (MDC/MDIO)
*/
/* misc. constants */
#define MII_MAX_RETRY	100

/* Write register thru MDC/MDIO */
static int
mii_wreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint16 cmd16, val16;
	int i;

	/* validate value length and buffer address */
	ASSERT (len == 1 || (len == 2 && !((int)val & 1)) || 
		(len == 4 && !((int)val & 3)));	

	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd16 = (page << 8)	/* page number */
			| 1		/* mdc/mdio access enable */
			;
		robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_PAGE, cmd16);
		robo->page = page;
	}
	/* write data - MII register 0x18-0x1B */
	switch (len) {
	case 1:
	case 2:
		val16 = (len == 1) ? *(uint8 *)val : *(uint16 *)val;
		robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		break;
	case 4:
		val16 = (uint16)*(uint32 *)val;
		robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_DATA0, val16);
		val16 = (uint16)(*(uint32 *)val >> 16);
		robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_DATA1, val16);
		break;
	}
	/* set register address - MII register 0x11 */
	cmd16 = (reg << 8)	/* register address */
		| 1		/* opcode write */
		;
	robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_ADDR, cmd16);
	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		val16 = robo->chop->phyrd(robo->ch, PSEUDO_PHYAD, REG_MII_ADDR);
		if ((val16 & 3) == 0)
			break;
	}
	/* timed out */
	if (!i) {
		ET_ERROR(("mii_wreg: timeout"));
		return -1;
	}
	return 0;
}

/* Read register thru MDC/MDIO */
static int
mii_rreg(robo_info_t *robo, uint8 page, uint8 reg, void *val, int len)
{
	uint16 cmd16, val16;
	int i;

	/* validate value length and buffer address */
	ASSERT (len == 1 || (len == 2 && !((int)val & 1)) || 
		(len == 4 && !((int)val & 3)));
	
	/* set page number - MII register 0x10 */
	if (robo->page != page) {
		cmd16 = (page << 8)	/* page number */
			| 1		/* mdc/mdio access enable */
			;
		robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_PAGE, cmd16);
		robo->page = page;
	}
	/* set register address - MII register 0x11 */
	cmd16 = (reg << 8)	/* register address */
		| 2		/* opcode read */
		;
	robo->chop->phywr(robo->ch, PSEUDO_PHYAD, REG_MII_ADDR, cmd16);
	/* is operation finished? */
	for (i = MII_MAX_RETRY; i > 0; i --) {
		val16 = robo->chop->phyrd(robo->ch, PSEUDO_PHYAD, REG_MII_ADDR);
		if ((val16 & 3) == 0)
			break;
	}
	/* timed out */
	if (!i) {
		ET_ERROR(("mii_rreg: timeout"));
		return -1;
	}
	/* read data - MII register 0x18-0x1B */
	switch (len) {
	case 1:
	case 2:
		val16 = robo->chop->phyrd(robo->ch, PSEUDO_PHYAD, REG_MII_DATA0);
		if (len == 1)
			*(uint8 *)val = (uint8)val16;
		else
			*(uint16 *)val = val16;
		break;
	case 4:
		val16 = robo->chop->phyrd(robo->ch, PSEUDO_PHYAD, REG_MII_DATA0);
		*(uint32 *)val = val16;
		val16 = robo->chop->phyrd(robo->ch, PSEUDO_PHYAD, REG_MII_DATA1);
		*(uint32 *)val += val16 << 16;
		break;
	}
	return 0;
}

/*
* High level switch configuration functions.
*/

/* device oprands */
static dev_ops_t bcm5325 = {
	gpio_enable,
	gpio_disable,
	spi_wreg,
	spi_rreg,
	"bcm5325, SPI (GPIO)"
};
static dev_ops_t bcm5325e = {
	NULL,
	NULL,
	mii_wreg,
	mii_rreg,
	"bcm5325e or compatible, MII (MDC/MDIO)"
};

/* Get access to the RoboSwitch */
robo_info_t *
robo_attach(void *sbh, struct chops *chop, void *ch, void *vars)
{
	robo_info_t *robo;

	/* Allocate and init private state */
	if (!(robo = MALLOC(sb_osh(sbh), sizeof(robo_info_t)))) {
		ET_ERROR(("robo_attach: out of memory, malloced %d bytes", MALLOCED(sb_osh(sbh))));
		return NULL;
	}
	bzero((char *)robo, sizeof(robo_info_t));
	robo->sbh = sbh;
	robo->vars = vars;
	robo->page = -1;
	
	/* 
	* Probe thru pseudo phyad 0x1E if the BCM5325E compatible device exists.
	* BCM5325 is not expected to respond to the query therefore phyid is -1.
	* BCM5325E and compatibles respond to MII query but the phyid may or may 
	* not be implemented therefore any non -1 phyid is valid.
	*/
	robo->devid = chop->phyrd(ch, PSEUDO_PHYAD, 2);
	robo->devid |= chop->phyrd(ch, PSEUDO_PHYAD, 3) << 16;

	/* BCM5325 */
	if (robo->devid == INVAL_DEVID) {
		/* Init GPIO mapping. Default 2, 3, 4, 5 */
		robo->ss = 1 << getgpiopin(vars, "robo_ss", 2);
		robo->sck = 1 << getgpiopin(vars, "robo_sck", 3);
		robo->mosi = 1 << getgpiopin(vars, "robo_mosi", 4);
		robo->miso = 1 << getgpiopin(vars, "robo_miso", 5);
		robo->ops = &bcm5325;
	}
	/* BCM5325E or compatible */
	else {
		/* cache etc chop */
		robo->ch = ch;
		robo->chop = chop;
		robo->ops = &bcm5325e;
	}

	/* sanity check */
	ASSERT(robo->ops);
	ASSERT(robo->ops->write_reg);
	ASSERT(robo->ops->read_reg);

	return robo;
}

/* Release access to the RoboSwitch */
void
robo_detach(robo_info_t *robo)
{
	MFREE(sb_osh(robo->sbh), robo, sizeof(robo_info_t));
}

/* Enable the device and set it to a known good state */
int
robo_enable_device(robo_info_t *robo)
{
	int status = 0;
	uint8 mii8;
	uint32 reset;
	
	/*
	* Explicitly enable the external switch by toggling the GPIO
	* pin if nvram variable 'gpioX=robo_reset' exists. This var
	* tells that GPIO pin X is connected to the switch's RESET pin
	* and keeps the switch in reset after a POR.
	*
	*   Note: Return value 0xff from getgpiopin() means there is 
	*         no need to enable the switch or the switch is an
	*         integrated robo core.
	*/
	if ((reset = getgpiopin(robo->vars, "robo_reset", 0xff)) == 0xff) {
		/* Enable the core if it exists. */
		uint idx = sb_coreidx(robo->sbh);
		if (sb_setcore(robo->sbh, SB_ROBO, 0))
			sb_core_reset(robo->sbh, 0);
		sb_setcoreidx(robo->sbh, idx);
		goto rvmii_enable;
	}
	
	/*
	* External switch enable sequence: RESET low(50ms)->high(20ms)
	*
	* We have to perform a full sequence for we don't know how long
	* it has been from power on till now.
	*/
	/* convert gpio pin to gpio register mask/value */
	reset = 1 << reset;

	/* Keep RESET low for 50 ms */
	sb_gpioout(robo->sbh, reset, 0);
	sb_gpioouten(robo->sbh, reset, reset);
	bcm_mdelay(50);
	
	/* Keep RESET high for at least 20 ms */
	sb_gpioout(robo->sbh, reset, reset);
	bcm_mdelay(20);

	/*
	* Must put the switch into Reverse MII mode!
	*/
rvmii_enable:
	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);
	
	/* MII port state override (page 0 register 14) */
	robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &mii8, sizeof(mii8));

	/* Bit 4 enables reverse MII mode */
	if (!(mii8 & (1 << 4))) {
		/* Enable RvMII */
		mii8 |= (1 << 4);
		robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &mii8, sizeof(mii8));

		/* Read back */
		robo->ops->read_reg(robo, PAGE_CTRL, REG_CTRL_MIIPO, &mii8, sizeof(mii8));
		if (!(mii8 & (1 << 4))) {
			ET_ERROR(("robo_enable_device: enabling RvMII mode failed\n"));
			status = -1;
		}
	}

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);
	
	return status;
}

/* Port flags */
#define FLAG_TAGGED	't'	/* output tagged (external ports only) */
#define FLAG_UNTAG	'u'	/* input & output untagged (CPU port only, for OS (linux, ...) */
#define FLAG_LAN	'*'	/* input & output untagged (CPU port only, for bootloader (CFE, ...) */

/* Configure the VLANs */
int
robo_config_vlan(robo_info_t *robo)
{
	uint8 val8;
	uint16 val16;
	uint32 val32;
	/* port descriptor */
	struct {
		uint16 untag;	/* untag enable bit (Page 0x34 Address 0x08-0x0B Bit[11:6]) */
		uint16 member;	/* vlan member bit (Page 0x34 Address 0x08-0x0B Bit[5:0]) */
		uint8 ptagr;	/* port tag register address (Page 0x34 Address 0x10-0x1D) */
		uint8 cpu;	/* is this cpu port? */
	} pdesc[] = {
		/* port 0 */ {1 << 6, 1 << 0, REG_VLAN_PTAG0, 0},
		/* port 1 */ {1 << 7, 1 << 1, REG_VLAN_PTAG1, 0},
		/* port 2 */ {1 << 8, 1 << 2, REG_VLAN_PTAG2, 0},
		/* port 3 */ {1 << 9, 1 << 3, REG_VLAN_PTAG3, 0},
		/* port 4 */ {1 << 10, 1 << 4, REG_VLAN_PTAG4, 0},
		/* mii port */ {1 << 11, 1 << 5, REG_VLAN_PTAG5, 1},
	};
	uint16 vid;

	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);
	
	/* setup global vlan configuration */
	/* VLAN Control 0 Register (Page 0x34, Address 0) */
	val8 = (1 << 7)		/* enable 802.1Q VLAN */
		| (3 << 5)	/* individual VLAN learning mode */
		;
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL0, &val8, sizeof(val8));
	
	/* VLAN Control 1 Register (Page 0x34, Address 1) */
	val8 = (1 << 1)		/* enable RSV multicast V Tagging */
		| (1 << 2)	/* enable RSV multicast V Fwdmap */
		| (1 << 3)	/* enable RSV multicast V Untagmap */
		;
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL1, &val8, sizeof(val8));
	
	/* VLAN Control 4 Register (Page 0x34, Address 4) */
	val8 = (1 << 6)		/* drop frame with VID violation */
		;
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL4, &val8, sizeof(val8));
	
	/* VLAN Control 5 Register (Page 0x34, Address 5) */
	val8 = (1 << 3)		/* drop frame when miss V table */
		;
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_CTRL5, &val8, sizeof(val8));
	
	/* setup each vlan. max. 16 vlans. */
	/* force vlan id to be equal to vlan number */
	for (vid = 0; vid < VLAN_NUMVLANS; vid ++) {
		char vlanports[] = "vlanXXXXports";
		char port[] = "XXXX", *ports, *next, *cur;
		uint16 untag = 0;
		uint16 member = 0;
		int pid, len;

		/* no members if VLAN id is out of limitation */
		if (vid > VLAN_MAXVID)
			goto vlan_setup;

		/* get vlan member ports from nvram */
		sprintf(vlanports, "vlan%dports", vid);
		ports = getvar(robo->vars, vlanports);
		
		/* disable this vlan if not defined */
		if (!ports) {
			goto vlan_setup;
		}

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
				ET_ERROR(("robo_config_vlan: port %d in vlan%dports is out of range\n", pid, vid));
				continue;
			}

			/* build VLAN registers values */
#if defined(PMON) || defined(_CFE_)
			untag |= pdesc[pid].untag;
#else
			if ((!pdesc[pid].cpu && !strchr(port, FLAG_TAGGED)) ||
			    (pdesc[pid].cpu && strchr(port, FLAG_UNTAG)))
				untag |= pdesc[pid].untag;
#endif
			member |= pdesc[pid].member;

			/* set port tag - applies to untagged ingress frames */
			/* Default Port Tag Register (Page 0x34, Addres 0x10-0x1D) */
			if (!pdesc[pid].cpu ||
#if defined(PMON) || defined(_CFE_)
				strchr(port, FLAG_LAN)
#else
				strchr(port, FLAG_UNTAG)
#endif
			    ) {
				val16 = (0 << 13)	/* priority - always 0 */
					| vid		/* vlan id */
					;
				robo->ops->write_reg(robo, PAGE_VLAN, pdesc[pid].ptagr, &val16, sizeof(val16));
			}
		}
		
		/* setup VLAN ID and VLAN memberships */
vlan_setup:
		if (sb_chip(robo->sbh) == BCM5365_DEVICE_ID) 
		{
			/* VLAN Write Register (Page 0x34, Address 0x0A) */
			val32 = (1 << 14)		/* valid write */
				| (untag << 1)		/* untag enable */
				| member		/* vlan members */
				;
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_WRITE_5365, &val32, sizeof(val32));
			/* VLAN Table Access Register (Page 0x34, Address 0x08) */
			val16 = (1 << 13) 	/* start command */
				| (1 << 12) 	/* write state */
				| vid		/* vlan id */
			;
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_ACCESS_5365, &val16, sizeof(val16));
		} else {
			/* VLAN Write Register (Page 0x34, Address 0x08-0x0B) */
			val32 = (1 << 20)		/* valid write */
				| ((vid >> 4) << 12)	/* vlan id bit[11:4] */
				| untag			/* untag enable */
				| member		/* vlan members */
				;
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_WRITE, &val32, sizeof(val32));
			/* VLAN Table Access Register (Page 0x34, Address 0x06-0x07) */
			val16 = (1 << 13) 	/* start command */
				| (1 << 12) 	/* write state */
				| vid		/* vlan id */
				;
			robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_ACCESS, &val16, sizeof(val16));
		}
	}

	/* setup priority mapping - applies to tagged ingress frames */
	/* Priority Re-map Register (Page 0x34, Address 0x20-0x23) */
	val32 = (0 << 0)	/* 0 -> 0 */
		| (1 << 3)	/* 1 -> 1 */
		| (2 << 6)	/* 2 -> 2 */
		| (3 << 9)	/* 3 -> 3 */
		| (4 << 12)	/* 4 -> 4 */
		| (5 << 15)	/* 5 -> 5 */
		| (6 << 18)	/* 6 -> 6 */
		| (7 << 21)	/* 7 -> 7 */
		;
	robo->ops->write_reg(robo, PAGE_VLAN, REG_VLAN_PMAP, &val32, sizeof(val32));
	
	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return 0;
}

/* Enable switching/forwarding */
int
robo_enable_switch(robo_info_t *robo)
{
	int status = 0;
	uint8 mode8;
	
	/* Enable management interface access */
	if (robo->ops->enable_mgmtif)
		robo->ops->enable_mgmtif(robo);

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

	/* Enable WAN port (#0) on the asus wl-500g deluxe boxes */
	mode8 = 0;
	robo->ops->write_reg(robo, PAGE_CTRL, REG_CTRL_PORT0, &mode8, sizeof(mode8));

	/* Disable management interface access */
	if (robo->ops->disable_mgmtif)
		robo->ops->disable_mgmtif(robo);

	return status;
}


