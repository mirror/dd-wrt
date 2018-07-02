/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#define _BCMENET_LOCAL_

#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "bcmswdefs.h"
#include "bcmsw.h"
/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "boardparms.h"
#include "bcmenet.h"
#if defined(CONFIG_BCM_GMAC)
#include "bcmgmac.h"
#endif
#include "bcmswaccess.h"
#include "eth_pwrmngt.h"

#ifndef SINGLE_CHANNEL_TX
/* for enet driver txdma channel selection logic */
extern int channel_for_queue[NUM_EGRESS_QUEUES];
/* for enet driver txdma channel selection logic */
extern int use_tx_dma_channel_for_priority;
#endif /*SINGLE_CHANNEL_TX*/

extern extsw_info_t extSwInfo;
extern uint32_t logicalport_to_imp_map[];
extern uint32_t imp_pbmap[];
extern uint32_t g_imp_use_lag;

#if defined(CONFIG_BCM_GMAC)
void gmac_hw_stats( int port,  struct net_device_stats *stats );
int gmac_dump_mib(int port, int type);
void gmac_reset_mib( void );
#endif

#define SWITCH_ADDR_MASK                   0xFFFF
#define ALL_PORTS_MASK                     0x1FF
#define ONE_TO_ONE_MAP                     0x00FAC688
#define MOCA_QUEUE_MAP                     0x0091B492
#define DEFAULT_FC_CTRL_VAL                0x1F
/* Tx: 0->0, 1->1, 2->2, 3->3. */
#define DEFAULT_IUDMA_QUEUE_SEL            0x688


/* Forward declarations */
extern int32 g_addl_wan ;
extern spinlock_t bcm_extsw_access;
uint8_t  port_in_loopback_mode[TOTAL_SWITCH_PORTS] = {0};
#if defined(STAR_FIGHTER2)
void extsw_rreg_mmap(int page, int reg, uint8 *data_out, int len);
void extsw_wreg_mmap(int page, int reg, uint8 *data_in, int len);
void sf2_enable_2_5g(void);
void sf2_mdio_master_enable(void);
void sf2_mdio_master_disable(void);
static int sf2_bcmsw_dump_mib_ext(int port, int type);
static void sf2_dump_page0(void);
static void sf2_tc_to_cos_default(void);
void sf2_rgmii_config(void);
void sf2_qos_default(void);
static void sf2_conf_que_thred(void);
void sf2_conf_acb_conges_profile(int profile);
static int sf2_config_acb(struct ethswctl_data *e);
static int sf2_port_erc_config(struct ethswctl_data *e);
static int sf2_port_shaper_config(struct ethswctl_data *e);
static int sf2_pid_to_priority_mapping(struct ethswctl_data *e);
static int sf2_cos_priority_method_config(struct ethswctl_data *e);
typedef struct port_reg_offset_s{
        uint32 rgmii_ctrl;
        uint32 rgmii_rx_clock_delay_ctrl;
        uint16 pad_offset;
        uint16 ext_port;
} port_reg_offset_t;

port_reg_offset_t rgmii_port_reg_offset [] = {
#if defined(CONFIG_BCM94908)
             {SF2_P11_RGMII_CTRL_REGS, SF2_P11_RGMII_RX_CLK_DELAY_CTRL, 3, 11},
#else
             {SF2_P5_RGMII_CTRL_REGS,  SF2_P5_RGMII_RX_CLK_DELAY_CTRL,  1, 5},
             {SF2_P7_RGMII_CTRL_REGS,  SF2_P7_RGMII_RX_CLK_DELAY_CTRL,  2, 7},
             {SF2_P11_RGMII_CTRL_REGS, SF2_P11_RGMII_RX_CLK_DELAY_CTRL, 3, 11},
             {SF2_P12_RGMII_CTRL_REGS, SF2_P12_RGMII_RX_CLK_DELAY_CTRL, 0, 12},
#endif
};

static port_reg_offset_t *get_rgmii_regs(int ext_port)
{
    int i;

    for (i=0; i<ARRAY_SIZE(rgmii_port_reg_offset); i++)
    {
        if (rgmii_port_reg_offset[i].ext_port == ext_port) return &rgmii_port_reg_offset[i];
    }
    return NULL;
}

typedef struct acb_config_s {
    uint16 total_xon_hyst;
    uint16 xon_hyst;
    acb_queue_config_t acb_queue_config[64];
} acb_config_t;

#if defined(STAR_FIGHTER2)
#define MaxStreamNumber 40
static uint32_t wanQueMap = DefaultWANQueBitMap,
                maxStreamNumber = MaxStreamNumber, queThreConfMode;
uint32_t queRemap = DefaultQueRemap;
static uint16_t sf2_sw_port_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];
static uint16_t sf2_imp0_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES];
#endif

static acb_config_t acb_profiles [] = {
    // profile 1
    {
        .total_xon_hyst = 6,
        .xon_hyst = 4,
        {
            // queue 0, (port 0) for LAN->LAN
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            {
                .pessimistic_mode = 0,
                .total_xon_en = 1,
                .pkt_len = 0,
                .xoff_threshold = 16,
            },
#else // For 148
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
#endif
            // queue 1
            {0},
            // queue 2
            {0},
            // queue 3
            {0},
            // queue 4
            {0},
            // queue 5
            {0},
            // queue 6
            {0},
            // queue 7, for IMP->LAN
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
            // queue 8 (port 1, q 0)
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
            // queue 9
            {0},
            // queue 10
            {0},
            // queue 11
            {0},
            // queue 12
            {0},
            // queue 13
            {0},
            // queue 14
            {0},
            // queue 15
            {
                .pessimistic_mode = 1,
                .total_xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
        },

    },
    // profile 2
    #if 0
    {
        .total_xon_hyst = 6,
        .xon_hyst = 4,
        {
            // queue 0
            {
                .pessimistic_mode = 1,
                .xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
            // queue 1
            {0},
            // queue 2
            {0},
            // queue 3
            {0},
            // queue 4
            {0},
            // queue 5
            {0},
            // queue 6
            {0},
            // queue 7, for IMP->LAN
            {
                .pessimistic_mode = 1,
                .xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q8, q9, q10, q11, q12, q13, q14, q15
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q16, q17, q18, q19, q20, q21, q22, q23
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q24, q25, q26, q27, q28, q29, q30, q31
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q32, q33, q34, q35, q36, q37, q38, q39
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q40, q41, q42, q43, q44, q45, q46, q47
            {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, // q48, q49, q50, q51, q52, q53, q54, q55
                            // P7 q0 (q56)
            {
                .pessimistic_mode = 1,
                .xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },
             {0}, {0}, {0}, {0}, {0}, {0},    // q57, q58, q59, q60, q61, q62
            {               // P7 q7 (q63)
                .pessimistic_mode = 1,
                .xon_en = 1,
                .pkt_len = 6,
                .xoff_threshold = 16,
            },

        },
    }
    #endif
};
#endif

/*  
    Maxmum Garanteed Streams, total buffers will support 96 streams of Jumbo frames. 
*/

/************************/
/* Ethernet Switch APIs */
/************************/

/* Stats API */
typedef enum bcm_hw_stat_e {
    TXOCTETSr = 0,
    TXDROPPKTSr,
    TXQOSPKTSr,
    TXBROADCASTPKTSr,
    TXMULTICASTPKTSr,
    TXUNICASTPKTSr,
    TXCOLLISIONSr,
    TXSINGLECOLLISIONr,
    TXMULTIPLECOLLISIONr,
    TXDEFERREDTRANSMITr,
    TXLATECOLLISIONr,
    TXEXCESSIVECOLLISIONr,
    TXFRAMEINDISCr,
    TXPAUSEPKTSr,
    TXQOSOCTETSr,
    RXOCTETSr,
    RXUNDERSIZEPKTSr,
    RXPAUSEPKTSr,
    PKTS64OCTETSr,
    PKTS65TO127OCTETSr,
    PKTS128TO255OCTETSr,
    PKTS256TO511OCTETSr,
    PKTS512TO1023OCTETSr,
    PKTS1024TO1522OCTETSr,
    RXOVERSIZEPKTSr,
    RXJABBERSr,
    RXALIGNMENTERRORSr,
    RXFCSERRORSr,
    RXGOODOCTETSr,
    RXDROPPKTSr,
    RXUNICASTPKTSr,
    RXMULTICASTPKTSr,
    RXBROADCASTPKTSr,
    RXSACHANGESr,
    RXFRAGMENTSr,
    RXEXCESSSIZEDISCr,
    RXSYMBOLERRORr,
    RXQOSPKTSr,
    RXQOSOCTETSr,
    PKTS1523TO2047r,
    PKTS2048TO4095r,
    PKTS4096TO8191r,
    PKTS8192TO9728r,
    MAXNUMCNTRS,
} bcm_hw_stat_t;


typedef struct bcm_reg_info_t {
    uint8_t offset;
    uint8_t len;
} bcm_reg_info_t;

#if defined(CONFIG_BCM_EXT_SWITCH)

void extsw_setup_imp_ports(void)
{
     uint8_t  val8;
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
     uint16_t val16;
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
    /* Assumption : External switch is always in MANAGED Mode w/ TAG enabled.
     * BRCM TAG enable in external switch is done via MDK as well
     * but it is not deterministic when the userspace app for external switch
     * will run. When it gets delayed and the device is already getting traffic,
     * all those packets are sent to CPU without external switch TAG.
     * To avoid the race condition - it is better to enable BRCM_TAG during driver init. */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    val8 &= (~(BRCM_HDR_EN_GMII_PORT_5|BRCM_HDR_EN_IMP_PORT)); /* Reset HDR_EN bit on both ports */
    val8 |= BRCM_HDR_EN_IMP_PORT; /* Set only for IMP Port */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* NOTE : Forcing these setting here; SWMDK doesn't setup IMP when multiple IMP ports in-use */

    /* Enable IMP Port */
    val8 = ENABLE_MII_PORT | RECEIVE_BPDU;
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, sizeof(val8));

    /* management mode, enable forwarding */
    extsw_rreg_wrap(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));
    val8 |= REG_SWITCH_MODE_FRAME_MANAGE_MODE | REG_SWITCH_MODE_SW_FWDG_EN;
    extsw_wreg_wrap(PAGE_CONTROL, REG_SWITCH_MODE, &val8, sizeof(val8));

    /* setup Switch MII1 port state override : 2000M with flow control enabled */
    val8 = IMP_LINK_OVERRIDE_2000FDX /*| REG_CONTROL_MPSO_FLOW_CONTROL*/; /* FIXME : Enabling flow control creates some issues */
    extsw_wreg_wrap(PAGE_CONTROL, REG_CONTROL_MII1_PORT_STATE_OVERRIDE, &val8, sizeof(val8));

    /* enable rx bcast, ucast and mcast of imp port */
    val8 = REG_MII_PORT_CONTROL_RX_UCST_EN | REG_MII_PORT_CONTROL_RX_MCST_EN |
           REG_MII_PORT_CONTROL_RX_BCST_EN;
    extsw_wreg_wrap(PAGE_CONTROL, REG_MII_PORT_CONTROL, &val8, sizeof(val8));

    /* Forward lookup failure to use ULF/MLF/IPMC lookup fail registers */
    val8 = REG_PORT_FORWARD_MCST | REG_PORT_FORWARD_UCST | REG_PORT_FORWARD_IP_MCST;
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_FORWARD, &val8, sizeof(val8));

    /* Forward unlearned unicast and unresolved mcast to the MIPS */
    val16 = PBMAP_MIPS;
    extsw_wreg_wrap(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &val16, sizeof(val16));
    extsw_wreg_wrap(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &val16, sizeof(val16));
    extsw_wreg_wrap(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &val16, sizeof(val16));

    /* Disable learning on MIPS*/
    val16 = PBMAP_MIPS;
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &val16, sizeof(val16));

    /* NOTE : All regular setup for P8 IMP is done above ; Same as what SWMDK would do*/

    /* Enable BRCM TAG on P5 */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    val8 |= BRCM_HDR_EN_GMII_PORT_5; /* Enable BRCM TAG on P5 */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL, &val8, sizeof(val8));
    /* Enale Link - port override register */
    extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE+5, &val8, sizeof(val8));
    val8 = LINK_OVERRIDE_1000FDX; /* Enable 1000FDX Link */
    val8 |= REG_PORT_GMII_SPEED_UP_2G; /* Speed up to 2G */
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE+5, &val8, sizeof(val8));

    imp_pbmap[1] |= PBMAP_P5_IMP ; /* Add P5 to the list of IMP ports */

    /* Enable BRCM TAG on P4 */
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL2, &val8, sizeof(val8));
    val8 = BRCM_HDR_EN_P4; /* Enable BRCM TAG on P4 */
    extsw_wreg_wrap(PAGE_MANAGEMENT, REG_BRCM_HDR_CTRL2, &val8, sizeof(val8));
    /* Enale Link - port override register */
    extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE+4, &val8, sizeof(val8));
    val8 = LINK_OVERRIDE_1000FDX; /* Enable 1000FDX Link */
    extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE+4, &val8, sizeof(val8));

    imp_pbmap[1] |= PBMAP_P4_IMP ; /* Add P4 to the list of IMP ports */

#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
}
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
void bcmsw_print_imp_port_grouping(unsigned long port_map)
{
    int port, imp_port, new_grp = 0;
    printk("NOTE: Using Port Grouping for IMP ports : ");
    for (imp_port = 0; imp_port <= BP_MAX_SWITCH_PORTS; imp_port++)
    {
        /* Not an IMP port -- continue */
        if (! ( (1<<imp_port) & (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP) ) ) continue;
        new_grp = 1;
        for (port = 0; port < BP_MAX_SWITCH_PORTS; port++) 
        {
            if ( ((1<<port) & port_map) && 
                 logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port,1)] == imp_port )
            {
                if (new_grp)
                {
                    printk("[");
                    new_grp = 0;
                }
                else
                {
                    printk(",");
                }
                printk(" %d",port);
            }
        }
        if (!new_grp)
        {
            printk(" --> %d ] ",imp_port);
        }
    }
    printk("\n");
}
#endif
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)

/* Define the mapping here for now */
static int port_imp_map[BP_MAX_SWITCH_PORTS] = 
                        {
                            /* P0 */  P4_PORT_ID,
                            /* P1 */  P5_PORT_ID,
                            /* P2 */  P5_PORT_ID,
                            /* P3 */  IMP_PORT_ID, /* 8 /*/
                            /* P4 */  -1,   /* IMP Port */
                            /* P5 */  -1,   /* IMP Port */
                            /* P6 */  -1,   /* Unused/undefined switch port */
                            /* P7 */  IMP_PORT_ID /* 8 */
                        };
#endif
void extsw_setup_imp_fwding(void)
{
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    unsigned char unit = 1, port, v8;
    unsigned long port_map;
    uint16 v16;
    uint32 v32;
    int crossbar_port;
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info = &EnetInfo[unit];
#if defined(BCM_ENET_MULTI_IMP_SUPPORT_USE_LAG)
    g_imp_use_lag = 1;
#endif

    port_map = enet_get_portmap(unit); /* Get port map for external switch */

    if (!g_imp_use_lag)
    {
        /* Make decision based on number of SF2 ports configured in board parameters;
           4908 supports only 5 LAN Ports on SF2 -
           P0-P3 = Quad GPHY                     
           P7 = Crossbar (1 GPHY, 1 RGMII/RvMII/TMII or 1 SERDES (2.5G) 
           Decision tree :
           1. If no crossbar LAN ports i.e. P7 is not used >>> Use port based forwarding
           2. If crossbar LAN ports i.e. P7 is in use but NOT connected to SERDES >>> Use port based forwarding
           3. TBD : if P7 is connected to SERDES and one of the other LAN ports P0-P3 are not used >>> Use Port based forwarding
           4. Otherwise use LAG */

        for (port = BP_CROSSBAR_PORT_BASE; (port < BCMENET_MAX_PHY_PORTS); port++) /* go through all the ports including crossbar */
        {
            /* No need to check BP_IS_CROSSBAR_PORT -- loop starts with CB port base */
            crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(port);
            if (!BP_IS_CROSSBAR_PORT_DEFINED(info->sw, crossbar_port)) continue;
            if (!IsSerdes(info->sw.crossbar[crossbar_port].phy_id)) continue;
            /* Serdes is configured - use LAG */
            g_imp_use_lag = 1;
            break;
        }

    }
    if (g_imp_use_lag)
    {
        int grp_no = 1; /* Switch only supports two LAG/Trunk groups - use the second one */
#if defined(CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT)
        /* Check against board parameters for Trunk Group conflict -- User not allowed to use 2nd Group */
        unsigned int port_flags;
        unsigned int port_trunk_grp = 0;
        for (port = 0; (port_trunk_grp == 0 && port < BCMENET_MAX_PHY_PORTS); port++) /* go through all the ports including crossbar */
        {
            /* Not sure where the TRUNK_GRP will be added in port_flags in case of crossbar port.
               Assumption is that it is a port property so should be part of real port and not crossbar port struct.
               But playing safe for now. */
            if (BP_IS_CROSSBAR_PORT(port))
            {
                crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(port);
                if (BP_IS_CROSSBAR_PORT_DEFINED(info->sw, crossbar_port))
                {
                    port_flags = info->sw.crossbar[crossbar_port].port_flags;
                }
            }
            else
            {
                port_flags = info->sw.port_flags[port];
            }
            if ( IsPortTrunkGrpEnabled(port_flags) ) 
            {
                port_trunk_grp = PortTrunkGroup(port_flags);
            }
        }

        BCM_ASSERT( (port_trunk_grp != 1) );

#endif /* CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT */
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);

        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_IMP0_TRUNK_CTL, &v16, 2);
        v16 |= (PBMAP_MIPS| PBMAP_P5_IMP | PBMAP_P4_IMP );
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_IMP0_TRUNK_CTL, &v16, 2);

        v16 = ( ( (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP ) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);

        /* P8/EMAC0=2.5G, P5/EMAC1=2.5G and P4/EMAC2=1.4G = 6.4G
         * Total Hash buckets = 256 --> P4=56, P5=100, P8=100 */
        v32 = HASH_WT_TRUNK_CTL_OVRD;
        v32 |= ((55 & HAS_WT_MEM_M) << HAS_WT_MEM_0_S);
        v32 |= ((155 & HAS_WT_MEM_M) << HAS_WT_MEM_1_S);
        v32 |= ((255 & HAS_WT_MEM_M) << HAS_WT_MEM_2_S);
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_HASH_WT_TRUNK_CTL + (4*grp_no), &v32, sizeof(v32));

        printk("NOTE: Using LAG/Port-Trunking for IMP ports\n");
    }
    else
    {
        /* Configure the Lookup failure registers to P4, P5, P8 */
        v16 = (PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP );
        extsw_wreg_wrap(PAGE_CONTROL, REG_UCST_LOOKUP_FAIL, &v16, sizeof(v16));
        extsw_wreg_wrap(PAGE_CONTROL, REG_MCST_LOOKUP_FAIL, &v16, sizeof(v16));
        extsw_wreg_wrap(PAGE_CONTROL, REG_IPMC_LOOKUP_FAIL, &v16, sizeof(v16));
        /* Disable learning on MIPS and P4/P5*/
        v16 = PBMAP_MIPS | PBMAP_P5_IMP | PBMAP_P4_IMP ;
        extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &v16, sizeof(v16));

        /* Any reason to make this logic more flexible -- based on boardparms portmap ??? */

        /* Configure forward based on Port Grouping
         * By default all port's pbvlan is 0x1FF */
        port_map = enet_get_portmap(unit); /* Get port map for external switch */
        for (port = 0; port < BP_MAX_SWITCH_PORTS; port++)
        {
            if ( (port_imp_map[port] != -1) && ( (1<<port) & port_map ) )
            {
                v16 = PBMAP_ALL & ~(PBMAP_P5_IMP | PBMAP_P4_IMP | PBMAP_MIPS);
                v16 |= (1<<port_imp_map[port]);
                extsw_set_pbvlan(port, v16);
                logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port,1)] = port_imp_map[port];
            }
        }

        bcmsw_print_imp_port_grouping(port_map);
    }
#endif
}

