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
#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/seq_file.h>

#include <asm/system.h>
#include <asm/dma.h>
#include <asm/io.h>

#include <linux/netdevice.h>
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mv_eth_proc.h"

#ifdef CONFIG_MV_ETH_NFP
#include "../nfp_mgr/mv_nfp_mgr.h"
#endif

#include "mv_netdev.h"

//#define MV_DEBUG
#ifdef MV_DEBUG
#define DP printk
#else
#define DP(fmt,args...)
#endif


/* global variables from 'regdump' */
static struct proc_dir_entry *mv_eth_tool;

static unsigned int port = 0, q = 0, weight = 0, status = 0, mac[6] = {0,};
static unsigned int policy =0, command = 0, packet = 0;
static unsigned int value = 0;

#ifdef CONFIG_MV_ETH_NFP
static unsigned int  dip, sip, inport, outport;
static unsigned int  da[6] = {0,}, sa[6] = {0,};
static unsigned int  db_type;
#endif /* CONFIG_MV_ETH_NFP */

void run_com_srq(void) 
{
    void* port_hndl = mvEthPortHndlGet(port);

    if(port_hndl == NULL)
        return;

    if(q >= MV_ETH_RX_Q_NUM)
	    q = -1;

    switch(packet) {
	case PT_BPDU:
		mvEthBpduRxQueue(port_hndl, q);
		break;
	case PT_ARP:
		mvEthArpRxQueue(port_hndl, q);
		break;
	case PT_TCP:
		mvEthTcpRxQueue(port_hndl, q);
		break;
	case PT_UDP:
		mvEthUdpRxQueue(port_hndl, q);
		break;
	default:
		printk("eth proc unknown packet type.\n");	
    }
	
}

extern void    		ethMcastAdd(int port, char* macStr, int queue);
void run_com_sq(void) {

    char mac_addr[20];

    if(q >= MV_ETH_RX_Q_NUM)
	    q = -1;
    
    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    ethMcastAdd(port, mac_addr, q);
}

extern void    	ethPortStatus (int port);
extern void    	ethPortQueues( int port, int rxQueue, int txQueue, int mode);
extern void    	ethPortMcast(int port);
extern void    	ethPortUcastShow(int port);
extern void    	ethPortRegs(int port);
extern void     ethTxPolicyRegs(int port);
extern void    	ethPortCounters(int port);
extern void 	ethPortRmonCounters(int port);

#ifdef CONFIG_MV_ETH_NFP_DUAL
extern void    eth_remote_port_status_print(int port, int mode);
#endif

void run_com_statis(void) {
	printk("\n\n#########################################################################################\n\n");
	switch(status) {
		case STS_PORT:
			printk("  PORT %d: GET ETH STATUS\n\n",port);
            mv_eth_status_print(port);
			ethPortStatus(port);
			break;

        case STS_PORT_MAC:
            ethPortUcastShow(port);
			ethPortMcast(port);
            break;

		case STS_PORT_Q:
			printk("  PORT %d: GET ETH STATUS ON Q %d\n\n",port,q);
			ethPortQueues(port, q, q, 1);
			break;

#if (MV_ETH_RX_Q_NUM > 1)
		case STS_PORT_RXP:
			printk("  PORT %d: GET ETH RX POLICY STATUS\n\n",port);
			printk("Not supported\n");
			break;
#endif /* MV_ETH_RX_Q_NUM > 1 */

#if (MV_ETH_TX_Q_NUM > 1)
		case STS_PORT_TXP:
			printk("  PORT %d: GET ETH TX POLICY STATUS\n\n",port);
			ethTxPolicyRegs(port);
			break;
#endif /* MV_ETH_TX_Q_NUM > 1 */

		case STS_PORT_REGS:
			printk("  PORT %d: GET ETH PORT REGS STATUS\n\n",port);
			ethPortRegs(port);
			break;

		case STS_PORT_MIB:
			ethPortCounters(port);
			ethPortRmonCounters(port);	
			break;

		case STS_PORT_STATIS:
			printk("  PORT %d: GET ETH STATISTIC STATUS\n\n",port);
			mv_eth_stats_print(port);
			break;

        case STS_NETDEV:
			mv_netdev_status_print(port);
            break;

#ifdef CONFIG_MV_ETH_NFP
		case STS_PORT_NFP_STATS:
			printk("  PORT %d: NFP statistics\n\n",port);
			mv_eth_nfp_stats_print(port);
			break;
#endif /* CONFIG_MV_ETH_NFP */

		default:
			printk(" Unknown status command \n");
	}
#ifdef CONFIG_MV_ETH_NFP_DUAL
    eth_remote_port_status_print(port, status);
#endif
}

