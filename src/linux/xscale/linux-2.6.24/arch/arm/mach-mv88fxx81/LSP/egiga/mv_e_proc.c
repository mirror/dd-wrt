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
#include <linux/autoconf.h>
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
/*
#include <linux/irq.h>
#include <asm/page.h>
#include <linux/smp.h>
#include <asm/time.h>
#include <asm/hardirq.h>
#include <asm/machdep.h>
#include <asm/prom.h>
#include <asm/smp.h>
#include <asm/todc.h>
#include <asm/bootinfo.h>
#include <marvell.h>
*/

#include <linux/netdevice.h>
#include "mvCtrlEnvLib.h"
#include "mv_e_proc.h"
#include "mvEthPolicy.h"

//#define MV_DEBUG
#ifdef MV_DEBUG
#define DP printk
#else
#define DP(fmt,args...)
#endif


/* global variables from 'regdump' */
static struct proc_dir_entry *mv_eth_tool;

static unsigned int port = 0, q = 0, weight = 0, status = 0, mac[6] = {0,};
static unsigned int policy =0, direct = 0,command = 0, packett = 0;

extern void    ethBpduRxQ(int port, int bpduQueue);
extern void    ethArpRxQ(int port, int bpduQueue);
extern void    ethTcpRxQ(int port, int bpduQueue);
extern void    ethUdpRxQ(int port, int bpduQueue);
void run_com_srq(void) {
    if(q >= MV_ETH_RX_Q_NUM)
	q = -1;

    switch(packett) {
	case PT_BPDU:
		ethBpduRxQ(port, q);
		break;
	case PT_ARP:
		ethArpRxQ(port, q);
		break;
	case PT_TCP:
		ethTcpRxQ(port, q);
		break;
	case PT_UDP:
		ethUdpRxQ(port, q);
		break;
	default:
		printk("egiga proc unknown packet type.\n");	
    }
	
}

extern void    		ethTxPolDA(int port, char* macStr, int txQ, char* headerHexStr);
extern void    		ethMcastAdd(int port, char* macStr, int queue);
void run_com_sq(void) {

    char mac_addr[20];

    if(q >= MV_ETH_RX_Q_NUM)
	q = -1;
    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

    switch(direct) {
	case TX:
		ethTxPolDA(port, mac_addr, q, NULL);
		break;
	case RX:
		ethMcastAdd(port, mac_addr, q);
		break;
	default:
		printk("egiga proc unknown direction.\n");	
    }
}

extern void    ethRxPolMode(int port, MV_ETH_PRIO_MODE prioMode);
void run_com_srp(void) {
	MV_ETH_PRIO_MODE hal_policy;

	if( policy == FIXED)
		hal_policy = MV_ETH_PRIO_FIXED;
	else
		hal_policy = MV_ETH_PRIO_WRR;

	ethRxPolMode(port,hal_policy);
}

extern void       ethRxPolQ(int port, int rxQueue, int rxQuota);
void run_com_srqw(void) {
	ethRxPolQ(port, q, weight);
}

extern void    ethTxQ(int port, int txQueue, MV_ETH_PRIO_MODE txPrioMode, int txQuota);
void run_com_stp(void) {
	MV_ETH_PRIO_MODE hal_policy;

	if( policy == FIXED)
		hal_policy = MV_ETH_PRIO_FIXED;
	else
		hal_policy = MV_ETH_PRIO_WRR;

	ethTxQ(port, q, hal_policy, weight);
}

extern void 	print_egiga_stat( unsigned int port);
extern void    	ethPortStatus (int port);
extern void    	ethPortQueues( int port, int rxQueue, int txQueue, int mode);
extern void    	ethRxPolicy( int port);
extern void    	ethTxPolicy( int port);
extern void    	ethPortMcast(int port);
extern void    	ethPortRegs(int port);
extern void    	ethPortCounters(int port);
extern void 	ethPortRmonCounters(int port);

void run_com_statis(void) {
	printk("\n\n#########################################################################################\n\n");
	switch(status) {
		case STS_PORT:
			printk("  PORT %d: GET EGIGA STATUS\n\n",port);
			ethPortStatus(port);
			break;
		case STS_PORT_Q:
			printk("  PORT %d: GET EGIGA STATUS ON Q %d\n\n",port,q);
			ethPortQueues(port, q, q, 1);
			break;
		case STS_PORT_RXP:
			printk("  PORT %d: GET EGIGA RX POLICY STATUS\n\n",port);
			ethRxPolicy(port);
			ethPortMcast(port);
			break;
		case STS_PORT_TXP:
			printk("  PORT %d: GET EGIGA TX POLICY STATUS\n\n",port);
			ethTxPolicy(port);
			break;
		case STS_PORT_REGS:
			printk("  PORT %d: GET EGIGA PORT REGS STATUS\n\n",port);
			ethPortRegs(port);
			break;
		case STS_PORT_MIB:
			ethPortCounters(port);
			ethPortRmonCounters(port);	
			break;
		case STS_PORT_STATIS:
			printk("  PORT %d: GET EGIGA STATISTIC STATUS\n\n",port);
			print_egiga_stat(port);
			break;
		default:
			printk(" Unknown status command \n");
	}
}

int run_com_general(const char *buffer) {

	int scan_count;

	scan_count = sscanf(buffer, PROC_STRING, PROC_SCANF_LIST);

	if( scan_count != LIST_LEN) {
		printk("egiga proc bad format %x != %x\n", scan_count, LIST_LEN );
		return 1;
	}

#ifndef INCLUDE_MULTI_QUEUE
	printk("\n Be carefull egiga is compiled without MULTI Q!! \n");
#endif	
	switch(command){
		case COM_SRQ:
			DP(" Port %x: Got SRQ command Q %x and packet type is %x <bpdu/arp/tcp/udp> \n",port,q,packett);
			run_com_srq();
			break;
		case COM_SQ:
			DP(" Port %x: Got SQ command Q %x direction %x <Rx/Tx> mac %2x:%2x:%2x:%2x:%2x:%2x\n",port, q, direct,
				mac[0],  mac[1],  mac[2],  mac[3],  mac[4],  mac[5]);
			run_com_sq();
			break;
		case COM_SRP:
			DP(" Port %x: Got SRP command Q %x policy %x <Fixed/WRR> \n",port,q,policy); 
			run_com_srp();
			break;
		case COM_SRQW:
			DP(" Port %x: Got SQRW command Q %x weight %x \n",port,q,weight);
			run_com_srqw();
			break;
		case COM_STP:
			DP(" Port %x: Got STP command Q %x policy %x <WRR/FIXED> weight %x\n",port,q,policy,weight); 
			run_com_stp();
			break;
		case COM_STS:
			DP("  Port %x: Got STS command status %x\n",port,status);
			run_com_statis();
			break;
		default:
			printk("egiga proc unknown command.\n");
	}
  	return 0;
}

#ifdef CONFIG_MV_ETH_HEADER
extern void run_com_header(const char *buffer);
#endif

int mv_eth_tool_write (struct file *file, const char *buffer,
                      unsigned long count, void *data) {

        sscanf(buffer,"%x",&command);
#ifdef CONFIG_MV_ETH_HEADER
        if(command == COM_HEAD)
                run_com_header(buffer);
        else
#endif
                run_com_general(buffer);

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