void extsw_rgmii_config(void)
{
    unsigned char data8;
    unsigned char unit = 1, port;
    uint16 v16;
    int phy_id, rgmii_ctrl;
    unsigned long port_map;
    unsigned long port_flags;

#if defined(STAR_FIGHTER2)
    sf2_rgmii_config();
    return;
#endif
    port_map = enet_get_portmap(unit);

    for (port = 0; port <= MAX_SWITCH_PORTS; port++)
    {
        if (port == MIPS_PORT_ID) {
            rgmii_ctrl = EXTSW_REG_RGMII_CTRL_IMP;
            port_flags = 0;
        }
        else {
            if (!(port_map & (1<<port))) {
                continue;
            }

            phy_id =  enet_logport_to_phyid(PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit));
            if (!IsRGMII(phy_id)) {
                continue;
            }

            if (port == EXTSW_RGMII_PORT) {
                rgmii_ctrl = REG_RGMII_CTRL_P5;
            }
            else {
                continue;
            }
            port_flags = enet_get_port_flags(unit, port);
        }

        extsw_rreg_wrap( PAGE_CONTROL, rgmii_ctrl, &data8, sizeof(data8));
        data8 |= EXTSW_RGMII_TXCLK_DELAY;
        data8 &= ~EXTSW_RGMII_RXCLK_DELAY;

        /* TXID default is on so no need to check IsPortTxInternalDelay(port_flags) */
        if (IsPortRxInternalDelay(port_flags)) {
            data8 |= EXTSW_RGMII_RXCLK_DELAY;
        }

        extsw_wreg_wrap(PAGE_CONTROL, rgmii_ctrl, &data8, sizeof(data8));
        if (port == MIPS_PORT_ID) {
            continue;
        }
        if (IsPortPhyConnected(unit, port)) {

            // GTXCLK to be  off (phy reg 0x1c, shadow 3 bit 9)  - phy defaults to ON
            // RXC skew to be on (phy reg 0x18, shadow 7 bit 8)  - phy defaults to ON

            v16 = MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S;
            ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);
            ethsw_phy_rreg(phy_id, MII_REGISTER_1C, &v16);
            v16 &= (~GTXCLK_DELAY_BYPASS_DISABLE);
            v16 |= MII_1C_WRITE_ENABLE;
            v16 &= ~(MII_1C_SHADOW_REG_SEL_M << MII_1C_SHADOW_REG_SEL_S);
            v16 |= (MII_1C_SHADOW_CLK_ALIGN_CTRL << MII_1C_SHADOW_REG_SEL_S);
            ethsw_phy_wreg(phy_id, MII_REGISTER_1C, &v16);

            /*
             * Follow it up with any overrides to the above Defaults.
             * It may be done in two stages.
             * 1. In the interim, customers may reprogramm the above RGMII defaults on the
             *    phy, by devising  a series of bp_mdio_init_t phy init data in boardparms.
             *    Ex.,  {bp_pPhyInit,                .u.ptr = (void *)g_phyinit_xxx},
             * 2. Eventually, if phyinit stuff above qualifies to become main stream,
             *    this is the place to code Rgmii phy config overrides.
             */
        }
    }//for all ports
}

static void extsw_addl_wan_config(void)
{
    unsigned char unit, port;
    uint16_t wan_port_map;

    if ( g_addl_wan != -1 )
    {
        unit = LOGICAL_PORT_TO_UNIT_NUMBER(g_addl_wan);
        port = LOGICAL_PORT_TO_PHYSICAL_PORT(g_addl_wan);
        if (unit != 1) /* Only External switch */
        { /* This check is only needed because we are calling external switch register read/write below */
            printk("ERROR : Additional WAN not on External switch\n");
            return;
        }
        /* Configure WAN port */
        extsw_rreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));
        wan_port_map |= (1<<port); /* Add the additional WAN port in the port map */
        extsw_wreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, sizeof(wan_port_map));

        /* Disable learning */
        extsw_rreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));
        wan_port_map |= (1<<port); /* Add the additional WAN port in the port map */
        extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, sizeof(wan_port_map));

        /* Only forward traffic from this port towards IMP port */
        extsw_set_pbvlan(port, PBMAP_MIPS);
        /* Age all dynamic learnt ARL on this port */
        extsw_fast_age_port(port, 0);
    }
}

#if defined(CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT)
void extsw_port_trunk_init(void)
{
    unsigned char unit = 1, port, sw_port, v8;
    uint16 v16;
    int enable_trunk = 0;
    unsigned long port_map, port_flags;
    int crossbar_port;
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info = &EnetInfo[unit];

    port_map = enet_get_portmap(unit); /* Get port map for external switch */

    BCM_ENET_DEBUG("%s : port_map <0x%04x> \n",__FUNCTION__, (unsigned int) port_map);
    for (port = 0; port_map && (port < BCMENET_MAX_PHY_PORTS); port++) /* go through all the ports including crossbar */
    {
        if (BP_IS_CROSSBAR_PORT(port)) /* Crossbar port ? Find the port_flags and actual switch port */
        {
            crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(port);
            if (!BP_IS_CROSSBAR_PORT_DEFINED(info->sw, crossbar_port)) continue;
            sw_port = info->sw.crossbar[crossbar_port].switch_port;
            port_flags = info->sw.crossbar[crossbar_port].port_flags;
        }
        else
        {
            sw_port = port;
            port_flags = info->sw.port_flags[port];
        }
        /* NOP if switch port is not part of map */
        if (!(port_map & (1<<sw_port) )) continue;
        /* Remove switch port from port map */
        port_map &= ~(1<<sw_port);
        BCM_ENET_DEBUG("%s : u=%d p=%d port_flags <0x%04x> port_map <0x%04x> \n",__FUNCTION__, unit, sw_port, (unsigned int) port_flags, (unsigned int) port_map );
        if ( IsPortTrunkGrpEnabled(port_flags) )
        {
            int grp_no = PortTrunkGroup(port_flags);
            extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
            v16 |= ( ( (1<<sw_port) & TRUNK_EN_GRP_M ) << TRUNK_EN_GRP_S );
            extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL + (2*grp_no), &v16, 2);
            enable_trunk = 1;
        }
    }
    if (enable_trunk)
    {
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S ); /* Enable Trunking */
        v8 |= ( (TRUNK_HASH_DA_SA_VID & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Default VID+DA+SA Hash */
        extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
        printk("LAG/Trunking enabled <0x%01x> <0x%02x>\n",v8,v16);
    }
}
#endif /* CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT */

void bcmeapi_conf_que_thred(void)
{
#if defined(STAR_FIGHTER2)
    if ( queThreConfMode == ThreModeDynamic)
    {
        sf2_conf_que_thred();
    }
#endif
}

void extsw_init_config(void)
{
    uint8_t v8;

    /* Retrieve external switch id - this can only be done after other globals have been initialized */
    if (extSwInfo.present) {
        uint8 val[4] = {0};

#if defined(STAR_FIGHTER2)

        /* Blocking the code below for now - this needs to be reworked as follows :
         * Ethernet Driver needs to bring in the GPHY work-around that is currently
         * put in CFE. If we reset the PHYs - that work-around is lost.
         * PHY Base address is currently being setup by CFE as well - so that should
         * be sufficient until Ethernet Driver is changed to work without CFE assistance.
         * Note : The CFE code in function bcm_ethsw_init() also sets the PHY address for
         * Single GPHY based on baseAddress - this is not done in below code. */
#if 0
        volatile u32 *quad_phy_ctrl = (void *)(SWITCH_BASE + SF2_QUAD_PHY_BASE_REG);
        uint16 bp_quad_base;
        // Shift Quad phy base address upfront in SF2_QUAD_PHY_BASE_REG

         if (BpGetGphyBaseAddress(&bp_quad_base) != BP_SUCCESS) {
             bp_quad_base = 1;
         }
         *quad_phy_ctrl = (bp_quad_base << SF2_QUAD_PHY_PHYAD_SHIFT) |SF2_QUAD_PHY_SYSTEM_RESET;
         udelay(128); // in micro sec
         *quad_phy_ctrl = bp_quad_base << SF2_QUAD_PHY_PHYAD_SHIFT;
         mdelay(128);

        /*
         * make SF2 the MDIO MASTER, so SF2 switch internal phy pages are auto refreshed.
         * These pages are referenced by link poll from swmdk process.
         *
         */
#endif
        sf2_enable_2_5g();
        sf2_mdio_master_enable();
#endif

        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_DEV_ID, (uint8 *)&val, 4);
        extSwInfo.switch_id = (*(uint32 *)val);

        extsw_setup_imp_ports();

        extsw_setup_imp_fwding();

        // RGMII
        extsw_rgmii_config();

        /* Initialize EEE on external switch */
        extsw_eee_init();

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        /* Power Savings: disable Tx/Rx MACs on unused ports */
        ethsw_shutdown_unused_macs(pVnetDev0_g);
#endif

        /* Configure Trunk groups if required */
        extsw_port_trunk_init();
        /* Configure additional experimental WAN interface */
        extsw_addl_wan_config();
        /* Set ARL AGE_DYNAMIC bit for aging operations */
        v8 = FAST_AGE_DYNAMIC;
        extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    }
#if defined(STAR_FIGHTER2)
   /* FIXME : We are polluting - need to find a way to not be so specific about SF2; Make all this generic */
   sf2_qos_default();

   if (queThreConfMode != ThreModeManual)
   {
       sf2_conf_que_thred(); // for acb testing
   }
   sf2_conf_acb_conges_profile(0);
#endif
}
#endif

/* Upon integration, the following function may be merged with extsw_rreg_wrap()
   and the appropriate cases that need swap will byte swap.
   Our expectation is that MBUS_MMAP case would Not need swap.
   Currently, this function always returns little endian data from history.
 */

void extsw_rreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_rreg: wrong length!\n");

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
          bcmsw_pmdio_rreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_rreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
#if defined(STAR_FIGHTER2)
      case MBUS_MMAP:
      // MMAP'PED external switch access
        extsw_rreg_mmap(page, reg, data, len);
        break;
#endif
      default:
        printk("Error Access Type %d: Neither SPI nor PMDIO access in %s <page:0x%x, reg:0x%x>\n",
            pVnetDev0_g->extSwitch->accessType, __func__, page, reg);
        break;
    }
    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                    __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);
}
/* Upon integration, the following function may be merged with extsw_rreg_wrap()
   and the appropriate cases that need swap will byte swap.
   Our expectation is that MBUS_MMAP case would Not need swap.
 */
void extsw_wreg(int page, int reg, uint8 *data, int len)
{
    if (((len != 1) && (len % 2) != 0) || len > 8)
        panic("extsw_wreg: wrong length!\n");

    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                   __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);

    switch (pVnetDev0_g->extSwitch->accessType)
    {
      case MBUS_MDIO:
        bcmsw_pmdio_wreg(page, reg, data, len);
        break;

      case MBUS_SPI:
      case MBUS_HS_SPI:
        bcmsw_spi_wreg(pVnetDev0_g->extSwitch->bus_num, pVnetDev0_g->extSwitch->spi_ss,
                       pVnetDev0_g->extSwitch->spi_cid, page, reg, data, len);
        break;
#if defined(STAR_FIGHTER2)
      case MBUS_MMAP:
      // MMAP'ed external switch access
        extsw_wreg_mmap(page, reg, data, len);
        break;
#endif
      default:
        printk("Error Access Type %d: Neither SPI nor PMDIO access in %s <page:0x%x, reg:0x%x>\n",
            pVnetDev0_g->extSwitch->accessType, __func__, page, reg);
        break;
    }
}

void extsw_fast_age_port(uint8 port, uint8 age_static)
{
    uint8 v8;
    uint8 timeout = 100;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC | FAST_AGE_PORT;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_PORT, &port, 1);

    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
    while (v8 & FAST_AGE_START_DONE) {
        mdelay(1);
        extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    v8 = FAST_AGE_DYNAMIC;
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, &v8, 1);
}

void extsw_set_wanoe_portmap(uint16 wan_port_map)
{
    int i;

#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
    /* can ignore wan_port_map */
    wan_port_map  = GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->softSwitchingMap, 1);
#else
    wan_port_map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->softSwitchingMap, 1);
#endif

    if (g_addl_wan != -1)
    {
        wan_port_map |= (1 << LOGICAL_PORT_TO_PHYSICAL_PORT(g_addl_wan));
    }
    extsw_wreg_wrap(PAGE_CONTROL, REG_WAN_PORT_MAP, &wan_port_map, 2);

    /* IMP port */
    wan_port_map |= imp_pbmap[1];

    wan_port_map |= GET_PORTMAP_FROM_LOGICAL_PORTMAP(pVnetDev0_g->learningDisabledPortMap, 1);

    /* Disable learning */
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &wan_port_map, 2);

    for(i=0; i < TOTAL_SWITCH_PORTS; i++) {
       if((wan_port_map >> i) & 0x1) {
            extsw_fast_age_port(i, 0);
       }
    }
}

void extsw_rreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len &= ~(SWAP_TYPE_MASK);

    /* Lower level driver always returnes in Little Endian data from history */
    extsw_rreg(page, reg, val, len);

    switch (len) {
        case 1:
            data[0] = val[0];
            break;
        case 2:
            *((uint16 *)data) = __le16_to_cpu(*((uint16 *)val));
            break;
        case 4:
            *((uint32 *)data) = __le32_to_cpu(*((uint32 *)val));
            break;
        case 6:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  val:Le64 from Lower driver API
                        Output: data:Host64, a pointer to the begining of 64 bit buffer
                    */
                    *(uint64*)data = __le64_to_cpu(*(uint64 *)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  val:Mac[5...0] from lower driver
                        Output: data:Mac[0...5]
                    */
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    memcpy(data, val+2, 6);
                    break;
            }
            break;
        case 8:
            switch (type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input:  val: Le64 for lower driver API
                        Output: data: Host64
                    */
                    *(uint64 *)data = __le64_to_cpu(*(uint64*)val);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  val:byte[7...0] from lower driver API
                        Output: data:byte[0...7]
                    */
                    *(uint64 *)data = __swab64(*(uint64*)val);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  val:Mac[5...0]|VidLeWord from Lower Driver API
                        Output: data:VidHostWord|Mac[0...5] for Caller
                    */
                    /* [Mac[5-0]]|[LEWord]; First always swap all bytes */
                    *((uint64 *)data) = __swab64(*((uint64 *)val));
                    /* Now is [BEWord]|[Mac[0-5]]; Conditional Swap 2 bytes */
                    *((uint16 *)&data[0]) = __be16_to_cpu(*((uint16 *)&data[0]));
                    /* Now is HostEndianWord|Mac[0-5] */
                    break;
                case DATA_TYPE_MIB_COUNT:
                    /*
                        MIB Counter Type:
                        Input:  [LeHiDw][LeLoDw]
                        Output: [HostHiDw][HostLoDw]
                    */
                    *((uint32 *)&data[0]) = __le32_to_cpu(*((uint32 *)&val[0]));
                    *((uint32 *)&data[4]) = __le32_to_cpu(*((uint32 *)&val[4]));
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    }
    BCM_ENET_DEBUG(" page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
            page, reg, data[0],data[1],data[2],data[3], len);
    if (len > 4)
        BCM_ENET_DEBUG(" page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
                page, reg, data[4],data[5],data[6],data[7], len);
}

void extsw_wreg_wrap(int page, int reg, void *vptr, int len)
{
    uint8 val[8];
    uint8 *data = (uint8*)vptr;
    int type = len & SWAP_TYPE_MASK;

    len  &= ~(SWAP_TYPE_MASK);
    BCM_ENET_DEBUG("%s : page=%d reg=%d Data [ 0x%02x 0x%02x 0x%02x 0x%02x ] Len = %d\n",
            __FUNCTION__, page, reg, data[0],data[1],data[2],data[3], len);

    switch (len) {
        case 1:
            val[0] = data[0];
            break;
        case 2:
            *((uint16 *)val) = __cpu_to_le16(*((uint16 *)data));
            break;
        case 4:
            *((uint32 *)val) = __cpu_to_le32(*((uint32 *)data));
            break;
        case 6:
            switch(type) {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Value type register access
                        Input:  data:Host64, a pointer to the begining of 64 bit buffer
                        Output: val:Le64
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64 *)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Byte array for MAC address
                        Input:  data:MAC[0...5] from Host
                        Output: val:Mac[5...0] for lower driver API
                    */
                    memcpy(val+2, data, 6);
                    *(uint64 *)val = __swab64(*(uint64*)val);
                    break;
            }
            break;
        case 8:
            switch (type)
            {
                case DATA_TYPE_HOST_ENDIAN:
                default:
                    /*
                        Input: data:Host64
                        Output:  val:Le64 for lower driver API
                    */
                    *(uint64 *)val = __cpu_to_le64(*(uint64*)data);
                    break;
                case DATA_TYPE_BYTE_STRING:
                    /*
                        Input:  data:byte[0...7]
                        Output: val:byte[7...0] for lower driver API
                    */
                    *(uint64 *)val = __swab64(*(uint64*)data);
                    break;
                case DATA_TYPE_VID_MAC:
                    /*
                        VID-MAC type;
                        Input:  VidHostWord|Mac[0...5]
                        Output: Mac[5..0.]|VidLeWord for Lower Driver API
                    */
                    /* Contains HostEndianWord|MAC[0-5] Always swap first*/
                    *((uint64 *)val) = __swab64(*((uint64 *)data));
                    /* Now it is MAC[5-0]|SwappedHostEndianWord */
                    /* Convert the SwappedHostEndianWord to Little Endian; thus BE */
                    *((uint16 *)&val[6]) = __cpu_to_be16(*((uint16 *)&val[6]));
                    /* Now is MAC[5-0]|LEWord as requested by HW */
                    break;
                case DATA_TYPE_MIB_COUNT:
                    /*
                        MIB Counter Type:
                        Input:  [HostHiDw][HostLoDw]
                        Output: [LeHiDw][LeLoDw]  for Lower driver API
                    */
                    *((uint32 *)&val[0]) = __le32_to_cpu(*((uint32 *)&data[0]));
                    *((uint32 *)&val[4]) = __le32_to_cpu(*((uint32 *)&data[4]));
                    break;
            } // switch type
            break;
        default:
            printk("Length %d not supported\n", len);
            break;
    } // switch len
    extsw_wreg(page, reg, val, len);
}

static void fast_age_start_done_ext(uint8_t ctrl)
{
    uint8_t timeout = 100;

    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
    while (ctrl & FAST_AGE_START_DONE) {
        mdelay(1);
        extsw_rreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
        if (!timeout--) {
            printk("Timeout of fast aging \n");
            break;
        }
    }

    /* Restore DYNAMIC bit for normal aging */
    ctrl = FAST_AGE_DYNAMIC;
    extsw_wreg_wrap(PAGE_CONTROL, REG_FAST_AGING_CTRL, (uint8_t *)&ctrl, 1);
}

void fast_age_all_ext(uint8_t age_static)
{
    uint8_t v8;

    v8 = FAST_AGE_START_DONE | FAST_AGE_DYNAMIC;
    if (age_static) {
        v8 |= FAST_AGE_STATIC;
    }

    fast_age_start_done_ext(v8);
}

static int enet_arl_access_reg_op(u8 v8)
{
    int timeout;

    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
    for ( timeout = 10, extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1);
            (v8 & ARL_TBL_CTRL_START_DONE) && timeout;
            --timeout, extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_TBL_CTRL, &v8, 1))
    {
        mdelay(1);
    }

    if (timeout <= 0)
    {
        printk("Error: ARL Operation Timeout\n");
        return 0;
    }
    return 1;
}

/* v32: b31 is raw bit,
    If raw: register format; etherwise: b15 is Valid bit */
int enet_arl_write_ext( uint8_t *mac, uint16_t vid, uint32_t v32)
{
    u8 mac_vid[8];
    u32 cur_v32;
    u16 ent_vid;
    int bin, empty_bin = -1;

    if (!(v32 & (1<<31))) v32 = ((v32 & 0xfc00) << 1) | (v32 & 0x1ff);  /* If it is raw, shift valid bit left */
    v32 &= ~(1<<31);

    /* Write the MAC Address and VLAN ID */
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_INDX_LO, mac, 6|DATA_TYPE_BYTE_STRING);
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_VLAN_INDX, &vid, 2);
    if (!enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE | ARL_TBL_CTRL_READ)) return 0;

    for (bin = 0; bin < REG_ARL_BINS_PER_HASH; bin++)
    {
        /* Read transaction complete - get the MAC + VID */
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, &mac_vid[0], 8|DATA_TYPE_VID_MAC);
        extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&cur_v32, 4);
        ent_vid = *(u16*)mac_vid;

        if (!(v32 & ARL_DATA_ENTRY_VALID_531xx))
        {
            /* If it is del op, find the matched bin */
            if (memcmp(&mac[0], &mac_vid[2], 6) != 0 || ent_vid != vid) continue;
        }
        else
        {
            /* If it is a modification or addition,
               find a matching entry, empty slot or last slot */
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 && vid == ent_vid) goto found_slot;
            if (!(cur_v32 & ARL_DATA_ENTRY_VALID_531xx) && empty_bin == -1) empty_bin = bin;  /* save empty bin for non matching case */
            if (bin < REG_ARL_BINS_PER_HASH-1) continue;  /* Continue to find next bin for matching if it not the last */
            /* No matching found here, if there is empty bin, use empty_bin or use last bin */
            if (empty_bin != -1) bin = empty_bin;
        }

        found_slot:

        /* Modify the data entry for this ARL */
        *(uint16 *)(&mac_vid[0]) = (vid & 0xFFF);
        memcpy(&mac_vid[2], &mac[0], 6);
        extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_MAC_LO_ENTRY + bin*0x10, mac_vid, 8|DATA_TYPE_VID_MAC);
        extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_DATA_ENTRY + bin*0x10,(uint8_t *)&v32, 4);

        /* Initiate a write transaction */
        if (!enet_arl_access_reg_op(ARL_TBL_CTRL_START_DONE)) return 0;
        return 1;
    }
    printk("Error - can't find the requested ARL entry\n");
    return 0;
}

