/******************************************************************************
 *
 * Name:        skge.c
 * Project:     Gigabit Ethernet Adapter
 * Purpose:     The main driver source module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	    (C)Copyright 1998-2002 SysKonnect GmbH.
 *	    (C)Copyright 2002-2012 Marvell.
 *
 *	    Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	    Address all question to: support@marvell.com
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

/******************************************************************************
 *
 * Description:
 *
 *	All source files in this sk98lin directory except of the sk98lin
 *	Linux specific files
 *
 *		- skdim.c
 *		- skethtool.c
 *		- skge.c
 *		- skproc.c
 *		- sky2.c
 *		- Makefile
 *		- h/skdrv1st.h
 *		- h/skdrv2nd.h
 *		- h/sktypes.h
 *		- h/skversion.h
 *
 *	are part of SysKonnect's common modules for the SK-9xxx adapters.
 *
 *	Those common module files which are not Linux specific are used to
 *	build drivers on different OS' (e.g. Windows, MAC OS) so that those
 *	drivers are based on the same set of files
 *
 *	At a first glance, this seems to complicate things unnescessarily on
 *	Linux, but please do not try to 'clean up' them without VERY good
 *	reasons, because this will make it more difficult to keep the sk98lin
 *	driver for Linux in synchronisation with the other drivers running on
 *	other operating systems.
 *
 ******************************************************************************/

#include	"h/skversion.h"

#include	<linux/module.h>
#include	<linux/init.h>
#include	<linux/ethtool.h>
#include    <linux/netdevice.h>
#include    <linux/inetdevice.h>

#ifdef CONFIG_PROC_FS
#include 	<linux/proc_fs.h>
#endif

#include	"h/skdrv1st.h"
#include	"h/skdrv2nd.h"
#include	"h/skpcidevid.h"

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
#include	<linux/moduleparam.h>
#endif

/*******************************************************************************
 *
 * Defines
 *
 ******************************************************************************/

/* for debuging on x86 only */
/* #define BREAKPOINT() asm(" int $3"); */

#define NOT_EC_CHIP (pAC->FwCommon.ChipID == CHIP_ID_YUKON_EX) || \
                (pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR)


/* Set blink mode*/
#define OEM_CONFIG_VALUE (	SK_ACT_LED_BLINK | \
				SK_DUP_LED_NORMAL | \
				SK_LED_LINK100_ON)

#define CLEAR_AND_START_RX(Port) SK_OUT8(pAC->IoBase, RxQueueAddr[(Port)]+Q_CSR, CSR_START | CSR_IRQ_CL_F)
#define START_RX(Port) SK_OUT8(pAC->IoBase, RxQueueAddr[(Port)]+Q_CSR, CSR_START)
#define CLEAR_TX_IRQ(Port,Prio) SK_OUT8(pAC->IoBase, TxQueueAddr[(Port)][(Prio)]+Q_CSR, CSR_IRQ_CL_F)

/*******************************************************************************
 *
 * Local Function Prototypes
 *
 ******************************************************************************/

static int 	sk98lin_init_device(struct pci_dev *pdev, const struct pci_device_id *ent);
static void 	sk98lin_remove_device(struct pci_dev *pdev);
#ifdef CONFIG_PM
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
static int	sk98lin_suspend(struct pci_dev *pdev, pm_message_t state);
#else
static int	sk98lin_suspend(struct pci_dev *pdev, SK_U32 state);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
static void	sk98lin_shutdown(struct pci_dev *pdev);
#endif
static int	sk98lin_resume(struct pci_dev *pdev);
static void	SkEnableWOMagicPacket(SK_AC *pAC, SK_IOC IoC);
static void	SkCheckWOParam(SK_AC *pAC, int);

#ifdef SK_AVB
static u16 SkSelectQueue(struct net_device *dev, struct sk_buff *skb);
#endif

#ifndef MV_INCLUDE_SDK_SUPPORT
static void	SetDynamicClockGating(SK_AC *pAC, SK_IOC IoC);
#endif
#endif
#ifdef Y2_RECOVERY
static void	SkGeHandleKernelTimer(unsigned long ptr);
void		SkGeCheckTimer(DEV_NET *pNet);
static SK_BOOL  CheckRXCounters(DEV_NET *pNet);
static void	CheckRxPath(DEV_NET *pNet);
#endif
static void	FreeResources(struct SK_NET_DEVICE *dev);
static int	SkGeBoardInit(struct SK_NET_DEVICE *dev, SK_AC *pAC);
SK_BOOL	BoardAllocMem(SK_AC *pAC);
void		BoardFreeMem(SK_AC *pAC);
static void	BoardInitMem(SK_AC *pAC);
static void	SetupRing(SK_AC*, void*, uintptr_t, RXD**, RXD**, RXD**, int*, int*, SK_BOOL);
#ifdef SK_NEW_ISR_DEFINITION
static SkIsrRetVar	SkGeIsr(int irq, void *dev_id);
#else
static SkIsrRetVar	SkGeIsr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif

static void	GetConfiguration(SK_AC*);
static void	GetConTypeConfiguration(SK_AC*);
static void	GetWolTypeConfiguration(SK_AC*);
static void	GetSpeedConfiguration(SK_AC*, SK_I32, char *, char *);
static void	GetLinkModeCapabilities(SK_AC*, SK_I32, char *, char *, char *, char *);
static void	GetFlowControlConfiguration(SK_AC*, SK_I32, char *, char *);
static void	GetRoleConfiguration(SK_AC*, SK_I32, char *, char *);
static void	GetInterruptModConfiguration(SK_AC*);
static void	GetMiscConfiguration(SK_AC*);
static void	SetConfiguration(SK_AC*);
static void	ProductStr(SK_AC*);

int	SkGeOpen(struct SK_NET_DEVICE *dev);
int	SkGeClose(struct SK_NET_DEVICE *dev);
static int	SkGeXmit(struct sk_buff *skb, struct SK_NET_DEVICE *dev);
static int	SkGeSetMacAddr(struct SK_NET_DEVICE *dev, void *p);
static void	SkGeSetRxMode(struct SK_NET_DEVICE *dev);
static struct	net_device_stats *SkGeStats(struct SK_NET_DEVICE *dev);
static int	SkGeIoctl(struct SK_NET_DEVICE *dev, struct ifreq *rq, int cmd);
static int	XmitFrame(SK_AC*, TX_PORT*, struct sk_buff*);
static void	FreeTxDescriptors(SK_AC*pAC, TX_PORT*);
static void	FillRxRing(SK_AC*, RX_PORT*);
static SK_BOOL	FillRxDescriptor(SK_AC*, RX_PORT*);
#ifdef CONFIG_SK98LIN_NAPI
#ifdef SK_NEW_NAPI_HANDLING
extern int      SkY2Poll(struct napi_struct *napi, int work_limit);
static int      SkGePoll(struct napi_struct *napi, int work_limit);
#else
extern int	SkY2Poll(struct net_device *dev, int *budget);
static int	SkGePoll(struct net_device *dev, int *budget);
#endif
static void	ReceiveIrq(SK_AC*, RX_PORT*, SK_BOOL, int*, int);
#else
static void	ReceiveIrq(SK_AC*, RX_PORT*, SK_BOOL);
#endif
#ifdef SK_POLL_CONTROLLER
static void	SkGeNetPoll(struct SK_NET_DEVICE *dev);
#endif
static void	ClearRxRing(SK_AC*, RX_PORT*);
static void	ClearTxRing(SK_AC*, TX_PORT*);
static int	SkGeChangeMtu(struct SK_NET_DEVICE *dev, int new_mtu);
static void	PortReInitBmu(SK_AC*, int);
static int	SkGeIocMib(DEV_NET*, unsigned int, int);
static int	SkGeInitPCI(SK_AC *pAC);
static SK_U32   ParseDeviceNbrFromSlotName(const char *SlotName);
static int      SkDrvInitAdapter(SK_AC *pAC, int devNbr);
static int      SkDrvDeInitAdapter(SK_AC *pAC, int devNbr);
static int	XmitFrameSG(SK_AC*, TX_PORT*, struct sk_buff*);
extern void	SkLocalEventQueue(SK_AC *pAC,
					SK_U32 Class,
					SK_U32 Event,
					SK_U32 Param1,
					SK_U32 Param2,
					SK_BOOL Flag);
extern void	SkLocalEventQueue64(SK_AC *pAC,
					SK_U32 Class,
					SK_U32 Event,
					SK_U64 Param,
					SK_BOOL Flag);
static void SkGeReleaseIrq(struct SK_NET_DEVICE *dev);
static int SK_DEVINIT SkGeRequestIrq(struct SK_NET_DEVICE *dev);
#ifdef SK_NEW_ISR_DEFINITION
static int SK_DEVINIT SkGeTestIsr(int irq, void *dev_id);
#else
static int SK_DEVINIT SkGeTestIsr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif
int SK_DEVINIT SkGeTestInt(struct SK_NET_DEVICE *dev, SK_AC *pAC, SK_U32 Int);

#ifdef USE_SK_RSS_SUPPORT
static void SkSetRssSupport(SK_AC *pAC, int Port);
#endif

/*******************************************************************************
 *
 * Extern Function Prototypes
 *
 ******************************************************************************/
#ifdef MV_INCLUDE_SDK_SUPPORT
extern int SetFwIpAddr(SK_AC *pAC, SK_U32 , SK_U32 , SK_U32 );
extern SK_U32 SendFwCommand (SK_AC *pAC, SK_U32 DatagramType, char *pFwDataBuff, SK_U32 FrameLength);
extern SK_U32 HandleDrvCommand (SK_AC *pAC, SK_GE_CMDIOCTL *pFwCommand, SK_U32 FrameLength);
#endif
extern SK_BOOL SkY2AllocateResources(SK_AC *pAC);
extern void SkY2FreeResources(SK_AC *pAC);
extern void SkY2AllocateRxBuffers(SK_AC *pAC,SK_IOC IoC,int Port);
extern void SkY2FreeRxBuffers(SK_AC *pAC,SK_IOC IoC,int Port);
extern void SkY2FreeTxBuffers(SK_AC *pAC,SK_IOC IoC,int Port);

#ifdef SK_NEW_ISR_DEFINITION
extern SkIsrRetVar SkY2Isr(int irq,void *dev_id);
#else
extern SkIsrRetVar SkY2Isr(int irq,void *dev_id,struct pt_regs *ptregs);
#endif
extern int SkY2Xmit(struct sk_buff *skb,struct SK_NET_DEVICE *dev);
extern void SkY2PortStop(SK_AC *pAC,SK_IOC IoC,int Port,int Dir,int RstMode);
extern void SkY2PortStart(SK_AC *pAC,SK_IOC IoC,int Port);
extern void SkY2RestartStatusUnit(SK_AC *pAC);
extern void FillReceiveTableYukon2(SK_AC *pAC,SK_IOC IoC,int Port);

extern void SkDimEnableModerationIfNeeded(SK_AC *pAC);
extern void SkDimStartModerationTimer(SK_AC *pAC);
extern void SkDimModerate(SK_AC *pAC);

/* Ethtool functions */
extern int SkGeGetSettings(struct net_device *dev, struct ethtool_cmd *ecmd);
extern void SkGeGetDrvInfo(struct net_device *dev, struct ethtool_drvinfo *ecmd);
extern void SkGeGetWolSettings(struct net_device *dev, struct ethtool_wolinfo *ecmd);
extern void SkGeGetPauseParam(struct net_device *dev, struct ethtool_pauseparam *ecmd);
extern int SkGeGetCoalesce(struct net_device *dev, struct ethtool_coalesce *ecmd);
extern SK_U32 SkGeGetRxCsum(struct net_device *dev);
extern void SkGeGetStrings(struct net_device *dev, u32 stringset, u8 *strings);
extern int SkGeGetStatsLen(struct net_device *dev);
extern int SkGeGetSsetCount(struct net_device *dev, int sset);
extern void SkGeGetEthStats(struct net_device *dev, struct ethtool_stats *stats, u64 *data);
extern int SkGeSetSettings(struct net_device *dev, struct ethtool_cmd *ecmd);
extern int SkGeSetWolSettings(struct net_device *dev, struct ethtool_wolinfo *ewol);
extern int SkGeSetPauseParam(struct net_device *dev, struct ethtool_pauseparam *ecmd);
extern int SkGeSetCoalesce(struct net_device *dev, struct ethtool_coalesce *ecmd);
extern int SkGeSetSG(struct net_device *dev, u32 data);
extern int SkGeSetTxCsum(struct net_device *dev, u32 data);
extern int SkGeSetRxCsum(struct net_device *dev, u32 data);
extern int SkGePhysId(struct net_device *dev, u32 data);

void SkGeGetRingParam(struct net_device *dev, struct ethtool_ringparam *eth_ring);
int SkGeSetRingParam(struct net_device *dev, struct ethtool_ringparam *eth_ring);
extern int SkGeGetEepromLen(struct net_device *dev);
extern int SkGeGetEeprom(struct net_device *dev, struct ethtool_eeprom *eeprom, u8 *data);
extern int SkGeSetEeprom(struct net_device *dev, struct ethtool_eeprom *eeprom, u8 *data);
extern int SkGeGetRegsLen(struct net_device *dev);
extern void SkGeGetRegs(struct net_device *dev, struct ethtool_regs *regs, void *data);

#ifdef SK98LIN_DIAG
extern int SkGeDiagTestCnt(struct net_device *netdev);
extern void SkGeDiagTest(struct net_device *netdev, struct ethtool_test *ethtest, u64 *data);
#endif

#ifdef NETIF_F_TSO
extern int SkGeSetTSO(struct net_device *netdev, u32 data);
#endif

#ifdef CONFIG_PROC_FS
static const char 	SK_Root_Dir_entry[] = "sk98lin";
static struct	proc_dir_entry *pSkRootDir;
extern struct	file_operations sk_proc_fops;
#endif

#ifdef DEBUG
void	DumpMsg(struct sk_buff*, char*);
static void	DumpData(char*, int);
static void	DumpLong(char*, int);
#endif

/* global variables *********************************************************/
static const char *BootString = BOOT_STRING;
struct SK_NET_DEVICE *SkGeRootDev = NULL;
static SK_BOOL DoPrintInterfaceChange = SK_TRUE;

/* local variables **********************************************************/
static uintptr_t TxQueueAddr[SK_MAX_MACS][2] = {{0x680, 0x600},{0x780, 0x700}};
static uintptr_t RxQueueAddr[SK_MAX_MACS] = {0x400, 0x480};
static int sk98lin_max_boards_found = 0;

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry	*pSkRootDir;
#endif

static struct ethtool_ops sk98lin_ethtool_ops = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
	.phys_id		= SkGePhysId,
	.get_sg			= ethtool_op_get_sg,
	.get_tx_csum		= ethtool_op_get_tx_csum,
	.get_rx_csum		= SkGeGetRxCsum,
	.set_tx_csum		= SkGeSetTxCsum,
	.set_rx_csum		= SkGeSetRxCsum,
	.set_sg			= SkGeSetSG,
#ifdef NETIF_F_TSO
	.get_tso		= ethtool_op_get_tso,
	.set_tso		= SkGeSetTSO,
#endif
#else
	.set_phys_id		= SkGePhysId,
#endif
	.get_link		= ethtool_op_get_link,
/*	.get_perm_addr		= ethtool_op_get_perm_addr, */
	.get_settings		= SkGeGetSettings,
	.get_drvinfo		= SkGeGetDrvInfo,
	.get_wol		= SkGeGetWolSettings,
	.get_pauseparam		= SkGeGetPauseParam,
	.get_coalesce		= SkGeGetCoalesce,
	.get_strings		= SkGeGetStrings,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	.get_stats_count	= SkGeGetStatsLen,
#ifdef SK98LIN_DIAG
	.self_test_count	= SkGeDiagTestCnt,
#endif
#else
	.get_sset_count		= SkGeGetSsetCount,
#endif
	.get_ethtool_stats	= SkGeGetEthStats,
	.set_settings		= SkGeSetSettings,
	.set_wol		= SkGeSetWolSettings,
	.set_pauseparam		= SkGeSetPauseParam,
	.set_coalesce		= SkGeSetCoalesce,

	.get_eeprom_len		= SkGeGetEepromLen,
	.get_eeprom		= SkGeGetEeprom,
	.set_eeprom		= SkGeSetEeprom,
	.get_regs_len		= SkGeGetRegsLen,
	.get_regs		= SkGeGetRegs,
	.get_ringparam		= SkGeGetRingParam,
	.set_ringparam		= SkGeSetRingParam,

#ifdef SK98LIN_DIAG
	.self_test		= SkGeDiagTest,
#endif

/*	.get_msglevel	= */
/*	.nway_reset	= */
/*	.set_msglevel	= */
};

#ifdef	SK_AVB
struct file_operations mv_sw_fops = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    ioctl:  mv_sw_ioctl,
#else
	unlocked_ioctl: mv_sw_ioctl,
#endif
    open:   mv_sw_open,
    release:mv_sw_release,
};
#endif

MODULE_DEVICE_TABLE(pci, sk98lin_pci_tbl);

static struct pci_driver sk98lin_driver = {
	.name		= DRIVER_FILE_NAME,
	.id_table	= sk98lin_pci_tbl,
	.probe		= sk98lin_init_device,
	.remove		= sk98lin_remove_device,
#ifdef CONFIG_PM
	.suspend	= sk98lin_suspend,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
	.shutdown	= sk98lin_shutdown,
#endif
	.resume		= sk98lin_resume
#endif
};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
/* conversion of netdev_ops */
static const struct net_device_ops sky2_netdev_ops = {
	.ndo_open               = SkGeOpen,
	.ndo_stop               = SkGeClose,
	.ndo_start_xmit         = SkY2Xmit,
#ifdef SK_AVB
	.ndo_select_queue       = SkSelectQueue,
#endif
	.ndo_get_stats          = SkGeStats,
	.ndo_do_ioctl           = SkGeIoctl,
	.ndo_set_mac_address    = SkGeSetMacAddr,
	.ndo_set_rx_mode        = SkGeSetRxMode,
	.ndo_change_mtu         = SkGeChangeMtu,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller    = SkGeNetPoll,
#endif
};

static const struct net_device_ops skge_netdev_ops = {
	.ndo_open               = SkGeOpen,
	.ndo_stop               = SkGeClose,
	.ndo_start_xmit         = SkGeXmit,
	.ndo_get_stats          = SkGeStats,
	.ndo_do_ioctl           = SkGeIoctl,
	.ndo_set_mac_address    = SkGeSetMacAddr,
	.ndo_set_rx_mode        = SkGeSetRxMode,
	.ndo_change_mtu         = SkGeChangeMtu,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller    = SkGeNetPoll,
#endif
};
#endif

/*****************************************************************************
 *
 * 	sk98lin_init_device - initialize the adapter
 *
 * Description:
 *	This function initializes the adapter. Resources for
 *	the adapter are allocated and the adapter is brought into Init 1
 *	state.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static int sk98lin_init_device(struct pci_dev *pdev,
				  const struct pci_device_id *ent)

{
	static SK_BOOL 		sk98lin_boot_string = SK_FALSE;
#ifdef CONFIG_PROC_FS
	static SK_BOOL 		sk98lin_proc_entry = SK_FALSE;
#endif
	static int		sk98lin_boards_found = 0;
	SK_AC			*pAC;
	DEV_NET			*pNet = NULL;
	struct SK_NET_DEVICE	*dev = NULL;
	int			retval;
	int			pci_using_dac;

	retval = pci_enable_device(pdev);
	if (retval) {
		printk(KERN_ERR "Cannot enable PCI device, "
			"aborting.\n");
		return retval;
	}

	dev = NULL;
	pNet = NULL;

	/* INSERT * We have to find the power-management capabilities */
	/* Find power-management capability. */

	pci_using_dac = 0;		/* Set 32 bit DMA per default */

	/* Configure DMA attributes. */
	retval = pci_set_dma_mask(pdev, (u64) 0xffffffffffffffffULL);
	if (!retval) {
		pci_using_dac = 1;
	} else {
		retval = pci_set_dma_mask(pdev, (u64) 0xffffffff);
		if (retval) {
			printk(KERN_ERR "No usable DMA configuration, "
			       "aborting.\n");
			return retval;
		}
	}

	if ((dev = ALLOC_ETHDEV(sizeof(DEV_NET))) == NULL) {
		printk(KERN_ERR "Unable to allocate etherdev "
			"structure!\n");
		return -ENODEV;
	}

	pNet = PRIV;

	pNet->pAC = kmalloc(sizeof(SK_AC), GFP_KERNEL);
	if (pNet->pAC == NULL){
		free_netdev(dev);
		printk(KERN_ERR "Unable to allocate adapter "
			"structure!\n");
		return -ENODEV;
	}

	/* Print message */
	if (!sk98lin_boot_string) {
		/* set display flag to TRUE so that */
		/* we only display this string ONCE */
		sk98lin_boot_string = SK_TRUE;
		printk("%s\n", BootString);
	}

	memset(pNet->pAC, 0, sizeof(SK_AC));
	pAC = pNet->pAC;
	pAC->PciDev = pdev;
	pAC->PciDevId = pdev->device;
	pAC->dev[0] = dev;
	pAC->dev[1] = dev;
	sprintf(pAC->Name, "SysKonnect SK-98xx");
	pAC->CheckQueue = SK_FALSE;
	pAC->InterfaceUp[0] = 0;
	pAC->InterfaceUp[1] = 0;
	dev->irq = pdev->irq;
	retval = SkGeInitPCI(pAC);
	if (retval) {
		printk("SKGE: PCI setup failed: %i\n", retval);
		free_netdev(dev);
		return -ENODEV;
	}

	pAC->NumTxQueues = TX_LE_Q_CNT;
#ifndef	SK_AVB
#ifdef	USE_SYNC_TX_QUEUE
	if (!HW_SYNC_TX_SUPPORTED(pAC)) {
		pAC->NumTxQueues = 1;
	}
#endif
#endif


	pAC->AvbModeEnabled = SK_FALSE;
	pAC->DefaultTxQ = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	SET_MODULE_OWNER(dev);
#endif
	dev->ethtool_ops = &sk98lin_ethtool_ops;

	pAC->Index = sk98lin_boards_found;

	if (SkGeBoardInit(dev, pAC)) {
		free_netdev(dev);
		return -ENODEV;
	} else {
		ProductStr(pAC);
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
	if (CHIP_ID_YUKON_2(pAC)) {
		dev->netdev_ops		=  &sky2_netdev_ops;
	} else {
		dev->netdev_ops		=  &skge_netdev_ops;
	}
#else
	dev->open		=  &SkGeOpen;
	dev->stop		=  &SkGeClose;
	dev->get_stats		=  &SkGeStats;
	dev->set_multicast_list	=  &SkGeSetRxMode;
	dev->set_mac_address	=  &SkGeSetMacAddr;
	dev->do_ioctl		=  &SkGeIoctl;
	dev->change_mtu		=  &SkGeChangeMtu;
#ifdef SK_POLL_CONTROLLER
	dev->poll_controller	=  SkGeNetPoll;
#endif
#endif
	dev->flags		&= ~IFF_RUNNING;
	SET_NETDEV_DEV(dev, &pdev->dev);

	if (pci_using_dac)
		dev->features |= NETIF_F_HIGHDMA;

	/* shifter to later moment in time... */
	if (CHIP_ID_YUKON_2(pAC)) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		dev->hard_start_xmit =	&SkY2Xmit;
#endif

#ifdef CONFIG_SK98LIN_NAPI
#ifdef SK_NEW_NAPI_HANDLING
		netif_napi_add(dev, &pNet->napi, SkY2Poll, 64);
#else
		dev->poll =  &SkY2Poll;
		dev->weight = 64;
#endif
#endif
	} else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		dev->hard_start_xmit =	&SkGeXmit;
#endif

#ifdef CONFIG_SK98LIN_NAPI
#ifdef SK_NEW_NAPI_HANDLING
		netif_napi_add(dev, &pNet->napi, SkGePoll, 64);
#else
		dev->poll =  &SkGePoll;
		dev->weight = 64;
#endif
#endif
	}

#ifdef USE_SK_VLAN_SUPPORT
	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
		dev->features |= NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX;
	}
#endif

#ifdef NETIF_F_TSO
#ifdef USE_SK_TSO_FEATURE
	if (CHIP_ID_YUKON_2(pAC)
#ifdef	SK_AVB
		&& (pAC->GIni.GIChipId != CHIP_ID_YUKON_OPT)
		&& (pAC->GIni.GIChipId != CHIP_ID_YUKON_PRM)
#endif
	) {
		dev->features |= NETIF_F_TSO;
#ifdef USE_SK_RSS_SUPPORT
	if (pAC->LinkInfo[0].RSS)
		dev->features |= NETIF_F_RXHASH;
#endif
	}
#endif
#endif

#ifdef CONFIG_SK98LIN_ZEROCOPY
	dev->features |= NETIF_F_SG;
#endif
#ifdef USE_SK_TX_CHECKSUM
	dev->features |= NETIF_F_IP_CSUM;
#endif
#ifdef USE_SK_RX_CHECKSUM
	pAC->RxPort[0].UseRxCsum = SK_TRUE;
	if (pAC->GIni.GIMacsFound == 2 ) {
		pAC->RxPort[1].UseRxCsum = SK_TRUE;
	}
#endif

	/* Save the hardware revision */
	pAC->HWRevision = (((pAC->GIni.GIPciHwRev >> 4) & 0x0F)*10) +
		(pAC->GIni.GIPciHwRev & 0x0F);

	/* Set driver globals */
	pAC->Pnmi.pDriverFileName    = DRIVER_FILE_NAME;
	pAC->Pnmi.pDriverReleaseDate = DRIVER_REL_DATE;

	SK_MEMSET(&(pAC->PnmiBackup), 0, sizeof(SK_PNMI_STRUCT_DATA));
	SK_MEMCPY(&(pAC->PnmiBackup), &(pAC->PnmiStruct),
			sizeof(SK_PNMI_STRUCT_DATA));

	/* Register net device */
	retval = register_netdev(dev);
	if (retval) {
		printk(KERN_ERR "SKGE: Could not register device.\n");
		FreeResources(dev);
		free_netdev(dev);
		return retval;
	}

#ifdef	SK_AVB
	retval = register_chrdev (SWDEV_MAGIC, "sw", &mv_sw_fops);
	if (retval < 0) {
		printk(KERN_ERR "ioctl:%d: can't get major %d\n", __LINE__, SWDEV_MAGIC);
		return retval;
	}
#endif

	/* Save initial device name */
	strcpy(pNet->InitialDevName, dev->name);

	/* Set network to off */
	NETIF_STOP_ALLQ(dev);
	netif_carrier_off(dev);

	/* Print adapter specific string from vpd and config settings */
	printk("%s: %s\n", pNet->InitialDevName, pAC->DeviceStr);

	SkGeYellowLED(pAC, pAC->IoBase, 1);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
	memcpy((caddr_t) &dev->dev_addr,
#else
	memcpy((caddr_t) dev->dev_addr,
#endif
		(caddr_t) &pAC->Addr.Net[0].CurrentMacAddress, 6);

	/* First adapter... Create proc and print message */
#ifdef CONFIG_PROC_FS
	if (!sk98lin_proc_entry) {
		sk98lin_proc_entry = SK_TRUE;
		SK_MEMCPY(&SK_Root_Dir_entry, BootString,
			sizeof(SK_Root_Dir_entry) - 1);

		/*Create proc (directory)*/
		if(!pSkRootDir) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
			pSkRootDir = proc_mkdir(SK_Root_Dir_entry, proc_net);
#else
			pSkRootDir = proc_mkdir(SK_Root_Dir_entry, init_net.proc_net);
#endif
			if (!pSkRootDir) {
				printk(KERN_WARNING "%s: Unable to create /proc/net/%s",
					dev->name, SK_Root_Dir_entry);
			} else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
				pSkRootDir->owner = THIS_MODULE;
#endif
			}
		}
	}

#endif

	pNet->PortNr = 0;
	pNet->NetNr  = 0;

#ifdef CONFIG_SK98LIN_NAPI
#ifdef SK_NEW_NAPI_HANDLING
	napi_enable(&pNet->napi);
#endif
#endif

	pci_set_drvdata(pdev, dev);

	/* More then one port found */
	if ((pAC->GIni.GIMacsFound == 2 ) && (pAC->RlmtNets == 2)) {
		dev = ALLOC_ETHDEV(sizeof(DEV_NET));
		if (!dev) {
			printk(KERN_ERR "Unable to allocate etherdev "
				"structure!\n");
			return -ENODEV;
		}

		pAC->dev[1]   = dev;
		pNet = PRIV;
		pNet->PortNr  = 1;
		pNet->NetNr   = 1;
		pNet->pAC     = pAC;

		if (CHIP_ID_YUKON_2(pAC)) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			dev->hard_start_xmit = &SkY2Xmit;
#endif

#ifdef CONFIG_SK98LIN_NAPI
#ifndef SK_NEW_NAPI_HANDLING
			dev->poll =  &SkY2Poll;
			dev->weight = 64;
#endif
#endif
		} else {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			dev->hard_start_xmit = &SkGeXmit;
#endif

#ifdef CONFIG_SK98LIN_NAPI
#ifndef SK_NEW_NAPI_HANDLING
			dev->poll =  &SkGePoll;
			dev->weight = 64;
#endif
#endif
		}

		dev->ethtool_ops = &sk98lin_ethtool_ops;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
		if (CHIP_ID_YUKON_2(pAC)) {
			dev->netdev_ops		=  &sky2_netdev_ops;
		} else {
			dev->netdev_ops		=  &skge_netdev_ops;
		}
#else
		dev->open		=  &SkGeOpen;
		dev->stop		=  &SkGeClose;
		dev->get_stats		=  &SkGeStats;
		dev->set_multicast_list	=  &SkGeSetRxMode;
		dev->set_mac_address	=  &SkGeSetMacAddr;
		dev->do_ioctl		=  &SkGeIoctl;
		dev->change_mtu		=  &SkGeChangeMtu;
#ifdef SK_POLL_CONTROLLER
		dev->poll_controller	=  SkGeNetPoll;
#endif
#endif
		dev->flags		&= ~IFF_RUNNING;
		SET_NETDEV_DEV(dev, &pdev->dev);

#ifdef NETIF_F_TSO
#ifdef USE_SK_TSO_FEATURE
		if (CHIP_ID_YUKON_2(pAC)
#ifdef	SK_AVB
			&& (pAC->GIni.GIChipId != CHIP_ID_YUKON_OPT)
			&& (pAC->GIni.GIChipId != CHIP_ID_YUKON_PRM)
#endif
		) {
			dev->features |= NETIF_F_TSO;
#ifdef USE_SK_RSS_SUPPORT
	if (pAC->LinkInfo[0].RSS)
			dev->features |= NETIF_F_RXHASH;
#endif
		}
#endif
#endif
#ifdef CONFIG_SK98LIN_ZEROCOPY
		dev->features |= NETIF_F_SG;
#endif
#ifdef USE_SK_TX_CHECKSUM
		dev->features |= NETIF_F_IP_CSUM;
#endif

		/* Set the features mask */
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0)
		dev->hw_features = dev->features & ~NETIF_F_HIGHDMA;
#endif

		if (register_netdev(dev)) {
			printk(KERN_ERR "SKGE: Could not register device.\n");
			free_netdev(dev);
			pAC->dev[1] = pAC->dev[0];
		} else {

			/* Save initial device name */
			strcpy(pNet->InitialDevName, dev->name);

			/* Set network to off */
			NETIF_STOP_ALLQ(dev);
			netif_carrier_off(dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
			memcpy((caddr_t) &dev->dev_addr,
#else
			memcpy((caddr_t) dev->dev_addr,
#endif
				(caddr_t) &pAC->Addr.Net[1].CurrentMacAddress, 6);

			printk("%s: %s\n", pNet->InitialDevName, pAC->DeviceStr);
		}
	}

	pAC->Index = sk98lin_boards_found;
	sk98lin_boards_found++;
	sk98lin_max_boards_found = sk98lin_boards_found;

#ifdef	SK_AVB
	mv_swdev.flags = 0;
	atu_node_table_init();
	avb_initsw(pAC, 1);
	avb_init_swdev(pAC);
#endif

	return 0;
}



/*****************************************************************************
 *
 * 	SkGeInitPCI - Init the PCI resources
 *
 * Description:
 *	This function initialize the PCI resources and IO
 *
 * Returns: N/A
 *
 */
