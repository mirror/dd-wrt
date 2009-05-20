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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>

/*****************/
/*
 * PHY Registers 
 */

/*****************/
#define IP_PHY_CONTROL                 0
#define IP_PHY_STATUS                  1
#define IP_PHY_ID1                     2
#define IP_PHY_ID2                     3
#define IP_AUTONEG_ADVERT              4
#define IP_LINK_PARTNER_ABILITY        5
#define IP_AUTONEG_EXPANSION           6

/*
 * IP_PHY_CONTROL fields 
 */
#define IP_CTRL_SOFTWARE_RESET                    0x8000
#define IP_CTRL_SPEED_100                         0x2000
#define IP_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define IP_CTRL_START_AUTONEGOTIATION             0x0200
#define IP_CTRL_SPEED_FULL_DUPLEX                 0x0100

/*
 * Phy status fields 
 */
#define IP_STATUS_AUTO_NEG_DONE                   0x0020
#define IP_STATUS_LINK_PASS                       0x0004

#define IP_AUTONEG_DONE(ip_phy_status)                   \
    (((ip_phy_status) &                                  \
        (IP_STATUS_AUTO_NEG_DONE)) ==                    \
        (IP_STATUS_AUTO_NEG_DONE))

/*
 * ICPLUS_PHY_ID1 fields 
 */
#define IP_PHY_ID1_EXPECTATION                    0x0243	/* OUI >> 6 */

/*
 * ICPLUS_PHY_ID2 fields 
 */
#define IP_OUI_LSB_MASK                           0xfc00
#define IP_OUI_LSB_EXPECTATION                    0x0c00
#define IP_OUI_LSB_SHIFT                              10
#define IP_MODEL_NUM_MASK                         0x03f0
#define IP_MODEL_NUM_SHIFT                             4
#define IP_REV_NUM_MASK                           0x000f
#define IP_REV_NUM_SHIFT                               0

/*
 * Link Partner ability 
 */
#define IP_LINK_100BASETX_FULL_DUPLEX       0x0100
#define IP_LINK_100BASETX                   0x0080
#define IP_LINK_10BASETX_FULL_DUPLEX        0x0040
#define IP_LINK_10BASETX                    0x0020

/*
 * Advertisement register. 
 */
#define IP_ADVERTISE_100FULL                0x0100
#define IP_ADVERTISE_100HALF                0x0080
#define IP_ADVERTISE_10FULL                 0x0040
#define IP_ADVERTISE_10HALF                 0x0020

#define IP_ADVERTISE_ALL (IP_ADVERTISE_10HALF | IP_ADVERTISE_10FULL | \
                       IP_ADVERTISE_100HALF | IP_ADVERTISE_100FULL)

#define IP_VLAN_TAG_VALID                   0x81
#define IP_VLAN_TAG_SIZE                    4
#define IP_VLAN_TAG_OFFSET                  12	/* After DA & SA */
#define IP_SPECIAL_TAG_VALID                0x81

/****************************/
/*
 * Global Control Registers 
 */

/****************************/
/*
 * IP Global register doesn't have names based on functionality hence has to
 * live with this names for now 
 */
#define IP_GLOBAL_PHY29_18_REG  18
#define IP_GLOBAL_PHY29_19_REG  19
#define IP_GLOBAL_PHY29_20_REG  20
#define IP_GLOBAL_PHY29_21_REG  21
#define IP_GLOBAL_PHY29_22_REG  22
#define IP_GLOBAL_PHY29_23_REG  23
#define IP_GLOBAL_PHY29_24_REG  24
#define IP_GLOBAL_PHY29_25_REG  25
#define IP_GLOBAL_PHY29_26_REG  26
#define IP_GLOBAL_PHY29_27_REG  27
#define IP_GLOBAL_PHY29_28_REG  28
#define IP_GLOBAL_PHY29_29_REG  29
#define IP_GLOBAL_PHY29_30_REG  30
#define IP_GLOBAL_PHY29_31_REG  31

