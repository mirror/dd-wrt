/*
 * Platform driver for the Realtek RTL8366 ethernet switch
 *
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/phy.h>
#include <linux/rtl8366rb_smi.h>

//#define DEBUG	1

#define RTL8366RB_SMI_DRIVER_NAME	"rtl8366rb"
#define RTL8366RB_SMI_DRIVER_DESC	"Realtek RTL8366rb switch driver"
#define RTL8366RB_SMI_DRIVER_VER	"0.1.0"

#define RTL8366S_PHY_NO_MAX		4
#define RTL8366S_PHY_PAGE_MAX		7
#define RTL8366S_PHY_ADDR_MAX		31

#define RTL8366S_CHIP_VERSION_CTRL_REG	0x0104
#define RTL8366S_CHIP_VERSION_MASK	0xf
#define RTL8366S_CHIP_ID_REG		0x0105
#define RTL8366S_CHIP_ID_8366		0x8366

/* RTL8366RB Support */
#define RTL8366RB_CHIP_VERSION_CTRL_REG	0x050A
#define RTL8366RB_CHIP_VERSION_MASK	0xf
#define RTL8366RB_CHIP_ID_REG		0x0509
#define RTL8366RB_CHIP_ID_8366		0x5937

/* PHY registers control */
#define RTL8366S_PHY_ACCESS_CTRL_REG	0x8028
#define RTL8366S_PHY_ACCESS_DATA_REG	0x8029

/* RTL8366RB Support */
#define RTL8366RB_PHY_ACCESS_CTRL_REG	0x8000
#define RTL8366RB_PHY_ACCESS_DATA_REG	0x8002

#define RTL8366S_PHY_CTRL_READ		1
#define RTL8366S_PHY_CTRL_WRITE		0

#define RTL8366S_PHY_REG_MASK		0x1f
#define RTL8366S_PHY_PAGE_OFFSET	5
#define RTL8366S_PHY_PAGE_MASK		(0x7 << 5)       
#define RTL8366S_PHY_NO_OFFSET		9
#define RTL8366S_PHY_NO_MASK		(0x1f << 9)

#define RTL8366RB_PHY_PAGE_MASK		(0xf << 5)

#define RTL8366_SMI_ACK_RETRY_COUNT	10
#define RTL8366_SMI_CLK_DELAY		2 /* nsec */
#define RTL8368S_REGBITLENGTH		16

// ADDED DEFINES
// SWITCH Global control regs
#define RTL8368S_SWITCH_GLOBAL_CTRL_REG		0x0000
#define RTL8368S_EN_VLAN_OFF				13
#define RTL8368S_EN_VLAN_4KTB_OFF			14

/* Port ability */
#define RTL8368S_PORT_ABILITY_BASE			0x0010

/* V-LAN member configuration */
#define RTL8368S_VLAN_MEMCONF_BASE			0x0020

/* cpu port control reg */
#define RTL8368S_CPU_CTRL_REG				0x0061
#define RTL8368S_CPU_PORTS_OFF				0
#define RTL8368S_CPU_PORTS_MSK				(0xff<<RTL8368S_CPU_PORTS_OFF)
#define RTL8368S_CPU_INSTAG_OFF				15
#define RTL8368S_CPU_INSTAG_MSK				(0x1<<RTL8368S_CPU_INSTAG_OFF)

/* Port VLAN Control Register */
#define RTL8368S_PORT_VLAN_CTRL_BASE		0x0063

#define RTL8366RB_VLANMCIDXMAX              15
#define RTL8366RB_PORTMASK                  0x3F
#define RTL8366RB_PRIORITYMAX               7
#define RTL8366RB_FIDMAX                    7
#define RTL8366RB_VIDMAX                    0xFFF

/* Table Acess Control */
#define RTL8368S_TABLE_ACCESS_CTRL_REG		0x0180
#define RTL8368S_TABLE_ACCESS_CMD_MSK		0x0001
#define RTL8368S_VLAN_TABLE_WRITE_BASE		0x0185
#define RTL8368S_VLAN_TABLE_READ_BASE		0x018C
#define RTL8368S_TABLE_VLAN_WRITE_CTRL		0x0f01	
#define RTL8368S_TABLE_VLAN_READ_CTRL		0x0e01

/*MAC control*/
#define RTL8368S_MAC_FORCE_CTRL_REG			0x0F11

/*PHY-- CHANGE THESE AS THERE DOUBLES*/
#define RTL8366RB_PORT_MAX 						6
#define RTL8366RB_PHY_NO_MAX                	4
#define RTL8366RB_PHY_PAGE_MAX              	7
#define RTL8366RB_PHY_ADDR_MAX              	31

#define RTL8366RB_PHY_CONTROL_REG               0
#define RTL8366RB_PHY_STATUS_REG                1
#define RTL8366RB_PHY_AN_ADVERTISEMENT_REG      4
#define RTL8366RB_PHY_AN_LINKPARTNER_REG        5
#define RTL8366RB_PHY_1000_BASET_CONTROL_REG    9
#define RTL8366RB_PHY_1000_BASET_STATUS_REG     10

#define RTL8368S_VLANMCIDXMAX					15
#define RTL8368S_VIDMAX							0xFFF
#define RTL8368S_TBL_CMD_CHECK_COUNTER			1000

u32 rtl8366_id;
u32 rtl8366_ver;

/* enum for port ID */
enum PORTID
{
	PORT0 =  0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT6,	
	PORT7,	
	PORT_MAX
};

/* enum for port current link speed */
enum PORTLINKSPEED
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum MACLINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum PORTLINKDUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

struct rtl8366_smi {
	struct platform_device			*pdev;
	struct rtl8366rb_smi_platform_data	*pdata;
	spinlock_t				lock;
	struct mii_bus				*mii_bus;
	int					mii_irq[PHY_MAX_ADDR];
};

struct rtl8366rb_phyAbility_s {
    u16  AutoNegotiation:1;/*PHY register 0.12 setting for auto-negotiation process*/
    u16  Half_10:1;      /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    u16  Full_10:1;      /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    u16  Half_100:1;     /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    u16  Full_100:1;     /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    u16  Full_1000:1;    /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    u16  FC:1;           /*PHY register 4.10 setting for flow control capability*/
    u16  AsyFC:1;        /*PHY register 4.11 setting for  asymmetric flow control capability*/
};

struct rtl8366rb_macConfig_s {
    enum MACLINKMODE force;
    enum PORTLINKSPEED speed;
    enum PORTLINKDUPLEXMODE duplex;
    u32 link;
    u32 txPause;
    u32 rxPause;
};

struct rtl8366rb_vlanConfig_s {
    u32 vid;
    u32 mbrmsk;
    u32 untagmsk;
    u32 fid;
};

struct rtl8368s_vlanconfig_s {
  	u16 	reserved2:1;
	u16 	priority:3;
	u16 	vid:12;
 	u16 	untag:8;
 	u16 	member:8;
	u16		stag_mbr:8;
	u16		stag_idx:3;
	u16 	reserved1:2;
 	u16 	fid:3;	
};

struct rtl8368s_vlan4kentry_s {
 	u16 	reserved1:4;
	u16 	vid:12;
 	u16 	untag:8;
 	u16 	member:8;
	u16 	reserved2:13;
 	u16 	fid:3;
};