static int SkGeInitPCI(SK_AC *pAC)
{
	struct SK_NET_DEVICE *dev = pAC->dev[0];
	struct pci_dev *pdev = pAC->PciDev;
	int retval;

	if (pci_enable_device(pdev) != 0) {
		return 1;
	}

	dev->mem_start = pci_resource_start (pdev, 0);
	pci_set_master(pdev);

	if (pci_request_regions(pdev, DRIVER_FILE_NAME) != 0) {
		retval = 2;
		goto out_disable;
	}

#ifdef SK_BIG_ENDIAN
	/*
	 * On big endian machines, we use the adapter's aibility of
	 * reading the descriptors as big endian.
	 */
	{
		SK_U32		our2;
		SkPciReadCfgDWord(pAC, PCI_OUR_REG_2, &our2);
		our2 |= PCI_REV_DESC;
		SkPciWriteCfgDWord(pAC, PCI_OUR_REG_2, our2);
	}
#endif

	/*
	 * Remap the regs into kernel space.
	 */
	pAC->IoBase = ioremap_nocache(dev->mem_start, 0x4000);

	if (!pAC->IoBase){
		retval = 3;
		goto out_release;
	}

	return 0;

out_release:
	pci_release_regions(pdev);
out_disable:
	pci_disable_device(pdev);
	return retval;
}

#ifdef CONFIG_PROC_FS
/*****************************************************************************
 *
 * 	SkGeHandleProcfsTimer - Handle the procfs timer requests
 *
 * Description:
 *      Checks, if the device's name changed. If this is the case
 *      it deletes the old profs entry and creates a new one with
 *      the new name.
 *
 * Returns:	N/A
 *
 */
static void SkGeHandleProcfsTimer(unsigned long ptr)
{
	DEV_NET         *pNet = (DEV_NET*) ptr;
	struct proc_dir_entry *pProcFile;

	/*
	 * If the current name and the last saved name of the device differ
	 * we need to update our procfs entry.
	 */
	if ( (pSkRootDir) &&
	     (strcmp(pNet->CurrentName, pNet->pAC->dev[pNet->NetNr]->name) != 0) ) {

	  if (pNet->pAC->InterfaceUp[pNet->NetNr] == 1)
	    remove_proc_entry(pNet->CurrentName, pSkRootDir);

	  /*
	   * InterfaceUp only holds 1 if both the network interface is up and
	   * the corresponding procfs entry is done. Otherwise it is set to 0.
	   */
	  pNet->pAC->InterfaceUp[pNet->NetNr] = 0;

	  pProcFile = create_proc_entry(pNet->pAC->dev[pNet->NetNr]->name, S_IRUGO, pSkRootDir);
	  pProcFile->proc_fops = &sk_proc_fops;
	  pProcFile->data      = pNet->pAC->dev[pNet->NetNr];

	  /*
	   * Remember, interface dev nr pNet->NetNr is up and procfs entry is created.
	   */
	  pNet->pAC->InterfaceUp[pNet->NetNr] = 1;

	  strcpy(pNet->CurrentName, pNet->pAC->dev[pNet->NetNr]->name);
	}

	/*
	 * Restart Procfs Timer
	 */
	pNet->ProcfsTimer.expires	= jiffies + HZ*5; /* 5 secs */
	add_timer(&pNet->ProcfsTimer);
}
#endif


#ifdef Y2_RECOVERY
/*****************************************************************************
 *
 * 	SkGeHandleKernelTimer - Handle the kernel timer requests
 *
 * Description:
 *	If the requested time interval for the timer has elapsed,
 *	this function checks the link state.
 *
 * Returns:	N/A
 *
 */
static void SkGeHandleKernelTimer(
unsigned long ptr)  /* holds the pointer to adapter control context */
{
	DEV_NET         *pNet = (DEV_NET*) ptr;
	SkGeCheckTimer(pNet);
}

/*****************************************************************************
 *
 * 	sk98lin_check_timer - Resume the the card
 *
 * Description:
 *	This function checks the kernel timer
 *
 * Returns: N/A
 *
 */
void SkGeCheckTimer(
DEV_NET *pNet)  /* holds the pointer to adapter control context */
{
	SK_AC           *pAC = pNet->pAC;
	SK_BOOL		StartTimer = SK_TRUE;
	SK_U32          StatSpeed, StatDuplex, NewTimerInterval;


	StatSpeed = pAC->GIni.GP[pNet->NetNr].PLinkSpeedUsed;
	if (StatSpeed == SK_LSPEED_STAT_10MBPS) {
		StatDuplex = pAC->GIni.GP[pNet->NetNr].PLinkModeStatus;
		if ((StatDuplex == SK_LMODE_STAT_AUTOHALF) ||
			(StatDuplex == SK_LMODE_STAT_HALF)) {
				NewTimerInterval = (HZ*2);
		} else {
			NewTimerInterval = (HZ);
		}
	} else if (StatSpeed == SK_LSPEED_STAT_100MBPS) {
		NewTimerInterval = (HZ/2);
	} else if (StatSpeed == SK_LSPEED_STAT_1000MBPS) {
		NewTimerInterval = (HZ/4);
	} else {
		NewTimerInterval = (HZ*2);
	}

	if (pNet->InRecover) {
		pNet->KernelTimer.expires = jiffies + NewTimerInterval;
		add_timer(&pNet->KernelTimer);

		return;
	}

	if (pNet->TimerExpired) {
		return;
	}

	pNet->TimerExpired = SK_TRUE;

	if (CHIP_ID_YUKON_2(pAC)) {
		int	queue;

#ifdef Y2_RX_CHECK
//		if (HW_FEATURE(pAC, HWF_WA_DEV_4167)) {
		/* Checks the RX path */
			CheckRxPath(pNet);
//		}
#endif

		/* Check the transmitter */
		for (queue=0;queue<pAC->NumTxQueues;queue++) {
			if ((!NETIF_Q_STOPPED(pAC->dev[pNet->PortNr],queue))
				&& (!(IS_Q_EMPTY(&pAC->TxPort[pNet->PortNr].TxQ_working[queue])))) {
				if (pAC->TxPort[pNet->PortNr].LastDoneIdx[queue] != pAC->TxPort[pNet->PortNr].TxLET[queue].Done) {
					pAC->TxPort[pNet->PortNr].LastDoneIdx[queue] = pAC->TxPort[pNet->PortNr].TxLET[queue].Done;
					pNet->TransmitTimeoutTimer = 0;
				} else {
					pNet->TransmitTimeoutTimer++;
					if (pNet->TransmitTimeoutTimer >= 10) {
						pNet->TransmitTimeoutTimer = 0;
#ifdef CHECK_TRANSMIT_TIMEOUT
						StartTimer =  SK_FALSE;
						SkLocalEventQueue(pAC, SKGE_DRV,
							SK_DRV_RECOVER,pNet->PortNr,-1,SK_FALSE);
#endif
					}
				}
			}
		}

#ifdef CHECK_TRANSMIT_TIMEOUT
		if (!timer_pending(&pNet->KernelTimer)) {
			pNet->KernelTimer.expires = jiffies + NewTimerInterval;
			add_timer(&pNet->KernelTimer);
			pNet->TimerExpired = SK_FALSE;
		}
#endif
	} else {
		pNet->KernelTimer.expires = jiffies + NewTimerInterval;
		add_timer(&pNet->KernelTimer);
		pNet->TimerExpired = SK_FALSE;
	}
}


/*****************************************************************************
*
* CheckRXCounters - Checks the the statistics for RX path hang
*
* Description:
*	This function is called periodical by a timer.
*
* Notes:
*
* Function Parameters:
*
* Returns:
*	Traffic status
*
*/
static SK_BOOL CheckRXCounters(
DEV_NET *pNet)  /* holds the pointer to adapter control context */
{
	SK_AC           	*pAC = pNet->pAC;
	SK_BOOL bStatus 	= SK_FALSE;

	/* Variable used to store the MAC RX FIFO RP, RPLev*/
	SK_U32			MACFifoRP = 0;
	SK_U32			MACFifoRLev = 0;

	/* Variable used to store the PCI RX FIFO RP, RPLev*/
	SK_U32			RXFifoRP = 0;
	SK_U8			RXFifoRLev = 0;
	SK_U32			CurrentJiffies = 0;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> CheckRXCounters()\n"));

	CurrentJiffies = pAC->dev[pNet->PortNr]->last_rx;

	/*Check if statistic counters hangs*/
	if (CurrentJiffies == pNet->LastJiffies) {
		/* Now read the values of read pointer/level from MAC RX FIFO again */
		SK_IN32(pAC->IoBase, MR_ADDR(pNet->PortNr, RX_GMF_RP), &MACFifoRP);
		SK_IN32(pAC->IoBase, MR_ADDR(pNet->PortNr, RX_GMF_RLEV), &MACFifoRLev);

		/* Now read the values of read pointer/level from RX FIFO again */
		SK_IN8(pAC->IoBase, Q_ADDR(pAC->GIni.GP[pNet->PortNr].PRxQOff, Q_RX_RP), &RXFifoRP);
		SK_IN8(pAC->IoBase, Q_ADDR(pAC->GIni.GP[pNet->PortNr].PRxQOff, Q_RX_RL), &RXFifoRLev);

		/* Check if the MAC RX hang */
		if ((MACFifoRP == pNet->PreviousMACFifoRP) &&
			(MACFifoRLev != 0) &&
			(MACFifoRLev >= pNet->PreviousMACFifoRLev)){
			bStatus = SK_TRUE;
		}

		/* Check if the PCI RX hang */
		if ((RXFifoRP == pNet->PreviousRXFifoRP) &&
			(RXFifoRLev != 0) &&
			(RXFifoRLev >= pNet->PreviousRXFifoRLev)){
			/*Set the flag to indicate that the RX FIFO hangs*/
			bStatus = SK_TRUE;
		}
	}

	/* Store now the values of counters for next check */
	pNet->LastJiffies = pAC->dev[pNet->PortNr]->last_rx;

	/* Store the values of  read pointer/level from MAC RX FIFO for next test */
	pNet->PreviousMACFifoRP = MACFifoRP;
	pNet->PreviousMACFifoRLev = MACFifoRLev;

	/* Store the values of  read pointer/level from RX FIFO for next test */
	pNet->PreviousRXFifoRP = RXFifoRP;
	pNet->PreviousRXFifoRLev = RXFifoRLev;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== CheckRXCounters()\n"));

	return bStatus;
}

/*****************************************************************************
*
* CheckRxPath - Checks if the RX path
*
* Description:
*	This function is called periodical by a timer.
*
* Notes:
*
* Function Parameters:
*
* Returns:
*	None.
*
*/
static void  CheckRxPath(
DEV_NET *pNet)  /* holds the pointer to adapter control context */
{
	unsigned long		Flags;    /* for the spin locks    */
	/* Initialize the pAC structure.*/
	SK_AC           	*pAC = pNet->pAC;

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("==> CheckRxPath()\n"));

	/*If the statistics are not changed then could be an RX problem */
	if (CheckRXCounters(pNet)){
		/*
		 * First we try the simple solution by resetting the Level Timer
		 */

		/* Stop Level Timer of Status BMU */
		SK_OUT8(pAC->IoBase, STAT_LEV_TIMER_CTRL, TIM_STOP);

		/* Start Level Timer of Status BMU */
		SK_OUT8(pAC->IoBase, STAT_LEV_TIMER_CTRL, TIM_START);

		if (!CheckRXCounters(pNet)) {
			return;
		}

		spin_lock_irqsave(&pAC->SlowPathLock, Flags);
		SkLocalEventQueue(pAC, SKGE_DRV,
			SK_DRV_RECOVER,pNet->PortNr,-1,SK_TRUE);

		/* Reset the fifo counters */
		pNet->PreviousMACFifoRP = 0;
		pNet->PreviousMACFifoRLev = 0;
		pNet->PreviousRXFifoRP = 0;
		pNet->PreviousRXFifoRLev = 0;

		spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MSG,
		("<== CheckRxPath()\n"));
}



#endif


#ifdef CONFIG_PM
/*****************************************************************************
 *
 * 	sk98lin_resume - Resume the the card
 *
 * Description:
 *	This function resumes the card into the D0 state
 *
 * Returns: N/A
 *
 */
static int sk98lin_resume(
struct pci_dev *pdev)   /* the device that is to resume */
{
	struct net_device   *dev  = pci_get_drvdata(pdev);
	DEV_NET		    *pNet = PPRIV;
	SK_AC		    *pAC  = pNet->pAC;
	SK_U16		     PmCtlSts;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
	int                  rCode;
#endif

	/* Set the power state to D0 */
	pci_set_power_state(pdev, 0);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
	pci_restore_state(pdev);
#else
	pci_restore_state(pdev, pAC->PciState);
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
	rCode = pci_enable_device(pdev);
	if (rCode)
		return rCode;
#else
	pci_enable_device(pdev);
#endif
	pci_set_master(pdev);

	pci_enable_wake(pdev, 3, 0);
	pci_enable_wake(pdev, 4, 0);

	/* Set the adapter power state to D0 */
	SkPciReadCfgWord(pAC, PCI_PM_CTL_STS, &PmCtlSts);
	PmCtlSts &= ~(PCI_PM_STATE_D3);	/* reset all DState bits */
	PmCtlSts |= PCI_PM_STATE_D0;
	SkPciWriteCfgWord(pAC, PCI_PM_CTL_STS, PmCtlSts);

	/* Reinit the adapter and start the port again */
	pAC->BoardLevel = SK_INIT_DATA;
	SkDrvLeaveDiagMode(pAC);

	if (((pAC->WasIfUp[0] == SK_TRUE) || (pAC->WasIfUp[1] == SK_TRUE)) &&
		(pAC->Suspended == SK_TRUE)) {

		/* Enable interrupts */
		SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);
		if (CHIP_ID_YUKON_2(pAC)) {
			SK_OUT32(pAC->IoBase, B0_HWE_IMSK, Y2_IRQ_HWE_MASK);
		}
		else {
			SK_OUT32(pAC->IoBase, B0_HWE_IMSK, IRQ_HWE_MASK);
		}
	}
	pAC->Suspended = SK_FALSE;

#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	if (pAC->GIni.GIDontInitPhy == SK_TRUE) {

		/*
		 * Start queue and carrier, set interface to running,
		 * but don't enable the queue if SDK queue handling
		 * is active
		 */
		if (!pAC->SdkQH) {
			NETIF_WAKE_ALLQ(pAC->dev[pNet->PortNr]);
		}
		netif_carrier_on(dev);
		dev->flags |= IFF_RUNNING;
	}

	/* Map any waiting RX buffers to HW */
#ifdef USE_SK_RSS_SUPPORT
	SkSetRssSupport(pAC, pNet->PortNr);
#endif
	FillReceiveTableYukon2(pAC, pAC->IoBase, pNet->PortNr);
	pAC->GIni.GIDontInitPhy = SK_FALSE;
#endif
	printk("sk98lin: resume complete\n");
#endif

	return 0;

}


/*****************************************************************************
 *
 * 	sk98lin_suspend - Suspend the card
 *
 * Description:
 *	This function suspends the card into a defined state
 *
 * Returns: N/A
 *
 */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
static int sk98lin_suspend(
struct pci_dev	*pdev,   /* pointer to the device that is to suspend */
pm_message_t	state)  /* what power state is desired by Linux?    */
#else
static int sk98lin_suspend(
struct pci_dev	*pdev,   /* pointer to the device that is to suspend */
SK_U32	state)  /* what power state is desired by Linux?    */
#endif
{
	struct net_device   *dev  = pci_get_drvdata(pdev);
	DEV_NET		    *pNet = PPRIV;
	SK_AC		    *pAC  = pNet->pAC;

	/* First Yukon revs do not support power management */
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON) {
		if (pAC->GIni.GIChipRev == 0) {
			return 0; /* power management not supported */
		}
	}

	if(netif_running(dev)) {
		NETIF_STOP_ALLQ(dev); /* stop device if running */
	}

	pAC->Suspended = SK_TRUE;

	SkDrvEnterDiagMode(pAC);
	SkEnableWOMagicPacket(pAC, pAC->IoBase);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,15)
	device_set_wakeup_enable(&pdev->dev, 1);
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
	pci_save_state(pdev);
#else
	pci_save_state(pdev, pAC->PciState);
#endif
	/* Possibly we need to evaluate the return values */
	pci_enable_wake(pdev, 3, 1);	/* D3 hot */
	pci_enable_wake(pdev, 4, 1);	/* D3 cold */

	pci_disable_device(pdev);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	pci_set_power_state(pdev, pci_choose_state(pdev, state)); /* set the state */
#else
	pci_set_power_state(pdev, state); /* set the state */
#endif

#ifdef MV_INCLUDE_SDK_SUPPORT
	printk("sk98lin: suspend complete\n");
#endif

	return 0;
}


/******************************************************************************
 *
 *	SkEnableWOMagicPacket - Enable Wake on Magic Packet on the adapter
 *
 * Context:
 *	init, pageable
 *	the adapter should be de-initialized before calling this function
 *
 * Returns:
 *	nothing
 */

static void SkEnableWOMagicPacket(
SK_AC	*pAC,		/* Adapter Control Context */
SK_IOC	IoC)		/* I/O control context */
{
	SK_U16	Word;
	int 	Port;
	int 	i;
#ifndef MV_INCLUDE_SDK_SUPPORT
	SK_U32	PhyCtrl;
	SK_U8	Leave;
	int 	LoopCount;
#endif

	/* WOL only supported by single port adapters */
	Port = 0;

	/* Enable VAUX */
	SK_OUT8(IoC, B0_POWER_CTRL, (PC_VAUX_ENA | PC_VCC_ENA | PC_VAUX_ON | PC_VCC_OFF));

#ifndef MV_INCLUDE_SDK_SUPPORT
	SetDynamicClockGating(pAC, IoC);

	/* Set up magic packet parameters */
	if (pAC->WolInfo.ConfiguredWolOptions != 0) {
		for (i = 0; i < 6; i+=2) {				/* Set up magic packet MAC address */
			SK_IN16(IoC, B2_MAC_1 + i, &Word);
			SK_OUT16(IoC, WOL_MAC_ADDR_LO + i, Word);
		}

		Word = 0x0;
		Word |= WOL_CTL_ENA_PME_ON_MAGIC_PKT;	/* Enable PME on magic packet */
		Word |= WOL_CTL_ENA_MAGIC_PKT_UNIT;		/* Enable magic packet unit */
		SK_OUT16(IoC, WOL_CTRL_STAT, Word);

		SK_OUT16(IoC, B0_CTST, Y2_HW_WOL_OFF);

		/* Set up WOL link */
		SkCheckWOParam(pAC, Port);
		SkGmInitMac(pAC, IoC, Port);

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_PRM) {
			Leave = SK_FALSE;
			LoopCount = 0;

			do {
				/* Check if link is up */
				SK_IN32(IoC, MR_ADDR(Port, GPHY_CTRL), &PhyCtrl);
				if ((PhyCtrl & GPC_PHY_LINK_UP) != 0) {
					Leave = SK_TRUE;
				}
				msleep(SK_WOL_LINK_WAIT_DELAY);
				LoopCount++;
			} while ((Leave != SK_TRUE) && (LoopCount < SK_WOL_LINK_MAX_LOOP));
		}

		GM_IN16(IoC, Port, GM_GP_CTRL, &Word);
		Word |= (GM_GPCR_TX_ENA | GM_GPCR_RX_ENA);
		GM_OUT16(IoC, Port, GM_GP_CTRL, Word);
	}
#else
	/* Disable magic packet parameters */
	for (i = 0; i < 6; i+=2) {				/* Delete magic packet MAC address */
		SK_OUT16(IoC, WOL_MAC_ADDR_LO + i, 0x0);
	}

	Word = 0x0;
	Word |= WOL_CTL_DIS_PME_ON_MAGIC_PKT;	/* Disable PME on magic packet */
	Word |= WOL_CTL_DIS_MAGIC_PKT_UNIT;		/* Disable magic packet unit */
	Word |= WOL_CTL_DIS_PME_ON_LINK_CHG;	/* Disable PME on link change */
	Word |= WOL_CTL_DIS_LINK_CHG_UNIT;		/* Disable link change unit */
	SK_OUT16(IoC, WOL_CTRL_STAT, Word);

	SK_OUT16(IoC, B0_CTST, Y2_HW_WOL_OFF);

#ifndef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	/* Set up WOL link */
	SkCheckWOParam(pAC, Port);
	SkGmInitMac(pAC, IoC, Port);

	GM_IN16(IoC, Port, GM_GP_CTRL, &Word);
	Word |= (GM_GPCR_TX_ENA | GM_GPCR_RX_ENA);
	GM_OUT16(IoC, Port, GM_GP_CTRL, Word);
#endif

	/* Hand over control to firmware */
	if (pAC->FwRun && pAC->FwState) {
		FwDriverGoodbye(pAC, IoC);
	}
#endif

} /* SkEnableWOMagicPacket */


/*****************************************************************************
 *
 *      SkCheckWOParam - Check the WOL link parameters
 *
 * Description:
 *
 *	Possible configurations:
 *	Autonegotiation is enabled, advertise 10 HD, 10 FD,
 *	100 HD and 100 FD.
 *
 * Returns:     N/A
 */
static void SkCheckWOParam(
SK_AC	*pAC,	/* Pointer to adapter context */
int		Port)
{
	/* Set the speed capabilities */
	if (pAC->GIni.GICopperType) {
		switch (pAC->WolSpeedType) {
		case SK_LSPEED_AUTO:
			pAC->GIni.GP[Port].PLinkSpeed = SK_LSPEED_AUTO;
			break;
		case SK_LSPEED_100MBPS:
			pAC->GIni.GP[Port].PLinkSpeed = SK_LSPEED_100MBPS;
			break;
		case SK_LSPEED_10MBPS:
			pAC->GIni.GP[Port].PLinkSpeed = SK_LSPEED_10MBPS;
			break;
		default:
			pAC->GIni.GP[Port].PLinkSpeed = SK_LSPEED_100MBPS;
		}
	}

	/* Set the duplex mode */
	switch (pAC->WolDuplexType) {
	case SK_TRUE:
		pAC->GIni.GP[Port].PLinkMode = SK_LMODE_AUTOFULL;
		break;
	case SK_FALSE:
		pAC->GIni.GP[Port].PLinkMode = SK_LMODE_AUTOHALF;
		break;
	default:
		pAC->GIni.GP[Port].PLinkMode = SK_LMODE_AUTOFULL;
	}

	/* Set the flow control mode */
	pAC->GIni.GP[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;

} /* SkCheckWOParam */

#ifndef MV_INCLUDE_SDK_SUPPORT
/******************************************************************************
 *
 *	SetDynamicClockGating - Enable dynamic clock gating
 *
 * Context:
 *
 * Returns:
 *	nothing
 */

static void SetDynamicClockGating(
SK_AC         *pAC,      /* Adapter Control Context          */
SK_IOC         IoC)      /* I/O control context              */
{
	SK_U32 TmpVal32;
	SK_U32 DCMask32;
	SK_U8  TmpVal8;
	SK_U16 SWord;

	/* Enable Config write (Reg 0x158) */
	SK_IN8(pAC->IoBase, B2_TST_CTRL1, &TmpVal8 );
	TmpVal8 &= ~TST_CFG_WRITE_OFF;
	TmpVal8 |= TST_CFG_WRITE_ON;
	SK_OUT8(pAC->IoBase, B2_TST_CTRL1, TmpVal8 );

	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PCI_OUR_REG_3, 0);

	SK_IN32(pAC->IoBase, Y2_CFG_SPC + PCI_OUR_REG_4, &TmpVal32);
	DCMask32 = PCIE_OUR4_DYN_CLK_GATE_SET | P_ASPM_FORCE_CLKREQ_ENA;
	DCMask32 &= ~P_TIMER_VALUE_MSK; /* 0x68003015 */
	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PCI_OUR_REG_4, TmpVal32 | DCMask32);

#if 0
	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PCI_OUR_REG_5,
		PCIE_OUR5_EVENT_CLK_D3_SET |
		PCIE_OUR5_EPROM_LDR_FIN |
		PCIE_OUR5_EPROM_LDR_NOT_FIN |
		P_GAT_CPU_TO_SLEEP |
		P_REL_CPU_TO_SLEEP); /* 0x034a8b4a */
#else
	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PCI_OUR_REG_5, 0x034a8b4a);
#endif

	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PCI_CFG_REG_1,
		PCIE_CFG1_EVENT_CLK_D3_SET); /* 0x01f60103 */

	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + CONFIG_REG0, CONFIG_REG0_CLK_RUN_ASF |
			CONFIG_REG0_CLK_RUN_FLASH);

	SK_IN32(pAC->IoBase, Y2_CFG_SPC + PSM_CONFIG_REG1, &TmpVal32);
	TmpVal32 |= PSM_CONFIG_REG1_CLK_RUN_ASF;
	SK_OUT32(pAC->IoBase, Y2_CFG_SPC + PSM_CONFIG_REG1, TmpVal32);

	SkGmPhyWrite(pAC, pAC->IoBase, 0, PHY_MARV_EXT_ADR, 0);
	SkGmPhyRead(pAC, pAC->IoBase, 0, PHY_MARV_EXT_CTRL_2, &SWord);
	SkGmPhyWrite(pAC, pAC->IoBase, 0, PHY_MARV_EXT_CTRL_2, SWord | BIT_0S);
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,11)
/*****************************************************************************
 *
 * 	sk98lin_shutdown
 *
 * Description:
 *	This function suspends the card into a defined state
 *
 * Returns: N/A
 *
 */
static void sk98lin_shutdown(struct pci_dev *pdev) {
	sk98lin_suspend(pdev, PMSG_SUSPEND);
}
#endif

#endif


/*****************************************************************************
 *
 * 	FreeResources - release resources allocated for adapter
 *
 * Description:
 *	This function releases the IRQ, unmaps the IO and
 *	frees the desriptor ring.
 *
 * Returns: N/A
 *
 */
static void FreeResources(struct SK_NET_DEVICE *dev)
{
DEV_NET		*pNet;
SK_AC		*pAC;

	if (PRIV) {
		pNet = PPRIV;
		pAC = pNet->pAC;

		if (pAC->PciDev) {
			pci_release_regions(pAC->PciDev);
		}
		if (pAC->IoBase) {
			iounmap(pAC->IoBase);
		}
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2FreeResources(pAC);
		} else {
			BoardFreeMem(pAC);
		}
	}

} /* FreeResources */

