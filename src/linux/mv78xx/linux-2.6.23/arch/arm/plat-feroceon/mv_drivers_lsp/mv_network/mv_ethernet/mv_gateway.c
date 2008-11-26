/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/

#include "mvCommon.h"  /* Should be included before mvSysHwConfig */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <net/ip.h>
#include <net/xfrm.h>

#include "mvOs.h"
#include "dbg-trace.h"
#include "mvSysHwConfig.h"
#include "eth/mvEth.h"
#include "eth-phy/mvEthPhy.h"
#include "ctrlEnv/sys/mvSysGbe.h"
#include "msApi.h"

#include "mv_netdev.h"

#if (MV_ETH_RX_Q_NUM > 1)
 /* RX priority (lower number = lower priority) */

# ifdef CONFIG_MV_GTW_QOS_ROUTING
#   define MV_ROUTING_PRIO	2
# endif

# ifdef CONFIG_MV_GTW_QOS_VOIP
#   define MV_VOIP_PRIO		3
# endif
#endif /* (MV_ETH_RX_Q_NUM > 1) */


int SWITCH_PORT_CPU;
int SWITCH_PORT_0;
int SWITCH_PORT_1;
int SWITCH_PORT_2;
int SWITCH_PORT_3;
int SWITCH_PORT_4;


/* use this MACRO to find if a certain port (0-7) is actually connected */
#define SWITCH_IS_PORT_CONNECTED(p)	                            \
            ( ((p) == SWITCH_PORT_CPU) || ((p) == SWITCH_PORT_0) || \
			  ((p) == SWITCH_PORT_1)   || ((p) == SWITCH_PORT_2) || \
			  ((p) == SWITCH_PORT_3)   || ((p) == SWITCH_PORT_4) ) 

/* helpers for VLAN tag handling */
#define MV_GTW_PORT_VLAN_ID(grp,port)  ((grp)+(port)+1)
#define MV_GTW_GROUP_VLAN_ID(grp)      (((grp)+1)<<8)
#define MV_GTW_VLANID_TO_PORT(vlanid)  (((vlanid) & 0xf)-1)

unsigned int        switch_enabled_ports;

#ifdef CONFIG_MV_GTW_LINK_STATUS
static int          switch_irq = -1;
struct timer_list   switch_link_timer;
#endif

#ifdef CONFIG_MV_GTW_QOS_VOIP
int mv_gtw_qos_tos_quota = -1;
extern int MAX_SOFTIRQ_RESTART;
int mv_gtw_qos_tos_enable(void);
int mv_gtw_qos_tos_disable(void);
EXPORT_SYMBOL(mv_gtw_qos_tos_enable);
EXPORT_SYMBOL(mv_gtw_qos_tos_disable);
#endif /* CONFIG_MV_GTW_QOS_VOIP */


#ifdef CONFIG_MV_GTW_IGMP
extern int mv_gtw_igmp_snoop_init(void);
extern int mv_gtw_igmp_snoop_exit(void);
extern int mv_gtw_igmp_snoop_process(struct sk_buff* skb, unsigned char port, unsigned char vlan_dbnum);
#endif 

/* Example: "mv_net_config=(eth0,00:99:88:88:99:77,0)(eth1,00:55:44:55:66:77,1:2:3:4),mtu=1500" */
static char *cmdline = NULL;

struct mv_gtw_config    gtw_config;

GT_QD_DEV qddev,        *qd_dev = NULL;
static GT_SYS_CONFIG    qd_cfg;

static int mv_gtw_port2lport(int port)
{
    if(port==SWITCH_PORT_0) return 0;
    if(port==SWITCH_PORT_1) return 1;
    if(port==SWITCH_PORT_2) return 2;
    if(port==SWITCH_PORT_3) return 3;
    if(port==SWITCH_PORT_4) return 4;
    return -1;
} 

/* Local function prototypes */
#ifdef CONFIG_MV_GTW_QOS
static void mv_gtw_set_qos_in_switch(void);
#endif

/* Required to get the configuration string from the Kernel Command Line */
int mv_gtw_cmdline_config(char *s);
__setup("mv_net_config=", mv_gtw_cmdline_config);

int mv_gtw_cmdline_config(char *s)
{
    cmdline = s;
    return 1;
}

static int mv_gtw_check_open_bracket(char **p_net_config)
{
    if (**p_net_config == '(') {
        (*p_net_config)++;
	return 0;
    }
    printk("Syntax error: could not find opening bracket\n");
    return -EINVAL;
}

static int mv_gtw_check_closing_bracket(char **p_net_config)
{
    if (**p_net_config == ')') {
        (*p_net_config)++;
	return 0;
    }
    printk("Syntax error: could not find closing bracket\n");
    return -EINVAL;
}

static int mv_gtw_check_comma(char **p_net_config)
{
    if (**p_net_config == ',') {
        (*p_net_config)++;
	    return 0;
    }
    printk("Syntax error: could not find comma\n");
    return -EINVAL;
}


static int mv_gtw_is_digit(char ch)
{
    if( ((ch >= '0') && (ch <= '9')) ||
	((ch >= 'a') && (ch <= 'f')) ||
	((ch >= 'A') && (ch <= 'F')) )
	    return 0;

    return -1;
}

static int mv_gtw_char_to_hex( char ch )
{
    if( (ch >= '0') && (ch <= '9') )
        return( ch - '0' );

    if( (ch >= 'a') && (ch <= 'f') )
	    return( ch - 'a' + 10 );

    if( (ch >= 'A') && (ch <= 'F') )
	    return( ch - 'A' + 10 );

    return -1;
}

static int mv_gtw_get_cmdline_mac_addr(char **p_net_config, int idx)
{
    /* the MAC address should look like: 00:99:88:88:99:77 */
    /* that is, 6 two-digit numbers, separated by :        */
    /* 6 times two-digits, plus 5 colons, total: 17 characters */
    const int   exact_len = 17; 
    int         i = 0;
    int         syntax_err = 0;
    char	    *p_mac_addr = *p_net_config;

    /* check first 15 characters in groups of 3 characters at a time */
    for (i = 0; i < exact_len-2; i+=3) 
    {
	    if ( (mv_gtw_is_digit(**p_net_config) == 0) &&
	         (mv_gtw_is_digit(*(*p_net_config+1)) == 0) &&
	         ((*(*p_net_config+2)) == ':') )
	    {
	        (*p_net_config) += 3;
	    }
	    else {
	        syntax_err = 1;
	        break;
	    }
    }

    /* two characters remaining, must be two digits */
    if ( (mv_gtw_is_digit(**p_net_config) == 0) &&
         (mv_gtw_is_digit(*(*p_net_config+1)) == 0) )
    {
	    (*p_net_config) += 2;
    }
    else
	    syntax_err = 1;

    if (syntax_err == 0) {
        mvMacStrToHex(p_mac_addr, gtw_config.vlan_cfg[idx].macaddr);
        return 0;
    }
    printk("Syntax error while parsing MAC address from command line\n");
    return -EINVAL;
}

static void mv_gtw_update_curr_port_mask(char digit, unsigned int *curr_port_mask)
{
    if (digit == '0')
	    *curr_port_mask |= (1<<SWITCH_PORT_0);
    if (digit == '1')
	    *curr_port_mask |= (1<<SWITCH_PORT_1);
    if (digit == '2')
	    *curr_port_mask |= (1<<SWITCH_PORT_2);
    if (digit == '3')
	    *curr_port_mask |= (1<<SWITCH_PORT_3);
    if (digit == '4')
	    *curr_port_mask |= (1<<SWITCH_PORT_4);
}