int enet_arl_entry_op(uint8_t *mac, uint32_t *vid, uint32_t *val, int op, int *count, u8 *mac_vid, u32 data)
{
    switch(op)
    {
        case TYPE_DUMP:
            if (*count == 0) printk("\nExternal Switch %x ARL Dump:\n", extSwInfo.switch_id);
            if ((((*count)++) % 10)==0) 
            {
                printk("  No: VLAN  MAC          DATA" "(15:Valid,14:Static,13:Age,12-10:Pri,8-0:Port/Pmap)\n");
            }

            printk("%4d: %04d  %02x%02x%02x%02x%02x%02x 0x%04x\n",
                    *count, *(uint16 *)&mac_vid[0],
                    mac_vid[2], mac_vid[3], mac_vid[4], mac_vid[5], mac_vid[6], mac_vid[7],
                    ((data & 0x1f800)>>1)|(data&0x1ff));
            break;
        case TYPE_SET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0)
            {
                enet_arl_write_ext(mac, *(u16*)mac_vid, 0);
                (*count)++;
            }
            break;
        case TYPE_GET:
            if (memcmp(&mac[0], &mac_vid[2], 6) == 0 &&
                    (*vid == -1 || *vid == *(u16*)mac_vid))
            {
                /* entry found */
                *vid = *(u16*)mac_vid;
                if (*val & (1<<31)) /* Raw flag passed down from users space */
                {
                    *val = data;
                }
                else
                {
                    *val = ((data & 0x1f800)>>1)|(data & 0x1ff);
                }
                /* Return FALSE to terminate loop */
                return TRUE;
            }
            break;
    }
    return FALSE;
}

int enet_arl_search_ext(uint8_t *mac, uint32_t *vid, uint32_t *val, int op)
{
    int timeout = 1000, count = 0, hash_ent;
    uint32_t cur_data;
    uint8_t v8, mac_vid[8];

    v8 = ARL_SRCH_CTRL_START_DONE;
    extsw_wreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);

    for( extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1);
            (v8 & ARL_SRCH_CTRL_START_DONE);
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1))
    {
        /* Now read the Search Ctrl Reg Until :
         * Found Valid ARL Entry --> ARL_SRCH_CTRL_SR_VALID, or
         * ARL Search done --> ARL_SRCH_CTRL_START_DONE */
        for(timeout = 1000;
                (v8 & ARL_SRCH_CTRL_SR_VALID) == 0 && (v8 & ARL_SRCH_CTRL_START_DONE) && timeout-- > 0;
                mdelay(1),
                extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_CTRL_531xx, (uint8_t *)&v8, 1));

        if ((v8 & ARL_SRCH_CTRL_SR_VALID) == 0 || timeout <= 0) break;

        /* Found a valid entry */
        for (hash_ent = 0; hash_ent < REG_ARL_SRCH_HASH_ENTS; hash_ent++)
        {
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_MAC_LO_ENTRY0_531xx + hash_ent*0x10,&mac_vid[0], 8|DATA_TYPE_VID_MAC);
            extsw_rreg_wrap(PAGE_AVTBL_ACCESS, REG_ARL_SRCH_DATA_ENTRY0_531xx + hash_ent*0x10,(uint8_t *)&cur_data, 4);

            BCM_ENET_DEBUG("ARL_SRCH_result (%02x%02x%02x%02x%02x%02x%02x%02x) \n",
                    mac_vid[0],mac_vid[1],mac_vid[2],mac_vid[3],mac_vid[4],mac_vid[5],mac_vid[6],mac_vid[7]);
            BCM_ENET_DEBUG("ARL_SRCH_DATA = 0x%08x \n", cur_data);

            if ((cur_data & ARL_DATA_ENTRY_VALID_531xx))
            {
                if (enet_arl_entry_op(mac, vid, val, op, &count, mac_vid, cur_data)) return TRUE;
            }
        }
    }

    if (timeout <= 0)
    {
        printk("ARL Search Timeout for Valid to be 1 \n");
    }

    if (op == TYPE_DUMP) printk("Done: Total %d entries\n", count);
    if (op == TYPE_GET) return FALSE;
    return TRUE;
}

void enet_arl_dump_ext_multiport_arl(void)
{
    uint16 v16;
    uint8 addr[8];
    int i, enabled;
    uint32 vect;
    static char *cmp_type[] = {"Disabled", "Etype", "MAC Addr", "MAC Addr & Etype"}; 

    extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, &v16, 2);
    enabled = v16 & ((MULTIPORT_CTRL_EN_M << (5*2))| (MULTIPORT_CTRL_EN_M << (4*2))| (MULTIPORT_CTRL_EN_M << (3*2))|
            (MULTIPORT_CTRL_EN_M << (2*2))| (MULTIPORT_CTRL_EN_M << (1*2))| (MULTIPORT_CTRL_EN_M << (0*2)));

    printk("\nExternal Switch Multiport Address Dump: Function %s\n", enabled? "Enabled": "Disabled");
    if (!enabled) return;

    printk("  Mapping to ARL matching: %s\n", v16 & (1<<MULTIPORT_CTRL_DA_HIT_EN)? "Lookup Hit": "Lookup Failed");
    for (i=0; i<6; i++)
    {
        enabled = (v16 & (MULTIPORT_CTRL_EN_M << (5*i)));
        extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_ADDR1_LO + i*16, (uint8 *)&addr, sizeof(addr)|DATA_TYPE_VID_MAC);
        extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_VECTOR1 + i*16, (uint8 *)&vect, sizeof(vect));
        printk("Mport Eth Type: 0x%04x, Mport Addrs: %02x:%02x:%02x:%02x:%02x:%02x, Port Map %04x, Cmp Type: %s\n",
                *(uint16 *)(addr),
                addr[2],
                addr[3],
                addr[4],
                addr[5],
                addr[6],
                addr[7],
                (int)vect, cmp_type[enabled & MULTIPORT_CTRL_EN_M]);
    }
    printk("External Switch Multiport Address Dump Done\n");
}

int remove_arl_entry_wrapper(void *ptr)
{
    int ret = 0;
    ret = enet_arl_remove(ptr); /* remove entry from internal switch */
    if (bcm63xx_enet_isExtSwPresent())
    {
        ret = enet_arl_remove_ext(ptr); /* remove entry from internal switch */
    }
    return ret;
}

void bcmeapi_reset_mib_ext(void)
{
#ifdef REPORT_HARDWARE_STATS
    uint8_t val8;
    if (pVnetDev0_g->extSwitch->present) {
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 |= GLOBAL_CFG_RESET_MIB;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
        val8 &= (~GLOBAL_CFG_RESET_MIB);
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_GLOBAL_CONFIG, &val8, 1);
    }

#endif
    return ;
}
void  bcmsw_dump_page_ext(int page)
{
    switch(page) {
#if defined(STAR_FIGHTER2)
        case 0:
            sf2_dump_page0();
            break;
#endif

        default:
            break;
    }
}
int bcmsw_dump_mib_ext(int port, int type)
{
    unsigned int v32, errcnt;
    unsigned long port_map;
    uint8 data[8] = {0};
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info;

    info = &EnetInfo[1];
    if (!((info->ucPhyType == BP_ENET_EXTERNAL_SWITCH) ||
         (info->ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)))
    {
        printk("No External switch connected\n");
        return -ENODEV;
    }

    info = &EnetInfo[1];
    port_map = info->sw.port_map;
    port_map |= imp_pbmap[1];

    if (!(port_map & (1<<port))) /* Only IMP port and ports in the port_map are allowed */
    {
        printk("Invalid/Unused External switch port %d\n",port);
        return -ENODEV;
    }
#if defined(STAR_FIGHTER2)
    sf2_bcmsw_dump_mib_ext(port, type);
    return 0;
#endif

    /* Display Tx statistics */
    printk("External Switch Stats : Port# %d\n",port);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDROPS, &v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("TxOctetsLo:             %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("TxOctetsHi:             %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ0PKT, &v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ1PKT, &v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ2PKT, &v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ3PKT, &v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ4PKT, &v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXQ5PKT, &v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXCOL, &v32, 4);
        printk("TxCol:                  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXSINGLECOL, &v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMULTICOL, &v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDEFERREDTX, &v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXLATECOL, &v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXEXCESSCOL, &v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, &v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXPAUSEPKTS, &v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt = 0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXSINGLECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMULTICOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDEFERREDTX, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXLATECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXEXCESSCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, &v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }
    /* Display Rx statistics */
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDROPS, &v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDISCARD, &v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("RxOctetsLo:             %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("RxOctetsHi:             %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXGOODOCT, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("RxGoodOctetsLo:         %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("RxGoodOctetsHi:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJABBERS, &v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, &v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, &v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFRAGMENTS, &v32, 4);
        printk("RxFragments:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOVERSIZE, &v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUNDERSIZEPKTS, &v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXPAUSEPKTS, &v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSACHANGES, &v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, &v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX64OCTPKTS, &v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX127OCTPKTS, &v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX255OCTPKTS, &v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX511OCTPKTS, &v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RX1023OCTPKTS, &v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMAXOCTPKTS, &v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJUMBOPKT , &v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOUTRANGEERR, &v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXINRANGEERR, &v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXJABBERS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOVERSIZE, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUNDERSIZEPKTS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOUTRANGEERR, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXINRANGEERR, &v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);
    }

    return 0;
}


int bcmsw_set_multiport_address_ext(uint8_t* addr)
{
    if (bcm63xx_enet_isExtSwPresent())
    {
        int i;
        uint32 v32;
        uint16 v16;
        uint8 v64[8];
        uint8 cur64[8];

        *(uint16*)(&v64[0]) = 0;
        memcpy(&v64[2], addr, 6);
        /* check if address is set already */
        for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
        {
           extsw_rreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&cur64, sizeof(cur64)|DATA_TYPE_VID_MAC);
           if ( 0 == memcmp(&v64[0], &cur64[0], 8) )
           {
               return 0;
           }
        }

        /* add new entry */
        for ( i = 0; i < MULTIPORT_CTRL_COUNT; i++ )
        {
            extsw_rreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
            if ( 0 == (v16 & (MULTIPORT_CTRL_EN_M << (i << 1))))
            {
                v16 |= (1<<MULTIPORT_CTRL_DA_HIT_EN) | (MULTIPORT_CTRL_ADDR_CMP << (i << 1));
                extsw_wreg_wrap(PAGE_ARLCTRL, REG_MULTIPORT_CTRL, (uint8 *)&v16, 2);
                *(uint16*)(&v64[0]) = 0;
                memcpy(&v64[2], addr, 6);
                extsw_wreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_ADDR1_LO + (i * 0x10)), (uint8 *)&v64, sizeof(v64)|DATA_TYPE_VID_MAC);
                v32 = imp_pbmap[1];
                extsw_wreg_wrap(PAGE_ARLCTRL, (REG_MULTIPORT_VECTOR1 + (i * 0x10)), (uint8 *)&v32, sizeof(v32));

                /* Set multiport VLAN control based on U/V_FWD_MAP;
                   This is required so that VLAN tagged frames matching Multiport Address are forwarded according to V/U forwarding map */
                extsw_rreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));
                v16 |=  (EN_MPORT_V_FWD_MAP | EN_MPORT_U_FWD_MAP) << (i*EN_MPORT_V_U_FWD_MAP_S) ;
                extsw_wreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_MULTI_PORT_ADDR_CTL, &v16, sizeof(v16));

                return 0;
            }
        }
    }

    return -1;
}

int bcmeapi_ioctl_set_multiport_address(struct ethswctl_data *e)
{
    if (e->unit == 0) {
       if (e->type == TYPE_GET) {
           return BCM_E_NONE;
       } else if (e->type == TYPE_SET) {
           bcmeapi_set_multiport_address(e->mac);
           bcmsw_set_multiport_address_ext(e->mac);
           return BCM_E_PARAM;
       }
    }
    return BCM_E_NONE;
}

#ifdef REPORT_HARDWARE_STATS
int bcmsw_get_hw_stats(int port, int extswitch, struct net_device_stats *stats)
{
    uint64 ctr64 = 0;           // Running 64 bit counter
    uint8 data[8] = {0};

    if (extswitch) {

        // Track RX unicast, multicast, and broadcast packets
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXUPKTS, data, 4);  // Get RX unicast packet count
        ctr64 = (*(uint32 *)data);                                // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXMPKTS, data, 4);  // Get RX multicast packet count
        stats->multicast = (*(uint32 *)data);                                   // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXBPKTS, data, 4);  // Get RX broadcast packet count
        stats->rx_broadcast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count
        stats->rx_packets = (unsigned long)ctr64;

        // Dump RX debug data if needed
        BCM_ENET_DEBUG("read data = %02x %02x %02x %02x \n",
            data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG("ctr64 = %x \n", (unsigned int)ctr64);

        // Track RX byte count
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        stats->rx_bytes = (unsigned long )(*(uint32 *)&data[0]); /* Truncate 4-MSByte */

        // Track RX packet errors
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXDROPS, data, 4);
        stats->rx_dropped = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXFCSERRORS, data, 4);
        stats->rx_errors = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXSYMBOLERRORS, data, 4);
        stats->rx_errors += (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_RXALIGNERRORS, data, 4);
        stats->rx_errors += (*(uint32 *)data);

        // Track TX unicast, multicast, and broadcast packets
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXUPKTS, data, 4);  // Get TX unicast packet count
        ctr64 = (*(uint32 *)data);                                // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXMPKTS, data, 4);  // Get TX multicast packet count
        stats->tx_multicast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count

        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXBPKTS, data, 4);  // Get TX broadcast packet count
        stats->tx_broadcast_packets = (*(uint32 *)data);                        // Save away count
        ctr64 += (*(uint32 *)data);                                             // Keep running count
        stats->tx_packets = (unsigned long)ctr64;

        // Track TX byte count
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        stats->tx_bytes = (unsigned long)(*(uint32 *)&data[0]); /* Truncate 4-MSByte */

        // Track TX packet errors
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXDROPS, data, 4);
        stats->tx_dropped = (*(uint32 *)data);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), REG_MIB_P0_EXT_TXFRAMEINDISC, data, 4);
        stats->tx_dropped += (*(uint32 *)data);
    } else
    {
       ethsw_get_hw_stats(port, stats);
    }
    return 0;
}
#endif

int bcmeapi_ioctl_extsw_port_jumbo_control(struct ethswctl_data *e)
{
    uint32 val32;

    if (e->type == TYPE_GET)
    {
        // Read & log current JUMBO configuration control register.
        extsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);
        e->ret_val = val32;
    }
    else
    {
        // Read & log current JUMBO configuration control register.
        extsw_rreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);
        BCM_ENET_DEBUG("Old JUMBO_PORT_MASK = 0x%08X", (unsigned int)val32);

        // Setup JUMBO configuration control register.
        val32 = ConfigureJumboPort(val32, e->port, e->val);
        extsw_wreg(PAGE_JUMBO, REG_JUMBO_PORT_MASK, (uint8 *)&val32, 4);

        // Attempt to transfer register write value to user space & test for success.
        e->ret_val = val32;
    }
    return BCM_E_NONE;
}

static uint16_t dis_learning_ext = 0x0100; /* This default value does not matter */


int bcmsw_enable_hw_switching(void)
{
    u8 i;

    /* restore disable learning register */
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 1) /* Not External switch port */
        {
            i++;  /* Go to next port */
            continue;
        }
        /* When hardware switching is enabled, enable the Linux bridge to
          not to forward the bcast packets on hardware ports */
        vnet_dev[i++]->priv_flags |= IFF_HW_SWITCH;
    }

    return 0;
}

int bcmsw_disable_hw_switching(void)
{
    u8 i;
    u16 reg_value;

    /* Save disable_learning_reg setting */
    extsw_rreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &dis_learning_ext, 2);
    /* disable learning on all ports */
    reg_value = PBMAP_ALL;
    extsw_wreg_wrap(PAGE_CONTROL, REG_DISABLE_LEARNING, &reg_value, 2);

    i = 0;
    while (vnet_dev[i])
    {
        if (LOGICAL_PORT_TO_UNIT_NUMBER(VPORT_TO_LOGICAL_PORT(i)) != 1) /* Not External switch port */
        {
            i++;  /* Go to next port */
            continue;
        }
        /* When hardware switching is disabled, enable the Linux bridge to
          forward the bcast on hardware ports as well */
        vnet_dev[i++]->priv_flags &= ~IFF_HW_SWITCH;
    }

    /* Flush arl table dynamic entries */
    fast_age_all_ext(0);
    return 0;
}

int bcmeapi_ioctl_extsw_pid_to_priority_mapping(struct ethswctl_data *e)
{

#if defined(STAR_FIGHTER2)
    return sf2_pid_to_priority_mapping(e);
#else
    BCM_ENET_ERROR("No handler! \n");
    return 0;
#endif
}


