/*
 * admtek.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#define SIOCGMIIREG 0x8948 /* Read MII PHY register.  */
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.  */

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#define ADM_PHY_BASE_REG_NUM 0x20

#define ADM_SW_PHY_PORT0_REG 0x0001
#define ADM_SW_PHY_PORT1_REG 0x0003
#define ADM_SW_PHY_PORT2_REG 0x0005
#define ADM_SW_PHY_PORT3_REG 0x0007
#define ADM_SW_PHY_PORT4_REG 0x0008
#define ADM_SW_PHY_PORT5_REG 0x0009
#define ADM_SW_VLAN_MODE_REG 0x0011

#define ADM_SW_SYS_CTL_REG1 0x000b
#define ADM_SW_VLAN_MAP_REG 0x0013

/*****************/
/*
 * PHY Registers 
 */

/*****************/
#define ADM_PHY_BASE_ADDR 0x0200
#define ADM_PHY_CONTROL 0x00
#define ADM_PHY_STATUS 0x01
#define ADM_PHY_ID1 0x02
#define ADM_PHY_ID2 0x03
#define ADM_AUTONEG_ADVERT 0x04
#define ADM_PHY_AN_STATUS_REG 0x05

/*
 * ADM_PHY_CONTROL fields 
 */
#define ADM_CTRL_SOFTWARE_RESET 0x8000
#define ADM_CTRL_SPEED_100 0x2000
#define ADM_CTRL_AUTONEGOTIATION_ENABLE 0x1000
#define ADM_CTRL_START_AUTONEGOTIATION 0x0200
#define ADM_CTRL_SPEED_FULL_DUPLEX 0x0100

/*
 * ADM_PHY_STATUS fields 
 */
#define ADM_STATUS_LINK_PASS 0x0004
#define ADM_STATUS_AUTO_NEG_DONE 0x0020

/*
 * Auto_Negotiation Link Partner 
 */
#define ADM_PHY_LINK_100FULL 0x0100
#define ADM_PHY_LINK_100HALF 0x0080
#define ADM_PHY_LINK_10FULL 0x0040
#define ADM_PHY_LINK_10HALF 0x0020

/*
 * Advertisement register. 
 */
#define ADM_ADVERTISE_100FULL 0x0100
#define ADM_ADVERTISE_100HALF 0x0080
#define ADM_ADVERTISE_10FULL 0x0040
#define ADM_ADVERTISE_10HALF 0x0020

#define ADM_ADVERTISE_ALL (ADM_ADVERTISE_10HALF | ADM_ADVERTISE_10FULL | ADM_ADVERTISE_100HALF | ADM_ADVERTISE_100FULL)

#define ADM_AUTONEG_DONE(adm_phy_status) (((adm_phy_status) & (ADM_STATUS_AUTO_NEG_DONE)) == (ADM_STATUS_AUTO_NEG_DONE))

/*
 * ADM_PHY_ID1 fields 
 */
#define ADM_PHY_ID1_EXPECTATION 0x002e

#define ADM_CHIP_ID1_EXPECTATION 0x1020
#define ADM_CHIP_ID2_EXPECTATION 0x0007

/*
 * ADM_PHY_ID2 fields 
 */
#define ADM_OUI_LSB_MASK 0xfc00
#define ADM_OUI_LSB_EXPECTATION 0xcc00
#define ADM_OUI_LSB_SHIFT 10
#define ADM_MODEL_NUM_MASK 0x03f0
#define ADM_MODEL_NUM_SHIFT 4
#define ADM_REV_NUM_MASK 0x000f
#define ADM_REV_NUM_SHIFT 0

#if defined(HAVE_ADMTEKNESTEDVLAN)
#define ADM_SW_SIZE_SEL (3 << 7)
#endif

#define ADM_SW_MAC_CLONE_EN 0x10
#define ADM_SW_VLAN_MODE_SEL 0x20

#define ADM_SW_CPU_PORT_NUM \
	0xA000 /* The CPU is
							 * attached to Port 5 
							 */