static int mv_gtw_get_port_mask(char **p_net_config, int idx)
{
    /* the port mask should look like this: */
    /* example 1: 0 */
    /* example 2: 1:2:3:4 */
    /* that is, one or more one-digit numbers, separated with : */
    /* we have up to GTW_MAX_NUM_OF_IFS interfaces */

    unsigned int curr_port_mask = 0, i = 0;
    int syntax_err = 0;

    for (i = 0; i < GTW_MAX_NUM_OF_IFS; i++) 
    {
	    if (mv_gtw_is_digit(**p_net_config) == 0) 
        {
	        if (*(*p_net_config+1) == ':') 
            {
		        mv_gtw_update_curr_port_mask(**p_net_config, &curr_port_mask);
		        (*p_net_config) += 2;
	        }
	        else if (*(*p_net_config+1) == ')') 
            {
		        mv_gtw_update_curr_port_mask(**p_net_config, &curr_port_mask);
		        (*p_net_config)++;
		        break;
	        }
	        else {
		        syntax_err = 1;
		        break;
	        }
	    }
	    else {
	        syntax_err = 1;
	        break;
	    }
    }

    if (syntax_err == 0) {
	    gtw_config.vlan_cfg[idx].ports_mask = curr_port_mask;
	    return 0;
    }
    printk("Syntax error while parsing port mask from command line\n");
    return -EINVAL;
}

static int mv_gtw_get_mtu(char **p_net_config)
{
    /* the mtu value is constructed as follows: */
    /* mtu=value                                */
    unsigned int mtu;
    int syntax_err = 0;

    if(strncmp(*p_net_config,"mtu=",4) == 0)
    {
        *p_net_config += 4;
        mtu = 0;
        while((**p_net_config >= '0') && (**p_net_config <= '9'))
        {       
            mtu = (mtu * 10) + (**p_net_config - '0');
            *p_net_config += 1;
        }
        if(**p_net_config != '\0')
            syntax_err = 1;
    }
    else
    {
        syntax_err = 1;
    }

    if(syntax_err == 0)
    {
        gtw_config.mtu = mtu;
        printk("     o MTU set to %d.\n", mtu);
        return 0;
    }

    printk("Syntax error while parsing mtu value from command line\n");
    return -EINVAL;
}

static int mv_gtw_parse_net_config(char* cmdline)
{
    char    *p_net_config = cmdline;
    int     i = 0;
    int     status = 0;

    if (p_net_config == NULL)
	    return -EINVAL;

    for (i=0; (i<GTW_MAX_NUM_OF_IFS) && (*p_net_config != '\0'); i++) 
    {
        status = mv_gtw_check_open_bracket(&p_net_config);
	    if (status != 0)
	        break;
	    status = mv_gtw_get_cmdline_mac_addr(&p_net_config, i);
	    if (status != 0)
	        break;
	    status = mv_gtw_check_comma(&p_net_config);
	    if (status != 0)
	        break;
	    status = mv_gtw_get_port_mask(&p_net_config, i);
	    if (status != 0)
	        break;
	    status = mv_gtw_check_closing_bracket(&p_net_config);
	    if (status != 0)
	        break;

	    gtw_config.vlans_num++;

        /* If we have a comma after the closing bracket, then interface */
        /* definition is done.                                          */
        if(*p_net_config == ',')
            break;
    }

    if(*p_net_config != '\0')
    {
        status = mv_gtw_check_comma(&p_net_config);
        if (status == 0)
        {
            status = mv_gtw_get_mtu(&p_net_config);
        }
    }
    else
    {
        gtw_config.mtu = 1500;
        printk("     o Using default MTU %d\n", gtw_config.mtu);
    }
    
    /* at this point, we have parsed up to GTW_MAX_NUM_OF_IFS, and the mtu value */
    /* if the net_config string is not finished yet, then its format is invalid */
    if (*p_net_config != '\0')
    {
        printk("Gateway config string is too long: %s\n", p_net_config);
        status = -EINVAL;
    }
    return status;
}

GT_BOOL gtwReadMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int* value)
{
    unsigned long   flags;
    MV_STATUS       status;

    spin_lock_irqsave(&mii_lock, flags);

    status = mvEthPhyRegRead(portNumber, MIIReg , (unsigned short *)value);
    spin_unlock_irqrestore(&mii_lock, flags);

    if (status == MV_OK)
        return GT_TRUE;

    return GT_FALSE;
}


GT_BOOL gtwWriteMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int data)
{
    unsigned long   flags;
    MV_STATUS       status;

    spin_lock_irqsave(&mii_lock, flags);

    status = mvEthPhyRegWrite(portNumber, MIIReg, (unsigned short)data);

    spin_unlock_irqrestore(&mii_lock, flags);

    if (status == MV_OK)
        return GT_TRUE;
    return GT_FALSE;
} 

static int mv_gtw_set_port_based_vlan(unsigned int ports_mask)
{
    unsigned int p, pl;
    unsigned char cnt;
    GT_LPORT port_list[MAX_SWITCH_PORTS];

    for(p=0; p<qd_dev->numOfPorts; p++) {
	if( MV_BIT_CHECK(ports_mask, p) && (p != SWITCH_PORT_CPU) ) {
	    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("port based vlan, port %d: ",p));
	    for(pl=0,cnt=0; pl<qd_dev->numOfPorts; pl++) {
		if( MV_BIT_CHECK(ports_mask, pl) && (pl != p) ) {
		    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("%d ",pl));
		    port_list[cnt] = pl;
                    cnt++;
                }
	    }
            if( gvlnSetPortVlanPorts(qd_dev, p, port_list, cnt) != GT_OK) {
	        printk("gvlnSetPortVlanPorts failed\n");
                return -1;
            }
	    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("\n"));
        }
    }
    return 0;
}


static int mv_gtw_set_vlan_in_vtu(unsigned short vlan_id,unsigned int ports_mask)
{
    GT_VTU_ENTRY vtu_entry;
    unsigned int p;

    vtu_entry.vid = vlan_id;
    vtu_entry.DBNum = MV_GTW_VLANID_TO_GROUP(vlan_id);
    vtu_entry.vidPriOverride = GT_FALSE;
    vtu_entry.vidPriority = 0;
    vtu_entry.vidExInfo.useVIDFPri = GT_FALSE;
    vtu_entry.vidExInfo.vidFPri = 0;
    vtu_entry.vidExInfo.useVIDQPri = GT_FALSE;
    vtu_entry.vidExInfo.vidQPri = 0;
    vtu_entry.vidExInfo.vidNRateLimit = GT_FALSE;
    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("vtu entry: vid=0x%x, port ", vtu_entry.vid));
    for(p=0; p<qd_dev->numOfPorts; p++) {
        if(MV_BIT_CHECK(ports_mask, p)) {
	    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("%d ", p));
	    if(qd_dev->deviceId == GT_88E6061) {
                /* for 6061 device, no double/provider tag controlling on ingress. */
                /* therefore, we need to strip the tag on egress on all ports except cpu port */
                /* anyway, if we're using header mode no vlan-tag need to be added here */
		vtu_entry.vtuData.memberTagP[p] = MEMBER_EGRESS_UNMODIFIED;
	    }
	    else {
		vtu_entry.vtuData.memberTagP[p] = MEMBER_EGRESS_UNMODIFIED;
	    }
	}
	else {
	    vtu_entry.vtuData.memberTagP[p] = NOT_A_MEMBER;
	}
	vtu_entry.vtuData.portStateP[p] = 0;
    }
    if(gvtuAddEntry(qd_dev, &vtu_entry) != GT_OK) {
        printk("gvtuAddEntry failed\n");
        return -1;
    }

    ETH_DBG( ETH_DBG_LOAD|ETH_DBG_MCAST|ETH_DBG_VLAN, ("\n"));
    return 0;
}