#define IP_GLOBAL_PHY30_0_REG   0
#define IP_GLOBAL_PHY30_1_REG   1
#define IP_GLOBAL_PHY30_2_REG   2
#define IP_GLOBAL_PHY30_3_REG   3
#define IP_GLOBAL_PHY30_4_REG   4
#define IP_GLOBAL_PHY30_5_REG   5
#define IP_GLOBAL_PHY30_6_REG   6
#define IP_GLOBAL_PHY30_7_REG   7
#define IP_GLOBAL_PHY30_8_REG   8
#define IP_GLOBAL_PHY30_9_REG   9
#define IP_GLOBAL_PHY30_10_REG  10
#define IP_GLOBAL_PHY30_11_REG  11
#define IP_GLOBAL_PHY30_12_REG  12
#define IP_GLOBAL_PHY30_13_REG  13
#define IP_GLOBAL_PHY30_16_REG  16
#define IP_GLOBAL_PHY30_17_REG  17
#define IP_GLOBAL_PHY30_18_REG  18
#define IP_GLOBAL_PHY30_20_REG  20
#define IP_GLOBAL_PHY30_21_REG  21
#define IP_GLOBAL_PHY30_22_REG  22
#define IP_GLOBAL_PHY30_23_REG  23
#define IP_GLOBAL_PHY30_24_REG  24
#define IP_GLOBAL_PHY30_25_REG  25
#define IP_GLOBAL_PHY30_26_REG  26
#define IP_GLOBAL_PHY30_27_REG  27
#define IP_GLOBAL_PHY30_28_REG  28
#define IP_GLOBAL_PHY30_29_REG  29
#define IP_GLOBAL_PHY30_30_REG  30
#define IP_GLOBAL_PHY30_31_REG  31

#define IP_GLOBAL_PHY31_0_REG   0
#define IP_GLOBAL_PHY31_1_REG   1
#define IP_GLOBAL_PHY31_2_REG   2
#define IP_GLOBAL_PHY31_3_REG   3
#define IP_GLOBAL_PHY31_4_REG   4
#define IP_GLOBAL_PHY31_5_REG   5
#define IP_GLOBAL_PHY31_6_REG   6

#define IP_GLOBAL_PHY29_31_REG  31

#define IP_VLAN0_OUTPUT_PORT_MASK_S     0
#define IP_VLAN1_OUTPUT_PORT_MASK_S     8
#define IP_VLAN2_OUTPUT_PORT_MASK_S     0
#define IP_VLAN3_OUTPUT_PORT_MASK_S     8

/*
 * Masks and shifts for 29.23 register 
 */
#define IP_PORTX_ADD_TAG_S               11
#define IP_PORTX_REMOVE_TAG_S            6
#define IP_PORT5_ADD_TAG_S               1
#define IP_PORT5_REMOVE_TAG_S            0

/*
 * 30.9   Definitions 
 */
#define TAG_VLAN_ENABLE         0x0080
#define VID_INDX_SEL_M          0x0070
#define VID_INDX_SEL_S          4

/*
 * PHY Addresses 
 */
#define IP_PHY0_ADDR    0
#define IP_PHY1_ADDR    1
#define IP_PHY2_ADDR    2
#define IP_PHY3_ADDR    3
#define IP_PHY4_ADDR    4

#define IP_GLOBAL_PHY29_ADDR    29
#define IP_GLOBAL_PHY30_ADDR    30
#define IP_GLOBAL_PHY31_ADDR    31

typedef enum
{
    PHY_SRCPORT_INFO,
    PHY_PORTINFO_SIZE,
} PHY_CAP_TYPE;

typedef enum
{
    PHY_SRCPORT_NONE,
    PHY_SRCPORT_VLANTAG,
    PHY_SRCPORT_TRAILER,
} PHY_SRCPORT_TYPE;

#define IP_WAN_PORT          4
#define IP_IS_LAN_PORT(port) ((port) <  IP_WAN_PORT)
#define IP_IS_WAN_PORT(port) ((port) == IP_WAN_PORT)

#define IP_LAN_PORT_VLAN          1
#define IP_WAN_PORT_VLAN          2

#define ENET_UNIT_DEFAULT 0

/*
 * Track per-PHY port information.
 */
typedef struct
{
    int isEnetPort;		/* normal enet port */
    int isPhyAlive;		/* last known state of link */
    int ethUnit;		/* MAC associated with this phy port */
    unsigned int phyAddr;	/* PHY registers associated with this phy
				 * port */
    unsigned int VLANTableSetting;	/* Value to be written to VLAN table */
} ipPhyInfo_t;

