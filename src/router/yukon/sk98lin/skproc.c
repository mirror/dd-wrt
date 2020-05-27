/******************************************************************************
 *
 * Name:	skproc.c
 * Project:	Yukon/Yukon2, PCI Gigabit Ethernet Adapter
 * Purpose:	Functions to display statictic data
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect GmbH.
 *	(C)Copyright 2002-2012 Marvell.
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	Address all question to: support@marvell.com
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 *****************************************************************************/

#include <linux/proc_fs.h>
#include <linux/module.h>

#ifndef SK98LIN_VMESX5
#include <linux/seq_file.h>
#endif

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skversion.h"

extern struct SK_NET_DEVICE *SkGeRootDev;

/******************************************************************************
 *
 * Local Function Prototypes and Local Variables
 *
 *****************************************************************************/

static int sk_proc_print(void *writePtr, char *format, ...);
static void sk_gen_browse(void *buffer);
static int len;

static int sk_seq_show(struct seq_file *seq, void *v);
static int sk_proc_open(struct inode *inode, struct file *file);
struct file_operations sk_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= sk_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
struct net_device *currDev = NULL;

/*****************************************************************************
 *
 * 	sk_gen_browse -generic  print "summaries" entry
 *
 * Description:
 *	This function fills the proc entry with statistic data about
 *	the ethernet device.
 *
 * Returns:	N/A
 *
 */