/* This works for internal ROBO and external 53125 switches */
int bcmeapi_ioctl_ethsw_cos_priority_method_config(struct ethswctl_data *e)
{
    uint32_t v32;
    uint16_t v16 = 0;
    uint8_t v8, u8 = 0;
    uint8_t port_qos_en, qos_layer_sel;
    uint16_t port_dscp_en, port_pcp_en;

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG(" type:  %02d\n ", e->type);
    if (e->type == TYPE_GET) {
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
        port_qos_en = (v8 >> PORT_QOS_EN_S) & PORT_QOS_EN_M;
        qos_layer_sel = (v8 >> QOS_LAYER_SEL_S) & QOS_LAYER_SEL_M;
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
        port_pcp_en = v16 & (1UL << e->port);
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        port_dscp_en = v16 & (1UL << e->port);
        BCM_ENET_DEBUG(" port %d v16 %#x qos_layer_sel %d port_pcp_en %#x "
                        "port_dscp_en %#x port_qos_en %#x \n",
                        e->port, v16, qos_layer_sel, port_pcp_en, port_dscp_en, port_qos_en);

        if (port_qos_en) {
            switch (qos_layer_sel)
            {
                case 3:
                    // ? when your are here, port based QOS is always enabled.
                    v32 = PORT_QOS;
                    break;
                default:
                    v32 = PORT_QOS;
                    break;
            }
        } else {
            switch (qos_layer_sel)
            {
                case 3:
                    if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                // When we program, we set qos_layer == 3. So, following are moot.
                case 2:
                    if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                case 1:
                    if (port_dscp_en) {
                        v32 = DIFFSERV_QOS;
                    } else {
                        v32 = TC_ZERO_QOS;
                    }
                    break;
                case 0:
                    if (port_pcp_en) {
                        v32 = IEEE8021P_QOS;
                    } else {
                        v32 = MAC_QOS;
                    }
                    break;
                default:
                    break;
            }
        }
        e->ret_val = v32;
        BCM_ENET_DEBUG("e->ret_val is = %02d", e->ret_val);
    // SET
    } else {
        BCM_ENET_DEBUG("port %d Given method: %02d ADD \n ", e->port, e->val);
        SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
        v8 &= ~(PORT_QOS_EN_M << PORT_QOS_EN_S);
        u8 = QOS_LAYER_SEL_M;
        if (e->val == MAC_QOS) {
           // disable per port .1p qos, & dscp

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        } else if (e->val == IEEE8021P_QOS) {
           // enable .1p qos and  disable dscp

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 |= (1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
        } else if (e->val == DIFFSERV_QOS) {  // DSCP QOS
           // enable dscp qos and disable .1p

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);
            v16 |= (1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_DSCP_EN, (void *)&v16, 2);

            SW_READ_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
            v16 &= ~(1 << e->port);
            SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_8021P_EN, (void *)&v16, 2);
        } else {
            v8 |= (PORT_QOS_EN_M << PORT_QOS_EN_S);
        }
        v8 |= u8 << QOS_LAYER_SEL_S;
        SW_WRITE_REG(e->unit, PAGE_QOS, REG_QOS_GLOBAL_CTRL, (void *)&v8, 1);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set PID to TC mapping Tabe entry given ingress port
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_ethsw_pid_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val16;
    BCM_ENET_DEBUG("Given uint %02d port %02d \n ", e->unit, e->port);

    if (e->port < 0 || (e->unit == 1 &&  e->port >= MAX_EXT_SWITCH_PORTS) ||
                          (e->unit == 0 &&  e->port >= TOTAL_SWITCH_PORTS))
    {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        SW_READ_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
        e->priority = (val16 >> DEFAULT_TAG_PRIO_S) & DEFAULT_TAG_PRIO_M;
        BCM_ENET_DEBUG("port %d is mapped to priority: %d \n ", e->port, e->priority);
    } else {
        BCM_ENET_DEBUG("Given port: %02d priority: %02d \n ", e->port, e->priority);
        SW_READ_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
        val16 &= ~(DEFAULT_TAG_PRIO_M << DEFAULT_TAG_PRIO_S);
        val16 |= (e->priority & DEFAULT_TAG_PRIO_M) << DEFAULT_TAG_PRIO_S;
        SW_WRITE_REG(e->unit, PAGE_8021Q_VLAN, REG_VLAN_DEFAULT_TAG + e->port * 2, (void *)&val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
int bcmeapi_ioctl_extsw_cos_priority_method_config(struct ethswctl_data *e)
{

#if defined(STAR_FIGHTER2)
    return sf2_cos_priority_method_config(e);
#else
    BCM_ENET_ERROR("No handler! \n");
    return 0;
#endif
}

/*
 * This applies only for Star Fighter switch
 */
int bcmeapi_ioctl_extsw_port_shaper_config(struct ethswctl_data *e)
{
#if defined(STAR_FIGHTER2)
    return sf2_port_shaper_config(e);
#else
    BCM_ENET_ERROR("No handler! \n");
    return 0;
#endif

}

int bcmeapi_ioctl_extsw_port_erc_config(struct ethswctl_data *e)
{
#if defined(STAR_FIGHTER2)
    return sf2_port_erc_config(e);
#else
    BCM_ENET_ERROR("No handler! \n");
    return 0;
#endif

}

/*
 * Get/Set PCP to TC mapping Tabe entry given 802.1p priotity (PCP)
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->val -  pcp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_pcp_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val32;
    uint16_t reg_addr;

    BCM_ENET_DEBUG("Given pcp: %02d \n ", e->val);
    if (e->val > MAX_PRIORITY_VALUE) {
        printk("Invalid PCP Value %02d \n", e->val);
        return BCM_E_ERROR;
    }

#if defined(STAR_FIGHTER2)
    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    reg_addr = e->port == SF2_IMP0_PORT? SF2_REG_QOS_PCP_IMP0:
               e->port == SF2_P7? SF2_REG_QOS_PCP_P7:
                          REG_QOS_8021P_PRIO_MAP + e->port * QOS_PCP_MAP_REG_SZ;
#else
    if (e->port < 0 || (e->port > MAX_EXT_SWITCH_PORTS &&  e->port != IMP_PORT_ID)) {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    reg_addr = e->port == IMP_PORT_ID? REG_QOS_8021P_PRIO_MAP_IP:
                          REG_QOS_8021P_PRIO_MAP + e->port * QOS_PCP_MAP_REG_SZ;
#endif

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
        e->priority = (val32 >> (e->val * QOS_TC_S)) & QOS_TC_M;
        BCM_ENET_DEBUG("pcp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given pcp: %02d priority: %02d \n ", e->val, e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority \n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->val * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->val * QOS_TC_S);
        extsw_wreg_wrap(PAGE_QOS, reg_addr, (void *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set DSCP to TC mapping Tabe entry given dscp value and priority
 *** Input params
 * e->type  GET/SET
 * e->val -  dscp
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_dscp_to_priority_mapping(struct ethswctl_data *e)
{

    uint64 val64 = 0;
    uint32_t mapnum;
    int dscplsbs;

    BCM_ENET_DEBUG("Given dscp: %02d \n ", e->val);
    if (e->val > QOS_DSCP_M) {
        printk("Invalid DSCP Value \n");
        return BCM_E_ERROR;
    }

    down(&bcm_ethlock_switch_config);

    dscplsbs = e->val & QOS_DSCP_MAP_LSBITS_M;
    mapnum = (e->val >> QOS_DSCP_MAP_S) & QOS_DSCP_MAP_M;

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                 (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        e->priority = (val64 >> (dscplsbs * QOS_TC_S)) & QOS_TC_M;
        BCM_ENET_DEBUG("dscp %d is mapped to priority: %d \n ", e->val, e->priority);
    } else {
        BCM_ENET_DEBUG("Given priority: %02d \n ", e->priority);
        if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
            printk("Invalid Priority \n");
            up(&bcm_ethlock_switch_config);
            return BCM_E_ERROR;
        }
        // LE assumptions below, TODO
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                     (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
        val64 &= ~(((uint64)(QOS_TC_M)) << (dscplsbs * QOS_TC_S));
        val64 |= ((uint64)(e->priority & QOS_TC_M)) << (dscplsbs * QOS_TC_S);
        BCM_ENET_DEBUG(" @ addr %#x val64 to write = 0x%llx \n",
                                (REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ),
                                (uint64) val64);

        extsw_wreg_wrap(PAGE_QOS, REG_QOS_DSCP_PRIO_MAP0LO + mapnum * QOS_DSCP_MAP_REG_SZ,
                                            (void *)&val64, QOS_DSCP_MAP_REG_SZ | DATA_TYPE_HOST_ENDIAN);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
/*
 * Get/Set cos(queue) mapping, given priority (TC)
 *** Input params
 * e->type  GET/SET
 * e->queue - target queue
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
int bcmeapi_ioctl_extsw_cosq_port_mapping(struct ethswctl_data *e)
{
    union {
        uint32_t val32;
        uint16_t val16;
    }val;
    int queue;
    int retval = 0;
    uint16_t reg_addr;
    uint16_t cos_shift;
    uint16_t cos_mask;
    uint16_t reg_len;

    BCM_ENET_DEBUG("Given port: %02d priority: %02d \n ", e->port, e->priority);
    if (e->port >= TOTAL_SWITCH_PORTS
#if defined(STAR_FIGHTER2)
              || e->port == SF2_INEXISTANT_PORT
#endif
       ) {
        printk("Invalid Switch Port %02d \n", e->port);
        return -BCM_E_ERROR;
    }
    if ((e->priority > MAX_PRIORITY_VALUE) || (e->priority < 0)) {
        printk("Invalid Priority \n");
        return -BCM_E_ERROR;
    }
#if defined(STAR_FIGHTER2)
    reg_addr  = SF2_REG_PORTN_TC_TO_COS + e->port * 4;
    cos_shift = SF2_QOS_COS_SHIFT;
    cos_mask  = SF2_QOS_COS_MASK;
    reg_len   = 4;
#else
    reg_addr  = REG_QOS_PORT_PRIO_MAP_EXT;
    cos_shift = REG_QOS_PRIO_TO_QID_SEL_BITS;
    cos_mask  = REG_QOS_PRIO_TO_QID_SEL_M;
    reg_len   = 2;
#endif

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
        BCM_ENET_DEBUG("REG_QOS_PORT_PRIO_MAP_Px = %p",
                (void*)&val);
        /* Get the queue */
#if !defined(STAR_FIGHTER2)
        val.val32 = val.val16;
#endif
        queue = (val.val32 >> (e->priority * cos_shift)) & cos_mask;
#if !defined(STAR_FIGHTER2)
        retval = queue & REG_QOS_PRIO_TO_QID_SEL_M;
#else
        retval = queue & SF2_QOS_COS_MASK;
#endif
        BCM_ENET_DEBUG("%s queue is = %4x", __FUNCTION__, retval);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
        extsw_rreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
#if !defined(STAR_FIGHTER2)
        /* Other External switches define 16 bit TC to COS */
        val.val16 &= ~(cos_mask << (e->priority * cos_shift));
        val.val16 |= (e->queue & cos_mask) << (e->priority * cos_shift);
#else
        val.val32 &= ~(cos_mask << (e->priority * cos_shift));
        val.val32 |= (e->queue & cos_mask) << (e->priority * cos_shift);
#endif
        extsw_wreg_wrap(PAGE_QOS, reg_addr, (uint8_t *)&val, reg_len);
    }
    up(&bcm_ethlock_switch_config);
    return retval;
}

#if !defined(STAR_FIGHTER2)
#define MAX_WRR_WEIGHT 0x31
static int extsw_cosq_sched(struct ethswctl_data *e)
{
    uint8_t  val8, txq_mode;
    int i, val, sched;

    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%2x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_EXT_S) & TXQ_CTRL_TXQ_MODE_EXT_M;
        if (txq_mode) {
            if(txq_mode == 3) {
                sched = BCM_COSQ_STRICT;
            } else {
                sched = BCM_COSQ_COMBO;
                e->val = txq_mode;
            }
        } else {
            sched = BCM_COSQ_WRR;
        }

        e->scheduling = sched;

        /* Get the weights */
        if(sched != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, val8);
                val = val8;
                e->weights[i] = val;
                BCM_ENET_DEBUG("e->weight[%2d] = %02d ", i, e->weights[i]);
            }
        }
    } else {
        BCM_ENET_DEBUG("Given scheduling mode: %02d", e->scheduling);
        BCM_ENET_DEBUG("Given sp_endq: %02d", e->queue);
        for (i=0; i < NUM_EGRESS_QUEUES; i++) {
            BCM_ENET_DEBUG("Given weight[%2d] = %02d ", i, e->weights[i]);

            // Is this a legal weight?
            if (e->weights[i] <= 0 || e->weights[i] > MAX_WRR_WEIGHT) {
                BCM_ENET_DEBUG("Invalid weight");
                up(&bcm_ethlock_switch_config);
                return BCM_E_ERROR;
            }
        }
        extsw_rreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        BCM_ENET_DEBUG("REG_QOS_TXQ_CTRL = 0x%02x", val8);
        txq_mode = (val8 >> TXQ_CTRL_TXQ_MODE_EXT_S) & TXQ_CTRL_TXQ_MODE_EXT_M;
        /* Set the scheduling mode */
        if (e->scheduling == BCM_COSQ_WRR) {
            // Set TXQ_MODE bits for 4 queue mode.  Leave high
            // queue preeempt bit cleared so queue weighting will be used.
            val8 = 0;  // ALL WRR queues in ext switch port
        } else if ((e->scheduling == BCM_COSQ_STRICT) ||
                   (e->scheduling == BCM_COSQ_COMBO)){
            if (e->scheduling == BCM_COSQ_STRICT) {
                txq_mode = 3;
            } else {
                txq_mode = e->queue;
            }
            val8 &= (~(TXQ_CTRL_TXQ_MODE_EXT_M << TXQ_CTRL_TXQ_MODE_EXT_S));
            val8 |= ((txq_mode & TXQ_CTRL_TXQ_MODE_EXT_M) << TXQ_CTRL_TXQ_MODE_EXT_S);
        } else {
            BCM_ENET_DEBUG("Invalid scheduling mode %02d", e->scheduling);
            up(&bcm_ethlock_switch_config);
            return BCM_E_PARAM;
        }
        BCM_ENET_DEBUG("Writing 0x%02x to REG_QOS_TXQ_CTRL", val8);
        extsw_wreg_wrap(PAGE_QOS, REG_QOS_TXQ_CTRL, &val8, 1);
        /* Set the weights if WRR or COMBO */
        if(e->scheduling != BCM_COSQ_STRICT) {
            for (i=0; i < NUM_EGRESS_QUEUES; i++) {
                BCM_ENET_DEBUG("Weight[%2d] = %02d ", i, e->weights[i]);
                val8 =  e->weights[i];
                extsw_wreg_wrap(PAGE_QOS, REG_QOS_TXQ_WEIGHT_Q0 + i, &val8, 1);
            }
        }
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}
#endif
int bcmeapi_ioctl_extsw_cosq_sched(struct ethswctl_data *e)
{
    int ret = 0;
#if defined(STAR_FIGHTER2)
    ret =  sf2_cosq_sched(e);
#else
    ret =  extsw_cosq_sched(e);
#endif
    if (ret >= 0) {
        if (e->type == TYPE_GET) {
            e->ret_val = e->val;
            BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
        }
    }
    return ret;
}

uint16_t extsw_get_pbvlan(int port)
{
    uint16_t val16;

    extsw_rreg_wrap(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&val16, 2);
    return val16;
}
void extsw_set_pbvlan(int port, uint16_t fwdMap)
{
    extsw_wreg_wrap(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2),
               (uint8_t *)&fwdMap, 2);
}

int bcmeapi_ioctl_extsw_pbvlan(struct ethswctl_data *e)
{
    uint16_t val16;

    BCM_ENET_DEBUG("Given Port: 0x%02x \n ", e->port);
    if (e->port >= TOTAL_SWITCH_PORTS) {
        printk("Invalid Switch Port \n");
        return BCM_E_ERROR;
    }

    if (e->type == TYPE_GET) {
        down(&bcm_ethlock_switch_config);
        val16 = extsw_get_pbvlan(e->port); 
        up(&bcm_ethlock_switch_config);
        BCM_ENET_DEBUG("Threshold read = %4x", val16);
        e->fwd_map = val16;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
    } else {
        val16 = (uint32_t)e->fwd_map;
        BCM_ENET_DEBUG("e->fwd_map is = %4x", e->fwd_map);
        down(&bcm_ethlock_switch_config);
        extsw_set_pbvlan(e->port, val16);
        up(&bcm_ethlock_switch_config);
    }

    return 0;
}

int bcmeapi_ioctl_extsw_prio_control(struct ethswctl_data *e)
{
    int ret = 0;
#if defined(STAR_FIGHTER2)
    if ((ret =  sf2_prio_control(e)) >= 0) {
        if (e->type == TYPE_GET) {
            e->ret_val = e->val;
            BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
        }
    }
#else
    BCM_ENET_ERROR("No handler! \n");
#endif
    return ret;
}

// Default buffer thresholds need to be arrived at and configured at switch init
// calling this function.
int bcmeapi_ioctl_extsw_control(struct ethswctl_data *e)
{
    int ret = 0;
    uint8_t val8 = 0;
    unsigned int val;
    switch (e->sw_ctrl_type) {
        case bcmSwitchBufferControl:

#if defined(STAR_FIGHTER2)
            if ((ret = sf2_pause_drop_ctrl(e)) >= 0) {
                if (e->type == TYPE_GET) {
                    e->ret_val = e->val;
                    BCM_ENET_DEBUG("e->ret_val is = %4x", e->ret_val);
                 }
            }
#else
            BCM_ENET_ERROR("No handler! \n");
#endif
            break;

        case bcmSwitch8021QControl:
            /* Read the 802.1Q control register */
            extsw_rreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            if (e->type == TYPE_GET) {
                val = (val8 >> VLAN_EN_8021Q_S) & VLAN_EN_8021Q_M;
                if (val && ((val8 >> VLAN_IVL_SVL_S) & VLAN_IVL_SVL_M))
                    val = 2; // IVL mode
                e->val  = val;
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
            } else {  // 802.1Q SET
                /* Enable/Disable the 802.1Q */
                if (e->val == 0)
                    val8 &= (~(VLAN_EN_8021Q_M << VLAN_EN_8021Q_S));
                else {
                    val8 |= (VLAN_EN_8021Q_M << VLAN_EN_8021Q_S);
                    if (e->val == 1) // SVL
                        val8 &= (~(VLAN_IVL_SVL_M << VLAN_IVL_SVL_S));
                    else if (e->val == 2) // IVL
                        val8 |= (VLAN_IVL_SVL_M << VLAN_IVL_SVL_S);
                }
                extsw_wreg_wrap(PAGE_8021Q_VLAN, REG_VLAN_GLOBAL_8021Q, &val8, 1);
            }
            break;

        default:
            //up(&bcm_ethlock_switch_config);
            ret = -BCM_E_PARAM;
            break;
    } //switch
    return ret;
}
int bcmeapi_ioctl_extsw_config_acb(struct ethswctl_data *e)
{
#if defined(STAR_FIGHTER2)
    return sf2_config_acb (e);
#endif
    return 0;

}

int enet_ioctl_ethsw_dos_ctrl(struct ethswctl_data *e)
{
    if (e->unit != 1)
    {
        return BCM_E_PARAM;
    }
    if (e->type == TYPE_GET)
    {
        if (bcm63xx_enet_isExtSwPresent())
        {
            uint32 v32 = 0;
            uint8 v8 = 0;

            extsw_rreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
            /* Taking short-cut : Not following BCM coding guidelines */
            if (v32 & IP_LAN_DROP_EN)  e->dosCtrl.ip_lan_drop_en = 1;
            if (v32 & TCP_BLAT_DROP_EN)  e->dosCtrl.tcp_blat_drop_en = 1;
            if (v32 & UDP_BLAT_DROP_EN)  e->dosCtrl.udp_blat_drop_en = 1;
            if (v32 & TCP_NULL_SCAN_DROP_EN)  e->dosCtrl.tcp_null_scan_drop_en = 1;
            if (v32 & TCP_XMAS_SCAN_DROP_EN)  e->dosCtrl.tcp_xmas_scan_drop_en = 1;
            if (v32 & TCP_SYNFIN_SCAN_DROP_EN)  e->dosCtrl.tcp_synfin_scan_drop_en = 1;
            if (v32 & TCP_SYNERR_SCAN_DROP_EN)  e->dosCtrl.tcp_synerr_drop_en = 1;
            if (v32 & TCP_SHORTHDR_SCAN_DROP_EN)  e->dosCtrl.tcp_shorthdr_drop_en = 1;
            if (v32 & TCP_FRAGERR_SCAN_DROP_EN)  e->dosCtrl.tcp_fragerr_drop_en = 1;
            if (v32 & ICMPv4_FRAG_DROP_EN)  e->dosCtrl.icmpv4_frag_drop_en = 1;
            if (v32 & ICMPv6_FRAG_DROP_EN)  e->dosCtrl.icmpv6_frag_drop_en = 1;
            if (v32 & ICMPv4_LONGPING_DROP_EN)  e->dosCtrl.icmpv4_longping_drop_en = 1;
            if (v32 & ICMPv6_LONGPING_DROP_EN)  e->dosCtrl.icmpv6_longping_drop_en = 1;

            extsw_rreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
            if (v8 & DOS_DISABLE_LRN) e->dosCtrl.dos_disable_lrn = 1;
        }
        else
        {
            return BCM_E_EXISTS;
        }
    }
    else if (e->type == TYPE_SET)
    {
        if (bcm63xx_enet_isExtSwPresent())
        {
            uint32 v32 = 0;
            uint8 v8 = 0;
            /* Taking short-cut : Not following BCM coding guidelines */
            if (e->dosCtrl.ip_lan_drop_en) v32 |= IP_LAN_DROP_EN;
            if (e->dosCtrl.tcp_blat_drop_en) v32 |= TCP_BLAT_DROP_EN;
            if (e->dosCtrl.udp_blat_drop_en) v32 |= UDP_BLAT_DROP_EN;
            if (e->dosCtrl.tcp_null_scan_drop_en) v32 |= TCP_NULL_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_xmas_scan_drop_en) v32 |= TCP_XMAS_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_synfin_scan_drop_en) v32 |= TCP_SYNFIN_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_synerr_drop_en) v32 |= TCP_SYNERR_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_shorthdr_drop_en) v32 |= TCP_SHORTHDR_SCAN_DROP_EN;
            if (e->dosCtrl.tcp_fragerr_drop_en) v32 |= TCP_FRAGERR_SCAN_DROP_EN;
            if (e->dosCtrl.icmpv4_frag_drop_en) v32 |= ICMPv4_FRAG_DROP_EN;
            if (e->dosCtrl.icmpv6_frag_drop_en) v32 |= ICMPv6_FRAG_DROP_EN;
            if (e->dosCtrl.icmpv4_longping_drop_en) v32 |= ICMPv4_LONGPING_DROP_EN;
            if (e->dosCtrl.icmpv6_longping_drop_en) v32 |= ICMPv6_LONGPING_DROP_EN;

            /* Enable DOS attack blocking functions) */
            extsw_wreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_CTRL, (uint8 *)&v32, 4);
            if (e->dosCtrl.dos_disable_lrn)
            { /* Enable */
                v8 = DOS_DISABLE_LRN;
            }
            else
            {
                v8 = 0;
            }
            extsw_wreg_wrap(PAGE_DOS_PREVENT_531xx, REG_DOS_DISABLE_LRN, (uint8 *)&v8, 1);
        }
        else
        {
            return BCM_E_EXISTS;
        }
    }
    return BCM_E_NONE;
}
void bcmsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap, unsigned int *eg_pmap, unsigned int *blk_no_mrr)
{
    uint16 v16;
    extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
    if (v16 & REG_MIRROR_ENABLE)
    {
        *enable = 1;
        *mirror_port = v16 & REG_CAPTURE_PORT_M;
        *blk_no_mrr = v16 & REG_BLK_NOT_MIRROR;
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
        *ing_pmap = v16 & REG_INGRESS_MIRROR_M;
        extsw_rreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
        *eg_pmap = v16 & REG_EGRESS_MIRROR_M;
    }
    else
    {
        *enable = 0;
    }
}
#if defined(CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT)
void bcmsw_port_trunk_set(unsigned int hash_sel)
{
    uint8 v8;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    v8 &= ~(TRUNK_HASH_SEL_M<<TRUNK_HASH_SEL_S); /* Clear old hash selection first */
    v8 |= ( (hash_sel & TRUNK_HASH_SEL_M) << TRUNK_HASH_SEL_S ); /* Set Hash Selection */
    extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    printk("LAG/Trunking hash selection changed <0x%01x>\n",v8);
}
void bcmsw_port_trunk_get(int *enable, unsigned int *hash_sel, unsigned int *grp0_pmap, unsigned int *grp1_pmap)
{
    uint16 v16;
    uint8 v8;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL , &v16, 2);
    *grp0_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;
    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL+2 , &v16, 2);
    *grp1_pmap = (v16 >> TRUNK_EN_GRP_S) & TRUNK_EN_GRP_M ;

    extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
    *enable = (v8 >> TRUNK_EN_LOCAL_S) & TRUNK_EN_LOCAL_M;
    *hash_sel = (v8 >> TRUNK_HASH_SEL_S) & TRUNK_HASH_SEL_M;
}
#endif /* CONFIG_BCM_SWITCH_PORT_TRUNK_SUPPORT */
void bcmsw_port_mirror_set (int enable, int mirror_port, unsigned int ing_pmap, unsigned int eg_pmap, unsigned int blk_no_mrr)
{
    uint16 v16;
    if (enable)
    {
        v16 = REG_MIRROR_ENABLE;
        v16 |= (mirror_port & REG_CAPTURE_PORT_M);
        v16 |= blk_no_mrr?REG_BLK_NOT_MIRROR:0;

        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
        v16 = ing_pmap & REG_INGRESS_MIRROR_M;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_INGRESS_CTRL, &v16, sizeof(v16));
        v16 = eg_pmap & REG_INGRESS_MIRROR_M;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_EGRESS_CTRL, &v16, sizeof(v16));
    }
    else
    {
        v16  = 0;
        extsw_wreg_wrap(PAGE_MANAGEMENT, REG_MIRROR_CAPTURE_CTRL, &v16, sizeof(v16));
    }
}

