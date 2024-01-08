#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <../../linux/linux/include/linux/if_vlan.h>
#include <linux/sockios.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define uint32 unsigned int
#define uint unsigned int
#define uint8 unsigned char
#define uint64 unsigned long long
#include <swmod.h>
#include <swapi.h>
#include <nvports.h>
#include <bcmdevs.h>
#include <nvparse.h>
extern int wan_valid(char *ifname);

/* 
 * VLAN Descriptors, per-board or platform setting 
 */
struct vlan_board_s {
	char *bd_name;
	char *bd_desc;
	int numports;
	int wan_port;
	int lan_port_start;
	int lan_port_end;
	int vlan_devno;
	int wan_vlan;
	int lan_vlan;
} vlan_boards[] = {
	{ "reference board", "Broadcom Sentry5", BCM_NUM_PORTS, BCM_WAN_PORT,
	  BCM_LAN_MIN_PORT, BCM_LAN_MAX_PORT, 0, BCM_DEF_WAN_VLAN,
	  BCM_DEF_LAN_VLAN },
};

int vlan_configured = 0;
int brcm_tag_driver_enabled = 0;

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static int vlan_configure(void)
{
	char *vlan_enable = nvram_safe_get("vlan_enable");
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char *lan_ifnames = nvram_safe_get("lan_ifnames");
	char *wan_hwaddr = nvram_safe_get("wan_hwaddr");
	char *wan_ifname = NULL;
	char *wan_proto;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *restore_lan_ifname = nvram_safe_get("restore_lan_ifname");
	char *restore_lan_ifnames = nvram_safe_get("restore_lan_ifnames");
	char *restore_wan_ifname = nvram_safe_get("restore_wan_ifname");
	char *restore_wan_hwaddr = nvram_safe_get("restore_wan_hwaddr");
	char *mibac_enable = nvram_safe_get("mibac_enable");
	char *mibac_interval = nvram_safe_get("mibac_interval");
	char *brcmtag_enable = nvram_safe_get("brcmtag_enable");
	char *wan_vlan = nvram_safe_get("wan_vlan");
	char *lan_vlan = nvram_safe_get("lan_vlan");
	char *lan_br_name;
	char buf[80];
	char lan_ifbase_realdev[80], wan_ifbase[80];
	char name[80], lan_ifbase[80], br_group[80], *next, vlan[4],
		vlan_wan[4], port[4];
	uint bVlan, board_index = 0, i, buf_index = 0, hasBridge = 0;
	uint bBrcmTag, bMIBAC;
	uint wan_vid = 0, lan_vid = 0, lan_vids[BCM_NUM_PORTS - 1], vid;
	int unit;

	bcm_l2_addr_t l2addr;
	bcm_mac_t eth_addr, mcast_addr = BRIDGE_GROUP_MAC_ADDR;
	int retval;
	unsigned short MIBport;
	bcm_mac_t MIBda;
	uint mibac_int = ROBO_AC_RATE_DEFAULT;

	if (!bcm_is_robo())
		return -1;
	bVlan = (atoi(vlan_enable) > 0) ||
		(*vlan_enable &&
		 !strncmp(vlan_enable, "true", strlen(vlan_enable)));
	printf("\n\nBroadcom Enterprise RG/AP\n");
	printf("\r\nBroadcom ROBO 802.1Q VLAN %s\n",
	       bVlan > 0 ? "enabled" : "disabled");

	bMIBAC = (atoi(mibac_enable) > 0) ||
		 (*(mibac_enable) &&
		  !strncmp(mibac_enable, "true", strlen(mibac_enable)));

	bBrcmTag = ((atoi(brcmtag_enable) > 0) ||
		    (*(brcmtag_enable) &&
		     !strncmp(brcmtag_enable, "true", strlen(brcmtag_enable))));

	if (*(mibac_interval)) {
		mibac_int = ROBO_GET_AC_RATE(atoi(mibac_interval));
		if (mibac_int > ROBO_AC_RATE_MAX)
			mibac_int = ROBO_AC_RATE_MAX;
	}

	/* 
	 * find configured and enabled WAN connection 
	 */
	for (unit = 0; unit < MAX_NVPARSE; unit++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));
		if (!*wan_ifname)
			continue;
		wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
		if (!*wan_proto || !strcmp(wan_proto, "disabled"))
			continue;
		/* 
		 * disable the connection if the i/f is not in wan_ifnames 
		 */
		if (!wan_valid(wan_ifname)) {
			nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
			continue;
		}
		break;
	}

	if (!wan_ifname) {
		if (bcm_is_robo()) {
			dprintf("No active, configured WAN interface found\n");
		}
		return -1;
	}

	/* 
	 * check to make sure vlan numbers are valid 
	 */
	/* 
	 * (if no nvram variables, use defaults) 
	 */
	if (*(wan_vlan))
		wan_vid = atoi(wan_vlan);
	else
		wan_vid = vlan_boards[board_index].wan_vlan;
	if (*(lan_vlan))
		lan_vid = atoi(lan_vlan);
	else
		lan_vid = vlan_boards[board_index].lan_vlan;
	for (i = 0; i < BCM_NUM_PORTS; i++)
		lan_vids[i] = 0;
	if (wan_vid == 0 || wan_vid > VLAN_ID_MAX) {
		printf("'wan_vlan' does not have valid vlan id\n");
		bVlan = 0;
	}
	if (lan_vid == 0 || lan_vid > VLAN_ID_MAX || lan_vid == wan_vid) {
		printf("'lan_vlan' does not have valid vlan id\n");
		bVlan = 0;
	}

	/* 
	 * first restore ifnames, if necessary.  note that if the user 
	 */
	/* 
	 * wants to change lan_ifname or wan_ifname, it will be necessary 
	 */
	/* 
	 * to delete the corresponding 'restore' value to avoid subsequent 
	 */
	/* 
	 * overwriting of the new value when the router starts 
	 */
	if (*(restore_wan_ifname)) {
		if (nvram_set(strcat_r(prefix, "ifname", tmp),
			      restore_wan_ifname))
			return -1;
		if (nvram_unset("restore_wan_ifname"))
			return -1;
	}
	if (*(restore_lan_ifname)) {
		if (nvram_set("lan_ifname", restore_lan_ifname))
			return -1;
		if (nvram_unset("restore_lan_ifname"))
			return -1;
	}
	if (*(restore_lan_ifnames)) {
		if (nvram_set("lan_ifnames", restore_lan_ifnames))
			return -1;
		if (nvram_unset("restore_lan_ifnames"))
			return -1;
	}
	if (*(restore_wan_hwaddr)) {
		if (nvram_set(strcat_r(prefix, "wan_hwaddr", tmp),
			      restore_wan_hwaddr))
			return -1;
		if (nvram_unset("restore_wan_hwaddr"))
			return -1;
	}

	/* 
	 * now check to see if vlan disabled 
	 */
	/* 
	 * disabled, just exit 
	 */
	if (!bVlan)
		return 0;

	/* 
	 * activate interface to Robo switch 
	 */
	if ((bcm_api_init()) < 0) {
		if (bcm_is_robo()) {
			dprintf("No ROBO device found\n");
		}
		return -1;
	}
	/* 
	 * create interface names 
	 */
	/* 
	 * get base name that interface will be based on 
	 */
	sprintf(lan_ifbase, "eth%d", vlan_boards[board_index].vlan_devno);

	/* 
	 * check to see if bridge has been set up 
	 */
	if (strncmp(lan_ifname, "br", 2) == 0) {
		/* 
		 * iterate though members of bridge group and extract lan name for 
		 */
		/* 
		 * use in creating vlans and create list w/o lan member, which will 
		 */
		/* 
		 * have vlan group members added below 
		 */
		buf_index = 0;
		char *ifnames = nvram_safe_get("lan_ifnames");

		foreach(name, ifnames, next)
		{
			if (strncmp(name, lan_ifbase, 4) == 0) {
				hasBridge = 1;
			} else {
				strcpy(&br_group[buf_index], name);
				strcat(br_group, " ");
				buf_index = strlen(br_group);
			}
		}
	}

	/* 
	 * note that if brcm tags are enabled, the WAN is configured with a vlan
	 * and 
	 */
	/* 
	 * the LAN ports are configured with the brcm tag driver.  if brcm tags
	 * are disabled 
	 */
	/* 
	 * the LAN is configured with a vlan and the WAN directly uses eth0.
	 * This 
	 */
	/* 
	 * only applies to vlan interfaces created with vconfig.  In both cases,
	 * the underlying 
	 */
	/* 
	 * switch is programmed with a separate vlan for WAN and LAN, but in the
	 * case w/o 
	 */
	/* 
	 * brcm tags, the WAN vlan tag is removed when it exits the switch to the 
	 * CPU 
	 */

	/* 
	 * modify lan_ifbase & wan_ifbase for brcm tag driver, but save to bring
	 * up interface 
	 */
	strcpy(lan_ifbase_realdev, lan_ifbase);
	strcpy(wan_ifbase, lan_ifbase);
	if (bBrcmTag) {
		/* 
		 * if brcm tag driver will be used, add 't' to root name 
		 */
		strcat(lan_ifbase, "t");
		strcat(wan_ifbase, "t");
		sprintf(buf, ".%d", vlan_boards[board_index].wan_port);
		strcat(wan_ifbase, buf);
		/* 
		 * need to create wan vlan name here.  the form of the name will be
		 * ethx.port.vlan 
		 */
		sprintf(buf, "%s.%d", wan_ifbase, wan_vid);
		nvram_set("restore_wan_ifname", wan_ifname);
		nvram_set(strcat_r(prefix, "ifname", tmp), buf);
		nvram_set(strcat_r(prefix, "ifnames", tmp), buf);
	}

	/* 
	 * we need to handle the case of if bridge exists or not.  if it does
	 * exist, and brcm tags are enabled brcm tag (for default) interface
	 * names will be substituted for 'ethx' name.  if it doesn't exist,
	 * bridge 'br0' will be created with only vlan or brcm tag interface
	 * names 
	 */
	if (hasBridge) {
		lan_br_name = lan_ifname;
		strcpy(&buf[0], &br_group[0]);
	} else {
		lan_br_name = "br0";
		buf_index = 0;
		hasBridge = 1;
	}
	for (i = vlan_boards[board_index].lan_port_start;
	     i <= vlan_boards[board_index].lan_port_end; i++) {
		if (bBrcmTag)
			/* 
			 * use port ids for interface suffixes 
			 */
			sprintf(&buf[buf_index], "%s.%d ", lan_ifbase, i);
		else
			/* 
			 * use single lan vlan id interface suffixes 
			 */
			sprintf(&buf[buf_index], "%s.%d ", lan_ifbase, lan_vid);
		buf_index = strlen(buf);
		if (!bBrcmTag)
			/* 
			 * if no brcm tag driver, just use one interface for all ports on 
			 * LAN side 
			 */
			break;
	}
	/* 
	 * get rid of last space 
	 */
	buf[strlen(buf) - 1] = '\0';
	printf("lan_ifnames %s\n", buf);
	nvram_set("restore_lan_ifnames", lan_ifnames);
	nvram_set("restore_lan_ifname", lan_ifname);
	nvram_set("lan_ifnames", buf);
	nvram_set("lan_ifname", lan_br_name);

	/* 
	 * change wan_hwaddr to be lan_hwaddr.  this is because underlying lan
	 * device has lan_hwaddr and if wan vlan device had different hwaddr, the 
	 * vlan processing would set the underlying device to promiscuous mode 
	 */
	nvram_set("restore_wan_hwaddr", wan_hwaddr);
	nvram_set(strcat_r(prefix, "hwaddr", tmp),
		  nvram_safe_get("lan_hwaddr"));

	/* 
	 * now create vlan i/f's 
	 */
	/* 
	 * need to bring up the underlying switch i/f to create vlans 
	 */
	ifconfig(lan_ifbase_realdev, IFUP, 0, 0);

	if (bBrcmTag) {
		/* 
		 * create brcm tag device for wan 
		 */
		bcm_reg_brcmtag_dev(lan_ifbase_realdev, "t",
				    vlan_boards[board_index].wan_port);
		ifconfig(wan_ifbase, IFUP, 0, 0);
	}

	/* 
	 * first, wan vlan.  note, must also configure wan here, in case 
	 */
	/* 
	 * it's the same wan being used for nfs, in order to allow nfs to 
	 */
	/* 
	 * keep working during configuration of vlans 
	 */
	sprintf(vlan_wan, "%d", wan_vid);
	eval("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
	if (bBrcmTag)
		eval("vconfig", "add", wan_ifbase, vlan_wan);
	else
		/* 
		 * don't configure WAN as vlan interface, just program switch with
		 * vlan id 
		 */
		bcm_vlan_create(0, wan_vid); /* this is for default case, which
						 * doesn't call vconfig */
	/* 
	 * Bring up WAN interface 
	 */
	ifconfig(nvram_safe_get(strcat_r(prefix, "ifname", tmp)), IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	/* 
	 * now configure wan vlan and enable vlans 
	 */
	sprintf(port, "%d", vlan_boards[board_index].wan_port);
	eval("vconfig", "add_port", vlan_wan, port);
	/* 
	 * MII port is set up to tag with wan vlan here, but that will be
	 * overridden below 
	 */
	/* 
	 * to tag with lan vlan (only matters for default case where CPU doesn't
	 * have vlan 
	 */
	/* 
	 * configured, but switch does; ie, mii port does tag/untag) 
	 */
	sprintf(port, "%d", BCM_MII_PORT); /* mii port, note that vlan
						 * tags retained on egress */
	if (bBrcmTag)
		eval("vconfig", "add_port_nountag", vlan_wan, port);
	else
		/* 
		 * if no BRCM tags, untag MII for WAN VLAN 
		 */
		eval("vconfig", "add_port", vlan_wan, port);
	if (bBrcmTag)
		/* 
		 * set flag to let vlan dev build headers in vlan_dev_hard_header 
		 */
		eval("vconfig", "set_flag",
		     nvram_safe_get(strcat_r(prefix, "wan_ifname", tmp)), "1",
		     "0");

	/* 
	 * now enable switch ports for vlan 
	 */
	sprintf(buf, "%d", 1);
	eval("vconfig", "ports_enable", buf);

	/* 
	 * set multicast related registers 
	 */
	buf[0] = 0x0a; /* Rsvd MC tag/untag */
	if (bBrcmTag)
		/* 
		 * if default mode, allow tagging of frames into MII.  this is to
		 * allow 
		 */
		/* 
		 * lan to have vlan handling in switch, but remove vlan tag when
		 * received 
		 */
		/* 
		 * by CPU 
		 */
		buf[0] |= 0x80;
	bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL1, buf, 1);
	buf[0] = 0x01; /* enable MC ARL entries to use port map */
	bcm_write_reg(0, ROBO_CTRL_PAGE, ROBO_IP_MULTICAST_CTRL, buf, 1);
	buf[0] = 0x1c; /* MII receive unicast, multicast, broadcast */
	bcm_write_reg(0, ROBO_CTRL_PAGE, ROBO_IM_PORT_CTRL, buf, 1);
	buf[0] = 0x40; /* drop frame if frame has VID violation */
	bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL4, buf, 1);
	buf[0] = 0x08; /* drop frame if vtable miss */
	bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5, buf, 1);
	if (bBrcmTag) {
		/* 
		 * now set up managed mode registers for brcm tag 
		 */
		buf[0] = 0x00; /* Rsvd MC tag/untag */
		bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL1, buf, 1);
		buf[0] = 0x8e; /* MII port as management port, IGMP snoop,
				 * rx BPDUs */
		bcm_write_reg(0, ROBO_MGMT_PAGE, ROBO_GLOBAL_CONFIG, buf, 1);
		buf[0] = 0x08; /* mgmt port ID */
		bcm_write_reg(0, ROBO_MGMT_PAGE, ROBO_MGMT_PORT_ID, buf, 1);
		buf[0] = 0x0a; /* ignore crc check on mgmt port */
		bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL5, buf, 1);
		buf[0] = 0x03; /* turn on managed mode */
		bcm_write_reg(0, ROBO_CTRL_PAGE, ROBO_SWITCH_MODE, buf, 1);
		brcm_tag_driver_enabled = 1;
	}
	/* 
	 * now assign 0.0.0.0 ip address to i/f underlying vlans 
	 */
	if (bBrcmTag)
		ifconfig(lan_ifbase_realdev, IFUP, "0.0.0.0", "0.0.0.0");

	/* 
	 * now lan vlan 
	 */
	for (i = vlan_boards[board_index].lan_port_start;
	     i <= vlan_boards[board_index].lan_port_end; i++) {
		/* 
		 * if default, no vconfig, because 
		 */
		/* 
		 * vlan untagged at MII port 
		 */
		if (bBrcmTag) {
			/* 
			 * default mode (w/brcm tags), create brcm tag device per port
			 * and set up 
			 */
			/* 
			 * single vlan for all ports 
			 */
			if (i == vlan_boards[board_index].lan_port_start) {
				bcm_vlan_create(
					0,
					lan_vid); /* this is for default case,
								 * which doesn't call vconfig 
								 */
				sprintf(vlan, "%d", lan_vid);
				sprintf(port, "%d",
					BCM_MII_PORT); /* mii port */
				eval("vconfig", "add_port", vlan, port);
			}
			sprintf(port, "%d", i);
			eval("vconfig", "add_port", vlan, port);
			bcm_reg_brcmtag_dev(lan_ifbase_realdev, "t", i);
		} else {
			/* 
			 * brcm tags disabled.  create one vlan interface for LAN 
			 */
			if (i == vlan_boards[board_index].lan_port_start) {
				/* 
				 * only create one vlan for default mode w/no brcm tags 
				 */
				sprintf(vlan, "%d", lan_vid);
				eval("vconfig", "add", lan_ifbase, vlan);
				/* 
				 * also add mii port 
				 */
				sprintf(port, "%d",
					BCM_MII_PORT); /* mii port */
				eval("vconfig", "add_port_nountag_nodeftag",
				     vlan, port);
			}
			sprintf(port, "%d", i);
			eval("vconfig", "add_port", vlan, port);
			/* 
			 * add interface to port/interface map 
			 */
			bcm_add_port_interface(i, lan_vid);
		}
	}

	if (hasBridge && bBrcmTag) {
		/* 
		 * init spanning tree state 
		 */
		/* 
		 * set WAN port & MII to forwarding, all others to disabled 
		 */
		bcm_port_stp_set(BCM_WAN_PORT, BCM_PORT_STP_FORWARD);
		bcm_port_stp_set(9, BCM_PORT_STP_FORWARD);
		for (i = vlan_boards[board_index].lan_port_start;
		     i <= vlan_boards[board_index].lan_port_end; i++)
			bcm_port_stp_set(i, BCM_PORT_STP_DISABLE);
	} else {
		/* 
		 * init spanning tree state to none for all ports 
		 */
		for (i = 1; i <= BCM_MAX_PORT; i++)
			bcm_port_stp_set(i, BCM_PORT_STP_NONE);
	}
	/* 
	 * add static entries to arl for wan hwaddr & lan hwaddr 
	 */
	ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), eth_addr);
	bcm_l2_addr_init(&l2addr, eth_addr, wan_vid);
	l2addr.port = BCM_MII_ARL_UC_PORT - 1;
	if (BCM_RET_SUCCESS != (retval = bcm_l2_addr_add(0, &l2addr)))
		dprintf("failure writing wan l2 addr: %d\n", retval);
	ether_atoe(nvram_safe_get("lan_hwaddr"), eth_addr);
	/* 
	 * set up MIB autocast, if enabled 
	 */
	if (bMIBAC) {
		/* 
		 * set up MIB AC (dst, src, port & rate) 
		 */
		memcpy(MIBda, eth_addr, sizeof(bcm_mac_t));
		bcm_byteswap(&MIBda);
		bcm_write_reg(0, ROBO_MIB_AC_PAGE, ROBO_MIB_AC_DA, MIBda,
			      sizeof(bcm_mac_t));
		memset(buf, 0, sizeof(bcm_mac_t));
		bcm_write_reg(0, ROBO_MIB_AC_PAGE, ROBO_MIB_AC_SA, buf,
			      sizeof(bcm_mac_t));
		MIBport = PBMP_PORT(BCM_MII_ARL_UC_PORT - 1);
		bcm_write_reg(0, ROBO_MIB_AC_PAGE, ROBO_MIB_AC_PORT,
			      (uint8 *)&MIBport, sizeof(MIBport));
		bcm_write_reg(0, ROBO_MIB_AC_PAGE, ROBO_MIB_AC_RATE,
			      (uint8 *)&mibac_int, sizeof(char));
		bcm_read_reg(0, ROBO_MGMT_PAGE, ROBO_GLOBAL_CONFIG, buf, 1);
		buf[0] |= 0x20; /* enable MIB AC */
		bcm_write_reg(0, ROBO_MGMT_PAGE, ROBO_GLOBAL_CONFIG, buf, 1);
	}
	/* 
	 * now multicast entry for BPDUs 
	 */
	bcm_l2_addr_init(&l2addr, mcast_addr, wan_vid);
	l2addr.pbmp = PBMP_PORT(i - 1) | PBMP_PORT(BCM_MII_PORT - 1);
	if (BCM_RET_SUCCESS != (retval = bcm_l2_addr_add(0, &l2addr)))
		dprintf("failure writing lan l2 addr: %d\n", retval);
	for (i = vlan_boards[board_index].lan_port_start;
	     i <= vlan_boards[board_index].lan_port_end; i++) {
		vid = lan_vid;
		if (i == vlan_boards[board_index].lan_port_start) {
			bcm_l2_addr_init(&l2addr, eth_addr, vid);
			l2addr.port = BCM_MII_ARL_UC_PORT - 1;
			if (BCM_RET_SUCCESS !=
			    (retval = bcm_l2_addr_add(0, &l2addr)))
				dprintf("failure writing lan l2 addr: %d\n",
					retval);
			/* 
			 * now multicast entry for BPDUs 
			 */
			bcm_l2_addr_init(&l2addr, mcast_addr, vid);
			l2addr.pbmp = PBMP_PORT(i - 1) |
				      PBMP_PORT(BCM_MII_PORT - 1);
			if (BCM_RET_SUCCESS !=
			    (retval = bcm_l2_addr_add(0, &l2addr)))
				dprintf("failure writing lan l2 addr: %d\n",
					retval);
		}
	}
	if (!bBrcmTag) {
		/* 
		 * if in mgmt mode (if brcm tag driver used), don't need to do this 
		 */
		/* 
		 * set bogus mc addr to bypass specian h/w handling of BPDUs and let
		 * ARL handle them 
		 */
		memset(&mcast_addr, 0, sizeof(mcast_addr));
		bcm_write_reg(0, ROBO_ARLCTRL_PAGE, ROBO_BPDU_MC_ADDR_REG,
			      mcast_addr, sizeof(mcast_addr));
	}
	vlan_configured = 1;
	/* 
	 * deinit Robo switch interface 
	 */
	bcm_api_deinit();

	/* 
	 * We had changed wan_hwaddr to be the same as lan_hwaddr (temporarily)
	 * due the issue of promiscuous. Now it's time to roll it back. 
	 */
	restore_wan_hwaddr = nvram_safe_get("restore_wan_hwaddr");
	if (*(restore_wan_hwaddr)) {
		if (nvram_set(strcat_r(prefix, "hwaddr", tmp),
			      restore_wan_hwaddr))
			return -1;
		if (nvram_unset("restore_wan_hwaddr"))
			return -1;
	}

	nvram_async_commit();

	return 0;
}