int mv_gtw_set_mac_addr_to_switch(unsigned char *mac_addr, unsigned char db, unsigned int ports_mask, unsigned char op)
{
    GT_ATU_ENTRY mac_entry;
    struct mv_vlan_cfg *nc;

    /* validate db with VLAN id */
    nc = &gtw_config.vlan_cfg[db];
    if(MV_GTW_VLANID_TO_GROUP(nc->vlan_grp_id) != db) {
        printk("mv_gtw_set_mac_addr_to_switch (invalid db)\n");
	return -1;
    }

    memset(&mac_entry,0,sizeof(GT_ATU_ENTRY));

    mac_entry.trunkMember = GT_FALSE;
    mac_entry.prio = 0;
    mac_entry.exPrio.useMacFPri = GT_FALSE;
    mac_entry.exPrio.macFPri = 0;
    mac_entry.exPrio.macQPri = 0;
    mac_entry.DBNum = db;
    mac_entry.portVec = ports_mask;
    memcpy(mac_entry.macAddr.arEther,mac_addr,6);

    if(is_multicast_ether_addr(mac_addr))
	mac_entry.entryState.mcEntryState = GT_MC_STATIC;
    else
	mac_entry.entryState.ucEntryState = GT_UC_NO_PRI_STATIC;

    ETH_DBG(ETH_DBG_ALL, ("mv_gateway: db%d port-mask=0x%x, %02x:%02x:%02x:%02x:%02x:%02x ",
	    db, (unsigned int)mac_entry.portVec,
	    mac_entry.macAddr.arEther[0],mac_entry.macAddr.arEther[1],mac_entry.macAddr.arEther[2],
	    mac_entry.macAddr.arEther[3],mac_entry.macAddr.arEther[4],mac_entry.macAddr.arEther[5]));

    if((op == 0) || (mac_entry.portVec == 0)) {
        if(gfdbDelAtuEntry(qd_dev, &mac_entry) != GT_OK) {
	    printk("gfdbDelAtuEntry failed\n");
	    return -1;
        }
	ETH_DBG(ETH_DBG_ALL, ("deleted\n"));
    }
    else {
        if(gfdbAddMacEntry(qd_dev, &mac_entry) != GT_OK) {
	    printk("gfdbAddMacEntry failed\n");
	    return -1;
        }
	ETH_DBG(ETH_DBG_ALL, ("added\n"));
    }

    return 0;
}


#ifdef CONFIG_MV_GTW_QOS_VOIP
int mv_gtw_qos_tos_enable(void)
{
    printk("mv_gateway: VoIP QoS ON\n");
    netdev_max_backlog = 15;
    /*MAX_SOFTIRQ_RESTART = 1;*/
    mv_gtw_qos_tos_quota = 5;
    return 0;
}


int mv_gtw_qos_tos_disable(void)
{
    printk("mv_gateway VoIP QoS OFF\n");
    netdev_max_backlog = 300;
    /*MAX_SOFTIRQ_RESTART = 10;*/
    mv_gtw_qos_tos_quota = -1;
    return 0;
}
#endif /*CONFIG_MV_GTW_QOS*/


#ifdef CONFIG_MV_GTW_IGMP
int mv_gtw_enable_igmp(void)
{
    unsigned char p;

    ETH_DBG( ETH_DBG_IGMP, ("enabling L2 IGMP snooping\n"));

    /* enable IGMP snoop on all ports (except cpu port) */
    for(p=0; p<qd_dev->numOfPorts; p++) {
	if(p != SWITCH_PORT_CPU) {
	    if(gprtSetIGMPSnoop(qd_dev, p, GT_TRUE) != GT_OK) {
		printk("gprtSetIGMPSnoop failed\n");
		return -1;
	    }
	}
    }
    return -1;
}
#endif /* CONFIG_MV_GTW_IGMP */


int mv_gtw_net_setup(int port)
{
    struct mv_vlan_cfg  *nc;
    int                 i = 0;

    SWITCH_PORT_CPU = mvBoardSwitchCpuPortGet(port);
    SWITCH_PORT_0   = mvBoardSwitchPortGet(port, 0);
    SWITCH_PORT_1   = mvBoardSwitchPortGet(port, 1);
    SWITCH_PORT_2   = mvBoardSwitchPortGet(port, 2);
    SWITCH_PORT_3   = mvBoardSwitchPortGet(port, 3);
    SWITCH_PORT_4   = mvBoardSwitchPortGet(port, 4);

    /* sanity check */
    if ( (SWITCH_PORT_CPU < 0) || (SWITCH_PORT_0 < 0) || (SWITCH_PORT_1 < 0) || 
	     (SWITCH_PORT_2   < 0) || (SWITCH_PORT_3 < 0) || (SWITCH_PORT_4 < 0) ) {
	    printk("Unsupported platform.\n");
	    return -1;
    }

#ifdef CONFIG_MV_GTW_LINK_STATUS
    switch_irq = IRQ_GPP_START + mvBoardLinkStatusIrqGet(port);
#endif

    /* build the net config table */
    memset(&gtw_config, 0, sizeof(struct mv_gtw_config));

    if(cmdline != NULL) 
    {
	    printk("     o Using command line network interface configuration\n");
    }
    else
    {
	    printk("     o Using default network configuration, overriding boot MAC address\n");
	    cmdline = CONFIG_MV_GTW_CONFIG;
    }

	if (mv_gtw_parse_net_config(cmdline) < 0) 
    {
	    printk("Error parsing mv_net_config\n");
	    return -EINVAL;
    }           

    /* CPU port should always be enabled */
    switch_enabled_ports = (1 << SWITCH_PORT_CPU); 

    for(i=0, nc=&gtw_config.vlan_cfg[i]; i<gtw_config.vlans_num; i++, nc++) 
    {
	    /* VLAN ID */
	    nc->vlan_grp_id = MV_GTW_GROUP_VLAN_ID(i);
	    nc->ports_link = 0;
	    nc->header = cpu_to_be16(	(MV_GTW_VLANID_TO_GROUP(nc->vlan_grp_id) << 12) 
                    			| nc->ports_mask);
	    /* print info */
	    printk("     o mac_addr %02x:%02x:%02x:%02x:%02x:%02x, VID 0x%03x, port list: ", 
                nc->macaddr[0], nc->macaddr[1], nc->macaddr[2], 
		        nc->macaddr[3], nc->macaddr[4], nc->macaddr[5], nc->vlan_grp_id);

	    if(nc->ports_mask & (1<<SWITCH_PORT_CPU)) 
            printk("port-CPU ");
	    if(nc->ports_mask & (1<<SWITCH_PORT_0)) 
            printk("port-0 ");
	    if(nc->ports_mask & (1<<SWITCH_PORT_1)) 
            printk("port-1 ");
	    if(nc->ports_mask & (1<<SWITCH_PORT_2)) 
            printk("port-2 ");
	    if(nc->ports_mask & (1<<SWITCH_PORT_3)) 
            printk("port-3 ");
	    if(nc->ports_mask & (1<<SWITCH_PORT_4)) 
            printk("port-4 ");
	    printk("\n");

	    /* collect per-interface port_mask into a global port_mask, used for enabling the Switch ports */
	    switch_enabled_ports |= nc->ports_mask;
    }

    return 0;
}