MODULE_AUTHOR("Mirko Lindner <support@marvell.com>");
MODULE_DESCRIPTION("Marvell/SysKonnect Yukon Ethernet Network Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

#ifdef LINK_SPEED_A
static char *Speed_A[SK_MAX_CARD_PARAM] = LINK_SPEED;
#else
static char *Speed_A[SK_MAX_CARD_PARAM];
#endif

#ifdef LINK_SPEED_B
static char *Speed_B[SK_MAX_CARD_PARAM] = LINK_SPEED;
#else
static char *Speed_B[SK_MAX_CARD_PARAM];
#endif

#ifdef AUTO_NEG_A
static char *AutoNeg_A[SK_MAX_CARD_PARAM] = AUTO_NEG_A;
#else
static char *AutoNeg_A[SK_MAX_CARD_PARAM];
#endif

#ifdef DUP_CAP_A
static char *DupCap_A[SK_MAX_CARD_PARAM] = DUP_CAP_A;
#else
static char *DupCap_A[SK_MAX_CARD_PARAM];
#endif

#ifdef FLOW_CTRL_A
static char *FlowCtrl_A[SK_MAX_CARD_PARAM] = FLOW_CTRL_A;
#else
static char *FlowCtrl_A[SK_MAX_CARD_PARAM];
#endif

#ifdef ROLE_A
static char *Role_A[SK_MAX_CARD_PARAM] = ROLE_A;
#else
static char *Role_A[SK_MAX_CARD_PARAM];
#endif

#ifdef AUTO_NEG_B
static char *AutoNeg_B[SK_MAX_CARD_PARAM] = AUTO_NEG_B;
#else
static char *AutoNeg_B[SK_MAX_CARD_PARAM];
#endif

#ifdef DUP_CAP_B
static char *DupCap_B[SK_MAX_CARD_PARAM] = DUP_CAP_B;
#else
static char *DupCap_B[SK_MAX_CARD_PARAM];
#endif

#ifdef FLOW_CTRL_B
static char *FlowCtrl_B[SK_MAX_CARD_PARAM] = FLOW_CTRL_B;
#else
static char *FlowCtrl_B[SK_MAX_CARD_PARAM];
#endif

#ifdef ROLE_B
static char *Role_B[SK_MAX_CARD_PARAM] = ROLE_B;
#else
static char *Role_B[SK_MAX_CARD_PARAM];
#endif

#ifdef CON_TYPE
static char *ConType[SK_MAX_CARD_PARAM] = CON_TYPE;
#else
static char *ConType[SK_MAX_CARD_PARAM];
#endif

#ifdef WOL_TYPE
static char *WolType[SK_MAX_CARD_PARAM] = WOL_TYPE;
#else
static char *WolType[SK_MAX_CARD_PARAM];
#endif

static int  IntsPerSec[SK_MAX_CARD_PARAM];
static char *Moderation[SK_MAX_CARD_PARAM];
static char *ModerationMask[SK_MAX_CARD_PARAM];
static int  TxModeration[SK_MAX_CARD_PARAM];
static char *LowLatency[SK_MAX_CARD_PARAM];
static char *MsiIrq[SK_MAX_CARD_PARAM];
#ifdef USE_SK_RSS_SUPPORT
static char *RSS[SK_MAX_CARD_PARAM];
#endif

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
static char *LinkMaintenance[SK_MAX_CARD_PARAM];
#endif
#ifdef MV_INCLUDE_SDK_SUPPORT
static char *SdkQH[SK_MAX_CARD_PARAM];
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
module_param_array(Speed_A, charp, NULL, 0);
module_param_array(Speed_B, charp, NULL, 0);
module_param_array(AutoNeg_A, charp, NULL, 0);
module_param_array(AutoNeg_B, charp, NULL, 0);
module_param_array(DupCap_A, charp, NULL, 0);
module_param_array(DupCap_B, charp, NULL, 0);
module_param_array(FlowCtrl_A, charp, NULL, 0);
module_param_array(FlowCtrl_B, charp, NULL, 0);
module_param_array(Role_A, charp, NULL, 0);
module_param_array(Role_B, charp, NULL, 0);
module_param_array(ConType, charp, NULL, 0);
module_param_array(WolType, charp, NULL, 0);
module_param_array(IntsPerSec, int, NULL, 0);
module_param_array(Moderation, charp, NULL, 0);
module_param_array(ModerationMask, charp, NULL, 0);
module_param_array(LowLatency, charp, NULL, 0);
module_param_array(TxModeration, int, NULL, 0);
module_param_array(MsiIrq, charp, NULL, 0);
#ifdef USE_SK_RSS_SUPPORT
module_param_array(RSS, charp, NULL, 0);
#endif

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
module_param_array(LinkMaintenance, charp, NULL, 0);
#endif
#ifdef MV_INCLUDE_SDK_SUPPORT
module_param_array(SdkQH, charp, NULL, 0);
#endif
#else
MODULE_PARM(Speed_A,          "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(Speed_B,          "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(AutoNeg_A,        "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(AutoNeg_B,        "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(DupCap_A,         "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(DupCap_B,         "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(FlowCtrl_A,       "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(FlowCtrl_B,       "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(Role_A,           "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(Role_B,           "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(ConType,	      "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(WolType,	      "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(IntsPerSec,       "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "i");
MODULE_PARM(Moderation,       "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(ModerationMask,   "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(LowLatency,       "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
MODULE_PARM(TxModeration,     "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "i");
MODULE_PARM(MsiIrq,           "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
#ifdef USE_SK_RSS_SUPPORT
MODULE_PARM(RSS,              "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
#endif
#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
MODULE_PARM(LinkMaintenance,  "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
#endif
MODULE_PARM(SdkQH,            "1-" __MODULE_STRING(SK_MAX_CARD_PARAM) "s");
#endif
#endif


/*****************************************************************************
 *
 * 	sk98lin_remove_device - device deinit function
 *
 * Description:
 *	Disable adapter if it is still running, free resources,
 *	free device struct.
 *
 * Returns: N/A
 */

static void sk98lin_remove_device(struct pci_dev *pdev)
{
DEV_NET		*pNet;
SK_AC		*pAC;
struct SK_NET_DEVICE *next;
unsigned long Flags;
struct net_device *dev = pci_get_drvdata(pdev);

	/* Device not available. Return. */
	if (!dev)
		return;

	pNet = PPRIV;
	pAC = pNet->pAC;
	next = pAC->Next;

	NETIF_STOP_ALLQ(dev);

	SkAddrMcUpdate(pAC,pAC->IoBase, 0); /* Mac update */
	SkGeYellowLED(pAC, pAC->IoBase, 0);

	if(pAC->BoardLevel == SK_INIT_RUN) {
		/* board is still alive */
		spin_lock_irqsave(&pAC->SlowPathLock, Flags);
		SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
					0, -1, SK_FALSE);
		SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
					1, -1, SK_TRUE);

		/* Disable interrupts */
		SK_OUT32(pAC->IoBase, B0_IMSK, 0);
		SkGeDeInit(pAC, pAC->IoBase);

		spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
		pAC->BoardLevel = SK_INIT_DATA;
		/* We do NOT check here, if IRQ was pending, of course*/
	}

	if(pAC->BoardLevel == SK_INIT_IO) {
		/* board is still alive */
		SkGeDeInit(pAC, pAC->IoBase);
		pAC->BoardLevel = SK_INIT_DATA;
	}

#ifdef CONFIG_PROC_FS
	/* Remove the sk98lin procfs device entries */
	if ((pAC->GIni.GIMacsFound == 2) && pAC->RlmtNets == 2){
		if (pAC->InterfaceUp[1] == 1) {
			remove_proc_entry(pAC->dev[1]->name, pSkRootDir);
		}
	}
	if (pAC->InterfaceUp[0] == 1) {
		remove_proc_entry(pAC->dev[0]->name, pSkRootDir);
	}
#endif
	if ((pAC->GIni.GIMacsFound == 2) && pAC->RlmtNets == 2){
		unregister_netdev(pAC->dev[1]);
		free_netdev(pAC->dev[1]);
	}

	FreeResources(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	dev->get_stats = NULL;
#endif

#ifdef	SK_AVB
	unregister_chrdev (SWDEV_MAGIC, "sw");
#endif
	/*
	 * otherwise unregister_netdev calls get_stats with
	 * invalid IO ...  :-(
	 */
	unregister_netdev(dev);
	free_netdev(dev);

#ifdef MV_INCLUDE_SDK_SUPPORT
	if (pAC->FwBufferLen > 0) {
		/* Clear the old buffer */
		kfree(pAC->pFwBuffer);
	}
#endif

	kfree(pAC);
	sk98lin_max_boards_found--;

#ifdef CONFIG_PROC_FS
	/* Remove all Proc entries if last device */
	if (sk98lin_max_boards_found == 0) {
		/* clear proc-dir */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#ifndef SK_DISABLE_PROC_UNLOAD
		remove_proc_entry(pSkRootDir->name, proc_net);
#endif
#else
		remove_proc_entry(pSkRootDir->name, init_net.proc_net);
#endif
	}
#endif

}


/*****************************************************************************
 *
 * 	SkGeRequestIrq
 *
 * Description:
 *	Handle software interrupt used during MSI test
 * Returns:
 *	IRQ_HANDLED if everything is ok
 *      IRQ_NONE on error
 */
static int SK_DEVINIT SkGeRequestIrq(struct SK_NET_DEVICE *dev)
{
	DEV_NET *pNet = PPRIV;
	SK_AC   *pAC  = pNet->pAC;
	int	Ret;

	if (!CHIP_ID_YUKON_2(pAC)) {
		Ret = request_irq(dev->irq, SkGeIsr, SK_IRQ_SHARED, dev->name, dev);
	} else {
		if ((pAC->MsiIrq) && (pci_enable_msi(pAC->PciDev) == 0)) {
			if (SkGeTestInt(dev, pAC, 0)) {
				/* MSI test failed, go back to INTx mode */
				printk("sk98lin: MSI enable error\n");
				SK_OUT8(pAC->IoBase, B0_CTST, CS_CL_SW_IRQ);
				pci_disable_msi(pAC->PciDev);
				pAC->AllocFlag &= ~SK_ALLOC_MSI;
			} else {
				pAC->AllocFlag |= SK_ALLOC_MSI;
				dev->irq = pAC->PciDev->irq;
			}
		}

		Ret = request_irq(dev->irq, SkY2Isr,
			(pAC->AllocFlag & SK_ALLOC_MSI) ? 0 : SK_IRQ_SHARED,
			dev->name, dev);
	}

	if (Ret) {
		printk(KERN_WARNING "sk98lin: Requested IRQ %d is busy.\n",
			dev->irq);
		return -EAGAIN;
	} else if ((pAC->AllocFlag & SK_ALLOC_MSI) == 0) {
		pAC->AllocFlag |= SK_ALLOC_IRQ;
	}

	pAC->IrqDev = dev;
	return 0;
}


/*****************************************************************************
 *
 * 	SkGeReleaseIrq
 *
 * Description:
 *	Handle software interrupt used during MSI test
 * Returns:
 *	IRQ_HANDLED if everything is ok
 *      IRQ_NONE on error
 */
static void SkGeReleaseIrq(struct SK_NET_DEVICE *dev)
{
	DEV_NET *pNet;
	SK_AC   *pAC;

	if (PRIV) {
		pNet = PPRIV;
		pAC = pNet->pAC;
		free_irq(dev->irq, dev);

		if (pAC->AllocFlag & SK_ALLOC_MSI) {
			pci_disable_msi(pAC->PciDev);
		}
	}

}


/*****************************************************************************
 *
 * 	SkGeTestIntr - SW interrupt test
 *
 * Description:
 *	Handle software interrupt used during MSI test
 * Returns:
 *	IRQ_HANDLED if everything is ok
 *      IRQ_NONE on error
 */
#ifdef SK_NEW_ISR_DEFINITION
static int SK_DEVINIT SkGeTestIsr(int irq, void *dev_id)
#else
static int SK_DEVINIT SkGeTestIsr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif
{
	struct SK_NET_DEVICE *dev = (struct SK_NET_DEVICE *)dev_id;
	DEV_NET         *pNet;
	SK_AC           *pAC;
	SK_U32		status;

	pNet = PPRIV;
	pAC = pNet->pAC;

	SK_IN32(pAC->IoBase, B0_Y2_SP_ISRC2, &status);

	if (status == 0)
		return IRQ_NONE;

	if (status & Y2_IS_IRQ_SW) {
		pAC->AllocFlag |= SK_ALLOC_OK;
		wake_up(&pAC->irq_wait);
		SK_OUT32(pAC->IoBase, B0_CTST, CS_CL_SW_IRQ);
	}

	SK_OUT32(pAC->IoBase, B0_Y2_SP_ICR, 2);
	return IRQ_HANDLED;
}


/*****************************************************************************
 *
 * 	SkGeTestMsi - Test interrupt path
 *
 * Description:
 *
 * Returns:
 *	0, if everything is ok
 *      !=0, on error
 */
int SK_DEVINIT SkGeTestInt(struct SK_NET_DEVICE *dev, SK_AC *pAC, SK_U32 Int)
{
	struct pci_dev *pdev = pAC->PciDev;
	int		Ret = 0;
	SK_U32		test32;
	SK_U8		test8;


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
	Ret = request_irq(pdev->irq, SkGeTestIsr, Int, dev->name, dev);
#else
	Ret = request_irq(pdev->irq, (irq_handler_t) SkGeTestIsr, Int, dev->name, dev);
#endif

	if (Ret) {
		dev_err(&pdev->dev, "cannot assign irq %d\n", pdev->irq);
		return Ret;
	}

	init_waitqueue_head (&pAC->irq_wait);

	/* Enable SW IRQ */
	SK_OUT32(pAC->IoBase, B0_IMSK, Y2_IS_IRQ_SW);

	SK_OUT8(pAC->IoBase, B0_CTST, CS_ST_SW_IRQ);
	SK_IN8(pAC->IoBase, B0_CTST, &test8);

	wait_event_timeout(pAC->irq_wait, (pAC->AllocFlag & SK_ALLOC_OK), HZ/10);
	if (!(pAC->AllocFlag & SK_ALLOC_OK))
		Ret = -EOPNOTSUPP;

	/* Disable the interrupts */
	SK_OUT32(pAC->IoBase, B0_IMSK, 0);
	SK_IN32(pAC->IoBase, B0_IMSK, &test32);
	free_irq(pdev->irq, dev);

	return Ret;
}

#ifdef USE_SK_RSS_SUPPORT
/*****************************************************************************
 *
 * 	SkSetRssSupport - Set the RSS support in the hardware
 *
 * Description:
 *
 * Returns:
 *	N/A
 */
static void SkSetRssSupport(SK_AC *pAC, int Port)
{
	SK_U32 i;
	SK_U32 nkeys = 4;
	nkeys = pAC->GIni.GINumOfRssKeys;

	if (!nkeys) {
	/* Don't handle RSS */
		return;
	}

	if (HW_FEATURE(pAC, HWF_WA_DEV_4229)) {
		SK_OUT32(pAC->IoBase, 0x434, BIT_13);
	}

	/*
	 * Starting with Extreme B0 we can configure the RSS based on HashType.
	 * Supports IPv6 and other modes
	 */
	if(HW_IS_EXT_LE_FORMAT(pAC)) {
		SK_OUT32(pAC->IoBase, RSS_CFG, 0x3f);	/* Hash all */
	}

	/* Program RSS initial values */
	if (pAC->dev[Port]->features & NETIF_F_RXHASH) {
		SK_U32 key[nkeys];

		get_random_bytes(key, nkeys * sizeof(u32));

		for (i = 0; i < nkeys; i++) {
			SK_OUT32(pAC->IoBase, RSS_KEY_ADDR(Port, (i*4)), key[i]);
		}

		SkGeRxRss(pAC, pAC->IoBase, Port, SK_TRUE);
	} else {
	/* Disable hashing */
		SK_OUT32(pAC->IoBase, Q_ADDR(pAC->GIni.GP[Port].PRxQOff, Q_CSR),
			     BMU_DIS_RX_RSS_HASH);
	}
}
#endif

/*****************************************************************************
 *
 * 	SkGeBoardInit - do level 0 and 1 initialization
 *
 * Description:
 *	This function prepares the board hardware for running. The desriptor
 *	ring is set up, the IRQ is allocated and the configuration settings
 *	are examined.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static int SkGeBoardInit(struct SK_NET_DEVICE *dev, SK_AC *pAC)
{
	short	i;
	char	*DescrString = "sk98lin: Driver for Linux"; /* this is given to PNMI */
	char	*VerStr	= VER_STRING;
#ifndef MV_INCLUDE_SDK_SUPPORT
	unsigned long   Flags;    	/* for the spin locks	*/
	SK_U32		TmpVal32;
#endif


	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("IoBase: %08lX\n", (unsigned long)pAC->IoBase));
	for (i=0; i<SK_MAX_MACS; i++) {
		pAC->TxPort[i].HwAddr = pAC->IoBase + TxQueueAddr[i][0];
		pAC->TxPort[i].PortIndex = i;
		pAC->RxPort[i].HwAddr = pAC->IoBase + RxQueueAddr[i];
		pAC->RxPort[i].PortIndex = i;
	}

	/* Initialize the mutexes */
	for (i=0; i<SK_MAX_MACS; i++) {
		spin_lock_init(&pAC->TxPort[i].TxDesRingLock);
		spin_lock_init(&pAC->RxPort[i].RxDesRingLock);
	}

	spin_lock_init(&pAC->InitLock);		/* Init lock */
	spin_lock_init(&pAC->SlowPathLock);
	spin_lock_init(&pAC->TxQueueLock);	/* for Yukon2 chipsets */
	spin_lock_init(&pAC->SetPutIndexLock);	/* for Yukon2 chipsets */
#ifdef MV_INCLUDE_SDK_SUPPORT
	spin_lock_init(&pAC->FwFifoLock);	/* For Yukon2 SDK */
#endif

	/* level 0 init common modules here */
#ifdef MV_INCLUDE_SDK_SUPPORT
	spin_lock(&pAC->SlowPathLock);
#else
	spin_lock_irqsave(&pAC->SlowPathLock, Flags);
#endif

	/* Does a RESET on board ...*/
	if (SkGeInit(pAC, pAC->IoBase, SK_INIT_DATA) != 0) {
		printk("HWInit (0) failed.\n");
#ifdef MV_INCLUDE_SDK_SUPPORT
		spin_unlock(&pAC->SlowPathLock);
#else
		spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
#endif
		return(-EAGAIN);
	}
	SkI2cInit(  pAC, pAC->IoBase, SK_INIT_DATA);
	SkEventInit(pAC, pAC->IoBase, SK_INIT_DATA);
	SkPnmiInit( pAC, pAC->IoBase, SK_INIT_DATA);
	SkAddrInit( pAC, pAC->IoBase, SK_INIT_DATA);
	SkTimerInit(pAC, pAC->IoBase, SK_INIT_DATA);
#ifdef MV_INCLUDE_SDK_SUPPORT
	if (FwInit(pAC, pAC->IoBase, SK_INIT_DATA) != SK_TRUE) {
		printk("FwInit(SK_INIT_DATA) failed!\n");
		spin_unlock(&pAC->SlowPathLock);
		return(-EAGAIN);
	}
#else
	/* Stop the firmware */
	if (( pAC->GIni.GIChipId  == CHIP_ID_YUKON_EX ) ||
		( pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR ))  {
		if( pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR )
			SK_OUT32(pAC->IoBase, CPU_WDOG, 0); /* stop the watch dog */

		SK_IN32( pAC->IoBase, HCU_CCSR, &TmpVal32 );
		TmpVal32 &= ~(BIT_0 | BIT_1);
		SK_OUT32( pAC->IoBase, HCU_CCSR, TmpVal32 );
	}
	else  {
		SK_IN32( pAC->IoBase, HCU_CCSR, &TmpVal32 );
		TmpVal32 &= ~(BIT_2 | BIT_3);
		TmpVal32 |= BIT_3;
		SK_OUT32( pAC->IoBase, HCU_CCSR, TmpVal32 );
	}
#endif

	pAC->BoardLevel = SK_INIT_DATA;
	pAC->RxPort[0].RxBufSize = ETH_BUF_SIZE;
	pAC->RxPort[1].RxBufSize = ETH_BUF_SIZE;

	SK_PNMI_SET_DRIVER_DESCR(pAC, DescrString);
	SK_PNMI_SET_DRIVER_VER(pAC, VerStr);

	/* level 1 init common modules here (HW init) */
	if (SkGeInit(pAC, pAC->IoBase, SK_INIT_IO) != 0) {
		printk("sk98lin: HWInit (1) failed.\n");
#ifdef MV_INCLUDE_SDK_SUPPORT
		spin_unlock(&pAC->SlowPathLock);
#else
		spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
#endif
		return(-EAGAIN);
	}
	SkI2cInit(  pAC, pAC->IoBase, SK_INIT_IO);
	SkEventInit(pAC, pAC->IoBase, SK_INIT_IO);
	SkPnmiInit( pAC, pAC->IoBase, SK_INIT_IO);
	SkAddrInit( pAC, pAC->IoBase, SK_INIT_IO);
	SkTimerInit(pAC, pAC->IoBase, SK_INIT_IO);
#ifdef MV_INCLUDE_SDK_SUPPORT
	if (FwInit(pAC, pAC->IoBase, SK_INIT_IO) != SK_TRUE) {
		printk("sk98lin: SDK support for this device disabled!\n");
		pAC->FwRun = SK_FALSE;
	} else {
		pAC->FwRun = SK_TRUE;
	}

#endif

#ifdef Y2_RECOVERY
	/* mark entries invalid */
	pAC->LastPort = 3;
	pAC->LastOpc = 0xFF;
#endif

	/* Set chipset type support */
	if ((pAC->GIni.GIChipId == CHIP_ID_YUKON) ||
		(pAC->GIni.GIChipId == CHIP_ID_YUKON_LITE) ||
		(pAC->GIni.GIChipId == CHIP_ID_YUKON_LP)) {
		pAC->ChipsetType = 1;	/* Yukon chipset (descriptor logic) */
	} else if (CHIP_ID_YUKON_2(pAC)) {
		pAC->ChipsetType = 2;	/* Yukon2 chipset (list logic) */
	}

	/* wake on lan support */
	pAC->WolInfo.SupportedWolOptions = 0;
#if defined (ETHTOOL_GWOL) && defined (ETHTOOL_SWOL)
	pAC->WolInfo.SupportedWolOptions  = WAKE_MAGIC;
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON) {
		if (pAC->GIni.GIChipRev == 0) {
			pAC->WolInfo.SupportedWolOptions = 0;
		}
	}
#endif
	pAC->WolInfo.ConfiguredWolOptions = pAC->WolInfo.SupportedWolOptions;

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	pAC->GIni.GIDontInitPhy = SK_FALSE;
	pAC->LinkInfo[0].LinkMaintenance = SK_TRUE;
#endif

	GetConfiguration(pAC);
	SetConfiguration(pAC);

	if (pAC->RlmtNets == 2) {
		pAC->GIni.GP[0].PPortUsage = SK_MUL_LINK;
		pAC->GIni.GP[1].PPortUsage = SK_MUL_LINK;
	}

	/* Set the tx moderation parameter */
	if (pAC->TxModeration) {
		pAC->GIni.GITxIdxRepThres = pAC->TxModeration;
	}

	pAC->BoardLevel = SK_INIT_IO;
#ifdef MV_INCLUDE_SDK_SUPPORT
	spin_unlock(&pAC->SlowPathLock);
#else
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
#endif

	/*
	** Alloc descriptor/LETable memory for this board (both RxD/TxD)
	*/
	pAC->NrOfRxLe[0] = NUMBER_OF_RX_LE;
	pAC->NrOfTxLe[0] = NUMBER_OF_TX_LE;
	if (pAC->RlmtNets == 2) {
		pAC->NrOfRxLe[1] = NUMBER_OF_RX_LE;
		pAC->NrOfTxLe[1] = NUMBER_OF_TX_LE;
	}

	if (CHIP_ID_YUKON_2(pAC)) {
		if (!SkY2AllocateResources(pAC)) {
			printk("No memory for Yukon2 settings\n");
			return(-EAGAIN);
		}
	} else {
		if(!BoardAllocMem(pAC)) {
			printk("No memory for descriptor rings.\n");
			return(-EAGAIN);
		}
	}

#ifdef SK_USE_CSUM
	SkCsSetReceiveFlags(pAC,
		SKCS_PROTO_IP | SKCS_PROTO_TCP | SKCS_PROTO_UDP,
		&pAC->CsOfs1, &pAC->CsOfs2, 0);
	pAC->CsOfs = (pAC->CsOfs2 << 16) | pAC->CsOfs1;
#endif

	/*
	** Function BoardInitMem() for Yukon dependent settings...
	*/
	BoardInitMem(pAC);

	/*
	 * Register the device here
	 */
	pAC->Next = SkGeRootDev;
	SkGeRootDev = dev;

	return (0);
} /* SkGeBoardInit */


/*****************************************************************************
 *
 * 	BoardAllocMem - allocate the memory for the descriptor rings
 *
 * Description:
 *	This function allocates the memory for all descriptor rings.
 *	Each ring is aligned for the desriptor alignment and no ring
 *	has a 4 GByte boundary in it (because the upper 32 bit must
 *	be constant for all descriptiors in one rings).
 *
 * Returns:
 *	SK_TRUE, if all memory could be allocated
 *	SK_FALSE, if not
 */
SK_BOOL BoardAllocMem(
SK_AC	*pAC)
{
caddr_t		pDescrMem;	/* pointer to descriptor memory area */
size_t		AllocLength;	/* length of complete descriptor area */
int		i;		/* loop counter */
unsigned long	BusAddr;


	/* rings plus one for alignment (do not cross 4 GB boundary) */
	/* RX_RING_SIZE is assumed bigger than TX_RING_SIZE */
#if (BITS_PER_LONG == 32)
	AllocLength = (RX_RING_SIZE + TX_RING_SIZE) * pAC->GIni.GIMacsFound + 8;
#else
	AllocLength = (RX_RING_SIZE + TX_RING_SIZE) * pAC->GIni.GIMacsFound
		+ RX_RING_SIZE + 8;
#endif

	pDescrMem = pci_alloc_consistent(pAC->PciDev, AllocLength,
					 &pAC->pDescrMemDMA);

	if (pDescrMem == NULL)
		return (SK_FALSE);

	pAC->pDescrMem = pDescrMem;
	BusAddr = (unsigned long) pAC->pDescrMemDMA;

	/* Descriptors need 8 byte alignment, and this is ensured
	 * by pci_alloc_consistent.
	 */
	for (i=0; i<pAC->GIni.GIMacsFound; i++) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("TX%d/A: pDescrMem: %lX,   PhysDescrMem: %lX\n",
			i, (unsigned long) pDescrMem,
			BusAddr));
		pAC->TxPort[i].pTxDescrRing = pDescrMem;
		pAC->TxPort[i].VTxDescrRing = BusAddr;
		pDescrMem += TX_RING_SIZE;
		BusAddr += TX_RING_SIZE;

		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
			("RX%d: pDescrMem: %lX,   PhysDescrMem: %lX\n",
			i, (unsigned long) pDescrMem,
			(unsigned long)BusAddr));
		pAC->RxPort[i].pRxDescrRing = pDescrMem;
		pAC->RxPort[i].VRxDescrRing = BusAddr;
		pDescrMem += RX_RING_SIZE;
		BusAddr += RX_RING_SIZE;
	} /* for */

	return (SK_TRUE);
} /* BoardAllocMem */


/****************************************************************************
 *
 *	BoardFreeMem - reverse of BoardAllocMem
 *
 * Description:
 *	Free all memory allocated in BoardAllocMem: adapter context,
 *	descriptor rings, locks.
 *
 * Returns:	N/A
 */
void BoardFreeMem(
SK_AC		*pAC)
{
size_t		AllocLength;	/* length of complete descriptor area */

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("BoardFreeMem\n"));

	if (pAC->pDescrMem) {

#if (BITS_PER_LONG == 32)
		AllocLength = (RX_RING_SIZE + TX_RING_SIZE) * pAC->GIni.GIMacsFound + 8;
#else
		AllocLength = (RX_RING_SIZE + TX_RING_SIZE) * pAC->GIni.GIMacsFound
			+ RX_RING_SIZE + 8;
#endif

		pci_free_consistent(pAC->PciDev, AllocLength,
			    pAC->pDescrMem, pAC->pDescrMemDMA);
		pAC->pDescrMem = NULL;
	}
} /* BoardFreeMem */


/*****************************************************************************
 *
 * 	BoardInitMem - initiate the descriptor rings
 *
 * Description:
 *	This function sets the descriptor rings or LETables up in memory.
 *	The adapter is initialized with the descriptor start addresses.
 *
 * Returns:	N/A
 */
static void BoardInitMem(
SK_AC	*pAC)	/* pointer to adapter context */
{
int	i;		/* loop counter */
int	RxDescrSize;	/* the size of a rx descriptor rounded up to alignment*/
int	TxDescrSize;	/* the size of a tx descriptor rounded up to alignment*/

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("BoardInitMem\n"));

	if (!pAC->GIni.GIYukon2) {
		RxDescrSize = (((sizeof(RXD) - 1) / DESCR_ALIGN) + 1) * DESCR_ALIGN;
		pAC->RxDescrPerRing = RX_RING_SIZE / RxDescrSize;
		TxDescrSize = (((sizeof(TXD) - 1) / DESCR_ALIGN) + 1) * DESCR_ALIGN;
		pAC->TxDescrPerRing = TX_RING_SIZE / RxDescrSize;

		for (i=0; i<pAC->GIni.GIMacsFound; i++) {
			SetupRing(
				pAC,
				pAC->TxPort[i].pTxDescrRing,
				pAC->TxPort[i].VTxDescrRing,
				(RXD**)&pAC->TxPort[i].pTxdRingHead,
				(RXD**)&pAC->TxPort[i].pTxdRingTail,
				(RXD**)&pAC->TxPort[i].pTxdRingPrev,
				&pAC->TxPort[i].TxdRingFree,
				&pAC->TxPort[i].TxdRingPrevFree,
				SK_TRUE);
			SetupRing(
				pAC,
				pAC->RxPort[i].pRxDescrRing,
				pAC->RxPort[i].VRxDescrRing,
				&pAC->RxPort[i].pRxdRingHead,
				&pAC->RxPort[i].pRxdRingTail,
				&pAC->RxPort[i].pRxdRingPrev,
				&pAC->RxPort[i].RxdRingFree,
				&pAC->RxPort[i].RxdRingFree,
				SK_FALSE);
		}
	}
} /* BoardInitMem */

/*****************************************************************************
 *
 * 	SetupRing - create one descriptor ring
 *
 * Description:
 *	This function creates one descriptor ring in the given memory area.
 *	The head, tail and number of free descriptors in the ring are set.
 *
 * Returns:
 *	none
 */
static void SetupRing(
SK_AC		*pAC,
void		*pMemArea,	/* a pointer to the memory area for the ring */
uintptr_t	VMemArea,	/* the virtual bus address of the memory area */
RXD		**ppRingHead,	/* address where the head should be written */
RXD		**ppRingTail,	/* address where the tail should be written */
RXD		**ppRingPrev,	/* address where the tail should be written */
int		*pRingFree,	/* address where the # of free descr. goes */
int		*pRingPrevFree,	/* address where the # of free descr. goes */
SK_BOOL		IsTx)		/* flag: is this a tx ring */
{
int	i;		/* loop counter */
int	DescrSize;	/* the size of a descriptor rounded up to alignment*/
int	DescrNum;	/* number of descriptors per ring */
RXD	*pDescr;	/* pointer to a descriptor (receive or transmit) */
RXD	*pNextDescr;	/* pointer to the next descriptor */
RXD	*pPrevDescr;	/* pointer to the previous descriptor */
uintptr_t VNextDescr;	/* the virtual bus address of the next descriptor */

	if (IsTx == SK_TRUE) {
		DescrSize = (((sizeof(TXD) - 1) / DESCR_ALIGN) + 1) *
			DESCR_ALIGN;
		DescrNum = TX_RING_SIZE / DescrSize;
	} else {
		DescrSize = (((sizeof(RXD) - 1) / DESCR_ALIGN) + 1) *
			DESCR_ALIGN;
		DescrNum = RX_RING_SIZE / DescrSize;
	}

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS,
		("Descriptor size: %d   Descriptor Number: %d\n",
		DescrSize,DescrNum));

	pDescr = (RXD*) pMemArea;
	pPrevDescr = NULL;
	pNextDescr = (RXD*) (((char*)pDescr) + DescrSize);
	VNextDescr = VMemArea + DescrSize;
	for(i=0; i<DescrNum; i++) {
		/* set the pointers right */
		pDescr->VNextRxd = VNextDescr & 0xffffffffULL;
		pDescr->pNextRxd = pNextDescr;
		pDescr->TcpSumStarts = pAC->CsOfs;

		/* advance one step */
		pPrevDescr = pDescr;
		pDescr = pNextDescr;
		pNextDescr = (RXD*) (((char*)pDescr) + DescrSize);
		VNextDescr += DescrSize;
	}
	pPrevDescr->pNextRxd = (RXD*) pMemArea;
	pPrevDescr->VNextRxd = VMemArea;
	pDescr               = (RXD*) pMemArea;
	*ppRingHead          = (RXD*) pMemArea;
	*ppRingTail          = *ppRingHead;
	*ppRingPrev          = pPrevDescr;
	*pRingFree           = DescrNum;
	*pRingPrevFree       = DescrNum;
} /* SetupRing */


/*****************************************************************************
 *
 * 	PortReInitBmu - re-initiate the descriptor rings for one port
 *
 * Description:
 *	This function reinitializes the descriptor rings of one port
 *	in memory. The port must be stopped before.
 *	The HW is initialized with the descriptor start addresses.
 *
 * Returns:
 *	none
 */
static void PortReInitBmu(
SK_AC	*pAC,		/* pointer to adapter context */
int	PortIndex)	/* index of the port for which to re-init */
{
	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("PortReInitBmu "));

	/* set address of first descriptor of ring in BMU */
	SK_OUT32(pAC->IoBase, TxQueueAddr[PortIndex][0]+ Q_DA_L,
		(uint32_t)(((caddr_t)
		(pAC->TxPort[PortIndex].pTxdRingHead) -
		pAC->TxPort[PortIndex].pTxDescrRing +
		pAC->TxPort[PortIndex].VTxDescrRing) &
		0xFFFFFFFF));
	SK_OUT32(pAC->IoBase, TxQueueAddr[PortIndex][0]+ Q_DA_H,
		(uint32_t)(((caddr_t)
		(pAC->TxPort[PortIndex].pTxdRingHead) -
		pAC->TxPort[PortIndex].pTxDescrRing +
		pAC->TxPort[PortIndex].VTxDescrRing) >> 32));
	SK_OUT32(pAC->IoBase, RxQueueAddr[PortIndex]+Q_DA_L,
		(uint32_t)(((caddr_t)(pAC->RxPort[PortIndex].pRxdRingHead) -
		pAC->RxPort[PortIndex].pRxDescrRing +
		pAC->RxPort[PortIndex].VRxDescrRing) & 0xFFFFFFFF));
	SK_OUT32(pAC->IoBase, RxQueueAddr[PortIndex]+Q_DA_H,
		(uint32_t)(((caddr_t)(pAC->RxPort[PortIndex].pRxdRingHead) -
		pAC->RxPort[PortIndex].pRxDescrRing +
		pAC->RxPort[PortIndex].VRxDescrRing) >> 32));
} /* PortReInitBmu */


/****************************************************************************
 *
 *	SkGeIsr - handle adapter interrupts for single port adapter
 *
 * Description:
 *	The interrupt routine is called when the network adapter
 *	generates an interrupt. It may also be called if another device
 *	shares this interrupt vector with the driver.
 *	This is the same as above, but handles only one port.
 *
 * Returns: N/A
 *
 */
#ifdef SK_NEW_ISR_DEFINITION
static SkIsrRetVar SkGeIsr(int irq, void *dev_id)
#else
static SkIsrRetVar SkGeIsr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif
{
struct SK_NET_DEVICE *dev = (struct SK_NET_DEVICE *)dev_id;
DEV_NET		*pNet;
SK_AC		*pAC;
SK_U32		IntSrc;		/* interrupts source register contents */

	pNet = PPRIV;
	pAC = pNet->pAC;

	/*
	 * Check and process if its our interrupt
	 */
	SK_IN32(pAC->IoBase, B0_SP_ISRC, &IntSrc);
	if ((IntSrc == 0) && (!pNet->NetConsoleMode)) {
		return SkIsrRetNone;
	}

#ifdef CONFIG_SK98LIN_NAPI
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
	if (napi_schedule_prep(&pNet->napi))
#elif defined SK_NEW_NAPI_HANDLING
	if (netif_rx_schedule_prep(dev, &pNet->napi))
#else
	if (netif_rx_schedule_prep(dev))
#endif
	{
		CLEAR_AND_START_RX(0);
		CLEAR_TX_IRQ(0, 0);
		pAC->GIni.GIValIrqMask &= ~(NAPI_DRV_IRQS);
		SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
		__napi_schedule(&pNet->napi);
#elif defined SK_NEW_NAPI_HANDLING
		__netif_rx_schedule(dev, &pNet->napi);
#else
		__netif_rx_schedule(dev);
#endif
	}

#ifdef USE_TX_COMPLETE /* only if tx complete interrupt used */
	if (IntSrc & IS_XA1_F) {
		CLEAR_TX_IRQ(0, 0);
	}
#endif
#else
	while (((IntSrc & IRQ_MASK) & ~SPECIAL_IRQS) != 0) {
#if 0 /* software irq currently not used */
		if (IntSrc & IS_IRQ_SW) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_INT_SRC,
				("Software IRQ\n"));
		}
#endif
		if (IntSrc & IS_R1_F) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_INT_SRC,
				("EOF RX1 IRQ\n"));
			ReceiveIrq(pAC, &pAC->RxPort[0], SK_TRUE);
			CLEAR_AND_START_RX(0);
			SK_PNMI_CNT_RX_INTR(pAC, 0);
		}
#ifdef USE_TX_COMPLETE /* only if tx complete interrupt used */
		if (IntSrc & IS_XA1_F) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_INT_SRC,
				("EOF AS TX1 IRQ\n"));
			CLEAR_TX_IRQ(0, 0);
			SK_PNMI_CNT_TX_INTR(pAC, 0);
			spin_lock(&pAC->TxPort[0].TxDesRingLock);
			FreeTxDescriptors(pAC, &pAC->TxPort[0]);
			spin_unlock(&pAC->TxPort[0].TxDesRingLock);
		}
#if 0 /* only if sync. queues used */
		if (IntSrc & IS_XS1_F) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_INT_SRC,
				("EOF SY TX1 IRQ\n"));
			CLEAR_TX_IRQ(0, 1);
			SK_PNMI_CNT_TX_INTR(pAC, 0);
			spin_lock(&pAC->TxPort[0].TxDesRingLock);
			FreeTxDescriptors(pAC, 0, 1);
			spin_unlock(&pAC->TxPort[0].TxDesRingLock);
		}