static void sk_gen_browse(
void *buffer)  /* buffer where the statistics will be stored in */
{
	struct SK_NET_DEVICE	*SkgeProcDev = SkGeRootDev;
	struct SK_NET_DEVICE	*next;
	SK_BOOL			DisableStatistic = 0;
	SK_PNMI_STRUCT_DATA 	*pPnmiStruct;
	SK_PNMI_STAT		*pPnmiStat;
	unsigned long		Flags;
	unsigned int		Size;
	DEV_NET			*pNet;
	SK_AC			*pAC;
	char			sens_msg[50];
	int 			card_type;
	int			MaxSecurityCount = 0;
	int 			t;
	int 			i;

	while (SkgeProcDev) {
		MaxSecurityCount++;
		if (MaxSecurityCount > 100) {
			printk("Max limit for sk_proc_read security counter!\n");
			return;
		}

		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			pNet = (DEV_NET*) SkgeProcDev->priv;
        	#else
                	pNet = netdev_priv(SkgeProcDev);
#endif
		pAC = pNet->pAC;
		next = pAC->Next;
		pPnmiStruct = &pAC->PnmiStruct;
		/* NetIndex in GetStruct is now required, zero is only dummy */

		for (t=pAC->GIni.GIMacsFound; t > 0; t--) {
			if ((pAC->GIni.GIMacsFound == 2) && pAC->RlmtNets == 1)
				t--;

			spin_lock_irqsave(&pAC->SlowPathLock, Flags);
			Size = SK_PNMI_STRUCT_SIZE;
			DisableStatistic = 0;
			if (pAC->BoardLevel == SK_INIT_DATA) {
				SK_MEMCPY(&(pAC->PnmiStruct), &(pAC->PnmiBackup), sizeof(SK_PNMI_STRUCT_DATA));
				if (pAC->DiagModeActive == DIAG_NOTACTIVE) {
					pAC->Pnmi.DiagAttached = SK_DIAG_IDLE;
				}
			} else {
				SkPnmiGetStruct(pAC, pAC->IoBase, pPnmiStruct, &Size, t-1);
			}
			spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
			if (strcmp(pAC->dev[t-1]->name, currDev->name) == 0) {
				if (!pAC->GIni.GIYukon32Bit)
					card_type = 64;
				else
					card_type = 32;

				pPnmiStat = &pPnmiStruct->Stat[0];
				len = sk_proc_print(buffer,
					"\nDetailed statistic for device %s\n",
					pAC->dev[t-1]->name);
				len += sk_proc_print(buffer,
					"=======================================\n");

				/* Board statistics */
				len += sk_proc_print(buffer,
					"\nBoard statistics\n\n");
				len += sk_proc_print(buffer,
					"Card name                      %s\n",
					pAC->DeviceStr);
				len += sk_proc_print(buffer,
					"Vendor/Device ID               %x/%x\n",
					pAC->PciDev->vendor,
					pAC->PciDev->device);
				len += sk_proc_print(buffer,
					"Card type (Bit)                %d\n",
					card_type);

				if (pAC->DynIrqModInfo.IntModTypeSelect == C_INT_MOD_STATIC) {
					len += sk_proc_print(buffer,
					"Interrupt Moderation           static (%d ints/sec)\n",
					pAC->DynIrqModInfo.MaxModIntsPerSec);
				} else if (pAC->DynIrqModInfo.IntModTypeSelect == C_INT_MOD_DYNAMIC) {
					len += sk_proc_print(buffer,
					"Interrupt Moderation           dynamic (%d ints/sec)\n",
					pAC->DynIrqModInfo.MaxModIntsPerSec);
				} else {
					len += sk_proc_print(buffer,
					"Interrupt Moderation           disabled\n");
				}

				if (pAC->GIni.GIPciBus == SK_PEX_BUS) {
					len += sk_proc_print(buffer,
						"Bus type                       PCI-Express\n");
					len += sk_proc_print(buffer,
						"Bus width (Lanes)              %d\n",
						pAC->GIni.GIPexWidth);
				} else {
					if (pAC->GIni.GIPciBus == SK_PCIX_BUS) {
						len += sk_proc_print(buffer,
							"Bus type                       PCI-X\n");
						if (pAC->GIni.GIPciMode == PCI_OS_SPD_X133) {
							len += sk_proc_print(buffer,
								"Bus speed (MHz)                133\n");
						} else if (pAC->GIni.GIPciMode == PCI_OS_SPD_X100) {
							len += sk_proc_print(buffer,
								"Bus speed (MHz)                100\n");
						} else if (pAC->GIni.GIPciMode == PCI_OS_SPD_X66) {
							len += sk_proc_print(buffer,
								"Bus speed (MHz)                66\n");
						} else {
							len += sk_proc_print(buffer,
								"Bus speed (MHz)                33\n");
						}
					} else {
						len += sk_proc_print(buffer,
							"Bus type                       PCI\n");
						len += sk_proc_print(buffer,
							"Bus speed (MHz)                %d\n",
							pPnmiStruct->BusSpeed);
					}
					len += sk_proc_print(buffer,
						"Bus width (Bit)                %d\n",
						pPnmiStruct->BusWidth);
				}

				len += sk_proc_print(buffer,
					"Driver version                 %s (%s)\n",
					VER_STRING, PATCHLEVEL);
				len += sk_proc_print(buffer,
					"Driver release date            %s\n",
					pAC->Pnmi.pDriverReleaseDate);
				len += sk_proc_print(buffer,
					"Hardware revision              v%d.%d\n",
					(pAC->GIni.GIPciHwRev >> 4) & 0x0F,
					pAC->GIni.GIPciHwRev & 0x0F);

				if (!netif_running(pAC->dev[t-1])) {
					len += sk_proc_print(buffer,
						"\n      Device %s is down.\n"
						"      Therefore no statistics are available.\n"
						"      After bringing the device up (ifconfig)"
						" statistics will\n"
						"      be displayed.\n",
						pAC->dev[t-1]->name);
					DisableStatistic = 1;
				}

				/* Display only if statistic info available */
				/* Print sensor informations */
				if (!DisableStatistic) {
					for (i=0; i < pAC->I2c.MaxSens; i ++) {
						/* Check type */
						switch (pAC->I2c.SenTable[i].SenType) {
						case 1:
							strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
							strcat(sens_msg, " (C)");
							len += sk_proc_print(buffer,
								"%-25s      %d.%02d\n",
								sens_msg,
								pAC->I2c.SenTable[i].SenValue / 10,
								pAC->I2c.SenTable[i].SenValue %
								10);

							strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
							strcat(sens_msg, " (F)");
							len += sk_proc_print(buffer,
								"%-25s      %d.%02d\n",
								sens_msg,
								((((pAC->I2c.SenTable[i].SenValue)
								*10)*9)/5 + 3200)/100,
								((((pAC->I2c.SenTable[i].SenValue)
								*10)*9)/5 + 3200) % 10);
							break;
						case 2:
							strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
							strcat(sens_msg, " (V)");
							len += sk_proc_print(buffer,
								"%-25s      %d.%03d\n",
								sens_msg,
								pAC->I2c.SenTable[i].SenValue / 1000,
								pAC->I2c.SenTable[i].SenValue % 1000);
							break;
						case 3:
							strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
							strcat(sens_msg, " (rpm)");
							len += sk_proc_print(buffer,
								"%-25s      %d\n",
								sens_msg,
								pAC->I2c.SenTable[i].SenValue);
							break;
						default:
							break;
						}
					}

					/*Receive statistics */
					len += sk_proc_print(buffer,
					"\nReceive statistics\n\n");

					len += sk_proc_print(buffer,
						"Received bytes                 %Lu\n",
						(unsigned long long) pPnmiStat->StatRxOctetsOkCts);
					len += sk_proc_print(buffer,
						"Received packets               %Lu\n",
						(unsigned long long) pPnmiStat->StatRxOkCts);
#if 0
					if (pAC->GIni.GP[0].PhyType == SK_PHY_XMAC &&
						pAC->HWRevision < 12) {
						pPnmiStruct->InErrorsCts = pPnmiStruct->InErrorsCts -
							pPnmiStat->StatRxShortsCts;
						pPnmiStat->StatRxShortsCts = 0;
					}
#endif
					if (pAC->dev[t-1]->mtu > 1500)
						pPnmiStruct->InErrorsCts = pPnmiStruct->InErrorsCts -
							pPnmiStat->StatRxTooLongCts;

					len += sk_proc_print(buffer,
						"Receive errors                 %Lu\n",
						(unsigned long long) pPnmiStruct->InErrorsCts);
					len += sk_proc_print(buffer,
						"Receive dropped                %Lu\n",
						(unsigned long long) pPnmiStruct->RxNoBufCts);
					len += sk_proc_print(buffer,
						"Received multicast             %Lu\n",
						(unsigned long long) pPnmiStat->StatRxMulticastOkCts);
#ifdef ADVANCED_STATISTIC_OUTPUT
					len += sk_proc_print(buffer,
						"Receive error types\n");
					len += sk_proc_print(buffer,
						"   length                      %Lu\n",
						(unsigned long long) pPnmiStat->StatRxRuntCts);
					len += sk_proc_print(buffer,
						"   buffer overflow             %Lu\n",
						(unsigned long long) pPnmiStat->StatRxFifoOverflowCts);
					len += sk_proc_print(buffer,
						"   bad crc                     %Lu\n",
						(unsigned long long) pPnmiStat->StatRxFcsCts);
					len += sk_proc_print(buffer,
						"   framing                     %Lu\n",
						(unsigned long long) pPnmiStat->StatRxFramingCts);
					len += sk_proc_print(buffer,
						"   missed frames               %Lu\n",
						(unsigned long long) pPnmiStat->StatRxMissedCts);

					if (pAC->dev[t-1]->mtu > 1500)
						pPnmiStat->StatRxTooLongCts = 0;

					len += sk_proc_print(buffer,
						"   too long                    %Lu\n",
						(unsigned long long) pPnmiStat->StatRxTooLongCts);
					len += sk_proc_print(buffer,
						"   carrier extension           %Lu\n",
						(unsigned long long) pPnmiStat->StatRxCextCts);
					len += sk_proc_print(buffer,
						"   too short                   %Lu\n",
						(unsigned long long) pPnmiStat->StatRxShortsCts);
					len += sk_proc_print(buffer,
						"   symbol                      %Lu\n",
						(unsigned long long) pPnmiStat->StatRxSymbolCts);
					len += sk_proc_print(buffer,
						"   LLC MAC size                %Lu\n",
						(unsigned long long) pPnmiStat->StatRxIRLengthCts);
					len += sk_proc_print(buffer,
						"   carrier event               %Lu\n",
						(unsigned long long) pPnmiStat->StatRxCarrierCts);
					len += sk_proc_print(buffer,
						"   jabber                      %Lu\n",
						(unsigned long long) pPnmiStat->StatRxJabberCts);
#endif

					/*Transmit statistics */
					len += sk_proc_print(buffer,
					"\nTransmit statistics\n\n");

					len += sk_proc_print(buffer,
						"Transmitted bytes              %Lu\n",
						(unsigned long long) pPnmiStat->StatTxOctetsOkCts);
					len += sk_proc_print(buffer,
						"Transmitted packets            %Lu\n",
						(unsigned long long) pPnmiStat->StatTxOkCts);
					len += sk_proc_print(buffer,
						"Transmit errors                %Lu\n",
						(unsigned long long) pPnmiStat->StatTxSingleCollisionCts);
					len += sk_proc_print(buffer,
						"Transmit dropped               %Lu\n",
						(unsigned long long) pPnmiStruct->TxNoBufCts);
					len += sk_proc_print(buffer,
						"Transmit collisions            %Lu\n",
						(unsigned long long) pPnmiStat->StatTxSingleCollisionCts);
#ifdef ADVANCED_STATISTIC_OUTPUT
					len += sk_proc_print(buffer,
						"Transmit error types\n");
					len += sk_proc_print(buffer,
						"   excessive collision         %ld\n",
						pAC->stats.tx_aborted_errors);
					len += sk_proc_print(buffer,
						"   carrier                     %Lu\n",
						(unsigned long long) pPnmiStat->StatTxCarrierCts);
					len += sk_proc_print(buffer,
						"   fifo underrun               %Lu\n",
						(unsigned long long) pPnmiStat->StatTxFifoUnderrunCts);
					len += sk_proc_print(buffer,
						"   heartbeat                   %Lu\n",
						(unsigned long long) pPnmiStat->StatTxCarrierCts);
					len += sk_proc_print(buffer,
						"   window                      %ld\n",
						pAC->stats.tx_window_errors);
#endif
				} /* if (!DisableStatistic) */

			} /* if (strcmp(pACname, currDeviceName) == 0) */
		}
		SkgeProcDev = next;
	}
}