#define TRUE 1
#define FALSE 0

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
    {TRUE,			/* phy port 0 -- LAN port 0 */
     FALSE,
     ENET_UNIT_DEFAULT,
     IP_PHY0_ADDR,
     IP_LAN_PORT_VLAN},

    {TRUE,			/* phy port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_DEFAULT,
     IP_PHY1_ADDR,
     IP_LAN_PORT_VLAN},

    {TRUE,			/* phy port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_DEFAULT,
     IP_PHY2_ADDR,
     IP_LAN_PORT_VLAN},

    {TRUE,			/* phy port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_DEFAULT,
     IP_PHY3_ADDR,
     IP_LAN_PORT_VLAN},

    {TRUE,			/* phy port 4 -- WAN port or LAN port 4 */
     FALSE,
     ENET_UNIT_DEFAULT,
     IP_PHY4_ADDR,
     IP_WAN_PORT_VLAN		/* Send to all ports */
     },

    {FALSE,			/* phy port 5 -- CPU port (no RJ45 connector) 
				 */
     TRUE,
     ENET_UNIT_DEFAULT,
     0x00,
     IP_LAN_PORT_VLAN		/* Send to all ports */
     },

};

#define IP_GLOBALREGBASE    ((UINT32) (PHYS_TO_K1(AR531X_ENET0)))

#define IP_PHY_MAX (sizeof(ipPhyInfo) / sizeof(ipPhyInfo[0]))

/*
 * Range of valid PHY IDs is [MIN..MAX] 
 */
#define IP_ID_MIN 0
#define IP_ID_MAX (IP_PHY_MAX-1)

/*
 * Convenience macros to access myPhyInfo 
 */
#define IP_IS_ENET_PORT(phyUnit) (ipPhyInfo[phyUnit].isEnetPort)
#define IP_IS_PHY_ALIVE(phyUnit) (ipPhyInfo[phyUnit].isPhyAlive)
#define IP_ETHUNIT(phyUnit) (ipPhyInfo[phyUnit].ethUnit)
#define IP_PHYBASE(phyUnit) (ipPhyInfo[phyUnit].phyBase)
#define IP_PHYADDR(phyUnit) (ipPhyInfo[phyUnit].phyAddr)
#define IP_VLAN_TABLE_SETTING(phyUnit) (ipPhyInfo[phyUnit].VLANTableSetting)

void setPhy( int addr, int reg, int value )
{
    struct mii_ioctl_data *data;
    struct ifreq iwr;
    int s = socket( AF_INET, SOCK_DGRAM, 0 );

    if( s < 0 )
    {
	return;
    }
    ( void )strncpy( iwr.ifr_name, "eth0", sizeof( "eth0" ) );
    data = ( struct mii_ioctl_data * )&iwr.ifr_data;
    data->phy_id = addr;
    data->reg_num = reg;
    data->val_in = value;
    ioctl( s, SIOCSMIIREG, &iwr );
    close( s );
}

int getPhy( int addr, int reg )
{
    struct mii_ioctl_data *data;
    struct ifreq iwr;
    int s = socket( AF_INET, SOCK_DGRAM, 0 );

    if( s < 0 )
    {
	return -1;
    }
    ( void )strncpy( iwr.ifr_name, "eth0", sizeof( "eth0" ) );
    data = ( struct mii_ioctl_data * )&iwr.ifr_data;
    data->phy_id = addr;
    data->reg_num = reg;
    ioctl( s, SIOCGMIIREG, &iwr );
    close( s );
    return data->val_out;
}