static int mv_switch_init(int port)
{
    unsigned int        i, p;
    unsigned char       cnt;
    GT_LPORT            port_list[MAX_SWITCH_PORTS];
    struct mv_vlan_cfg  *nc;
    GT_JUMBO_MODE       jumbo_mode;

    printk("init switch layer... ");

    memset((char*)&qd_cfg,0,sizeof(GT_SYS_CONFIG));

    /* init config structure for qd package */
    qd_cfg.BSPFunctions.readMii   = gtwReadMiiWrap;
    qd_cfg.BSPFunctions.writeMii  = gtwWriteMiiWrap;
    qd_cfg.BSPFunctions.semCreate = NULL;
    qd_cfg.BSPFunctions.semDelete = NULL;
    qd_cfg.BSPFunctions.semTake   = NULL;
    qd_cfg.BSPFunctions.semGive   = NULL;
    qd_cfg.initPorts = GT_TRUE;
    qd_cfg.cpuPortNum = SWITCH_PORT_CPU;
    if (mvBoardSmiScanModeGet(port) == 1) {
        qd_cfg.mode.scanMode = SMI_MANUAL_MODE;
    }

    /* load switch sw package */
    if( qdLoadDriver(&qd_cfg, &qddev) != GT_OK) {
	printk("qdLoadDriver failed\n");
        return -1;
    }
    qd_dev = &qddev;

    ETH_DBG( ETH_DBG_LOAD, ("Device ID     : 0x%x\n",qd_dev->deviceId));
    ETH_DBG( ETH_DBG_LOAD, ("Base Reg Addr : 0x%x\n",qd_dev->baseRegAddr));
    ETH_DBG( ETH_DBG_LOAD, ("No. of Ports  : %d\n",qd_dev->numOfPorts));
    ETH_DBG( ETH_DBG_LOAD, ("CPU Ports     : %d\n",qd_dev->cpuPortNum));

    /* disable all ports */
    for(p=0; p<qd_dev->numOfPorts; p++) {
	    gstpSetPortState(qd_dev, p, GT_PORT_DISABLE);
    }

    /* initialize Switch according to Switch ID */
    switch (qd_dev->deviceId) 
    {
	    case GT_88E6131:
	    case GT_88E6108:
	        /* enable external ports */
	        ETH_DBG( ETH_DBG_LOAD, ("enable phy polling for external ports\n"));
	        if(gsysSetPPUEn(qd_dev, GT_TRUE) != GT_OK) {
		        printk("gsysSetPPUEn failed\n");
		        return -1;
	        }
	        /* Note: The GbE unit in SoCs connected to these switches does not support Marvell Header Mode */
	        /* so we always use VLAN tags here */
	        /* set cpu-port with ingress double-tag mode */
	        ETH_DBG( ETH_DBG_LOAD, ("cpu port ingress double-tag mode\n"));
	        if(gprtSetDoubleTag(qd_dev, SWITCH_PORT_CPU, GT_TRUE) != GT_OK) {
	            printk("gprtSetDoubleTag failed\n");
	            return -1;
	        }
	        /* set cpu-port with egrees add-tag mode */
	        ETH_DBG( ETH_DBG_LOAD, ("cpu port egrees add-tag mode\n"));
	        if(gprtSetEgressMode(qd_dev, SWITCH_PORT_CPU, GT_ADD_TAG) != GT_OK) {
		        printk("gprtSetEgressMode failed\n");
		    return -1;
	        }
	        /* config the switch to use the double tag data (relevant to cpu-port only) */
	        ETH_DBG( ETH_DBG_LOAD, ("use double-tag and remove\n"));
	        if(gsysSetUseDoubleTagData(qd_dev,GT_TRUE) != GT_OK) {
	            printk("gsysSetUseDoubleTagData failed\n");
	            return -1;
	        }
	        /* set cpu-port with 802.1q secured mode */
	        ETH_DBG( ETH_DBG_LOAD, ("cpu port-based 802.1q secure mode\n"));
	        if(gvlnSetPortVlanDot1qMode(qd_dev,SWITCH_PORT_CPU,GT_SECURE) != GT_OK) {
		        printk("gvlnSetPortVlanDot1qMode failed\n");
	            return -1;
	        }
	        break;
	
	    case GT_88E6065:
	        /* set CPU port number */
            if(gsysSetCPUPort(qd_dev, SWITCH_PORT_CPU) != GT_OK) {
	            printk("gsysSetCPUPort failed\n");
	            return -1;
	        }

	        if(gstatsFlushAll(qd_dev) != GT_OK)
    	        printk("gstatsFlushAll failed\n");

	        /* set all ports not to unmodify the vlan tag on egress */
	        for(i=0; i<qd_dev->numOfPorts; i++) 
            {
		        if(gprtSetEgressMode(qd_dev, i, GT_UNMODIFY_EGRESS) != GT_OK) {
		            printk("gprtSetEgressMode GT_UNMODIFY_EGRESS failed\n");
		            return -1;
		        }
	        }
	        if(gprtSetHeaderMode(qd_dev, SWITCH_PORT_CPU, GT_TRUE) != GT_OK) {
		        printk("gprtSetHeaderMode GT_TRUE failed\n");
		        return -1;
	        }

	        /* init counters */
	        if(gprtClearAllCtr(qd_dev) != GT_OK)
	            printk("gprtClearAllCtr failed\n");
	        if(gprtSetCtrMode(qd_dev, GT_CTR_ALL) != GT_OK)
	            printk("gprtSetCtrMode failed\n");

	        break;

	    case GT_88E6061: 
	        /* set CPU port number */
            if(gsysSetCPUPort(qd_dev, SWITCH_PORT_CPU) != GT_OK) {
	            printk("gsysSetCPUPort failed\n");
	            return -1;
	        }

	        /* set all ports not to unmodify the vlan tag on egress */
	        for(i=0; i<qd_dev->numOfPorts; i++) {
		        if(gprtSetEgressMode(qd_dev, i, GT_UNMODIFY_EGRESS) != GT_OK) {
		            printk("gprtSetEgressMode GT_UNMODIFY_EGRESS failed\n");
		            return -1;
		        }
	        }
	    
            if(gprtSetHeaderMode(qd_dev,SWITCH_PORT_CPU,GT_TRUE) != GT_OK) {
		        printk("gprtSetHeaderMode GT_TRUE failed\n");
		        return -1;
	        }   

	        /* init counters */
	        if(gprtClearAllCtr(qd_dev) != GT_OK)
	            printk("gprtClearAllCtr failed\n");
	        if(gprtSetCtrMode(qd_dev, GT_CTR_ALL) != GT_OK)
	            printk("gprtSetCtrMode failed\n");

	        break;

	    case GT_88E6161:
	    case GT_88E6165:
            if(gstatsFlushAll(qd_dev) != GT_OK) {
                printk("gstatsFlushAll failed\n");
            }

	        /* set all ports not to unmodify the vlan tag on egress */
	        for(i=0; i<qd_dev->numOfPorts; i++) {
		        if(gprtSetEgressMode(qd_dev, i, GT_UNMODIFY_EGRESS) != GT_OK) {
		            printk("gprtSetEgressMode GT_UNMODIFY_EGRESS failed\n");
		            return -1;
		        }
	        }
	        if(gprtSetHeaderMode(qd_dev,SWITCH_PORT_CPU,GT_TRUE) != GT_OK) {
		        printk("gprtSetHeaderMode GT_TRUE failed\n");
		        return -1;
	        }

            	/* Setup jumbo frames mode.                         */
            	/* 2 bytes for header Marvell header.               */
	        if((gtw_config.mtu) <= 1522)
                	jumbo_mode = GT_JUMBO_MODE_1522;
            	else if((gtw_config.mtu) <= 2048)
                	jumbo_mode = GT_JUMBO_MODE_2048;
            	else
                	jumbo_mode = GT_JUMBO_MODE_10240;

            	for(i=0; i<qd_dev->numOfPorts; i++) {
                	if(gsysSetJumboMode(qd_dev, i, jumbo_mode) != GT_OK) {
                    		printk("gsysSetJumboMode %d failed\n",jumbo_mode);
                    		return -1;
                	}
            	}
	        break;	

	    default:
	        printk("Unsupported Switch. Switch ID is 0x%X.\n",qd_dev->deviceId);
	        return -1;
    }

    /* set priorities rules */
    for(i=0; i<qd_dev->numOfPorts; i++) {
        /* default port priority to queue zero */
	    if(gcosSetPortDefaultTc(qd_dev, i, 0) != GT_OK)
	        printk("gcosSetPortDefaultTc failed (port %d)\n", i);
        
        /* enable IP TOS Prio */
	    if(gqosIpPrioMapEn(qd_dev, i, GT_TRUE) != GT_OK)
	        printk("gqosIpPrioMapEn failed (port %d)\n",i);
	
        /* set IP QoS */
	    if(gqosSetPrioMapRule(qd_dev, i, GT_FALSE) != GT_OK)
	        printk("gqosSetPrioMapRule failed (port %d)\n",i);
        
        /* disable Vlan QoS Prio */
	    if(gqosUserPrioMapEn(qd_dev, i, GT_FALSE) != GT_OK)
	        printk("gqosUserPrioMapEn failed (port %d)\n",i);
        
        /* Set force flow control to FALSE for all ports */
	    if(gprtSetForceFc(qd_dev, i, GT_FALSE) != GT_OK)
	        printk("gprtSetForceFc failed (port %d)\n",i);
    }

    /* The switch CPU port is not part of the VLAN, but rather connected by tunneling to each */
    /* of the VLAN's ports. Our MAC addr will be added during start operation to the VLAN DB  */
    /* at switch level to forward packets with this DA to CPU port.                           */
    ETH_DBG( ETH_DBG_LOAD, ("Enabling Tunneling on ports: "));
    for(i=0; i<qd_dev->numOfPorts; i++) {
	    if(i != SWITCH_PORT_CPU) {
	        if(gprtSetVlanTunnel(qd_dev, i, GT_TRUE) != GT_OK) {
		        printk("gprtSetVlanTunnel failed (port %d)\n",i);
		        return -1;
	        }
	        else {
		        ETH_DBG( ETH_DBG_LOAD, ("%d ",i));
	        }
	    }
    }
    ETH_DBG( ETH_DBG_LOAD, ("\n"));

    /* configure ports (excluding CPU port) for each network interface (VLAN): */
    for(i=0, nc=&gtw_config.vlan_cfg[i]; i<gtw_config.vlans_num; i++,nc++) {
        ETH_DBG( ETH_DBG_LOAD, ("vlan%d configuration (nc->ports_mask = 0x%08x) \n",
                                i, nc->ports_mask));
	    /* set port's defaul private vlan id and database number (DB per group): */
	    for(p=0; p<qd_dev->numOfPorts; p++) {
	        if( MV_BIT_CHECK(nc->ports_mask, p) && (p != SWITCH_PORT_CPU) ) {
		        ETH_DBG(ETH_DBG_LOAD,("port %d default private vlan id: 0x%x\n", p, MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p)));
		        if( gvlnSetPortVid(qd_dev, p, MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p)) != GT_OK ) {
			        printk("gvlnSetPortVid failed");
			        return -1;
		        }
		        if( gvlnSetPortVlanDBNum(qd_dev, p, MV_GTW_VLANID_TO_GROUP(nc->vlan_grp_id)) != GT_OK) {
		            printk("gvlnSetPortVlanDBNum failed\n");
		            return -1;
		        }
	        }
	    }

	    /* set port's port-based vlan (CPU port is not part of VLAN) */
        if(mv_gtw_set_port_based_vlan(nc->ports_mask & ~(1<<SWITCH_PORT_CPU)) != 0) {
	        printk("mv_gtw_set_port_based_vlan failed\n");
	    }

        /* set vtu with group vlan id (used in tx) */
        if(mv_gtw_set_vlan_in_vtu(nc->vlan_grp_id, nc->ports_mask | (1<<SWITCH_PORT_CPU)) != 0) {
	        printk("mv_gtw_set_vlan_in_vtu failed\n");
	    }

        /* set vtu with each port private vlan id (used in rx) */
 	    for(p=0; p<qd_dev->numOfPorts; p++) {
	        if(MV_BIT_CHECK(nc->ports_mask, p) && (p!=SWITCH_PORT_CPU)) {
                if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(nc->vlan_grp_id,p),
                                          nc->ports_mask & ~(1<<SWITCH_PORT_CPU)) != 0) {
		            printk("mv_gtw_set_vlan_in_vtu failed\n");
		        }
	        }
	    }
    }

    /* set cpu-port with port-based vlan to all other ports */
    ETH_DBG( ETH_DBG_LOAD, ("cpu port-based vlan:"));
    for(p=0,cnt=0; p<qd_dev->numOfPorts; p++) {
        if(p != SWITCH_PORT_CPU) {
	        ETH_DBG( ETH_DBG_LOAD, ("%d ",p));
            port_list[cnt] = p;
            cnt++;
        }
    }
    ETH_DBG( ETH_DBG_LOAD, ("\n"));
    if( gvlnSetPortVlanPorts(qd_dev, SWITCH_PORT_CPU, port_list, cnt) != GT_OK) {
        printk("gvlnSetPortVlanPorts failed\n");
        return -1;
    }