inline static void extsw_reg16_bit_ops(uint16 page, uint16 reg, int bit, int on)
{
    uint16 val16;

    extsw_rreg_wrap(page, reg, &val16, 2);
    val16 &= ~(1 << bit);
    val16 |= on << bit;
    extsw_wreg_wrap(page, reg, &val16, 2);
}

int bcmsw_set_mac_port_state(int unit, int phy_port, int link, int speed, int duplex)
{
    uint8 v8, v8cur;

#if defined(STAR_FIGHTER2)
    if (!IsExternalSwitchUnit(unit)) 
    {
        PHY_STAT ps = {0};
        ps.lnk = link;
        if ( link && speed != 0 )
        {
            if ( speed == 2500 )
            {
                ps.spd2500 = 1;
            }
            else if ( speed == 1000 )
            {
                ps.spd1000 = 1;
            }
            else if ( speed == 100 )
            {
                ps.spd100 = 1;
            }
            else
            {
                ps.spd10 = 1;
            }
            ps.fdx = duplex > 0;
        }

        ethsw_set_mac_hw(phy_port, ps);
        return 0;
    }
#endif

    if (IsExternalSwitchUnit(unit)) 
    {
        extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8cur, 1);
    } 
    else
    {
        ethsw_rreg(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8cur, 1);
    }

    if (link && speed)  /* If link is up and speed is known, set all */
    {
        v8 = REG_PORT_STATE_OVERRIDE | REG_PORT_STATE_LNK | (duplex? REG_PORT_STATE_FDX: 0) |
            (speed==1000? REG_PORT_STATE_1000: speed==100? REG_PORT_STATE_100: 0) |
            (v8cur & REG_PORT_GMII_SPEED_UP_2G);
    }
    else    /* If link is down or speed is unknow(0), change the link status only */
    {
        v8 = (v8cur & ~REG_PORT_STATE_RESERVE_0 & ~REG_PORT_STATE_LNK) | 
                REG_PORT_STATE_OVERRIDE | (link? REG_PORT_STATE_LNK: 0);
    }

    down(&bcm_ethlock_switch_config);

    if (IsExternalSwitchUnit(unit)) 
    {
        extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8, 1);
    } 
    else
    {
        ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + phy_port, &v8, 1);
    }

    up(&bcm_ethlock_switch_config);

    return 0;
}

int bcmsw_mac_rxtx_op(int unit, int phy_port, int get, int *disable)
{
    int rc = 0;
    u8 v8;

    if (IsExternalSwitchUnit(unit)) 
    {
        if (get)
        {
            extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            *disable = v8 & REG_PORT_CTRL_DISABLE;
        }
        else /* set */
        {
            extsw_rreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            v8 &= ~REG_PORT_CTRL_DISABLE;
            v8 |= (*disable & REG_PORT_CTRL_DISABLE);
            extsw_wreg_wrap(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
        }
    } 
    else
    {
#if defined(STAR_FIGHTER2)
        rc = bcmsw_unimac_rxtx_op(phy_port, get, disable);
#else
        if (get)
        {
            ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            *disable = v8 & REG_PORT_CTRL_DISABLE;
        }
        else
        {
            ethsw_rreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
            v8 &= ~REG_PORT_CTRL_DISABLE;
            v8 |= (*disable & REG_PORT_CTRL_DISABLE);
            ethsw_wreg(PAGE_CONTROL, REG_PORT_CTRL + phy_port, &v8, 1);
        }
#endif
    }

    return rc;
}

void bcmsw_enable_all_macs_rxtx(int enable)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    int lgp, lgpMap, portMap = pDevCtrl->allPortMap;

    for (lgp = 0; lgp < MAX_TOTAL_SWITCH_PORTS && portMap; lgp++)
    {
        lgpMap = 1 << lgp;
        if ((lgpMap & portMap) == 0) continue;
        portMap &= ~(lgpMap);

        /* Skip enabling WAN Only Port that has not be configured as WAN port */
        if (enable && (pDevCtrl->wanOnlyPorts & ~pDevCtrl->wanPort & lgpMap) != 0) continue;

        bcmsw_enable_mac_rxtx_log(lgp, enable);
    }
}

int bcmeapi_ioctl_ethsw_port_traffic_control(struct ethswctl_data *e)
{
    bcmsw_mac_rxtx_op(e->unit, e->port, e->type==TYPE_GET, 
        e->type==TYPE_GET?&e->ret_val:&e->val);
#if !defined(STAR_FIGHTER2)
    if (e->type == TYPE_SET && !(e->val & REG_PORT_CTRL_DISABLE))
    {
        PHY_STAT ps = {0};

        /* update link status bit in port state override sw register */
        ps = ethsw_phyid_stat(enet_sw_port_to_phyid(e->unit, e->port));
        if(ps.lnk) ethsw_set_mac_hw(PHYSICAL_PORT_TO_LOGICAL_PORT(e->unit, e->port), ps);
    }
#endif

    return BCM_E_NONE;
}

#if defined(STAR_FIGHTER2)

static void sf2_dump_page0(void)
{
    int i = 0, page = 0;
    volatile EthernetSwitchCore *e = ETHSW_CORE;
    EthernetSwitchCore *f = (void *)NULL;

    printk("#The Page0 Registers \n");
    for (i=0; i<9; i++) {
        printk("%02x %02x = 0x%02x (%u) \n", page,
                ((int)(uintptr_t)&f->port_traffic_ctrl[i])/4 & 0xFF, e->port_traffic_ctrl[i],
                e->port_traffic_ctrl[i]); /* 0x00 - 0x08 */
    }
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->switch_mode)/4 & 0xFF,
            e->switch_mode, e->switch_mode); /* 0x0b */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->pause_quanta)/4 & 0xFF,
            e->pause_quanta, e->pause_quanta); /*0x0c */
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->imp_port_state)/4 & 0xFF,
            e->imp_port_state, e->imp_port_state); /*0x0e */
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->led_refresh)/4 & 0xFF,
            e->led_refresh, e->led_refresh); /* 0x0f */
    for (i=0; i<2; i++) {
        printk("%02x %02x 0x%04x (%u) \n", page,
                ((int)(uintptr_t)&f->led_function[i])/4 & 0xFF, e->led_function[i].led_f,
                e->led_function[i].led_f); /* 0x10 */
    }
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->led_function_map)/4 & 0xFF,
            e->led_function_map, e->led_function_map); /* 0x14 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->led_enable_map)/4 & 0xFF,
            e->led_enable_map, e->led_enable_map); /* 0x16 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->led_mode_map0)/4 & 0xFF,
            e->led_mode_map0, e->led_mode_map0); /* 0x18 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->led_function_map1)/4 & 0xFF,
            e->led_function_map1, e->led_function_map1); /* 0x1a */
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->reserved2[3])/4 & 0xFF,
            e->reserved2[3], e->reserved2[3]); /* 0x1f */
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->port_forward_ctrl)/4 & 0xFF,
            e->port_forward_ctrl, e->port_forward_ctrl); /* 0x21 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->protected_port_selection)/4
            & 0xFF, e->protected_port_selection,
            e->protected_port_selection); /* 0x24 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->wan_port_select)/4 & 0xFF,
            e->wan_port_select, e->wan_port_select); /* 0x26 */
    printk("%02x %02x 0x%08x (%u) \n", page, ((int)(uintptr_t)&f->pause_capability)/4
            & 0xFF, e->pause_capability, e->pause_capability);/*0x28*/
    printk("%02x %02x 0x%02x (%u) \n", page,
            ((int)(uintptr_t)&f->reserved_multicast_control)/4 & 0xFF, e->reserved_multicast_control,
            e->reserved_multicast_control); /* 0x2f */
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->txq_flush_mode_control)/4 &
            0xFF, e->txq_flush_mode_control, e->txq_flush_mode_control); /* 0x31 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->ulf_forward_map)/4 & 0xFF,
            e->ulf_forward_map, e->ulf_forward_map); /* 0x32 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->mlf_forward_map)/4 & 0xFF,
            e->mlf_forward_map, e->mlf_forward_map); /* 0x34 */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->mlf_impc_forward_map)/4 &
            0xFF, e->mlf_impc_forward_map, e->mlf_impc_forward_map); /* 0x36 */
    printk("%02x %02x 0x%04x (%u) \n", page,
            ((int)(uintptr_t)&f->pause_pass_through_for_rx)/4 & 0xFF, e->pause_pass_through_for_rx,
            e->pause_pass_through_for_rx); /* 0x38 */
    printk("%02x %02x 0x%04x (%u) \n", page,
            ((int)(uintptr_t)&f->pause_pass_through_for_tx)/4 & 0xFF, e->pause_pass_through_for_tx,
            e->pause_pass_through_for_tx); /* 0x3a */
    printk("%02x %02x 0x%04x (%u) \n", page, ((int)(uintptr_t)&f->disable_learning)/4 & 0xFF,
            e->disable_learning, e->disable_learning); /* 0x3c */
    for (i=0; i<8; i++) {
        printk("%02x %02x 0x%02x (%u) \n", page,
                ((int)(uintptr_t)&f->port_state_override[i])/4 & 0xFF, e->port_state_override[i],
                e->port_state_override[i]); /* 0x58 - 0x5f */
    }
    printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->software_reset)/4 & 0xFF,
            e->software_reset, e->software_reset); /* 0x79 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->pause_frame_detection)/4 &
         0xFF, e->pause_frame_detection, e->pause_frame_detection); /* 0x80 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->fast_aging_ctrl)/4 & 0xFF,
               e->fast_aging_ctrl, e->fast_aging_ctrl); /* 0x88 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->fast_aging_port)/4 & 0xFF,
               e->fast_aging_port, e->fast_aging_port); /* 0x89 */
        printk("%02x %02x 0x%02x (%u) \n", page, ((int)(uintptr_t)&f->fast_aging_vid)/4 & 0xFF,
               e->fast_aging_vid, e->fast_aging_vid); /* 0x8a */

}