struct rtl8368s_user_vlan4kentry_s {
	u16 	vid:12;
 	u16 	member:8;
 	u16 	untag:8;
 	u16 	fid:3;

};

static inline void rtl8366_smi_clk_delay(struct rtl8366_smi *smi)
{
	udelay(RTL8366_SMI_CLK_DELAY);
}



static inline void gpio_direction_input(uint32_t gpio)
{
	*(volatile int *)(0xb8040000) &= ~(1<<gpio);//change to input	
}

static inline void gpio_direction_output(uint32_t gpio)
{
	*(volatile int *)(0xb8040000) |= (1<<gpio);//change to output	
}

static inline uint32_t gpio_get_value(uint32_t gpio)
{	
	 if((*(volatile unsigned long *)0xb8040004) & (1<<gpio))
	 	return 1;
	 else
	 	return 0;			
}

static inline void gpio_set_value(uint32_t gpio, uint32_t v)
{	
	if (v) {//hifh		
		*(volatile int *)(0xb8040008) |= 1<<gpio;
	} else {//low		
		*(volatile int *)(0xb8040008) &= ~(1<<gpio);		
	}
}


static int init_smi = 0;

static void rtl8366_smi_init(struct rtl8366_smi *smi)
{
	unsigned int sda = smi->pdata->gpio_sda;
	unsigned int sck = smi->pdata->gpio_sck;
	if(init_smi == 1)
	{
		return;
	}
	gpio_direction_input(sda);
	gpio_direction_input(sck);
	rtl8366_smi_clk_delay(smi);

}

static void rtl8366_smi_start(struct rtl8366_smi *smi)
{
	unsigned int sda = smi->pdata->gpio_sda;
	unsigned int sck = smi->pdata->gpio_sck;

	/*
	 * Set GPIO pins to output mode, with initial state:
	 * SCK = 0, SDA = 1
	 */
	gpio_direction_output(sck);
	gpio_direction_output(sda);
	rtl8366_smi_clk_delay(smi);

	gpio_set_value(sck, 0);
	gpio_set_value(sda, 1);
	rtl8366_smi_clk_delay(smi);
	
	/* CLK 1: 0 -> 1, 1 -> 0 */
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);

	/* CLK 2: */
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 1);
}

static void rtl8366_smi_stop(struct rtl8366_smi *smi)
{
	unsigned int sda = smi->pdata->gpio_sda;
	unsigned int sck = smi->pdata->gpio_sck;

	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 0);
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sda, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);

	/* add a click */
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 0);
	rtl8366_smi_clk_delay(smi);
	gpio_set_value(sck, 1);

	/* set GPIO pins to input mode */
	gpio_direction_input(sda);
	gpio_direction_input(sck);
}

static void rtl8366_smi_write_bits(struct rtl8366_smi *smi, u32 data, u32 len)
{
	unsigned int sda = smi->pdata->gpio_sda;
	unsigned int sck = smi->pdata->gpio_sck;

	for (; len > 0; len--) {
		rtl8366_smi_clk_delay(smi);

		/* prepare data */
		if ( data & ( 1 << (len - 1)) )
			gpio_set_value(sda, 1);
		else
			gpio_set_value(sda, 0);
		rtl8366_smi_clk_delay(smi);

		/* clocking */
		gpio_set_value(sck, 1);
		rtl8366_smi_clk_delay(smi);
		gpio_set_value(sck, 0);
	}
}

static void rtl8366_smi_read_bits(struct rtl8366_smi *smi, u32 len, u32 *data)
{
	unsigned int sda = smi->pdata->gpio_sda;
	unsigned int sck = smi->pdata->gpio_sck;

	gpio_direction_input(sda);

	for (*data = 0; len > 0; len--) {
		u32 u;

		rtl8366_smi_clk_delay(smi);

		/* clocking */
		gpio_set_value(sck, 1);
		rtl8366_smi_clk_delay(smi);
		u = gpio_get_value(sda);
		gpio_set_value(sck, 0);

		*data |= (u << (len - 1));
	}

	gpio_direction_output(sda);
}

static int rtl8366_smi_wait_for_ack(struct rtl8366_smi *smi)
{
	int retry_cnt;

	retry_cnt = 0;
	do {
		u32 ack;

		rtl8366_smi_read_bits(smi, 1, &ack);
		if (ack == 0)
			break;

		if (++retry_cnt > RTL8366_SMI_ACK_RETRY_COUNT)
			return -EIO;
	} while (1);

	return 0;
}

static int rtl8366_smi_write_byte(struct rtl8366_smi *smi, u8 data)
{
	rtl8366_smi_write_bits(smi, data, 8);
	return rtl8366_smi_wait_for_ack(smi);
}

static int rtl8366_smi_read_byte0(struct rtl8366_smi *smi, u8 *data)
{
	u32 t;

	/* read data */
	rtl8366_smi_read_bits(smi, 8, &t);
	*data = (t & 0xff);

	/* send an ACK */
	rtl8366_smi_write_bits(smi, 0x00, 1);

	return 0;
}

static int rtl8366_smi_read_byte1(struct rtl8366_smi *smi, u8 *data)
{
	u32 t;

	/* read data */
	rtl8366_smi_read_bits(smi, 8, &t);
	*data = (t & 0xff);

	/* send an ACK */
	rtl8366_smi_write_bits(smi, 0x01, 1);

	return 0;
}

static int rtl8366_smi_read_reg(struct rtl8366_smi *smi, u32 addr, u32 *data)
{
	unsigned long flags;
	u8 lo = 0;
	u8 hi = 0;
	int ret;

	spin_lock_irqsave(&smi->lock, flags);

	rtl8366_smi_start(smi);

	rtl8366_smi_write_bits(smi,0x0a, 4); 					/* CTRL code: 4'b1010 */

	rtl8366_smi_write_bits(smi,0x4, 3);						/* CTRL code: 3'b100 */

	rtl8366_smi_write_bits(smi,0x1, 1);						/* 1: issue READ command */

	ret = rtl8366_smi_wait_for_ack(smi);
	/* send READ command */
//	ret = rtl8366_smi_write_byte(smi, 0x0a << 4 | 0x04 << 1 | 0x01);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = rtl8366_smi_write_byte(smi, addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = rtl8366_smi_write_byte(smi, addr >> 8);
	if (ret)
		goto out;

	/* read DATA[7:0] */
	rtl8366_smi_read_byte0(smi, &lo);
	/* read DATA[15:8] */
	rtl8366_smi_read_byte1(smi, &hi);

	*data = ((u32) lo) | (((u32) hi) << 8);

	ret = 0;

 out:
	if (ret)
	    printk(KERN_EMERG "%s: no ack received\n",__func__);
	rtl8366_smi_stop(smi);
	spin_unlock_irqrestore(&smi->lock, flags);

	return ret;
}

static int rtl8366_smi_write_reg(struct rtl8366_smi *smi, u32 addr, u32 data)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&smi->lock, flags);

	rtl8366_smi_start(smi);

	/* send WRITE command */
	rtl8366_smi_write_bits(smi,0x0a, 4); 					/* CTRL code: 4'b1010 */

	rtl8366_smi_write_bits(smi,0x4, 3);						/* CTRL code: 3'b100 */

	rtl8366_smi_write_bits(smi,0x0, 1);						/* 1: issue READ command */


	ret = rtl8366_smi_wait_for_ack(smi);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = rtl8366_smi_write_byte(smi, addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = rtl8366_smi_write_byte(smi, addr >> 8);
	if (ret)
		goto out;

	/* write DATA[7:0] */
	ret = rtl8366_smi_write_byte(smi, data & 0xff);
	if (ret)
		goto out;

	/* write DATA[15:8] */
	ret = rtl8366_smi_write_byte(smi, data >> 8);
	if (ret)
		goto out;

	ret = 0;

 out:
	if (ret)
	    printk(KERN_EMERG "%s: no ack received\n",__func__);
	rtl8366_smi_stop(smi);
	spin_unlock_irqrestore(&smi->lock, flags);

	return ret;
}

