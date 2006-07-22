/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software may be redistributed and/or modified under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 *
 * File: rhine_main.c
 *
 * Purpose: Functions for Linux drver interfaces.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#undef __NO_VERSION__

#include "rhine.h"

static int          rhine_nics              =0;
static PRHINE_INFO  pRhine3_Infos           =NULL;
static int          msglevel                =MSG_LEVEL_INFO;

#ifdef  RHINE_ETHTOOL_IOCTL_SUPPORT
static int  rhine_ethtool_ioctl(struct net_device* dev, struct ifreq* ifr);
#endif

#ifdef SIOCGMIIPHY
static int  rhine_mii_ioctl(struct net_device* dev, struct ifreq* ifr, int cmd);
#endif


/*
    Define module options
*/

MODULE_AUTHOR("VIA Technologies, Inc.");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("VIA Rhine Family Fast Ethernet Adapter Driver");

#define RHINE_PARAM(N,D) static const int N[MAX_UINTS]=OPTION_DEFAULT; module_parm(N,int[MAX_UINTS],0); MODULE_PARM_DESC(N, D);

#define RX_DESC_MIN     64
#define RX_DESC_MAX     128
#define RX_DESC_DEF     64
RHINE_PARAM(RxDescriptors,"Number of receive descriptors");

#define TX_DESC_MIN     16
#define TX_DESC_MAX     128
#define TX_DESC_DEF     64
RHINE_PARAM(TxDescriptors,"Number of transmit descriptors");

#define VLAN_ID_MIN     0
#define VLAN_ID_MAX     4094
#define VLAN_ID_DEF     0
/* VID_setting[] is used for setting the VID of NIC.
   0: default VID.
   1-4094: other VIDs.
*/
RHINE_PARAM(VID_setting,"802.1Q VLAN ID");

#define TX_THRESH_MIN   0
#define TX_THRESH_MAX   4
#define TX_THRESH_DEF   0
/* tx_thresh[] is used for controlling the transmit fifo threshold.
   0: indicate the txfifo threshold is 128 bytes.
   1: indicate the txfifo threshold is 256 bytes.
   2: indicate the txfifo threshold is 512 bytes.
   3: indicate the txfifo threshold is 1024 bytes.
   4: indicate that we use store and forward
*/
RHINE_PARAM(tx_thresh,"Transmit fifo threshold");

#define RX_THRESH_MIN   0
#define RX_THRESH_MAX   7
#define RX_THRESH_DEF   0
/* rx_thresh[] is used for controlling the receive fifo threshold.
   0: indicate the rxfifo threshold is 64 bytes.
   1: indicate the rxfifo threshold is 32 bytes.
   2: indicate the rxfifo threshold is 128 bytes.
   3: indicate the rxfifo threshold is 256 bytes.
   4: indicate the rxfifo threshold is 512 bytes.
   5: indicate the rxfifo threshold is 768 bytes.
   6: indicate the rxfifo threshold is 1024 bytes.
   7: indicate that we use store and forward
*/
RHINE_PARAM(rx_thresh,"Receive fifo threshold");

#define DMA_LENGTH_MIN  0
#define DMA_LENGTH_MAX  7
#define DMA_LENGTH_DEF  0
/* DMA_length[] is used for controlling the DMA length
   0: 8 DWORDs
   1: 16 DWORDs
   2: 32 DWORDs
   3: 64 DWORDs
   4: 128 DWORDs
   5: 256 DWORDs
   6: SF(flush till emply)
   7: SF(flush till emply)
*/
RHINE_PARAM(DMA_length,"DMA length");

#define TAGGING_DEF     0
/* enable_tagging[] is used for enabling 802.1Q VID tagging.
   0: disable VID seeting(default).
   1: enable VID setting.
*/
RHINE_PARAM(enable_tagging,"Enable 802.1Q tagging");

#define IP_ALIG_DEF     0
/* IP_byte_align[] is used for IP header DWORD byte aligned
   0: indicate the IP header won't be DWORD byte aligned.(Default) .
   1: indicate the IP header will be DWORD byte aligned.
      In some enviroment, the IP header should be DWORD byte aligned,
      or the packet will be droped when we receive it. (eg: IPVS)
*/
RHINE_PARAM(IP_byte_align,"Enable IP header dword aligned");

#define RX_CSUM_DEF     1
/* rxcsum_offload[] is used for setting the receive checksum offload ability
   of NIC.
   0: disable
   1: enable (Default)
*/
RHINE_PARAM(rxcsum_offload,"Enable receive packet checksum offload");

#ifdef RHINE_TX_CSUM_SUPPORT
#define TX_CSUM_DEF     0
/* txcsum_offload[] is used for setting the transmit checksum offload ability
   of NIC.
   0: disable (Default)
   1: enable
*/
RHINE_PARAM(txcsum_offload,"Enable transmit packet checksum offload");
#endif

#define FLOW_CNTL_DEF   2
#define FLOW_CNTL_MIN   1
#define FLOW_CNTL_MAX   3
/* flow_control[] is used for setting the flow control ability of NIC.
   1: hardware deafult(default). Use Hardware default value in ANAR.
   2: disable PAUSE in ANAR.
   3: enable PAUSE in ANAR.
*/
RHINE_PARAM(flow_control,"Enable flow control ability");

#define MED_LNK_DEF 0
#define MED_LNK_MIN 0
#define MED_LNK_MAX 4
/* speed_duplex[] is used for setting the speed and duplex mode of NIC.
   0: indicate autonegotiation for both speed and duplex mode
   1: indicate 100Mbps half duplex mode
   2: indicate 100Mbps full duplex mode
   3: indicate 10Mbps half duplex mode
   4: indicate 10Mbps full duplex mode

   Note:
        if EEPROM have been set to the force mode, this option is ignored
            by driver.
*/
RHINE_PARAM(speed_duplex,"Setting the speed and duplex mode");

#define VAL_PKT_LEN_DEF     0
/* ValPktLen[] is used for setting the checksum offload ability of NIC.
   0: Receive frame with invalid layer 2 length (Default)
   1: Drop frame with invalid layer 2 length
*/
RHINE_PARAM(ValPktLen,"Receiving or Drop invalid 802.3 frame");

#define WOL_OPT_DEF     0
#define WOL_OPT_MIN     0
#define WOL_OPT_MAX     7
/* wol_opts[] is used for controlling wake on lan behavior.
   0: Wake up if recevied a magic packet. (Default)
   1: Wake up if link status is on/off.
   2: Wake up if recevied an arp packet.
   4: Wake up if recevied any unicast packet.
   Those value can be sumed up to support more than one option.
*/
RHINE_PARAM(wol_opts,"Wake On Lan options");

#define INT_WORKS_DEF   32
#define INT_WORKS_MIN   10
#define INT_WORKS_MAX   64

RHINE_PARAM(int_works,"Number of packets per interrupt services");

static int  rhine_found1(struct pci_dev *pcid, const struct pci_device_id *ent);
static void rhine_print_info(PRHINE_INFO pInfo);
static int  rhine_open(struct net_device *dev);
static int  rhine_xmit(struct sk_buff *skb, struct net_device *dev);
static irqreturn_t rhine_intr(int irq, void *dev_instance, struct pt_regs *regs);
static void rhine_set_multi(struct net_device *dev);
static struct net_device_stats *rhine_get_stats(struct net_device *dev);
static int  rhine_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static int  rhine_close(struct net_device *dev);
static int  rhine_rx_srv(PRHINE_INFO pInfo, U32 status);
static BOOL rhine_receive_frame(PRHINE_INFO pInfo, int idx);
static BOOL rhine_alloc_rx_buf(PRHINE_INFO pInfo, int idx);
static void rhine_init_adapter(PRHINE_INFO pInfo, RHINE_INIT_TYPE);
static void rhine_init_pci(PRHINE_INFO pInfo);
static void rhine_free_tx_buf(PRHINE_INFO pInfo, int QNo, int idx);
//static void rhine_print_link_status(PRHINE_INFO pInfo);
static void rhine_shutdown(PRHINE_INFO pInfo);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)
#ifdef CONFIG_PM
static int rhine_notify_reboot(struct notifier_block *, unsigned long event, void *ptr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
static int rhine_suspend(struct pci_dev *pcid, pm_message_t state);
#else
static int rhine_suspend(struct pci_dev *pcid, u32 state);
#endif

static int rhine_resume(struct pci_dev *pcid);
struct notifier_block rhine_notifier = {
        notifier_call:  rhine_notify_reboot,
        next:           NULL,
        priority:       0
};

static int
rhine_netdev_event(struct notifier_block *nb,
    unsigned long notification, void *ptr);

static struct notifier_block rhine_inetaddr_notifier = {
    notifier_call: rhine_netdev_event,
    };

#endif
#endif

static CHIP_INFO chip_info_table[]= {
    { VT86C100A,    "VIA VT86C100A Rhine Fast Ethernet Adapter",
        0},
    { VT6102,       "VIA Rhine II Fast Ethernet Adapter",
        0},
    { VT6105,       "VIA Rhine III Fast Ethernet Adapter",
        0},
    { VT6105M,      "VIA Rhine III Management Adapter",
        0},
    {0,NULL}
};

static struct pci_device_id rhine_id_table[] __devinitdata = {
{VENDORID, DEVICEID_3043, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)&chip_info_table[0]},
{VENDORID, DEVICEID_3065, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)&chip_info_table[1]},
{VENDORID, DEVICEID_3106, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)&chip_info_table[2]},
{VENDORID, DEVICEID_3053, PCI_ANY_ID, PCI_ANY_ID, 0, 0, (unsigned long)&chip_info_table[3]},
{0,}
};

static char* get_product_name(int chip_id) {
    int i;
    for (i=0;chip_info_table[i].name!=NULL;i++)
        if (chip_info_table[i].chip_id==chip_id)
            break;
    return chip_info_table[i].name;
}

static void __devexit rhine_remove1(struct pci_dev *pcid)
{
    PRHINE_INFO pInfo = pci_get_drvdata(pcid);
    PRHINE_INFO         ptr;
    struct net_device *dev = pInfo->dev;
    int power_status;       // to silence the compiler

    if (pInfo == NULL)
        return;

    for (ptr=pRhine3_Infos; ptr && (ptr!=pInfo); ptr=ptr->next)
        do {} while (0);

    if (ptr == pInfo) {
        if (ptr == pRhine3_Infos)
            pRhine3_Infos = ptr->next;
        else
            ptr->prev->next = ptr->next;
    }
    else {
        RHINE_PRT(MSG_LEVEL_ERR, KERN_ERR "info struct not found\n");
        return;
    }

    if (pInfo->hw.flags & RHINE_FLAGS_WOL_ENABLED) {
        rhine_get_ip(pInfo);
        rhine_set_wol(pInfo);
        power_status = pci_enable_wake(pcid, PCI_D3hot, 1);
        //pci_set_power_state(pcid, 1);
        power_status = pci_set_power_state(pcid, PCI_D3hot);
    }

#ifdef CONFIG_PROC_FS
    rhine_free_proc_entry(pInfo);
    rhine_free_proc_fs(pInfo);
#endif

    if (dev)
        unregister_netdev(dev);
        
    if (pInfo->hw.hw_addr)
        iounmap(pInfo->hw.hw_addr);

    pci_disable_device(pcid);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    pci_release_regions(pcid);
    free_netdev(dev);
#else
    if (pInfo->hw.ioaddr)
        release_region(pInfo->hw.ioaddr, pInfo->hw.io_size);
    kfree(dev);
#endif

    pci_set_drvdata(pcid, NULL);
}