#define ADM_SW_SPECIAOL_TAG_RX 0x1000
#define ADM_SW_SPECIAOL_TAG_TX 0x0800
#define ADM_SW_SPECIAOL_TAG_PAUSE 0x0400
#define ADM_SW_SMAX_PKT_SIZE 0x0080 /* MAx Packet Length is 1536 */
#define ADM_SW_SYS_CTL3_CONFIG \
	ADM_SW_CPU_PORT_NUM | ADM_SW_SPECIAOL_TAG_RX | ADM_SW_SPECIAOL_TAG_TX | ADM_SW_SPECIAOL_TAG_PAUSE | ADM_SW_SMAX_PKT_SIZE
#define ADM_SW_FLOW_CTRL_EN 0x0001
#define ADM_SW_AUTO_NEGO_EN 0x0002
#define ADM_SW_100M_SPEED_EN 0x0004
#define ADM_SW_FULL_DUP_EN 0x0008
#define ADM_SW_OUT_PKT_TAG_EN 0x0010
#define ADM_SW_PORT_VLAN_ID_1 0x0400
#define ADM_SW_PORT_VLAN_ID_2 0x0800
#define ADM_SW_AUTO_MDIX_EN 0x8000
/*
				        * PORT: 5 4 3 x 2 x 1 x 0 
				        */
#if defined(HAVE_WGT624) || defined(HAVE_NP25G)
#define ADM_SW_LAN_MAP_TAB 0x0155 /* 1 1 1 0 1 0 1 0 0 */
#define ADM_SW_WAN_MAP_TAB 0x0180 /* 1 0 0 0 0 0 0 0 1 */
#define ADM_SW_ALLPORT_MAP_TAB 0x01D5 /* 1 1 1 0 1 0 1 0 1 */

#else
#define ADM_SW_LAN_MAP_TAB 0x01d4 /* 1 1 1 0 1 0 1 0 0 */
#define ADM_SW_WAN_MAP_TAB 0x0101 /* 1 0 0 0 0 0 0 0 1 */
#define ADM_SW_ALLPORT_MAP_TAB 0x01D5 /* 1 1 1 0 1 0 1 0 1 */

#endif
#define ADM_SW_LAN_PORT_CONFIG                                                                                           \
	(ADM_SW_AUTO_MDIX_EN | ADM_SW_PORT_VLAN_ID_1 | ADM_SW_FULL_DUP_EN | ADM_SW_100M_SPEED_EN | ADM_SW_AUTO_NEGO_EN | \
	 ADM_SW_FLOW_CTRL_EN)

#define ADM_SW_WAN_PORT_CONFIG                                                                                           \
	(ADM_SW_AUTO_MDIX_EN | ADM_SW_PORT_VLAN_ID_2 | ADM_SW_FULL_DUP_EN | ADM_SW_100M_SPEED_EN | ADM_SW_AUTO_NEGO_EN | \
	 ADM_SW_FLOW_CTRL_EN)

#define ADM_SW_MII_PORT_CONFIG                                                                                           \
	(ADM_SW_AUTO_MDIX_EN | ADM_SW_OUT_PKT_TAG_EN | ADM_SW_FULL_DUP_EN | ADM_SW_100M_SPEED_EN | ADM_SW_AUTO_NEGO_EN | \
	 ADM_SW_FLOW_CTRL_EN)
#define ADM_SW_LAN_VID (ADM_SW_PORT_VLAN_ID_1 >> 10) & 0xf
#define ADM_SW_WAN_VID (ADM_SW_PORT_VLAN_ID_2 >> 10) & 0xf

/*
 * PHY Addresses 
 */
#define ADM_PHY0_ADDR (ADM_PHY_BASE_ADDR / ADM_PHY_BASE_REG_NUM)
#define ADM_PHY1_ADDR ((ADM_PHY_BASE_ADDR / ADM_PHY_BASE_REG_NUM) + 1)
#define ADM_PHY2_ADDR ((ADM_PHY_BASE_ADDR / ADM_PHY_BASE_REG_NUM) + 2)
#define ADM_PHY3_ADDR ((ADM_PHY_BASE_ADDR / ADM_PHY_BASE_REG_NUM) + 3)
#define ADM_PHY4_ADDR ((ADM_PHY_BASE_ADDR / ADM_PHY_BASE_REG_NUM) + 4)