#endif
#endif

		SK_IN32(pAC->IoBase, B0_ISRC, &IntSrc);
	} /* while (IntSrc & IRQ_MASK != 0) */
#endif

#ifndef CONFIG_SK98LIN_NAPI
	spin_lock(&pAC->TxPort[0].TxDesRingLock);
	FreeTxDescriptors(pAC, &pAC->TxPort[0]);
	spin_unlock(&pAC->TxPort[0].TxDesRingLock);
	ReceiveIrq(pAC, &pAC->RxPort[0], SK_TRUE);
	START_RX(0);
#endif

	IntSrc &= pAC->GIni.GIValIrqMask;
	if ((IntSrc & SPECIAL_IRQS) || pAC->CheckQueue) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_INT_SRC,
			("SPECIAL IRQ SP-Cards => %x\n", IntSrc));
		pAC->CheckQueue = SK_FALSE;
		spin_lock(&pAC->SlowPathLock);
		if (IntSrc & SPECIAL_IRQS)
			SkGeSirqIsr(pAC, pAC->IoBase, IntSrc);

		SkEventDispatcher(pAC, pAC->IoBase);
		spin_unlock(&pAC->SlowPathLock);
		START_RX(0);
	}

	/* IRQ is processed - Enable IRQs again*/
	SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);

	return SkIsrRetHandled;
} /* SkGeIsr */


/****************************************************************************
 *
 *	SkGeOpen - handle start of initialized adapter
 *
 * Description:
 *	This function starts the initialized adapter.
 *	The board level variable is set and the adapter is
 *	brought to full functionality.
 *	The device flags are set for operation.
 *	Do all necessary level 2 initialization, enable interrupts and
 *	give start command to RLMT.
 *
 * Returns:
 *	0 on success
 *	!= 0 on error
 */
int SkGeOpen(
struct SK_NET_DEVICE *dev)  /* the device that is to be opened */
{
	DEV_NET        *pNet = PPRIV;
	SK_AC          *pAC  = pNet->pAC;
	unsigned long   Flags;    	/* for the spin locks */
	SK_BOOL         DualNet;
	int             CurrMac;	/* loop ctr for ports */
	unsigned long   InitFlags;
#ifdef Y2_RECOVERY
	SK_U8           StatSpeed, StatDuplex;
	SK_U32          NewTimerInterval;
#endif
	SK_BOOL         ModuleCount = SK_FALSE;

#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_SET_FW_IP_DRIVER
	struct		in_device *pindev;
	SK_U32		ip_address;
	SK_U32		ip_netmask;
#endif
#endif
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *pProcFile;
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeOpen: pAC=0x%lX:\n", (unsigned long)pAC));

	spin_lock_irqsave(&pAC->InitLock, InitFlags);
	pAC->MaxPorts++;
	if (pAC->DiagModeActive == DIAG_ACTIVE) {
		if (pAC->Pnmi.DiagAttached == SK_DIAG_RUNNING)
			goto open_failed;
	}

	if (!try_module_get(THIS_MODULE)) {
		/* increase of usage count not possible */
		goto open_failed;
	}
	ModuleCount = SK_TRUE;
	SetConfiguration(pAC);

#ifdef MV_INCLUDE_SDK_SUPPORT
	spin_unlock_irqrestore(&pAC->InitLock, InitFlags);
	if ((pAC->FwRun) && (pAC->FwState) && ((pAC->BoardLevel == SK_INIT_RUN) ||
		(pAC->Suspended == SK_TRUE))) {
		FwDriverHello(pAC, pAC->IoBase);
	}

	if (pAC->FwRun && ((pAC->BoardLevel == SK_INIT_RUN) ||
		(pAC->Suspended == SK_TRUE)) &&
		(pAC->FwState == SK_FALSE)) {

		/* Communication DRV <=> FW is broken. Try to recover... */
		printk("%s: SkGeOpen: FW not running => Restart CPU!\n",
			pAC->dev[pNet->PortNr]->name);

		FwAppInit2(pAC, pAC->IoBase);
		FwDriverHello(pAC, pAC->IoBase);
	}
	spin_lock_irqsave(&pAC->InitLock, InitFlags);
#endif
#ifdef	SK_AVB
	atu_node_table_init();
#endif

	/* Set blink mode */
	if ((pAC->PciDev->vendor == 0x1186) || (pAC->PciDev->vendor == 0x11ab ))
		pAC->GIni.GILedBlinkCtrl = OEM_CONFIG_VALUE;

#ifdef MV_INCLUDE_SDK_SUPPORT
	pAC->FwOs.pFilePathName = FW_FILE_PATHNAME;
	pAC->FwOs.FileSize = ASF_FLASH_SIZE_384K;
#endif

#ifdef SK98LIN_DIAG_LOOPBACK
	pAC->YukonLoopbackStatus = GBE_LOOPBACK_NONE;
	pAC->LoopbackRunning = SK_FALSE;
#endif

	if (pAC->BoardLevel == SK_INIT_DATA) {
#ifdef MV_INCLUDE_SDK_SUPPORT
		if (pAC->FwRun && pAC->Suspended == SK_TRUE) {
			FwCheckLinkMode(pAC, pAC->IoBase, pNet->PortNr);
		}
#endif
		/* Level 1 init common modules here */
		if (SkGeInit(pAC, pAC->IoBase, SK_INIT_IO) != 0) {
			module_put(THIS_MODULE); /* decrease usage count */
			printk("%s: HWInit (1) failed.\n", pAC->dev[pNet->PortNr]->name);
			goto open_failed;
		}
		SkI2cInit	(pAC, pAC->IoBase, SK_INIT_IO);
		SkEventInit	(pAC, pAC->IoBase, SK_INIT_IO);
		SkPnmiInit	(pAC, pAC->IoBase, SK_INIT_IO);
		SkAddrInit	(pAC, pAC->IoBase, SK_INIT_IO);
		SkTimerInit	(pAC, pAC->IoBase, SK_INIT_IO);
#ifdef MV_INCLUDE_SDK_SUPPORT
		/* Nothing to do here if card returns from suspend... */
		if (pAC->Suspended == SK_FALSE) {
			if (FwInit(pAC, pAC->IoBase, SK_INIT_IO) != SK_TRUE) {
				printk("sk98lin: SDK support for this device disabled!\n");
				pAC->FwState = SK_FALSE;
			}
		}
#endif
		pAC->BoardLevel = SK_INIT_IO;
#ifdef Y2_RECOVERY
		/* mark entries invalid */
		pAC->LastPort = 3;
		pAC->LastOpc = 0xFF;
#endif
	}

#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
	printk("SkGeOpen: pAC->Suspended:                   %d\n",
		pAC->Suspended);
	printk("SkGeOpen: pAC->LinkInfo[0].LinkMaintenance: %d\n",
		pAC->LinkInfo[0].LinkMaintenance);
	printk("SkGeOpen: pAC->GIni.GIDontInitPhy:          %d\n",
		pAC->GIni.GIDontInitPhy);
#endif

	/* Transformerless mode */
	if ((pAC->PciDev->vendor == 0x11ab ) &&
		(pAC->PciDev->device == 0x4365) &&
		(pAC->PciDev->subsystem_vendor == 0x10b7) &&
		(pAC->PciDev->subsystem_device == 0x1700)) {

		/* Enable transformerless mode */
		pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_TRAFO_LESS_ENABLE;

		/* Set manual slave and MDX */
		pAC->GIni.HwF.Features[HW_FEAT_LIST] |= HWF_PHY_SET_SLAVE_MDIX;
	}

	if (pAC->BoardLevel != SK_INIT_RUN) {
		if (pAC->Suspended) {
		/* Set the configuration after suspend */
			SetConfiguration(pAC);
		}

		/* Level 2 init modules here, check return value. */
		if (SkGeInit(pAC, pAC->IoBase, SK_INIT_RUN) != 0) {
			module_put(THIS_MODULE); /* decrease usage count */
			printk("%s: HWInit (2) failed.\n", pAC->dev[pNet->PortNr]->name);
			goto open_failed;
		}

		SkI2cInit	(pAC, pAC->IoBase, SK_INIT_RUN);
		SkEventInit	(pAC, pAC->IoBase, SK_INIT_RUN);
		SkPnmiInit	(pAC, pAC->IoBase, SK_INIT_RUN);
		SkAddrInit	(pAC, pAC->IoBase, SK_INIT_RUN);
		SkTimerInit	(pAC, pAC->IoBase, SK_INIT_RUN);

#ifdef MV_INCLUDE_SDK_SUPPORT
		/* Nothing to do here if card returns from suspend... */
		if (pAC->FwRun && pAC->Suspended == SK_FALSE) {
			spin_unlock_irqrestore(&pAC->InitLock, InitFlags);
			if (FwInit(pAC, pAC->IoBase, SK_INIT_RUN) != SK_TRUE) {
				module_put(THIS_MODULE); /* decrease usage count */
				printk("%s: SkGeOpen: FwInit(SK_INIT_RUN) failed!\n",
					pAC->dev[pNet->PortNr]->name);
				spin_lock_irqsave(&pAC->InitLock, InitFlags);
				goto open_failed;
			}
			if (pAC->FwState) {
				FwDriverHello(pAC, pAC->IoBase);
			}
			spin_lock_irqsave(&pAC->InitLock, InitFlags);
		}
#endif
		pAC->BoardLevel = SK_INIT_RUN;
	}

	DualNet = SK_FALSE;
	if (pAC->RlmtNets == 2) {
		DualNet = SK_TRUE;
	}

	if (SkGeInitAssignRamToQueues(
		pAC,
		pAC->ActivePort,
		DualNet)) {
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2FreeResources(pAC);
		} else {
			BoardFreeMem(pAC);
		}
		module_put(THIS_MODULE); /* decrease usage count */
		printk("sk98lin: SkGeInitAssignRamToQueues failed.\n");
		goto open_failed;
	}

	if (!CHIP_ID_YUKON_2(pAC)) {
		for (CurrMac=0; CurrMac<pAC->GIni.GIMacsFound; CurrMac++) {
			/* Enable transmit descriptor polling. */
			SkGePollTxD(pAC, pAC->IoBase, CurrMac, SK_TRUE);
			FillRxRing(pAC, &pAC->RxPort[CurrMac]);
			SkMacRxTxEnable(pAC, pAC->IoBase, CurrMac);
		}

		/*
		** Has been setup already at SkGeInit(SK_INIT_IO),
		** but additional masking added for Yukon
		** chipsets -> modify it...
		*/
		pAC->GIni.GIValIrqMask &= IRQ_MASK;
#ifndef USE_TX_COMPLETE
		pAC->GIni.GIValIrqMask &= ~(TX_COMPL_IRQS);
#endif
	}

	SkGeYellowLED(pAC, pAC->IoBase, 1);
	SkDimEnableModerationIfNeeded(pAC);

	spin_lock_irqsave(&pAC->SlowPathLock, Flags);

	/* Send event to set the number of nets in PNMI */
	SkLocalEventQueue(pAC, SKGE_PNMI, SK_PNMI_EVT_RLMT_SET_NETS,
		pAC->RlmtNets, -1, SK_TRUE);

	if (CHIP_ID_YUKON_2(pAC) && (pAC->MaxPorts == 1)) {
		/* Clear all LE tables */
		SK_MEMSET(pAC->pVirtMemAddr, 0, pAC->SizeOfAlignedLETables);

		/* Enable the status unit */
		pAC->StatusLETable.Done  = 0;
		pAC->StatusLETable.Put   = 0;
		pAC->StatusLETable.HwPut = 0;
		SkGeY2InitStatBmu(pAC, pAC->IoBase, &pAC->StatusLETable);
	}

	if (pAC->GIni.GIDontInitPhy == SK_FALSE) {
		/* Send event to start the ports (RX/TX) */
		SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START,
			pNet->PortNr, -1, SK_TRUE);
	}

	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);

#ifdef Y2_RECOVERY
	pNet->TimerExpired = SK_FALSE;
	pNet->InRecover = SK_FALSE;
	pNet->NetConsoleMode = SK_FALSE;

	StatSpeed = pAC->GIni.GP[pNet->NetNr].PLinkSpeedUsed;
	if (StatSpeed == SK_LSPEED_STAT_10MBPS) {
		StatDuplex = pAC->GIni.GP[pNet->NetNr].PLinkModeStatus;
		if ((StatDuplex == SK_LMODE_STAT_AUTOHALF) ||
			(StatDuplex == SK_LMODE_STAT_HALF)) {
				NewTimerInterval = (HZ*2);
		} else {
			NewTimerInterval = (HZ);
		}
	} else if (StatSpeed == SK_LSPEED_STAT_100MBPS) {
		NewTimerInterval = (HZ/2);
	} else if (StatSpeed == SK_LSPEED_STAT_1000MBPS) {
		NewTimerInterval = (HZ/4);
	} else {
		NewTimerInterval = (HZ*2);
	}

	/* Initialize the kernel timer */
	init_timer(&pNet->KernelTimer);
	pNet->KernelTimer.function	= SkGeHandleKernelTimer;
	pNet->KernelTimer.data		= (unsigned long) pNet;
	pNet->KernelTimer.expires	= jiffies + NewTimerInterval;
#endif

	/* Map any waiting RX buffers to HW */
	for (CurrMac=0; CurrMac<pAC->GIni.GIMacsFound; CurrMac++) {
#ifdef USE_SK_RSS_SUPPORT
		SkSetRssSupport(pAC, CurrMac);
#endif
		FillReceiveTableYukon2(pAC,pAC->IoBase, CurrMac);
	}

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	if (pAC->GIni.GIDontInitPhy == SK_TRUE) {
		SkY2PortStop(pAC, pAC->IoBase, pNet->PortNr, SK_STOP_ALL, SK_SOFT_RST);
		SkY2PortStart(pAC, pAC->IoBase, pNet->PortNr);
	}
#endif

	spin_unlock_irqrestore(&pAC->InitLock, InitFlags);
	if (pAC->RequestedIrq == SK_FALSE) {
		if ((SkGeRequestIrq(pAC->dev[0])) == 0) {
			pAC->RequestedIrq = SK_TRUE;

			if (pAC->Suspended == SK_FALSE) {

				/* Enable interrupts */
				SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);
				if (CHIP_ID_YUKON_2(pAC)) {
					SK_OUT32(pAC->IoBase, B0_HWE_IMSK, Y2_IRQ_HWE_MASK);
				}
				else {
				SK_OUT32(pAC->IoBase, B0_HWE_IMSK, IRQ_HWE_MASK);
				}
			}
		}
		else {
			printk("SkGeRequestIrq() failed => Interrupts not enabled!\n");
			spin_lock_irqsave(&pAC->InitLock, InitFlags);
			goto open_failed;
		}
	}
	spin_lock_irqsave(&pAC->InitLock, InitFlags);

#ifdef CONFIG_PROC_FS
	/* Initialize the procfs timer */
	init_timer(&pNet->ProcfsTimer);
	pNet->ProcfsTimer.function	= SkGeHandleProcfsTimer;
	pNet->ProcfsTimer.data		= (unsigned long) pNet;
	pNet->ProcfsTimer.expires	= jiffies + HZ*5; /* initially 5 secs */
	add_timer(&pNet->ProcfsTimer);
#endif
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_LITE) {
		SK_OUT8(pAC->IoBase, B0_POWER_CTRL, (SK_U8)(PC_VAUX_ENA | PC_VCC_ENA |
			PC_VAUX_OFF | PC_VCC_ON));
	}

	spin_unlock_irqrestore(&pAC->InitLock, InitFlags);

#ifdef CONFIG_PROC_FS
	if ((!pAC->InterfaceUp[pNet->NetNr]) && (pSkRootDir)) {
		pProcFile = create_proc_entry(pAC->dev[pNet->NetNr]->name, S_IRUGO, pSkRootDir);
		pProcFile->proc_fops = &sk_proc_fops;
		pProcFile->data      = dev;

		/*
		 * Remember, interface dev nr pNet->NetNr is up
		 */
		pAC->InterfaceUp[pNet->NetNr] = 1;

		strcpy(pNet->CurrentName, pNet->pAC->dev[pNet->NetNr]->name);
	}
#endif
#ifdef MV_SET_FW_IP_DRIVER
	/* Set the IP address */
	if (dev->ip_ptr != NULL) {
		pindev = (struct in_device*) dev->ip_ptr;
		ip_address = htonl(pindev->ifa_list->ifa_address);
		ip_netmask = htonl(pindev->ifa_list->ifa_mask);
		SetFwIpAddr(pAC, ip_address, ip_netmask, ip_address);
	}
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeOpen suceeded\n"));

	return (0);

	/* SkGeOpen error handling*/
open_failed:
	if (ModuleCount) {
		module_put(THIS_MODULE);
	}
	pAC->MaxPorts--;
	spin_unlock_irqrestore(&pAC->InitLock, InitFlags);
	return (-1);
} /* SkGeOpen */

/****************************************************************************
 *
 *	SkGeClose - Stop initialized adapter
 *
 * Description:
 *	Close initialized adapter.
 *
 * Returns:
 *	0 - on success
 *	error code - on error
 */
int SkGeClose(
struct SK_NET_DEVICE *dev)  /* the device that is to be closed */
{
	DEV_NET    	    *pNet = PPRIV;
	SK_AC           *pAC  = pNet->pAC;
	DEV_NET         *newPtrNet;
	unsigned long   Flags;		/* for the spin locks		*/
	int             CurrMac;	/* loop ctr for the current MAC	*/
	int             PortIdx;
#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_SET_FW_IP_DRIVER
	struct		in_device *pindev;
	SK_U32		ip_address;
	SK_U32		ip_netmask;
#endif
#endif
	int			RstMode;
#ifdef CONFIG_SK98LIN_NAPI
	int			WorkToDo = 1; /* min(*budget, dev->quota);    */
	int			WorkDone = 0;
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeClose: pAC=0x%lX ", (unsigned long)pAC));
	spin_lock_irqsave(&pAC->InitLock, Flags);
	if (pAC->MaxPorts == 1) {
		/* Disable interrupts */
		SK_OUT32(pAC->IoBase, B0_IMSK, 0);
	}
	NETIF_STOP_ALLQ(dev);
#ifdef SK_NEW_NAPI_HANDLING
	synchronize_irq(dev->irq);
	napi_synchronize(&pNet->napi);
#endif


#ifdef CONFIG_PROC_FS
	del_timer(&pNet->ProcfsTimer);
#endif

#ifdef Y2_RECOVERY
	pNet->InRecover = SK_TRUE;
	del_timer(&pNet->KernelTimer);
#endif

	if (pAC->DiagModeActive == DIAG_ACTIVE) {
		if (pAC->DiagFlowCtrl == SK_FALSE) {
			module_put(THIS_MODULE);
			/*
			** notify that the interface which has been closed
			** by operator interaction must not be started up
			** again when the DIAG has finished.
			*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			newPtrNet = (DEV_NET *) pAC->dev[0]->priv;
#else
			newPtrNet = netdev_priv(pAC->dev[0]);
#endif
			if (newPtrNet == pNet) {
				pAC->WasIfUp[0] = SK_FALSE;
			} else {
				pAC->WasIfUp[1] = SK_FALSE;
			}
			spin_unlock_irqrestore(&pAC->InitLock, Flags);
			return 0; /* return to system everything is fine... */
		} else {
			pAC->DiagFlowCtrl = SK_FALSE;
		}
	}

	PortIdx = pNet->PortNr;

#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_SET_FW_IP_DRIVER
	/* Set the IP address */
	if (pAC->FwRun && dev->ip_ptr != NULL) {
		pindev = (struct in_device*) dev->ip_ptr;
		ip_address = htonl(pindev->ifa_list->ifa_address);
		ip_netmask = htonl(pindev->ifa_list->ifa_mask);
		SetFwIpAddr(pAC, ip_address, ip_netmask, ip_address);
	}
#endif
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	if (pAC->FwRun && pAC->Suspended == SK_TRUE) {
		FwCheckLinkMode(pAC, pAC->IoBase, PortIdx);
	}
#ifdef MV_FW_SDK_LINK_MAINTENANCE_DEBUG
	printk("SkGeClose: pAC->Suspended:                   %d\n",
		pAC->Suspended);
	printk("SkGeClose: pAC->LinkInfo[0].LinkMaintenance: %d\n",
		pAC->LinkInfo[0].LinkMaintenance);
	printk("SkGeClose: pAC->GIni.GIDontInitPhy:          %d\n",
		pAC->GIni.GIDontInitPhy);
#endif
#endif
#endif

	if (pAC->MaxPorts == 1) {
		spin_lock(&pAC->SlowPathLock);

		/* Disable the status unit */
		SK_OUT8(pAC->IoBase, STAT_CTRL, SC_STAT_RST_SET);

		spin_unlock(&pAC->SlowPathLock);
		if (pAC->RequestedIrq == SK_TRUE) {
			pAC->RequestedIrq = SK_FALSE;
			SkGeReleaseIrq(pAC->IrqDev);
		}
		spin_lock(&pAC->SlowPathLock);

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
		if (pAC->Suspended == SK_FALSE) {

			/* Send event to stop the ports (RX/TX) */
			SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
				PortIdx, -1, SK_TRUE);
		}
		else {

			/* Stop queue and carrier, set interface to not running */
			NETIF_STOP_ALLQ(dev);
			netif_carrier_off(dev);
			dev->flags &= ~IFF_RUNNING;
		}
#else
		/* Send event to stop the ports (RX/TX) */
		SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
			PortIdx, -1, SK_TRUE);
#endif
		if (CHIP_ID_YUKON_2(pAC)) {
#ifdef MV_INCLUDE_SDK_SUPPORT
			if (pAC->FwRun &&
				pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR) {
				SK_OUT32(pAC->IoBase, MR_ADDR(PortIdx, RX_GMF_CTRL_T),
					RXMF_TCTL_MACSEC_FLSH_ENA);
			}
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
			RstMode = SK_SOFT_RST;
#endif
#else
			RstMode = SK_HARD_RST;
#endif
			SkY2PortStop(pAC,
					pAC->IoBase,
					PortIdx,
					SK_STOP_ALL,
					RstMode);
		} else {
			SkGeStopPort(pAC,
					pAC->IoBase,
					PortIdx,
					SK_STOP_ALL,
					SK_HARD_RST);
		}
		spin_unlock(&pAC->SlowPathLock);
	} else {
		spin_lock(&pAC->SlowPathLock);

		/* Send event to stop the ports (RX/TX) */
		SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
			PortIdx, -1, SK_TRUE);

		SkLocalEventQueue(pAC, SKGE_PNMI, SK_PNMI_EVT_XMAC_RESET,
				PortIdx, -1, SK_TRUE);
		spin_unlock(&pAC->SlowPathLock);

		/* Stop ports */
		spin_lock(&pAC->TxPort[PortIdx].TxDesRingLock);
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2PortStop(pAC, pAC->IoBase, PortIdx,
				SK_STOP_ALL, SK_HARD_RST);
		}
		else {
			SkGeStopPort(pAC, pAC->IoBase, PortIdx,
				SK_STOP_ALL, SK_HARD_RST);
		}
		spin_unlock(&pAC->TxPort[PortIdx].TxDesRingLock);
	}

	if (pAC->RlmtNets == 1) {
		/* Clear all descriptor rings */
		for (CurrMac=0; CurrMac<pAC->GIni.GIMacsFound; CurrMac++) {
			if (!CHIP_ID_YUKON_2(pAC)) {
#ifdef CONFIG_SK98LIN_NAPI
				WorkToDo = 1;
				ReceiveIrq(pAC,&pAC->RxPort[CurrMac],
						SK_TRUE,&WorkDone,WorkToDo);
#else
				ReceiveIrq(pAC,&pAC->RxPort[CurrMac],SK_TRUE);
#endif
				ClearRxRing(pAC, &pAC->RxPort[CurrMac]);
				ClearTxRing(pAC, &pAC->TxPort[CurrMac]);
			} else {
				SkY2FreeRxBuffers(pAC, pAC->IoBase, CurrMac);
				SkY2FreeTxBuffers(pAC, pAC->IoBase, CurrMac);
			}
		}
	} else {
		/* Clear port descriptor rings */
		if (!CHIP_ID_YUKON_2(pAC)) {
#ifdef CONFIG_SK98LIN_NAPI
			WorkToDo = 1;
			ReceiveIrq(pAC, &pAC->RxPort[PortIdx], SK_TRUE, &WorkDone, WorkToDo);
#else
			ReceiveIrq(pAC, &pAC->RxPort[PortIdx], SK_TRUE);
#endif
			ClearRxRing(pAC, &pAC->RxPort[PortIdx]);
			ClearTxRing(pAC, &pAC->TxPort[PortIdx]);
		}
		else {
			SkY2FreeRxBuffers(pAC, pAC->IoBase, PortIdx);
			SkY2FreeTxBuffers(pAC, pAC->IoBase, PortIdx);
		}
	}

	SK_OUT8(pAC->IoBase, B0_POWER_CTRL, (SK_U8)(PC_VAUX_ENA | PC_VCC_ENA |
		PC_VAUX_ON | PC_VCC_OFF));

#ifdef MV_INCLUDE_SDK_SUPPORT
	if ((pAC->FwRun) && (pAC->FwState) && (pAC->Suspended == SK_FALSE)) {
		FwDriverGoodbye(pAC, pAC->IoBase);
	}
#endif
#ifdef SK_AVB
	atu_node_table_cleanup();
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeClose: done "));

	SK_MEMSET(&(pAC->PnmiBackup), 0, sizeof(SK_PNMI_STRUCT_DATA));
	SK_MEMCPY(&(pAC->PnmiBackup), &(pAC->PnmiStruct),
			sizeof(SK_PNMI_STRUCT_DATA));

	pAC->MaxPorts--;
	module_put(THIS_MODULE);

#ifdef Y2_RECOVERY
	pNet->InRecover = SK_FALSE;
#endif
	spin_unlock_irqrestore(&pAC->InitLock, Flags);

	return (0);
} /* SkGeClose */


/*****************************************************************************
 *
 * 	SkGeXmit - Linux frame transmit function
 *
 * Description:
 *	The system calls this function to send frames onto the wire.
 *	It puts the frame in the tx descriptor ring. If the ring is
 *	full then, the 'tbusy' flag is set.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 * WARNING: returning 1 in 'tbusy' case caused system crashes (double
 *	allocated skb's) !!!
 */
static int SkGeXmit(struct sk_buff *skb, struct SK_NET_DEVICE *dev)
{
DEV_NET		*pNet;
SK_AC		*pAC;
int			Rc;	/* return code of XmitFrame */

	pNet = PPRIV;
	pAC = pNet->pAC;

	if (!skb_shinfo(skb)->nr_frags) {
		/* Don't activate scatter-gather and hardware checksum */

		if (pAC->RlmtNets == 2)
			Rc = XmitFrame(
				pAC,
				&pAC->TxPort[pNet->PortNr],
				skb);
		else
			Rc = XmitFrame(
				pAC,
				&pAC->TxPort[pAC->ActivePort],
				skb);
	} else {
		/* scatter-gather and hardware TCP checksumming anabled*/
		if (pAC->RlmtNets == 2)
			Rc = XmitFrameSG(
				pAC,
				&pAC->TxPort[pNet->PortNr],
				skb);
		else
			Rc = XmitFrameSG(
				pAC,
				&pAC->TxPort[pAC->ActivePort],
				skb);
	}

	/* Transmitter out of resources? */
	if (Rc <= 0) {
		NETIF_STOP_ALLQ(dev);
	}

	/* If not taken, give buffer ownership back to the
	 * queueing layer.
	 */

	if (Rc < 0) {
		return NETDEV_TX_BUSY;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
	dev->trans_start = jiffies;
#endif
	return NETDEV_TX_OK;

} /* SkGeXmit */

#ifdef CONFIG_SK98LIN_NAPI
/*****************************************************************************
 *
 * 	SkGePoll - NAPI Rx polling callback for Yukon chipsets
 *
 * Description:
 *	Called by the Linux system in case NAPI polling is activated
 *
 * Returns:
 *	The number of work data still to be handled
 */
#ifdef SK_NEW_NAPI_HANDLING
static int SkGePoll(struct napi_struct *napi, int WorkToDo) {
#else
static int SkGePoll(struct net_device *dev, int *budget) {
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
	DEV_NET		*pNet    = container_of(napi, DEV_NET, napi);
	SK_AC		*pAC     = pNet->pAC;
#elif defined SK_NEW_NAPI_HANDLING
	DEV_NET		*pNet    = container_of(napi, DEV_NET, napi);
	SK_AC		*pAC     = pNet->pAC;
	struct		SK_NET_DEVICE *dev = pAC->dev[0];
#else
	SK_AC		*pAC = ((DEV_NET*)dev->priv)->pAC;
	int		WorkToDo = min(*budget, dev->quota);
#endif

	int		WorkDone = 0;
	unsigned long	Flags;

	spin_lock(&pAC->TxPort[0].TxDesRingLock);
	FreeTxDescriptors(pAC, &pAC->TxPort[0]);
	spin_unlock(&pAC->TxPort[0].TxDesRingLock);

	ReceiveIrq(pAC, &pAC->RxPort[0], SK_TRUE, &WorkDone, WorkToDo);
	CLEAR_AND_START_RX(0);

#ifndef SK_NEW_NAPI_HANDLING
	*budget -= WorkDone;
	dev->quota -= WorkDone;
#endif

	if(WorkDone < WorkToDo) {
		spin_lock_irqsave(&pAC->SlowPathLock, Flags);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28)
		__napi_complete(napi);
#elif defined SK_NEW_NAPI_HANDLING
		__netif_rx_complete(dev, napi);
#else
		netif_rx_complete(dev);
#endif
		pAC->GIni.GIValIrqMask |= (NAPI_DRV_IRQS);
#ifndef USE_TX_COMPLETE
		pAC->GIni.GIValIrqMask &= ~(TX_COMPL_IRQS);
#endif
		/* enable interrupts again */
		SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);
		spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
	}
	return (WorkDone >= WorkToDo);
} /* SkGePoll */
#endif

#ifdef SK_POLL_CONTROLLER
/*****************************************************************************
 *
 * 	SkGeNetPoll - Polling "interrupt"
 *
 * Description:
 *	Polling 'interrupt' - used by things like netconsole and netdump
 *	to send skbs without having to re-enable interrupts.
 *	It's not called while the interrupt routine is executing.
 */
static void SkGeNetPoll(
struct SK_NET_DEVICE *dev)
{
DEV_NET		*pNet;
SK_AC		*pAC;

	pNet = PPRIV;
	pAC = pNet->pAC;
	pNet->NetConsoleMode = SK_TRUE;

	/*  Prevent any reconfiguration while handling
	    the 'interrupt' */
	SK_OUT32(pAC->IoBase, B0_IMSK, 0);

	if (!CHIP_ID_YUKON_2(pAC)) {
	/* Handle the Yukon Isr */
#ifdef SK_NEW_ISR_DEFINITION
		SkGeIsr(dev->irq, dev);
#else
		SkGeIsr(dev->irq, dev, NULL);
#endif
	} else {
		/* Handle the Yukon2 Isr */
#ifdef SK_NEW_ISR_DEFINITION
		SkY2Isr(dev->irq, dev);
#else
		SkY2Isr(dev->irq, dev, NULL);
#endif
	}

}
#endif


/*****************************************************************************
 *
 * 	XmitFrame - fill one socket buffer into the transmit ring
 *
 * Description:
 *	This function puts a message into the transmit descriptor ring
 *	if there is a descriptors left.
 *	Linux skb's consist of only one continuous buffer.
 *	The first step locks the ring. It is held locked
 *	all time to avoid problems with SWITCH_../PORT_RESET.
 *	Then the descriptoris allocated.
 *	The second part is linking the buffer to the descriptor.
 *	At the very last, the Control field of the descriptor
 *	is made valid for the BMU and a start TX command is given
 *	if necessary.
 *
 * Returns:
 *	> 0 - on succes: the number of bytes in the message
 *	= 0 - on resource shortage: this frame sent or dropped, now
 *		the ring is full ( -> set tbusy)
 *	< 0 - on failure: other problems ( -> return failure to upper layers)
 */
static int XmitFrame(
SK_AC 		*pAC,		/* pointer to adapter context	        */
TX_PORT		*pTxPort,	/* pointer to struct of port to send to */
struct sk_buff	*pMessage)	/* pointer to send-message              */
{
	TXD		*pTxd;		/* the rxd to fill */
	TXD		*pOldTxd;
	unsigned long	 Flags;
	SK_U64		 PhysAddr;
	int	 	 Protocol;
	int		 IpHeaderLength;
	int		 BytesSend = pMessage->len;

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_TX_PROGRESS, ("X"));

	spin_lock_irqsave(&pTxPort->TxDesRingLock, Flags);