static void
rhine_set_int_opt(int *opt, int val, int min, int max, int def, char* name) {
    
    if (val==-1)
        *opt=def;
    else if (val<min || val>max) {
        RHINE_PRT(MSG_LEVEL_INFO, KERN_NOTICE "the value of parameter %s is invalid, the valid range is (%d-%d)\n" ,
            name, min, max);
        *opt=def;
    } else {
        RHINE_PRT(MSG_LEVEL_INFO, KERN_INFO "set value of parameter %s to %d\n",
            name, val);
        *opt=val;
    }
}

static void
rhine_set_bool_opt(PU32 opt, int val, BOOL def, U32 flag, char* name) {
    
    (*opt)&=(~flag);
    
    if (val==-1)
        *opt|=(def ? flag : 0);
    else if (val<0 || val>1) {
        printk(KERN_NOTICE
            "the value of parameter %s is invalid, the valid range is (0-1)\n", name);
        *opt|=(def ? flag : 0);
    } else {
        printk(KERN_INFO "set parameter %s to %s\n",
            name , val ? "TRUE" : "FALSE");
        *opt|=(val ? flag : 0);
    }
}

static void
rhine_get_options(POPTIONS pOpts, int index) {

    rhine_set_int_opt(&pOpts->tx_thresh,tx_thresh[index],
        TX_THRESH_MIN, TX_THRESH_MAX, TX_THRESH_DEF,"tx_thresh");

    rhine_set_int_opt(&pOpts->rx_thresh,rx_thresh[index],
        RX_THRESH_MIN, RX_THRESH_MAX, RX_THRESH_DEF,"rx_thresh");

    rhine_set_int_opt(&pOpts->DMA_length,DMA_length[index],
        DMA_LENGTH_MIN, DMA_LENGTH_MAX, DMA_LENGTH_DEF,"DMA_length");

    rhine_set_int_opt(&pOpts->nRxDescs,RxDescriptors[index],
        RX_DESC_MIN, RX_DESC_MAX, RX_DESC_DEF, "RxDescriptors");

    rhine_set_int_opt(&pOpts->nTxDescs,TxDescriptors[index],
        TX_DESC_MIN, TX_DESC_MAX, TX_DESC_DEF, "TxDescriptors");

    rhine_set_int_opt(&pOpts->vid,VID_setting[index],
        VLAN_ID_MIN, VLAN_ID_MAX, VLAN_ID_DEF,"VID_setting");

    rhine_set_bool_opt(&pOpts->flags,enable_tagging[index],
        TAGGING_DEF,RHINE_FLAGS_TAGGING, "enable_tagging");

    rhine_set_bool_opt(&pOpts->flags,rxcsum_offload[index],
        RX_CSUM_DEF,RHINE_FLAGS_RX_CSUM,"rxcsum_offload");

#ifdef RHINE_TX_CSUM_SUPPORT
    rhine_set_bool_opt(&pOpts->flags,txcsum_offload[index],
        TX_CSUM_DEF,RHINE_FLAGS_TX_CSUM,"txcsum_offload");
#endif

    rhine_set_int_opt(&pOpts->flow_cntl,flow_control[index],
        FLOW_CNTL_MIN,FLOW_CNTL_MAX, FLOW_CNTL_DEF, "flow_control");

    rhine_set_bool_opt(&pOpts->flags,IP_byte_align[index],
        IP_ALIG_DEF, RHINE_FLAGS_IP_ALIGN, "IP_byte_align");

    rhine_set_bool_opt(&pOpts->flags,ValPktLen[index],
        VAL_PKT_LEN_DEF, RHINE_FLAGS_VAL_PKT_LEN, "ValPktLen");

    rhine_set_int_opt((int*) &pOpts->spd_dpx,speed_duplex[index],
        MED_LNK_MIN, MED_LNK_MAX, MED_LNK_DEF,"Media link mode");

    rhine_set_int_opt((int*) &pOpts->wol_opts,wol_opts[index],
        WOL_OPT_MIN, WOL_OPT_MAX, WOL_OPT_DEF,"Wake On Lan options");

    rhine_set_int_opt((int*) &pOpts->int_works,int_works[index],
        INT_WORKS_MIN, INT_WORKS_MAX, INT_WORKS_DEF,"Interrupt service works");

}

/*
static void rhine_print_options(PRHINE_INFO pInfo) {
    if (pInfo->flags & RHINE_FLAGS_TAGGING)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: Hardware 802.1Q tagging enabled (VID=%d)\n",
                pInfo->dev->name, pInfo->sOpts.vid);

    if (pInfo->flags & RHINE_FLAGS_TX_CSUM)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: Hardware TX checksum offload enabled\n",
                pInfo->dev->name);

    if (pInfo->flags & RHINE_FLAGS_RX_CSUM)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: Hardware RX checksum offload enabled\n",
                pInfo->dev->name);

    if (pInfo->flags & RHINE_FLAGS_FLOW_CTRL)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: Hardware flow control enabled\n",
                pInfo->dev->name);

    if (pInfo->flags & RHINE_FLAGS_HAVE_CAM)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: Hardware multicast cam enabled\n",
                pInfo->dev->name);

    if (pInfo->flags & RHINE_FLAGS_TX_ALIGN)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: TX buffer dword alignment enabled\n",
                pInfo->dev->name);

    if (pInfo->flags & RHINE_FLAGS_IP_ALIGN)
        RHINE_PRT(MSG_LEVEL_INFO,
            KERN_INFO "%s: OPTION: RX buffer IP header dword alignment enabled\n",
                pInfo->dev->name);

}
*/

void rhine_init_cam_filter(PRHINE_INFO pInfo) {

    BYTE_REG_BITS_SET(&pInfo->hw,(TCR_PQEN),(TCR_RTGOPT|TCR_PQEN),MAC_REG_TCR);
    BYTE_REG_BITS_ON(&pInfo->hw, BCR1_VIDFR,MAC_REG_BCR1);
    //Disable all cam
    rhine_set_cam_mask(&pInfo->hw,0,RHINE_VLAN_ID_CAM);
    rhine_set_cam_mask(&pInfo->hw,0,RHINE_MULTICAST_CAM);
    //Enable first vcam
    if (pInfo->hw.flags & RHINE_FLAGS_TAGGING) {
        rhine_set_cam(&pInfo->hw,0,(PU8) &(pInfo->hw.sOpts.vid),RHINE_VLAN_ID_CAM);
        rhine_set_cam_mask(&pInfo->hw,1,RHINE_VLAN_ID_CAM);
    }
    else {
        U16 wTemp=0;
        rhine_set_cam(&pInfo->hw,0,(PU8) &wTemp,RHINE_VLAN_ID_CAM);
        rhine_set_cam_mask(&pInfo->hw,1,RHINE_VLAN_ID_CAM);
    }
}

//
// Initialiation of adapter
//
static void
rhine_init_adapter(PRHINE_INFO pInfo, RHINE_INIT_TYPE InitType) {
    struct net_device *dev = pInfo->dev;
    int         i;
    U8          byRevId = pInfo->hw.byRevId;
    U32         status;

    rhine_wol_reset(&pInfo->hw);
    switch (InitType) {
    case RHINE_INIT_RESET:
    case RHINE_INIT_WOL:
        // Now, the auto/reauto isn't executed always, and this command will disable CR0_FDX bit.
        // If this command is executed after rhine_set_media_mode, the duplex setting
        // maybe un-matchs between the PHY and MAC.
        CSR_WRITE_2(&pInfo->hw, (CR0_DPOLL|CR0_TXON|CR0_RXON|CR0_STRT), MAC_REG_CR0);

        netif_stop_queue(dev);
        if (rhine_set_media_mode(&pInfo->hw, &pInfo->hw.sOpts)!=RHINE_LINK_CHANGE) {
            status = rhine_check_media_mode(&pInfo->hw);
            printk("%s: ",pInfo->dev->name);
            rhine_print_link_status(status);
            if (!(status & RHINE_LINK_FAIL))
                netif_wake_queue(dev);
        }else{
            status = rhine_check_media_mode(&pInfo->hw);
            if(status & RHINE_LINK_FAIL){
                printk("%s: ",pInfo->dev->name);
                rhine_print_link_status(status);
            }
        }

        if ((pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL) && (pInfo->hw.sOpts.flow_cntl == 1))
            enable_flow_control_ability(&pInfo->hw);

        rhine_clearISR(&pInfo->hw);

        return;
        break;

    case RHINE_INIT_COLD:
    default:
        // write dev->dev_addr to MAC address field for MAC address override
        for (i = 0; i < 6; i++){
            CSR_WRITE_1(&pInfo->hw, dev->dev_addr[i], MAC_REG_PAR+i);
	}

        if (byRevId >= REV_ID_VT6102_A)
            BYTE_REG_BITS_OFF(&pInfo->hw, CFGA_LED0S0, MAC_REG_CFGA);

        rhine_set_tx_thresh(&pInfo->hw, pInfo->hw.sOpts.tx_thresh);
        rhine_set_rx_thresh(&pInfo->hw, pInfo->hw.sOpts.rx_thresh);
        rhine_set_DMA_length(&pInfo->hw, pInfo->hw.sOpts.DMA_length);
        // for VT3043,VT3071 only
        if (byRevId < REV_ID_VT6102_A) {
        // disable queue packet, PATCH.... Aladdin 4/5 bug
            BYTE_REG_BITS_ON(&pInfo->hw, CFGB_QPKTDIS, MAC_REG_CFGB);
        }
        else {
            // enable queue packet
            BYTE_REG_BITS_OFF(&pInfo->hw, CFGB_QPKTDIS, MAC_REG_CFGB);
            // suspend-well accept broadcast, multicast
            CSR_WRITE_1(&pInfo->hw, WOLCFG_SAM|WOLCFG_SAB, MAC_REG_WOLCG_SET);
        }

        // back off algorithm use original IEEE standard
        BYTE_REG_BITS_ON(&pInfo->hw, TCR_OFSET,MAC_REG_TCR);
        BYTE_REG_BITS_OFF(&pInfo->hw, (CFGD_CRADOM | CFGD_CAP | CFGD_MBA | CFGD_BAKOPT), MAC_REG_CFGD);
        // set packet filter
        // receive directed and broadcast address
        rhine_set_multi(dev);

        pInfo->hw.IntMask=
            (IMR_PRXM   | IMR_PTXM  | IMR_RXEM  | IMR_TXEM  | IMR_TUM   |
            IMR_RUM     | IMR_BEM   | IMR_CNTM      | IMR_ERM   | IMR_ETM   |
            IMR_ABTM    | IMR_SRCM  | IMR_NORBFM | IMR_OVFM);

        if (pInfo->hw.byRevId > REV_ID_VT6102_A) {
            if (pInfo->hw.byRevId < REV_ID_VT6105M_A0)
                pInfo->hw.IntMask |= (IMR_GENM|IMR_TDWBRAI);
            else
                pInfo->hw.IntMask |= (IMR_GENM|IMR_TM1_INT);
        }

        CSR_WRITE_4(&pInfo->hw, pInfo->rd_pool_dma, MAC_REG_CUR_RD_ADDR);

        for (i=0; i<pInfo->hw.nTxQueues; i++)
            CSR_WRITE_4(&pInfo->hw, pInfo->td_pool_dma[i], MAC_REG_CUR_TD_ADDR+(4*i));

        if (byRevId>=REV_ID_VT6105M_A0)
            rhine_init_cam_filter(pInfo);

        if (pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL)
            rhine_init_flow_control_register(&pInfo->hw, pInfo->hw.sOpts.nRxDescs);

        CSR_WRITE_2(&pInfo->hw, (CR0_DPOLL|CR0_TXON|CR0_RXON|CR0_STRT), MAC_REG_CR0);



        netif_stop_queue(dev);
        if (rhine_set_media_mode(&pInfo->hw, &pInfo->hw.sOpts) != RHINE_LINK_CHANGE) {
            status = rhine_check_media_mode(&pInfo->hw);
            printk("%s: ",pInfo->dev->name);
            rhine_print_link_status(status);
            if (!(status & RHINE_LINK_FAIL))
                netif_wake_queue(dev);
        }
        /*else if (pInfo->hw.mii_status & RHINE_LINK_FAIL){*/
        else {
            status = rhine_check_media_mode(&pInfo->hw);
            if(status & RHINE_LINK_FAIL){
                printk("%s: ",pInfo->dev->name);
                rhine_print_link_status(status);
            }
        }

        if ((pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL) && (pInfo->hw.sOpts.flow_cntl == 1))
            enable_flow_control_ability(&pInfo->hw);
        rhine_clearISR(&pInfo->hw);
    }
}