#define ADM_VLAN_TAG_VALID 0x81
#define ADM_VLAN_TAG_SIZE 4
#define ADM_VLAN_TAG_OFFSET 12 /* After DA & SA */
#define ADM_SPECIAL_TAG_VALID 0x8

/*
 * On AR5312 with CONFIG_VENETDEV==1,
 *   ports 0..3 are LAN ports  (accessed through ae0)
 *   port 4 is the WAN port.   (accessed through ae1)
 * 
 * The phy switch settings in the mvPhyInfo table are set accordingly.
 */
#if defined(HAVE_WGT624) || defined(HAVE_NP25G)
#define ADM_WAN_PORT 4
#define ADM_IS_LAN_PORT(port) ((port) >= 0 && (port) < 4)
#define ADM_IS_WAN_PORT(port) ((port) == ADM_WAN_PORT)
#else
#define ADM_WAN_PORT 0
#define ADM_IS_LAN_PORT(port) ((port) > 0 && (port) < 5)
#define ADM_IS_WAN_PORT(port) ((port) == ADM_WAN_PORT)
#endif

#define ENET_UNIT_DEFAULT 0

#define ADM_LAN_PORT_VLAN 1
#define ADM_WAN_PORT_VLAN 2

#define UINT32 unsigned int
#define UINT16 unsigned short
#define BOOL int
/*
 * Track per-PHY port information.
 */
typedef struct {
	BOOL isEnetPort; /* normal enet port */
	BOOL isPhyAlive; /* last known state of link */
	int ethUnit; /* MAC associated with this phy port */
	UINT32 phyAddr; /* PHY registers associated with this phy
				 * port */
	UINT32 configReg; /* Port config register */
	UINT32 VLANTableSetting; /* Value to be written to VLAN table */
} admPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
#if defined(HAVE_WGT624) || defined(HAVE_NP25G)
admPhyInfo_t admPhyInfo[] = {
	{
		TRUE, /* phy port 0 -- LAN port */
		FALSE, ENET_UNIT_DEFAULT, ADM_PHY0_ADDR, ADM_SW_PHY_PORT0_REG, ADM_LAN_PORT_VLAN /* LAN port */
	},

	{ TRUE, /* phy port 1 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY1_ADDR, ADM_SW_PHY_PORT1_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 2 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY2_ADDR, ADM_SW_PHY_PORT2_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 3 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY3_ADDR, ADM_SW_PHY_PORT3_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 4 -- WAN port */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY4_ADDR, ADM_SW_PHY_PORT4_REG, ADM_WAN_PORT_VLAN },

	{ FALSE, /* phy port 5 -- CPU port (no RJ45 connector) 
				 */
	  TRUE, ENET_UNIT_DEFAULT, 0x00, ADM_SW_PHY_PORT5_REG, ADM_WAN_PORT_VLAN },
};
#else
admPhyInfo_t admPhyInfo[] = {
	{
		TRUE, /* phy port 0 -- WAN port */
		FALSE, ENET_UNIT_DEFAULT, ADM_PHY0_ADDR, ADM_SW_PHY_PORT0_REG, ADM_WAN_PORT_VLAN /* WAN port */
	},

	{ TRUE, /* phy port 1 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY1_ADDR, ADM_SW_PHY_PORT1_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 2 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY2_ADDR, ADM_SW_PHY_PORT2_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 3 -- NC */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY3_ADDR, ADM_SW_PHY_PORT3_REG, ADM_LAN_PORT_VLAN },

	{ TRUE, /* phy port 4 -- LAN port */
	  FALSE, ENET_UNIT_DEFAULT, ADM_PHY4_ADDR, ADM_SW_PHY_PORT4_REG, ADM_LAN_PORT_VLAN },

	{ FALSE, /* phy port 5 -- CPU port (no RJ45 connector) 
				 */
	  TRUE, ENET_UNIT_DEFAULT, 0x00, ADM_SW_PHY_PORT5_REG, ADM_WAN_PORT_VLAN },
};