#ifndef USE_TX_COMPLETE
	if ((pTxPort->TxdRingPrevFree - pTxPort->TxdRingFree) > 6)  {
		FreeTxDescriptors(pAC, pTxPort);
		pTxPort->TxdRingPrevFree = pTxPort->TxdRingFree;
	}
#endif
	if (pTxPort->TxdRingFree == 0) {
		/*
		** not enough free descriptors in ring at the moment.
		** Maybe free'ing some old one help?
		*/
		FreeTxDescriptors(pAC, pTxPort);
		if (pTxPort->TxdRingFree == 0) {
			spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);
			SK_PNMI_CNT_NO_TX_BUF(pAC, pTxPort->PortIndex);
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_TX_PROGRESS,
				("XmitFrame failed\n"));
			/*
			** the desired message can not be sent
			** Because tbusy seems to be set, the message
			** should not be freed here. It will be used
			** by the scheduler of the ethernet handler
			*/
				return (-1);
		}
	}

	/*
	** If the passed socket buffer is of smaller MTU-size than 60,
	** copy everything into new buffer and fill all bytes between
	** the original packet end and the new packet end of 60 with 0x00.
	** This is to resolve faulty padding by the HW with 0xaa bytes.
	*/
#ifndef SK98LIN_VMESX4
	if (BytesSend < C_LEN_ETHERNET_MINSIZE) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
		if ((pMessage = skb_padto(pMessage, C_LEN_ETHERNET_MINSIZE)) == NULL) {
#else
		if (skb_padto(pMessage, C_LEN_ETHERNET_MINSIZE)) {
#endif
			spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);
			return (0);
		}
		pMessage->len = C_LEN_ETHERNET_MINSIZE;
	}
#endif

	/*
	** advance head counter behind descriptor needed for this frame,
	** so that needed descriptor is reserved from that on. The next
	** action will be to add the passed buffer to the TX-descriptor
	*/
	pTxd = pTxPort->pTxdRingHead;
	pTxPort->pTxdRingHead = pTxd->pNextTxd;
	pTxPort->TxdRingFree--;

#ifdef SK_DUMP_TX
	DumpMsg(pMessage, "XmitFrame");
#endif

	/*
	** First step is to map the data to be sent via the adapter onto
	** the DMA memory. Kernel 2.2 uses virt_to_bus(), but kernels 2.4
	** and 2.6 need to use pci_map_page() for that mapping.
	*/
	PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
					virt_to_page(pMessage->data),
					((unsigned long) pMessage->data & ~PAGE_MASK),
					pMessage->len,
					PCI_DMA_TODEVICE);
	pTxd->VDataLow  = (SK_U32) (PhysAddr & 0xffffffff);
	pTxd->VDataHigh = (SK_U32) (PhysAddr >> 32);
	pTxd->pMBuf     = pMessage;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	if (pMessage->ip_summed == CHECKSUM_PARTIAL) {
#else
	if (pMessage->ip_summed == CHECKSUM_HW) {
#endif
		Protocol = ((SK_U8)pMessage->data[C_OFFSET_IPPROTO] & 0xff);
		if ((Protocol == C_PROTO_ID_UDP) &&
			(pAC->GIni.GIChipRev == 0) &&
			(pAC->GIni.GIChipId == CHIP_ID_YUKON)) {
			pTxd->TBControl = BMU_TCP_CHECK;
		} else {
			pTxd->TBControl = BMU_UDP_CHECK;
		}

		IpHeaderLength  = (SK_U8)pMessage->data[C_OFFSET_IPHEADER];
		IpHeaderLength  = (IpHeaderLength & 0xf) * 4;
		pTxd->TcpSumOfs = 0; /* PH-Checksum already calculated */
		pTxd->TcpSumWr  = C_LEN_ETHERMAC_HEADER + IpHeaderLength +
							(Protocol == C_PROTO_ID_UDP ?
							C_OFFSET_UDPHEADER_UDPCS :
							C_OFFSET_TCPHEADER_TCPCS);
		pTxd->TcpSumSt  = C_LEN_ETHERMAC_HEADER + IpHeaderLength;

		pTxd->TBControl |= BMU_OWN | BMU_STF |
				   BMU_SW  | BMU_EOF |
#ifdef USE_TX_COMPLETE
				   BMU_IRQ_EOF |
#endif
				   pMessage->len;
	} else {
		pTxd->TBControl = BMU_OWN | BMU_STF | BMU_CHECK |
				  BMU_SW  | BMU_EOF |
#ifdef USE_TX_COMPLETE
				   BMU_IRQ_EOF |
#endif
			pMessage->len;
	}

	/*
	** If previous descriptor already done, give TX start cmd
	*/
	pOldTxd = xchg(&pTxPort->pTxdRingPrev, pTxd);
	if ((pOldTxd->TBControl & BMU_OWN) == 0) {
		SK_OUT8(pTxPort->HwAddr, Q_CSR, CSR_START);
	}

	/*
	** after releasing the lock, the skb may immediately be free'd
	*/
	spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);
	if (pTxPort->TxdRingFree != 0) {
		return (BytesSend);
	} else {
		return (0);
	}

} /* XmitFrame */

/*****************************************************************************
 *
 * 	XmitFrameSG - fill one socket buffer into the transmit ring
 *                (use SG and TCP/UDP hardware checksumming)
 *
 * Description:
 *	This function puts a message into the transmit descriptor ring
 *	if there is a descriptors left.
 *
 * Returns:
 *	> 0 - on succes: the number of bytes in the message
 *	= 0 - on resource shortage: this frame sent or dropped, now
 *		the ring is full ( -> set tbusy)
 *	< 0 - on failure: other problems ( -> return failure to upper layers)
 */
static int XmitFrameSG(
SK_AC 		*pAC,		/* pointer to adapter context           */
TX_PORT		*pTxPort,	/* pointer to struct of port to send to */
struct sk_buff	*pMessage)	/* pointer to send-message              */
{

	TXD		*pTxd;
	TXD		*pTxdFst;
	TXD		*pTxdLst;
	int 	 	 CurrFrag;
	int		 BytesSend;
	int		 IpHeaderLength;
	int		 Protocol;
	int		 FragSize;
	SK_U64		 PhysAddr;
	unsigned long	 Flags;

	spin_lock_irqsave(&pTxPort->TxDesRingLock, Flags);
#ifndef USE_TX_COMPLETE
	FreeTxDescriptors(pAC, pTxPort);
#endif
	if ((skb_shinfo(pMessage)->nr_frags +1) > pTxPort->TxdRingFree) {
		FreeTxDescriptors(pAC, pTxPort);
		if ((skb_shinfo(pMessage)->nr_frags + 1) > pTxPort->TxdRingFree) {
			spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);
			SK_PNMI_CNT_NO_TX_BUF(pAC, pTxPort->PortIndex);
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_TX_PROGRESS,
				("XmitFrameSG failed - Ring full\n"));
				/* this message can not be sent now */
				return(-1);
		}
	}

	pTxd      = pTxPort->pTxdRingHead;
	pTxdFst   = pTxd;
	pTxdLst   = pTxd;
	BytesSend = 0;
	Protocol  = 0;

	/*
	** Map the first fragment (header) into the DMA-space
	*/
	PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
			virt_to_page(pMessage->data),
			((unsigned long) pMessage->data & ~PAGE_MASK),
			skb_headlen(pMessage),
			PCI_DMA_TODEVICE);

	pTxd->VDataLow  = (SK_U32) (PhysAddr & 0xffffffff);
	pTxd->VDataHigh = (SK_U32) (PhysAddr >> 32);

	/*
	** Does the HW need to evaluate checksum for TCP or UDP packets?
	*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	if (pMessage->ip_summed == CHECKSUM_PARTIAL) {
#else
	if (pMessage->ip_summed == CHECKSUM_HW) {
#endif
		pTxd->TBControl = BMU_STF | BMU_STFWD | skb_headlen(pMessage);
		/*
		** We have to use the opcode for tcp here,  because the
		** opcode for udp is not working in the hardware yet
		** (Revision 2.0)
		*/
		Protocol = ((SK_U8)pMessage->data[C_OFFSET_IPPROTO] & 0xff);
		if ((Protocol == C_PROTO_ID_UDP) &&
			(pAC->GIni.GIChipRev == 0) &&
			(pAC->GIni.GIChipId == CHIP_ID_YUKON)) {
			pTxd->TBControl |= BMU_TCP_CHECK;
		} else {
			pTxd->TBControl |= BMU_UDP_CHECK;
		}

		IpHeaderLength  = ((SK_U8)pMessage->data[C_OFFSET_IPHEADER] & 0xf)*4;
		pTxd->TcpSumOfs = 0; /* PH-Checksum already claculated */
		pTxd->TcpSumWr  = C_LEN_ETHERMAC_HEADER + IpHeaderLength +
							(Protocol == C_PROTO_ID_UDP ?
							C_OFFSET_UDPHEADER_UDPCS :
							C_OFFSET_TCPHEADER_TCPCS);
		pTxd->TcpSumSt  = C_LEN_ETHERMAC_HEADER + IpHeaderLength;
	} else {
		pTxd->TBControl = BMU_CHECK | BMU_SW | BMU_STF |
					skb_headlen(pMessage);
	}

	pTxd = pTxd->pNextTxd;
	pTxPort->TxdRingFree--;
	BytesSend += skb_headlen(pMessage);

	/*
	** Browse over all SG fragments and map each of them into the DMA space
	*/
	for (CurrFrag = 0; CurrFrag < skb_shinfo(pMessage)->nr_frags; CurrFrag++) {

		/*
		** we already have the proper value in entry
		*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
		const skb_frag_t *sk_frag = &skb_shinfo(pMessage)->frags[CurrFrag];
		FragSize = skb_frag_size(sk_frag);
		PhysAddr = (SK_U64) skb_frag_dma_map(&pAC->PciDev->dev,
						sk_frag,
						0,
						FragSize,
						DMA_TO_DEVICE);
#else
		skb_frag_t *sk_frag = &skb_shinfo(pMessage)->frags[CurrFrag];
		FragSize = sk_frag->size;
		PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
						 sk_frag->page,
						 sk_frag->page_offset,
						 FragSize,
						 PCI_DMA_TODEVICE);
#endif
		pTxd->VDataLow  = (SK_U32) (PhysAddr & 0xffffffff);
		pTxd->VDataHigh = (SK_U32) (PhysAddr >> 32);
		pTxd->pMBuf     = pMessage;

		/*
		** Does the HW need to evaluate checksum for TCP or UDP packets?
		*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
		if (pMessage->ip_summed == CHECKSUM_PARTIAL) {
#else
		if (pMessage->ip_summed == CHECKSUM_HW) {
#endif
			pTxd->TBControl = BMU_OWN | BMU_SW | BMU_STFWD;
			/*
			** We have to use the opcode for tcp here because the
			** opcode for udp is not working in the hardware yet
			** (revision 2.0)
			*/
			if ((Protocol == C_PROTO_ID_UDP) &&
				(pAC->GIni.GIChipRev == 0) &&
				(pAC->GIni.GIChipId == CHIP_ID_YUKON)) {
				pTxd->TBControl |= BMU_TCP_CHECK;
			} else {
				pTxd->TBControl |= BMU_UDP_CHECK;
			}
		} else {
			pTxd->TBControl = BMU_CHECK | BMU_SW | BMU_OWN;
		}

		/*
		** Do we have the last fragment?
		*/
		if( (CurrFrag+1) == skb_shinfo(pMessage)->nr_frags )  {
#ifdef USE_TX_COMPLETE
			pTxd->TBControl |= BMU_EOF | BMU_IRQ_EOF | FragSize;
#else
			pTxd->TBControl |= BMU_EOF | FragSize;
#endif
			pTxdFst->TBControl |= BMU_OWN | BMU_SW;

		} else {
			pTxd->TBControl |= FragSize;
		}
		pTxdLst = pTxd;
		pTxd    = pTxd->pNextTxd;
		pTxPort->TxdRingFree--;
		BytesSend += FragSize;
	}

	/*
	** If previous descriptor already done, give TX start cmd
	*/
	if ((pTxPort->pTxdRingPrev->TBControl & BMU_OWN) == 0) {
		SK_OUT8(pTxPort->HwAddr, Q_CSR, CSR_START);
	}

	pTxPort->pTxdRingPrev = pTxdLst;
	pTxPort->pTxdRingHead = pTxd;

	spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);

	if (pTxPort->TxdRingFree > 0) {
		return (BytesSend);
	} else {
		return (0);
	}
}

/*****************************************************************************
 *
 * 	FreeTxDescriptors - release descriptors from the descriptor ring
 *
 * Description:
 *	This function releases descriptors from a transmit ring if they
 *	have been sent by the BMU.
 *	If a descriptors is sent, it can be freed and the message can
 *	be freed, too.
 *	The SOFTWARE controllable bit is used to prevent running around a
 *	completely free ring for ever. If this bit is no set in the
 *	frame (by XmitFrame), this frame has never been sent or is
 *	already freed.
 *	The Tx descriptor ring lock must be held while calling this function !!!
 *
 * Returns:
 *	none
 */
static void FreeTxDescriptors(
SK_AC	*pAC,		/* pointer to the adapter context */
TX_PORT	*pTxPort)	/* pointer to destination port structure */
{
TXD	*pTxd;		/* pointer to the checked descriptor */
TXD	*pNewTail;	/* pointer to 'end' of the ring */
SK_U32	Control;	/* TBControl field of descriptor */
SK_U64	PhysAddr;	/* address of DMA mapping */

	pNewTail = pTxPort->pTxdRingTail;
	pTxd     = pNewTail;
	/*
	** loop forever; exits if BMU_SW bit not set in start frame
	** or BMU_OWN bit set in any frame
	*/
	while (1) {
		Control = pTxd->TBControl;
		if ((Control & BMU_SW) == 0) {
			/*
			** software controllable bit is set in first
			** fragment when given to BMU. Not set means that
			** this fragment was never sent or is already
			** freed ( -> ring completely free now).
			*/
			pTxPort->pTxdRingTail = pTxd;
			NETIF_WAKE_ALLQ(pAC->dev[pTxPort->PortIndex]);
			return;
		}
		if (Control & BMU_OWN) {
			pTxPort->pTxdRingTail = pTxd;
			if (pTxPort->TxdRingFree > 0) {
				NETIF_WAKE_ALLQ(pAC->dev[pTxPort->PortIndex]);
			}
			return;
		}

		/*
		** release the DMA mapping, because until not unmapped
		** this buffer is considered being under control of the
		** adapter card!
		*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
		if (pTxd->pMBuf != NULL) {
#endif
		/* Release the page and free the message */
			PhysAddr = ((SK_U64) pTxd->VDataHigh) << (SK_U64) 32;
			PhysAddr |= (SK_U64) pTxd->VDataLow;
			pci_unmap_page(pAC->PciDev, PhysAddr,
				 pTxd->pMBuf->len,
				 PCI_DMA_TODEVICE);

			if (Control & BMU_EOF)
				DEV_KFREE_SKB_ANY(pTxd->pMBuf);	/* free message */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,26)
		}
#endif

		pTxPort->TxdRingFree++;
		pTxd->TBControl &= ~BMU_SW;
		pTxd = pTxd->pNextTxd; /* point behind fragment with EOF */
	} /* while(forever) */
} /* FreeTxDescriptors */

/*****************************************************************************
 *
 * 	FillRxRing - fill the receive ring with valid descriptors
 *
 * Description:
 *	This function fills the receive ring descriptors with data
 *	segments and makes them valid for the BMU.
 *	The active ring is filled completely, if possible.
 *	The non-active ring is filled only partial to save memory.
 *
 * Description of rx ring structure:
 *	head - points to the descriptor which will be used next by the BMU
 *	tail - points to the next descriptor to give to the BMU
 *
 * Returns:	N/A
 */
static void FillRxRing(
SK_AC		*pAC,		/* pointer to the adapter context */
RX_PORT		*pRxPort)	/* ptr to port struct for which the ring
				   should be filled */
{
unsigned long	Flags;

	spin_lock_irqsave(&pRxPort->RxDesRingLock, Flags);
	while (pRxPort->RxdRingFree > pRxPort->RxFillLimit) {
		if(!FillRxDescriptor(pAC, pRxPort))
			break;
	}
	spin_unlock_irqrestore(&pRxPort->RxDesRingLock, Flags);
} /* FillRxRing */


/*****************************************************************************
 *
 * 	FillRxDescriptor - fill one buffer into the receive ring
 *
 * Description:
 *	The function allocates a new receive buffer and
 *	puts it into the next descriptor.
 *
 * Returns:
 *	SK_TRUE - a buffer was added to the ring
 *	SK_FALSE - a buffer could not be added
 */
static SK_BOOL FillRxDescriptor(
SK_AC		*pAC,		/* pointer to the adapter context struct */
RX_PORT		*pRxPort)	/* ptr to port struct of ring to fill */
{
struct sk_buff	*pMsgBlock;	/* pointer to a new message block */
RXD		*pRxd;		/* the rxd to fill */
SK_U16		Length;		/* data fragment length */
SK_U64		PhysAddr;	/* physical address of a rx buffer */

	pMsgBlock = alloc_skb(pRxPort->RxBufSize, GFP_ATOMIC);
	if (pMsgBlock == NULL) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
			SK_DBGCAT_DRV_ENTRY,
			("%s: Allocation of rx buffer failed !\n",
			pAC->dev[pRxPort->PortIndex]->name));
		SK_PNMI_CNT_NO_RX_BUF(pAC, pRxPort->PortIndex);
		return(SK_FALSE);
	}
	skb_reserve(pMsgBlock, 2); /* to align IP frames */
	/* skb allocated ok, so add buffer */
	pRxd = pRxPort->pRxdRingTail;
	pRxPort->pRxdRingTail = pRxd->pNextRxd;
	pRxPort->RxdRingFree--;
	Length = pRxPort->RxBufSize;
	PhysAddr = (SK_U64) pci_map_page(pAC->PciDev,
		virt_to_page(pMsgBlock->data),
		((unsigned long) pMsgBlock->data &
		~PAGE_MASK),
		pRxPort->RxBufSize - 2,
		PCI_DMA_FROMDEVICE);

	pRxd->VDataLow  = (SK_U32) (PhysAddr & 0xffffffff);
	pRxd->VDataHigh = (SK_U32) (PhysAddr >> 32);
	pRxd->pMBuf     = pMsgBlock;
	pRxd->RBControl = BMU_OWN       |
			  BMU_STF       |
			  BMU_IRQ_EOF   |
			  BMU_TCP_CHECK |
			  Length;
	return (SK_TRUE);

} /* FillRxDescriptor */


/*****************************************************************************
 *
 * 	ReQueueRxBuffer - fill one buffer back into the receive ring
 *
 * Description:
 *	Fill a given buffer back into the rx ring. The buffer
 *	has been previously allocated and aligned, and its phys.
 *	address calculated, so this is no more necessary.
 *
 * Returns: N/A
 */
static void ReQueueRxBuffer(
SK_AC		*pAC,		/* pointer to the adapter context struct */
RX_PORT		*pRxPort,	/* ptr to port struct of ring to fill */
struct sk_buff	*pMsg,		/* pointer to the buffer */
SK_U32		PhysHigh,	/* phys address high dword */
SK_U32		PhysLow)	/* phys address low dword */
{
RXD		*pRxd;		/* the rxd to fill */
SK_U16		Length;		/* data fragment length */

	pRxd = pRxPort->pRxdRingTail;
	pRxPort->pRxdRingTail = pRxd->pNextRxd;
	pRxPort->RxdRingFree--;
	Length = pRxPort->RxBufSize;

	pRxd->VDataLow  = PhysLow;
	pRxd->VDataHigh = PhysHigh;
	pRxd->pMBuf     = pMsg;
	pRxd->RBControl = BMU_OWN       |
			  BMU_STF       |
			  BMU_IRQ_EOF   |
			  BMU_TCP_CHECK |
			  Length;
	return;
} /* ReQueueRxBuffer */

/*****************************************************************************
 *
 * 	ReceiveIrq - handle a receive IRQ
 *
 * Description:
 *	This function is called when a receive IRQ is set.
 *	It walks the receive descriptor ring and sends up all
 *	frames that are complete.
 *
 * Returns:	N/A
 */
static void ReceiveIrq(
#ifdef CONFIG_SK98LIN_NAPI
SK_AC    *pAC,          /* pointer to adapter context          */
RX_PORT  *pRxPort,      /* pointer to receive port struct      */
SK_BOOL   SlowPathLock, /* indicates if SlowPathLock is needed */
int      *WorkDone,
int       WorkToDo)
#else
SK_AC    *pAC,          /* pointer to adapter context          */
RX_PORT  *pRxPort,      /* pointer to receive port struct      */
SK_BOOL   SlowPathLock) /* indicates if SlowPathLock is needed */
#endif
{
	RXD             *pRxd;          /* pointer to receive descriptors         */
	struct sk_buff  *pMsg;          /* pointer to message holding frame       */
	struct sk_buff  *pNewMsg;       /* pointer to new message for frame copy  */
	SK_U32           Control;       /* control field of descriptor     */
	int              PortIndex = pRxPort->PortIndex;
	int              FrameLength;   /* total length of received frame  */
	int              IpFrameLength; /* IP length of the received frame */
	SK_BOOL          IsBadFrame;    /* the frame received is bad!      */
	SK_U32           FrameStat;
	unsigned short   Csum1;
	unsigned short   Csum2;
	unsigned short   Type;
	int              Result;
	SK_U64           PhysAddr;

rx_start:
	/* do forever; exit if BMU_OWN found */
	for ( pRxd = pRxPort->pRxdRingHead ;
		  pRxPort->RxdRingFree < pAC->RxDescrPerRing ;
		  pRxd = pRxd->pNextRxd,
		  pRxPort->pRxdRingHead = pRxd,
		  pRxPort->RxdRingFree ++) {

		/*
		 * For a better understanding of this loop
		 * Go through every descriptor beginning at the head
		 * Please note: the ring might be completely received so the OWN bit
		 * set is not a good crirteria to leave that loop.
		 * Therefore the RingFree counter is used.
		 * On entry of this loop pRxd is a pointer to the Rxd that needs
		 * to be checked next.
		 */

		Control = pRxd->RBControl;

#ifdef CONFIG_SK98LIN_NAPI
		if (*WorkDone >= WorkToDo) {
			break;
		}
		(*WorkDone)++;
#endif

		/* check if this descriptor is ready */
		if ((Control & BMU_OWN) != 0) {
			/* this descriptor is not yet ready */
			/* This is the usual end of the loop */
			/* We don't need to start the ring again */
			FillRxRing(pAC, pRxPort);
			return;
		}

		/* get length of frame and check it */
		FrameLength = Control & BMU_BBC;
		if (FrameLength > pRxPort->RxBufSize) {
			goto rx_failed;
		}

		/* check for STF and EOF */
		if ((Control & (BMU_STF | BMU_EOF)) != (BMU_STF | BMU_EOF)) {
			goto rx_failed;
		}

		/* here we have a complete frame in the ring */
		pMsg = pRxd->pMBuf;

		FrameStat = pRxd->FrameStat;

		/* check for frame length mismatch */
#define XMR_FS_LEN_SHIFT	18
#define GMR_FS_LEN_SHIFT	16
		if (FrameLength != (SK_U32) (FrameStat >> GMR_FS_LEN_SHIFT)) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS,
				("skge: Frame length mismatch (%u/%u).\n",
				FrameLength,
				(SK_U32) (FrameStat >> XMR_FS_LEN_SHIFT)));
			goto rx_failed;
		}

		/* Set Rx Status */
		IsBadFrame = (((FrameStat & GMR_FS_ANY_ERR) != 0) ||
					((FrameStat & GMR_FS_RX_OK) == 0));

		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, 0,
			("Received frame of length %d on port %d\n",
			FrameLength, PortIndex));
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, 0,
			("Number of free rx descriptors: %d\n",
			pRxPort->RxdRingFree));
		/* DumpMsg(pMsg, "Rx");	*/

		if ((Control & BMU_STAT_VAL) != BMU_STAT_VAL || (IsBadFrame)) {
			/* there is a receive error in this frame */
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS,
				("skge: Error in received frame, dropped!\n"
				"Control: %x\nRxStat: %x\n",
				Control, FrameStat));

			PhysAddr = ((SK_U64) pRxd->VDataHigh) << (SK_U64)32;
			PhysAddr |= (SK_U64) pRxd->VDataLow;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
			pci_dma_sync_single(pAC->PciDev,
						(dma_addr_t) PhysAddr,
						FrameLength,
						PCI_DMA_FROMDEVICE);
#else
			pci_dma_sync_single_for_cpu(pAC->PciDev,
						(dma_addr_t) PhysAddr,
						FrameLength,
						PCI_DMA_FROMDEVICE);
#endif
			ReQueueRxBuffer(pAC, pRxPort, pMsg,
				pRxd->VDataHigh, pRxd->VDataLow);

			continue;
		}

		/*
		 * if short frame then copy data to reduce memory waste
		 */
		if ((FrameLength < SK_COPY_THRESHOLD) &&
			((pNewMsg = alloc_skb(FrameLength+2, GFP_ATOMIC)) != NULL)) {
			/*
			 * Short frame detected and allocation successfull
			 */
			/* use new skb and copy data */
			skb_reserve(pNewMsg, 2);
			skb_put(pNewMsg, FrameLength);
			PhysAddr = ((SK_U64) pRxd->VDataHigh) << (SK_U64)32;
			PhysAddr |= (SK_U64) pRxd->VDataLow;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
			pci_dma_sync_single(pAC->PciDev,
						(dma_addr_t) PhysAddr,
						FrameLength,
						PCI_DMA_FROMDEVICE);
#else
			pci_dma_sync_single_for_device(pAC->PciDev,
						(dma_addr_t) PhysAddr,
						FrameLength,
						PCI_DMA_FROMDEVICE);
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
			skb_copy_to_linear_data(pNewMsg, pMsg->data,
				FrameLength);
#else
			eth_copy_and_sum(pNewMsg, pMsg->data,
				FrameLength, 0);
#endif
			ReQueueRxBuffer(pAC, pRxPort, pMsg,
				pRxd->VDataHigh, pRxd->VDataLow);

			pMsg = pNewMsg;

		} else {
			/*
			 * if large frame, or SKB allocation failed, pass
			 * the SKB directly to the networking
			 */
			PhysAddr = ((SK_U64) pRxd->VDataHigh) << (SK_U64)32;
			PhysAddr |= (SK_U64) pRxd->VDataLow;

			/* release the DMA mapping */
			pci_unmap_single(pAC->PciDev,
					 PhysAddr,
					 pRxPort->RxBufSize - 2,
					 PCI_DMA_FROMDEVICE);
			skb_put(pMsg, FrameLength); /* set message len */
			pMsg->ip_summed = CHECKSUM_NONE; /* initial default */

			if (pRxPort->UseRxCsum) {
				Type = ntohs(*((short*)&pMsg->data[12]));
				if (Type == 0x800) {
					IpFrameLength = (int) ntohs((unsigned short)
							((unsigned short *) pMsg->data)[8]);
					if ((FrameLength - IpFrameLength) == 0xe) {
						Csum1=le16_to_cpu(pRxd->TcpSums & 0xffff);
						Csum2=le16_to_cpu((pRxd->TcpSums >> 16) & 0xffff);
						if (pAC->ChipsetType) {
							Result = SkCsGetReceiveInfo(pAC, &pMsg->data[14],
								Csum1, Csum2, PortIndex, IpFrameLength);
							if ((Result == SKCS_STATUS_IP_FRAGMENT) ||
							    (Result == SKCS_STATUS_IP_CSUM_OK)  ||
							    (Result == SKCS_STATUS_TCP_CSUM_OK) ||
							    (Result == SKCS_STATUS_UDP_CSUM_OK)) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
								pMsg->ip_summed = CHECKSUM_COMPLETE;
								pMsg->csum = Csum1 & 0xffff;
#else
								pMsg->ip_summed = CHECKSUM_UNNECESSARY;
#endif
							} else if ((Result == SKCS_STATUS_TCP_CSUM_ERROR)    ||
							           (Result == SKCS_STATUS_UDP_CSUM_ERROR)    ||
							           (Result == SKCS_STATUS_IP_CSUM_ERROR_UDP) ||
							           (Result == SKCS_STATUS_IP_CSUM_ERROR_TCP) ||
							           (Result == SKCS_STATUS_IP_CSUM_ERROR)) {
								/* HW Checksum error */
								SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
								SK_DBGCAT_DRV_RX_PROGRESS,
								("skge: CRC error. Frame dropped!\n"));
								goto rx_failed;
							} else {
								pMsg->ip_summed = CHECKSUM_NONE;
							}
						}/* checksumControl calculation valid */
					} /* Frame length check */
				} /* IP frame */
			} /* pRxPort->UseRxCsum */
		} /* frame > SK_COPY_TRESHOLD */

		/* Send up only frames from active port */
		if ((PortIndex == pAC->ActivePort)||(pAC->RlmtNets == 2)) {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, 1,("U"));
#ifdef xDEBUG
			DumpMsg(pMsg, "Rx");
#endif
			SK_PNMI_CNT_RX_OCTETS_DELIVERED(pAC,FrameLength,PortIndex);
			pMsg->dev = pAC->dev[PortIndex];
			pMsg->protocol = eth_type_trans(pMsg,pAC->dev[PortIndex]);

			netif_rx(pMsg); /* frame for upper layer */
			pAC->dev[PortIndex]->last_rx = jiffies;
		} else {
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV,
				SK_DBGCAT_DRV_RX_PROGRESS,("D"));
			DEV_KFREE_SKB(pMsg); /* drop frame */
		}
	} /* for ... scanning the RXD ring */

	/* RXD ring is empty -> fill and restart */
	FillRxRing(pAC, pRxPort);
	return;

rx_failed:
	/* remove error frame */
	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ERROR,
		("Schrottdescriptor, length: 0x%x\n", FrameLength));

	/* release the DMA mapping */

	PhysAddr = ((SK_U64) pRxd->VDataHigh) << (SK_U64)32;
	PhysAddr |= (SK_U64) pRxd->VDataLow;
	pci_unmap_page(pAC->PciDev,
			 PhysAddr,
			 pRxPort->RxBufSize - 2,
			 PCI_DMA_FROMDEVICE);
	DEV_KFREE_SKB_IRQ(pRxd->pMBuf);
	pRxd->pMBuf = NULL;
	pRxPort->RxdRingFree++;
	pRxPort->pRxdRingHead = pRxd->pNextRxd;
	goto rx_start;

} /* ReceiveIrq */

/*****************************************************************************
 *
 * 	ClearRxRing - remove all buffers from the receive ring
 *
 * Description:
 *	This function removes all receive buffers from the ring.
 *	The receive BMU must be stopped before calling this function.
 *
 * Returns: N/A
 */
static void ClearRxRing(
SK_AC	*pAC,		/* pointer to adapter context */
RX_PORT	*pRxPort)	/* pointer to rx port struct */
{
RXD		*pRxd;	/* pointer to the current descriptor */
unsigned long	Flags;
SK_U64		PhysAddr;

	if (pRxPort->RxdRingFree == pAC->RxDescrPerRing) {
		return;
	}
	spin_lock_irqsave(&pRxPort->RxDesRingLock, Flags);
	pRxd = pRxPort->pRxdRingHead;
	do {
		if (pRxd->pMBuf != NULL) {

			PhysAddr = ((SK_U64) pRxd->VDataHigh) << (SK_U64)32;
			PhysAddr |= (SK_U64) pRxd->VDataLow;
			pci_unmap_page(pAC->PciDev,
					 PhysAddr,
					 pRxPort->RxBufSize - 2,
					 PCI_DMA_FROMDEVICE);
			DEV_KFREE_SKB(pRxd->pMBuf);
			pRxd->pMBuf = NULL;
		}
		pRxd->RBControl &= BMU_OWN;
		pRxd = pRxd->pNextRxd;
		pRxPort->RxdRingFree++;
	} while (pRxd != pRxPort->pRxdRingTail);
	pRxPort->pRxdRingTail = pRxPort->pRxdRingHead;
	spin_unlock_irqrestore(&pRxPort->RxDesRingLock, Flags);
} /* ClearRxRing */

/*****************************************************************************
 *
 *	ClearTxRing - remove all buffers from the transmit ring
 *
 * Description:
 *	This function removes all transmit buffers from the ring.
 *	The transmit BMU must be stopped before calling this function
 *	and transmitting at the upper level must be disabled.
 *	The BMU own bit of all descriptors is cleared, the rest is
 *	done by calling FreeTxDescriptors.
 *
 * Returns: N/A
 */