static void
rhine_init_pci(PRHINE_INFO pInfo) {

    // for VT3043,VT3071 only
    if (pInfo->hw.byRevId < REV_ID_VT6102_A) {
        PCI_BYTE_REG_BITS_ON(MODE2_MODE10T, PCI_REG_MODE2, pInfo->pcid);
    }
    else {
        // turn this on to avoid retry forever
        PCI_BYTE_REG_BITS_ON(MODE2_PCEROPT, PCI_REG_MODE2, pInfo->pcid);
        // for some legacy BIOS and OS don't open BusM
        // bit in PCI configuration space. So, turn it on.
        PCI_BYTE_REG_BITS_ON(COMMAND_BUSM, PCI_REG_COMMAND, pInfo->pcid);
        // turn this on to detect MII coding error
        PCI_BYTE_REG_BITS_ON(MODE3_MIION, PCI_REG_MODE3, pInfo->pcid);
        // Turn on MODE10T if it is 3206
        if ((pInfo->hw.byRevId >= REV_ID_VT6105_LOM) && (pInfo->hw.byRevId < REV_ID_VT6105M_A0)) {
            PCI_BYTE_REG_BITS_ON(MODE2_MODE10T, PCI_REG_MODE2, pInfo->pcid);
        }

        // Enable Memory-Read-Multiple since VT6107A1
        if ((pInfo->hw.byRevId >= REV_ID_VT6107_A1) && (pInfo->hw.byRevId < REV_ID_VT6105M_A0)) {
            PCI_BYTE_REG_BITS_ON(MODE2_MRDPL, PCI_REG_MODE2, pInfo->pcid);
        }
    }
}

static int
rhine_found1(struct pci_dev *pcid, const struct pci_device_id *ent)
{
    static BOOL         bFirst = TRUE;
    struct net_device*  dev = NULL;
    int                 i, rc;
    PCHIP_INFO          pChip_info = (PCHIP_INFO)ent->driver_data;
    PRHINE_INFO         pInfo, p;
    PU8                 hw_addr;
    long                ioaddr, memaddr;

    if (rhine_nics++ >= MAX_UINTS) {
        printk(KERN_NOTICE RHINE_NAME ": already found %d NICs\n", rhine_nics);
        return -ENODEV;
    }

    rc = pci_enable_device(pcid);
    if (rc)
        goto err_out;

    rc = pci_set_dma_mask(pcid, 0xffffffff);
    if (rc) {
        printk(KERN_ERR RHINE_NAME "PCI DMA not supported!\n");
        goto err_out;
    }

    ioaddr = pci_resource_start(pcid, 0);
    memaddr = pci_resource_start(pcid, 1);

    pci_set_master(pcid);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    dev = alloc_etherdev(sizeof(RHINE_INFO));
#else
    dev = init_etherdev(NULL, sizeof(RHINE_INFO));
#endif

    if (dev == NULL) {
        rc = -ENOMEM;
        printk(KERN_ERR RHINE_NAME ": allocate net device failed!\n");
        goto err_out;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    /* Chain it all together */
    SET_MODULE_OWNER(dev);
    SET_NETDEV_DEV(dev, &pcid->dev);
#endif
    pInfo = netdev_priv(dev);

    if (bFirst) {
        printk(KERN_INFO "%s Ver. %s\n",RHINE_FULL_DRV_NAM, RHINE_VERSION);
        printk(KERN_INFO "Copyright (c) 2002 VIA Technologies, Inc.\n");
        bFirst=FALSE;
    }

    // init pRhine3_Infos information
    if (pRhine3_Infos == NULL) {
        pRhine3_Infos = pInfo;
    }
    else {
        for (p=pRhine3_Infos; p->next!=NULL; p=p->next)
            do {} while (0);
        p->next = pInfo;
        pInfo->prev = p;
    }

    // init pInfo information
    pci_read_config_word(pcid, PCI_SUBSYSTEM_ID, &pInfo->hw.SubSystemID);
    pci_read_config_word(pcid, PCI_SUBSYSTEM_VENDOR_ID, &pInfo->hw.SubVendorID);
    pci_read_config_byte(pcid, PCI_REVISION_ID, &pInfo->hw.byRevId);

    // set up chip io size
    if (pInfo->hw.byRevId < REV_ID_VT6102_A)
        pInfo->hw.io_size = 128;
    else
        pInfo->hw.io_size = 256;

    //pInfo->SubSystemID=pInfo->hw.SubSystemID;
    //pInfo->SubVendorID=pInfo->hw.SubVendorID;

    pInfo->hw.ioaddr = ioaddr;
    pInfo->hw.memaddr = memaddr;

   
    pInfo->chip_id = pChip_info->chip_id;
    pInfo->hw.multicast_limit = 32;
    pInfo->pcid = pcid;
    spin_lock_init(&(pInfo->lock));
    spin_lock_init(&(pInfo->xmit_lock));

#ifdef CONFIG_PROC_FS
    rhine_init_proc_fs(pInfo);
#endif

    pInfo->dev = dev;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    rc = pci_request_regions(pcid, RHINE_NAME);
    if (rc) {
        printk(KERN_ERR RHINE_NAME ": Failed to find PCI device\n");
        goto err_out_free_dev;
    }
#else
    if (check_region(pInfo->hw.ioaddr, pInfo->hw.io_size)) {
        printk(KERN_ERR RHINE_NAME ": Failed to find PCI device\n");
        goto err_out_free_dev;
    }
    request_region(pInfo->hw.ioaddr, pInfo->hw.io_size, RHINE_NAME);
#endif
    rhine_create_proc_entry(pInfo);


    rhine_enable_mmio(&pInfo->hw);

    hw_addr = ioremap(pInfo->hw.memaddr & PCI_BASE_ADDRESS_MEM_MASK, pInfo->hw.io_size);
    if (!hw_addr) {
        rc = -EIO;
        printk(KERN_ERR RHINE_NAME ": ioremap failed for region 0x%lx\n", pInfo->hw.memaddr);
        goto err_out_free_res;
    }

    pInfo->hw.hw_addr = hw_addr;

    rhine_wol_reset(&pInfo->hw);

    rhine_get_phy_id(&pInfo->hw);

    // software reset
    rhine_soft_reset(&pInfo->hw);
    mdelay(5);

    // EEPROM reload
    rhine_reload_eeprom(&pInfo->hw);

    // set net_device related stuffs
    dev->base_addr = pInfo->hw.ioaddr;
    for (i=0; i<6; i++)
        dev->dev_addr[i] = CSR_READ_1(&pInfo->hw, MAC_REG_PAR+i);
    dev->irq                = pcid->irq;
    dev->open               = rhine_open;
    dev->hard_start_xmit    = rhine_xmit;
    dev->stop               = rhine_close;
    dev->get_stats          = rhine_get_stats;
    dev->set_multicast_list = rhine_set_multi;
    dev->do_ioctl           = rhine_ioctl;

#ifdef  RHINE_ZERO_COPY_SUPPORT
    dev->features           |= NETIF_F_SG;
#endif

#ifdef  RHINE_TX_CSUM_SUPPORT
    if ((pInfo->hw.byRevId>=REV_ID_VT6105M_A0)
        && (pInfo->hw.flags & RHINE_FLAGS_TX_CSUM)) {
        dev->features       |= NETIF_F_IP_CSUM;
    }
#endif

    rhine_get_options(&pInfo->hw.sOpts, rhine_nics-1);

    // set up TX queue number
    pInfo->hw.nTxQueues = 1;

    // set up chip flags
    pChip_info->flags |= (RHINE_FLAGS_IP_ALIGN|RHINE_FLAGS_VAL_PKT_LEN);

    if (pInfo->hw.byRevId < REV_ID_VT6102_A) {
        pChip_info->flags |= RHINE_FLAGS_TX_ALIGN;
    }
    else {
        pChip_info->flags |= RHINE_FLAGS_FLOW_CTRL;
    }

    if (pInfo->hw.byRevId >= REV_ID_VT6105M_A0)
        pChip_info->flags |= (RHINE_FLAGS_TAGGING|RHINE_FLAGS_TX_CSUM|RHINE_FLAGS_HAVE_CAM|RHINE_FLAGS_RX_CSUM);

    // Mask out the options cannot be set to the chip
    pInfo->hw.sOpts.flags &= pChip_info->flags;

    //Enable the chip specified capbilities that is not in the option list
    pInfo->hw.flags = pInfo->hw.sOpts.flags | (pChip_info->flags & 0xFF000000UL);

    if (pInfo->hw.byRevId >= REV_ID_VT6102_A) {
        pInfo->wol_opts = pInfo->hw.sOpts.wol_opts;
        pInfo->hw.flags |= RHINE_FLAGS_WOL_ENABLED;
    }
    
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
    rc = register_netdev(dev);
    if (rc)
    {
        printk(KERN_ERR RHINE_NAME ": Failed to register netdev\n");
        goto err_out_unmap;
    }
#endif

    rhine_print_info(pInfo);

    pci_set_drvdata(pcid, pInfo);

    return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
err_out_unmap:
    iounmap(pInfo->hw.hw_addr);
err_out_free_res:
    pci_release_regions(pcid);
err_out_free_dev:
    free_netdev(dev);
#else
err_out_free_res:
    if (pInfo->hw.ioaddr)
        release_region(pInfo->hw.ioaddr, pInfo->hw.io_size);
err_out_free_dev:
    kfree(dev);
#endif
err_out:
    return rc;
}

static void rhine_print_info(PRHINE_INFO pInfo)
{
    struct net_device* dev = pInfo->dev;

    printk(KERN_INFO "%s: %s\n",dev->name, get_product_name(pInfo->chip_id));
    printk(KERN_INFO "%s: MAC=%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
        dev->name,
        dev->dev_addr[0],dev->dev_addr[1],dev->dev_addr[2],
        dev->dev_addr[3],dev->dev_addr[4],dev->dev_addr[5]);

    printk(" IO=0x%lx Mem=0x%lx ", pInfo->hw.ioaddr, pInfo->hw.memaddr);
    printk(" IRQ=%d \n", dev->irq);
}

static BOOL rhine_init_rings(PRHINE_INFO pInfo) {
    struct net_device* dev = pInfo->dev;
    void*   vir_pool;
    int     i;

    /*allocate all RD/TD rings a single pool*/
    vir_pool=pci_alloc_consistent(pInfo->pcid,
                    pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) +
                    pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues,
                    &pInfo->pool_dma);

    if (vir_pool==NULL) {
        printk(KERN_ERR "%s : allocate dma memory failed\n", dev->name);
        return FALSE;
    }

    memset(vir_pool,0, pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) +
        pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues);

    pInfo->hw.aRDRing=vir_pool;
    pInfo->rd_pool_dma=pInfo->pool_dma;

    pInfo->tx_bufs=pci_alloc_consistent(pInfo->pcid,
                pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues,
                &pInfo->tx_bufs_dma);

    if (pInfo->tx_bufs==NULL) {
        printk(KERN_ERR "%s: allocate dma memory failed\n", dev->name);
        pci_free_consistent(pInfo->pcid,
            pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) +
            pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues,
            pInfo->hw.aRDRing, pInfo->pool_dma);
        return FALSE;
    }

    memset(pInfo->tx_bufs,0,pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues);

    for (i=0;i<pInfo->hw.nTxQueues;i++) {

        pInfo->td_pool_dma[i]=pInfo->rd_pool_dma+
                pInfo->hw.sOpts.nRxDescs*sizeof(RX_DESC)+
                pInfo->hw.sOpts.nTxDescs*sizeof(TX_DESC)*i;

        pInfo->hw.apTDRings[i]=vir_pool+
                pInfo->hw.sOpts.nRxDescs*sizeof(RX_DESC)+
                pInfo->hw.sOpts.nTxDescs*sizeof(TX_DESC)*i;
    }

    return TRUE;
}