/*****************************************************************************
 *
 *      sk_proc_print - generic line print
 *
 * Description:
 *	This function fills the proc entry with statistic data about the
 *	ethernet device.
 *
 * Returns:
 *	the number of bytes written
 *
 */
static int sk_proc_print(
void *writePtr, /* the buffer pointer         */
char *format,   /* the format of the string   */
...)            /* variable list of arguments */
{
#define MAX_LEN_SINGLE_LINE 256
	char     str[MAX_LEN_SINGLE_LINE];
	va_list  a_start;
	int      lenght = 0;

	struct seq_file *seq = (struct seq_file *) writePtr;

	SK_MEMSET(str, 0, MAX_LEN_SINGLE_LINE);

	va_start(a_start, format);
	vsprintf(str, format, a_start);
	va_end(a_start);

	lenght = strlen(str);

	seq_printf(seq, str);
	return lenght;
}

/*****************************************************************************
 *
 *      sk_seq_show - show proc information of a particular adapter
 *
 * Description:
 *	This function fills the proc entry with statistic data about the
 *	ethernet device. It invokes the generic sk_gen_browse() to print
 *	out all items one per one.
 *
 * Returns:
 *	the number of bytes written
 *
 */
static int sk_seq_show(
struct seq_file *seq,  /* the sequence pointer */
void            *v)    /* additional pointer   */
{
	void *castedBuffer = (void *) seq;
	currDev = seq->private;
	sk_gen_browse(castedBuffer);
	return 0;
}

/*****************************************************************************
 *
 *      sk_proc_open - register the show function when proc is open'ed
 *
 * Description:
 *	This function is called whenever a sk98lin proc file is queried.
 *
 * Returns:
 *	the return value of single_open()
 *
 */
static int sk_proc_open(
struct inode *inode,  /* the inode of the file   */
struct file  *file)   /* the file pointer itself */
{
	return single_open(file, sk_seq_show, PDE_DATA(inode));
}

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