#ifdef CONFIG_ETH_FLOW_CONTROL
    /* Force flow control on CPU-port */
    if(gprtSetForceFc(qd_dev, SWITCH_PORT_CPU, GT_TRUE) != GT_OK)
        printk("gprtSetForceFc failed (port %d)\n", SWITCH_PORT_CPU);
#endif

#ifdef CONFIG_MV_GTW_QOS
    mv_gtw_set_qos_in_switch();
#endif

    if(gfdbFlush(qd_dev,GT_FLUSH_ALL) != GT_OK) {
	    printk("gfdbFlush failed\n");
    }

    /* done! enable all Switch ports according to the net config table */
    ETH_DBG( ETH_DBG_LOAD, ("enabling: ports "));
    for(p=0; p<qd_dev->numOfPorts; p++) {
	    if (MV_BIT_CHECK(switch_enabled_ports, p)) {
	        ETH_DBG( ETH_DBG_LOAD, ("%d ",p));
	        if(gstpSetPortState(qd_dev, p, GT_PORT_FORWARDING) != GT_OK) {
	            printk("gstpSetPortState failed\n");
	        }
	    }
    }
    ETH_DBG( ETH_DBG_LOAD, ("\n"));

#ifdef CONFIG_MV_GTW_LINK_STATUS
    /* Enable Phy Link Status Changed interrupt at Phy level for the all enabled ports */
    for(p=0; p<qd_dev->numOfPorts; p++) {
	    if(MV_BIT_CHECK(switch_enabled_ports, p) && (p != SWITCH_PORT_CPU)) {
	        if(gprtPhyIntEnable(qd_dev, p, (GT_LINK_STATUS_CHANGED)) != GT_OK) {
		        printk("gprtPhyIntEnable failed port %d\n", p);
	        }
	    }
    }

    if ((qd_dev->deviceId != GT_88E6161) && (qd_dev->deviceId != GT_88E6165)) {
    	if (switch_irq != -1) {
            if(eventSetActive(qd_dev, GT_PHY_INTERRUPT) != GT_OK) {
	    	    printk("eventSetActive failed\n");
            }
    	}
    }
    else {
	    GT_DEV_EVENT gt_event = {GT_DEV_INT_PHY, 0, 0x1F}; /* 0x1F is a bit mask for ports 0-4 */
	    if (switch_irq != -1) {
	        if(eventSetDevInt(qd_dev, &gt_event) != GT_OK) {
		        printk("eventSetDevInt failed\n");
	        }
	        if(eventSetActive(qd_dev, GT_DEVICE_INT) != GT_OK) {
    	    	printk("eventSetActive failed\n");
            }
	    }
    }