static void rhine_free_rings(PRHINE_INFO pInfo) {
    pci_free_consistent(pInfo->pcid,
        pInfo->hw.sOpts.nRxDescs * sizeof(RX_DESC) +
        pInfo->hw.sOpts.nTxDescs * sizeof(TX_DESC)*pInfo->hw.nTxQueues,
        pInfo->hw.aRDRing, pInfo->pool_dma);

    if (pInfo->tx_bufs)
        pci_free_consistent(pInfo->pcid,
                pInfo->hw.sOpts.nTxDescs * PKT_BUF_SZ*pInfo->hw.nTxQueues,
                pInfo->tx_bufs, pInfo->tx_bufs_dma);

}

static BOOL rhine_init_rd_ring(PRHINE_INFO pInfo) {
    struct net_device* dev = pInfo->dev;
    int i;
    dma_addr_t      curr = pInfo->rd_pool_dma;
    PRX_DESC        pDesc;
    PRHINE_RD_INFO  pRDInfo;

    pInfo->aRDInfo = kmalloc(sizeof(RHINE_RD_INFO)*pInfo->hw.sOpts.nRxDescs, GFP_KERNEL);
    memset(pInfo->aRDInfo, 0, sizeof(RHINE_RD_INFO)*pInfo->hw.sOpts.nRxDescs);

    /* Init the RD ring entries */
    for (i = 0; i < pInfo->hw.sOpts.nRxDescs; i++, curr+=sizeof(RX_DESC)) {

        pDesc = &(pInfo->hw.aRDRing[i]);
        pRDInfo = &(pInfo->aRDInfo[i]);

        if (!rhine_alloc_rx_buf(pInfo, i)) {
            RHINE_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc rx bufs\n",
            dev->name);
            return FALSE;
        }

        pRDInfo->curr_desc = cpu_to_le32(curr);
        pDesc->next_desc = cpu_to_le32(curr+sizeof(RX_DESC));
    }

    pInfo->hw.aRDRing[i-1].next_desc = cpu_to_le32(pInfo->rd_pool_dma);
    pInfo->hw.iCurrRDIdx = 0;
    return TRUE;
}

static void rhine_free_rd_ring(PRHINE_INFO pInfo) {
    int i;
    PRX_DESC        pDesc;
    PRHINE_RD_INFO  pRDInfo;

    /* Init the RD ring entries */
    for (i = 0; i < pInfo->hw.sOpts.nRxDescs; i++) {
        pDesc = &(pInfo->hw.aRDRing[i]);
        pRDInfo = &(pInfo->aRDInfo[i]);

        if (pRDInfo->skb_dma) {
            pci_unmap_single(pInfo->pcid, pRDInfo->skb_dma,
                    pInfo->hw.rx_buf_sz, PCI_DMA_FROMDEVICE);
        }

        if (pRDInfo->skb)
            dev_kfree_skb(pRDInfo->skb);
    }

    if (pInfo->aRDInfo)
        kfree(pInfo->aRDInfo);
    pInfo->aRDInfo = NULL;
}

static BOOL rhine_init_td_ring(PRHINE_INFO pInfo) {
    //struct net_device* dev = pInfo->dev;
    int i, j;
    dma_addr_t  curr;
    PTX_DESC    pDesc;
    PRHINE_TD_INFO  pTDInfo;

    /* Init the RD ring entries */
    for (j=0; j<pInfo->hw.nTxQueues; j++) {
        curr = pInfo->td_pool_dma[j];

        pInfo->apTDInfos[j] = kmalloc(sizeof(RHINE_TD_INFO)*pInfo->hw.sOpts.nTxDescs,
                GFP_KERNEL);
        memset(pInfo->apTDInfos[j], 0,
                sizeof(RHINE_TD_INFO)*pInfo->hw.sOpts.nTxDescs);

        for (i = 0; i < pInfo->hw.sOpts.nTxDescs; i++, curr+=sizeof(TX_DESC)) {

            pDesc = &(pInfo->hw.apTDRings[j][i]);
            pTDInfo = &(pInfo->apTDInfos[j][i]);

            pTDInfo->buf = pInfo->tx_bufs+(i+j)*PKT_BUF_SZ;
            pTDInfo->buf_dma = pInfo->tx_bufs_dma+(i+j)*PKT_BUF_SZ;
            pTDInfo->curr_desc = cpu_to_le32(curr);

            pDesc->tdesc1 |= cpu_to_le32(TCR_CHAIN);
            pDesc->next_desc = cpu_to_le32(curr+sizeof(TX_DESC));
        }

        pInfo->hw.apTDRings[j][i-1].next_desc = cpu_to_le32(pInfo->td_pool_dma[j]);
        pInfo->hw.aiTailTDIdx[j] = pInfo->hw.aiCurrTDIdx[j] = pInfo->hw.iTDUsed[j] = 0;
    }
    return TRUE;
}

static void rhine_free_td_ring(PRHINE_INFO pInfo) {
    int i, j;
    PTX_DESC        pDesc;
    PRHINE_TD_INFO  pTDInfo;

    for (j=0; j<pInfo->hw.nTxQueues; j++) {
        for (i = 0; i < pInfo->hw.sOpts.nTxDescs; i++) {
            pDesc = &(pInfo->hw.apTDRings[j][i]);
            pTDInfo = &(pInfo->apTDInfos[j][i]);

            if (pTDInfo->skb_dma)
                pci_unmap_single(pInfo->pcid, pTDInfo->skb_dma,
                        pTDInfo->skb->len, PCI_DMA_TODEVICE);

            if (pTDInfo->skb)
                dev_kfree_skb(pTDInfo->skb);
        }

        if (pInfo->apTDInfos[j]) {
            kfree(pInfo->apTDInfos[j]);
            pInfo->apTDInfos[j] = NULL;
        }
    }
}

/*-----------------------------------------------------------------*/
static int rhine_rx_srv(PRHINE_INFO pInfo, U32 status) {
    struct net_device*          dev = pInfo->dev;
    PRX_DESC                    pRD;
    PRHINE_RD_INFO              pRDInfo;
    //struct net_device_stats*    pStats = &pInfo->stats;
    int                         iCurrRDIdx = pInfo->hw.iCurrRDIdx;
    int                         works = 0;
    U16                         frame_length;

    rhine_reset_rx_stats(&pInfo->hw.stats);

    while (TRUE) {
        pRD = &(pInfo->hw.aRDRing[iCurrRDIdx]);
        pRDInfo = &(pInfo->aRDInfo[iCurrRDIdx]);

         //No more skb buff?
        if (pRDInfo->skb == NULL) {
            if (!rhine_alloc_rx_buf(pInfo, iCurrRDIdx))
                break;
        }

        if (works++ > INT_WORKS_DEF)
            break;

        //if (pRD->rdesc0.f1Owner == OWNED_BY_NIC)
        if(rhine_rd_own_bit_on(pRD))
            break;

        frame_length=rhine_get_rx_frame_length(pRD);
        //pInfo->adwRMONStats[RMON_Octets] += pRD->rdesc0.f15Length;
        pInfo->adwRMONStats[RMON_Octets] += frame_length;

        if (frame_length >= 64) {
            if (frame_length == 64)
                pInfo->adwRMONStats[RMON_Pkts64Octets]++;
            else if (frame_length < 128)
                pInfo->adwRMONStats[RMON_Pkts65to127Octets]++;
            else if (frame_length < 256)
                pInfo->adwRMONStats[RMON_Pkts128to255Octets]++;
            else if (frame_length < 512)
                pInfo->adwRMONStats[RMON_Pkts256to511Octets]++;
            else if (frame_length < 1024)
                pInfo->adwRMONStats[RMON_Pkts512to1023Octets]++;
            else if (frame_length <= 1518)
                pInfo->adwRMONStats[RMON_Pkts1024to1518Octets]++;
        }

        if (pRD->rdesc0 & cpu_to_le32(RSR1_RXOK)) {
            if (rhine_receive_frame(pInfo, iCurrRDIdx)) {
                if (!rhine_alloc_rx_buf(pInfo, iCurrRDIdx)) {
                    RHINE_PRT(MSG_LEVEL_ERR, KERN_ERR
                    "%s: can not allocate rx buf\n", dev->name);
                    //pStats->rx_errors++;
                    pInfo->hw.stats.rx_errors++;
                    break;
                }
            }
            else {
                //pStats->rx_errors++;
                //pStats->rx_dropped++;
                pInfo->hw.stats.rx_errors++;
                pInfo->hw.stats.rx_dropped++;
            }
        }
        else {
            //pStats->rx_errors++;
            pInfo->hw.stats.rx_errors++;
            if (pRD->rdesc0 & cpu_to_le32(RSR0_CRC))
                pInfo->hw.stats.rx_crc_errors++;
            if (pRD->rdesc0 & cpu_to_le32(RSR0_FAE))
                pInfo->hw.stats.rx_frame_errors++;
            if (pRD->rdesc0 & cpu_to_le32(RSR0_FOV))
                pInfo->hw.stats.rx_fifo_errors++;
            pInfo->hw.stats.rx_dropped++;
        }

        rhine_set_rd_own(pRD);

        if (pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL)
            CSR_WRITE_1(&pInfo->hw, 1, MAC_REG_FLOWCR0);
        dev->last_rx = jiffies;

        ADD_ONE_WITH_WRAP_AROUND(iCurrRDIdx, pInfo->hw.sOpts.nRxDescs);
    }

    pInfo->hw.iCurrRDIdx = iCurrRDIdx;
    return works;
}

