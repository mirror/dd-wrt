/*
 * icplus.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.       */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.      */

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>


#define VLAN_MAXVID	15	/* Max. VLAN ID supported/allowed */

#define ADM_PHY0_ADDR   0x10
#define ADM_PHY1_ADDR   0x11
#define ADM_PHY2_ADDR   0x12
#define ADM_PHY3_ADDR   0x13
#define ADM_PHY4_ADDR   0x14

#define ADM_VLAN_TAG_VALID                   0x81
#define ADM_VLAN_TAG_SIZE                    4
#define ADM_VLAN_TAG_OFFSET                  12   /* After DA & SA */

/*****************/
/* PHY Registers */
/*****************/
#define ADM_PHY_CONTROL                 0
#define ADM_PHY_STATUS                  1
#define ADM_PHY_ID1                     2
#define ADM_PHY_ID2                     3
#define ADM_AUTONEG_ADVERT              4
#define ADM_LINK_PARTNER_ABILITY        5
#define ADM_AUTONEG_EXPANSION           6


/* ADM_PHY_CONTROL fields */
#define ADM_CTRL_SOFTWARE_RESET                    0x8000
#define ADM_CTRL_SPEED_100                         0x2000
#define ADM_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ADM_CTRL_START_AUTONEGOTIATION             0x0200
#define ADM_CTRL_SPEED_FULL_DUPLEX                 0x0100

/* Phy status fields */
#define ADM_STATUS_AUTO_NEG_DONE                   0x0020
#define ADM_STATUS_LINK_PASS                       0x0004

#define ADM_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (ADM_STATUS_AUTO_NEG_DONE)) ==                    \
        (ADM_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define ADM_LINK_100BASETX_FULL_DUPLEX       0x0100
#define ADM_LINK_100BASETX                   0x0080
#define ADM_LINK_10BASETX_FULL_DUPLEX        0x0040
#define ADM_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define ADM_ADVERTISE_100FULL                0x0100
#define ADM_ADVERTISE_100HALF                0x0080  
#define ADM_ADVERTISE_10FULL                 0x0040  
#define ADM_ADVERTISE_10HALF                 0x0020  

#define ADM_ADVERTISE_ALL (ADM_ADVERTISE_10HALF | ADM_ADVERTISE_10FULL | \
                       ADM_ADVERTISE_100HALF | ADM_ADVERTISE_100FULL)
               

int adm_phySetup(int ethUnit);
int adm_phyIsUp(int ethUnit);
int adm_phyIsFullDuplex(int ethUnit);
int adm_phySpeed(int ethUnit);


/* PHY selections and access functions */