#endif /* CONFIG_MV_GTW_LINK_STATUS */

    /* Configure Ethernet related LEDs, currently according to Switch ID */
    switch (qd_dev->deviceId) {
	    case GT_88E6131:
	    case GT_88E6108:
            /* config LEDs: Bi-Color Mode-4:  */
            /* 1000 Mbps Link - Solid Green; 1000 Mbps Activity - Blinking Green */
            /* 100 Mbps Link - Solid Red; 100 Mbps Activity - Blinking Red */
            for(p=0; p<qd_dev->numOfPorts; p++) {	        
                if( (p != SWITCH_PORT_CPU) && (SWITCH_IS_PORT_CONNECTED(p)) ) {
			/* Configure Register 16 page 3 to 0x888F for mode 4 */
	                if(gprtSetPagedPhyReg(qd_dev,p,16,3,0x888F)) { 
		                printk("gprtSetPagedPhyReg failed (port=%d)\n", p);
	                }
			    /* Configure Register 17 page 3 to 0x4400 50% mixed LEDs */
		            if(gprtSetPagedPhyReg(qd_dev,p,17,3,0x4400)) {
		                printk("gprtSetPagedPhyReg failed (port=%d)\n", p);
	                }
	            }
            }
	        break;

	    case GT_88E6161:
	    case GT_88E6165:
		    break; /* do nothing */

	    default:
	        for(p=0; p<qd_dev->numOfPorts; p++) {
	            if( (p != SWITCH_PORT_CPU) && (SWITCH_IS_PORT_CONNECTED(p)) ) {
	                if(gprtSetPhyReg(qd_dev,p,22,0x1FFA)) { 
                        /* Configure Register 22 LED0 to 0xA for Link/Act */
	    	            printk("gprtSetPhyReg failed (port=%d)\n", p);
		            }
	            }
	        }
	        break;
    }
    printk("done\n");

    return 0;
}

#ifdef CONFIG_MV_GTW_QOS
static void mv_gtw_set_qos_in_switch(void)
{
    /* The ToS value must be represented as follow: "0xVV;0xYY..."        */
    unsigned char   tos = 0;
    char            *str;
    int             i;

    /* Clear all */
    for(i=0; i<=0x40; i++) {
        if(gcosSetDscp2Tc(qd_dev, i, 0) != GT_OK) {
	        printk("gcosSetDscp2Tc failed\n");
	    }
    }

#ifdef CONFIG_MV_GTW_QOS_VOIP
    /* VoIP support only one ToS value */
    str = CONFIG_MV_GTW_QOS_VOIP_TOS;
    tos = (mv_gtw_char_to_hex(str[2])<<4) + (mv_gtw_char_to_hex(str[3]));
    if(gcosSetDscp2Tc(qd_dev, tos>>2, MV_VOIP_PRIO) != GT_OK) {
	    printk("gcosSetDscp2Tc failed\n");
    }
    ETH_DBG( ETH_DBG_LOAD, ("VoIP ToS 0x%x goes to queue %d\n",tos,MV_VOIP_PRIO));
#endif /* CONFIG_MV_GTW_QOS_VOIP */

#ifdef CONFIG_MV_GTW_QOS_ROUTING
    /* Routing supports multiple ToS values */
    for(i=0;;i+=5) {
	    str = CONFIG_MV_GTW_QOS_ROUTING_TOS + i;
	    tos = (mv_gtw_char_to_hex(str[2])<<4) + (mv_gtw_char_to_hex(str[3]));
	    if(gcosSetDscp2Tc(qd_dev, tos>>2, MV_ROUTING_PRIO) != GT_OK) {
	        printk("gcosSetDscp2Tc failed\n");
	    }
	    ETH_DBG( ETH_DBG_LOAD, ("Routing ToS 0x%x goes to queue %d\n",tos,MV_ROUTING_PRIO));
	    if(str[4] != ';')
	        break;
    }
#endif /* CONFIG_MV_GTW_QOS_ROUTING */
}
#endif /* CONFIG_MV_GTW_QOS */


static struct net_device* mv_gtw_main_net_dev_get(void)
{
    int i;

    for(i=0; i<mv_net_devs_num; i++) {
        if(netif_running(mv_net_devs[i])) {
            return mv_net_devs[i];
	}
    }
    return NULL;
}

int mv_gtw_set_mac_addr( struct net_device *dev, void *p )
{
    struct mv_vlan_cfg *vlan_cfg = MV_NETDEV_VLAN(dev);
    struct sockaddr *addr = p;

    if(!is_valid_ether_addr(addr->sa_data))
	    return -EADDRNOTAVAIL;

    /* remove old mac addr from VLAN DB */
    mv_gtw_set_mac_addr_to_switch(dev->dev_addr,MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id),(1<<SWITCH_PORT_CPU),0);

    memcpy(dev->dev_addr, addr->sa_data, 6);

    /* add new mac addr to VLAN DB */
    mv_gtw_set_mac_addr_to_switch(dev->dev_addr,MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id),(1<<SWITCH_PORT_CPU),1);

    printk("mv_gateway: %s change mac address to %02x:%02x:%02x:%02x:%02x:%02x\n", 
        dev->name, *(dev->dev_addr), *(dev->dev_addr+1), *(dev->dev_addr+2), 
       *(dev->dev_addr+3), *(dev->dev_addr+4), *(dev->dev_addr+5));

    return 0;
}

void    mv_gtw_set_multicast_list(struct net_device *dev)
{
    struct dev_mc_list *curr_addr = dev->mc_list;
    struct mv_vlan_cfg *vlan_cfg = MV_NETDEV_VLAN(dev);
    int i;
    GT_ATU_ENTRY mac_entry;
    GT_BOOL found = GT_FALSE;
    GT_STATUS status;

    disable_irq(ETH_PORT0_IRQ_NUM);

    if((dev->flags & IFF_PROMISC) || (dev->flags & IFF_ALLMULTI)) {
	/* promiscuous mode - connect the CPU port to the VLAN (port based + 802.1q) */
	/*
	if(dev->flags & IFF_PROMISC)
	    printk("mv_gateway: setting promiscuous mode\n");
	if(dev->flags & IFF_ALLMULTI)
	    printk("mv_gateway: setting multicast promiscuous mode\n");
	*/
	mv_gtw_set_port_based_vlan(vlan_cfg->ports_mask|(1<<SWITCH_PORT_CPU));
	for(i=0; i<qd_dev->numOfPorts; i++) {
	    if(MV_BIT_CHECK(vlan_cfg->ports_mask, i) && (i!=SWITCH_PORT_CPU)) {
		if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(vlan_cfg->vlan_grp_id,i),vlan_cfg->ports_mask|(1<<SWITCH_PORT_CPU)) != 0) {
		    printk("mv_gtw_set_vlan_in_vtu failed\n");
		}
	   }
	}
    }
    else {
	/* not in promiscuous or allmulti mode - disconnect the CPU port to the VLAN (port based + 802.1q) */
	mv_gtw_set_port_based_vlan(vlan_cfg->ports_mask&(~(1<<SWITCH_PORT_CPU)));
	for(i=0; i<qd_dev->numOfPorts; i++) {
	    if(MV_BIT_CHECK(vlan_cfg->ports_mask, i) && (i!=SWITCH_PORT_CPU)) {
		if(mv_gtw_set_vlan_in_vtu(MV_GTW_PORT_VLAN_ID(vlan_cfg->vlan_grp_id,i),vlan_cfg->ports_mask&(~(1<<SWITCH_PORT_CPU))) != 0) {
		    printk("mv_gtw_set_vlan_in_vtu failed\n");
		}
	   }
	}
	if(dev->mc_count) {
	    /* accept specific multicasts */
	    for(i=0; i<dev->mc_count; i++, curr_addr = curr_addr->next) {
	        if (!curr_addr)
		    break;
		/* The Switch may already have information about this multicast address in      */
		/* its ATU. If this address is already in the ATU, use the existing port vector */
		/* ORed with the CPU port. Otherwise, just use the CPU port.                    */
		memset(&mac_entry,0,sizeof(GT_ATU_ENTRY));
		mac_entry.DBNum = MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id);
		memcpy(mac_entry.macAddr.arEther, curr_addr->dmi_addr, 6);
		status = gfdbFindAtuMacEntry(qd_dev, &mac_entry, &found);
		if ( (status != GT_OK) || (found != GT_TRUE) ) {
		    mv_gtw_set_mac_addr_to_switch(curr_addr->dmi_addr,
		    				  MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id),
						  (1<<SWITCH_PORT_CPU)|(vlan_cfg->ports_mask), 1);
		}
		else {
		    mv_gtw_set_mac_addr_to_switch(curr_addr->dmi_addr,
						  MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id),
						  (mac_entry.portVec | (1<<SWITCH_PORT_CPU)), 1);
		}
	    }
	}
    }
    enable_irq(ETH_PORT0_IRQ_NUM);
}
 