static inline void
rhine_rx_csum(PRX_DESC pRD, struct sk_buff* skb) {

    U32 status;
    skb->ip_summed=CHECKSUM_NONE;
    if (pRD->rdesc0 & cpu_to_le32(RSR0_FRAG))
        return;

    status = cpu_to_le32(pRD->rdesc1);
    if (status & PQSTS_IPKT) {
            if (status & PQSTS_IPOK) {
            if ((status & PQSTS_TCPKT)
                ||(status & PQSTS_UDPKT)) {
                if (!(status & PQSTS_TUOK )) {
                    return;
                }
            }
            skb->ip_summed=CHECKSUM_UNNECESSARY;
        }
    }

}

static BOOL
rhine_receive_frame(PRHINE_INFO pInfo, int idx) {
    struct net_device*          dev = pInfo->dev;
    struct net_device_stats*    pStats = &pInfo->stats;
    PRHINE_RD_INFO              pRDInfo = &(pInfo->aRDInfo[idx]);
    PRX_DESC                    pRD = &(pInfo->hw.aRDRing[idx]);
    struct sk_buff*             skb;
    U16                         wTag;
    U16                         frame_length;
    

    if ((cpu_to_le32(pRD->rdesc0) & (RSR1_STP|RSR1_EDP)) != (RSR1_STP|RSR1_EDP)) {
        RHINE_PRT(MSG_LEVEL_VERBOSE,
            KERN_NOTICE " %s : the received frame span multple RDs\n",
            dev->name);
        pStats->rx_length_errors++;
        return FALSE;
    }

    frame_length = rhine_get_rx_frame_length(pRD);

    //Drop long packet
    //if ((pRD->rdesc0.f15Length-4) > (ETH_FRAME_LEN)) {
    if((frame_length-4) > (ETH_FRAME_LEN)) {
        pStats->rx_length_errors++;
        pInfo->adwRMONStats[RMON_OversizePkts]++;
        return FALSE;
    }

    if (pRD->rdesc0 & cpu_to_le32(RSR1_MAR))
        pInfo->adwRMONStats[RMON_MulticastPkts]++;

    if (pRD->rdesc0 & cpu_to_le32(RSR1_BAR))
        pInfo->adwRMONStats[RMON_BroadcastPkts]++;

    skb = pRDInfo->skb;
    skb->dev = dev;

    //Get Tag
    wTag = htons(*(PU16)((PU8)skb->data+((frame_length+3) & ~3)+2));

    if (pRD->rdesc1 & cpu_to_le32(PQSTS_TAG)) {
        if (!(pInfo->hw.flags & RHINE_FLAGS_TAGGING)) {
            if ((wTag & 0x0FFF) != 0) {
                pStats->rx_dropped++;
                return FALSE;
            }
        }
    }

    if (pInfo->hw.flags & RHINE_FLAGS_IP_ALIGN) {
        int i;
        //for (i = pRD->rdesc0.f15Length+4; i >= 0 ; i--)
        for (i = frame_length+4; i >= 0 ; i--)
            *(skb->data + i + 2) = *(skb->data + i);
        skb->data += 2;
        skb->tail += 2;
    }

    //skb_put(skb, (pRD->rdesc0.f15Length-4));
    skb_put(skb, frame_length-4);

    skb->protocol = eth_type_trans(skb, skb->dev);

    //drop frame not met IEEE 802.3
    if (pInfo->hw.flags & RHINE_FLAGS_VAL_PKT_LEN) {
        if ( (skb->protocol == htons(ETH_P_802_2)) &&
             (skb->len != htons(*(PU16)(skb->mac.raw + 12))) )
        {
            //skb_put(skb, -(pRD->rdesc0.f15Length-4));
            skb_put(skb, -(frame_length-4));
            pStats->rx_length_errors++;
            return FALSE;
        }
    }

    pci_unmap_single(pInfo->pcid,pRDInfo->skb_dma,
                pInfo->hw.rx_buf_sz, PCI_DMA_FROMDEVICE);
    pRDInfo->skb_dma = (dma_addr_t) NULL;

    skb->ip_summed = CHECKSUM_NONE;

    if (pInfo->hw.flags & RHINE_FLAGS_RX_CSUM)
        rhine_rx_csum(pRD, skb);

    if( pRD->rdesc0 & cpu_to_le32(RSR0_FRAG))
        skb->ip_summed = CHECKSUM_NONE;

    pStats->rx_bytes += skb->len;
    pStats->rx_packets++;

    netif_rx(skb);

    return TRUE;
}

static BOOL rhine_alloc_rx_buf(PRHINE_INFO pInfo, int idx) {
    struct net_device* dev = pInfo->dev;
    PRX_DESC        pRD = &(pInfo->hw.aRDRing[idx]);
    PRHINE_RD_INFO pRDInfo = &(pInfo->aRDInfo[idx]);

    pRDInfo->skb = dev_alloc_skb(pInfo->hw.rx_buf_sz+15);

    if (pRDInfo->skb == NULL)
        return FALSE;

    ASSERT(pRDInfo->skb);
    //skb_reserve(pRDInfo->skb, ((unsigned long)pRDInfo->skb->tail) & 15);
    skb_reserve(pRDInfo->skb, 16 - (((unsigned long)pRDInfo->skb->tail) & 15));

    pRDInfo->skb->dev = dev;
    pRDInfo->skb_dma=
        pci_map_single(pInfo->pcid, pRDInfo->skb->tail, pInfo->hw.rx_buf_sz,
            PCI_DMA_FROMDEVICE);

    //*((PU32)&(pRD->rdesc0)) = 0;
    pRD->rdesc0=0;

    //pRD->rdesc1.f15BufLen = cpu_to_le16((unsigned short)pInfo->hw.rx_buf_sz);
    rhine_set_rx_buf_sz(pRD, (unsigned short)pInfo->hw.rx_buf_sz);
    //pRD->rdesc0.f1Owner = OWNED_BY_NIC;
    rhine_set_rd_own(pRD);
    pRD->buff_addr = cpu_to_le32(pRDInfo->skb_dma);

    return TRUE;
}

//
// Re-transmited the frame
//
static void rhine_tx_srv_resend(PRHINE_INFO pInfo, int idx, int iQNo) {
    PTX_DESC    pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
    PRHINE_TD_INFO pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

    WAIT_MAC_TX_OFF(pInfo);

    //pTD->tdesc0.f1Owner = OWNED_BY_NIC;
    rhine_set_td_own(pTD);

    CSR_WRITE_4(&pInfo->hw, pTDInfo->curr_desc, MAC_REG_CUR_TD_ADDR+(4*iQNo));


    BYTE_REG_BITS_ON(&pInfo->hw, CR0_TXON, MAC_REG_CR0);

    if( pInfo->hw.flags & RHINE_FLAGS_TAGGING)
         BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1, MAC_REG_CR1);
}

//
//  Drop the frame
//
static void rhine_tx_srv_drop(PRHINE_INFO pInfo, int idx, int iQNo) {
    PTX_DESC    pTD;
    PRHINE_TD_INFO pTDInfo;

    WAIT_MAC_TX_OFF(pInfo);

    ADD_ONE_WITH_WRAP_AROUND(idx, pInfo->hw.sOpts.nTxDescs);
    pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
    pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

    CSR_WRITE_4(&pInfo->hw, pTDInfo->curr_desc, MAC_REG_CUR_TD_ADDR+(iQNo*4));
    BYTE_REG_BITS_ON(&pInfo->hw, CR0_TXON, MAC_REG_CR0);

    if( pInfo->hw.flags & RHINE_FLAGS_TAGGING)
         BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1, MAC_REG_CR1);
}

//
// Drop all frame on the TX Queue
//
static void rhine_tx_srv_drop_all(PRHINE_INFO pInfo) {
    PTX_DESC    pTD;
    PRHINE_TD_INFO  pTDInfo = NULL;
    int         iQNo, idx;    

    rhine_reset_tx_stats(&pInfo->hw.stats);

    WAIT_MAC_TX_OFF(pInfo);

    //Drop all transmited packets in the TD queue, because
    //The TD write back status may be incorrect.
    for (iQNo=0; iQNo<pInfo->hw.nTxQueues; iQNo++) {
        for (idx=pInfo->hw.aiTailTDIdx[iQNo]; pInfo->hw.iTDUsed[iQNo]>0;
            idx=(idx+1) % pInfo->hw.sOpts.nTxDescs) {

            //Get Tx Descriptor
            pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
            pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

            if(rhine_td_own_bit_on(pTD))
                break;

            rhine_free_tx_buf(pInfo, iQNo, idx);
            pInfo->hw.iTDUsed[iQNo]--;
            pInfo->hw.stats.tx_dropped++;
        }

        pInfo->hw.aiTailTDIdx[iQNo] = idx;
        CSR_WRITE_4(&pInfo->hw, pTDInfo->curr_desc, MAC_REG_CUR_TD_ADDR+(4*iQNo));

        if (pInfo->hw.flags & RHINE_FLAGS_TAGGING)
            BYTE_REG_BITS_ON(&pInfo->hw, 1<<(7-iQNo), MAC_REG_TQWK);
    }

    BYTE_REG_BITS_ON(&pInfo->hw, CR0_TXON, MAC_REG_CR0);
    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1, MAC_REG_CR1);
}

static int rhine_tx_srv(PRHINE_INFO pInfo, U32 status) {
    struct net_device*          dev = pInfo->dev;
    PTX_DESC                    pTD;
    int                         iQNo;
    BOOL                        bFull = FALSE;
    int                         idx;
    int                         works = 0;
    PRHINE_TD_INFO              pTDInfo;
    U32                         tdesc0;

    rhine_reset_tx_stats(&pInfo->hw.stats);

    for (iQNo = 0; iQNo < pInfo->hw.nTxQueues; iQNo++) {        
        for (idx = pInfo->hw.aiTailTDIdx[iQNo];
            pInfo->hw.iTDUsed[iQNo] > 0;
            idx = ((idx+1) % pInfo->hw.sOpts.nTxDescs)) {

            //Get Tx Descriptor
            pTD = &(pInfo->hw.apTDRings[iQNo][idx]);
            pTDInfo = &(pInfo->apTDInfos[iQNo][idx]);

            if(rhine_td_own_bit_on(pTD))
                break;

            if ( (works++ > INT_WORKS_DEF) &&
                 (!(status & (ISR_UDFI|ISR_ABTI))) )
            {
                break;
            }

            tdesc0 = pTD->tdesc0;
            /* clear status first */
            pTD->tdesc0=0;

            // Only the status of first TD in the chain is correct
            if (pTD->tdesc1 & cpu_to_le32(TCR_STP)) {
                if (tdesc0 & cpu_to_le32(TSR1_TERR)) {
                    RHINE_PRT(MSG_LEVEL_DEBUG, KERN_INFO
                        "%s : td error 0x%4X\n", dev->name,
                        tdesc0);

                    pInfo->hw.stats.tx_errors++;

                    if (tdesc0 & cpu_to_le32(TSR1_UDF)) {
                        if (pInfo->hw.sOpts.tx_thresh < 4) {
                            pInfo->hw.sOpts.tx_thresh++;

                            RHINE_PRT(MSG_LEVEL_VERBOSE, KERN_INFO
                                    "%s: transmitter fifo underrun occurred, increase fifo threshold to  %d\n",
                                    dev->name, pInfo->hw.sOpts.tx_thresh);

                            rhine_set_tx_thresh(&pInfo->hw, pInfo->hw.sOpts.tx_thresh);
                        }
                        rhine_tx_srv_resend(pInfo, idx, iQNo);
                        //pInfo->hw.IntMask |= IMR_PTXM;
                        pInfo->hw.stats.tx_fifo_errors++;
                        break;
                    }

                    if (tdesc0 & cpu_to_le32(TSR1_ABT)) {
                        RHINE_PRT(MSG_LEVEL_VERBOSE, KERN_INFO
                            "%s: transmitter fifo abort occurred\n", dev->name);
                        rhine_tx_srv_drop(pInfo, idx, iQNo);
                        //pInfo->hw.IntMask |= IMR_PTXM;
                        pInfo->hw.stats.tx_aborted_errors++;
                    }

                    pInfo->hw.stats.tx_dropped++;
                    if (tdesc0 & cpu_to_le32(TSR1_CRS))
                        pInfo->hw.stats.tx_carrier_errors++;
                    if (tdesc0 & cpu_to_le32(TSR1_OWC))
                        pInfo->hw.stats.tx_window_errors++;
                    if (tdesc0 & cpu_to_le32(TSR0_CDH))
                        pInfo->hw.stats.tx_heartbeat_errors++;

                }
                else {
                    pInfo->hw.stats.collisions += cpu_to_le32(tdesc0) & TSR0_NCR;
                    pInfo->hw.stats.tx_packets++;
                    pInfo->hw.stats.tx_bytes += pTDInfo->skb->len;
                }
                rhine_free_tx_buf(pInfo, iQNo, idx);
                pInfo->hw.iTDUsed[iQNo]--;
            }
        }
        pInfo->hw.aiTailTDIdx[iQNo] = idx;

        if (AVAIL_TD(&pInfo->hw, iQNo) < 4) {
            bFull = TRUE;
        }
    }

    if ( netif_queue_stopped(dev) &&
         (bFull == FALSE) &&
         !BYTE_REG_BITS_IS_ON(&pInfo->hw, MIISR_LNKFL, MAC_REG_MIISR) )
    {
        netif_wake_queue(dev);
    }
    return works;
}