static int rtl8366_smi_read_phy_reg(struct rtl8366_smi *smi,
				    u32 phy_no, u32 page, u32 addr, u32 *data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366S_PHY_NO_MAX)
		return -EINVAL;
	
	if (page > RTL8366S_PHY_PAGE_MAX)
		return -EINVAL;
	
	if (addr > RTL8366S_PHY_ADDR_MAX)
		return -EINVAL;
	
	
	if (rtl8366_id == RTL8366S_CHIP_ID_8366)
		ret = rtl8366_smi_write_reg(smi, RTL8366S_PHY_ACCESS_CTRL_REG,
						RTL8366S_PHY_CTRL_READ);
	
	
	if (rtl8366_id == RTL8366RB_CHIP_ID_8366)
		ret = rtl8366_smi_write_reg(smi, RTL8366RB_PHY_ACCESS_CTRL_REG,
						RTL8366S_PHY_CTRL_READ);		
	
	if (ret)
		return ret;

	if (rtl8366_id == RTL8366S_CHIP_ID_8366)	
		reg = 0x8000 | (1 << (phy_no + RTL8366S_PHY_NO_OFFSET)) |
			((page << RTL8366S_PHY_PAGE_OFFSET) & RTL8366S_PHY_PAGE_MASK) |
			(addr & RTL8366S_PHY_REG_MASK);

	if (rtl8366_id == RTL8366RB_CHIP_ID_8366)	
		reg = 0x8000 | (1 << (phy_no + RTL8366S_PHY_NO_OFFSET)) |
			((page << RTL8366S_PHY_PAGE_OFFSET) & RTL8366RB_PHY_PAGE_MASK) |
			(addr & RTL8366S_PHY_REG_MASK);
			
	ret = rtl8366_smi_write_reg(smi, reg, 0);
	
	if (ret)
		return ret;
	
	if (rtl8366_id == RTL8366S_CHIP_ID_8366)
		ret = rtl8366_smi_read_reg(smi, RTL8366S_PHY_ACCESS_DATA_REG, data);
	if (rtl8366_id == RTL8366RB_CHIP_ID_8366)
		ret = rtl8366_smi_read_reg(smi, RTL8366RB_PHY_ACCESS_DATA_REG, data);	
		
	if (ret)
		return ret;
		
	return 0;
}

static int rtl8366_smi_write_phy_reg(struct rtl8366_smi *smi,
				     u32 phy_no, u32 page, u32 addr, u32 data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366S_PHY_NO_MAX)
		return -EINVAL;

	if (page > RTL8366S_PHY_PAGE_MAX)
		return -EINVAL;

	if (addr > RTL8366S_PHY_ADDR_MAX)
		return -EINVAL;

	if (rtl8366_id == RTL8366S_CHIP_ID_8366)	
		ret = rtl8366_smi_write_reg(smi, RTL8366S_PHY_ACCESS_CTRL_REG,
						RTL8366S_PHY_CTRL_WRITE);
					
	if (rtl8366_id == RTL8366RB_CHIP_ID_8366)	
		ret = rtl8366_smi_write_reg(smi, RTL8366RB_PHY_ACCESS_CTRL_REG,
						RTL8366S_PHY_CTRL_WRITE);
					
	if (ret)
		return ret;

	reg = 0x8000 | (1 << (phy_no + RTL8366S_PHY_NO_OFFSET)) |
	      ((page << RTL8366S_PHY_PAGE_OFFSET) & RTL8366S_PHY_PAGE_MASK) |
	      (addr & RTL8366S_PHY_REG_MASK);

	ret = rtl8366_smi_write_reg(smi, reg, data);
	if (ret)
		return ret;

	return 0;
}

#ifdef DEBUG
static void rtl8366_smi_dump_regs(struct rtl8366_smi *smi)
{
	u32 t;
	int err;
	int i;

	for (i = 0; i < 0x200; i++) {
		err = rtl8366_smi_read_reg(smi, i, &t);
		if (err) {
			dev_err(&smi->pdev->dev,
				"unable to read register %04x\n", i);
			return;
		}
		dev_info(&smi->pdev->dev, "reg %04x: %04x\n", i, t);
	}

	for (i = 0; i <= RTL8366S_PHY_NO_MAX; i++) {
		int j;

		for (j = 0; j <= RTL8366S_PHY_ADDR_MAX; j++) {
			err = rtl8366_smi_read_phy_reg(smi, i, 0, j, &t);
			if (err) {
				dev_err(&smi->pdev->dev,
					"unable to read PHY%u:%02x register\n",
					i, j);
				return;
			}
			dev_info(&smi->pdev->dev,
				 "PHY%u:%02x: %04x\n", i, j, t);
		}
	}
}
#else
static inline void rtl8366_smi_dump_regs(struct rtl8366_smi *smi) {}
#endif

static int rtl8366_smi_mii_read(struct mii_bus *bus, int addr, int reg)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 val = 0;
	int err;

	err = rtl8366_smi_read_phy_reg(smi, addr, 0, reg, &val);
	if (err)
		return 0xffff;

	return val;
}

void _rtl8368s_VlanStUser2Smi( struct rtl8368s_user_vlan4kentry_s*VlanUser, struct rtl8368s_vlan4kentry_s*VlanSmi)
{
	VlanSmi->vid		=	VlanUser->vid;
	VlanSmi->untag	=	VlanUser->untag;
	VlanSmi->member	=	VlanUser->member;
	VlanSmi->fid		=	VlanUser->fid;
}

void _rtl8368s_VlanStSmi2User( struct rtl8368s_user_vlan4kentry_s*VlanUser, struct rtl8368s_vlan4kentry_s*VlanSmi)
{
	VlanUser->vid	=	VlanSmi->vid	;
	VlanUser->untag	=	VlanSmi->untag;
	VlanUser->member=	VlanSmi->member;
	VlanUser->fid	=	VlanSmi->fid;
}