typedef enum {
    PHY_SRCPORT_INFO, 
    PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum {
    PHY_SRCPORT_NONE,
    PHY_SRCPORT_VLANTAG, 
    PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#define DRV_MSG(x,a,b,c,d,e,f)
#define DRV_PRINT(DBG_SW,X)

#define ADM_LAN_PORT_VLAN          1
#define ADM_WAN_PORT_VLAN          2

#define ENET_UNIT_DEFAULT 1

#define TRUE    1
#define FALSE   0

/*
 * Track per-PHY port information.
 */
typedef struct {
    int   isEnetPort;       /* normal enet port */
    int   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    unsigned int phyBase;
    unsigned int phyAddr;          /* PHY registers associated with this phy port */
    unsigned int VLANTableSetting; /* Value to be written to VLAN table */
} ipPhyInfo_t;

#define ADM_PHY0_ADDR   0x10
#define ADM_PHY1_ADDR   0x11
#define ADM_PHY2_ADDR   0x12
#define ADM_PHY3_ADDR   0x13
#define ADM_PHY4_ADDR   0x14

#define P0_TXL  0xcc
#define P5_TXL  0xdc

#define P0_TXH  0xcd
#define P5_TXH  0xdd

#define P0_TXBL 0xde
#define P5_TXBL 0xee

#define P0_TXBH 0xdf
#define P5_TXBH 0xef

#define P0_RXL  0xac
#define P5_RXL  0xb8

#define P0_RXH  0xa9
#define P5_RXH  0xb9

#define P0_ERRL 0x102
#define P5_ERRL 0x112

#define P0_ERRH 0x103
#define P5_ERRH 0x113

/*
 * Per-PHY information, indexed by PHY unit number.
 */
ipPhyInfo_t ipPhyInfo[] = {
    /*
     * On AP30/AR5312, all PHYs are associated with MAC0.
     * AP30/AR5312's MAC1 isn't used for anything.
     * CONFIG_VENETDEV==1 (router) configuration:
     *    Ports 0,1,2, and 3 are "LAN ports"
     *    Port 4 is a WAN port
     *    Port 5 connects to MAC0 in the AR5312
     * CONFIG_VENETDEV==0 (bridge) configuration:
     *    Ports 0,1,2,3,4 are "LAN ports"
     *    Port 5 connects to the MAC0 in the AR5312
     */
    {TRUE,   /* phy port 0 -- LAN port 0 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY0_ADDR,
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY1_ADDR,
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY2_ADDR, 
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_DEFAULT,
     0,
     ADM_PHY3_ADDR, 
     ADM_LAN_PORT_VLAN
    },

    {TRUE,   /* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     0,
     0,
     ADM_PHY4_ADDR, 
     ADM_LAN_PORT_VLAN   /* Send to all ports */
    },

    {FALSE,  /* phy port 5 -- CPU port (no RJ45 connector) */
     TRUE,
     ENET_UNIT_DEFAULT,
     0,
     0x00, 
     ADM_LAN_PORT_VLAN    /* Send to all ports */
    },
};

char *bcmstrstr (char *haystack, char *needle)
{
  int len, nlen;
  int i;

  if ((haystack == NULL) || (needle == NULL))
    return (haystack);

  nlen = strlen (needle);
  len = strlen (haystack) - nlen + 1;

  for (i = 0; i < len; i++)
    if (memcmp (needle, &haystack[i], nlen) == 0)
      return (&haystack[i]);
  return (NULL);
}

#define ADM_GLOBALREGBASE    0

//#define ADM_PHY_MAX (sizeof(ipPhyInfo) / sizeof(ipPhyInfo[0]))
#define ADM_PHY_MAX 5

/* Range of valid PHY IDs is [MIN..MAX] */
#define ADM_ID_MIN 0
#define ADM_ID_MAX (ADM_PHY_MAX-1)

/* Convenience macros to access myPhyInfo */
#define ADM_IS_ENET_PORT(phyUnit) (ipPhyInfo[phyUnit].isEnetPort)
#define ADM_IS_PHY_ALIVE(phyUnit) (ipPhyInfo[phyUnit].isPhyAlive)
#define ADM_ETHUNIT(phyUnit) (ipPhyInfo[phyUnit].ethUnit)
#define ADM_PHYBASE(phyUnit) (ipPhyInfo[phyUnit].phyBase)
#define ADM_PHYADDR(phyUnit) (ipPhyInfo[phyUnit].phyAddr)
#define ADM_VLAN_TABLE_SETTING(phyUnit) (ipPhyInfo[phyUnit].VLANTableSetting)


#define ADM_IS_ETHUNIT(phyUnit, ethUnit) \
            (ADM_IS_ENET_PORT(phyUnit) &&        \
            ADM_ETHUNIT(phyUnit) == (ethUnit))

/* Forward references */
int       adm_phyIsLinkAlive(int phyUnit);
void       adm_get_counters(void);





void
setPhy (int addr, int reg, int value)
{
  struct mii_ioctl_data *data;
  struct ifreq iwr;
  int s = socket (AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    {
      return;
    }
  (void) strncpy (iwr.ifr_name, "eth0", sizeof ("eth0"));
  data = (struct mii_ioctl_data *) &iwr.ifr_data;
  data->phy_id = addr;
  data->reg_num = reg;
  data->val_in = value;
  ioctl (s, SIOCSMIIREG, &iwr);
  close (s);
}

int
getPhy (int addr, int reg)
{
  struct mii_ioctl_data *data;
  struct ifreq iwr;
  int s = socket (AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    {
      return -1;
    }
  (void) strncpy (iwr.ifr_name, "eth0", sizeof ("eth0"));
  data = (struct mii_ioctl_data *) &iwr.ifr_data;
  data->phy_id = addr;
  data->reg_num = reg;
  ioctl (s, SIOCGMIIREG, &iwr);
  close (s);
  return data->val_out;
}

#define ADM_CHIP_ID1_EXPECTATION                   0x1020
#define ADM_CHIP_ID2_EXPECTATION                   0x0007
#define ADM_PHY_ADDR                               0x5

#define PHY_ADDR_SW_PORT 0
#define ADM_SW_AUTO_MDIX_EN     0x8000


void config_vlan(void)
{

	/* Port configuration */
	struct {
		unsigned char addr;	/* port configuration register */
		unsigned short vlan;	/* vlan port mapping */
		unsigned char tagged;	/* output tagging */
		unsigned char cpu;	/* cpu port? 1 - yes, 0 - no */
		unsigned short pvid;	/* cpu port pvid */
	} port_cfg_tab[] = {
		{1, 1<<0, 0, 0, -1},
		{3, 1<<2, 0, 0, -1},
		{5, 1<<4, 0, 0, -1},
		{7, 1<<6, 0, 0, -1},
		{8, 1<<7, 0, 0, -1},
		{9, 1<<8, 1, 1, -1}	/* output tagging for linux... */
	};
	/* Vlan ports bitmap */
	struct {
		unsigned char addr;	/* vlan port map register */
	} vlan_cfg_tab[] = {
		{0x13},
		{0x14},
		{0x15},
		{0x16},
		{0x17},
		{0x18},
		{0x19},
		{0x1a},
		{0x1b},
		{0x1c},
		{0x1d},
		{0x1e},
		{0x1f},
		{0x20},
		{0x21},
		{0x22}
	};
	unsigned short vid, i;

setPhy(0,0x11,0xff30);


	for (vid = 0; vid < 16; vid ++) {
		char port[] = "XXXX", *next, *ports, *cur;
		char vlanports[] = "vlanXXXXports";
		uint16 vlan_map = 0;
		int port_num, len;
		uint16 port_cfg;

		/* no members if VLAN id is out of limitation */
		if (vid > VLAN_MAXVID)
			goto vlan_setup;

		/* get nvram port settings */
		sprintf(vlanports, "vlan%dports", vid);
		ports = nvram_get(vlanports);

		/* disable this vlan if not defined */
		if (!ports)
			goto vlan_setup;

		/*
		* port configuration register (0x01, 0x03, 0x05, 0x07, 0x08, 0x09):
		*   input/output tagging, pvid, auto mdix, auto negotiation, ...
		* cpu port needs special handing to support pmon/cfe/linux...
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
			port_num = atoi(port);
			if (port_num >= sizeof(port_cfg_tab) / sizeof(port_cfg_tab[0])) {
				fprintf(stderr,"number %d is out of range\n", port_num);
				continue;
			}

			/* build vlan port map */
			vlan_map |= port_cfg_tab[port_num].vlan;

			/* cpu port needs special care */
			if (port_cfg_tab[port_num].cpu) {
				/* cpu port's default VLAN is lan! */
				if (strchr(port, '*'))
					port_cfg_tab[port_num].pvid = vid;
				/* will be done later */
				continue;
			}

			/* configure port */
			port_cfg = 0x8000 |	/* auto mdix */
				(vid << 10) | 	/* pvid */
				0x000f;		/* full duplex, 100Mbps, auto neg, flow ctrl */
			setPhy(0,port_cfg_tab[port_num].addr, port_cfg);
		}
vlan_setup:
		/* vlan port map register (0x13 - 0x22) */
		setPhy(0,vlan_cfg_tab[vid].addr, vlan_map);
	}

	/* cpu port config: auto mdix, pvid, output tagging, ... */
	for (i = 0; i < sizeof(port_cfg_tab)/sizeof(port_cfg_tab[0]); i ++) {
		uint16 tagged, pvid;
		uint16 port_cfg;

		/* cpu port only */
		if (port_cfg_tab[i].cpu == 0 || port_cfg_tab[i].pvid == 0xffff)
			continue;

		/* configure port */
		tagged = port_cfg_tab[i].tagged ? 1 : 0;
		pvid = port_cfg_tab[i].pvid;
		port_cfg = 0x8000 |	/* auto mdix */
			(pvid << 10) | 	/* pvid */
			(tagged << 4) |	/* output tagging */
			0x000f;		/* full duplex, 100Mbps, auto neg, flow ctrl */
		setPhy(0, port_cfg_tab[i].addr, port_cfg);
	}

}

void
vlan_init (int numports)
{
    int ethUnit=0;
    int     phyUnit;
    unsigned short  phyHwStatus;
    unsigned short  timeout;
    int     liveLinks = 0;
    unsigned int  phyBase = 0;
    int    foundPhy = FALSE;
    unsigned int  phyAddr;
    
    /* Reset PHYs*/
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyAddr = ADM_PHYADDR(phyUnit);
        setPhy(phyAddr, ADM_PHY_CONTROL,
                    ADM_CTRL_SOFTWARE_RESET);
    }
    /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    sleep(1);


    /* start auto negogiation on each phy */
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        phyAddr = ADM_PHYADDR(phyUnit);
        
        setPhy(phyAddr, ADM_AUTONEG_ADVERT,
                                        ADM_ADVERTISE_ALL);

        setPhy(phyAddr, ADM_PHY_CONTROL,
                    ADM_CTRL_AUTONEGOTIATION_ENABLE | ADM_CTRL_START_AUTONEGOTIATION);
    }

    /*
     * Wait up to .75 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=5;
    for (phyUnit=0; (phyUnit < ADM_PHY_MAX) /*&& (timeout > 0) */; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }
        for (;;) {
            phyAddr = ADM_PHYADDR(phyUnit);

            phyHwStatus = getPhy(phyAddr, ADM_PHY_STATUS);

            if (ADM_AUTONEG_DONE(phyHwStatus)) {
            fprintf(stderr,"Port %d, Neg Success\n", phyUnit);
                break;
            }
            if (timeout == 0) {
                fprintf(stderr,"Port %d, Negogiation timeout\n", phyUnit);
                break;
            }
            if (--timeout == 0) {
                fprintf(stderr,"Port %d, Negogiation timeout\n", phyUnit);
                break;
            }
	    sleep(1);
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
    for (phyUnit=0; phyUnit < ADM_PHY_MAX; phyUnit++) {
        if (!ADM_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }

        if (adm_phyIsLinkAlive(phyUnit)) {
            liveLinks++;
            ADM_IS_PHY_ALIVE(phyUnit) = TRUE;
        } else {
            ADM_IS_PHY_ALIVE(phyUnit) = FALSE;
        }

        fprintf(stderr,"eth%d: Phy Status=%4.4x\n",ethUnit, getPhy(ADM_PHYADDR(phyUnit),ADM_PHY_STATUS));
    }

    /*
     * XXX
     */
nvram_set("vlan0ports","0 5*");
nvram_set("vlan1ports","1 2 3 4 5");
  config_vlan();
  eval ("/sbin/vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
  eval ("/sbin/vconfig", "add", "eth0", "0");
  eval ("/sbin/vconfig", "add", "eth0", "1");
}

int
adm_phyIsLinkAlive(int phyUnit)
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

void
start_vlantest (void)
{
  config_vlan ();
}