int run_eth_com(const char *buffer) {

    int scan_count;
    scan_count = sscanf(buffer, ETH_CMD_STRING, ETH_SCANF_LIST);
    if( scan_count != ETH_LIST_LEN) {
	printk("eth command bad format %x != %x\n", scan_count, ETH_LIST_LEN );
	return 1;
    }
    switch(command) {

        case COM_TXDONE_Q:
            mv_eth_tx_done_quota = value;
            break;

#ifdef CONFIG_MV_SKB_REUSE
        case COM_SKB_REUSE:
            eth_skb_reuse_enable = value;
            break;
#endif
	default:
            printk(" Unknown ETH command \n");
    }
    return 0;
}

/* Giga Port commands */
int run_port_com(const char *buffer) {

	int scan_count;
    void*   port_hndl;

	scan_count = sscanf(buffer, PORT_CMD_STRING, PORT_SCANF_LIST);
    
	if( scan_count != PORT_LIST_LEN) {
		printk("eth port command bad format %x != %x\n", scan_count, PORT_LIST_LEN );
		return 1;
	}
    port_hndl = mvEthPortHndlGet(port);
    if(port_hndl == NULL)
        return 1;

    switch(command) {
       	case COM_RX_COAL:
           	mvEthRxCoalSet(mvEthPortHndlGet(port), value);
           	break;

       	case COM_TX_COAL:
           	mvEthTxCoalSet(mvEthPortHndlGet(port), value);
        	break;

#ifdef ETH_MV_TX_EN
        case COM_TX_EN:
            if(value > ETH_NUM_OF_RX_DESCR)
            {
                printk("Eth TX_EN command bad param: value=%d\n", value);
                return;
            }

            eth_tx_en_config(port, value);
            break;
#endif /* ETH_MV_TX_EN */

#if (MV_ETH_VERSION >= 4)
        case COM_EJP_MODE:
           	mvEthEjpModeSet(mvEthPortHndlGet(port), value);
           	break;
#endif /* (MV_ETH_VERSION >= 4) */

  		default:
			printk(" Unknown port command \n");
    	}
   	return 0;
}

#ifdef CONFIG_MV_ETH_NFP
int run_ip_rule_set_com(const char *buffer)
{
    int scan_count, i;
    MV_FP_RULE  ip_rule;
    MV_STATUS   status = MV_OK;

    scan_count = sscanf(buffer, IP_RULE_STRING, IP_RULE_SCANF_LIST);

    if( scan_count != IP_RULE_LIST_LEN) {
	printk("eth proc bad format %x != %x\n", scan_count, IP_RULE_LIST_LEN);
	return 1;
    }
    memset(&ip_rule, 0, sizeof(ip_rule));
    
    printk("run_ip_rule_set_com: dip=%08x, sip=%08x, inport=%d, outport=%d\n", 
            dip, sip, inport, outport);

    ip_rule.routingInfo.dstIp = MV_32BIT_BE(dip);
    ip_rule.routingInfo.srcIp = MV_32BIT_BE(sip);
    ip_rule.routingInfo.defGtwIp = MV_32BIT_BE(dip);
    ip_rule.routingInfo.inIfIndex = inport;
    ip_rule.routingInfo.outIfIndex = outport;
    ip_rule.routingInfo.aware_flags = 0;

    for(i=0; i<MV_MAC_ADDR_SIZE; i++)
    {
        ip_rule.routingInfo.dstMac[i] = (MV_U8)(da[i] & 0xFF);
        ip_rule.routingInfo.srcMac[i] = (MV_U8)(sa[i] & 0xFF);;
    }
    ip_rule.mgmtInfo.actionType = MV_FP_ROUTE_CMD;
    ip_rule.mgmtInfo.ruleType = MV_FP_STATIC_RULE;

    status = fp_rule_set(&ip_rule);
    if(status != MV_OK)
    {
        printk("fp_rule_set FAILED: status=%d\n", status);
    }
    return status;
}

int run_ip_rule_del_com(const char *buffer)
{
    int scan_count;
    MV_STATUS status = MV_OK;

    scan_count = sscanf(buffer, IP_RULE_DEL_STRING, IP_RULE_DEL_SCANF_LIST);

    if( scan_count != IP_RULE_DEL_LIST_LEN) {
	printk("eth proc bad format %x != %x\n", scan_count, IP_RULE_DEL_LIST_LEN);
	return 1;
    }

    status = fp_rule_delete(MV_32BIT_BE(sip), MV_32BIT_BE(dip), MV_FP_STATIC_RULE);
    if(status != MV_OK)
    {
        printk("fp_rule_delete FAILED: status=%d\n", status);
    }
    return status;
}