static int rtl8368s_setAsicRegBit(struct rtl8366_smi *smi,u32 reg, u32 bit, u32 value)
{
	u32 regData;
	u32 retVal;
	
	if(bit>=RTL8368S_REGBITLENGTH)
		return -EINVAL;

	retVal = rtl8366_smi_read_reg(smi,reg, &regData);
	if (retVal != 0) return -EINVAL;

#ifdef DEBUG
		printk("R:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

	if (value) 
		regData = regData | (1<<bit);
	else
		regData = regData & (~(1<<bit));
	
	retVal = rtl8366_smi_write_reg(smi,reg, regData);
	if (retVal != 0) return -EINVAL;

	return 0;
}

static int rtl8368s_setAsicRegBits(struct rtl8366_smi *smi,u32 reg, u32 bits, u32 value)
{
	u32 regData;
	int retVal;
	
	if(bits>= (1<<RTL8368S_REGBITLENGTH) )
		return -1;

	retVal = rtl8366_smi_read_reg(smi,reg, &regData);
	if (retVal != 0) return -1;

	regData = regData & (~bits);
	regData = regData | (value & bits);
	
	retVal = rtl8366_smi_write_reg(smi,reg, regData);
	if (retVal != 0) return -1;

	return 0;
}

static int rtl8368s_setAsicVlan(struct rtl8366_smi *smi,u32 enabled)
{
	u32 retVal;

	retVal = rtl8368s_setAsicRegBit(smi,RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_EN_VLAN_OFF,enabled);
	
	return retVal;
}

static int rtl8368s_setAsicVlan4kTbUsage(struct rtl8366_smi *smi, u32 enabled)
{
	u32 retVal;

	retVal = rtl8368s_setAsicRegBit(smi,RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_EN_VLAN_4KTB_OFF,enabled);

	return retVal;
}

static int rtl8368s_setAsicVlanPortBasedVID(struct rtl8366_smi *smi,enum PORTID port, u32 index)
{
	int retVal;
	u32 regAddr;
	u32 regData;
	u32 regBits;

	/* bits mapping to port vlan control register of port n */
	const u16 bits[8]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0,0x0F00,0xF000 };
	/* bits offset to port vlan control register of port n */
	const u16 bitOff[8] = { 0,4,8,12,0,4,8,12 };
	/* address offset to port vlan control register of port n */
	const u16 addrOff[8]= { 0,0,0,0,1,1,1,1 };

	if(port >=PORT_MAX)
		return -EINVAL;

	if(index > RTL8368S_VLANMCIDXMAX)
		return -EINVAL;

	regAddr = RTL8368S_PORT_VLAN_CTRL_BASE + addrOff[port];

	regBits = bits[port];

	regData =  (index << bitOff[port]) & regBits;

	retVal = rtl8368s_setAsicRegBits(smi,regAddr,regBits,regData);		
	
	return retVal;
}

static int rtl8368s_setAsicVlan4kEntry(struct rtl8366_smi *smi,struct rtl8368s_user_vlan4kentry_s vlan4kEntry )
{
	u32 retVal;
	u32 regData;
	u16* tableAddr;
	u32 i;
	struct rtl8368s_vlan4kentry_s smiVlan4kentry;

	memset(&smiVlan4kentry, 0x0, sizeof(struct rtl8368s_vlan4kentry_s));

	_rtl8368s_VlanStUser2Smi( &vlan4kEntry, &smiVlan4kentry);

	tableAddr = (u16*)&smiVlan4kentry;

	regData = *tableAddr;
	
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_VLAN_TABLE_WRITE_BASE,regData);
	if(retVal !=  0)
		return retVal;

	tableAddr ++;
	regData = *tableAddr;
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_VLAN_TABLE_WRITE_BASE+1,regData);

	if(retVal !=  0)
		return retVal;
	
	tableAddr ++;
	regData = *tableAddr;
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_VLAN_TABLE_WRITE_BASE+2,regData);

	if(retVal !=  0)
		return retVal;	
	
	/*write table access Control word*/
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_TABLE_ACCESS_CTRL_REG,RTL8368S_TABLE_VLAN_WRITE_CTRL);
	if(retVal !=  0)
		return retVal;

	/*check ASIC command*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8366_smi_read_reg(smi,RTL8368S_TABLE_ACCESS_CTRL_REG,&regData);
		if(retVal !=  0)
			return retVal;

		if(regData &RTL8368S_TABLE_ACCESS_CMD_MSK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return -1;
		}
		else
			break;
	}

	return 0;
}

static int rtl8368s_setAsicVlanMemberConfig(struct rtl8366_smi *smi, u32 index,struct  rtl8368s_vlanconfig_s vlanmconf )
{
	u32 retVal;
	u32 regAddr;
	u32 regData;
	u16* tableAddr;
	
	if(index > RTL8368S_VLANMCIDXMAX)
		return -1;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + (index*3);

	tableAddr = (u16*)&vlanmconf;
	regData = *tableAddr;

	retVal = rtl8366_smi_write_reg(smi,regAddr,regData);
	if(retVal !=  0)
		return retVal;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 1 + (index*3);

	tableAddr ++;
	regData = *tableAddr;

	retVal = rtl8366_smi_write_reg(smi,regAddr,regData);
	if(retVal !=  0)
		return retVal;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 2 + (index*3);

	tableAddr ++;
	regData = *tableAddr;

	retVal = rtl8366_smi_write_reg(smi,regAddr,regData);
	if(retVal !=  0)
		return retVal;
	
	return 0;
}

static int rtl8368s_setAsicCpuPortMask(struct rtl8366_smi *smi,u32 portMask)
{
	u32 retVal;
	retVal = rtl8368s_setAsicRegBits(smi,RTL8368S_CPU_CTRL_REG,RTL8368S_CPU_PORTS_MSK,portMask<<RTL8368S_CPU_PORTS_OFF);		
	return retVal;	
}

static int rtl8368s_setAsicCpuDisableInsTag(struct rtl8366_smi *smi,u32 enable)
{
	u32 retVal;
	retVal = rtl8368s_setAsicRegBit(smi,RTL8368S_CPU_CTRL_REG,RTL8368S_CPU_INSTAG_OFF,enable);
	return retVal;
}

static int rtl8368s_getAsicVlanMemberConfig(struct rtl8366_smi *smi,u32 index,struct  rtl8368s_vlanconfig_s *vlanmconf )
{
	u32 retVal;
	u32 regAddr;
	u32 regData;
	u16* tableAddr;

	if(index > RTL8368S_VLANMCIDXMAX)
		return -EINVAL;

	tableAddr = (u16*)vlanmconf;
	
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + (index*3);

	retVal = rtl8366_smi_read_reg(smi,regAddr,&regData);
	if(retVal !=  0)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;
			
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 1 + (index*3);

	retVal = rtl8366_smi_read_reg(smi,regAddr,&regData);
	if(retVal !=  0)
		return retVal;
	
	*tableAddr = regData;
	tableAddr ++;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 2 + (index*3);

	retVal = rtl8366_smi_read_reg(smi,regAddr,&regData);
	if(retVal !=  0)
		return retVal;
	
	*tableAddr = regData;

	return 0;
}

static int rtl8368s_getAsicVlan4kEntry(struct rtl8366_smi *smi,struct rtl8368s_user_vlan4kentry_s *vlan4kEntry )
{
	u32 retVal;
	u32 regData;
	u32 vid;
	u32 i;
	u16* tableAddr;
	struct rtl8368s_vlan4kentry_s smiVlan4kentry;

	memset(&smiVlan4kentry, 0x0, sizeof(struct rtl8368s_vlan4kentry_s));

	_rtl8368s_VlanStUser2Smi( vlan4kEntry, &smiVlan4kentry);

	vid = smiVlan4kentry.vid;
	
	if(vid > RTL8368S_VIDMAX)
		return -EINVAL;

	tableAddr = (u16*)&smiVlan4kentry;

	/*write VID first*/
	regData = *tableAddr;	
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_VLAN_TABLE_WRITE_BASE,regData);

	if(retVal !=  0)
		return retVal;

	/*write table access Control word*/
	retVal = rtl8366_smi_write_reg(smi,RTL8368S_TABLE_ACCESS_CTRL_REG,RTL8368S_TABLE_VLAN_READ_CTRL);
	if(retVal !=  0)
		return retVal;

	/*check ASIC command*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8366_smi_read_reg(smi,RTL8368S_TABLE_ACCESS_CTRL_REG,&regData);
		if(retVal !=  0)
			return retVal;

		if(regData &RTL8368S_TABLE_ACCESS_CMD_MSK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return -1;
		}
		else
			break;
	}

	retVal = rtl8366_smi_read_reg(smi,RTL8368S_VLAN_TABLE_READ_BASE,&regData);

	if(retVal !=  0)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;

	retVal = rtl8366_smi_read_reg(smi,RTL8368S_VLAN_TABLE_READ_BASE+1,&regData);

	if(retVal !=  0)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;

	retVal = rtl8366_smi_read_reg(smi,RTL8368S_VLAN_TABLE_READ_BASE+2,&regData);

	if(retVal !=  0)
		return retVal;

	*tableAddr = regData;

	_rtl8368s_VlanStSmi2User( vlan4kEntry, &smiVlan4kentry);

	vlan4kEntry->vid = vid;
	
	return 0;
}

static int rtl8368s_getAsicVlanPortBasedVID(struct rtl8366_smi *smi,enum PORTID port, u32* index)
{
	int retVal;
	u32 regAddr;
	u32 regData;

	/* bits mapping to port vlan control register of port n */
	const u16 bits[8]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0,0x0F00,0xF000 };
	/* bits offset to port vlan control register of port n */
	const u16 bitOff[8] = { 0,4,8,12,0,4,8,12 };
	/* address offset to port vlan control register of port n */
	const u16 addrOff[8]= { 0,0,0,0,1,1,1,1 };


	if(port >=PORT_MAX)
		return -EINVAL;

	regAddr = RTL8368S_PORT_VLAN_CTRL_BASE + addrOff[port];

	retVal = rtl8366_smi_read_reg(smi,regAddr,&regData);
	if(retVal != 0)
		return retVal;

	*index =  (regData & bits[port]) >> bitOff[port];
	return retVal;
}