static void ClearTxRing(
SK_AC	*pAC,		/* pointer to adapter context */
TX_PORT	*pTxPort)	/* pointer to tx prt struct */
{
TXD		*pTxd;		/* pointer to the current descriptor */
int		i;
unsigned long	Flags;

	spin_lock_irqsave(&pTxPort->TxDesRingLock, Flags);
	pTxd = pTxPort->pTxdRingHead;
	for (i=0; i<pAC->TxDescrPerRing; i++) {
		pTxd->TBControl &= ~BMU_OWN;
		pTxd = pTxd->pNextTxd;
	}
	FreeTxDescriptors(pAC, pTxPort);
	spin_unlock_irqrestore(&pTxPort->TxDesRingLock, Flags);
} /* ClearTxRing */

/*****************************************************************************
 *
 * 	SkGeSetMacAddr - Set the hardware MAC address
 *
 * Description:
 *	This function sets the MAC address used by the adapter.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static int SkGeSetMacAddr(struct SK_NET_DEVICE *dev, void *p)
{

DEV_NET *pNet = PPRIV;
SK_AC	*pAC = pNet->pAC;
int	Ret;

struct sockaddr	*addr = p;
unsigned long	Flags;

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeSetMacAddr starts now...\n"));

	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	if (addr->sa_data == NULL) {
		printk(KERN_ERR "Null pointer from the upper layer\n");
		return -EBUSY;
	}

	if (dev->dev_addr == NULL) {
		printk(KERN_ERR "Device not available - NULL pointer\n");
		return -EBUSY;
	}

	if (pAC->BoardLevel == SK_INIT_DATA) {
		printk(KERN_ERR "Device already down - Don't handle event\n");
		return -EBUSY;
	}

	spin_lock_irqsave(&pAC->SlowPathLock, Flags);

	if (pAC->RlmtNets == 2)
		Ret = SkAddrOverride(pAC, pAC->IoBase, pNet->NetNr,
		      (SK_MAC_ADDR*) dev->dev_addr, SK_ADDR_VIRTUAL_ADDRESS);
	else
		Ret = SkAddrOverride(pAC, pAC->IoBase, pAC->ActivePort,
		      (SK_MAC_ADDR*) dev->dev_addr, SK_ADDR_VIRTUAL_ADDRESS);
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);

	if (Ret != SK_ADDR_OVERRIDE_SUCCESS)
		return -EBUSY;

	return 0;
} /* SkGeSetMacAddr */


/*****************************************************************************
 *
 * 	SkGeSetRxMode - set receive mode
 *
 * Description:
 *	This function sets the receive mode of an adapter. The adapter
 *	supports promiscuous mode, allmulticast mode and a number of
 *	multicast addresses. If more multicast addresses the available
 *	are selected, a hash function in the hardware is used.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static void SkGeSetRxMode(struct SK_NET_DEVICE *dev)
{

DEV_NET		*pNet;
SK_AC		*pAC;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
struct netdev_hw_addr   *ha;
#else
struct dev_mc_list      *pMcList;
int                     i;
#endif
int			PortIdx;
unsigned long		Flags;

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeSetRxMode starts now... "));

	pNet = PPRIV;
	pAC = pNet->pAC;
	if (pAC->RlmtNets == 1)
		PortIdx = pAC->ActivePort;
	else
		PortIdx = pNet->NetNr;

	spin_lock_irqsave(&pAC->SlowPathLock, Flags);
	if (dev->flags & IFF_PROMISC) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
			("PROMISCUOUS mode\n"));
		SkAddrPromiscuousChange(pAC, pAC->IoBase, PortIdx,
			SK_PROM_MODE_LLC);
	} else if (dev->flags & IFF_ALLMULTI) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
			("ALLMULTI mode\n"));
		SkAddrPromiscuousChange(pAC, pAC->IoBase, PortIdx,
			SK_PROM_MODE_ALL_MC);
	} else {
		SkAddrPromiscuousChange(pAC, pAC->IoBase, PortIdx,
			SK_PROM_MODE_NONE);
		SkAddrMcClear(pAC, pAC->IoBase, PortIdx, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
		netdev_for_each_mc_addr(ha,dev) {
			SkAddrMcAdd(pAC, pAC->IoBase, PortIdx,
			(SK_MAC_ADDR*)ha->addr, 0);
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MCA,
				("%02x:%02x:%02x:%02x:%02x:%02x\n",
				ha->addr[0],
				ha->addr[1],
				ha->addr[2],
				ha->addr[3],
				ha->addr[4],
				ha->addr[5]));
		}
#else
		pMcList = dev->mc_list;
		for (i=0; i<dev->mc_count; i++, pMcList = pMcList->next) {
			SkAddrMcAdd(pAC, pAC->IoBase, PortIdx,
				(SK_MAC_ADDR*)pMcList->dmi_addr, 0);
			SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_MCA,
				("%02x:%02x:%02x:%02x:%02x:%02x\n",
				pMcList->dmi_addr[0],
				pMcList->dmi_addr[1],
				pMcList->dmi_addr[2],
				pMcList->dmi_addr[3],
				pMcList->dmi_addr[4],
				pMcList->dmi_addr[5]));
		}
#endif
		SkAddrMcUpdate(pAC, pAC->IoBase, PortIdx);
	}
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);

	return;
} /* SkGeSetRxMode */


/*****************************************************************************
 *
 * 	SkSetMtuBufferSize - set the MTU buffer to another value
 *
 * Description:
 *	This function sets the new buffers and is called whenever the MTU
 *      size is changed
 *
 * Returns:
 *	N/A
 */

static void SkSetMtuBufferSize(
SK_AC	*pAC,		/* pointer to adapter context */
int	PortNr,		/* Port number */
int	Mtu)		/* pointer to tx prt struct */
{
	pAC->RxPort[PortNr].RxBufSize = Mtu + 32;

	/* RxBufSize must be a multiple of 8 */
	while (pAC->RxPort[PortNr].RxBufSize % 8) {
		pAC->RxPort[PortNr].RxBufSize =
			pAC->RxPort[PortNr].RxBufSize + 1;
	}

	if (Mtu > ETH_MAX_MTU) {
		pAC->GIni.GP[PortNr].PPortUsage = SK_JUMBO_LINK;
	} else {
		if ((pAC->GIni.GIMacsFound == 2 ) && (pAC->RlmtNets == 2)) {
			pAC->GIni.GP[PortNr].PPortUsage = SK_MUL_LINK;
		} else {
			pAC->GIni.GP[PortNr].PPortUsage = SK_RED_LINK;
		}
	}

	return;
}


/*****************************************************************************
 *
 * 	SkGeChangeMtu - set the MTU to another value
 *
 * Description:
 *	This function sets is called whenever the MTU size is changed
 *	(ifconfig mtu xxx dev ethX). If the MTU is bigger than standard
 *	ethernet MTU size, long frame support is activated.
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static int SkGeChangeMtu(struct SK_NET_DEVICE *dev, int NewMtu)
{
DEV_NET			*pNet;
SK_AC			*pAC;
unsigned long		Flags;
#ifdef CONFIG_SK98LIN_NAPI
int			WorkToDo = 1; // min(*budget, dev->quota);
int			WorkDone = 0;
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeChangeMtu starts now...\n"));

	pNet = PPRIV;
	pAC  = pNet->pAC;

	/* MTU size outside the spec */
	if ((NewMtu < 68) || (NewMtu > SK_JUMBO_MTU)) {
		return -EINVAL;
	}

	/* MTU > 1500 on yukon FE and FE+ not allowed */
	if (((pAC->GIni.GIChipId == CHIP_ID_YUKON_FE) ||
		 (pAC->GIni.GIChipId == CHIP_ID_YUKON_FE_P))
		&& (NewMtu > ETH_MAX_MTU)) {
		return -EINVAL;
	}

	/* Diag access active */
	if (pAC->DiagModeActive == DIAG_ACTIVE) {
		if (pAC->DiagFlowCtrl == SK_FALSE) {
			return -1; /* still in use, deny any actions of MTU */
		} else {
			pAC->DiagFlowCtrl = SK_FALSE;
		}
	}

	/* TSO on Yukon Ultra and MTU > 1500 not supported */
#ifdef NETIF_F_TSO
#ifdef USE_SK_TSO_FEATURE
	if (CHIP_ID_YUKON_2(pAC)) {
		if (((pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) && (NewMtu > ETH_MAX_MTU))
#ifdef	SK_AVB
			|| (pAC->GIni.GIChipId == CHIP_ID_YUKON_OPT)
			|| (pAC->GIni.GIChipId == CHIP_ID_YUKON_PRM)
#endif
		) {
			dev->features &= ~NETIF_F_TSO;
		} else {
			dev->features |= NETIF_F_TSO;
		}
	}
#endif
#endif
	dev->mtu = NewMtu;

	if (NewMtu < 1500) {
		return 0;
	}

	SkSetMtuBufferSize(pAC, pNet->PortNr, NewMtu);

	if (!netif_running(dev)) {
		/* Preset MTU size if device not ready/running */
		return 0;
	}

	/*  Prevent any reconfiguration while changing the MTU
	    by disabling any interrupts */
	SK_OUT32(pAC->IoBase, B0_IMSK, 0);
	spin_lock_irqsave(&pAC->SlowPathLock, Flags);

	NETIF_STOP_ALLQ(dev);
	SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP,
				pNet->PortNr, -1, SK_TRUE);
	spin_lock(&pAC->TxPort[pNet->PortNr].TxDesRingLock);


	/* Change RxFillLimit to 1 */
	if ((pAC->GIni.GIMacsFound == 2 ) && (pAC->RlmtNets == 2)) {
		pAC->RxPort[pNet->PortNr].RxFillLimit = 1;
	} else {
		pAC->RxPort[1 - pNet->PortNr].RxFillLimit = 1;
		pAC->RxPort[pNet->PortNr].RxFillLimit = pAC->RxDescrPerRing -
					(pAC->RxDescrPerRing / 4);
	}

	/* clear and reinit the rx rings here, because of new MTU size */
	if (CHIP_ID_YUKON_2(pAC)) {
		SkY2PortStop(pAC, pAC->IoBase, pNet->PortNr, SK_STOP_ALL, SK_SOFT_RST);
		SkY2AllocateRxBuffers(pAC, pAC->IoBase, pNet->PortNr);
		SkY2PortStart(pAC, pAC->IoBase, pNet->PortNr);
	} else {
#ifdef CONFIG_SK98LIN_NAPI
		WorkToDo = 1;
		ReceiveIrq(pAC, &pAC->RxPort[pNet->PortNr], SK_TRUE, &WorkDone, WorkToDo);
#else
		ReceiveIrq(pAC, &pAC->RxPort[pNet->PortNr], SK_TRUE);
#endif
		ClearRxRing(pAC, &pAC->RxPort[pNet->PortNr]);
		FillRxRing(pAC, &pAC->RxPort[pNet->PortNr]);

		/* Enable transmit descriptor polling */
		SkGePollTxD(pAC, pAC->IoBase, pNet->PortNr, SK_TRUE);
		FillRxRing(pAC, &pAC->RxPort[pNet->PortNr]);
	}

	NETIF_START_ALLQ(pAC->dev[pNet->PortNr]);
	spin_unlock(&pAC->TxPort[pNet->PortNr].TxDesRingLock);
	SkLocalEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START,
				pNet->PortNr, -1, SK_TRUE);

	/* Enable Interrupts again */
	SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);
	if (CHIP_ID_YUKON_2(pAC)) {
		SK_OUT32(pAC->IoBase, B0_HWE_IMSK, Y2_IRQ_HWE_MASK);
	}
	else {
		SK_OUT32(pAC->IoBase, B0_HWE_IMSK, IRQ_HWE_MASK);
	}

	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
	return 0;

}


/*****************************************************************************
 *
 * 	SkGeStats - return ethernet device statistics
 *
 * Description:
 *	This function return statistic data about the ethernet device
 *	to the operating system.
 *
 * Returns:
 *	pointer to the statistic structure.
 */
static struct net_device_stats *SkGeStats(struct SK_NET_DEVICE *dev)
{
	DEV_NET		*pNet = PPRIV;
	SK_AC		*pAC = pNet->pAC;
	unsigned long	LateCollisions, ExcessiveCollisions, RxTooLong, RxTooShort;
	unsigned long	Flags; /* for spin lock */
	SK_U32		MaxNumOidEntries, Oid, Len;
	char		Buf[8];
	struct {
		SK_U32         Oid;
		unsigned long *pVar;
	} Vars[] = {
		{ OID_SKGE_STAT_TX_LATE_COL,   &LateCollisions               },
		{ OID_SKGE_STAT_TX_EXCESS_COL, &ExcessiveCollisions          },
		{ OID_SKGE_STAT_RX_TOO_LONG,   &RxTooLong                    },
		{ OID_SKGE_STAT_RX_SHORTS,     &RxTooShort                   },
		{ OID_SKGE_STAT_RX,            &pAC->stats.rx_packets        },
		{ OID_SKGE_STAT_TX,            &pAC->stats.tx_packets        },
		{ OID_SKGE_STAT_RX_OCTETS,     &pAC->stats.rx_bytes          },
		{ OID_SKGE_STAT_TX_OCTETS,     &pAC->stats.tx_bytes          },
		{ OID_SKGE_RX_NO_BUF_CTS,      &pAC->stats.rx_dropped        },
		{ OID_SKGE_TX_NO_BUF_CTS,      &pAC->stats.tx_dropped        },
		{ OID_SKGE_STAT_RX_MULTICAST,  &pAC->stats.multicast         },
		{ OID_SKGE_STAT_RX_RUNT,       &pAC->stats.rx_length_errors  },
		{ OID_SKGE_STAT_RX_FCS,        &pAC->stats.rx_crc_errors     },
		{ OID_SKGE_STAT_RX_FRAMING,    &pAC->stats.rx_frame_errors   },
		{ OID_SKGE_STAT_RX_OVERFLOW,   &pAC->stats.rx_over_errors    },
		{ OID_SKGE_STAT_RX_MISSED,     &pAC->stats.rx_missed_errors  },
		{ OID_SKGE_STAT_TX_CARRIER,    &pAC->stats.tx_carrier_errors },
		{ OID_SKGE_STAT_TX_UNDERRUN,   &pAC->stats.tx_fifo_errors    },
	};

	spin_lock_irqsave(&pAC->SlowPathLock, Flags);
	if ((pAC->DiagModeActive == DIAG_NOTACTIVE) &&
		(pAC->BoardLevel     == SK_INIT_RUN)) {

		Len = sizeof(Buf);
		MaxNumOidEntries = sizeof(Vars) / sizeof(Vars[0]);
		for (Oid = 0; Oid < MaxNumOidEntries; Oid++) {
			if (SkPnmiGetVar(pAC,pAC->IoBase, Vars[Oid].Oid,
				Buf, &Len, 1, pNet->NetNr) != SK_PNMI_ERR_OK) {
				memset(Buf, 0x00, sizeof(Buf));
			}
			*Vars[Oid].pVar = (unsigned long) (*((SK_U64 *) Buf));
		}

		pAC->stats.rx_frame_errors = pAC->stats.rx_frame_errors +
					RxTooShort + RxTooLong;
		pAC->stats.collisions =	LateCollisions + ExcessiveCollisions;
		pAC->stats.tx_errors =	pAC->stats.tx_carrier_errors +
					pAC->stats.tx_fifo_errors;
		pAC->stats.rx_errors = 	pAC->stats.rx_length_errors +
					pAC->stats.rx_crc_errors +
					pAC->stats.rx_frame_errors +
					pAC->stats.rx_over_errors +
					pAC->stats.rx_missed_errors;

		if (dev->mtu > ETH_MAX_MTU) {
			pAC->stats.rx_errors = pAC->stats.rx_errors - RxTooLong;
		}
	}
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);

	return(&pAC->stats);
} /* SkGeStats */

/*****************************************************************************
 *
 * 	SkGeIoctl - IO-control function
 *
 * Description:
 *	This function is called if an ioctl is issued on the device.
 *	There are three subfunction for reading, writing and test-writing
 *	the private MIB data structure (usefull for SysKonnect-internal tools).
 *
 * Returns:
 *	0, if everything is ok
 *	!=0, on error
 */
static int SkGeIoctl(
struct SK_NET_DEVICE *dev,  /* the device the IOCTL is to be performed on   */
struct ifreq         *rq,   /* additional request structure containing data */
int                   cmd)  /* requested IOCTL command number               */
{
	DEV_NET          *pNet = PPRIV;
	SK_AC            *pAC  = pNet->pAC;
	struct pci_dev   *pdev = NULL;
	void             *pMemBuf;
	SK_GE_IOCTL       Ioctl;
	SK_GE_FWIOCTL     FwIoctl;
	unsigned long     Flags; /* for spin lock */
	unsigned int      Err = 0;
	unsigned int      Length = 0;
	int               HeaderLength = sizeof(SK_U32) + sizeof(SK_U32);
	int               Size = 0;
	int               Ret = 0;
#ifdef MV_INCLUDE_SDK_SUPPORT
	SK_U8             LinkData[256];
	SK_U16            LinkCap;
#endif

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeIoctl starts now...\n"));

	if(copy_from_user(&FwIoctl, rq->ifr_data, sizeof(SK_GE_FWIOCTL))) {
		return -EFAULT;
	}

	switch(cmd) {
	case SK_IOCTL_SETMIB:     /* FALL THRU */
	case SK_IOCTL_PRESETMIB:  /* FALL THRU (if capable!) */
		if (!capable(CAP_NET_ADMIN)) return -EPERM;
 	case SK_IOCTL_GETMIB:
		if(!copy_from_user(&pAC->PnmiStruct, Ioctl.pData,
			Ioctl.Len<sizeof(pAC->PnmiStruct)?
			Ioctl.Len : sizeof(pAC->PnmiStruct))) {
			Size = SkGeIocMib(pNet, Ioctl.Len, cmd);
			if(!copy_to_user(Ioctl.pData,&pAC->PnmiStruct,Ioctl.Len<Size? Ioctl.Len:Size)) {
				Ioctl.Len = Size;
				if(!copy_to_user(rq->ifr_data, &Ioctl, sizeof(SK_GE_IOCTL))) {
					return 0;
				}
			}
		}
		return -EFAULT;
		break;
	case SK_IOCTL_GEN:
		Length = sizeof(pAC->PnmiStruct) + HeaderLength;
		if (Ioctl.Len < (sizeof(pAC->PnmiStruct) + HeaderLength)) {
			Length = Ioctl.Len;
		}
		if ((pMemBuf = kmalloc(Length, GFP_KERNEL)) != NULL) {
			if(!copy_from_user(pMemBuf, Ioctl.pData, Length)) {
				spin_lock_irqsave(&pAC->SlowPathLock, Flags);
				Ret = SkPnmiGenIoctl(pAC,pAC->IoBase,pMemBuf,&Length,0);
				spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
				if (Ret >= 0) {
					if(!copy_to_user(Ioctl.pData, pMemBuf, Length) ) {
						Ioctl.Len = Length;
						if(!copy_to_user(rq->ifr_data,&Ioctl,sizeof(SK_GE_IOCTL))) {
							kfree(pMemBuf);
							return 0; /* copy to userland OK! */
						}
					}
				}
			}
			kfree(pMemBuf);
		}
		return -EFAULT;
		break;
	case SK_IOCTL_DIAG:
		if (!capable(CAP_NET_ADMIN)) return -EPERM;
		Length = sizeof(pAC->PnmiStruct) + HeaderLength;
		if (Ioctl.Len < (sizeof(pAC->PnmiStruct) + HeaderLength)) {
			Length = Ioctl.Len;
		}
		if ((pMemBuf = kmalloc(Length, GFP_KERNEL)) != NULL) {
			if(!copy_from_user(pMemBuf, Ioctl.pData, Length)) {
				pdev = pAC->PciDev;
				Length = 3 * sizeof(SK_U32);  /* Error, Bus and Device */
				* ((SK_U32 *)pMemBuf) = 0;
				* ((SK_U32 *)pMemBuf + 1) = pdev->bus->number;
				* ((SK_U32 *)pMemBuf + 2) = ParseDeviceNbrFromSlotName(pci_name(pdev));

				if(!copy_to_user(Ioctl.pData, pMemBuf, Length) ) {
					Ioctl.Len = Length;
					if(!copy_to_user(rq->ifr_data, &Ioctl, sizeof(SK_GE_IOCTL))) {
						kfree(pMemBuf);
						return 0; /* copy to userland OK! */
					}
				}
			}
			kfree(pMemBuf);
		}
		return -EFAULT;
		break;
#ifdef MV_INCLUDE_SDK_SUPPORT
	case SK_IOCTL_APPTOFW:
		if(!netif_running(dev))  return -EPERM;
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
		printk("Receive command from App layer. Command: 0x%x\n", FwIoctl.Command);
#endif
		if ((pMemBuf = kmalloc(FwIoctl.Len, GFP_KERNEL)) != NULL) {
			unsigned long Flags;  /* for spin lock */
			spin_lock_irqsave(&pAC->SlowPathLock, Flags);
			if(!copy_from_user(pMemBuf, FwIoctl.pData, FwIoctl.Len)) {
				if ( FwIoctl.Command == SK_IOCTL_FW_SEND) {
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
					printk("Receive data from App layer. Len: 0x%x\n", FwIoctl.Len);
#endif
					SendFwCommand(pAC, SKGE_DGRAM_MSG2FW, pMemBuf, FwIoctl.Len);
				} else if (FwIoctl.Command == SK_IOCTL_COMMAND) {
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
					printk("Receive command from App layer. Len: 0x%x\n", FwIoctl.Len);
#endif
					HandleDrvCommand(pAC, pMemBuf, FwIoctl.Len);
				} else if (FwIoctl.Command == SK_IOCTL_FW_RECEIVE) {
					if ((pAC->pFwBuffer != NULL) && (pAC->FwBufferLen)) {
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
						printk("Send data to the App layer. Len: 0x%x\n", pAC->FwBufferLen);
#endif
						if(!copy_to_user(FwIoctl.pData, pAC->pFwBuffer, pAC->FwBufferLen) ) {
							FwIoctl.Len = pAC->FwBufferLen;
							if(!copy_to_user(rq->ifr_data, &FwIoctl, sizeof(SK_GE_FWIOCTL))) {
								pAC->FwBufferLen = 0;
								kfree(pAC->pFwBuffer);
#ifdef VERBOSE_ASF_FIFO_DBUG_RPINTS
								printk("SEND DONE!!!!\n");
#endif
							}
						}
					}
				} else {
					Err = -EPERM;
				}
			}
			kfree(pMemBuf);
			spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
		}
		break;
	case SK_IOCTL_GETLINKCAP:
		if (netif_running(dev)) {
			if (copy_from_user(&LinkData[0], rq->ifr_data, sizeof(LinkData))) {
				return -EFAULT;
			}

			/* Get information about link partner capabilities. */
			SkGmPhyRead(pAC, pAC->IoBase, pNet->PortNr, PHY_MARV_AUNE_LP, &LinkCap);
			memcpy((char *) &LinkData[0], (char *) &LinkCap, sizeof(LinkCap));

			if ((pAC->dev[pNet->PortNr]->flags & IFF_RUNNING) == 0) {

				LinkData[2] = SK_AUTONEG_NOLINK; /* No link. */
			}
			else if ((pAC->GIni.GP[pNet->PortNr].PLinkModeConf == SK_LMODE_HALF) ||
				(pAC->GIni.GP[pNet->PortNr].PLinkModeConf == SK_LMODE_FULL)) {

				LinkData[2] = SK_AUTONEG_OFF; /* Autonegotiation not used. */
			}
			else {

				/* Get information about autonegotiation status. */
				LinkData[2] = pAC->GIni.GP[pNet->PortNr].PAutoNegFail;
			}

			if (copy_to_user(rq->ifr_data, &LinkData[0], sizeof(LinkData))) {
				return -EFAULT;
			}

			rq->ifr_name[0] = 'A';
			rq->ifr_name[1] = 'C';
			rq->ifr_name[2] = 'K';
		}
		else {
			rq->ifr_name[0] = 'D';
			rq->ifr_name[1] = 'E';
			rq->ifr_name[2] = 'C';
		}

		break;
#endif
	default:
#ifdef	SK_AVB
		Err = avb_ioctl(dev, rq, cmd);
#else	/* SK_AVB */
		Err = -EOPNOTSUPP;
#endif
	}
	return(Err);

} /* SkGeIoctl */


/*****************************************************************************
 *
 * 	SkGeIocMib - handle a GetMib, SetMib- or PresetMib-ioctl message
 *
 * Description:
 *	This function reads/writes the MIB data using PNMI (Private Network
 *	Management Interface).
 *	The destination for the data must be provided with the
 *	ioctl call and is given to the driver in the form of
 *	a user space address.
 *	Copying from the user-provided data area into kernel messages
 *	and back is done by copy_from_user and copy_to_user calls in
 *	SkGeIoctl.
 *
 * Returns:
 *	returned size from PNMI call
 */
static int SkGeIocMib(
DEV_NET		*pNet,	/* pointer to the adapter context */
unsigned int	Size,	/* length of ioctl data */
int		mode)	/* flag for set/preset */
{
	SK_AC		*pAC = pNet->pAC;
	unsigned long	Flags;  /* for spin lock */

	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("SkGeIocMib starts now...\n"));

	/* access MIB */
	spin_lock_irqsave(&pAC->SlowPathLock, Flags);
	switch(mode) {
	case SK_IOCTL_GETMIB:
		SkPnmiGetStruct(pAC, pAC->IoBase, &pAC->PnmiStruct, &Size,
			pNet->NetNr);
		break;
	case SK_IOCTL_PRESETMIB:
		SkPnmiPreSetStruct(pAC, pAC->IoBase, &pAC->PnmiStruct, &Size,
			pNet->NetNr);
		break;
	case SK_IOCTL_SETMIB:
		SkPnmiSetStruct(pAC, pAC->IoBase, &pAC->PnmiStruct, &Size,
			pNet->NetNr);
		break;
	default:
		break;
	}
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ENTRY,
		("MIB data access succeeded\n"));
	return (Size);
} /* SkGeIocMib */

/*****************************************************************************
 *
 * 	SetConfiguration - Get the configuration information
 *
 * Description:
 *	This function sets the per-adapter configuration information from
 *	the options provided on the command line and ethtool
 *
 * Returns:
 *	none
 */
static void SetConfiguration(
SK_AC	*pAC)	/* pointer to the adapter context structure */
{
	/* Set the link speed */
	pAC->GIni.GP[0].PLinkSpeed = pAC->LinkInfo[0].PLinkSpeed;
	pAC->GIni.GP[1].PLinkSpeed = pAC->LinkInfo[1].PLinkSpeed;

	/* Set the desired link mode */
	pAC->GIni.GP[0].PLinkModeConf = pAC->LinkInfo[0].PLinkModeConf;
	pAC->GIni.GP[1].PLinkModeConf = pAC->LinkInfo[1].PLinkModeConf;

	/* Set flow control */
	pAC->GIni.GP[0].PFlowCtrlMode = pAC->LinkInfo[0].PFlowCtrlMode;
	pAC->GIni.GP[1].PFlowCtrlMode = pAC->LinkInfo[1].PFlowCtrlMode;

	/* Set the role parameter */
	pAC->GIni.GP[0].PMSMode = pAC->LinkInfo[0].PMSMode;
	pAC->GIni.GP[1].PMSMode = pAC->LinkInfo[1].PMSMode;

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	if (pAC->LinkInfo[0].PLinkSpeed == SK_LSPEED_AUTO) {
	/* Autonegotiation enabled - disable Link Maintenance*/
		pAC->LinkInfo[0].LinkMaintenance = SK_FALSE;
	} else {
		if (pAC->LinkInfo[0].PLinkMaintenance)
			pAC->LinkInfo[0].LinkMaintenance = SK_TRUE;
		else
			pAC->LinkInfo[0].LinkMaintenance = SK_FALSE;
	}

	if (pAC->LinkInfo[1].PLinkSpeed == SK_LSPEED_AUTO) {
	/* Autonegotiation enabled - disable Link Maintenance*/
		pAC->LinkInfo[1].LinkMaintenance = SK_FALSE;
	}
#endif
}


/*****************************************************************************
 *
 * 	GetConfiguration - Get the configuration information
 *
 * Description:
 *	This function reads per-adapter configuration information from
 *	the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetConfiguration(
SK_AC	*pAC)	/* pointer to the adapter context structure */
{
	/* Get the ConType parameters */
	GetConTypeConfiguration(pAC);

	/* Get the WolType parameters */
	GetWolTypeConfiguration(pAC);

	/* Get link parameters for port A */
	if (!pAC->LinkInfo[0].PLinkSpeed) {
		GetSpeedConfiguration(pAC, 0, "Speed_A", Speed_A[pAC->Index]);
		GetLinkModeCapabilities(pAC, 0, "AutoNeg_A", AutoNeg_A[pAC->Index],
				"DupCap_A", DupCap_A[pAC->Index]);
		GetFlowControlConfiguration(pAC, 0, "FlowCtrl_A", FlowCtrl_A[pAC->Index]);
		GetRoleConfiguration(pAC, 0, "Role_A", Role_A[pAC->Index]);
	}

	/* Get link parameters for port B */
	if (!pAC->LinkInfo[1].PLinkSpeed) {
		GetSpeedConfiguration(pAC, 1, "Speed_B", Speed_B[pAC->Index]);
		GetLinkModeCapabilities(pAC, 1, "AutoNeg_B", AutoNeg_B[pAC->Index],
				"DupCap_B", DupCap_A[pAC->Index]);
		GetFlowControlConfiguration(pAC, 1, "FlowCtrl_B", FlowCtrl_B[pAC->Index]);
		GetRoleConfiguration(pAC, 1, "Role_B", Role_B[pAC->Index]);
	}

	/* Get interrupt moderation parameters */
	GetInterruptModConfiguration(pAC);

	/* Get misc parameters */
	GetMiscConfiguration(pAC);

}

/*****************************************************************************
 *
 * 	GetLinkCapabilities - Get the link Capabilities definition
 *
 * Description:
 *
 * Returns:
 *	LinkMode
 */
static int GetLinkCapabilities (
SK_U32	AutoNeg,
SK_U32	Duplex) {
	int Capabilities[3][3] = \
		{ {                -1, SK_LMODE_FULL     , SK_LMODE_HALF     },
		  {SK_LMODE_AUTOBOTH , SK_LMODE_AUTOFULL , SK_LMODE_AUTOHALF },
		  {SK_LMODE_AUTOSENSE, SK_LMODE_AUTOSENSE, SK_LMODE_AUTOSENSE} };
	return(Capabilities[AutoNeg][Duplex]);
}

/*****************************************************************************
 *
 * 	GetConTypeConfiguration - read ConType configuration information
 *
 * Description:
 *	This function reads per-adapter ConType configuration information from
 *	the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetConTypeConfiguration(
SK_AC	*pAC)	/* pointer to the adapter context structure */
{
SK_I32	Port;		/* preferred port */

	/*
	** Check merged parameter ConType. If it has not been used,
	** verify any other parameter (e.g. AutoNeg) and use default values.
	**
	** Stating both ConType and other lowlevel link parameters is also
	** possible. If this is the case, the passed ConType-parameter is
	** overwritten by the lowlevel link parameter.
	**
	** The following settings are used for a merged ConType-parameter:
	**
	** ConType   DupCap   AutoNeg   FlowCtrl      Role      Speed
	** -------   ------   -------   --------   ----------   -----
	**  Auto      Both      On      SymOrRem      Auto       Auto
	**  1000FD    Full      On        None      <ignored>    1000
	**  100FD     Full      Off       None      <ignored>    100
	**  100HD     Half      Off       None      <ignored>    100
	**  10FD      Full      Off       None      <ignored>    10
	**  10HD      Half      Off       None      <ignored>    10
	**
	** This ConType parameter is used for all ports of the adapter!
	*/

	if ((ConType != NULL)                &&
		(pAC->Index < SK_MAX_CARD_PARAM) &&
		(ConType[pAC->Index] != NULL) ) {

		/* Check chipset family */
		if ((!pAC->ChipsetType) &&
			(strcmp(ConType[pAC->Index],"Auto")!=0) &&
			(strcmp(ConType[pAC->Index],"")!=0)) {
			/* Set the speed parameter back */
			printk("sk98lin: Illegal value \"%s\" "
				"for ConType."
				" Using Auto.\n",
				ConType[pAC->Index]);

			ConType[pAC->Index] = "Auto";
		}

		if ((pAC->ChipsetType) &&
			(pAC->GIni.GICopperType != SK_TRUE) &&
			(strcmp(ConType[pAC->Index],"") != 0) &&
			(strcmp(ConType[pAC->Index],"1000FD") != 0)) {
			/* Set the speed parameter back */
			printk("sk98lin: Illegal value \"%s\" "
				"for ConType."
				" Using Auto.\n",
				ConType[pAC->Index]);
			ConType[pAC->Index] = "Auto";
		}

		if (strcmp(ConType[pAC->Index],"")==0) {
			/* No ConType defined */
		} else if (strcmp(ConType[pAC->Index],"Auto")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_ON, SKL_DC_BOTH);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_SYM_OR_REM;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_AUTO;
			}
		} else if (strcmp(ConType[pAC->Index],"1000FD")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_ON, SKL_DC_FULL);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_1000MBPS;
			}
		} else if (strcmp(ConType[pAC->Index],"100FD")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_OFF, SKL_DC_FULL);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_100MBPS;
			}
		} else if (strcmp(ConType[pAC->Index],"100HD")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_OFF, SKL_DC_HALF);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_100MBPS;
			}
		} else if (strcmp(ConType[pAC->Index],"10FD")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_OFF, SKL_DC_FULL);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_10MBPS;
			}
		} else if (strcmp(ConType[pAC->Index],"10HD")==0) {
			for (Port = 0; Port < SK_MAX_MACS; Port++) {
			pAC->LinkInfo[Port].PLinkModeConf =
					GetLinkCapabilities(SKL_AN_OFF, SKL_DC_HALF);
			pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_NONE;
			pAC->LinkInfo[Port].PMSMode       = SK_MS_MODE_AUTO;
			pAC->LinkInfo[Port].PLinkSpeed    = SK_LSPEED_10MBPS;
			}
		} else {
			printk("sk98lin: Illegal value \"%s\" for ConType\n",
				ConType[pAC->Index]);
		}
	}
}