int mv_gtw_change_mtu(struct net_device *dev, int mtu)
{
	printk("mv_gateway does not support changing MTU at runtime.\n"); 
	return -EPERM;
} 


int mv_gtw_start( struct net_device *dev )
{
    mv_eth_priv		*priv = MV_ETH_PRIV(dev);
    struct 		    mv_vlan_cfg *vlan_cfg = MV_NETDEV_VLAN(dev);
    unsigned char	broadcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
    int			first_up_flag = 0;

    printk("mv_gateway: starting %s\n",dev->name);

    /* start upper layer */
    netif_carrier_on(dev);
    netif_wake_queue(dev);
    netif_poll_enable(dev);

    /* Add our MAC addr to the VLAN DB at switch level to forward packets with this DA */
    /* to CPU port by using the tunneling feature. The device is always in promisc mode.      */
    mv_gtw_set_mac_addr_to_switch(dev->dev_addr, MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id), (1<<SWITCH_PORT_CPU), 1);

    /* We also need to allow L2 broadcasts comming up for this interface */
    mv_gtw_set_mac_addr_to_switch(broadcast, MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id), 
					vlan_cfg->ports_mask|(1<<SWITCH_PORT_CPU), 1);

    if(priv->timer_flag == 0)
    {
        priv->timer.expires = jiffies + ((HZ*CONFIG_MV_ETH_TIMER_PERIOD)/1000); /*ms*/
        add_timer( &(priv->timer) );
        priv->timer_flag = 1;
    }

    if (priv->net_dev == NULL)
		first_up_flag = 1;
	
    priv->net_dev = mv_gtw_main_net_dev_get();

    if(first_up_flag) {
		/* connect to MAC port interrupt line */
    	if( request_irq( ETH_PORT_IRQ_NUM(priv->port), mv_eth_interrupt_handler, 
            (IRQF_DISABLED | IRQF_SAMPLE_RANDOM), "mv_gateway", priv->net_dev) ) 
		{
        	printk(KERN_ERR "failed to assign irq%d\n", ETH_PORT_IRQ_NUM(priv->port));
    	}

        /* unmask interrupts */
        mv_eth_unmask_interrupts(priv);
    }
    
    return 0;
}


int mv_gtw_stop( struct net_device *dev )
{
    mv_eth_priv		    *priv = MV_ETH_PRIV(dev);
    struct mv_vlan_cfg	*vlan_cfg = MV_NETDEV_VLAN(dev);

    printk("mv_gateway: stopping %s\n",dev->name);

    /* stop upper layer */
    netif_poll_disable(dev);
    netif_carrier_off(dev);
    netif_stop_queue(dev);

    /* stop switch from forwarding packets from this VLAN toward CPU port */
    if( gfdbFlushInDB(qd_dev, GT_FLUSH_ALL, MV_GTW_VLANID_TO_GROUP(vlan_cfg->vlan_grp_id)) != GT_OK) {
        printk("gfdbFlushInDB failed\n");
    }
    priv->net_dev = mv_gtw_main_net_dev_get();

    if(priv->net_dev == NULL)
    {
        mv_eth_mask_interrupts(priv);
    	priv->timer_flag = 0;
        del_timer(&priv->timer);
    }
    return 0;
}

#ifdef CONFIG_MV_GTW_LINK_STATUS
static void mv_gtw_update_link_status(unsigned int p, unsigned int link_up)
{
    struct mv_vlan_cfg  *vlan_cfg;
    int                 i = 0;
    unsigned int        prev_ports_link = 0;

    for(i=0; i<mv_net_devs_num; i++) {
	if (mv_net_devs[i] == NULL)
		break;
        vlan_cfg = MV_NETDEV_VLAN(mv_net_devs[i]);
	    if ( vlan_cfg != NULL) {

	        if ((vlan_cfg->ports_mask & (1 << p)) == 0)
		        continue;

	        prev_ports_link = vlan_cfg->ports_link;
 
	        if (link_up)
	            vlan_cfg->ports_link |= (1 << p);
	        else
		        vlan_cfg->ports_link &= ~(1 << p);

	        if ((vlan_cfg->ports_link & vlan_cfg->ports_mask) == 0) {
                    netif_carrier_off(mv_net_devs[i]);
                    netif_stop_queue(mv_net_devs[i]);
	        }
	        else if (prev_ports_link == 0) {
		        netif_carrier_on(mv_net_devs[i]);
                netif_wake_queue(mv_net_devs[i]);          
	        }
	    }
    }
}