static int rtl8368s_setAsicMacForceLink(struct rtl8366_smi *smi,enum PORTID port,enum MACLINKMODE force,enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,u32 link,u32 txPause,u32 rxPause)
{
	u32 retVal;
	u32 macData;
	u32 regBits;
	u32 regAddr;
	u32 regData;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return -EINVAL;

	/*not force mode*/
	if(MAC_NORMAL == force)
	{
		retVal = rtl8366_smi_read_reg(smi,RTL8368S_MAC_FORCE_CTRL_REG,&regData);          
		if (retVal !=  0) 
			return retVal;
		
		regData = regData & (~(1<<port));

		retVal = rtl8366_smi_write_reg(smi,RTL8368S_MAC_FORCE_CTRL_REG,regData);
		if (retVal !=  0) 
			return retVal;

		return 0;
	}

	if(speed > SPD_1000M)
		return -EINVAL;
		
	/*prepare force status first*/
	macData = speed;

	if(duplex)
	{
		macData = macData | (duplex<<2);
	}

	if(link)
	{
		macData = macData | (link<<4);
	}

	if(txPause)
	{
		macData = macData | (txPause<<5);
	}
	
	if(rxPause)
	{
		macData = macData | (rxPause<<6);
	}
	
	regBits = 0xFF << (8*(port&0x01));
	macData = macData <<(8*(port&0x01));
	
	/* Set register value */
	regAddr = RTL8368S_PORT_ABILITY_BASE + (port>>1);

	retVal= rtl8368s_setAsicRegBits(smi,regAddr,regBits,macData);
	if (retVal !=  0) 
		return retVal;

	/* Set register value */
	retVal = rtl8366_smi_read_reg(smi,RTL8368S_MAC_FORCE_CTRL_REG,&regData);
	if (retVal !=  0) 
		return retVal;

	regData = regData | (1<<port);

	retVal = rtl8366_smi_write_reg(smi,RTL8368S_MAC_FORCE_CTRL_REG,regData);
	if (retVal !=  0) 
		return retVal;

	return 0;
}

static int rtl8366rb_setEthernetPHY(struct rtl8366_smi *smi,u32 phy, struct rtl8366rb_phyAbility_s *ptr_ability)
{
	u32 phyData;
	u32 phyEnMsk0;
	u32 phyEnMsk4;
	u32 phyEnMsk9;
	
	if(phy > RTL8366RB_PHY_NO_MAX)
		return -1;

	phyEnMsk0 = 0;
	phyEnMsk4 = 0;
	phyEnMsk9 = 0;

	if(1 == ptr_ability->Half_10)
	{
		/*10BASE-TX half duplex capable in reg 4.5*/
		phyEnMsk4 = phyEnMsk4 | (1<<5);

		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
	}

	if(1 == ptr_ability->Full_10)
	{
		/*10BASE-TX full duplex capable in reg 4.6*/
		phyEnMsk4 = phyEnMsk4 | (1<<6);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
		/*Full duplex mode in reg 0.8*/
		phyEnMsk0 = phyEnMsk0 | (1<<8);
		
	}

	if(1 == ptr_ability->Half_100)
	{
		//PRINT("if(1 == ptr_ability->Half_100)\n ");
		/*100BASE-TX half duplex capable in reg 4.7*/
		phyEnMsk4 = phyEnMsk4 | (1<<7);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 | (1<<13);
	}

	if(1 == ptr_ability->Full_100)
	{
		/*100BASE-TX full duplex capable in reg 4.8*/
		phyEnMsk4 = phyEnMsk4 | (1<<8);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 | (1<<13);
		/*Full duplex mode in reg 0.8*/
		phyEnMsk0 = phyEnMsk0 | (1<<8);
	}
		
	if(1 == ptr_ability->Full_1000)
	{
		/*1000 BASE-T FULL duplex capable setting in reg 9.9*/
		phyEnMsk9 = phyEnMsk9 | (1<<9);

		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 | (1<<6);
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
	
		if(ptr_ability->AutoNegotiation != 1)
			return -1;			
	}
	
	if(1 == ptr_ability->AutoNegotiation)
	{
		/*Auto-Negotiation setting in reg 0.12*/
		phyEnMsk0 = phyEnMsk0 | (1<<12);
	}
	if(1 == ptr_ability->AsyFC)
	{
		/*Asymetric flow control in reg 4.11*/
		phyEnMsk4 = phyEnMsk4 | (1<<11);
	}
	if(1 == ptr_ability->FC)
	{
		/*Flow control in reg 4.10*/
		phyEnMsk4 = phyEnMsk4 | (1<<10);
	}

	/*1000 BASE-T control register setting*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData) != 0)
		return -1;

	phyData = (phyData & (~0x0200)) | phyEnMsk9 ;
							 
	if(rtl8366_smi_write_phy_reg(smi,phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData) != 0)
		return -1;

	/*Auto-Negotiation control register setting*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,&phyData) != 0)
		return -1;

	phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
		
	if(rtl8366_smi_write_phy_reg(smi,phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,phyData) != 0)
		return -1;

	/*Control register setting and restart auto*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_CONTROL_REG,&phyData) != 0)
		return -1;

	phyData = (phyData & (~0x3140)) | phyEnMsk0;
	/*If have auto-negotiation capable, then restart auto negotiation*/
	if(ptr_ability->AutoNegotiation)
	{
		phyData = phyData | (1 << 9);
	}
	
	if(rtl8366_smi_write_phy_reg(smi,phy,0,RTL8366RB_PHY_CONTROL_REG,phyData) != 0)
		return -1;

	return 0;
}