/*****************************************************************************
 *
 * 	GetWolTypeConfiguration - read WolType configuration information
 *
 * Description:
 *	This function reads per-adapter WolType configuration information from
 *	the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetWolTypeConfiguration(
SK_AC	*pAC)	/* pointer to the adapter context structure */
{

	/*
	** Check merged parameter WolType.
	**
	** The following settings are used for a merged WolType-parameter:
	**
	** WolType   DupCap   AutoNeg   FlowCtrl      Role      Speed
	** -------   ------   -------   --------   ----------   -----
	**  Auto      Both      On      SymOrRem      Auto       Auto
	**  100FD     Full      Off       None      <ignored>    100
	**  100HD     Half      Off       None      <ignored>    100
	**  10FD      Full      Off       None      <ignored>    10
	**  10HD      Half      Off       None      <ignored>    10
	**
	*/

	/* Default without user configuration */
	pAC->WolSpeedType = SK_LSPEED_100MBPS;
	pAC->WolDuplexType = SK_TRUE;

	if ((WolType != NULL)                &&
		(pAC->Index < SK_MAX_CARD_PARAM) &&
		(WolType[pAC->Index] != NULL) ) {

		/* Set the WOL parameter */
		if (strcmp(WolType[pAC->Index],"")==0) {
			pAC->WolSpeedType = SK_LSPEED_100MBPS;
			pAC->WolDuplexType = SK_TRUE;
		} else if (strcmp(WolType[pAC->Index],"Auto")==0) {
			pAC->WolSpeedType = SK_LSPEED_AUTO;
			pAC->WolDuplexType = SK_TRUE;
		} else if (strcmp(WolType[pAC->Index],"100FD")==0) {
			pAC->WolSpeedType = SK_LSPEED_100MBPS;
			pAC->WolDuplexType = SK_TRUE;
		} else if (strcmp(WolType[pAC->Index],"100HD")==0) {
			pAC->WolSpeedType = SK_LSPEED_100MBPS;
			pAC->WolDuplexType = SK_FALSE;
		} else if (strcmp(WolType[pAC->Index],"10FD")==0) {
			pAC->WolSpeedType = SK_LSPEED_10MBPS;
			pAC->WolDuplexType = SK_TRUE;
		} else if (strcmp(WolType[pAC->Index],"10HD")==0) {
			pAC->WolSpeedType = SK_LSPEED_10MBPS;
			pAC->WolDuplexType = SK_FALSE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for WolType\n",
				WolType[pAC->Index]);
			pAC->WolSpeedType = SK_LSPEED_100MBPS;
			pAC->WolDuplexType = SK_TRUE;

		}
	}
}

/*****************************************************************************
 *
 * 	GetSpeedConfiguration - read Speed configuration information
 *
 * Description:
 *	This function reads per-adapter speed configuration information from
 *	the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetSpeedConfiguration (
SK_AC	*pAC,		/* pointer to the adapter context structure */
SK_I32	Port,		/* Port number */
char	*ParamName,	/* Parameter name */
char	*ParamValue)	/* Parameter value */
{

	/*
	** Parse any parameter settings for port
	** a) any LinkSpeed stated?
	*/
	pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_AUTO;
	if (ParamValue != NULL && pAC->Index<SK_MAX_CARD_PARAM) {
		if (strcmp(ParamValue,"")==0) {
			/* Not defined */
		} else if (strcmp(ParamValue,"Auto")==0) {
			pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_AUTO;
		} else if (strcmp(ParamValue,"10")==0) {
			pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_10MBPS;
		} else if (strcmp(ParamValue,"100")==0) {
			pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_100MBPS;
		} else if (strcmp(ParamValue,"1000")==0) {
			if ((pAC->PciDev->vendor == 0x11ab ) &&
				((pAC->PciDev->device == 0x4350) ||
				 (pAC->PciDev->device == 0x4354))) {
				pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_100MBPS;
				printk("sk98lin: Illegal value \"%s\" for %s.\n"
					"Gigabit speed not possible with this chip revision!",
					ParamValue, ParamName);
			} else {
				pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_1000MBPS;
			}
		} else {
			printk("sk98lin: Illegal value \"%s\" for %s\n",
			ParamValue, ParamName);
		}
	} else {
		if ((pAC->PciDev->vendor == 0x11ab ) &&
			((pAC->PciDev->device == 0x4350) ||
			 (pAC->PciDev->device == 0x4354))) {
			/* Gigabit speed not supported
			 * Swith to speed 100
			 */
			pAC->LinkInfo[Port].PLinkSpeed = SK_LSPEED_100MBPS;
		}
	}
}

/*****************************************************************************
 *
 * 	GetLinkModeCapabilities - read link mode capabilities
 *
 * Description:
 *	This function reads per-adapter link capabilities information from
 *	the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetLinkModeCapabilities (
SK_AC	*pAC,			/* pointer to the adapter context structure */
SK_I32	Port,			/* Port number */
char	*AutoNegParamName,	/* AutoNeg Parameter name */
char	*AutoNegParamValue,	/* AutoNeg Parameter value */
char	*DupCapParamName,	/* DupCap Parameter name */
char	*DupCapParamValue)	/* DupCap Parameter value */
{
	SK_BOOL		AutoSet;
	SK_BOOL		DupSet;
	int		AutoNeg	= 1;	/* autoneg off (0) or on (1) */
	int		DuplexCap = 0;	/* 0=both,1=full,2=half */

/*
 *	The two parameters AutoNeg. and DuplexCap. map to one configuration
 *	parameter. The mapping is described by this table:
 *	DuplexCap ->	|	both	|	full	|	half	|
 *	AutoNeg		|		|		|		|
 *	-----------------------------------------------------------------
 *	Off		|    illegal	|	Full	|	Half	|
 *	-----------------------------------------------------------------
 *	On		|   AutoBoth	|   AutoFull	|   AutoHalf	|
 *	-----------------------------------------------------------------
 *	Sense		|   AutoSense	|   AutoSense	|   AutoSense	|
 */


	/* Set the default value */
	pAC->LinkInfo[Port].PLinkModeConf =
		GetLinkCapabilities(SKL_AN_ON, SKL_DC_BOTH);

	/*
	** Any Autonegotiation and DuplexCapabilities set?
	** Please note that both belong together...
	*/
	AutoNeg = SKL_AN_ON;
	AutoSet = SK_FALSE;
	if (AutoNegParamValue != NULL && pAC->Index<SK_MAX_CARD_PARAM) {
		AutoSet = SK_TRUE;
		if (strcmp(AutoNegParamValue,"")==0) {
			AutoSet = SK_FALSE;
		} else if (strcmp(AutoNegParamValue,"On")==0) {
			AutoNeg = SKL_AN_ON;
		} else if (strcmp(AutoNegParamValue,"Off")==0) {
			AutoNeg = SKL_AN_OFF;
		} else if (strcmp(AutoNegParamValue,"Sense")==0) {
			AutoNeg = SKL_AN_SENS;
		} else {
			printk("sk98lin: Illegal value \"%s\" for %s\n",
			AutoNegParamValue, AutoNegParamName);
		}
	}

	DuplexCap = SKL_DC_BOTH;
	DupSet    = SK_FALSE;
	if (DupCapParamValue != NULL && pAC->Index<SK_MAX_CARD_PARAM) {
		DupSet = SK_TRUE;
		if (strcmp(DupCapParamValue,"")==0) {
			DupSet = SK_FALSE;
		} else if (strcmp(DupCapParamValue,"Both")==0) {
			DuplexCap = SKL_DC_BOTH;
		} else if (strcmp(DupCapParamValue,"Full")==0) {
			DuplexCap = SKL_DC_FULL;
		} else if (strcmp(DupCapParamValue,"Half")==0) {
			DuplexCap = SKL_DC_HALF;
		} else {
			printk("sk98lin: Illegal value \"%s\" for %s\n",
			DupCapParamValue, DupCapParamName);
		}
	}

	/*
	** Check for illegal combinations
	*/
	if ((pAC->LinkInfo[Port].PLinkSpeed == SK_LSPEED_1000MBPS) &&
		(AutoNeg==SKL_AN_OFF)) {
		printk("sk98lin: Autonegotiation mandatory for Gigabit Speed\n"
			"	Enabling Autonegotiation\n");
		AutoNeg = SKL_AN_ON;
	}


	if ((pAC->LinkInfo[Port].PLinkSpeed == SK_LSPEED_1000MBPS) &&
		((DuplexCap == SK_LMODE_STAT_AUTOHALF) ||
		(DuplexCap == SK_LMODE_STAT_HALF)) &&
		(pAC->ChipsetType)) {
			printk("sk98lin: Half Duplex not possible with Gigabit speed!\n"
					"    Using Full Duplex.\n");
				DuplexCap = SKL_DC_FULL;
	}

	if ( AutoSet && AutoNeg==SKL_AN_SENS && DupSet) {
		printk("sk98lin, Port A: DuplexCapabilities"
			" ignored using Sense mode\n");
	}

	if (AutoSet && AutoNeg==SKL_AN_OFF &&
		DupSet && DuplexCap==SKL_DC_BOTH){
		printk("sk98lin: Port A: Illegal combination"
			" of values AutoNeg. and DuplexCap.\n    Using "
			"Full Duplex\n");
		DuplexCap = SKL_DC_FULL;
	}

	if (AutoSet && AutoNeg==SKL_AN_OFF && !DupSet) {
		DuplexCap = SKL_DC_FULL;
	}

	if (!AutoSet && DupSet) {
		AutoNeg = SKL_AN_ON;
	}


	pAC->LinkInfo[Port].PAutoNeg = AutoNeg;
	pAC->LinkInfo[Port].PDuplexCap = DuplexCap;

	/*
	** set the desired mode
	*/
	if (AutoSet || DupSet) {
		pAC->LinkInfo[Port].PLinkModeConf =
				GetLinkCapabilities(AutoNeg, DuplexCap);
	}
}

/*****************************************************************************
 *
 * 	GetFlowControlConfiguration - read the flow control configuration
 *
 * Description:
 *	This function reads per-adapter flow control configuration information
 *	from the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetFlowControlConfiguration (
SK_AC	*pAC,		/* pointer to the adapter context structure */
SK_I32	Port,		/* Port number */
char	*ParamName,	/* Parameter name */
char	*ParamValue)	/* Parameter value */
{
	int	FlowCtrl = SK_FLOW_MODE_SYM_OR_REM;	/* FlowControl  */
	SK_BOOL IsFlowCtrlDefined	= SK_TRUE;

	/* Set the default value */
	pAC->LinkInfo[Port].PFlowCtrlMode = SK_FLOW_MODE_SYM_OR_REM;

	/*
	** Any Flowcontrol-parameter set?
	*/
	if (ParamValue != NULL && pAC->Index<SK_MAX_CARD_PARAM) {
		if (strcmp(ParamValue,"") == 0) {
			/* Not defined */
			IsFlowCtrlDefined = SK_FALSE;
		} else if (strcmp(ParamValue,"SymOrRem") == 0) {
			FlowCtrl = SK_FLOW_MODE_SYM_OR_REM;
		} else if (strcmp(ParamValue,"Sym")==0) {
			FlowCtrl = SK_FLOW_MODE_SYMMETRIC;
		} else if (strcmp(ParamValue,"LocSend")==0) {
			FlowCtrl = SK_FLOW_MODE_LOC_SEND;
		} else if (strcmp(ParamValue,"None")==0) {
			FlowCtrl = SK_FLOW_MODE_NONE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for %s\n",
			ParamValue, ParamName);
			IsFlowCtrlDefined = SK_FALSE;
		}
	} else {
			IsFlowCtrlDefined = SK_FALSE;
	}

	/* Check the values */
	if (IsFlowCtrlDefined) {
		if ((pAC->LinkInfo[Port].PAutoNeg == SKL_AN_OFF) &&
		(FlowCtrl != SK_FLOW_MODE_NONE)) {
		printk("sk98lin: %s: FlowControl"
			" impossible without AutoNegotiation,"
			" disabled\n", ParamName);
		FlowCtrl = SK_FLOW_MODE_NONE;
		}
		pAC->LinkInfo[Port].PFlowCtrlMode = FlowCtrl;
	}
}

/*****************************************************************************
 *
 * 	GetRoleConfiguration - read the role configuration
 *
 * Description:
 *	This function reads per-adapter role configuration information
 *	from the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetRoleConfiguration (
SK_AC	*pAC,		/* pointer to the adapter context structure */
SK_I32	Port,		/* Port number */
char	*ParamName,	/* Parameter name */
char	*ParamValue)	/* Parameter value */
{

	/* Set the default value */
	pAC->LinkInfo[Port].PMSMode = SK_MS_MODE_AUTO;

	/*
	** What is with the RoleParameter?
	*/
	if (ParamValue != NULL && pAC->Index<SK_MAX_CARD_PARAM) {
		if (strcmp(ParamValue,"")==0) {
		} else if (strcmp(ParamValue,"Auto")==0) {
			pAC->LinkInfo[Port].PMSMode = SK_MS_MODE_AUTO;
		} else if (strcmp(ParamValue,"Master")==0) {
			pAC->LinkInfo[Port].PMSMode = SK_MS_MODE_MASTER;
		} else if (strcmp(ParamValue,"Slave")==0) {
			pAC->LinkInfo[Port].PMSMode = SK_MS_MODE_SLAVE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for %s\n",
			ParamValue, ParamName);
		}
	}
}

/*****************************************************************************
 *
 * 	GetInterruptModConfiguration - read the IntMod configuration
 *
 * Description:
 *	This function reads per-adapter interrupt moderation configuration
 *	information from the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetInterruptModConfiguration (
SK_AC	*pAC)		/* pointer to the adapter context structure */
{
	int IrqModMaskOffset	= 6; /* all ints moderated=default */
	SK_U32	IrqModMask[7][2] =
		{ { IRQ_MASK_RX_ONLY , Y2_DRIVER_IRQS  },
		  { IRQ_MASK_TX_ONLY , Y2_DRIVER_IRQS  },
		  { IRQ_MASK_SP_ONLY , Y2_SPECIAL_IRQS },
		  { IRQ_MASK_SP_RX   , Y2_IRQ_MASK     },
		  { IRQ_MASK_TX_RX   , Y2_DRIVER_IRQS  },
		  { IRQ_MASK_SP_TX   , Y2_IRQ_MASK     },
		  { IRQ_MASK_RX_TX_SP, Y2_IRQ_MASK     } };

	/*
	** Check the interrupt moderation parameters
	*/
	pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_NONE;
	if (Moderation[pAC->Index] != NULL) {
		if (strcmp(Moderation[pAC->Index], "") == 0) {
			pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_NONE;
		} else if (strcmp(Moderation[pAC->Index], "Static") == 0) {
			pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_STATIC;
		} else if (strcmp(Moderation[pAC->Index], "Dynamic") == 0) {
			pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_DYNAMIC;
		} else if (strcmp(Moderation[pAC->Index], "None") == 0) {
			pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_NONE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for Moderation.\n"
				"      Disable interrupt moderation.\n",
				Moderation[pAC->Index]);
		}
	} else {
/* Set interrupt moderation if wished */
#ifdef CONFIG_SK98LIN_STATINT
		pAC->DynIrqModInfo.IntModTypeSelect = C_INT_MOD_STATIC;
#endif
	}

	if (ModerationMask[pAC->Index] != NULL) {
		if (strcmp(ModerationMask[pAC->Index], "Rx") == 0) {
			IrqModMaskOffset = 0;
		} else if (strcmp(ModerationMask[pAC->Index], "Tx") == 0) {
			IrqModMaskOffset = 1;
		} else if (strcmp(ModerationMask[pAC->Index], "Sp") == 0) {
			IrqModMaskOffset = 2;
		} else if (strcmp(ModerationMask[pAC->Index], "RxSp") == 0) {
			IrqModMaskOffset = 3;
		} else if (strcmp(ModerationMask[pAC->Index], "SpRx") == 0) {
			IrqModMaskOffset = 3;
		} else if (strcmp(ModerationMask[pAC->Index], "RxTx") == 0) {
			IrqModMaskOffset = 4;
		} else if (strcmp(ModerationMask[pAC->Index], "TxRx") == 0) {
			IrqModMaskOffset = 4;
		} else if (strcmp(ModerationMask[pAC->Index], "TxSp") == 0) {
			IrqModMaskOffset = 5;
		} else if (strcmp(ModerationMask[pAC->Index], "SpTx") == 0) {
			IrqModMaskOffset = 5;
		} else { /* some rubbish stated */
			// IrqModMaskOffset = 6; ->has been initialized
			// already at the begin of this function...
		}
	}
	if (!CHIP_ID_YUKON_2(pAC)) {
		pAC->DynIrqModInfo.MaskIrqModeration = IrqModMask[IrqModMaskOffset][0];
	} else {
		pAC->DynIrqModInfo.MaskIrqModeration = IrqModMask[IrqModMaskOffset][1];
	}

	if (!CHIP_ID_YUKON_2(pAC)) {
		pAC->DynIrqModInfo.MaxModIntsPerSec = C_INTS_PER_SEC_DEFAULT;
	} else {
		pAC->DynIrqModInfo.MaxModIntsPerSec = C_Y2_INTS_PER_SEC_DEFAULT;
	}
	if (IntsPerSec[pAC->Index] != 0) {
		if ((IntsPerSec[pAC->Index]< C_INT_MOD_IPS_LOWER_RANGE) ||
			(IntsPerSec[pAC->Index] > C_INT_MOD_IPS_UPPER_RANGE)) {
			printk("sk98lin: Illegal value \"%d\" for IntsPerSec. (Range: %d - %d)\n"
				"      Using default value of %i.\n",
				IntsPerSec[pAC->Index],
				C_INT_MOD_IPS_LOWER_RANGE,
				C_INT_MOD_IPS_UPPER_RANGE,
				pAC->DynIrqModInfo.MaxModIntsPerSec);
		} else {
			pAC->DynIrqModInfo.MaxModIntsPerSec = IntsPerSec[pAC->Index];
		}
	}

	/*
	** Evaluate upper and lower moderation threshold
	*/
	pAC->DynIrqModInfo.MaxModIntsPerSecUpperLimit =
		pAC->DynIrqModInfo.MaxModIntsPerSec +
		(pAC->DynIrqModInfo.MaxModIntsPerSec / 5);

	pAC->DynIrqModInfo.MaxModIntsPerSecLowerLimit =
		pAC->DynIrqModInfo.MaxModIntsPerSec -
		(pAC->DynIrqModInfo.MaxModIntsPerSec / 5);

	pAC->DynIrqModInfo.DynIrqModSampleInterval =
		SK_DRV_MODERATION_TIMER_LENGTH;
}

/*****************************************************************************
 *
 * 	GetMiscConfiguration - read the misc configuration
 *
 * Description:
 *	This function reads per-adapter misc configuration information
 *	from the options provided on the command line.
 *
 * Returns:
 *	none
 */
static void GetMiscConfiguration (
SK_AC	*pAC)		/* pointer to the adapter context structure */
{
	pAC->LinkInfo[0].RSS = SK_FALSE;
	pAC->LinkInfo[1].RSS = SK_FALSE;

#ifdef USE_SK_RSS_SUPPORT
	/*
	 * Check the RSS parameter configuration
	 */
	if (RSS[pAC->Index] != NULL) {
		if (strcmp(RSS[pAC->Index], "On") == 0) {
			pAC->LinkInfo[0].RSS = SK_TRUE;
			pAC->LinkInfo[1].RSS = SK_TRUE;
		} else if (strcmp(RSS[pAC->Index], "Off") == 0) {
			pAC->LinkInfo[0].RSS = SK_FALSE;
			pAC->LinkInfo[1].RSS = SK_FALSE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for RSS.\n"
				"      Disable Receive-Side Scaling.\n",
				RSS[pAC->Index]);
		}
	}
#endif

#ifdef MV_INCLUDE_SDK_SUPPORT
	/*
	 * Check the SDK queue handling
	 */
	pAC->SdkQH = 0;
	if (SdkQH[pAC->Index] != NULL) {
		if (strcmp(SdkQH[pAC->Index], "On") == 0) {
			pAC->SdkQH = 1;
		} else if (strcmp(SdkQH[pAC->Index], "Off") == 0) {
			pAC->SdkQH = 0;
		} else if (strcmp(SdkQH[pAC->Index], "1") == 0) {
			pAC->SdkQH = 1;
		} else if (strcmp(SdkQH[pAC->Index], "0") == 0) {
			pAC->SdkQH = 0;
		} else {
			printk("sk98lin: Illegal value \"%s\" for SdkQH.\n"
				"      Disable SDK queue handling.\n",
				SdkQH[pAC->Index]);
		}
	}
#endif

#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
	/*
	** Check the LinkMaintenance parameter
	*/
	pAC->LinkInfo[0].PLinkMaintenance = SK_TRUE;
	pAC->LinkInfo[1].PLinkMaintenance = SK_FALSE;

	if (LinkMaintenance[pAC->Index] != NULL) {
		if (strcmp(LinkMaintenance[pAC->Index], "On") == 0) {
			pAC->LinkInfo[0].PLinkMaintenance = SK_TRUE;
		} else if (strcmp(LinkMaintenance[pAC->Index], "Off") == 0) {
			pAC->LinkInfo[0].PLinkMaintenance = SK_FALSE;
		} else {
			printk("sk98lin: Illegal value \"%s\" for LinkMaintenance.\n"
				"      Enable LinkMaintenance.\n",
				LinkMaintenance[pAC->Index]);
		}
	}
#endif

	/* Evaluate settings for both ports */
	pAC->ActivePort = 0;
	pAC->RlmtNets = 1;

	if (pAC->GIni.GIMacsFound == 2) {
		pAC->RlmtNets = 2;
	}

	/*
	** Check the TxModeration parameters
	*/
	pAC->TxModeration = 0;
	if (TxModeration[pAC->Index] != 0) {
		if (!HW_IS_EXT_LE_FORMAT(pAC)) {
			printk("sk98lin: Illegal value for TxModeration. "
				"Not a Yukon 2 card\n    Disable tx moderation.\n");

		} else if ((TxModeration[pAC->Index] < 1) ||
			(TxModeration[pAC->Index] > C_TX_INT_MOD_UPPER_RANGE)) {
			printk("sk98lin: Illegal value \"%d\" for TxModeration. (Range: 1 - %d)\n"
				"      Disable tx moderation.\n",
				TxModeration[pAC->Index],
				C_TX_INT_MOD_UPPER_RANGE);
		} else {
			pAC->TxModeration = TxModeration[pAC->Index];
		}
	}

	/*
	** Check the LowLatency parameters
	*/
	pAC->LowLatency = SK_FALSE;
	if (LowLatency[pAC->Index] != NULL) {
		if (strcmp(LowLatency[pAC->Index], "On") == 0) {
			pAC->LowLatency = SK_TRUE;
		}
	}

	/*
	** Check the MsiIrq parameters
	*/
	pAC->MsiIrq = SK_TRUE;
	if (MsiIrq[pAC->Index] != NULL) {
		if (strcmp(MsiIrq[pAC->Index], "Off") == 0) {
			pAC->MsiIrq = SK_FALSE;
		}
	}
}

/*****************************************************************************
 *
 * 	ProductStr - return a adapter identification string from vpd
 *
 * Description:
 *	This function reads the product name string from the vpd area
 *	and puts it the field pAC->DeviceString.
 *
 * Returns: N/A
 */
static void ProductStr(SK_AC *pAC)
{
	char Default[] = "Generic Marvell Yukon chipset Ethernet device";
	char Key[] = VPD_NAME; /* VPD productname key */
	int StrLen = 80;       /* stringlen           */
	unsigned long Flags;
	int ReturnCode = 0;

	spin_lock_irqsave(&pAC->SlowPathLock, Flags);
	if ((ReturnCode = VpdRead(pAC, pAC->IoBase, Key, pAC->DeviceStr, &StrLen)) != 0) {
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_ERROR,
			("Error reading VPD data: %d\n", ReturnCode));
		strcpy(pAC->DeviceStr, Default);
	}
	spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);
} /* ProductStr */

/****************************************************************************/
/* functions for common modules *********************************************/
/****************************************************************************/

/*****************************************************************************
 *
 *	SkOsGetTime - provide a time value
 *
 * Description:
 *	This routine provides a time value. The unit is 1/HZ (defined by Linux).
 *	It is not used for absolute time, but only for time differences.
 *
 *
 * Returns:
 *	Time value
 */
SK_U64 SkOsGetTime(SK_AC *pAC)
{
	SK_U64	PrivateJiffies;

	SkOsGetTimeCurrent(pAC, &PrivateJiffies);

	return PrivateJiffies;
} /* SkOsGetTime */


/*****************************************************************************
 *
 *	SkPciReadCfgDWord - read a 32 bit value from pci config space
 *
 * Description:
 *	This routine reads a 32 bit value from the pci configuration
 *	space.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciReadCfgDWord(
SK_AC *pAC,		/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U32 *pVal)		/* pointer to store the read value */
{
	pci_read_config_dword(pAC->PciDev, PciAddr, pVal);
	return(0);
} /* SkPciReadCfgDWord */


/*****************************************************************************
 *
 *	SkPciReadCfgWord - read a 16 bit value from pci config space
 *
 * Description:
 *	This routine reads a 16 bit value from the pci configuration
 *	space.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciReadCfgWord(
SK_AC *pAC,	/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U16 *pVal)		/* pointer to store the read value */
{
	pci_read_config_word(pAC->PciDev, PciAddr, pVal);
	return(0);
} /* SkPciReadCfgWord */


/*****************************************************************************
 *
 *	SkPciReadCfgByte - read a 8 bit value from pci config space
 *
 * Description:
 *	This routine reads a 8 bit value from the pci configuration
 *	space.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciReadCfgByte(
SK_AC *pAC,	/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U8 *pVal)		/* pointer to store the read value */
{
	pci_read_config_byte(pAC->PciDev, PciAddr, pVal);
	return(0);
} /* SkPciReadCfgByte */


/*****************************************************************************
 *
 *	SkPciWriteCfgDWord - write a 32 bit value to pci config space
 *
 * Description:
 *	This routine writes a 32 bit value to the pci configuration
 *	space.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciWriteCfgDWord(
SK_AC *pAC,	/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U32 Val)		/* pointer to store the read value */
{
	pci_write_config_dword(pAC->PciDev, PciAddr, Val);
	return(0);
} /* SkPciWriteCfgDWord */


/*****************************************************************************
 *
 *	SkPciWriteCfgWord - write a 16 bit value to pci config space
 *
 * Description:
 *	This routine writes a 16 bit value to the pci configuration
 *	space. The flag PciConfigUp indicates whether the config space
 *	is accesible or must be set up first.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciWriteCfgWord(
SK_AC *pAC,	/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U16 Val)		/* pointer to store the read value */
{
	pci_write_config_word(pAC->PciDev, PciAddr, Val);
	return(0);
} /* SkPciWriteCfgWord */


/*****************************************************************************
 *
 *	SkPciWriteCfgWord - write a 8 bit value to pci config space
 *
 * Description:
 *	This routine writes a 8 bit value to the pci configuration
 *	space. The flag PciConfigUp indicates whether the config space
 *	is accesible or must be set up first.
 *
 * Returns:
 *	0 - indicate everything worked ok.
 *	!= 0 - error indication
 */
int SkPciWriteCfgByte(
SK_AC *pAC,	/* Adapter Control structure pointer */
int PciAddr,		/* PCI register address */
SK_U8 Val)		/* pointer to store the read value */
{
	pci_write_config_byte(pAC->PciDev, PciAddr, Val);
	return(0);
} /* SkPciWriteCfgByte */


/*****************************************************************************
 *
 *	SkDrvEvent - handle driver events
 *
 * Description:
 *	This function handles events from all modules directed to the driver
 *
 * Context:
 *	Is called under protection of slow path lock.
 *
 * Returns:
 *	0 if everything ok
 *	< 0  on error
 *
 */
int SkDrvEvent(
SK_AC     *pAC,    /* pointer to adapter context */
SK_IOC     IoC,    /* IO control context         */
SK_U32     Event,  /* event-id                   */
SK_EVPARA  Param)  /* event-parameter            */
{
	unsigned long    Flags;
	int              FromPort;    /* the port from which we switch away */
	int              Stat = 0;
	DEV_NET          *pNet = NULL;
#ifdef CONFIG_SK98LIN_NAPI
	int              WorkToDo = 1; /* min(*budget, dev->quota); */
	int              WorkDone = 0;
#endif
	SK_EVPARA        Para;

	switch (Event) {
	case SK_DRV_PORT_FAIL:
		FromPort = Param.Para32[0];
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("PORT FAIL EVENT, Port: %d\n", FromPort));
		if (FromPort == 0) {
			printk("%s: Port A failed.\n", pAC->dev[0]->name);
		} else {
			printk("%s: Port B failed.\n", pAC->dev[1]->name);
		}
		break;
	case SK_DRV_PORT_RESET:
		FromPort = Param.Para32[0];
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("PORT RESET EVENT, Port: %d ", FromPort));
		SkLocalEventQueue64(pAC, SKGE_PNMI, SK_PNMI_EVT_XMAC_RESET,
					FromPort, SK_FALSE);
		spin_lock_irqsave(
			&pAC->TxPort[FromPort].TxDesRingLock,
			Flags);
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2PortStop(pAC, IoC, FromPort, SK_STOP_ALL, SK_HARD_RST);
		} else {
			SkGeStopPort(pAC, IoC, FromPort, SK_STOP_ALL, SK_HARD_RST);
		}
		pAC->dev[FromPort]->flags &= ~IFF_RUNNING;
		spin_unlock_irqrestore(
			&pAC->TxPort[FromPort].TxDesRingLock,
			Flags);

		if (!CHIP_ID_YUKON_2(pAC)) {
#ifdef CONFIG_SK98LIN_NAPI
			WorkToDo = 1;
			ReceiveIrq(pAC, &pAC->RxPort[FromPort], SK_FALSE, &WorkDone, WorkToDo);
#else
			ReceiveIrq(pAC, &pAC->RxPort[FromPort], SK_FALSE);
#endif
			ClearTxRing(pAC, &pAC->TxPort[FromPort]);
		}
		spin_lock_irqsave(
			&pAC->TxPort[FromPort].TxDesRingLock,
			Flags);

#ifdef USE_TIST_FOR_RESET
		if (!HW_IS_EXT_LE_FORMAT(pAC) && pAC->GIni.GIYukon2) {
#ifdef Y2_RECOVERY
			/* for Yukon II we want to have tist enabled all the time */
			if (!SK_ADAPTER_WAITING_FOR_TIST(pAC)) {
				SK_Y2_TIST_LE_ENA(pAC->IoBase);
			}
#else
			/* make sure that we do not accept any status LEs from now on */
			if (SK_ADAPTER_WAITING_FOR_TIST(pAC)) {
#endif
				/* port already waiting for tist */
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
					("Port %c is now waiting for specific Tist\n",
					'A' +  FromPort));
				SK_SET_WAIT_BIT_FOR_PORT(
					pAC,
					SK_PSTATE_WAITING_FOR_SPECIFIC_TIST,
					FromPort);
				/* get current timestamp */
				Y2_GET_TIST_LOW_VAL(pAC->IoBase, &pAC->MinTistLo);
				pAC->MinTistHi = pAC->GIni.GITimeStampCnt;
#ifndef Y2_RECOVERY
			} else {
				/* nobody is waiting yet */
				SK_SET_WAIT_BIT_FOR_PORT(
					pAC,
					SK_PSTATE_WAITING_FOR_ANY_TIST,
					FromPort);
				SK_DBG_MSG(pAC, SK_DBGMOD_DRV, SK_DBGCAT_DUMP,
					("Port %c is now waiting for any Tist (0x%X)\n",
					'A' +  FromPort, pAC->AdapterResetState));
				/* start tist */
				SK_Y2_TIST_LE_ENA(pAC-IoBase);
			}