static void rhine_error(PRHINE_INFO pInfo, int status) {
    struct net_device* dev = pInfo->dev;
    U32 mii_status;

    if (status & ISR_BE) {
        RHINE_PRT(MSG_LEVEL_ERR, KERN_ERR
            "%s: Hardware fatal error.\n",
            dev->name);
        netif_stop_queue(dev);
        rhine_shutdown(pInfo);
        return;
    }

    if (status & ISR_SRCI) {
        mii_status = rhine_check_media_mode(&pInfo->hw);
        printk("%s: ",pInfo->dev->name);
        rhine_print_link_status(mii_status);

        if ((pInfo->hw.flags & RHINE_FLAGS_FLOW_CTRL) && (pInfo->hw.sOpts.flow_cntl == 1))
            enable_flow_control_ability(&pInfo->hw);

        if (mii_status & RHINE_LINK_FAIL) {
            netif_carrier_off(dev);
            netif_stop_queue(dev);
        }
        else {
            netif_carrier_on(dev);
            netif_wake_queue(dev);
        }

    };

    if ((pInfo->hw.byRevId >= REV_ID_VT6102_A) && (pInfo->hw.byRevId < REV_ID_VT6105_A0)) {
        if (status & ISR_TDWBRAI) {
            RHINE_PRT(MSG_LEVEL_VERBOSE, KERN_INFO
                "%s: Tx descriptor status write back race occurred.\n",
                dev->name);
            rhine_tx_srv_drop_all(pInfo);
            pInfo->stats.tx_dropped+=pInfo->hw.stats.tx_dropped;
        }
    }

    if (pInfo->hw.byRevId >= REV_ID_VT6105M_A0) {
        if (status & ISR_CNT)
            rhine_UpdateMIBCounter(pInfo);
    }

#ifdef RHINE_DBG
    if (status & ISR_NORBF) {
        RHINE_PRT(MSG_LEVEL_VERBOSE, KERN_INFO
            "%s: No more receive buffer to be used\n",
            dev->name);
    }

    if (status & ISR_OVFI) {
        RHINE_PRT(MSG_LEVEL_VERBOSE, KERN_INFO
            "%s: Received FIFO Overflow\n",
            dev->name);
    }
#endif

}

static void rhine_free_tx_buf(PRHINE_INFO pInfo, int iQNo, int idx) {
    struct sk_buff* skb;
   
    PRHINE_TD_INFO pTDInfo=&pInfo->apTDInfos[iQNo][idx];
#ifdef RHINE_ZERO_COPY_SUPPORT
    PTX_DESC pTD=&pInfo->hw.apTDRings[iQNo][idx];
#endif    
    skb=pTDInfo->skb;

    if (pTDInfo->skb_dma && (pTDInfo->skb_dma != pTDInfo->buf_dma)) {
#ifdef RHINE_ZERO_COPY_SUPPORT
        pci_unmap_single(pInfo->pcid, pTDInfo->skb_dma,
            rhine_get_tx_buf_sz(pTD),PCI_DMA_TODEVICE);
#else
        pci_unmap_single(pInfo->pcid, pTDInfo->skb_dma, skb->len,
            PCI_DMA_TODEVICE);
#endif
    }

    dev_kfree_skb_irq(skb);

    pTDInfo->skb_dma=0;
    pTDInfo->skb=0;
}

static int  rhine_open(struct net_device *dev) {
    PRHINE_INFO pInfo = netdev_priv(dev);
    int i;    

    pInfo->hw.rx_buf_sz=(dev->mtu <= 1504 ? PKT_BUF_SZ : dev->mtu + 32);

    if (!rhine_init_rings(pInfo)) {
        return -ENOMEM;
    }

    if (!rhine_init_rd_ring(pInfo)) {
        rhine_free_rings(pInfo);
        return -ENOMEM;
    }

    if (!rhine_init_td_ring(pInfo)) {
        rhine_free_rd_ring(pInfo);
        rhine_free_rings(pInfo);
        return -ENOMEM;
    }

    rhine_init_pci(pInfo);
    rhine_init_adapter(pInfo, RHINE_INIT_COLD);
    i=request_irq(pInfo->pcid->irq, &rhine_intr, SA_SHIRQ, dev->name, dev);

    if (i)
        return i;

    rhine_enable_int(&pInfo->hw, 0x00000000UL);

    netif_start_queue(dev);

    pInfo->hw.flags |=RHINE_FLAGS_OPENED;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    MOD_INC_USE_COUNT;
#endif
    return 0;
}

static void rhine_shutdown(PRHINE_INFO pInfo) {

    rhine_disable_int(&pInfo->hw);
    CSR_WRITE_1(&pInfo->hw, CR0_STOP, MAC_REG_CR0);
    rhine_disable_mii_auto_poll(&pInfo->hw);
    rhine_clearISR(&pInfo->hw);
}

static int  rhine_close(struct net_device *dev) {
    PRHINE_INFO pInfo = netdev_priv(dev);

    netif_stop_queue(dev);

    rhine_shutdown(pInfo);

    if (pInfo->hw.flags & RHINE_FLAGS_WOL_ENABLED)
        rhine_get_ip(pInfo);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
    MOD_DEC_USE_COUNT;
#endif

    free_irq(dev->irq, dev);

    rhine_free_td_ring(pInfo);
    rhine_free_rd_ring(pInfo);
    rhine_free_rings(pInfo);

    pInfo->hw.flags &= (~RHINE_FLAGS_OPENED);
    return 0;
}

static int rhine_xmit(struct sk_buff *skb, struct net_device *dev) {
    PRHINE_INFO pInfo = netdev_priv(dev);
    int             iQNo = 0;
    PTX_DESC        pTD, pHeadTD;
    PRHINE_TD_INFO  pTDInfo, pHeadTDInfo;
    unsigned long   flags;
    int             iCurrTDIdx, iHeadTDIdx;

    spin_lock_irqsave(&pInfo->lock, flags);

    iCurrTDIdx = iHeadTDIdx = pInfo->hw.aiCurrTDIdx[iQNo];
    pTD = pHeadTD = &(pInfo->hw.apTDRings[iQNo][iCurrTDIdx]);
    pTDInfo = pHeadTDInfo = &(pInfo->apTDInfos[iQNo][iCurrTDIdx]);

    pHeadTD->tdesc1 |= cpu_to_le32(TCR_IC|TCR_STP|TCR_EDP);

#ifdef RHINE_ZERO_COPY_SUPPORT
    if (skb_shinfo(skb)->nr_frags>0) {
        int nfrags=skb_shinfo(skb)->nr_frags;
        pTDInfo->skb = skb;

        if ((AVAIL_TD(&pInfo->hw,iQNo)<nfrags) ||
            (pInfo->hw.flags & RHINE_FLAGS_TX_ALIGN)) {
            skb_linearize(skb,GFP_ATOMIC);
            memcpy(pTDInfo->buf, skb->data, skb->len);
            pTDInfo->skb_dma = pTDInfo->buf_dma;
            pTD->buff_addr = cpu_to_le32(pTDInfo->skb_dma);
            //pTD->tdesc1.f15BufLen = (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN);
            rhine_set_tx_buf_sz(pTD, (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN));
        }
        else {
            int i;
            pTDInfo->skb_dma =
                pci_map_single(pInfo->pcid, skb->data,
                    skb->len - skb->data_len, PCI_DMA_TODEVICE);

            pTD->buff_addr = cpu_to_le32(pTDInfo->skb_dma);
            //pTD->tdesc1.f15BufLen=skb->len - skb->data_len;
            rhine_set_tx_buf_sz(pTD, skb->len - skb->data_len);

            for (i=0; i<nfrags; i++) {
                skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
                void* addr = ((void *) page_address(frag->page +
                            frag->page_offset));

                ADD_ONE_WITH_WRAP_AROUND(iCurrTDIdx, pInfo->hw.sOpts.nTxDescs);
                // get new TD and pTDInfo
                pTD = &(pInfo->hw.apTDRings[iQNo][iCurrTDIdx]);
                pTDInfo = &(pInfo->apTDInfos[iQNo][iCurrTDIdx]);

                pTDInfo->skb_dma =
                    pci_map_single(pInfo->pcid, addr, frag->size, PCI_DMA_TODEVICE);
                pTD->buff_addr = cpu_to_le32(pTDInfo->skb_dma);
                //pTD->tdesc1.f15BufLen = frag->size;
                rhine_set_tx_buf_sz(pTD, frag->size);
            }
            pTD->tdesc1 |= cpu_to_le32(TCR_EDP);
        }
    }
    else
#endif
    if (((pInfo->hw.flags & RHINE_FLAGS_TX_ALIGN) && ((long)skb->data & 3))
            || (skb->len<ETH_ZLEN)) {

        memcpy(pHeadTDInfo->buf, skb->data, skb->len);
        pHeadTDInfo->skb = skb;
        pHeadTDInfo->skb_dma = pHeadTDInfo->buf_dma;
        pHeadTD->buff_addr = cpu_to_le32(pHeadTDInfo->skb_dma);

        //pHeadTD->tdesc1.f15BufLen = (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN);
        rhine_set_tx_buf_sz(pHeadTD, (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN));
        // padding zero if packet size is less than 60 bytes
        if (skb->len < ETH_ZLEN)
            memset(pHeadTDInfo->buf+skb->len, 0, ETH_ZLEN-skb->len);
    }
    else {
        pHeadTDInfo->skb = skb;
        pHeadTDInfo->skb_dma = pci_map_single(pInfo->pcid, skb->data, skb->len,
            PCI_DMA_TODEVICE);
        pHeadTD->buff_addr = cpu_to_le32(pHeadTDInfo->skb_dma);
        //pHeadTD->tdesc1.f15BufLen = skb->len;
        rhine_set_tx_buf_sz(pHeadTD, skb->len);
    }

    if (pInfo->hw.flags & RHINE_FLAGS_TAGGING) {
        //pHeadTD->tdesc0.f12VID = (pInfo->hw.sOpts.vid & 0xfff);
        //pHeadTD->tdesc0.f3Priority = 0;
        pHeadTD->tdesc0 &= cpu_to_le32(~FET_TXSTAT_PQMASK);
        pHeadTD->tdesc0 |= cpu_to_le32((pInfo->hw.sOpts.vid & 0xfff) << 16);
        //pHeadTD->tdesc1.byTCR |= TCR_TAG;
        pHeadTD->tdesc1 |= cpu_to_le32(TCR_TAG);
    }

#ifdef RHINE_TX_CSUM_SUPPORT
    if ((pInfo->hw.flags & RHINE_FLAGS_TX_CSUM) &&
        (skb->ip_summed == CHECKSUM_HW)) {
        struct iphdr* ip=skb->nh.iph;
        if (ip->protocol == IPPROTO_TCP)
            pHeadTD->tdesc1 |= cpu_to_le32(TCR_TCPCK);
        else if (ip->protocol == IPPROTO_UDP)
            pHeadTD->tdesc1 |= cpu_to_le32(TCR_UDPCK);
        pHeadTD->tdesc1 |= cpu_to_le32(TCR_IPCK);
    }
#endif

    wmb();
    //pHeadTD->tdesc0.f1Owner = OWNED_BY_NIC;
    rhine_set_td_own(pHeadTD);
    wmb();

    pInfo->hw.iTDUsed[iQNo]++;

    if (AVAIL_TD(&pInfo->hw,iQNo)<=1) {
        netif_stop_queue(dev);
    }

    pInfo->hw.aiCurrTDIdx[iQNo]=
            (iCurrTDIdx+1) % pInfo->hw.sOpts.nTxDescs;

    if (pInfo->hw.flags & RHINE_FLAGS_TAGGING)
        BYTE_REG_BITS_ON(&pInfo->hw, 1 << (7-iQNo), MAC_REG_TQWK);

    BYTE_REG_BITS_ON(&pInfo->hw, CR1_TDMD1,MAC_REG_CR1);

    dev->trans_start = jiffies;

    spin_unlock_irqrestore(&pInfo->lock, flags);
    return 0;
}