static int sf2_bcmsw_dump_mib_ext(int port, int type)
{
    unsigned int v32, errcnt;
    uint8 data[8] = {0};

    /* Display Tx statistics */
    printk("External Switch Stats : Port# %d\n",port);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, &v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, &v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, &v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, &v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("TxOctetsLo:             %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("TxOctetsHi:             %10u \n", v32);
//
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX64OCTPKTS, &v32, 4);
        printk("TxPkts64Octets:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX127OCTPKTS, &v32, 4);
        printk("TxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX255OCTPKTS, &v32, 4);
        printk("TxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX511OCTPKTS, &v32, 4);
        printk("TxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX1023OCTPKTS, &v32, 4);
        printk("TxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMAXOCTPKTS, &v32, 4);
        printk("TxPkts1024OrMoreOctets: %10u \n", v32);
//
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ0PKT, &v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ1PKT, &v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ2PKT, &v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ3PKT, &v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ4PKT, &v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ5PKT, &v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ6PKT, &v32, 4);
        printk("TxQ6Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ7PKT, &v32, 4);
        printk("TxQ7Pkts:               %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        printk("TxCol:                  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXPAUSEPKTS, &v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, &v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }

    /* Display Rx statistics */
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, &v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, &v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, &v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, &v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, &v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (type)
    {
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOCTETS, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("RxOctetsLo:             %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("RxOctetsHi:             %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXGOODOCT, data, DATA_TYPE_MIB_COUNT|8);
        v32 = (((uint32*)data)[0]);
        printk("RxGoodOctetsLo:         %10u \n", v32);
        v32 = (((uint32*)data)[1]);
        printk("RxGoodOctetsHi:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        printk("RxFragments:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXPAUSEPKTS, &v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSACHANGES, &v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX64OCTPKTS, &v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX127OCTPKTS, &v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX255OCTPKTS, &v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX511OCTPKTS, &v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX1023OCTPKTS, &v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMAXOCTPKTS, &v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJUMBOPKT , &v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, &v32, 4);
        errcnt += v32;
        extsw_rreg_wrap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, &v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);

    }

    return 0;
}
static void sf2_tc_to_cos_default(void)
{
    int i, j;
    uint16_t reg_addr;
    uint32_t val32;

    for (i = 0; i <= SF2_IMP0_PORT; i++) // all ports except 6
    {
        if (i == SF2_INEXISTANT_PORT) continue; // skip port 6
        reg_addr = SF2_REG_PORTN_TC_TO_COS + i * 4;
        val32 = 0;
        for (j = 0; j <= SF2_QOS_TC_MAX; j++) // all TC s
        {
            //  TC to COS one-one mapping
            val32 |= (j & SF2_QOS_COS_MASK) << (j * SF2_QOS_COS_SHIFT);
        }
        extsw_wreg_wrap(PAGE_QOS, reg_addr, &val32, 4);
        BCM_ENET_DEBUG("%s: Write to: len %d page 0x%x reg 0x%x val 0x%x\n", __FUNCTION__,
                        4, PAGE_QOS, reg_addr, val32);
    }
}

void sf2_qos_default(void)
{
    uint32_t val32;
    uint32_t port;
    /* Set Global QoS Control */
    extsw_rreg_wrap(PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    val32 |= SF2_QOS_P8_AGGREGATION_MODE;
    extsw_wreg_wrap(PAGE_QOS, SF2_REG_QOS_GLOBAL_CTRL, &val32, 4);
    /* set SP scheduling on all ports (including IMP) by default */
    for (port=0; port <= MAX_SWITCH_PORTS;  port++)
    {
        if (port == SF2_INEXISTANT_PORT) continue;
        extsw_rreg_wrap(PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
        val32 &= ~(PN_QOS_SCHED_SEL_M<<PN_QOS_SCHED_SEL_S); /* Clear Bits */
        val32 |= (SF2_ALL_Q_SP<<PN_QOS_SCHED_SEL_S); /* Set SP for all */
        extsw_wreg_wrap(PAGE_QOS_SCHEDULER, REG_PN_QOS_PRI_CTL_PORT_0 + (port), &val32, 4);
    }
    /* Set default TC to COS config */
    sf2_tc_to_cos_default();
}

void sf2_rgmii_config(void)
{
    volatile u32 *sw_rgmii_ctrl_reg;
    uint32 val32;
    int phy_id, crossbar_port;
    unsigned int port_map;
    int phy_port, unit;
    ETHERNET_MAC_INFO *EnetInfo = EnetGetEthernetMacInfo();
    ETHERNET_MAC_INFO *info;
    unsigned long port_flags;
    port_reg_offset_t *rgmii_regs;

    port_map = enet_get_consolidated_portmap();
    for (unit=0; unit < BP_MAX_ENET_MACS; unit++)
    {
        /* Get the switch info corresponding to unit */
        info = &EnetInfo[unit];

        for(phy_port = 0; phy_port < BCMENET_MAX_PHY_PORTS; phy_port++)
        {
            if (BP_IS_CROSSBAR_PORT(phy_port))
            {
                crossbar_port = BP_PHY_PORT_TO_CROSSBAR_PORT(phy_port);
                if (!BP_IS_CROSSBAR_PORT_DEFINED(info->sw, crossbar_port)) continue;
                phy_id = info->sw.crossbar[crossbar_port].phy_id;
                port_flags = info->sw.crossbar[crossbar_port].port_flags;
            }
            else
            {
                if (!(port_map & (1 << PHYSICAL_PORT_TO_LOGICAL_PORT(phy_port, unit)))) continue;
                phy_id = info->sw.phy_id[phy_port];
                port_flags = info->sw.port_flags[phy_port];

                /* Skip dynamic Runner port to connect through the cross bar */
                if (phy_id == BP_PHY_ID_NOT_SPECIFIED) continue;
            }

            if (!(IsRGMII(phy_id) || IsTMII(phy_id))) continue;

            if ( (rgmii_regs = get_rgmii_regs(phy_port)) == NULL) continue;

            // set pad controls for 1.8v operation.
            if (IsRGMII_1P8V(phy_id)) {
                sw_rgmii_ctrl_reg = (u32 *)SF2_MISC_MII_PAD_CTL + rgmii_regs->pad_offset;
                *sw_rgmii_ctrl_reg   = (int)0xf;
            }

            extsw_rreg_wrap(PAGE_SWITCH_EXTEND_REG, rgmii_regs->rgmii_ctrl, &val32, 4); /* Read the current register value */

            val32 &= ~SF2_RGMII_PORT_MODE_M; /* Clear Mode bits defaults and set based on interface type */
            val32 |= SF2_ENABLE_PORT_RGMII_INTF ; /* Enable the (R)(G)MII mode */
            if (IsRGMII(phy_id))
            {
                val32 |= SF2_TX_ID_DIS; /* Disable the RGMII Internal Delay */
                val32 |= (SF2_RGMII_PORT_MODE_EXT_GPHY_RGMII<<SF2_RGMII_PORT_MODE_S); /* Set port mode as RGMII */
            }
            else if (IsTMII(phy_id))
            {
                val32 |= (SF2_RGMII_PORT_MODE_EXT_EPHY_MII<<SF2_RGMII_PORT_MODE_S); /* For TMII - Port works as External MII */
            }
            extsw_wreg_wrap(PAGE_SWITCH_EXTEND_REG, rgmii_regs->rgmii_ctrl, &val32, 4); /* Set the value in register */

            if (IsRGMII(phy_id)) /* These delay setting applicable to RGMII only */
            {
                /* Phy Tx clk Delay is on by default. phy reg 24, shadow reg 7
                 * works with Rx delay -- ON by default. phy reg 28, shadow reg 3
                 * No action on the phy side unless specified in boardparms
                 */

                if (IsPortTxInternalDelay(port_flags)) {
                    /* Clear TX_ID_DIS */
                    val32 &= ~SF2_TX_ID_DIS;
                    extsw_wreg_wrap(PAGE_SWITCH_EXTEND_REG, rgmii_regs->rgmii_ctrl, &val32, 4); /* Set the value in register */
                }

                if (IsPortRxInternalDelay(port_flags)) {
                    /* Clear Rx bypass */
                    extsw_rreg_wrap(PAGE_SWITCH_EXTEND_REG, rgmii_regs->rgmii_rx_clock_delay_ctrl, &val32, 4);
                    val32 &= ~(SF2_RGMII_RX_CLK_IDDQ|SF2_RX_ID_BYPASS);
                    extsw_wreg_wrap(PAGE_SWITCH_EXTEND_REG, rgmii_regs->rgmii_rx_clock_delay_ctrl, &val32, 4);
                }
            }
        }
    }
}

void sf2_enable_2_5g(void)
{
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM94908)
    volatile u32 *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32 val32 = *sw_ctrl_reg;
    val32 |= SF2_IMP_2_5G_EN;
    *sw_ctrl_reg = val32;
#endif
}

void sf2_mdio_master_enable(void)
{
    volatile u32 *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32 val32 = *sw_ctrl_reg;
    val32 |= SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}
void sf2_mdio_master_disable(void)
{
    volatile u32 *sw_ctrl_reg = (void *)(SWITCH_REG_BASE);
    uint32 val32 = *sw_ctrl_reg;
    val32 &= ~SF2_MDIO_MASTER;
    *sw_ctrl_reg = val32;
}



// Little Endian Assmptions here. We do not need the BE counterparts for now.
void extsw_rreg_mmap(int page, int reg, uint8 *data_out, int len)
{
    uint32 addr = ((page << 8) + reg) << 2;
    uint32 val;
    uint64 data64 = 0;
    volatile unsigned *switch_directReadReg   = (unsigned int *) (SWITCH_DIRECT_DATA_RD_REG);
    void *data = &data64;
    // Add ASSERTs if address is not aligned for the requested len.

    volatile uint32 *base = (volatile uint32 *) (SWITCH_BASE + addr);
    BCM_ENET_DEBUG("%s: Read from %8p len %d page 0x%x reg 0x%x\n", __FUNCTION__,
            base, len, page, reg);

    spin_lock_bh(&bcm_extsw_access);
    val = *base;

    BCM_ENET_DEBUG(" Read from %8p len %d page 0x%x reg 0x%x val 0x%x\n",
            base, len, page, reg, (unsigned int)val);

    switch (len) {
        case 1:
            *(uint32 *)data = (uint8)val;
            break;
        case 2:
            *(uint32 *)data = (uint16)val;
            break;
        case 4:
            *(uint32 *)data = val;
            break;
        case 6:
            *(uint64 *)data    = val | ((uint64)(*switch_directReadReg & 0xffff)) << 32;
            break;
        case 8:
            *(uint64 *)data = val | ((uint64)*switch_directReadReg) << 32;
            break;
        default:
            printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
            break;
    }
    spin_unlock_bh(&bcm_extsw_access);
    memcpy(data_out, data, len);
    BCM_ENET_DEBUG(" Read from %8p len %d page 0x%x reg 0x%x data 0x%llx\n",
            base, len, page, reg, *(uint64 *)data);
}

void extsw_wreg_mmap(int page, int reg, uint8 *data_in, int len)
{
    uint32 addr = ((page << 8) + reg) << 2;
    uint32 val = 0;
    uint64 data64;
    volatile unsigned *switch_directWriteReg  = (unsigned int *) (SWITCH_DIRECT_DATA_WR_REG);
    void *data = &data64;
    // Add ASSERTs if address is not aligned for the requested len.

    volatile uint32 *base = (volatile uint32 *) (SWITCH_BASE + addr);
    memcpy(data, data_in, len);
    BCM_ENET_DEBUG("%s: Write  %llx to %8p len %d \n", __FUNCTION__, data64, base, len);

    spin_lock_bh(&bcm_extsw_access);
    switch (len) {
        case 1:
            val = *(uint8 *)data;
            break;
        case 2:
            val = *(uint16 *)data;
            break;
        case 4:
            val = *(uint32 *)data;
            break;
        case 6:
            //[] = m0m1m2m3m4m5????
            *switch_directWriteReg = (data64 >> 32) & 0xffff;
            val = (uint32 )data64;
            break;
        case 8:
            // for mac vid case, you have
            // m0m1m2m3m4m5,vidL,vidM
            *switch_directWriteReg = data64 >> 32;
            val = (uint32 )data64;
            break;
        default:
            spin_unlock_bh(&bcm_extsw_access);
            printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
            return;
    }
    *base = val;
    spin_unlock_bh(&bcm_extsw_access);
}

/*
 * Get/Set PID to TC mapping Table entry given ingress port
 * and mapped priority
 *** Input params
 * e->type  GET/SET
 * e->priority - mapped TC value, case of SET
 *** Output params
 * e->priority - mapped TC value, case of GET
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_pid_to_priority_mapping(struct ethswctl_data *e)
{

    uint32_t val32;

    BCM_ENET_DEBUG("Given uint %02d port %02d \n ", e->unit, e->port);

    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        printk("Invalid port number %02d \n", e->port);
        return BCM_E_ERROR;
    }
    down(&bcm_ethlock_switch_config);

    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
        e->priority = (val32 >> (e->port * QOS_TC_S)) & QOS_TC_M;
        BCM_ENET_DEBUG("port %d is mapped to priority: %d \n ", e->port, e->priority);
    } else {
        BCM_ENET_DEBUG("Given port: %02d priority: %02d \n ", e->port, e->priority);
        extsw_rreg_wrap(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
        val32 &= ~(QOS_TC_M << (e->port * QOS_TC_S));
        val32 |= (e->priority & QOS_TC_M) << (e->port * QOS_TC_S);
        extsw_wreg_wrap(PAGE_QOS, SF2_REG_PORT_ID_PRIO_MAP, (void *)&val32, 4);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/* This function just serves Star Fighter. Legacy External switch
 * goes with Robo.
 *
 * Get/Set StarFighter cos priority method
 *** Input params
 * e->type  GET/SET
 * e->pkt_type_mask - ipv4/ipv6:802.1p:static mac destination or port Id based
 * e->val - ingress classifier TC src selection -- DSCP, vlan pri,
 *        -  MAC addr, PORT based (default vlan tag)
 * e->port  per port
 *** Output params
 * Returns 0 for Success, Negative value for failure.
 */
static int qos_dscp_is_enabled(int port)
{
    u16 val16;
    extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static int qos_8021p_is_enabled(int port)
{
    u16 val16;
    extsw_rreg_wrap(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    return (val16 >> port) & 1;
}

static void enable_dscp_qos(int port, int enable)
{
    u16 val16;
    extsw_rreg_wrap(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    extsw_wreg_wrap(PAGE_QOS, REG_QOS_DSCP_EN, (void *)&val16, 2);
}

static void enable_8021p_qos(int port, int enable)
{
    u16 val16;
    extsw_rreg_wrap(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
    val16 &= ~(1 << port);
    val16 |= enable << port;
    extsw_wreg_wrap(PAGE_QOS, REG_QOS_8021P_EN, (void *)&val16, 2);
}

/* Note: Method values are UAPI definition */
static int isQoSMethodEnabled(int port, int method)
{
    switch(method)
    {
        case PORT_QOS:
        case MAC_QOS:
            return 1;
        case IEEE8021P_QOS:
            return qos_8021p_is_enabled(port);
        case DIFFSERV_QOS:
            return qos_dscp_is_enabled(port);
    }
    return 0;
}

/* Note: Method values are UAPI definition */
static void enableQosMethod(int port, int method, int enable)
{
    switch(method)
    {
        case MAC_QOS:
        case PORT_QOS:
            return;
        case IEEE8021P_QOS:
            if (enable)
            {
                enable_8021p_qos(port, 1); // Enable PCP for the port
            }
            else
            {
                enable_8021p_qos(port, 0);
            }
            return;
        case DIFFSERV_QOS:
            if (enable)
            {
                enable_dscp_qos(port, 1); // Enable DSCP for the port
            }
            else
            {
                enable_dscp_qos(port, 0);
            }
            return;
    }
}

#define QOS_METHOD_CNVT_UAPI_AND_REG(regQoS)  (~(regQoS) & SF2_QOS_TC_SRC_SEL_VAL_MASK)
#define QOS_METHODS_CNVT_UAPI_AND_REG(regQoSPorts)  (~(regQoSPorts) & 0xffff)
int sf2_cos_priority_method_config(struct ethswctl_data *e)
{
    uint16_t val16, reg_addr, pkt_type_mask, tc_sel_src ;
    uint32_t val32;
    int i, enable_qos;

    down(&bcm_ethlock_switch_config);

    BCM_ENET_DEBUG("%s port %d pkt_type 0x%x Given method: %02d \n ",__FUNCTION__,
            e->port, e->pkt_type_mask, e->val);
    if (e->port < 0 || e->port > SF2_IMP0_PORT ||  e->port == SF2_INEXISTANT_PORT) {
        BCM_ENET_DEBUG("%s parameter error, port %d \n", __FUNCTION__, e->port);
        return -BCM_E_PARAM;
    }
    reg_addr = SF2_REG_PORTN_TC_SELECT_TABLE + e->port * 2;
    pkt_type_mask = e->pkt_type_mask;
    if (e->type == TYPE_GET) {
        extsw_rreg_wrap(PAGE_QOS, reg_addr, &val16, 2);
        if (e->pkt_type_mask == SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            val32 = QOS_METHODS_CNVT_UAPI_AND_REG(val16);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = (val16 >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = isQoSMethodEnabled(e->port, QOS_METHOD_CNVT_UAPI_AND_REG(tc_sel_src));
                val32 |= !enable_qos << (16+i);
            }
        } else {
            pkt_type_mask &=  SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 =  (val16 >> (pkt_type_mask * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
            val16 = QOS_METHOD_CNVT_UAPI_AND_REG(val16);
            enable_qos = isQoSMethodEnabled(e->port, val16);
            val32 = (!enable_qos << 16) | val16;
        }
        // bits programmed in TC Select Table registers and software notion are bit inversed.
        e->ret_val = val32;
    } else { // TYPE_SET
        reg_addr = SF2_REG_PORTN_TC_SELECT_TABLE + e->port * 2;

        /* when pkt_type_mask is NOT SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src b0-1 2 bit show of TC.  b16-17 2bits contains disable bit.  */
        if (e->pkt_type_mask != SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL)
        {
            tc_sel_src = QOS_METHOD_CNVT_UAPI_AND_REG(e->val);
            extsw_rreg_wrap(PAGE_QOS, reg_addr, &val16, 2);
            pkt_type_mask = e->pkt_type_mask & SF2_QOS_TC_SRC_SEL_PKT_TYPE_MASK;
            val16 &= ~(SF2_QOS_TC_SRC_SEL_VAL_MASK << (pkt_type_mask * 2));
            val16 |=  (tc_sel_src & SF2_QOS_TC_SRC_SEL_VAL_MASK ) << (pkt_type_mask * 2);
            enable_qos = !((e->val >> 16) & 1);
            BCM_ENET_DEBUG("%s: Write to: len %d page 0x%x reg 0x%x val 0x%x\n",
                    __FUNCTION__, 2, PAGE_QOS, reg_addr, val16);
            enableQosMethod(e->port, e->val & SF2_QOS_TC_SRC_SEL_VAL_MASK, enable_qos);
        }
        else    /* SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL */
        /* when pkt_type_mask is SF2_QOS_TC_SRC_SEL_PKT_TYPE_ALL,
            tc_sel_src's lower 16 bits contains TC selections for all 8 packet types.
            higher 8 bits contains disable bit for corresponding methods in lower 16bits.  */
        {
            val16 = QOS_METHODS_CNVT_UAPI_AND_REG(e->val);
            for (i = 0; i < e->pkt_type_mask; i++)
            {
                tc_sel_src = ((e->val) >> (i * 2)) & SF2_QOS_TC_SRC_SEL_VAL_MASK;
                enable_qos = !((e->val >> (16 + i)) & 1);
                enableQosMethod(e->port, tc_sel_src, enable_qos);
            }
        }
        extsw_wreg_wrap(PAGE_QOS, reg_addr, &val16, 2);
    }

    up(&bcm_ethlock_switch_config);
    return 0;
}

/*
 * Get/Set StarFighter flow control options.
 *** Input params
 * e->type  GET/SET
 * e->sub_type Flow control type
 * e->val enable/disable above flow control type
 *** Output params
 * e->val has result for GET
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_pause_drop_ctrl(struct ethswctl_data *e)
{
    uint16 val = 0;
    uint16 val2 = 0;
    if (e->type == TYPE_SET)    { // SET
        extsw_rreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, (uint8 *)&val, 2);
        switch (e->sub_type) {
            case bcmSwitchFcMode:
                val2 = e->val? FC_CTRL_MODE_PORT: 0;
                extsw_wreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, (uint8 *)&val2, 2);
                return 0;
            case bcmSwitchQbasedpauseEn:
                val &= ~FC_QUEUE_BASED_PAUSE_EN;
                val |= e->val? FC_QUEUE_BASED_PAUSE_EN: 0;
                break;
            case bcmSwitchTxBasedFc:
                val &= ~FC_TX_BASED_CTRL_EN;
                val |= e->val? FC_TX_BASED_CTRL_EN: 0;
                val &= ~FC_RX_BASED_CTRL_EN;
                val |= e->val? 0: FC_RX_BASED_CTRL_EN;
                val &= ~FC_RX_DROP_EN;
                val |= e->val? 0: FC_RX_DROP_EN;
                break;
            case bcmSwitchTxQdropEn:
                val &= ~FC_TX_TXQ_DROP_EN;
                val |= e->val? FC_TX_TXQ_DROP_EN: 0;
                break;
            case bcmSwitchTxTotdropEn:
                val &= ~FC_TX_TOTAL_DROP_EN;
                val |= e->val? FC_TX_TOTAL_DROP_EN: 0;
                break;
            case bcmSwitchTxQpauseEn:
                val &= ~FC_TX_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEn:
                val &= ~FC_TX_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_TOTAL_PAUSE_EN: 0;
                break;
            case bcmSwitchTxQpauseEnImp0:
                val &= ~FC_TX_IMP0_TXQ_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TXQ_PAUSE_EN: 0;
                break;
            case bcmSwitchTxTotPauseEnImp0:
                val &= ~FC_TX_IMP0_TOTAL_PAUSE_EN;
                val |= e->val? FC_TX_IMP0_TOTAL_PAUSE_EN: 0;
                break;
            default:
                printk("%s unknown fc type %u \n", __FUNCTION__, (unsigned int)e->sub_type);
                return -BCM_E_ERROR;
                break;
        }
        extsw_wreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, (uint8 *)&val, 2);
    } else {
        //   GET
        val2 = 0;
        extsw_rreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_PAUSE_DROP_CTRL, (uint8 *)&val, 2);
        val2 |= val & FC_QUEUE_BASED_PAUSE_EN? 1 << bcmSwitchQbasedpauseEn: 0;
        val2 |= val & FC_TX_BASED_CTRL_EN? 1 << bcmSwitchTxBasedFc: 0;
        val2 |= val & FC_TX_TXQ_DROP_EN? 1 << bcmSwitchTxQdropEn: 0;
        val2 |= val & FC_TX_TOTAL_DROP_EN? 1 << bcmSwitchTxTotdropEn: 0;
        val2 |= val & FC_TX_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEn: 0;
        val2 |= val & FC_TX_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEn: 0;
        val2 |= val & FC_TX_IMP0_TXQ_PAUSE_EN? 1 << bcmSwitchTxQpauseEnImp0: 0;
        val2 |= val & FC_TX_IMP0_TOTAL_PAUSE_EN? 1 << bcmSwitchTxTotPauseEnImp0: 0;

        extsw_rreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, (uint8 *)&val, 2);
        val2 |= val & FC_CTRL_MODE_PORT? 1 << bcmSwitchFcMode: 0;
        e->val = val2;
        BCM_ENET_DEBUG("%s: val2 = 0x%x \n", __FUNCTION__, val2);
    }
    return 0;
}

/*
 * As part of setting StarFighter Egress shaper
 * configuration, turn on/off various shaper modes.
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  SET
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 * e->sub_type - Or'ed Flags
 * e->val = 1 | 0  for On or Off
 *  Output params None
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_port_shaper_config(struct ethswctl_data *e)
{
    uint16 page, reg;
    unsigned char q_shaper;

    if (e->type == TYPE_SET) {
        /* confiure requested shaper parameters.
         * Notice: each q has its separate page.
         */
        q_shaper = e->queue >= 0;
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        if (e->sub_type & SHAPER_ENABLE) {
            reg = SF2_REG_SHAPER_ENB;
            extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_RATE_PACKET_MODE) {
            reg = SF2_REG_SHAPER_ENB_PKT_BASED;
            extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_BLOCKING_MODE) {
            reg = SF2_REG_SHAPER_BLK_CTRL_ENB;
            extsw_reg16_bit_ops(page, reg, e->port, e->val);
        }
        if (e->sub_type & SHAPER_INCLUDE_IFG) {
            // applies only for port shaper
            if (!q_shaper) {
                reg = SF2_REG_SHAPER_INC_IFG_CTRL;
                extsw_reg16_bit_ops(page, reg, e->port, e->val);
            }
        }
    }

    return 0;
}

/*
 * Get/Set StarFighter Egress shaper control
 *** Input params
 * e->port  egress port that is configured
 * e->unit  switch unit
 * e->type  GET/SET
 * e->limit egress rate control limit in
 *          64 kbps(Byte mode) 125 pps packet mode
 * e->burst_size egress burst in 64 Byte units(Byte mode)
 *          or in packets (packet mode)
 * e->queue egress queue if specified (per queue config)
 *          or -1 is not specified
 *** Output params
 * e->vptr has result copied for GET
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_port_erc_config(struct ethswctl_data *e)
{
    uint16 val16, page, reg;
    uint32 val32;
    uint32 pkt_flag;
    unsigned char q_shaper = e->queue >= 0;

    if (e->type == TYPE_SET) {

        /* find queue or port page*/
        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        /* find the shaper mode */
        pkt_flag =  (e->sub_type == SHAPER_PACKET_MODE) ? SHAPER_PACKET_MODE : 0;

        /* configure shaper rate limit; limit = 0 means disable shaper */
        val32 = 0; /* reset rate limiting by default */
        if (e->limit)
        {
            if (pkt_flag == SHAPER_PACKET_MODE)
            {
                val32 = e->limit/125; /* shaper rate config in 125pps units */
            }
            else {
                val32 = e->limit/64; /* shaper rate config in 64Kbps units */
            }
            if (!val32) { 
                val32 = 1; /* At least 64Kbps */
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_RATE_PKT:
                                                  SF2_REG_PN_SHAPER_RATE_BYTE;
        extsw_wreg_wrap(page, reg + e->port * 4, &val32, 4);

        /* configure shaper burst size */
        val32 = 0; /* reset burst size by default */
        if (e->limit) { /* Only set burst size if shaper is getting enabled */
            if (pkt_flag == SHAPER_PACKET_MODE) {
                val32 = e->burst_size; /* shaper burst config in 1 packet units */
            }
            else {
                val32 = (e->burst_size /* Kbits */ * 1000)/(8*64); /* shaper burst config in 64Byte units */
            }
            if (!val32) { 
                val32 = 1; 
            }
            val32 = val32 & SHAPER_RATE_BURST_VAL_MASK;
        }
        reg  =   pkt_flag == SHAPER_PACKET_MODE? SF2_REG_PN_SHAPER_BURST_SZ_PKT:
                                                  SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        extsw_wreg_wrap(page, reg + e->port * 4, &val32, 4);

        /* enable shaper for byte mode or pkt mode as the case may be. */
        extsw_rreg_wrap(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        val16 &= ~(1 << e->port);
        val16 |= pkt_flag == SHAPER_PACKET_MODE? (1 << e->port): 0;
        extsw_wreg_wrap(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        
        /* Enable/disable shaper */
        extsw_rreg_wrap(page, SF2_REG_SHAPER_ENB, &val16, 2);
        val16 &= ~(1 << e->port); /* Disable Shaper */
        val16 |= e->limit? (1 << e->port): 0; /* Enable Shaper, if needed */
        extsw_wreg_wrap(page, SF2_REG_SHAPER_ENB, &val16, 2);

        return 0;
    } else {
        /* Egress shaper stats*/
        egress_shaper_stats_t stats;

        page = q_shaper? PAGE_Q0_EGRESS_SHAPER + e->queue:
                                    PAGE_PORT_EGRESS_SHAPER;
        extsw_rreg_wrap(page, SF2_REG_SHAPER_ENB_PKT_BASED, &val16, 2);
        pkt_flag = (val16 & (1 << e->port))? 1: 0;
        stats.egress_shaper_flags = 0;
        stats.egress_shaper_flags |= pkt_flag? SHAPER_RATE_PACKET_MODE: 0;

        reg = pkt_flag? SF2_REG_PN_SHAPER_RATE_PKT: SF2_REG_PN_SHAPER_RATE_BYTE;
        extsw_rreg_wrap(page, reg + e->port * 4, &val32, 4);
        stats.egress_rate_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = pkt_flag? SF2_REG_PN_SHAPER_BURST_SZ_PKT: SF2_REG_PN_SHAPER_BURST_SZ_BYTE;
        extsw_rreg_wrap(page, reg + e->port * 4, &val32, 4);
        stats.egress_burst_sz_cfg = val32 & SHAPER_RATE_BURST_VAL_MASK;

        reg = SF2_REG_PN_SHAPER_STAT;
        extsw_rreg_wrap(page, reg + e->port * 4, &val32, 4);
        stats.egress_cur_tokens = val32 & SHAPER_STAT_COUNT_MASK;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_OVF_MASK? SHAPER_OVF_FLAG: 0;
        stats.egress_shaper_flags |= val32 & SHAPER_STAT_INPF_MASK? SHAPER_INPF_FLAG: 0;

        extsw_rreg_wrap(page, SF2_REG_SHAPER_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_ENABLE: 0;

        extsw_rreg_wrap(page, SF2_REG_SHAPER_BLK_CTRL_ENB, &val16, 2);
        stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_BLOCKING_MODE: 0;

        // applies only for port shaper
        if (!q_shaper) {
            extsw_rreg_wrap(page, SF2_REG_SHAPER_INC_IFG_CTRL, &val16, 2);
            stats.egress_shaper_flags |= (val16 & (1 << e->port))? SHAPER_INCLUDE_IFG: 0;
        }

        /* Convert the return values based on mode */
        if (pkt_flag)
        {
            stats.egress_rate_cfg *= 125; /* Shaper rate in 125pps unit */
            /* stats.egress_burst_sz_cfg  - burst unit in packets */
        }
        else {
            stats.egress_rate_cfg *= 64; /* Shaper rate in 64Kbps unit */
            stats.egress_burst_sz_cfg = (stats.egress_burst_sz_cfg*8*64)/1000; /* Shaper burst is in 64Byte unit - convert into kbits */
        }
        if (e->vptr) {
            if (copy_to_user (e->vptr, &stats, sizeof(egress_shaper_stats_t))) {
                return -EFAULT;
            }
        } else {
            // Just support Legacy API
            e->limit = stats.egress_rate_cfg;
            e->burst_size =  stats.egress_burst_sz_cfg;
        }
    }
    return 0;
}

/*
 * Get/Set StarFighter switch Flowcontrol thresholds.
 *** Input params
 * e->type  GET/SET
 * e->sw_ctrl_type buffer threshold type
 * e->port that determines LAN/IMP0/IMP1 to pick the register set
 * e->val  buffer threshold value to write
 *** Output params
 * e->val has buffer threshold value read for GET
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_prio_control(struct ethswctl_data *e)
{
    uint16_t val16;
    int reg = 0, page, curThreConfMode, curMaxStreamNumber;

    down(&bcm_ethlock_switch_config);
    switch (e->sw_ctrl_type)
    {
        case bcmSwitchTxQThresholdConfigMode:
        case bcmSwitchTotalPorts:
        case bcmSwitchLinkUpLanPorts:
        case bcmSwitchLinkUpWanPorts:
        case bcmSwitchMaxStreams:
            break;
        case bcmSwitchTxQHiReserveThreshold:
            reg = REG_FC_LAN_TXQ_THD_RSV_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiHysteresisThreshold:
            reg = REG_FC_LAN_TXQ_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiPauseThreshold:
            reg = REG_FC_LAN_TXQ_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTxQHiDropThreshold:
            reg = REG_FC_LAN_TXQ_THD_DROP_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalHysteresisThreshold:
            reg = REG_FC_LAN_TOTAL_THD_HYST_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalPauseThreshold:
            reg = REG_FC_LAN_TOTAL_THD_PAUSE_QN0 + (e->priority * 2);
            break;
        case bcmSwitchTotalDropThreshold:
            reg = REG_FC_LAN_TOTAL_THD_DROP_QN0 + (e->priority * 2);
            break;
        default:
            BCM_ENET_NOTICE("Unknown threshold type \n");
            up(&bcm_ethlock_switch_config);
            return -BCM_E_PARAM;
    }

    if(e->port == SF2_IMP0_PORT)
    {
        page = PAGE_FC_IMP0_TXQ;
    } else if(e->port == SF2_WAN_IMP1_PORT) {
        page = PAGE_FC_IMP1_TXQ;
    } else if ((e->port < SF2_IMP0_PORT) && (e->port != SF2_INEXISTANT_PORT)) {
        page = PAGE_FC_LAN_TXQ;
    } else {
        BCM_ENET_NOTICE("port # %d error \n", e->port);
        up(&bcm_ethlock_switch_config);
        return -BCM_E_PARAM;
    }

    BCM_ENET_DEBUG("Threshold: page %d  register offset = %#4x", page, reg);
    /* select port if port based threshold configuration in force */
    if (page == PAGE_FC_LAN_TXQ)
    {
        extsw_rreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_MODE, (uint8 *)&val16, 2);
        if (val16 & FC_CTRL_MODE_PORT) {
            /* port number to port select register */
            val16 = 1 << (REG_FC_CTRL_PORT_P0 + e->port);
            extsw_wreg_wrap(PAGE_FLOW_CTRL_XTN, REG_FC_CTRL_PORT_SEL, (uint8 *)&val16, 2);
        }
    }

    if (e->type == TYPE_GET)
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                e->val = queThreConfMode;
                break;
            case bcmSwitchTotalPorts:
                e->val = enet_get_total_ports_num() + 1;
                break;
            case bcmSwitchLinkUpLanPorts:
                e->val = pVnetDev0_g->lanUpPorts;
                break;
            case bcmSwitchLinkUpWanPorts:
                e->val = pVnetDev0_g->wanUpPorts;
                break;
            case bcmSwitchMaxStreams:
                e->val = maxStreamNumber;
                break;
            default:
                extsw_rreg_wrap(page, reg, (uint8_t *)&val16, 2);
                BCM_ENET_DEBUG("Threshold read = %4x", val16);
                e->val = val16;
        }
    } 
    else
    {
        switch (e->sw_ctrl_type)
        {
            case bcmSwitchTxQThresholdConfigMode:
                curThreConfMode = queThreConfMode;
                queThreConfMode = (uint32_t)e->val;
                if (curThreConfMode != queThreConfMode && queThreConfMode != ThreModeManual )
                {
                    sf2_conf_que_thred();
                }
                break;
            case bcmSwitchMaxStreams:
                curMaxStreamNumber = maxStreamNumber;
                maxStreamNumber = e->val;
                if (maxStreamNumber == -1) maxStreamNumber = MaxStreamNumber;
                if (curMaxStreamNumber != maxStreamNumber && queThreConfMode != ThreModeManual )
                {
                    sf2_conf_que_thred();
                }
                break;
            default:
                val16 = (uint32_t)e->val;
                BCM_ENET_DEBUG("e->val is = %4x", e->val);
                extsw_wreg_wrap(page, reg, (uint8_t *)&val16, 2);
        }
    }
    up(&bcm_ethlock_switch_config);
    return 0;
}

/*
 * Get/Set StarFighter port scheduling policy
 *** Input params
 * e->type  GET/SET
 * e->port_qos_sched.num_spq  Tells SP/WRR policy to use on the port's queues
 * e->port_qos_sched.wrr_type Granularity packet or 256 byte
 * e->port  per port
 *** Output params
 * e->val has current sched policy - GET
 * Returns 0 for Success, Negative value for failure.
 */
int sf2_cosq_sched(struct ethswctl_data *e)
{
    int reg;
    int i, j;
    uint8 data[8];
    uint8 val8 = 0;

    down(&bcm_ethlock_switch_config);

    reg = REG_PN_QOS_PRI_CTL_PORT_0 + e->port * REG_PN_QOS_PRI_CTL_SZ;
    extsw_rreg_wrap(PAGE_QOS_SCHEDULER, reg, (uint8_t *)&val8, REG_PN_QOS_PRI_CTL_SZ);
    if (e->type == TYPE_GET) {
        switch ((val8 >> PN_QOS_SCHED_SEL_S ) & PN_QOS_SCHED_SEL_M)
        {
            case 0:
                e->port_qos_sched.sched_mode = BCM_COSQ_STRICT;
                break;
            case 5:
                e->port_qos_sched.sched_mode = BCM_COSQ_WRR;
                break;
            default:
                e->port_qos_sched.sched_mode = BCM_COSQ_COMBO;
                e->port_qos_sched.num_spq    = (val8 & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                break;
        }
        e->port_qos_sched.port_qos_caps = QOS_SCHED_SP_CAP | QOS_SCHED_WRR_CAP | QOS_SCHED_WDR_CAP |
                                        QOS_SCHED_COMBO | QOS_PORT_SHAPER_CAP | QOS_QUEUE_SHAPER_CAP;
        e->port_qos_sched.max_egress_q = NUM_EGRESS_QUEUES;
        e->port_qos_sched.max_egress_spq = MAX_EGRESS_SPQ;
        e->port_qos_sched.wrr_type = (val8 >> PN_QOS_WDRR_GRAN_S) & PN_QOS_WDRR_GRAN_M?
                       QOS_ENUM_WRR_PKT: QOS_ENUM_WDRR_PKT;
        extsw_rreg_wrap(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                       e->port * REG_PN_QOS_WEIGHTS, (uint8_t *)data, DATA_TYPE_HOST_ENDIAN|REG_PN_QOS_WEIGHTS);
        for (i = 0; i < BCM_COS_COUNT; i++) {
            e->weights[i] = data[i];
        }
        BCM_ENET_DEBUG("GET: sched mode %d num_spq %d port %d \n",
                    e->port_qos_sched.sched_mode, e->port_qos_sched.num_spq, e->port );
        BCM_ENET_DEBUG("GET: val[0] %#x val[1] %#x val[2] %#x val[3] %#x \n",
                         data[0], data[1], data[2], data[3]);
        BCM_ENET_DEBUG(" GET: val[4] %#x val[5] %#x val[6] %#x val[7] %#x \n",
                         data[4], data[5], data[6], data[7]);
    } else { // TYPE_SET
        val8 &= ~(PN_QOS_SCHED_SEL_M << PN_QOS_SCHED_SEL_S);
        if (e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                val8 |= (SF2_ALL_Q_WRR & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
        } else if ((e->port_qos_sched.sched_mode == BCM_COSQ_SP) &&
                           (e->port_qos_sched.num_spq == 0)) {
                val8 |= (SF2_ALL_Q_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;

        } else {
            switch (e->port_qos_sched.num_spq) {
                case 1:
                    val8 |= (SF2_Q7_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 2:
                    val8 |= (SF2_Q7_Q6_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 3:
                    val8 |= (SF2_Q7_Q5_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                case 4:
                    val8 |= (SF2_Q7_Q4_SP & PN_QOS_SCHED_SEL_M) << PN_QOS_SCHED_SEL_S;
                    break;
                default:
                    BCM_ENET_DEBUG("Incorrect num_spq param %d", e->port_qos_sched.num_spq);
                    up(&bcm_ethlock_switch_config);
                    return -BCM_E_PARAM;
                    break;
            }
        }
        if (e->port_qos_sched.wrr_type == QOS_ENUM_WRR_PKT) {
            val8 |= SF2_WRR_PKT << PN_QOS_WDRR_GRAN_S;
        } else if (e->port_qos_sched.wrr_type == QOS_ENUM_WDRR_PKT) {
            val8 |= SF2_WDRR_PKT << PN_QOS_WDRR_GRAN_S;
        }
        extsw_wreg_wrap(PAGE_QOS_SCHEDULER, reg, (uint8_t *)&val8, REG_PN_QOS_PRI_CTL_SZ);
        BCM_ENET_DEBUG("SET: sched mode %d num_spq %d weights_upper %d port %d val %#x \n",
                    e->port_qos_sched.sched_mode, e->port_qos_sched.num_spq,
                    e->port_qos_sched.weights_upper, e->port, val8);
 // programming queue weights.
        if (e->port_qos_sched.num_spq || e->port_qos_sched.sched_mode == BCM_COSQ_WRR) {
                      // some or all queues in weighted mode.
            extsw_rreg_wrap(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 +
                           e->port * REG_PN_QOS_WEIGHTS, (uint8_t *)data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
            i = e->port_qos_sched.weights_upper? (BCM_COS_COUNT/2): 0;
            for (j = 0; j < BCM_COS_COUNT/2; i++, j++) {
                data[i] = e->weights[i];
            }
            extsw_wreg_wrap(PAGE_QOS_SCHEDULER, REG_PN_QOS_WEIGHT_PORT_0 + e->port * REG_PN_QOS_WEIGHTS,
                            (uint8_t *)data, DATA_TYPE_HOST_ENDIAN | REG_PN_QOS_WEIGHTS);
            BCM_ENET_DEBUG("SET: val[0] %#x val[1] %#x val[2] %#x val[3] %#x \n",
                         data[0], data[1], data[2], data[3]);
            BCM_ENET_DEBUG(" SET: val[4] %#x val[5] %#x val[6] %#x val[7] %#x \n",
                         data[4], data[5], data[6], data[7]);
        }
    } // SET
    up(&bcm_ethlock_switch_config);
    return 0;
}

#if defined(ACB_ALGORITHM2)
static void sf2_set_acb_algorithm(int algorithm)
{
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
    uint32_t val32, q;

    if (algorithm) // ACB_ALGORITHM2
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));

        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~((SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S)|
                    (SF2_ACB_QUE_PKT_LEN_M<<SF2_ACB_QUE_PKT_LEN_S));
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }

        *sf2_acb_control |= SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
            (0x32<<SF2_ACB_EOP_DELAY_S);
    }
    else
    {
        *sf2_acb_control &= ~(SF2_ACB_EN|(SF2_ACB_ALGORITHM_M<<SF2_ACB_ALGORITHM_S)|
                (SF2_ACB_FLUSH_M<<SF2_ACB_FLUSH_S)|(SF2_ACB_EOP_DELAY_M<<SF2_ACB_EOP_DELAY_S));
        *sf2_acb_control &= ~SF2_ACB_EN;
        for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
        {
            val32 = *(sf2_acb_que_config + q);
            val32 &= ~(SF2_ACB_QUE_PKT_LEN_M << SF2_ACB_QUE_PKT_LEN_S);
            val32 |= 6 << SF2_ACB_QUE_PKT_LEN_S;
            val32 |= (SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
            *(sf2_acb_que_config + q) =  val32;
            *(sf2_acb_que_in_flight + q) = 0;
        }
        *sf2_acb_control |= SF2_ACB_EN;
    }
}
#endif

static int sf2_config_acb (struct ethswctl_data *e)
{
    uint32_t val32, val;
    acb_q_params_t acb_conf_info;
    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    volatile uint32_t *sf2_acb_que_in_flight = (void *)(SF2_ACB_QUE0_PKTS_IN_FLIGHT);
#endif

    if (e->queue < 0 || e->queue > SF2_ACB_QUE_MAX) {
        printk("%s parameter error, queue 0x%x \n", 	__FUNCTION__, e->queue);
        return BCM_E_PARAM;
    }
    val   = *sf2_acb_xon_thresh;
    val32 = *(sf2_acb_que_config + e->queue);
    if (e->type == TYPE_GET) {
        switch (e->sw_ctrl_type) {
            case acb_en:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                break;
            case acb_flush:
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
                break;
            case acb_algorithm:
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
                break;
#endif
            case acb_tot_xon_hyst:
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                    & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_xon_hyst:
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_pessimistic_mode:
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                break;
            case acb_q_total_xon_en:
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                break;
            case acb_q_xon_en:
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                break;
            case acb_q_total_xoff_en:
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                break;
            case acb_q_pkt_len:
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                break;
            case acb_q_tot_xoff_thresh:
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
            case acb_q_xoff_thresh:
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                break;
#endif
            case acb_parms_all:
                acb_conf_info.acb_en =  *sf2_acb_control & SF2_ACB_EN;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.eop_delay =  (*sf2_acb_control >> SF2_ACB_EOP_DELAY_S) & SF2_ACB_EOP_DELAY_M;
                acb_conf_info.flush =  (*sf2_acb_control >> SF2_ACB_FLUSH_S) & SF2_ACB_FLUSH_M;
                acb_conf_info.algorithm =  (*sf2_acb_control >> SF2_ACB_ALGORITHM_S) & SF2_ACB_ALGORITHM_M;
#endif
                acb_conf_info.total_xon_hyst =  (val >> SF2_ACB_TOTAL_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.xon_hyst =  (val >> SF2_ACB_XON_BUFS_S)
                                                        & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.pessimistic_mode = (val32  >> SF2_ACB_QUE_PESSIMISTIC_S)
                                                        & SF2_ACB_QUE_PESSIMISTIC_M;
                acb_conf_info.acb_queue_config.total_xon_en = (val32  >> SF2_ACB_QUE_TOTAL_XON_S)
                                                         & SF2_ACB_QUE_TOTAL_XON_M;
                acb_conf_info.acb_queue_config.xon_en = (val32  >> SF2_ACB_QUE_XON_S)
                                                         & SF2_ACB_QUE_XON_M;
                acb_conf_info.acb_queue_config.total_xoff_en = (val32  >> SF2_ACB_QUE_TOTAL_XOFF_S)
                                                         & SF2_ACB_QUE_TOTAL_XOFF_M;
                acb_conf_info.acb_queue_config.pkt_len = (val32  >> SF2_ACB_QUE_PKT_LEN_S)
                                                         & SF2_ACB_QUE_PKT_LEN_M;
                acb_conf_info.acb_queue_config.total_xoff_threshold = (val32  >> SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
                acb_conf_info.acb_queue_config.xoff_threshold = (val32  >> SF2_ACB_QUE_XOFF_BUFS_S)
                                                         & SF2_ACB_BUFS_THRESH_M;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
                acb_conf_info.pkts_in_flight = *(sf2_acb_que_in_flight + e->queue) & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
#endif
                break;
            default:
                printk("%s: Get op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        if (copy_to_user (e->vptr, &acb_conf_info, sizeof(acb_q_params_t))) {
            return -EFAULT;
        }
    } else {  // SET
        switch (e->sw_ctrl_type) {
            case acb_en:
                if (e->val)
                    *sf2_acb_control |= SF2_ACB_EN;
                else
                    *sf2_acb_control &= ~SF2_ACB_EN;
                return 0;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_eop_delay:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_EOP_DELAY_M << SF2_ACB_EOP_DELAY_S);
                *sf2_acb_control |= (e->val & SF2_ACB_EOP_DELAY_M) << SF2_ACB_EOP_DELAY_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_flush:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_control &= ~(SF2_ACB_FLUSH_M << SF2_ACB_FLUSH_S);
                *sf2_acb_control |= (e->val & SF2_ACB_FLUSH_M) << SF2_ACB_FLUSH_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
            case acb_algorithm:
                sf2_set_acb_algorithm(e->val);
                return 0;
#endif
            case acb_tot_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_xon_hyst:
                *sf2_acb_control &= ~SF2_ACB_EN;
                *sf2_acb_xon_thresh = val | (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;
                *sf2_acb_control |= SF2_ACB_EN;
                return 0;
                break;
            case acb_q_pessimistic_mode:
                val32 &= ~(SF2_ACB_QUE_PESSIMISTIC_M << SF2_ACB_QUE_PESSIMISTIC_S);
                val32 |= (e->val & SF2_ACB_QUE_PESSIMISTIC_M) << SF2_ACB_QUE_PESSIMISTIC_S;
                break;
            case acb_q_total_xon_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XON_M << SF2_ACB_QUE_TOTAL_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_xon_en:
                val32 &= ~(SF2_ACB_QUE_XON_M << SF2_ACB_QUE_XON_S);
                val32 |= (e->val & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
                break;
            case acb_q_total_xoff_en:
                val32 &= ~(SF2_ACB_QUE_TOTAL_XOFF_M << SF2_ACB_QUE_TOTAL_XOFF_S);
                val32 |= (e->val & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
                break;
            case acb_q_pkt_len:
                val32 |=  (e->val & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
                break;
            case acb_q_tot_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S;
                break;
            case acb_q_xoff_thresh:
                val32 |=  (e->val & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
                break;
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
            case acb_q_pkts_in_flight:
                printk("Warning: This register should only be set by HW, but set here.\n");
                *sf2_acb_control &= ~SF2_ACB_EN;
                *(sf2_acb_que_in_flight + e->queue) = e->val & SF2_ACB_QUE_PKTS_IN_FLIGHT_M;
                *sf2_acb_control |= SF2_ACB_EN;
                return BCM_E_PARAM;
#endif
            default:
                printk("%s: Set op %#x Unsupported \n", __FUNCTION__, e->sw_ctrl_type);
                return BCM_E_PARAM;
                break;
        }
        *sf2_acb_control &= ~SF2_ACB_EN;
        *(sf2_acb_que_config + e->queue) = val32;
        *sf2_acb_control |= SF2_ACB_EN;
        return 0;
    }// Set
    return 0;
}

void sf2_conf_acb_conges_profile(int profile)
{

    volatile uint32_t *sf2_acb_control    = (void *) (SF2_ACB_CONTROL_REG);
    volatile uint32_t *sf2_acb_xon_thresh = (void *) (sf2_acb_control + 1);
    volatile uint32_t *sf2_acb_que_config = (void *) (sf2_acb_control + 2);
    acb_config_t *p = &acb_profiles [profile];
    uint32_t val32;
    int q;
    acb_queue_config_t *qp;

    // acb disable
    *sf2_acb_control &= ~SF2_ACB_EN;
    *sf2_acb_xon_thresh = (p->total_xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_TOTAL_XON_BUFS_S |
                          (p->xon_hyst & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_XON_BUFS_S;
    for (q = 0; q <= SF2_ACB_QUE_MAX; q++)
    {

        /*
         * We have made room to configure each of the 64 queues with differently
         * defined per q values.
         * Here, we are however, duplicating q0 profiled ACB  config on every queue so we do not
         * leave queues congested for ever when ACB is enabled by default.
         */
        val32 = 0;
        qp = &p->acb_queue_config[0];
        val32 |= (qp->pessimistic_mode & SF2_ACB_QUE_PESSIMISTIC_M)
                                        << SF2_ACB_QUE_PESSIMISTIC_S;
        val32 |= (qp->total_xon_en & SF2_ACB_QUE_TOTAL_XON_M) << SF2_ACB_QUE_TOTAL_XON_S;
        val32 |= (qp->xon_en & SF2_ACB_QUE_XON_M) << SF2_ACB_QUE_XON_S;
        val32 |= (qp->total_xoff_en & SF2_ACB_QUE_TOTAL_XOFF_M) << SF2_ACB_QUE_TOTAL_XOFF_S;
        val32 |= (qp->pkt_len & SF2_ACB_QUE_PKT_LEN_M) << SF2_ACB_QUE_PKT_LEN_S;
        val32 |= (qp->total_xoff_threshold & SF2_ACB_BUFS_THRESH_M)
                                        << SF2_ACB_QUE_TOTOAL_XOFF_BUFS_S;
        val32 |= (qp->xoff_threshold & SF2_ACB_BUFS_THRESH_M) << SF2_ACB_QUE_XOFF_BUFS_S;
        *(sf2_acb_que_config + q) =  val32;
    }
    // acb enable
#if defined(ACB_ALGORITHM2)   // For 138, Algorithm2 is applied
    *sf2_acb_control |= SF2_ACB_EN | (SF2_ACB_ALGORITHM_M << SF2_ACB_ALGORITHM_S);
#else
    *sf2_acb_control |= SF2_ACB_EN;
#endif
}

    /************************************************************
     * SF2 Flow Control Hardware Reset Default Threshold Values *
     * List the values as software configuration reference      *
     ************************************************************/
#ifdef DEFINE_SF2_FC_HARD_DEFAULT_VALUES  /* By default, not defined */
    /* LAN Ports Hardware Default Value */
static int16_t sf2_hrddef_lanport_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES] =
{
    { 0x18,  0x18,  0x18,  0x18, /* RSRVD */  0x18,  0x18,  0x18,  0x18},
    { 0x4b,  0x4f,  0x53,  0x57, /* HYSTR */  0x5b,  0x5f,  0x63,  0x67},
    { 0x97,  0x9f,  0xa7,  0xaf, /* PAUSE */  0xb7,  0xbf,  0xc7,  0xcf},
    {0x5cf, 0x5cf, 0x5cf, 0x5cf, /* DROP  */ 0x5cf, 0x5cf, 0x5ff, 0x5ff},
    /* Below is Total */
    {0x38f, 0x38f, 0x38f, 0x38f, /* HYSTR */ 0x38f, 0x38f, 0x38f, 0x38f},
    {0x40f, 0x417, 0x41f, 0x427, /* PAUSE */ 0x42f, 0x437, 0x43f, 0x447},
    {0x58f, 0x597, 0x59f, 0x5a7, /* DROP  */ 0x5af, 0x5b7, 0x5bf, 0x5c7}
};

    /* IMP0 Hardware Default Value */
static int16_t sf2_hrddef_imp0_thred[FC_THRED_TOTAL_TYPES][FC_LAN_TXQ_QUEUES] = 
{ 
    { 0x18,  0x18,  0x18,  0x18, /* RSVRD */  0x18,  0x18,  0x18,  0x18},
    { 0x63,  0x67,  0x6b,  0x6f, /* HYSTR */  0x73,  0x77,  0x7b,  0x7f},
    { 0xc7,  0xcf,  0xd7,  0xdf, /* PAUSE */  0xe7,  0xef,  0xf7,  0xff},
    {0x5ff, 0x5ff, 0x5ff, 0x5ff, /* DROP  */ 0x5ff, 0x5ff, 0x5ff, 0x5ff},
    /* Below is Total */
    {0x3bf, 0x3bf, 0x3bf, 0x3bf, /* HYSTR */ 0x3bf, 0x3bf, 0x3bf, 0x3bf},
    {0x43f, 0x447, 0x44f, 0x457, /* PAUSE */ 0x45f, 0x467, 0x46f, 0x477},
    {0x5bf, 0x5c7, 0x5cf, 0x5d7, /* DROP  */ 0x5df, 0x5e7, 0x5ef, 0x5f7}
};
#endif

static int get_next_queue(int cur_que, int for_wan, int reverse)
{
    int q;

    if(reverse == 0)
    {
        q = 0;
        if (cur_que != -1) q = cur_que + 1;

        for (; q < FC_LAN_TXQ_QUEUES; q++)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    else
    {
        q = FC_LAN_TXQ_QUEUES - 1;
        if (cur_que != -1) q = cur_que - 1;
        for (; q >= 0; q--)
        {
            if (for_wan)
            {
                if (wanQueMap & (1<<q)) return q;
            }
            else
            {
                if ((wanQueMap & (1<<q)) == 0) return q;
            }
        }
    }
    return -1;
}
#define GET_NEXT_WAN_QUE(CUR_QUE) get_next_queue(CUR_QUE, 1, 0)
#define GET_NEXT_LAN_QUE(CUR_QUE) get_next_queue(CUR_QUE, 0, 0)
#define GET_PREV_WAN_QUE(CUR_QUE) get_next_queue(CUR_QUE, 1, 1)
#define GET_PREV_LAN_QUE(CUR_QUE) get_next_queue(CUR_QUE, 0, 1)
    
static void sf2_conf_thred_2reg(int page, int reg, uint16_t tbl[][FC_LAN_TXQ_QUEUES])
{
    int t, q;

    for (t = 0; t < FC_THRED_TOTAL_TYPES; t++)
    {
        for (q = 0; q < FC_LAN_TXQ_QUEUES; q++)
        {
            extsw_wreg_wrap (page, reg + t*0x10 + q*2, (void*)&tbl[t][q], sizeof(tbl[0][0]));
        }
    }
}

static void sf2_conf_que_thred(void)
{
    int q, q1, t;
    int thredBase[FC_THRED_TOTAL_TYPES], maxStreams,
        maxFrameLength = 4096, lastLanQue, lastWanQue, wanUpPorts, lanUpPorts;

/* 
    The percentage of various factors for different condition
    from base computation result. Don't use parantheses to avoid underflow.
    The design goal is:
        o WAN queues have higher thredsholds than all LAN queues.
        o Higher queues have higher thresholds than lower queues in the same group.
*/

/* Define IMP Port over LAN port threshold factor */
#define IMP_OVER_WAN_QUE_FACTOR 100/100

/* 
   Define Threshold increment factor with higher priority queue to
   guarantee higher priority queue get slight more chance of resource.
   The value should not caused WAN/LAN threshold reserved when scaled up.
*/
#define LOW_QUE_OVER_HIGH_QUE_FACTOR 100/101

/* Define WAN Queue over LAN queu factor so that WAN queue gets higher resource chance */
#define LAN_QUE_OVER_WAN_QUE_FACTOR 100/105

/*
    PER Queue thredshold is to control resource ratio between multiple ports
    This parameter defines maximum stream number of traffic that will be controled
    strictly by Per Queue threads. When stream number is over this, configuration will
    become oversubscription in terms of stream number, thus total Thred will be kicked
    in first.
*/ 
#define PER_QUE_CONTROL_STREAMS 2

/*
    Define Factor of PerQueue Oversubscription Factors when streams are over PER_QUE_CONTROL_STREAMS.
    This is used to times to the difference to total thredsholds.
*/
#define PER_QUE_OVERSUB_FACTOR 2/100

    lanUpPorts = pVnetDev0_g->lanUpPorts;
    wanUpPorts = pVnetDev0_g->wanUpPorts;

    /* maxStreams is used to calculat Total Pause(Not Drop) to
        reserve head room in buffer to guarantee the minimum buffer by queue reserved threshold
        when stream number is below this number. Term "stream" means traffic to ONE queue. */ 
    if (queThreConfMode == ThreModeDynamic)
    {
        maxStreams = (lanUpPorts + wanUpPorts + 1) * FC_LAN_TXQ_QUEUES;
    }
    else
    {
        maxStreams = (enet_get_total_ports_num() + 1) * FC_LAN_TXQ_QUEUES;
    }

    if (maxStreams > maxStreamNumber)
    {
        maxStreams = maxStreamNumber;
    }

    /* Unify all computation to page unit */
    maxFrameLength /= SF2_BYTES_PER_PAGE;

    /* Set Reserved Threshold to one frame size to create hard guarantee for each queue */
    thredBase[FC_THRED_QUE_RSRVD_TYPE] = maxFrameLength;

    /* 
        Total Drop Threshold:
        When total queue length exceeds Total Drop, all packet will be dropped even
        for queues under Reserved Threshold. This can only happen when external device
        ignores Pause frame. As the values for protocol violation case or misconfiguration case, 
        the value is design as high as possible to minimum the impact of overrun above 
        the total Pause and as final guard to total buffer.  Thus set to to one packet size room 
        below hard limits.
    */
    thredBase[FC_THRED_TTL_DROP_TYPE] = SF2_MAX_BUFFER_IN_PAGE - maxFrameLength;

    /* 
        Compute Total Pause Threshold: 
        Need to guarantee hardware reserved threshold in EACH queuue.
        The value depends on simultaneous buffer requesting streams and frame length.
        The value is set to guarantee no drop
    */
    thredBase[FC_THRED_TTL_PAUSE_TYPE] = SF2_MAX_BUFFER_IN_PAGE - maxFrameLength * maxStreams;

    /* 
        Total Hysteresis:
        Hysteresis will reflect the hop count from this device to the source and
        the latency of each hop's resume operation. Set this as high as possible
        related to PAUSE but at least one frame below PAUSE and no lower than reserve.
    */
    thredBase[FC_THRED_TTL_HYSTR_TYPE] = thredBase[FC_THRED_TTL_PAUSE_TYPE] - maxFrameLength;

    /*
        Calculate Per Queue thresholds based on maxStreams first. 
        The Queue Thresholds will control the ratio before total Threshold is kicked in.
        But when the stream number is big, the Per queue threshold will be unnecessary too low.
        Do Per queue oversubscription if port number is more than strict control stream number 
    */
    thredBase[FC_THRED_QUE_PAUSE_TYPE] = thredBase[FC_THRED_TTL_PAUSE_TYPE]/PER_QUE_CONTROL_STREAMS;
    thredBase[FC_THRED_QUE_DROP_TYPE] = thredBase[FC_THRED_TTL_DROP_TYPE]/PER_QUE_CONTROL_STREAMS;

    if ( maxStreams > PER_QUE_CONTROL_STREAMS)
    {
        thredBase[FC_THRED_QUE_PAUSE_TYPE] = 
            thredBase[FC_THRED_QUE_PAUSE_TYPE] *100/(100 + maxStreams*100*PER_QUE_OVERSUB_FACTOR);
        thredBase[FC_THRED_QUE_DROP_TYPE] = 
            thredBase[FC_THRED_QUE_DROP_TYPE]*100/(100 + maxStreams*100*PER_QUE_OVERSUB_FACTOR);
    }

    /* Set all queue's Hysteresis to be one frame below the PAUSE */
    thredBase[FC_THRED_QUE_HYSTR_TYPE] = thredBase[FC_THRED_QUE_PAUSE_TYPE] - maxFrameLength;

    /* Compute base WAN queue threashold */
    lastWanQue = GET_PREV_WAN_QUE(-1);

    /* Now Scale WAN Queue Thredshold up */
    q1 = lastWanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_WAN_QUE(q))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            if ( q == q1) /* Initialize last Queue thresholds */
            {
                sf2_sw_port_thred[t][q] = thredBase[t];
            }
            else
            {
                switch (t)
                {
                    case FC_THRED_QUE_RSRVD_TYPE:
                        /* Set reserve threshold to be the same */
                        sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                        break;
                    case FC_THRED_QUE_HYSTR_TYPE:
                        /* Set Hysteresis one frame lower from Pause */
                        sf2_sw_port_thred[t][q] = sf2_sw_port_thred[FC_THRED_QUE_PAUSE_TYPE][q] - maxFrameLength;
                        break;
                    default:
                        /* Set the rest thresholds lowered down with Lower Queue Factor. */
                        sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                }
            }
        }
    }

    /* Now Scale LAN Queue Thredshold down */
    lastLanQue = GET_PREV_LAN_QUE(-1);
    q1 = lastLanQue;
    for (q = q1; q != -1; q1 = q, q = GET_PREV_LAN_QUE(q1))
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    /* Set reserve threshold to be the same */
                    sf2_sw_port_thred[t][q] = thredBase[FC_THRED_QUE_RSRVD_TYPE];
                    break;
                case FC_THRED_QUE_HYSTR_TYPE:
                    sf2_sw_port_thred[t][q] = sf2_sw_port_thred[FC_THRED_QUE_PAUSE_TYPE][q] - maxFrameLength;
                    break;
                default:
                    if (q == lastLanQue)
                    {
                        sf2_sw_port_thred[t][q] = thredBase[t];

                        /* If there is WAN Queue configured, scale down LAN queue with a factor */
                        if (lastWanQue != -1)
                        {
                            sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q] * LAN_QUE_OVER_WAN_QUE_FACTOR;
                        }
                    }
                    else
                    {
                        sf2_sw_port_thred[t][q] = sf2_sw_port_thred[t][q1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }

    /* Configure IMP port */
    for (q = FC_LAN_TXQ_QUEUES - 1; q >= 0; q--)
    {
        for (t = FC_THRED_TOTAL_TYPES - 1; t >= 0; t--)
        {
            switch (t)
            {
                case FC_THRED_QUE_RSRVD_TYPE:
                    sf2_imp0_thred[t][q] = thredBase[t];
                    break;
                case FC_THRED_QUE_HYSTR_TYPE:
                    sf2_imp0_thred[t][q] = sf2_imp0_thred[FC_THRED_QUE_PAUSE_TYPE][q] - maxFrameLength;
                    break;
                default:
                    if (q == FC_LAN_TXQ_QUEUES - 1)
                    {
                        sf2_imp0_thred[t][q] = thredBase[t] * IMP_OVER_WAN_QUE_FACTOR;
                    }
                    else
                    {
                        sf2_imp0_thred[t][q] = sf2_imp0_thred[t][q + 1] * LOW_QUE_OVER_HIGH_QUE_FACTOR;
                    }
            }
        }
    }
    sf2_conf_thred_2reg(PAGE_FC_LAN_TXQ, REG_FC_LAN_TXQ_THD_RSV_QN0, sf2_sw_port_thred);
    sf2_conf_thred_2reg(PAGE_FC_IMP0_TXQ, REG_FC_IMP0_TXQ_THD_RSV_QN0, sf2_imp0_thred);
}

int bcmeapi_ioctl_que_map(BcmEnet_devctrl *pDevCtrl, struct ethswctl_data *e)
{
    switch (e->type)
    {
        case TYPE_SET:
            if (e->val != -1) wanQueMap = e->val;
            if (e->priority != -1) queRemap = e->priority;
            sf2_conf_que_thred();
            break;
        default:
            break;
    }

    e->val = wanQueMap;
    e->priority = queRemap;
    return 0;
}

#endif

int bcmeapi_ioctl_extsw_port_storm_ctrl(struct ethswctl_data *e)
{
#if defined(STAR_FIGHTER2)
    uint32 val32;

    down(&bcm_ethlock_switch_config);
    if (e->type == TYPE_SET) {        
        /* configure storm control rate & burst size */
        extsw_rreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        val32 |= REG_PN_BUCK0_ENABLE_MASK | REG_PN_BUCK0_MODE_MASK; // use bucket 0
        val32 |= (e->limit & REG_PN_BUCK0_REF_CNT_M) << REG_PN_BUCK0_REF_CNT_S;
        val32 |= (e->burst_size & REG_PN_BUCK0_SIZE_M) << REG_PN_BUCK0_SIZE_S;
        extsw_wreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);

        /* pkt type */
        extsw_rreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
	    val32 &= ~(REG_PN_BUCK0_PKT_SEL_M << REG_PN_BUCK0_PKT_SEL_S);
	    val32 |= (e->pkt_type_mask & REG_PN_BUCK0_PKT_SEL_M) << REG_PN_BUCK0_PKT_SEL_S;
        extsw_wreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
    } else {        
        extsw_rreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_RATE_PORT_0 + e->port * 4, &val32, 4);
        e->limit = (val32 >> REG_PN_BUCK0_REF_CNT_S) & REG_PN_BUCK0_REF_CNT_M;
        e->burst_size = (val32 >> REG_PN_BUCK0_SIZE_S) & REG_PN_BUCK0_SIZE_M;

        extsw_rreg_wrap(PAGE_PORT_STORM_CONTROL, REG_PN_STORM_CTL_CFG_PORT_0 + e->port * 2, &val32, 4);
        e->pkt_type_mask = (val32 >> REG_PN_BUCK0_PKT_SEL_S) & REG_PN_BUCK0_PKT_SEL_M;
    }

    up(&bcm_ethlock_switch_config);
    return 0;
#else
    BCM_ENET_ERROR("No handler! \n");
    return 0;
#endif
}

int bcmeapi_ioctl_port_imp_remap(struct ethswctl_data *e)
{
	uint8 v8, port, idx = 0;
	uint16 v16;
	uint32 portmask = e->val, port_map;
	int port_not_imp_map[2] = {P4_PORT_ID, P5_PORT_ID};

	printk("Bonding Port enabled mask 0x%4.4x\n", portmask);

	/* change port_imp_map, it will not change port_imp_map if portmask is 0 */
	if (portmask) {
		for (port = 0; port < 4; port++) {
			if (portmask & (1 << port)) {
				port_imp_map[port] = IMP_PORT_ID;
			} else {
				port_imp_map[port] = port_not_imp_map[idx++];
				if (idx == 2)
					idx = 0;
			}
		}

		/* Configure forward based on Port Grouping
		 * By default all port's pbvlan is 0x1FF */
		port_map = enet_get_portmap(1); /* Get port map for external switch */
		for (port = 0; port < BP_MAX_SWITCH_PORTS; port++)
		{
		    if ( (port_imp_map[port] != -1) && ( (1<<port) & port_map ) )
		    {
			v16 = PBMAP_ALL & ~(PBMAP_P5_IMP | PBMAP_P4_IMP | PBMAP_MIPS);
			v16 |= (1<<port_imp_map[port]);
			extsw_set_pbvlan(port, v16);
			logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port,1)] = port_imp_map[port];
		    }
		}

		bcmsw_print_imp_port_grouping(port_map);
	}

	/* set port trunking register */
	if (portmask) {
		/* Set GRP0 Port Map */
		extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL , &v16, 2);
		v16 |= portmask;
		extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL, &v16, 2);

		/* Enable Trunking */
		extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
		v8 |= ( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S );
		extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
	} else {
		/* Disable Trunking */
		extsw_rreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);
		v8 &= ~( (1 & TRUNK_EN_LOCAL_M) << TRUNK_EN_LOCAL_S );
		extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_MAC_TRUNK_CTL, &v8, 1);

		/* Clear GRP0 Port Map */
		v16 = 0;
		extsw_wreg_wrap(PAGE_MAC_BASED_TRUNK, REG_TRUNK_GRP_CTL, &v16, 2);
	}
	
	return 0;
}