static int rtl8366rb_getEthernetPHY(struct rtl8366_smi *smi, u32 phy, struct rtl8366rb_phyAbility_s *ptr_ability)
{
	u32 phyData0;
	u32 phyData4;
	u32 phyData9;
	
	if(phy > RTL8366RB_PHY_NO_MAX)
		return -1;

	/*Control register setting and restart auto*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_CONTROL_REG,&phyData0) != 0)
		return -1;

	/*Auto-Negotiation control register setting*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,&phyData4) != 0)
		return -1;

	/*1000 BASE-T control register setting*/
	if(rtl8366_smi_read_phy_reg(smi,phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData9) != 0)
		return -1;

	if(phyData9 & (1<<9))
		ptr_ability->Full_1000 = 1;
	else
		ptr_ability->Full_1000 = 0;

	if(phyData4 & (1<<11))
		ptr_ability->AsyFC = 1;
	else
		ptr_ability->AsyFC = 0;

	if(phyData4 & (1<<10))
		ptr_ability->FC = 1;
	else
		ptr_ability->FC = 0;
	

	if(phyData4 & (1<<8))
		ptr_ability->Full_100= 1;
	else
		ptr_ability->Full_100= 0;
	
	if(phyData4 & (1<<7))
		ptr_ability->Half_100= 1;
	else
		ptr_ability->Half_100= 0;

	if(phyData4 & (1<<6))
		ptr_ability->Full_10= 1;
	else
		ptr_ability->Full_10= 0;
	
	if(phyData4 & (1<<5))
		ptr_ability->Half_10= 1;
	else
		ptr_ability->Half_10= 0;


	if(phyData0 & (1<<12))
		ptr_ability->AutoNegotiation= 1;
	else
		ptr_ability->AutoNegotiation= 0;

	return 0;
}

static int rtl8366rb_initVlan(struct rtl8366_smi *smi)
{
	u32 i;
	struct rtl8368s_user_vlan4kentry_s vlan4K;
	struct rtl8368s_vlanconfig_s vlanMC;
	struct rtl8366rb_phyAbility_s phy_ability;
	
	/* clear 16 VLAN member configuration */
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		vlanMC.vid = 0;
		vlanMC.priority = 0;
		vlanMC.member = 0;		
		vlanMC.untag = 0;			
		vlanMC.fid = 0;
		if(rtl8368s_setAsicVlanMemberConfig(smi, i, vlanMC) != 0)
			return -1;	

		if(rtl8366rb_getEthernetPHY(smi,i, &phy_ability) != 0)
		{
			printk("rtl8366s_getEthernetPHY fail for %d!!!!\n", i);
			//return -1;
		}
		else
		{
			printk("[%d]phy_abilityAutoNegotiation : %d\n", i, phy_ability.AutoNegotiation);
			printk("[%d]phy_abilityHalf_10 : %d\n", i, phy_ability.Half_10);
			printk("[%d]phy_abilityFull_10 : %d\n", i, phy_ability.Full_10);
			printk("[%d]phy_abilityHalf_100 : %d\n", i, phy_ability.Half_100);
			printk("[%d]phy_abilityFull_100 : %d\n", i, phy_ability.Full_100);
			printk("[%d]phy_abilityFull_1000 : %d\n", i, phy_ability.Full_1000);
			printk("[%d]phy_abilityFC : %d\n", i, phy_ability.FC);
			printk("[%d]phy_abilityAsyFC : %d\n", i, phy_ability.AsyFC);

			phy_ability.AutoNegotiation = 1;
			phy_ability.Half_10 = 1;
			phy_ability.Full_10 = 1;
			phy_ability.Half_100 = 1;
			phy_ability.Full_100 = 1;
			phy_ability.Full_1000 = 1;
			phy_ability.FC = 1;
			phy_ability.AsyFC = 1;
			rtl8366rb_setEthernetPHY(smi,i, &phy_ability);			
			
		}
	}

	/* Set a default VLAN with vid 1 to 4K table for all ports */
   	vlan4K.vid = 1;
 	vlan4K.member = RTL8366RB_PORTMASK;
 	vlan4K.untag = RTL8366RB_PORTMASK;
 	vlan4K.fid = 0;	
	if(rtl8368s_setAsicVlan4kEntry(smi,vlan4K) != 0)
		return -1;	

	/* Also set the default VLAN to 16 member configuration index 0 */
	vlanMC.vid = 1;
	vlanMC.priority = 0;
	vlanMC.member = RTL8366RB_PORTMASK;		
	vlanMC.untag = RTL8366RB_PORTMASK;			
	vlanMC.fid = 0;
	if(rtl8368s_setAsicVlanMemberConfig(smi,0, vlanMC) != 0)
		return -1;	

	/* Set all ports PVID to default VLAN */	
	for(i = 0; i < PORT_MAX; i++)
	{	
		if(rtl8368s_setAsicVlanPortBasedVID(smi,i, 0) != 0)
			return -1;		
	}	

	/* enable VLAN and 4K VLAN */
	if(rtl8368s_setAsicVlan(smi,1)!= 0)
		return -1;	
	if(rtl8368s_setAsicVlan4kTbUsage(smi,1)!= 0)
		return -1;
		
	return 0;
}

static int rtl8366rb_setCPUPort(struct rtl8366_smi *smi,u32 port, u32 noTag)
{
	if(port >= RTL8366RB_PORT_MAX)
		return -EINVAL;

	/* clean CPU port first */
	if(rtl8368s_setAsicCpuPortMask(smi,0x00) != 0)
		return -1;	

	if(rtl8368s_setAsicCpuPortMask(smi,0x1<<port) != 0)
		return -1;		

	if(rtl8368s_setAsicCpuDisableInsTag(smi, noTag) != 0)
		return -1;	

	return 0;
}

static int rtl8366rb_setVlan(struct rtl8366_smi *smi,struct rtl8366rb_vlanConfig_s *ptr_vlancfg)
{
	u32 i;
	struct rtl8368s_user_vlan4kentry_s vlan4K;
	struct rtl8368s_vlanconfig_s vlanMC;	
	
    if(ptr_vlancfg == NULL )
        return -EINVAL;

	/* vid must be 0~4095 */
	if(ptr_vlancfg->vid > RTL8366RB_VIDMAX)
		return -EINVAL;

	if(ptr_vlancfg->mbrmsk > RTL8366RB_PORTMASK)
		return -EINVAL;

	if(ptr_vlancfg->untagmsk > RTL8366RB_PORTMASK)
		return -EINVAL;

		/* fid must be 0~7 */
	if(ptr_vlancfg->fid > RTL8366RB_FIDMAX)
		return -EINVAL;

	/* update 4K table */
   	vlan4K.vid    = ptr_vlancfg->vid;			
 	vlan4K.member = ptr_vlancfg->mbrmsk;
 	vlan4K.untag  = ptr_vlancfg->untagmsk;
 	vlan4K.fid    = ptr_vlancfg->fid;	
	if(rtl8368s_setAsicVlan4kEntry(smi,vlan4K) != 0)
		return -1;
	
	/* 
		Since VLAN entry would be copied from 4K to 16 member configuration while
		setting Port-based VLAN. So also update the entry in 16 member configuration
		if it existed.
	*/
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(smi,i, &vlanMC) != 0)
			return -1;	

		if(ptr_vlancfg->vid == vlanMC.vid)
		{
			vlanMC.member = ptr_vlancfg->mbrmsk;		
			vlanMC.untag  = ptr_vlancfg->untagmsk;			
			vlanMC.fid    = ptr_vlancfg->fid;
			if(rtl8368s_setAsicVlanMemberConfig(smi,i, vlanMC) != 0)
				return -1;	

			return 0;
		}	
	}
	
	return 0;
}