#endif
#define ADM_GLOBALREGBASE ((UINT32)(PHYS_TO_K1(AR5315_ENET0)))

#define ADM_PHY_MAX (sizeof(admPhyInfo) / sizeof(admPhyInfo[0]))

/*
 * Range of valid PHY IDs is [MIN..MAX] 
 */
#define ADM_ID_MIN 0
#define ADM_ID_MAX (ADM_PHY_MAX - 1)

/*
 * Convenience macros to access myPhyInfo 
 */
#define ADM_IS_ENET_PORT(phyUnit) (admPhyInfo[phyUnit].isEnetPort)
#define ADM_IS_PHY_ALIVE(phyUnit) (admPhyInfo[phyUnit].isPhyAlive)
#define ADM_ETHUNIT(phyUnit) (admPhyInfo[phyUnit].ethUnit)
#define ADM_PHYADDR(phyUnit) (admPhyInfo[phyUnit].phyAddr)
#define ADM_CONFIG_REG(phyUnit) (admPhyInfo[phyUnit].configReg)
#define ADM_VLAN_TABLE_SETTING(phyUnit) (admPhyInfo[phyUnit].VLANTableSetting)

#define ADM_IS_ETHUNIT(phyUnit, ethUnit) (ADM_IS_ENET_PORT(phyUnit) && ADM_ETHUNIT(phyUnit) == (ethUnit))

void setPhy(int addr, int reg, int value)
{
	struct mii_ioctl_data *data;
	struct ifreq iwr;
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0) {
		return;
	}
	(void)strncpy(iwr.ifr_name, "eth0", sizeof("eth0"));
	data = (struct mii_ioctl_data *)&iwr.ifr_data;
	data->phy_id = addr;
	data->reg_num = reg;
	data->val_in = value;
	ioctl(s, SIOCSMIIREG, &iwr);
	close(s);
}

int getPhy(int addr, int reg)
{
	struct mii_ioctl_data *data;
	struct ifreq iwr;
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0) {
		return -1;
	}
	(void)strncpy(iwr.ifr_name, "eth0", sizeof("eth0"));
	data = (struct mii_ioctl_data *)&iwr.ifr_data;
	data->phy_id = addr;
	data->reg_num = reg;
	ioctl(s, SIOCGMIIREG, &iwr);
	close(s);
	return data->val_out;
}

#define ADM_CHIP_ID1_EXPECTATION 0x1020
#define ADM_CHIP_ID2_EXPECTATION 0x0007
#define ADM_PHY_ADDR 0x5

#define PHY_ADDR_SW_PORT 0
#define ADM_SW_AUTO_MDIX_EN 0x8000

void config_vlan(void)
{
	UINT32 phyBase;
	UINT32 phyAddr;
	UINT32 reg = 0;
	int phyUnit;

	/*
	 * Set PVID for the ports, Port 0-3 are LAN ports and 
	 * Port 4 is WAN Port
	 */
	for (phyUnit = 0; phyUnit < ADM_PHY_MAX; phyUnit++) {
		phyAddr = ADM_CONFIG_REG(phyUnit) / ADM_PHY_BASE_REG_NUM;
		reg = getPhy(phyAddr, ADM_CONFIG_REG(phyUnit));

		if (ADM_IS_LAN_PORT(phyUnit)) {
			reg = ADM_SW_LAN_PORT_CONFIG;

		} else if (ADM_IS_WAN_PORT(phyUnit)) {
			reg = ADM_SW_WAN_PORT_CONFIG;

		} else {
			reg |= ADM_SW_OUT_PKT_TAG_EN;
		}
		setPhy(phyAddr, ADM_CONFIG_REG(phyUnit), reg);
	}

	/*
	 * Set up the port memberships for the VLAN Groups 1 and 2 
	 */

	phyAddr = (ADM_SW_VLAN_MAP_REG + ADM_LAN_PORT_VLAN) / ADM_PHY_BASE_REG_NUM;
	setPhy(phyAddr, (ADM_SW_VLAN_MAP_REG + ADM_LAN_PORT_VLAN), ADM_SW_LAN_MAP_TAB);

	phyAddr = (ADM_SW_VLAN_MAP_REG + ADM_WAN_PORT_VLAN) / ADM_PHY_BASE_REG_NUM;
	setPhy(phyAddr, (ADM_SW_VLAN_MAP_REG + ADM_WAN_PORT_VLAN), ADM_SW_WAN_MAP_TAB);

	/*
	 * Put the chip in 802.1q mode 
	 */
	phyAddr = ADM_SW_VLAN_MODE_REG / ADM_PHY_BASE_REG_NUM;

#if defined(HAVE_ADMTEKNESTEDVLAN)
	setPhy(phyAddr, ADM_SW_VLAN_MODE_REG, (ADM_SW_MAC_CLONE_EN | ADM_SW_SIZE_SEL));
#else
	setPhy(phyAddr, ADM_SW_VLAN_MODE_REG, (ADM_SW_MAC_CLONE_EN | ADM_SW_VLAN_MODE_SEL));
#endif
}