static irqreturn_t rhine_intr(int irq, void *dev_instance, struct pt_regs *regs) {
    struct net_device*  dev = dev_instance;
    PRHINE_INFO         pInfo = netdev_priv(dev);
    U32                 isr_status;
    U32                 AllIsrStatus = 0;
    int                 max_count = 0;
    int                 handled = 0;

    if (!spin_trylock(&pInfo->lock))
        return IRQ_RETVAL(handled);

    isr_status = rhine_ReadISR(&pInfo->hw);

    if ( ((isr_status & pInfo->hw.IntMask) == 0) ||
         (isr_status == 0x00FFFFFFUL) )
    {
        spin_unlock(&pInfo->lock);
        return IRQ_RETVAL(handled);
    }

    handled = 1;
    rhine_disable_int(&pInfo->hw);

    while (isr_status != 0) {
        // [4.39]
        AllIsrStatus |= isr_status;

        rhine_WriteISR(isr_status, &pInfo->hw);

        if (isr_status & (ISR_SRCI|ISR_TDWBRAI|ISR_BE|ISR_CNT
#ifdef RHINE_DBG
            |ISR_OVFI|ISR_UDFI))
#else
            ))
#endif
            rhine_error(pInfo, isr_status);

        //if (isr_status & (ISR_RXE|ISR_PRX))
        max_count += rhine_rx_srv(pInfo, isr_status);
        rhine_update_rx_stats(pInfo->stats, pInfo->hw.stats);

        //if (isr_status & (ISR_TXE|ISR_PTX))
        max_count += rhine_tx_srv(pInfo, isr_status);
        rhine_update_tx_stats(pInfo->stats, pInfo->hw.stats);

        //if (isr_status & (ISR_RXE|ISR_PRX))
        max_count += rhine_rx_srv(pInfo, isr_status);
        rhine_update_rx_stats(pInfo->stats, pInfo->hw.stats);

        //if (isr_status & (ISR_TXE|ISR_PTX))
        max_count += rhine_tx_srv(pInfo, isr_status);
        rhine_update_tx_stats(pInfo->stats, pInfo->hw.stats);

        isr_status = rhine_ReadISR(&pInfo->hw);

        if (max_count > pInfo->hw.sOpts.int_works)
            break;
    } // while

    rhine_enable_int(&pInfo->hw, AllIsrStatus);
    spin_unlock(&pInfo->lock);
    return IRQ_RETVAL(handled);
}




static unsigned const ethernet_polynomial = 0x04c11db7U;
static inline u32 ether_crc(int length, unsigned char *data)
{
    int crc = -1;

    while(--length >= 0) {
        unsigned char current_octet = *data++;
        int bit;
        for (bit = 0; bit < 8; bit++, current_octet >>= 1) {
            crc = (crc << 1) ^
                ((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
        }
    }
    return crc;
}

static void rhine_set_multi(struct net_device *dev) {

    PRHINE_INFO pInfo = netdev_priv(dev);
    u32                 mc_filter[2];
    //u8                  rx_mode=0;
    int                 i;
    unsigned long       flags;
    struct dev_mc_list  *mclist;

    spin_lock_irqsave(&pInfo->lock,flags);

    CSR_WRITE_1(&pInfo->hw, (pInfo->hw.sOpts.rx_thresh<<5), MAC_REG_RCR);

    if (dev->flags & IFF_PROMISC) {         /* Set promiscuous. */
        /* Unconditionally log net taps. */
        printk(KERN_NOTICE "%s: Promiscuous mode enabled.\n", dev->name);
        rhine_set_promiscuous(&pInfo->hw);
    }
    else if ((dev->mc_count > pInfo->hw.multicast_limit)
        ||  (dev->flags & IFF_ALLMULTI)) {
        rhine_set_all_multicast(&pInfo->hw);
    }
    else{ 
        if (pInfo->hw.flags & RHINE_FLAGS_HAVE_CAM) {
            U32 mask=0;
            int offset=MCAM_SIZE-pInfo->hw.multicast_limit;
            rhine_get_cam_mask(&pInfo->hw,&mask,RHINE_MULTICAST_CAM);
            for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
                i++, mclist = mclist->next) {
                rhine_set_cam(&pInfo->hw,i+offset,mclist->dmi_addr,RHINE_MULTICAST_CAM);
                mask|=1<<(offset+i);
            }
            rhine_set_cam_mask(&pInfo->hw,mask,RHINE_MULTICAST_CAM);
        }
        else {
            memset(mc_filter, 0, sizeof(mc_filter));
            for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
                 i++, mclist = mclist->next) {
                int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
                mc_filter[bit_nr >> 5] |= (1 << (bit_nr & 31));
            }
            CSR_WRITE_4(&pInfo->hw, mc_filter[0], MAC_REG_MAR);
            CSR_WRITE_4(&pInfo->hw, mc_filter[1], MAC_REG_MAR+4);
        }
        BYTE_REG_BITS_ON(&pInfo->hw, (RCR_AM|RCR_AB), MAC_REG_RCR);
    }
    //CSR_WRITE_1(&pInfo->hw, (pInfo->hw.sOpts.rx_thresh<<5) | rx_mode, MAC_REG_RCR);
    spin_unlock_irqrestore(&pInfo->lock, flags);
}

static struct net_device_stats *rhine_get_stats(struct net_device *dev) {
    PRHINE_INFO pInfo = netdev_priv(dev);

    return &pInfo->stats;
}

static int  rhine_ioctl(struct net_device *dev, struct ifreq *rq, int cmd) {

    switch(cmd) {
#ifdef RHINE_ETHTOOL_IOCTL_SUPPORT
    case SIOCETHTOOL:
        return rhine_ethtool_ioctl(dev, rq);
        break;
#endif

#ifdef RHINE_MII_IOCTL_SUPPORT
    case SIOCGMIIPHY:       /* Get address of MII PHY in use. */
    case SIOCGMIIREG:       /* Read MII PHY register. */
    case SIOCSMIIREG:       /* Write to MII PHY register. */
        return rhine_mii_ioctl(dev, rq, cmd);
        break;
#endif

    default:
        return -EOPNOTSUPP;
    }
    return 0;
}



/*------------------------------------------------------------------*/

MODULE_DEVICE_TABLE(pci, rhine_id_table);

static struct pci_driver rhine_driver = {
        name:       RHINE_NAME,
        id_table:   rhine_id_table,
        probe:      rhine_found1,
        remove:     rhine_remove1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)
#ifdef CONFIG_PM
        suspend:    rhine_suspend,
        resume:     rhine_resume,
#endif
#endif
};

static int __init rhine_init_module(void)
{
    int ret;
    ret=pci_module_init(&rhine_driver);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)
#ifdef CONFIG_PM
    register_inetaddr_notifier(&rhine_inetaddr_notifier);
    if(ret >= 0)
        register_reboot_notifier(&rhine_notifier);

#endif
#endif

    return ret;
}

static void __exit rhine_cleanup_module(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)
#ifdef CONFIG_PM
    unregister_reboot_notifier(&rhine_notifier);
    unregister_inetaddr_notifier(&rhine_inetaddr_notifier);
#endif
#endif
    pci_unregister_driver(&rhine_driver);
}

module_init(rhine_init_module);
module_exit(rhine_cleanup_module);