static int rtl8366rb_setVlanPVID(struct rtl8366_smi *smi,u32 port, u32 vid, u32 priority)
{
	u32 i;
	u32 j;
	u32 index;	
	u8 bUsed;	
	struct rtl8368s_user_vlan4kentry_s vlan4K;
	struct rtl8368s_vlanconfig_s vlanMC;	

	if(port >= PORT_MAX)
		return -EINVAL;
	
	/* vid must be 0~4095 */
	if(vid > RTL8366RB_VIDMAX)
		return -EINVAL;

	/* priority must be 0~7 */
	if(priority > RTL8366RB_PRIORITYMAX)
		return -EINVAL;
	
	/* 
		Search 16 member configuration to see if the entry already existed.
		If existed, update the priority and assign the index to the port.
	*/
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(smi,i, &vlanMC) != 0)
			return -1;	

		if(vid == vlanMC.vid)
		{
			vlanMC.priority = priority;		
			if(rtl8368s_setAsicVlanMemberConfig(smi,i, vlanMC) != 0)
				return -1;	
		
			if(rtl8368s_setAsicVlanPortBasedVID(smi,port, i) != 0)
				return -1;	

			return 0;
		}	
	}

	/*
		vid doesn't exist in 16 member configuration. Find an empty entry in 
		16 member configuration, then copy entry from 4K. If 16 member configuration
		are all full, then find an entry which not used by Port-based VLAN and 
		then replace it with 4K. Finally, assign the index to the port.
	*/
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(smi,i, &vlanMC) != 0)
			return -1;	

		if(vlanMC.vid == 0 && vlanMC.member == 0)
		{
			vlan4K.vid = vid;
			if(rtl8368s_getAsicVlan4kEntry(smi,&vlan4K) != 0)
				return -1;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;			
			if(rtl8368s_setAsicVlanMemberConfig(smi,i, vlanMC) != 0)
				return -1;	

			if(rtl8368s_setAsicVlanPortBasedVID(smi,port, i) != 0)
				return -1;	

			return 0;			
		}	
	}	

	/* 16 member configuration is full, found a unused entry to replace */
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		bUsed = 0;	

		for(j = 0; j < PORT_MAX; j++)
		{	
			if(rtl8368s_getAsicVlanPortBasedVID(smi,j, &index) != 0)
				return -1;	

			if(i == index)/*index i is in use by port j*/
			{
				bUsed = 1;
				break;
			}	
		}

		if(bUsed == 0)/*found a unused index, replace it*/
		{
			vlan4K.vid = vid;
			if(rtl8368s_getAsicVlan4kEntry(smi,&vlan4K) != 0)               
				return -1;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;
			vlanMC.stag_idx = 0;
			vlanMC.stag_mbr = 0;			
			if(rtl8368s_setAsicVlanMemberConfig(smi,i, vlanMC) != 0)
				return -1;	

			if(rtl8368s_setAsicVlanPortBasedVID(smi,port, i) != 0)
				return -1;	

			return 0;			
		}
	}	
	
	return -1;
}

static int rtl8366rb_setMac5ForceLink(struct rtl8366_smi *smi,struct rtl8366rb_macConfig_s *ptr_maccfg)
{
	if(ptr_maccfg->speed > SPD_1000M)
		return -EINVAL;

	if(0 != rtl8368s_setAsicMacForceLink(smi,PORT5,1,ptr_maccfg->speed,ptr_maccfg->duplex,ptr_maccfg->link,ptr_maccfg->txPause,ptr_maccfg->rxPause))
		return -1;

	return 0;
}

static int rtl8366rb_switch_reset(struct rtl8366_smi *smi)
{
    struct rtl8366rb_vlanConfig_s  vlan_cfg;
    struct rtl8366rb_macConfig_s   mac5_cfg;

    //if(rtl8366rb_initChip() != 0)   			/* Initial RTL8366RB */
    //    return -1;							/* Chip init done by bootloader */	

    mac5_cfg.force  = MAC_FORCE;
    mac5_cfg.speed  = SPD_1000M;
    mac5_cfg.duplex = FULL_DUPLEX;
    mac5_cfg.link   = 1;
    mac5_cfg.txPause= 1;
    mac5_cfg.rxPause= 1;
//    if(rtl8366rb_setMac5ForceLink(smi,&mac5_cfg) != 0)
//        return -1;								/* Mac5 set by bootloader */


    if(rtl8366rb_setCPUPort(smi,5, 1) != 0)  		/* Set Port 5 as CPU port, third param must always be 1 otherwse Realtek proprietary tag (ethernet length/type 0x8899) is inserted  */
        return -1;
    if(rtl8366rb_initVlan(smi) != 0)    			/* Initial VLAN */
        return -1;
    vlan_cfg.vid        = 1;
    vlan_cfg.mbrmsk     = 0x3E;						/* Member config  Port 5 > 111110 */
    vlan_cfg.untagmsk   = 0x1E;						/* Untag ports 	Port 5 > 011110 */
    vlan_cfg.fid        = 0;
    if(rtl8366rb_setVlan(smi,&vlan_cfg) != 0)    	/* Set LAN, VID = 1, FID = 0 */
        return -1;
    vlan_cfg.vid        = 2;
    vlan_cfg.mbrmsk     = 0x21;						/* Member config  Port 5 > 100001 */
    vlan_cfg.untagmsk   = 0x01;						/* Untag ports  Port 5 > 000001 */
    vlan_cfg.fid        = 1;
    if(rtl8366rb_setVlan(smi,&vlan_cfg) != 0)    	/* Set WAN, VID = 2, FID = 1 */
        return -1;
    if(rtl8366rb_setVlanPVID(smi,0, 2, 0) != 0)
        return -1;
    if(rtl8366rb_setVlanPVID(smi,1, 1, 0) != 0)
        return -1;
    if(rtl8366rb_setVlanPVID(smi,2, 1, 0) != 0)
        return -1;
    if(rtl8366rb_setVlanPVID(smi,3, 1, 0) != 0)
        return -1;
    if(rtl8366rb_setVlanPVID(smi,4, 1, 0) != 0)
        return -1;
    if(rtl8366rb_setVlanPVID(smi,5, 1, 0) != 0)
        return -1;
   
	return 0;        
}


static int rtl8366_smi_mii_write(struct mii_bus *bus, int addr, int reg,
				     u16 val)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 t;
	int err;

	err = rtl8366_smi_write_phy_reg(smi, addr, 0, reg, val);
	/* flush write */
	(void) rtl8366_smi_read_phy_reg(smi, addr, 0, reg, &t);

	return err;
}