#endif
		}
#endif

#ifdef Y2_LE_CHECK
		/* mark entries invalid */
		pAC->LastPort = 3;
		pAC->LastOpc = 0xFF;
#endif
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2PortStart(pAC, IoC, FromPort);
		} else {
			if (SkGeInitPort(pAC, IoC, FromPort)) {
				if (FromPort == 0) {
					printk("%s: SkGeInitPort A failed.\n", pAC->dev[0]->name);
				} else {
					printk("%s: SkGeInitPort B failed.\n", pAC->dev[1]->name);
				}
			}
			SkAddrMcUpdate(pAC,IoC, FromPort);
			PortReInitBmu(pAC, FromPort);
			SkGePollTxD(pAC, IoC, FromPort, SK_TRUE);
			CLEAR_AND_START_RX(FromPort);
		}
		spin_unlock_irqrestore(
			&pAC->TxPort[FromPort].TxDesRingLock,
			Flags);
		break;
	case SK_DRV_LINK_UP:	/* From SIRQ. */
		spin_lock_irqsave(&pAC->InitLock, Flags);
		FromPort = Param.Para32[0];

		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("LINK UP EVENT, Port: %d ", FromPort));

#ifdef MV_INCLUDE_SDK_SUPPORT
#ifdef MV_INCLUDE_FW_SDK_LINK_MAINTENANCE
		FwCheckLinkState(pAC, pAC->IoBase, FromPort, &(pAC->DriverLinkStat));
#endif
#endif
		/* MAC update */
		SkAddrMcUpdate(pAC, IoC, FromPort);
		if (pAC->dev[FromPort]->flags & IFF_PROMISC) {
			SkAddrPromiscuousChange(pAC, IoC, FromPort,
				SK_PROM_MODE_LLC);
		} else if (pAC->dev[FromPort]->flags & IFF_ALLMULTI) {
			SkAddrPromiscuousChange(pAC, IoC, FromPort,
				SK_PROM_MODE_ALL_MC);
		}

		Para.Para32[0] = FromPort;
		Para.Para32[1] = 0;
		SkPnmiEvent(pAC, pAC->IoBase, SK_PNMI_EVT_RLMT_ACTIVE_UP, Para);

		if (DoPrintInterfaceChange) {
			printk("%s: network connection up using port %c\n",
				pAC->dev[FromPort]->name, 'A'+FromPort);

			if (pAC->AllocFlag & SK_ALLOC_MSI) {
				printk("    interrupt src:   MSI\n");
			} else {
				printk("    interrupt src:   INTx\n");
			}

			Stat = pAC->GIni.GP[FromPort].PLinkSpeedUsed;
			if (Stat == SK_LSPEED_STAT_10MBPS) {
				printk("    speed:           10\n");
			} else if (Stat == SK_LSPEED_STAT_100MBPS) {
				printk("    speed:           100\n");
			} else if (Stat == SK_LSPEED_STAT_1000MBPS) {
				printk("    speed:           1000\n");
			} else {
				printk("    speed:           unknown\n");
			}

			Stat = pAC->GIni.GP[FromPort].PLinkModeStatus;
			if ((Stat == SK_LMODE_STAT_AUTOHALF) ||
			    (Stat == SK_LMODE_STAT_AUTOFULL)) {
				printk("    autonegotiation: yes\n");
			} else {
				printk("    autonegotiation: no\n");
			}

			if ((Stat == SK_LMODE_STAT_AUTOHALF) ||
			    (Stat == SK_LMODE_STAT_HALF)) {
				printk("    duplex mode:     half\n");
			} else {
				printk("    duplex mode:     full\n");
			}

			Stat = pAC->GIni.GP[FromPort].PFlowCtrlStatus;
			if (Stat == SK_FLOW_STAT_REM_SEND ) {
				printk("    flowctrl:        remote send\n");
			} else if (Stat == SK_FLOW_STAT_LOC_SEND ) {
				printk("    flowctrl:        local send\n");
			} else if (Stat == SK_FLOW_STAT_SYMMETRIC ) {
				printk("    flowctrl:        symmetric\n");
			} else {
				printk("    flowctrl:        none\n");
			}

			if ((pAC->GIni.GICopperType == SK_TRUE) &&
				(pAC->GIni.GP[FromPort].PLinkSpeedUsed ==
				SK_LSPEED_STAT_1000MBPS)) {
				Stat = pAC->GIni.GP[FromPort].PMSStatus;
				if (Stat == SK_MS_STAT_MASTER ) {
					printk("    role:            master\n");
				} else if (Stat == SK_MS_STAT_SLAVE ) {
					printk("    role:            slave\n");
				} else {
					printk("    role:            ???\n");
				}
			}

			/* Display interrupt moderation informations */
			if (pAC->DynIrqModInfo.IntModTypeSelect == C_INT_MOD_STATIC) {
				printk("    irq moderation:  static (%d ints/sec)\n",
					pAC->DynIrqModInfo.MaxModIntsPerSec);
			} else if (pAC->DynIrqModInfo.IntModTypeSelect == C_INT_MOD_DYNAMIC) {
				printk("    irq moderation:  dynamic (%d ints/sec)\n",
					pAC->DynIrqModInfo.MaxModIntsPerSec);
			}

#ifdef NETIF_F_TSO
			if (CHIP_ID_YUKON_2(pAC)) {
				if (pAC->dev[FromPort]->features & NETIF_F_TSO) {
					printk("    tcp offload:     enabled\n");
				} else {
					printk("    tcp offload:     disabled\n");
				}
			}
#endif

			if (pAC->dev[FromPort]->features & NETIF_F_SG) {
				printk("    scatter-gather:  enabled\n");
			} else {
				printk("    scatter-gather:  disabled\n");
			}

			if (pAC->dev[FromPort]->features & NETIF_F_IP_CSUM) {
				printk("    tx-checksum:     enabled\n");
			} else {
				printk("    tx-checksum:     disabled\n");
			}

			if (pAC->RxPort[FromPort].UseRxCsum) {
				printk("    rx-checksum:     enabled\n");
			} else {
				printk("    rx-checksum:     disabled\n");
			}
#ifdef CONFIG_SK98LIN_NAPI
				printk("    rx-polling:      enabled\n");
#endif
			if (pAC->TxModeration) {
				printk("    tx moderation:   %d\n",
					pAC->TxModeration);
			}

			if (pAC->LowLatency) {
				printk("    low latency:     enabled\n");
			}
#ifdef MV_INCLUDE_SDK_SUPPORT
			if (pAC->FwRun) {
				printk("    SDK:             enabled\n");
				if (pAC->SdkQH) {
					printk("    SDK queue  :     enabled\n");
				}
			}
#endif

#ifdef USE_SK_RSS_SUPPORT
			if (pAC->dev[FromPort]->features & NETIF_F_RXHASH) {
				printk("    RSS:             enabled\n");
			}
#endif
		} else {
			DoPrintInterfaceChange = SK_TRUE;
		}

		/* Inform the world that link protocol is up. */
		Stat = pAC->GIni.GP[FromPort].PLinkSpeedUsed;
		if ( (Stat == SK_LSPEED_STAT_10MBPS) ||
		     (Stat == SK_LSPEED_STAT_100MBPS) ||
		     (Stat == SK_LSPEED_STAT_1000MBPS) ) {

			/* Don't enable the queue if SDK queue handling */
#ifdef MV_INCLUDE_SDK_SUPPORT
			if (!pAC->SdkQH) {
				NETIF_WAKE_ALLQ(pAC->dev[FromPort]);
			}
#else
			NETIF_WAKE_ALLQ(pAC->dev[FromPort]);
#endif
			netif_carrier_on(pAC->dev[FromPort]);

			pAC->dev[FromPort]->flags |= IFF_RUNNING;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			pNet = (DEV_NET *) pAC->dev[FromPort]->priv;
#else
			pNet = netdev_priv(pAC->dev[FromPort]);
#endif
#ifdef Y2_RECOVERY
			if (!timer_pending(&pNet->KernelTimer)) {
				pNet->TimerExpired = SK_FALSE;
				SkGeCheckTimer(pNet);
			}
#endif
		}

		/* Map any waiting RX buffers to HW */
		FillReceiveTableYukon2(pAC, pAC->IoBase, FromPort);

		spin_unlock_irqrestore(&pAC->InitLock, Flags);
		break;
	case SK_DRV_LINK_DOWN:	/* From SIRQ. */
		if (pAC->RlmtNets == 2) {
			FromPort = Param.Para32[0];
		}
		else {
			FromPort = pAC->ActivePort;
		}

		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("LINK DOWN EVENT "));

		/* Stop queue and carrier */
		NETIF_STOP_ALLQ(pAC->dev[FromPort]);
		netif_carrier_off(pAC->dev[FromPort]);

		Para.Para32[0] = FromPort;
		Para.Para32[1] = 0;
		SkPnmiEvent(pAC, pAC->IoBase, SK_PNMI_EVT_RLMT_ACTIVE_DOWN, Para);

		/* Print link change */
		if (DoPrintInterfaceChange) {
			if (pAC->dev[FromPort]->flags & IFF_RUNNING) {
				printk("%s: network connection down\n",
					pAC->dev[FromPort]->name);
			}
		} else {
			DoPrintInterfaceChange = SK_TRUE;
		}
		pAC->dev[FromPort]->flags &= ~IFF_RUNNING;
		break;
	case SK_DRV_TIMER:
		if (Param.Para32[0] == SK_DRV_MODERATION_TIMER) {
			/* check what IRQs are to be moderated */
			SkDimStartModerationTimer(pAC);
			SkDimModerate(pAC);
		} else {
			printk("Expiration of unknown timer\n");
		}
		break;
	case SK_DRV_ADAP_FAIL:
#if (!defined (Y2_RECOVERY) && !defined (Y2_LE_CHECK))
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("ADAPTER FAIL EVENT\n"));
		printk("%s: Adapter failed.\n", pAC->dev[0]->name);
		SK_OUT32(pAC->IoBase, B0_IMSK, 0); /* disable interrupts */
		break;
#endif

#if (defined (Y2_RECOVERY) || defined (Y2_LE_CHECK))
	case SK_DRV_RECOVER:
		FromPort = Param.Para32[0];
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		pNet = (DEV_NET *) pAC->dev[FromPort]->priv;
#else
		pNet = netdev_priv(pAC->dev[FromPort]);
#endif

#ifdef Y2_RECOVERY
		/* Recover already in progress */
		if (pNet->InRecover) {
			break;
		}
#endif
		SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
			("DRV RECOVER EVENT, Port: %d ", FromPort));

		spin_lock_irqsave(&pAC->InitLock, Flags);

		NETIF_STOP_ALLQ(pAC->dev[FromPort]); /* stop device if running */
		pAC->dev[FromPort]->flags &= ~IFF_RUNNING;

#ifdef Y2_RECOVERY
		pNet->InRecover = SK_TRUE;
#endif
		/* Disable interrupts */
		SK_OUT32(pAC->IoBase, B0_IMSK, 0);
		SK_OUT32(pAC->IoBase, B0_HWE_IMSK, 0);

		SkLocalEventQueue64(pAC, SKGE_PNMI, SK_PNMI_EVT_XMAC_RESET,
					FromPort, SK_FALSE);

		/* Disable RX/TX */
		SkMacRxTxDisable(pAC, IoC, FromPort);

		/* Disable the status unit */
		if (HW_IS_EXT_LE_FORMAT(pAC)) {
			SK_OUT8(pAC->IoBase, STAT_CTRL, SC_STAT_RST_SET);
			SK_MEMSET(pAC->pVirtMemAddr, 0, pAC->SizeOfAlignedLETables);
		}

		spin_lock(&pAC->TxPort[FromPort].TxDesRingLock);
		if (CHIP_ID_YUKON_2(pAC)) {
			SkY2PortStop(pAC, IoC, FromPort, SK_STOP_ALL, SK_SOFT_RST);
		} else {
			SkGeStopPort(pAC, IoC, FromPort, SK_STOP_ALL, SK_SOFT_RST);
		}
		spin_unlock(&pAC->TxPort[FromPort].TxDesRingLock);

		if (!CHIP_ID_YUKON_2(pAC)) {
#ifdef CONFIG_SK98LIN_NAPI
			WorkToDo = 1;
			ReceiveIrq(pAC, &pAC->RxPort[FromPort], SK_FALSE, &WorkDone, WorkToDo);
#else
			ReceiveIrq(pAC, &pAC->RxPort[FromPort], SK_FALSE);
#endif
			ClearTxRing(pAC, &pAC->TxPort[FromPort]);
		}
		spin_lock(&pAC->TxPort[FromPort].TxDesRingLock);

#ifdef USE_TIST_FOR_RESET
		if (!HW_IS_EXT_LE_FORMAT(pAC) && pAC->GIni.GIYukon2) {
			SK_SET_WAIT_BIT_FOR_PORT(
				pAC,
				SK_PSTATE_WAITING_FOR_ANY_TIST,
				FromPort);

			/* start tist */
			SK_Y2_TIST_LE_ENA(pAC->IoBase);
		}
#endif

		/* Restart Receive BMU on Yukon-2 */
		if (HW_FEATURE(pAC, HWF_WA_DEV_4167)) {
			SkYuk2RestartRxBmu(pAC, IoC, FromPort);
		}

#ifdef Y2_LE_CHECK
		/* mark entries invalid */
		pAC->LastPort = 3;
		pAC->LastOpc = 0xFF;
#endif

#endif
		/* Restart ports but do not initialize PHY. */
		if (CHIP_ID_YUKON_2(pAC)) {
			/* Enable the status unit */
			if (HW_IS_EXT_LE_FORMAT(pAC)) {
				pAC->StatusLETable.Done  = 0;
				pAC->StatusLETable.Put   = 0;
				pAC->StatusLETable.HwPut = 0;
				SkGeY2InitStatBmu(pAC, pAC->IoBase, &pAC->StatusLETable);
			}
			SkY2PortStart(pAC, IoC, FromPort);
		} else {
			if (SkGeInitPort(pAC, IoC, FromPort)) {
				if (FromPort == 0) {
					printk("%s: SkGeInitPort A failed.\n", pAC->dev[0]->name);
				} else {
					printk("%s: SkGeInitPort B failed.\n", pAC->dev[1]->name);
				}
			}
			SkAddrMcUpdate(pAC,IoC, FromPort);
			PortReInitBmu(pAC, FromPort);
			SkGePollTxD(pAC, IoC, FromPort, SK_TRUE);
			CLEAR_AND_START_RX(FromPort);
		}
		spin_unlock(&pAC->TxPort[FromPort].TxDesRingLock);

		/* Map any waiting RX buffers to HW */
#ifdef USE_SK_RSS_SUPPORT
		SkSetRssSupport(pAC, FromPort);
#endif
		FillReceiveTableYukon2(pAC, pAC->IoBase, FromPort);

#ifdef Y2_RECOVERY
		pNet->InRecover = SK_FALSE;
#endif
		/* Enable Interrupts */
		SK_OUT32(pAC->IoBase, B0_IMSK, pAC->GIni.GIValIrqMask);
		if (CHIP_ID_YUKON_2(pAC)) {
			SK_OUT32(pAC->IoBase, B0_HWE_IMSK, Y2_IRQ_HWE_MASK);
		}
		else {
			SK_OUT32(pAC->IoBase, B0_HWE_IMSK, IRQ_HWE_MASK);
		}
		NETIF_WAKE_ALLQ(pAC->dev[FromPort]);
		pAC->dev[FromPort]->flags |= IFF_RUNNING;

		spin_unlock_irqrestore(&pAC->InitLock, Flags);
		break;
	default:
		break;
	}
	SK_DBG_MSG(NULL, SK_DBGMOD_DRV, SK_DBGCAT_DRV_EVENT,
		("END EVENT "));

	return (0);
} /* SkDrvEvent */


/******************************************************************************
 *
 *	SkLocalEventQueue()	-	add event to queue
 *
 * Description:
 *	This function adds an event to the event queue and run the
 *	SkEventDispatcher. At least Init Level 1 is required to queue events,
 *	but will be scheduled add Init Level 2.
 *
 * returns:
 *	nothing
 */
void SkLocalEventQueue(
SK_AC *pAC,		/* Adapters context */
SK_U32 Class,		/* Event Class */
SK_U32 Event,		/* Event to be queued */
SK_U32 Param1,		/* Event parameter 1 */
SK_U32 Param2,		/* Event parameter 2 */
SK_BOOL Dispatcher)	/* Dispatcher flag:
			 *	TRUE == Call SkEventDispatcher
			 *	FALSE == Don't execute SkEventDispatcher
			 */
{
	SK_EVPARA 	EvPara;
	EvPara.Para32[0] = Param1;
	EvPara.Para32[1] = Param2;

	/* Don't queue events if the device is not activated */
	if(pAC->BoardLevel != SK_INIT_RUN) {
		return;
	}

	if (Class == SKGE_PNMI) {
		SkPnmiEvent(	pAC,
				pAC->IoBase,
				Event,
				EvPara);
	} else {
		SkEventQueue(	pAC,
				Class,
				Event,
				EvPara);
	}

	/* Run the dispatcher */
	if (Dispatcher) {
		SkEventDispatcher(pAC, pAC->IoBase);
	}

}

/******************************************************************************
 *
 *	SkLocalEventQueue64()	-	add event to queue (64bit version)
 *
 * Description:
 *	This function adds an event to the event queue and run the
 *	SkEventDispatcher. At least Init Level 1 is required to queue events,
 *	but will be scheduled add Init Level 2.
 *
 * returns:
 *	nothing
 */
void SkLocalEventQueue64(
SK_AC *pAC,		/* Adapters context */
SK_U32 Class,		/* Event Class */
SK_U32 Event,		/* Event to be queued */
SK_U64 Param,		/* Event parameter */
SK_BOOL Dispatcher)	/* Dispatcher flag:
			 *	TRUE == Call SkEventDispatcher
			 *	FALSE == Don't execute SkEventDispatcher
			 */
{
	SK_EVPARA 	EvPara;
	EvPara.Para64 = Param;

	/* Don't queue events if the device is not activated */
	if(pAC->BoardLevel != SK_INIT_RUN) {
		return;
	}

	if (Class == SKGE_PNMI) {
		SkPnmiEvent(	pAC,
				pAC->IoBase,
				Event,
				EvPara);
	} else {
		SkEventQueue(	pAC,
				Class,
				Event,
				EvPara);
	}

	/* Run the dispatcher */
	if (Dispatcher) {
		SkEventDispatcher(pAC, pAC->IoBase);
	}

}


/*****************************************************************************
 *
 *	SkErrorLog - log errors
 *
 * Description:
 *	This function logs errors to the system buffer and to the console
 *
 * Returns:
 *	0 if everything ok
 *	< 0  on error
 *
 */
void SkErrorLog(
SK_AC	*pAC,
int	ErrClass,
int	ErrNum,
char	*pErrorMsg)
{
char	ClassStr[80];

	switch (ErrClass) {
	case SK_ERRCL_OTHER:
		strcpy(ClassStr, "Other error");
		break;
	case SK_ERRCL_CONFIG:
		strcpy(ClassStr, "Configuration error");
		break;
	case SK_ERRCL_INIT:
		strcpy(ClassStr, "Initialization error");
		break;
	case SK_ERRCL_NORES:
		strcpy(ClassStr, "Out of resources error");
		break;
	case SK_ERRCL_SW:
		strcpy(ClassStr, "internal Software error");
		break;
	case SK_ERRCL_HW:
		strcpy(ClassStr, "Hardware failure");
		break;
	case SK_ERRCL_COMM:
		strcpy(ClassStr, "Communication error");
		break;
	case SK_ERRCL_INFO:
		strcpy(ClassStr, "Information");
		break;
	}

	if (ErrClass == SK_ERRCL_INFO) {
		printk(KERN_INFO "%s: -- INFORMATION --\n"
			"        Msg:  %s\n", pAC->dev[0]->name,
			pErrorMsg);
	} else {
		printk(KERN_INFO "%s: -- ERROR --\n        Class:  %s\n"
			"        Nr:  0x%x\n        Msg:  %s\n", pAC->dev[0]->name,
			ClassStr, ErrNum, pErrorMsg);
	}

} /* SkErrorLog */

/*****************************************************************************
 *
 *	SkDrvEnterDiagMode - handles DIAG attach request
 *
 * Description:
 *	Notify the kernel to NOT access the card any longer due to DIAG
 *	Deinitialize the Card
 *
 * Returns:
 *	int
 */
int SkDrvEnterDiagMode(
SK_AC   *pAc)   /* pointer to adapter context */
{
	SK_AC   *pAC  = NULL;
	DEV_NET *pNet = NULL;

	#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		pNet = (DEV_NET *) pAc->dev[0]->priv;
	#else
		pNet = netdev_priv(pAc->dev[0]);
#endif
	pAC = pNet->pAC;

	SK_MEMCPY(&(pAc->PnmiBackup), &(pAc->PnmiStruct),
			sizeof(SK_PNMI_STRUCT_DATA));

	pAC->DiagModeActive = DIAG_ACTIVE;
	if (pAC->BoardLevel > SK_INIT_DATA) {
		if (netif_running(pAC->dev[0])) {
			pAC->WasIfUp[0] = SK_TRUE;
			pAC->DiagFlowCtrl = SK_TRUE; /* for SkGeClose      */
			DoPrintInterfaceChange = SK_FALSE;
			SkDrvDeInitAdapter(pAC, 0);  /* performs SkGeClose */
		} else {
			pAC->WasIfUp[0] = SK_FALSE;
		}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
			if (pNet != (DEV_NET *) pAc->dev[1]->priv) {
				pNet = (DEV_NET *) pAc->dev[1]->priv;
#else
			if (pNet != netdev_priv(pAc->dev[1])) {
				pNet = netdev_priv(pAc->dev[1]);
#endif
			if (netif_running(pAC->dev[1])) {
				pAC->WasIfUp[1] = SK_TRUE;
				pAC->DiagFlowCtrl = SK_TRUE; /* for SkGeClose */
				DoPrintInterfaceChange = SK_FALSE;
				SkDrvDeInitAdapter(pAC, 1);  /* do SkGeClose  */
			} else {
				pAC->WasIfUp[1] = SK_FALSE;
			}
		}
		pAC->BoardLevel = SK_INIT_DATA;
	}
	return(0);
}

/*****************************************************************************
 *
 *	SkDrvLeaveDiagMode - handles DIAG detach request
 *
 * Description:
 *	Notify the kernel to may access the card again after use by DIAG
 *	Initialize the Card
 *
 * Returns:
 * 	int
 */
int SkDrvLeaveDiagMode(
SK_AC   *pAc)   /* pointer to adapter control context */
{
	SK_MEMCPY(&(pAc->PnmiStruct), &(pAc->PnmiBackup),
			sizeof(SK_PNMI_STRUCT_DATA));
	pAc->DiagModeActive    = DIAG_NOTACTIVE;
	pAc->Pnmi.DiagAttached = SK_DIAG_IDLE;
	if (pAc->WasIfUp[0] == SK_TRUE) {
		pAc->DiagFlowCtrl = SK_TRUE; /* for SkGeClose */
		DoPrintInterfaceChange = SK_FALSE;
		SkDrvInitAdapter(pAc, 0);    /* first device  */
	}
	if (pAc->WasIfUp[1] == SK_TRUE) {
		pAc->DiagFlowCtrl = SK_TRUE; /* for SkGeClose */
		DoPrintInterfaceChange = SK_FALSE;
		SkDrvInitAdapter(pAc, 1);    /* second device */
	}
	return(0);
}

/*****************************************************************************
 *
 *	ParseDeviceNbrFromSlotName - Evaluate PCI device number
 *
 * Description:
 * 	This function parses the PCI slot name information string and will
 *	retrieve the devcie number out of it. The slot_name maintianed by
 *	linux is in the form of '02:0a.0', whereas the first two characters
 *	represent the bus number in hex (in the sample above this is
 *	pci bus 0x02) and the next two characters the device number (0x0a).
 *
 * Returns:
 *	SK_U32: The device number from the PCI slot name
 */

static SK_U32 ParseDeviceNbrFromSlotName(
const char *SlotName)   /* pointer to pci slot name eg. '02:0a.0' */
{
	char	*CurrCharPos	= (char *) SlotName;
	int	FirstNibble	= -1;
	int	SecondNibble	= -1;
	SK_U32	Result		=  0;

	while (*CurrCharPos != '\0') {
		if (*CurrCharPos == ':') {
			while (*CurrCharPos != '.') {
				CurrCharPos++;
				if (	(*CurrCharPos >= '0') &&
					(*CurrCharPos <= '9')) {
					if (FirstNibble == -1) {
						/* dec. value for '0' */
						FirstNibble = *CurrCharPos - 48;
					} else {
						SecondNibble = *CurrCharPos - 48;
					}
				} else if (	(*CurrCharPos >= 'a') &&
						(*CurrCharPos <= 'f')  ) {
					if (FirstNibble == -1) {
						FirstNibble = *CurrCharPos - 87;
					} else {
						SecondNibble = *CurrCharPos - 87;
					}
				} else {
					Result = 0;
				}
			}

			Result = FirstNibble;
			Result = Result << 4; /* first nibble is higher one */
			Result = Result | SecondNibble;
		}
		CurrCharPos++;   /* next character */
	}
	return (Result);
}

/****************************************************************************
 *
 *	SkDrvDeInitAdapter - deinitialize adapter (this function is only
 *				called if Diag attaches to that card)
 *
 * Description:
 *	Close initialized adapter.
 *
 * Returns:
 *	0 - on success
 *	error code - on error
 */
static int SkDrvDeInitAdapter(
SK_AC   *pAC,		/* pointer to adapter context   */
int      devNbr)	/* what device is to be handled */
{
	struct SK_NET_DEVICE *dev;

	dev = pAC->dev[devNbr];

	/*
	** Function SkGeClose() uses MOD_DEC_USE_COUNT (2.2/2.4)
	** or module_put() (2.6) to decrease the number of users for
	** a device, but if a device is to be put under control of
	** the DIAG, that count is OK already and does not need to
	** be adapted! Hence the opposite MOD_INC_USE_COUNT or
	** try_module_get() needs to be used again to correct that.
	*/
	if (!try_module_get(THIS_MODULE)) {
		return (-1);
	}

	if (SkGeClose(dev) != 0) {
		module_put(THIS_MODULE);
		return (-1);
	}
	return (0);

} /* SkDrvDeInitAdapter() */

/****************************************************************************
 *
 *	SkDrvInitAdapter - Initialize adapter (this function is only
 *				called if Diag deattaches from that card)
 *
 * Description:
 *	Close initialized adapter.
 *
 * Returns:
 *	0 - on success
 *	error code - on error
 */
static int SkDrvInitAdapter(
SK_AC   *pAC,		/* pointer to adapter context   */
int      devNbr)	/* what device is to be handled */
{
	struct SK_NET_DEVICE *dev;

	dev = pAC->dev[devNbr];

	if (SkGeOpen(dev) != 0) {
		return (-1);
	} else {
		/*
		** Function SkGeOpen() uses MOD_INC_USE_COUNT (2.2/2.4)
		** or try_module_get() (2.6) to increase the number of
		** users for a device, but if a device was just under
		** control of the DIAG, that count is OK already and
		** does not need to be adapted! Hence the opposite
		** MOD_DEC_USE_COUNT or module_put() needs to be used
		** again to correct that.
		*/
		module_put(THIS_MODULE);
	}

#ifdef MV_INCLUDE_SDK_SUPPORT
	/*
	** Use correct MTU size and indicate to kernel TX queue can be started
	*/
	if (pAC->FwRun && pAC->Suspended == SK_FALSE) {
		if (SkGeChangeMtu(dev, dev->mtu) != 0) {
			return (-1);
		}
	}
#endif
	return (0);

} /* SkDrvInitAdapter */

#ifdef SK_AVB
/*****************************************************************************
 *
 *	SkSelectQueue
 *
 * Description:
 *	Selects HW queue for transmit
 *
 * Returns: N/A
 *
 */
static u16 SkSelectQueue(struct net_device *dev, struct sk_buff *skb)
{
	DEV_NET	*pNet = PPRIV;
	SK_AC	*pAC = pNet->pAC;
	u16		q = pAC->DefaultTxQ;

	q = mv_select_avb_queue(pAC, skb);

	if (q >= pAC->NumTxQueues) {
		q = pAC->DefaultTxQ;
	}

	return q;
}
#endif

static int __init sk98lin_init(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
	return pci_register_driver(&sk98lin_driver);
#else
	return pci_module_init(&sk98lin_driver);
#endif
}

static void __exit sk98lin_cleanup(void)
{
	pci_unregister_driver(&sk98lin_driver);
}

module_init(sk98lin_init);
module_exit(sk98lin_cleanup);


#ifdef DEBUG
/****************************************************************************/
/* "debug only" section *****************************************************/
/****************************************************************************/

/*****************************************************************************
 *
 *	DumpMsg - print a frame
 *
 * Description:
 *	This function prints frames to the system logfile/to the console.
 *
 * Returns: N/A
 *
 */
void DumpMsg(
struct sk_buff *skb,  /* linux' socket buffer  */
char           *str)  /* additional msg string */
{
	int msglen = (skb->len > 64) ? 64 : skb->len;

	if (skb == NULL) {
		printk("DumpMsg(): NULL-Message\n");
		return;
	}

	if (skb->data == NULL) {
		printk("DumpMsg(): Message empty\n");
		return;
	}

	printk("DumpMsg: PhysPage: %p\n",
		page_address(virt_to_page(skb->data)));
	printk("--- Begin of message from %s , len %d (from %d) ----\n",
		str, msglen, skb->len);
	DumpData((char *)skb->data, msglen);
	printk("------- End of message ---------\n");
} /* DumpMsg */

/*****************************************************************************
 *
 *	DumpData - print a data area
 *
 * Description:
 *	This function prints a area of data to the system logfile/to the
 *	console.
 *
 * Returns: N/A
 *
 */
static void DumpData(
char  *p,     /* pointer to area containing the data */
int    size)  /* the size of that data area in bytes */
{
	register int  i;
	int           haddr = 0, addr = 0;
	char          hex_buffer[180] = { '\0' };
	char          asc_buffer[180] = { '\0' };
	char          HEXCHAR[] = "0123456789ABCDEF";

	for (i=0; i < size; ) {
		if (*p >= '0' && *p <='z') {
			asc_buffer[addr] = *p;
		} else {
			asc_buffer[addr] = '.';
		}
		addr++;
		asc_buffer[addr] = 0;
		hex_buffer[haddr] = HEXCHAR[(*p & 0xf0) >> 4];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[*p & 0x0f];
		haddr++;
		hex_buffer[haddr] = ' ';
		haddr++;
		hex_buffer[haddr] = 0;
		p++;
		i++;
		if (i%16 == 0) {
			printk("%s  %s\n", hex_buffer, asc_buffer);
			addr = 0;
			haddr = 0;
		}
	}
} /* DumpData */


/*****************************************************************************
 *
 *	DumpLong - print a data area as long values
 *
 * Description:
 *	This function prints a long variable to the system logfile/to the
 *	console.
 *
 * Returns: N/A
 *
 */
static void DumpLong(
char  *pc,    /* location of the variable to print */
int    size)  /* how large is the variable?        */
{
	register int   i;
	int            haddr = 0;
	char           hex_buffer[180] = { '\0' };
	char           HEXCHAR[] = "0123456789ABCDEF";
	long          *p = (long*) pc;
	int            l;

	for (i=0; i < size; ) {
		l = (long) *p;
		hex_buffer[haddr] = HEXCHAR[(l >> 28) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 24) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 20) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 16) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 12) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 8) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[(l >> 4) & 0xf];
		haddr++;
		hex_buffer[haddr] = HEXCHAR[l & 0x0f];
		haddr++;
		hex_buffer[haddr] = ' ';
		haddr++;
		hex_buffer[haddr] = 0;
		p++;
		i++;
		if (i%8 == 0) {
			printk("%4x %s\n", (i-8)*4, hex_buffer);
			haddr = 0;
		}
	}
	printk("------------------------\n");
} /* DumpLong */

#endif

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/