//================================================
// ETHTOOL ioctl support routine
//================================================
#ifdef RHINE_ETHTOOL_IOCTL_SUPPORT
static
int rhine_ethtool_ioctl (
    struct net_device*  dev,
    struct ifreq*       ifr
    )
{
    struct ethtool_cmd  ecmd;
    PRHINE_INFO         pInfo = netdev_priv(dev);
    unsigned long       flags;

    if (copy_from_user(&ecmd, ifr->ifr_data, sizeof(ecmd.cmd))) {
        return -EFAULT;
    }

    switch (ecmd.cmd) {
    case ETHTOOL_GSET: {
        U32 status;

        spin_lock_irqsave(&pInfo->lock,flags);
        status = rhine_check_media_mode(&pInfo->hw);
        spin_unlock_irqrestore(&pInfo->lock, flags);

        ecmd.supported = SUPPORTED_TP            |
                         SUPPORTED_Autoneg       |
                         SUPPORTED_10baseT_Half  |
                         SUPPORTED_10baseT_Full  |
                         SUPPORTED_100baseT_Half |
                         SUPPORTED_100baseT_Full;

        if (status & RHINE_SPEED_100)
            ecmd.speed = SPEED_100;
        else
            ecmd.speed=SPEED_10;

        ecmd.autoneg = (status & RHINE_AUTONEG_ENABLE) ? AUTONEG_ENABLE : AUTONEG_DISABLE;
        ecmd.port = PORT_TP;
        ecmd.transceiver = XCVR_INTERNAL;
        ecmd.phy_address = CSR_READ_1(&pInfo->hw, MAC_REG_MIIAD) & 0x1F;

        if (status & RHINE_DUPLEX_FULL)
            ecmd.duplex = DUPLEX_FULL;
        else
            ecmd.duplex = DUPLEX_HALF;

        if (copy_to_user(ifr->ifr_data, &ecmd, sizeof(ecmd)))
            return -EFAULT;
    } break;

    case ETHTOOL_SSET: {
        SPD_DPX_OPT connection_type=SPD_DPX_AUTO;
        
        if (!capable(CAP_NET_ADMIN)) {
            return -EPERM;
        }

        if (BYTE_REG_BITS_IS_ON(&pInfo->hw, CFGC_MEDEN, MAC_REG_CFGC)) {
            printk(KERN_INFO "%s: The media mode have been forced by EEPROM utiltiy\n", dev->name);
            return -EPERM;
        }

        if (copy_from_user(&ecmd, ifr->ifr_data, sizeof(ecmd)))
            return -EFAULT;


        if( ecmd.autoneg == 1)
            connection_type = SPD_DPX_AUTO;
        else
        {
            if(ecmd.speed == SPEED_100){
                if(ecmd.duplex == DUPLEX_FULL)
                    connection_type = SPD_DPX_100_FULL;
                else
                    connection_type = SPD_DPX_100_HALF;
            } else if(ecmd.speed == SPEED_10) {
                if(ecmd.duplex == DUPLEX_FULL)
                    connection_type = SPD_DPX_10_FULL;
                else
                    connection_type = SPD_DPX_10_HALF;
            }
               
        }

        // [4.38] Save ConnectionType Info when via Ethertool
        if( connection_type != pInfo->hw.sOpts.spd_dpx)
        {
            pInfo->hw.sOpts.spd_dpx = connection_type;
            spin_lock_irqsave(&pInfo->lock, flags);
            rhine_set_media_mode(&pInfo->hw, &pInfo->hw.sOpts);
            spin_unlock_irqrestore(&pInfo->lock, flags);
        }

    } break;

#ifdef ETHTOOL_GLINK
    case ETHTOOL_GLINK: {
        struct ethtool_value info;
        memset((void *) &info, 0, sizeof (info));
        info.cmd = ETHTOOL_GLINK;
        info.data = BYTE_REG_BITS_IS_ON(&pInfo->hw, MIISR_LNKFL,MAC_REG_MIISR)
                    ? FALSE : TRUE;

        if (copy_to_user(ifr->ifr_data, &info, sizeof (info)))
            return -EFAULT;
        }
        break;
#endif

#ifdef ETHTOOL_GDRVINFO
    case ETHTOOL_GDRVINFO:
        {
            struct ethtool_drvinfo info={ETHTOOL_GDRVINFO};
            strcpy(info.driver, RHINE_NAME);
            strcpy(info.version, RHINE_VERSION);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
            strcpy(info.bus_info, pci_name(pInfo->pcid));
#else
            strcpy(info.bus_info, pInfo->pcid->slot_name);
#endif
            if (copy_to_user(ifr->ifr_data, &info, sizeof(info)))
                return -EFAULT;
        }
        break;
#endif

#ifdef ETHTOOL_GWOL
    case ETHTOOL_GWOL: {

        struct ethtool_wolinfo  wol={ETHTOOL_GWOL};
        memset(&wol,0,sizeof(wol));

        if (pInfo->hw.byRevId >= REV_ID_VT6102_A) {
            wol.supported=WAKE_PHY | WAKE_MAGIC | WAKE_UCAST |
                    WAKE_ARP;
            wol.wolopts|=WAKE_MAGIC;
            if (pInfo->wol_opts & RHINE_WOL_PHY)
                wol.wolopts|=WAKE_PHY;
            if (pInfo->wol_opts & RHINE_WOL_UCAST)
                wol.wolopts|=WAKE_UCAST;
            if (pInfo->wol_opts & RHINE_WOL_ARP)
                wol.wolopts|=WAKE_ARP;

            memcpy(&wol.sopass,pInfo->wol_passwd,6);

        } else
            return -EFAULT;

        if (copy_to_user(ifr->ifr_data, &wol,sizeof(wol)))
            return -EFAULT;
        }
        break;
#endif

#ifdef ETHTOOL_SWOL
    case ETHTOOL_SWOL: {
        struct ethtool_wolinfo  wol;

        if (copy_from_user(&wol, ifr->ifr_data, sizeof(wol)))
            return -EFAULT;

        if (!(wol.wolopts & (WAKE_PHY|WAKE_MAGIC|WAKE_UCAST|WAKE_ARP)))
            return -EFAULT;
        pInfo->wol_opts=RHINE_WOL_MAGIC;
        if (pInfo->hw.byRevId >= REV_ID_VT6102_A) {
            if (wol.wolopts & WAKE_PHY) {
                pInfo->wol_opts|=RHINE_WOL_PHY;
                pInfo->hw.flags |=RHINE_FLAGS_WOL_ENABLED;
            }
            if (wol.wolopts & WAKE_MAGIC) {
                pInfo->wol_opts|=RHINE_WOL_MAGIC;
                pInfo->hw.flags |=RHINE_FLAGS_WOL_ENABLED;
            }
            if (wol.wolopts & WAKE_UCAST) {
                pInfo->wol_opts|=RHINE_WOL_UCAST;
                pInfo->hw.flags |=RHINE_FLAGS_WOL_ENABLED;
            }
            if (wol.wolopts & WAKE_ARP) {
                pInfo->wol_opts|=RHINE_WOL_ARP;
                pInfo->hw.flags |=RHINE_FLAGS_WOL_ENABLED;
            }
            memcpy(pInfo->wol_passwd,wol.sopass,6);
        }
        else
            return -EFAULT;

        if (copy_to_user(ifr->ifr_data, &wol,sizeof(wol)))
            return -EFAULT;
    }
    break;
#endif


#ifdef ETHTOOL_GMSGLVL
    case ETHTOOL_GMSGLVL: {
        struct ethtool_value edata={ETHTOOL_GMSGLVL};
        edata.data=msglevel;
        if (copy_to_user(ifr->ifr_data, &edata,sizeof(edata)))
            return -EFAULT;
        }
        break;
#endif

#ifdef ETHTOOL_SMSGLVL
    case ETHTOOL_SMSGLVL: {
        struct ethtool_value edata={ETHTOOL_SMSGLVL};
        if (copy_from_user(&edata, ifr->ifr_data, sizeof(edata)))
            return -EFAULT;
        msglevel=edata.data;
        }
        break;
#endif
    default:
        return -EOPNOTSUPP;
    }
    return 0;
}

#endif

/***************************************************************************
*    MII ioctl support routine
****************************************************************************/
#ifdef RHINE_MII_IOCTL_SUPPORT
static int  rhine_mii_ioctl(struct net_device* dev, struct ifreq* ifr,int cmd) {
    PRHINE_INFO pInfo = netdev_priv(dev);
    unsigned long   flags;
    struct mii_ioctl_data* pMiiData=(struct mii_ioctl_data*) &(ifr->ifr_data);

    switch(cmd) {
    case SIOCGMIIPHY:
        pMiiData->phy_id=CSR_READ_1(&pInfo->hw, MAC_REG_MIIAD) & 0x1f;
        break;

    case SIOCGMIIREG:
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;
        spin_lock_irqsave(&pInfo->lock,flags);
        rhine_mii_read(&pInfo->hw,pMiiData->reg_num & 0x1f, &(pMiiData->val_out));
        spin_unlock_irqrestore(&pInfo->lock, flags);
        break;

    case SIOCSMIIREG:
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;
        spin_lock_irqsave(&pInfo->lock,flags);
        rhine_mii_write(&pInfo->hw,pMiiData->reg_num & 0x1f,pMiiData->val_in);
        spin_unlock_irqrestore(&pInfo->lock, flags);
        break;
    default:
          return -EOPNOTSUPP;
    }
    return 0;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,9)

#ifdef CONFIG_PM
static int
rhine_notify_reboot(struct notifier_block *nb, unsigned long event, void *p)
{
    struct pci_dev *pcid = NULL;

    switch(event) {
    case SYS_DOWN:
    case SYS_HALT:
    case SYS_POWER_OFF:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        while ((pcid = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pcid)) != NULL) {
#else
        pci_for_each_dev(pcid) {
#endif
            if (pci_dev_driver(pcid) == &rhine_driver) {
                if (pci_get_drvdata(pcid))
                    rhine_suspend(pcid, PCI_D3hot);
            }
        }
    }
    return NOTIFY_DONE;
}

static int
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
rhine_suspend(struct pci_dev *pcid, pm_message_t state)
#else
rhine_suspend(struct pci_dev *pcid, u32 state)
#endif
{
    PRHINE_INFO pInfo = pci_get_drvdata(pcid);
    struct net_device *dev = pInfo->dev;
    int power_status;       // to silence the compiler

    netif_stop_queue(dev);
    spin_lock_irq(&pInfo->lock);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
    pci_save_state(pcid);
#else
    pci_save_state(pcid, pInfo->pci_state);
#endif

#ifdef ETHTOOL_GWOL
        rhine_shutdown(pInfo);
    if (pInfo->hw.flags & RHINE_FLAGS_WOL_ENABLED) {
        rhine_get_ip(pInfo);
        rhine_save_mac_context(pInfo, &pInfo->mac_context);
        rhine_save_pci_context(pInfo, &pInfo->pci_context);
        rhine_set_wol(pInfo);
        power_status = pci_enable_wake(pcid, PCI_D3hot, 1);
        power_status = pci_set_power_state(pcid, PCI_D3hot);
    } else {
        rhine_save_mac_context(pInfo,&pInfo->mac_context);
        rhine_save_pci_context(pInfo, &pInfo->pci_context);
        pci_disable_device(pcid);
        power_status = pci_set_power_state(pcid, state);
    }
#else
    pci_disable_device(pcid);
    power_status = pci_set_power_state(pcid, state);
#endif
    spin_unlock_irq(&pInfo->lock);
    return 0;
}

static int
rhine_resume(struct pci_dev *pcid)
{
    PRHINE_INFO pInfo = pci_get_drvdata(pcid);
    struct net_device *dev = pInfo->dev;
    unsigned long flags;
    int power_status;       // to silence the compiler

    power_status = pci_set_power_state(pcid, PCI_D0);
    power_status = pci_enable_wake(pcid, PCI_D0, 0);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
    pci_restore_state(pcid);
#else
    pci_restore_state(pcid, pInfo->pci_state);
#endif
    rhine_enable_mmio(&pInfo->hw);

    rhine_wol_reset(&pInfo->hw);

    if (netif_running(dev)) {
        spin_lock_irqsave(&pInfo->lock, flags);
        rhine_restore_mac_context(pInfo, &pInfo->mac_context);
        rhine_restore_pci_context(pInfo, pInfo->pci_context);
        rhine_init_adapter(pInfo, RHINE_INIT_WOL);
        rhine_tx_srv(pInfo, 0);
        rhine_enable_int(&pInfo->hw, 0x00000000UL);
        spin_unlock_irqrestore(&pInfo->lock, flags);
    }

    return 0;
}

static int
rhine_netdev_event(struct notifier_block *nb,
    unsigned long notification, void *ptr) {
    struct in_ifaddr* ifa=(struct in_ifaddr*) ptr;
    struct net_device *dev;
    PRHINE_INFO pInfo;

    if (ifa)  {
        dev=ifa->ifa_dev->dev;
        for (pInfo=pRhine3_Infos; pInfo; pInfo=pInfo->next) {
            if (pInfo->dev == dev)
                rhine_get_ip(pInfo);
        }
    }
    return NOTIFY_DONE;
}
#endif /*CONFIG_PM*/

#endif /*VERSION >= 2.4.9*/