static int rtl8366_smi_mii_init(struct rtl8366_smi *smi)
{
	int ret;
	int i;

	smi->mii_bus = (struct mii_bus *)kzalloc(sizeof(struct mii_bus),GFP_KERNEL);
	if (smi->mii_bus == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	spin_lock_init(&smi->lock);
	smi->mii_bus->priv = (void *) smi;
	smi->mii_bus->name = "rtl8366-smi";
	smi->mii_bus->read = rtl8366_smi_mii_read;
	smi->mii_bus->write = rtl8366_smi_mii_write;
	//smi->mii_bus->id = smi->pdev->id;
	snprintf(smi->mii_bus->id, MII_BUS_ID_SIZE, "%s",
		 dev_name(&smi->pdev->dev));
	smi->mii_bus->parent = &smi->pdev->dev;
	smi->mii_bus->phy_mask = ~(0x1f);
	smi->mii_bus->irq = smi->mii_irq;
	for (i = 0; i < PHY_MAX_ADDR; i++)
		smi->mii_irq[i] = PHY_POLL;

	rtl8366_smi_dump_regs(smi);

	ret = mdiobus_register(smi->mii_bus);
	if (ret)
		goto err_free;
	
	rtl8366_smi_dump_regs(smi);

	return 0;

 err_free:
	kfree(smi->mii_bus);
 err:
	return ret;
}

static void rtl8366_smi_mii_cleanup(struct rtl8366_smi *smi)
{
	mdiobus_unregister(smi->mii_bus);
	kfree(smi->mii_bus);
}

static int rtl8366_smi_setup(struct rtl8366_smi *smi)
{
	u32 t;
	u32 chip_id = 0;
	u32 chip2_id = 0;
	u32 chip_ver = 0;
	int ret;
	rtl8366_smi_init(smi);

	ret = rtl8366_smi_read_reg(smi, RTL8366S_CHIP_ID_REG, &chip_id);
		
	if (chip_id != RTL8366S_CHIP_ID_8366)
		ret = rtl8366_smi_read_reg(smi, RTL8366RB_CHIP_ID_REG, &chip_id);

		ret = rtl8366_smi_read_reg(smi, RTL8366RB_CHIP_ID_REG, &chip_id);

	
	if (ret) {
		dev_err(&smi->pdev->dev, "unable to read chip id\n");
		return ret;
	}
	rtl8366_id = chip_id;
	
	switch (chip_id) {
	case RTL8366S_CHIP_ID_8366:
		ret = rtl8366_smi_read_reg(smi, RTL8366S_CHIP_VERSION_CTRL_REG,
				   &chip_ver);
		break;
	case RTL8366RB_CHIP_ID_8366:
		ret = rtl8366_smi_read_reg(smi, RTL8366RB_CHIP_VERSION_CTRL_REG,
				   &chip_ver);
		break;
	default:
		dev_err(&smi->pdev->dev, "unknown chip id (%04x)\n", chip_id);
		return -ENODEV;
	}
	
	rtl8366_ver = chip_id;
	
	if (ret) {
		dev_err(&smi->pdev->dev, "unable to read chip version\n");
		return ret;
	}

	dev_info(&smi->pdev->dev, "RTL%04x ver. %u chip found\n",
		 chip_id, chip_ver & RTL8366S_CHIP_VERSION_MASK);
	
	return 0;
}

static int __init rtl8366rb_smi_probe(struct platform_device *pdev)
{
	static int rtl8366_smi_version_printed;
	struct rtl8366rb_smi_platform_data *pdata;
	struct rtl8366_smi *smi;
	int err;

	if (!rtl8366_smi_version_printed++)
		printk(KERN_NOTICE RTL8366RB_SMI_DRIVER_DESC
		       " version " RTL8366RB_SMI_DRIVER_VER"\n");

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data specified\n");
		err = -EINVAL;
		goto err_out;
	}

	smi = kzalloc(sizeof(struct rtl8366_smi), GFP_KERNEL);
	if (!smi) {
		dev_err(&pdev->dev, "no memory for private data\n");
		err = -ENOMEM;
		goto err_out;
	}

/*	err = gpio_request(pdata->gpio_sda, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "gpio_request failed for %u, err=%d\n",
			pdata->gpio_sda, err);
		goto err_free_smi;
	}

	err = gpio_request(pdata->gpio_sck, dev_name(&pdev->dev));
	if (err) {
		dev_err(&pdev->dev, "gpio_request failed for %u, err=%d\n",
			pdata->gpio_sck, err);
		goto err_free_sda;
	}*/

	smi->pdev = pdev;
	smi->pdata = pdata;
	spin_lock_init(&smi->lock);

	platform_set_drvdata(pdev, smi);

	dev_info(&pdev->dev, "using GPIO pins %u (SDA) and %u (SCK)\n",
		 pdata->gpio_sda, pdata->gpio_sck);

	err = rtl8366_smi_setup(smi);
	if (err)
		goto err_clear_drvdata;

	err = rtl8366_smi_mii_init(smi);
	if (err)
		goto err_clear_drvdata;

	printk("rtl8366rb init vlan \n ");
	err = rtl8366rb_switch_reset(smi);		
	if (err)
		printk("rtl8366rb VLAN ERROR  %04x\n", err);
		
	return 0;

 err_clear_drvdata:
	platform_set_drvdata(pdev, NULL);
 err_free_sda:
 err_free_smi:
	kfree(smi);
 err_out:
	return err;
}

static int rtl8366rb_smi_remove(struct platform_device *pdev)
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi) {
		struct rtl8366rb_smi_platform_data *pdata;

		pdata = pdev->dev.platform_data;

		rtl8366_smi_mii_cleanup(smi);
		platform_set_drvdata(pdev, NULL);
		kfree(smi);
	}

	return 0;
}

int rtl8366rb_phy_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static struct platform_driver rtl8366rb_smi_driver = {
	.driver = {
		.name		= RTL8366RB_SMI_DRIVER_NAME,
		.owner		= THIS_MODULE,
	},
	.probe		= rtl8366rb_smi_probe,
	.remove		= rtl8366rb_smi_remove,
};

static struct phy_driver rtl8366rb_smi_phy_driver = {
	.phy_id		= 0x001cc961,
	.name		= "Realtek RTL8366rb",
	.phy_id_mask	= 0x1ffffff0,
	.features	= PHY_GBIT_FEATURES,
	.config_aneg	= rtl8366rb_phy_config_aneg,
	.read_status	= genphy_read_status,
	.driver		= {
		.owner = THIS_MODULE,
	},
};

static int __init rtl8366rb_smi_init(void)
{
	int ret;

	ret = phy_driver_register(&rtl8366rb_smi_phy_driver);
	if (ret)
		return ret;

	ret = platform_driver_register(&rtl8366rb_smi_driver);
	if (ret)
		goto err_phy_unregister;
		
	return 0;

 err_phy_unregister:
	phy_driver_unregister(&rtl8366rb_smi_phy_driver);
	return ret;
}
module_init(rtl8366rb_smi_init);

static void __exit rtl8366rb_smi_exit(void)
{
	platform_driver_unregister(&rtl8366rb_smi_driver);
	phy_driver_unregister(&rtl8366rb_smi_phy_driver);
}
module_exit(rtl8366rb_smi_exit);

MODULE_DESCRIPTION(RTL8366RB_SMI_DRIVER_DESC);
MODULE_VERSION(RTL8366RB_SMI_DRIVER_VER);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RTL8366RB_SMI_DRIVER_NAME);