void vlan_init( int portmask )
{
    int phyUnit;
    unsigned int phyBase;
    unsigned int phyReg;
    unsigned int phyAddr;
    int i;
    int numports = 5;

    for( i = 0; i < numports - 1; i++ )	// last one will be wan port
    {
	ipPhyInfo[i].VLANTableSetting = IP_LAN_PORT_VLAN;
    }
    ipPhyInfo[i++].VLANTableSetting = IP_WAN_PORT_VLAN;
    ipPhyInfo[i].VLANTableSetting = IP_WAN_PORT_VLAN;
    ipPhyInfo[i].isEnetPort = FALSE;
    ipPhyInfo[i].isPhyAlive = TRUE;
    ipPhyInfo[i++].phyAddr = 0x0;

    numports = i;
    fprintf( stderr, "Reset ICPLUS Phy\n" );
    for( phyUnit = 0; phyUnit < numports; phyUnit++ )
    {
	if( ( ( 1 << phyUnit ) & portmask ) )
	{
	    phyAddr = IP_PHYADDR( phyUnit );
	    setPhy( phyAddr, IP_PHY_CONTROL, IP_CTRL_SOFTWARE_RESET );
	}
    }
    sleep( 1 );
    fprintf( stderr, "Start Autonegotiation\n" );
    for( phyUnit = 0; phyUnit < numports; phyUnit++ )
    {

	if( ( ( 1 << phyUnit ) & portmask ) )
	{
	    phyAddr = IP_PHYADDR( phyUnit );

	    setPhy( phyAddr, IP_AUTONEG_ADVERT, IP_ADVERTISE_ALL );
	    setPhy( phyAddr, IP_PHY_CONTROL,
		    IP_CTRL_AUTONEGOTIATION_ENABLE |
		    IP_CTRL_START_AUTONEGOTIATION );
	}
    }
    int timeout = 5;

    for( phyUnit = 0; ( phyUnit < numports ); phyUnit++ )
    {
	if( ( ( 1 << phyUnit ) & portmask ) )
	{
	    for( ;; )
	    {
		phyAddr = IP_PHYADDR( phyUnit );

		int phyHwStatus = getPhy( phyAddr, IP_PHY_STATUS );

		if( IP_AUTONEG_DONE( phyHwStatus ) )
		{
		    fprintf( stderr, "Port %d, Neg Success\n", phyUnit );
		    break;
		}
		if( timeout == 0 )
		{
		    fprintf( stderr, "Port %d, Negogiation timeout\n",
			     phyUnit );
		    break;
		}
		if( --timeout == 0 )
		{
		    fprintf( stderr, "Port %d, Negogiation timeout\n",
			     phyUnit );
		    break;
		}
		usleep( 150 );
	    }
	}
    }

    fprintf( stderr, "Setup VLANS\n" );
    /*
     * setPhy(29,24,0); setPhy(29,25,0); setPhy(29,26,0); setPhy(29,27,0);
     * setPhy(29,28,2); setPhy(29,30,0); setPhy(29,23,0x07c2);
     * setPhy(30,1,0x002f); setPhy(30,2,0x0030); setPhy(30,9,0x1089);
     */
    unsigned int phy1Reg=0;
    unsigned int phy2Reg=0;
    unsigned int phy23Reg=0;
    unsigned int phy9Reg=0;
    for( phyUnit = 0; phyUnit < numports; phyUnit++ )
    {
	if( ( ( 1 << phyUnit ) & portmask ) )
	{
	    setPhy( IP_GLOBAL_PHY29_ADDR,IP_GLOBAL_PHY29_24_REG +( ( phyUnit == 5 ) ? ( phyUnit + 1 ) : phyUnit ),IP_VLAN_TABLE_SETTING( phyUnit ) );
	    fprintf(stderr,"write register %d, addr %d with %X\n",IP_GLOBAL_PHY29_ADDR,IP_GLOBAL_PHY29_24_REG +( ( phyUnit == 5 ) ? ( phyUnit + 1 ) : phyUnit ),IP_VLAN_TABLE_SETTING( phyUnit ) );
	    if( IP_IS_ENET_PORT( phyUnit ) )
	    {
		if( IP_IS_WAN_PORT( phyUnit ) )
		{
		    phy2Reg |= ( ( 1 << phyUnit ) <<IP_VLAN2_OUTPUT_PORT_MASK_S );
		}
		else
		{
		    phy1Reg |= ( ( 1 << phyUnit ) <<IP_VLAN1_OUTPUT_PORT_MASK_S );
		}
		phy23Reg = phy23Reg | ( ( 1 << phyUnit ) << IP_PORTX_REMOVE_TAG_S );
		phy23Reg = phy23Reg & ~( ( 1 << phyUnit ) << IP_PORTX_ADD_TAG_S );
	    }
	    else
	    {
		phy1Reg |= ( ( 1 << phyUnit ) <<IP_VLAN1_OUTPUT_PORT_MASK_S );
		phy2Reg |= ( ( 1 << phyUnit ) <<IP_VLAN2_OUTPUT_PORT_MASK_S );
		phy23Reg = phy23Reg | ( 1 << IP_PORT5_ADD_TAG_S );
		phy23Reg = phy23Reg & ~( 1 << IP_PORT5_REMOVE_TAG_S );

	    }
	}
    }
    phy9Reg = 0;//getPhy(IP_GLOBAL_PHY30_ADDR,IP_GLOBAL_PHY30_9_REG);
    phy9Reg = phy9Reg | TAG_VLAN_ENABLE;
    phy9Reg = phy9Reg & ~VID_INDX_SEL_M;
    phy9Reg = phy9Reg | 1; //1 vlan group used for lan
    phy9Reg = phy9Reg | 1<<3; //enable smart mac
    phy9Reg = phy9Reg | 1<<12; //port 0 is a wan port (required for smart mac)
    
    fprintf(stderr,"write register %d, addr %d with %X\n",IP_GLOBAL_PHY29_ADDR, IP_GLOBAL_PHY29_23_REG,phy23Reg);
    fprintf(stderr,"write register %d, addr %d with %X\n",IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_1_REG,phy1Reg);
    fprintf(stderr,"write register %d, addr %d with %X\n",IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_2_REG,phy2Reg);
    fprintf(stderr,"write register %d, addr %d with %X\n",IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_9_REG,phy9Reg);
    setPhy( IP_GLOBAL_PHY29_ADDR, IP_GLOBAL_PHY29_23_REG,phy23Reg );
    setPhy( IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_1_REG, phy1Reg );
    setPhy( IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_2_REG, phy2Reg );
    setPhy( IP_GLOBAL_PHY30_ADDR, IP_GLOBAL_PHY30_9_REG, phy9Reg );
//
//		echo "echo \"WRITE 29 23 07c2\" > ".$mii_dev."\n";
//
//		echo "echo \"WRITE 29 24 0\"    > ".$mii_dev."\n";	/* PORT0 Default VLAN ID */
//		echo "echo \"WRITE 29 25 0\"    > ".$mii_dev."\n";	/* PORT1 Default VLAN ID */
//		echo "echo \"WRITE 29 26 0\"    > ".$mii_dev."\n";	/* PORT2 Default VLAN ID */
//		echo "echo \"WRITE 29 27 0\"    > ".$mii_dev."\n";	/* PORT3 Default VLAN ID */
//		echo "echo \"WRITE 29 28 2\"    > ".$mii_dev."\n";	/* PORT4 Default VLAN ID */
//		echo "echo \"WRITE 29 30 0\"    > ".$mii_dev."\n";	/* PORT5 Default VLAN ID (CPU) */
//		echo "echo \"WRITE 29 23 07c2\" > ".$mii_dev."\n";
//		echo "echo \"WRITE 30 1 002f\"  > ".$mii_dev."\n";	/* Port 5,3,2,1,0 = VLAN 0 */
//		echo "echo \"WRITE 30 2 0030\"  > ".$mii_dev."\n";	/* Port 5,4 = VLAN 2 */
//		echo "echo \"WRITE 30 9 1089\"  > ".$mii_dev."\n";
    eval( "vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD" );
    eval( "vconfig", "add", "eth0", "1" );
    eval( "vconfig", "add", "eth0", "2" );
    struct ifreq ifr;
    int s;

    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) )
    {
	char eabuf[32];

	strncpy( ifr.ifr_name, "eth0", IFNAMSIZ );
	ioctl( s, SIOCGIFHWADDR, &ifr );
	char macaddr[32];

	strcpy( macaddr,
		ether_etoa( ( unsigned char * )ifr.ifr_hwaddr.sa_data,
			    eabuf ) );
	nvram_set( "et0macaddr", macaddr );
	// MAC_ADD (macaddr);
	ether_atoe( macaddr, ( unsigned char * )ifr.ifr_hwaddr.sa_data );
	strncpy( ifr.ifr_name, "vlan2", IFNAMSIZ );
	ioctl( s, SIOCSIFHWADDR, &ifr );
	close( s );
    }
    eval("ifconfig","vlan1","promisc");
    eval("ifconfig","vlan2","promisc");
}