static void adm_verifyReady(int ethUnit)
{
	UINT32 phyBase = 0;
	UINT16 phyID1;
	UINT16 phyID2;

	phyID1 = getPhy(0x5, 0x0);
	phyID2 = getPhy(0x5, 0x1);
	if (((phyID1 & 0xfff0) == ADM_CHIP_ID1_EXPECTATION) && (phyID2 == ADM_CHIP_ID2_EXPECTATION)) {
		fprintf(stderr, "Found ADM6996FC! PHYID1 is 0x%x, PHYID2 is 0x%x\n", phyID1, phyID2);
	} else {
		fprintf(stderr, "Couldn't find ADM6996FC!\n, PHYID1 is 0x%x, PHYID2 is 0x%x\n", phyID1, phyID2);
	}
}

void vlan_init(int numports)
{
	int phyUnit;
	UINT16 phyHwStatus;
	UINT16 timeout;
	int liveLinks = 0;
	UINT32 phyBase = 0;
	BOOL foundPhy = FALSE;
	UINT32 phyAddr;
	UINT32 reg = 0;

	/*
	 * Reset PHYs 
	 */
	for (phyUnit = 0; phyUnit < ADM_PHY_MAX; phyUnit++) {
		if (!ADM_IS_ETHUNIT(phyUnit, 0)) {
			continue;
		}

		phyAddr = ADM_PHYADDR(phyUnit);

		setPhy(phyAddr, ADM_PHY_CONTROL, ADM_CTRL_SOFTWARE_RESET);
	}
	/*
	 * After the phy is reset, it takes a little while before
	 * it can respond properly.
	 */
	sleep(1);
	/*
	 * Verify that the switch is what we think it is, and that it's ready 
	 */
	adm_verifyReady(0);

	/*
	 * LAN SETTING: enable Auto-MDIX 
	 */
	phyAddr = ADM_SW_PHY_PORT0_REG / ADM_PHY_BASE_REG_NUM;
	reg = getPhy(phyAddr, ADM_SW_PHY_PORT0_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT0_REG, reg);

	phyAddr = ADM_SW_PHY_PORT1_REG / ADM_PHY_BASE_REG_NUM;
	reg = getPhy(phyAddr, ADM_SW_PHY_PORT1_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT1_REG, reg);

	phyAddr = ADM_SW_PHY_PORT2_REG / ADM_PHY_BASE_REG_NUM;
	getPhy(phyAddr, ADM_SW_PHY_PORT2_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT2_REG, reg);

	phyAddr = ADM_SW_PHY_PORT3_REG / ADM_PHY_BASE_REG_NUM;
	reg = getPhy(phyAddr, ADM_SW_PHY_PORT3_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT3_REG, reg);

	phyAddr = ADM_SW_PHY_PORT4_REG / ADM_PHY_BASE_REG_NUM;
	reg = getPhy(phyAddr, ADM_SW_PHY_PORT4_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT4_REG, reg);

	phyAddr = ADM_SW_PHY_PORT5_REG / ADM_PHY_BASE_REG_NUM;
	reg = getPhy(phyAddr, ADM_SW_PHY_PORT5_REG);
	reg |= ADM_SW_AUTO_MDIX_EN;
	setPhy(phyAddr, ADM_SW_PHY_PORT5_REG, reg);

	/*
	 * See if there's any configuration data for this enet 
	 */
	for (phyUnit = 0; phyUnit < ADM_PHY_MAX; phyUnit++) {
		if (ADM_ETHUNIT(phyUnit) != 0) {
			continue;
		}

		foundPhy = TRUE;
		break;
	}

	if (!foundPhy) {
		return FALSE; /* No PHY's configured for this ethUnit */
	}

	/*
	 * start auto negogiation on each phy 
	 */
	for (phyUnit = 0; phyUnit < ADM_PHY_MAX; phyUnit++) {
		if (!ADM_IS_ETHUNIT(phyUnit, 0)) {
			continue;
		}
		phyAddr = ADM_PHYADDR(phyUnit);

		setPhy(phyAddr, ADM_AUTONEG_ADVERT, ADM_ADVERTISE_ALL);

		setPhy(phyAddr, ADM_PHY_CONTROL, ADM_CTRL_AUTONEGOTIATION_ENABLE | ADM_CTRL_START_AUTONEGOTIATION);
	}

	/*
	 * Wait up to .75 seconds for ALL associated PHYs to finish
	 * autonegotiation.  The only way we get out of here sooner is
	 * if ALL PHYs are connected AND finish autonegotiation.
	 */
	timeout = 15;
	for (phyUnit = 0; (phyUnit < ADM_PHY_MAX) /* && (timeout > 0) */; phyUnit++) {
		if (!ADM_IS_ETHUNIT(phyUnit, 0)) {
			continue;
		}
		for (;;) {
			phyAddr = ADM_PHYADDR(phyUnit);

			phyHwStatus = getPhy(phyAddr, ADM_PHY_STATUS);

			if (ADM_AUTONEG_DONE(phyHwStatus)) {
				fprintf(stderr, "Port %d, Negotiation Success\n", phyUnit);
				break;
			}
			if (timeout == 0) {
				fprintf(stderr, "Port %d, Negotiation timeout\n", phyUnit);
				break;
			}
			if (--timeout == 0) {
				fprintf(stderr, "Port %d, Negotiation timeout\n", phyUnit);
				break;
			}

			usleep(75);
		}
	}

	/*
	 * All PHYs have had adequate time to autonegotiate.
	 * Now initialize software status.
	 *
	 * It's possible that some ports may take a bit longer
	 * to autonegotiate; but we can't wait forever.  They'll
	 * get noticed by mv_phyCheckStatusChange during regular
	 * polling activities.
	 */
	for (phyUnit = 0; phyUnit < ADM_PHY_MAX; phyUnit++) {
		if (adm_phyIsLinkAlive(phyUnit)) {
			liveLinks++;
			ADM_IS_PHY_ALIVE(phyUnit) = TRUE;
		} else {
			ADM_IS_PHY_ALIVE(phyUnit) = FALSE;
		}

		fprintf(stderr, "adm_phySetup: eth%d phy%d: Phy Status=%4.4x\n", 0, phyUnit,
			getPhy(ADM_PHYADDR(phyUnit), ADM_PHY_STATUS));
	}

	config_vlan();
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		MAC_ADD(macaddr);
		set_hwaddr("vlan2", macaddr);
	}
}

int adm_phyIsLinkAlive(int phyUnit)
{
	unsigned short phyHwStatus;
	unsigned int phyBase;
	unsigned int phyAddr;

	phyAddr = ADM_PHYADDR(phyUnit);

	phyHwStatus = getPhy(phyAddr, ADM_PHY_STATUS);

	if (phyHwStatus & ADM_STATUS_LINK_PASS) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void start_vlantest(void)
{
	vlan_init(0);
}

/*
 * void main(int argc,char *argv[]) { config_vlan (); config_bw(); }
 */