static int vlan_deconfigure(void)
{
	char *restore_lan_ifname = nvram_safe_get("restore_lan_ifname");
	char *restore_lan_ifnames = nvram_safe_get("restore_lan_ifnames");
	char *restore_wan_ifname = nvram_safe_get("restore_wan_ifname");
	char *restore_wan_hwaddr = nvram_safe_get("restore_wan_hwaddr");
	char *wan_ifname = NULL;
	char *wan_proto;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char buf[80];
	char name[80], *next;
	uint board_index = 0;
	int unit;

	/* 
	 * first, bring down vlans, if enabled 
	 */
	dprintf("deconfigure vlan\n");
	if (vlan_configured) {
		/* 
		 * activate interface to Robo switch 
		 */
		if ((bcm_api_init()) < 0) {
			dprintf("No ROBO device found\n");
			return -1;
		}

		/* 
		 * find configured and enabled WAN connection 
		 */
		for (unit = 0; unit < MAX_NVPARSE; unit++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			wan_ifname =
				nvram_safe_get(strcat_r(prefix, "ifname", tmp));
			if (!*wan_ifname)
				continue;
			wan_proto =
				nvram_safe_get(strcat_r(prefix, "proto", tmp));
			if (!*wan_proto || !strcmp(wan_proto, "disabled"))
				continue;
			/* 
			 * disable the connection if the i/f is not in wan_ifnames 
			 */
			if (!wan_valid(wan_ifname)) {
				nvram_set(strcat_r(prefix, "proto", tmp),
					  "disabled");
				continue;
			}
		}

		if (!wan_ifname) {
			if (bcm_is_robo()) {
				dprintf("No active, configured WAN interface found\n");
			}
			return -1;
		}

		/* 
		 * vlan is non-zero, that means that eth0 is wan port 
		 */
		if (vlan_boards[board_index].vlan_devno)
			ifconfig(
				"eth0", IFUP,
				nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
				nvram_safe_get(
					strcat_r(prefix, "netmask", tmp)));

		/* 
		 * now bring down brcm tag device for wan device 
		 */
		if (brcm_tag_driver_enabled) {
			/* 
			 * Strip off vlan suffix 
			 */
			strcpy(name, wan_ifname);
			next = strrchr(name, '.');
			if (next != NULL) {
				*next = '\0';
				bcm_unreg_brcmtag_dev(name);
			}
			buf[0] = 0x02; /* turn off managed mode */
			bcm_write_reg(0, ROBO_CTRL_PAGE, ROBO_SWITCH_MODE, buf,
				      1);
			buf[0] = 0x00; /* turn off vlan mode */
			bcm_write_reg(0, ROBO_VLAN_PAGE, ROBO_VLAN_CTRL0, buf,
				      1);
			/* 
			 * now, deconfigure wan vlan 
			 */
			eval("vconfig", "rem", wan_ifname);
		}

		/* 
		 * now lan vlan 
		 */
		if (brcm_tag_driver_enabled) {
			char *ifnames = nvram_safe_get("lan_ifnames");

			/* 
			 * default mode, just bring down brcm tag devices per port 
			 */
			foreach(name, ifnames, next)
			{
				/* 
				 * check to see if has suffix '.port' before removing 
				 */
				if (strrchr(name, '.') != NULL)
					bcm_unreg_brcmtag_dev(name);
			}
			brcm_tag_driver_enabled = 0;
		} else
			/* 
			 * no brcm tags, bring down lan vlan 
			 */
			char *ifnames = nvram_safe_get("lan_ifnames");

		foreach(name, ifnames, next)
		{
			/* 
			 * check to see if has suffix '.port' before removing 
			 */
			if (strrchr(name, '.') != NULL)
				eval("vconfig", "rem", name);
		}
	}
	/* 
	 * now disable switch ports for vlan 
	 */
	sprintf(buf, "%d\n", 0);
	eval("vconfig", "ports_enable", buf);
	vlan_configured = 0;
	bcm_api_deinit();

	/* 
	 * now restore environment 
	 */
	if (*(restore_wan_ifname)) {
		if (nvram_set(strcat_r(prefix, "ifname", tmp),
			      restore_wan_ifname))
			return -1;
		if (nvram_unset("restore_wan_ifname"))
			return -1;
	}
	if (*(restore_lan_ifname)) {
		if (nvram_set("lan_ifname", restore_lan_ifname))
			return -1;
		if (nvram_unset("restore_lan_ifname"))
			return -1;
	}
	if (*(restore_lan_ifnames)) {
		if (nvram_set("lan_ifnames", restore_lan_ifnames))
			return -1;
		if (nvram_unset("restore_lan_ifnames"))
			return -1;
	}
	if (*(restore_wan_hwaddr)) {
		if (nvram_set(strcat_r(prefix, "wan_hwaddr", tmp),
			      restore_wan_hwaddr))
			return -1;
		if (nvram_unset("restore_wan_hwaddr"))
			return -1;
	}

	return 0;
}

/* 
 * Setup VLAN interfaces, if enabled
 */
int start_vlan(void)
{
	/* 
	 * Bringup vlan interfaces 
	 */
	vlan_configure();
	return 0;
}

/* 
 * Stop VLANs, if they have been created.
 */
int stop_vlan(void)
{
	/* 
	 * Shut down vifs 
	 */
	vlan_deconfigure();
	return 0;
}

int start_portmon()
{
	/* 
	 * Start Linkscan 
	 */
	return 0;
}

int stop_portmon()
{
	/* 
	 * Stop linkscan 
	 */
	return 0;
}