int run_fp_db_print_com(const char *buffer)
{
    int scan_count;
    MV_STATUS status = MV_OK;

    scan_count = sscanf(buffer, FP_DB_PRINT_STRING, FP_DB_PRINT_SCANF_LIST);

    if( scan_count != FP_DB_PRINT_LIST_LEN) {
	    printk("eth proc bad format %x != %x\n", scan_count, FP_DB_PRINT_LIST_LEN);
	    return 1;
    }

    if (db_type == DB_ROUTING)
	    status = fp_rule_db_print(MV_FP_DATABASE);
#ifdef CONFIG_MV_ETH_NFP_NAT_SUPPORT 
    else if (db_type == DB_NAT)
	    status = fp_nat_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_NAT_SUPPORT */
#ifdef CONFIG_MV_ETH_NFP_FDB_SUPPORT
    else if (db_type == DB_FDB)
            status = fp_fdb_db_print(MV_FP_DATABASE);
#endif /* CONFIG_MV_ETH_NFP_FDB_SUPPORT */
    else {
	    printk("Failed to print rule database: unknown DB type\n");
	    return 1;
    }
    return status;
}
#endif /* CONFIG_MV_ETH_NFP */

int run_com_general(const char *buffer) {

	int scan_count;

	scan_count = sscanf(buffer, PROC_STRING, PROC_SCANF_LIST);

	if( scan_count != LIST_LEN) {
		printk("eth proc bad format %x != %x\n", scan_count, LIST_LEN );
		return 1;
	}

	switch(command){
		case COM_SRQ:
			DP(" Port %x: Got SRQ command Q %x and packet type is %x <bpdu/arp/tcp/udp> \n",port,q,packet);
			run_com_srq();
			break;
		case COM_SQ:
			DP(" Port %x: Got SQ command Q %x mac %2x:%2x:%2x:%2x:%2x:%2x\n",port, q, 
				mac[0],  mac[1],  mac[2],  mac[3],  mac[4],  mac[5]);
			run_com_sq();
			break;

#if (MV_ETH_RX_Q_NUM > 1)
		case COM_SRP:
			DP(" Port %x: Got SRP command Q %x policy %x <Fixed/WRR> \n",port,q,policy); 
            printk("Not supported\n");
			break;
		case COM_SRQW:
			DP(" Port %x: Got SQRW command Q %x weight %x \n",port,q,weight);
			printk("Not supported\n");
			break;
		case COM_STP:
			DP("STP cmd - Unsupported: Port %x Q %x policy %x <WRR/FIXED> weight %x\n",port,q,policy,weight); 
			break;
#endif /* MV_ETH_RX_Q_NUM > 1 */

		case COM_STS:
			DP("  Port %x: Got STS command status %x\n",port,status);
			run_com_statis();
			break;
		default:
			printk("eth proc unknown command.\n");
	}
  	return 0;
}

int mv_eth_tool_write (struct file *file, const char *buffer,
                      unsigned long count, void *data) {

	sscanf(buffer,"%x",&command);

	switch (command) {
		case COM_RX_COAL:
		case COM_TX_COAL:
        case COM_TX_EN:
        case COM_EJP_MODE:
			run_port_com(buffer);
			break;
		case COM_TXDONE_Q:
        case COM_SKB_REUSE:
			run_eth_com(buffer);
			break;

#ifdef CONFIG_MV_ETH_NFP
		case COM_IP_RULE_SET:
			run_ip_rule_set_com(buffer);
			break;
		case COM_IP_RULE_DEL:
			run_ip_rule_del_com(buffer);
			break;
		case COM_FP_DISABLE:
			fp_mgr_disable();
			break;
		case COM_FP_ENABLE:
			fp_mgr_enable();
			break;
		case COM_FP_STATUS:
			fp_mgr_status();
			break;
		case COM_FP_PRINT:
			run_fp_db_print_com(buffer);
			break;
#endif /* CONFIG_MV_ETH_NFP */

		default:
			run_com_general(buffer);
			break;
	}
	return count;
}

static int proc_calc_metrics(char *page, char **start, off_t off,
                                 int count, int *eof, int len)
{
        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count) len = count;
        if (len<0) len = 0;
        return len;
}



int mv_eth_tool_read (char *page, char **start, off_t off,
                            int count, int *eof, void *data) {
	unsigned int len = 0;

	//len  = sprintf(page, "\n");
	//len += sprintf(page+len, "\n");
	
   	return proc_calc_metrics(page, start, off, count, eof, len);
}



int __init start_mv_eth_tool(void)
{
  mv_eth_tool = proc_net_create(FILE_NAME , 0666 , NULL);
  mv_eth_tool->read_proc = mv_eth_tool_read;
  mv_eth_tool->write_proc = mv_eth_tool_write;
  mv_eth_tool->nlink = 1;
  return 0;
}

module_init(start_mv_eth_tool);