static irqreturn_t mv_gtw_link_interrupt_handler(int irq , void *dev_id)
{
    unsigned short switch_cause, phy_cause, phys_port, p;

    if (switch_irq != -1 ) {
	    if ( (qd_dev->deviceId == GT_88E6161) || (qd_dev->deviceId == GT_88E6165) ) {
	        OUT GT_DEV_INT_STATUS devIntStatus;
            /* required to clear the interrupt, and updates phys_port */
	        geventGetDevIntStatus(qd_dev, &devIntStatus); 
	        phys_port = devIntStatus.phyInt & 0xFF;
	        if (phys_port)
		        switch_cause = GT_PHY_INTERRUPT;
	    }
	    else {
	        if(eventGetIntStatus(qd_dev, &switch_cause) != GT_OK)
	            switch_cause = 0;
	    }
    }
    else {
	    switch_cause = GT_PHY_INTERRUPT;
    }

    if(switch_cause & GT_PHY_INTERRUPT) {
	    /* If we're using a 6161/6165 Switch and using the Switch interrupt, we already have phys_port updated above */
	    /* If we're using any other Switch, or if we're using polling, we need to update phys_port now */
	    if ( (qd_dev->deviceId == GT_88E6161) || (qd_dev->deviceId == GT_88E6165)) {
		    if (switch_irq == -1) {
			    gprtGetPhyIntPortSummary(qd_dev,&phys_port);
			    phys_port |= 0x18; /* we cannot get indication for these ports in this method, so check them */
		    }
	    }
	    else {
		    /* not 6161 or 6165 */
        	gprtGetPhyIntPortSummary(qd_dev,&phys_port);
	    }

        for(p=0; p<qd_dev->numOfPorts; p++) {
            if (MV_BIT_CHECK(phys_port, p)) {
		        if(gprtGetPhyIntStatus(qd_dev,p,&phy_cause) == GT_OK) {
		            if(phy_cause & GT_LINK_STATUS_CHANGED) 
                    {
			            char *link=NULL, *duplex=NULL, *speed=NULL;
			            GT_BOOL flag;
			            GT_PORT_SPEED_MODE speed_mode;

			            if(gprtGetLinkState(qd_dev,p,&flag) != GT_OK) {
			                printk("gprtGetLinkState failed (port %d)\n",p);
			                link = "ERR";
			            }
                        else 
                            link = (flag)?"up":"down";

			            if(flag) {
			                if(gprtGetDuplex(qd_dev,p,&flag) != GT_OK) {
			                    printk("gprtGetDuplex failed (port %d)\n",p);
			                    duplex = "ERR";
			                }
                            else 
                                duplex = (flag)?"Full":"Half";

			                if(gprtGetSpeedMode(qd_dev,p,&speed_mode) != GT_OK) {
			                    printk("gprtGetSpeedMode failed (port %d)\n",p);
			                    speed = "ERR";
			                }
			                else {
			                    if (speed_mode == PORT_SPEED_1000_MBPS)
			                        speed = "1000Mbps";
			                    else if (speed_mode == PORT_SPEED_100_MBPS)
			                        speed = "100Mbps";
			                    else
			                        speed = "10Mbps";
			                }

			                mv_gtw_update_link_status(p, 1);

                            printk("Port %d: Link-%s, %s-duplex, Speed-%s.\n",
                                   mv_gtw_port2lport(p),link,duplex,speed);
                        }
			            else {
			                mv_gtw_update_link_status(p, 0);

			                printk("Port %d: Link-down\n",mv_gtw_port2lport(p));
			            }
		            }
		        }
	        }
	    }
    }

    if (switch_irq == -1 ) {
    	switch_link_timer.expires = jiffies + (HZ); /* 1 second */
    	add_timer(&switch_link_timer);
    }

    return IRQ_HANDLED;
}

static void mv_gtw_link_timer_function(unsigned long data)
{
    mv_gtw_link_interrupt_handler(switch_irq, NULL);
}
#endif /* CONFIG_MV_GTW_LINK_STATUS */


/*********************************************************** 
 * gtw_init_complete --                                    *
 *   complete all initializations relevant for Gateway.    *
 ***********************************************************/
void    mv_gtw_init_complete(mv_eth_priv* priv)
{
    mv_switch_init(priv->port);

    mv_eth_start_internals(priv, gtw_config.mtu);

    /* Extra initializations for Gateways */
#ifdef CONFIG_MV_GTW_QOS_VOIP
    /* set VoIP packets (marked with vlan-prio 3) to MAC queue 3 */
    if(mvEthVlanPrioRxQueue(priv->hal_priv, (MV_VOIP_PRIO<<1), MV_VOIP_PRIO) != MV_OK) 
    {
        printk(KERN_ERR "failed to prioritize VoIP VlanPrio=%d (queue %d)\n",
                (MV_VOIP_PRIO << 1), MV_VOIP_PRIO);
    }
#endif /* CONFIG_MV_GTW_QOS_VOIP */

#ifdef CONFIG_MV_GTW_QOS_ROUTING
    /* set Routing packets (marked with vlan-prio 2) to MAC queue 2 */
    if(mvEthVlanPrioRxQueue(priv->hal_priv, (MV_ROUTING_PRIO<<1), MV_ROUTING_PRIO) != MV_OK) 
    {
        printk(KERN_ERR "failed to prioritize Routing VlanPrio=%d (queue %d)\n",
                    (MV_ROUTING_PRIO<<1), MV_ROUTING_PRIO);
    }
#endif /* CONFIG_MV_GTW_QOS_ROUTING */

        mvEthHeaderModeSet(priv->hal_priv, MV_ETH_ENABLE_HEADER_MODE_PRI_2_1);
    
    /* Mask interrupts */
    mv_eth_mask_interrupts(priv);

#ifdef CONFIG_MV_GTW_IGMP
    /* Initialize the IGMP snooping handler */
    if(mv_gtw_igmp_snoop_init()) {
        printk("failed to init IGMP snooping handler\n");
    }
#endif

#ifdef CONFIG_MV_GTW_LINK_STATUS
    if (switch_irq != -1) {
        if(request_irq(switch_irq, mv_gtw_link_interrupt_handler, 
            (IRQF_DISABLED | IRQF_SAMPLE_RANDOM), "link status", NULL))
	    {
            printk(KERN_ERR "failed to assign irq%d\n", switch_irq);
	    }
    }
    else {
        memset( &switch_link_timer, 0, sizeof(struct timer_list) );
	    init_timer(&switch_link_timer);
        switch_link_timer.function = mv_gtw_link_timer_function;
        switch_link_timer.data = -1;
   	    switch_link_timer.expires = jiffies + (HZ); /* 1 second */
    	add_timer(&switch_link_timer);
    }
#endif /* CONFIG_MV_GTW_LINK_STATUS */
}

void mv_gtw_update_tx_skb(unsigned int *switch_info, struct net_device *dev, MV_PKT_INFO *pPktInfo)
{
	/* Note: this function currently supports only working with Marvell Header mode */
	struct mv_vlan_cfg  	*vlan_cfg = MV_NETDEV_VLAN(dev); 
	struct sk_buff      	*skb = (struct sk_buff *)pPktInfo->osInfo;
	MV_BUF_INFO             *p_buf_info_first = pPktInfo->pFrags;
	MV_BUF_INFO             *p_buf_info_last = (pPktInfo->pFrags + pPktInfo->numFrags - 1);


	/* check if we have place inside skb for the header */
	if(skb_headroom(skb) >= ETH_MV_HEADER_SIZE) {
		*(unsigned short *)((skb->data)-ETH_MV_HEADER_SIZE) = vlan_cfg->header;
		pPktInfo->pFrags[0].bufVirtPtr -= ETH_MV_HEADER_SIZE;
		pPktInfo->pFrags[0].dataSize += ETH_MV_HEADER_SIZE;
		pPktInfo->pktSize += ETH_MV_HEADER_SIZE;
	}
	else {
		printk("mv_gtw_update_tx_skb: insufficient skb headroom for Marvell Header\n");

		/* make room for one cell (safe because the array is big enough) */
		do {
	        	*(p_buf_info_last+1) = *p_buf_info_last;
        		p_buf_info_last--;
		} while(p_buf_info_last >= p_buf_info_first);

		/* the header (safe on stack) */
		*switch_info = vlan_cfg->header;
		p_buf_info_first->bufVirtPtr = (unsigned char *)switch_info;
		p_buf_info_first->dataSize = ETH_MV_HEADER_SIZE;

		/* count the new frags */
		pPktInfo->numFrags += 1;
		pPktInfo->pktSize += ETH_MV_HEADER_SIZE;
	}
}


struct net_device *mv_gtw_get_rx_dev(struct sk_buff *skb, unsigned int *if_num, int *port)
{
	unsigned char if_index;

	/* Note: this function currently supports only working with Marvell Header mode */

	/* use hedaer to find interface */

    	/* get DBNum[3:0] from high nibble of first byte of the header */
	if_index = ( (skb->data[0] & 0xF0) >> 4); 
    	/* get port number from the last nibble of the header */
    	if(port != NULL)
		*port = (skb->data[1] & 0x0F);	

	if(unlikely(if_index >= mv_net_devs_num)) {
		printk("mv_gtw_get_rx_dev: Error - got wrong interface index\n");
		return NULL;
    	}

    	if(if_num != NULL)
		*if_num = if_index;

	return (mv_net_devs[if_index]);
}
