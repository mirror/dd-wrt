/*
 * Copyright (C) 2002-2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/resource.h>
#include <linux/netdevice.h>
#include <linux/io.h>
#include <linux/mii.h>
#include <linux/workqueue.h>
#include <asm/hardware.h>
#include <linux/ixp_qmgr.h>

/* 32 bit offsets to be added to u32 *pointers */
#define MAC_TX_CNTRL1       0x00  // 0x000
#define MAC_TX_CNTRL2       0x01  // 0x004
#define MAC_RX_CNTRL1       0x04  // 0x010
#define MAC_RX_CNTRL2       0x05  // 0x014
#define MAC_RANDOM_SEED     0x08  // 0x020
#define MAC_THRESH_P_EMPTY  0x0c  // 0x030
#define MAC_THRESH_P_FULL   0x0e  // 0x038
#define MAC_BUF_SIZE_TX     0x10  // 0x040
#define MAC_TX_DEFER        0x14  // 0x050
#define MAC_RX_DEFER        0x15  // 0x054
#define MAC_TX_TWO_DEFER_1  0x18  // 0x060
#define MAC_TX_TWO_DEFER_2  0x19  // 0x064
#define MAC_SLOT_TIME       0x1c  // 0x070
#define MAC_MDIO_CMD        0x20  // 0x080 4 registers 0x20 - 0x23
#define MAC_MDIO_STS        0x24  // 0x090 4 registers 0x24 - 0x27
#define MAC_ADDR_MASK       0x28  // 0x0A0 6 registers 0x28 - 0x2d
#define MAC_ADDR            0x30  // 0x0C0 6 registers 0x30 - 0x35
#define MAC_INT_CLK_THRESH  0x38  // 0x0E0 1 register
#define MAC_UNI_ADDR        0x3c  // 0x0F0 6 registers 0x3c - 0x41
#define MAC_CORE_CNTRL      0x7f  // 0x1fC

/* TX Control Register 1*/

#define TX_CNTRL1_TX_EN         BIT(0)
#define TX_CNTRL1_DUPLEX        BIT(1)
#define TX_CNTRL1_RETRY         BIT(2)
#define TX_CNTRL1_PAD_EN        BIT(3)
#define TX_CNTRL1_FCS_EN        BIT(4)
#define TX_CNTRL1_2DEFER        BIT(5)
#define TX_CNTRL1_RMII          BIT(6)

/* TX Control Register 2 */
#define TX_CNTRL2_RETRIES_MASK  0xf

/* RX Control Register 1 */
#define RX_CNTRL1_RX_EN         BIT(0)
#define RX_CNTRL1_PADSTRIP_EN   BIT(1)
#define RX_CNTRL1_CRC_EN        BIT(2)
#define RX_CNTRL1_PAUSE_EN      BIT(3)
#define RX_CNTRL1_LOOP_EN       BIT(4)
#define RX_CNTRL1_ADDR_FLTR_EN  BIT(5)
#define RX_CNTRL1_RX_RUNT_EN    BIT(6)
#define RX_CNTRL1_BCAST_DIS     BIT(7)

/* RX Control Register 2 */
#define RX_CNTRL2_DEFER_EN      BIT(0)

/* Core Control Register */
#define CORE_RESET              BIT(0)
#define CORE_RX_FIFO_FLUSH      BIT(1)
#define CORE_TX_FIFO_FLUSH      BIT(2)
#define CORE_SEND_JAM           BIT(3)
#define CORE_MDC_EN             BIT(4)

/* Definitions for MII access routines*/

#define MII_REG_SHL    16
#define MII_ADDR_SHL   21

#define MII_GO                  BIT(31)
#define MII_WRITE               BIT(26)
#define MII_READ_FAIL           BIT(31)

#define MII_TIMEOUT_10TH_SECS        5
#define MII_10TH_SEC_IN_MILLIS     100

/*
 *
 * Default values
 *
 */

#define MAC_DEF_MSG_ENABLE (NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK)

#define MAC_TX_CNTRL1_DEFAULT  (\
		TX_CNTRL1_TX_EN | \
		TX_CNTRL1_RETRY  | \
		TX_CNTRL1_FCS_EN | \
		TX_CNTRL1_2DEFER | \
		TX_CNTRL1_PAD_EN )

#define MAC_TX_MAX_RETRIES_DEFAULT 0x0f

#define MAC_RX_CNTRL1_DEFAULT ( \
		RX_CNTRL1_PADSTRIP_EN | \
		RX_CNTRL1_CRC_EN | \
		RX_CNTRL1_RX_EN )

#define MAC_RX_CNTRL2_DEFAULT       0x0
#define MAC_TX_CNTRL2_DEFAULT       TX_CNTRL2_RETRIES_MASK

/* Thresholds determined by NPE firmware FS */
#define MAC_THRESH_P_EMPTY_DEFAULT  0x12
#define MAC_THRESH_P_FULL_DEFAULT   0x30

/* Number of bytes that must be in the tx fifo before
 * transmission commences */
#define MAC_BUF_SIZE_TX_DEFAULT     0x8

/* One-part deferral values */
#define MAC_TX_DEFER_DEFAULT        0x15
#define MAC_RX_DEFER_DEFAULT        0x16

/* Two-part deferral values... */
#define MAC_TX_TWO_DEFER_1_DEFAULT  0x08
#define MAC_TX_TWO_DEFER_2_DEFAULT  0x07

/* This value applies to MII */
#define MAC_SLOT_TIME_DEFAULT       0x80

/* This value applies to RMII */
#define MAC_SLOT_TIME_RMII_DEFAULT  0xFF

#define MAC_ADDR_MASK_DEFAULT       0xFF

#define MAC_INT_CLK_THRESH_DEFAULT  0x1
/* The following is a value chosen at random */
#define MAC_RANDOM_SEED_DEFAULT     0x8

/* By default we must configure the MAC to generate the MDC clock*/
#define CORE_DEFAULT                (CORE_MDC_EN)

/* End of Intel provided register information */

extern int
mdio_read_register(struct net_device *dev, int phy_addr, int phy_reg);
extern void
mdio_write_register(struct net_device *dev, int phy_addr, int phy_reg, int val);
extern void init_mdio(struct net_device *dev, int phy_id,int kendin);

struct mac_info {
	u32 __iomem *addr;
	struct resource *res;
	struct device *npe_dev;
	struct net_device *netdev;
	struct qm_qmgr *qmgr;
	struct qm_queue *rxq;
	struct qm_queue *txq;
	struct qm_queue *rxdoneq;
	u32 irqflags;
	struct net_device_stats stat;
	struct mii_if_info mii;
	struct delayed_work mdio_thread;
	int rxq_pkt;
	int txq_pkt;
	int unloading;
	struct mac_plat_info *plat;
	int npe_stat_num;
	spinlock_t rx_lock;
	u32 msg_enable;
	int iskendin;
};

static inline void mac_write_reg(struct mac_info *mac, int offset, u32 val)
{
	*(mac->addr + offset) = val;
}
static inline u32 mac_read_reg(struct mac_info *mac, int offset)
{
	return *(mac->addr + offset);
}
static inline void mac_set_regbit(struct mac_info *mac, int offset, u32 bit)
{
	mac_write_reg(mac, offset, mac_read_reg(mac, offset) | bit);
}
static inline void mac_reset_regbit(struct mac_info *mac, int offset, u32 bit)
{
	mac_write_reg(mac, offset, mac_read_reg(mac, offset) & ~bit);
}

static inline void mac_mdio_cmd_write(struct mac_info *mac, u32 cmd)
{
	int i;
	for(i=0; i<4; i++) {
		mac_write_reg(mac, MAC_MDIO_CMD + i, cmd & 0xff);
		cmd >>=8;
	}
}

#define mac_mdio_cmd_read(mac) mac_mdio_read((mac), MAC_MDIO_CMD)
#define mac_mdio_status_read(mac) mac_mdio_read((mac), MAC_MDIO_STS)
static inline u32 mac_mdio_read(struct mac_info *mac, int offset)
{
	int i;
	u32 data = 0;
	for(i=0; i<4; i++) {
		data |= (mac_read_reg(mac, offset + i) & 0xff) << (i*8);
	}
	return data;
}

static inline u32 mdio_cmd(int phy_addr, int phy_reg)
{
	return phy_addr << MII_ADDR_SHL |
		phy_reg << MII_REG_SHL |
	        MII_GO;
}

#define MAC_REG_LIST { \
	MAC_TX_CNTRL1, MAC_TX_CNTRL2, \
	MAC_RX_CNTRL1, MAC_RX_CNTRL2, \
	MAC_RANDOM_SEED, MAC_THRESH_P_EMPTY, MAC_THRESH_P_FULL, \
	MAC_BUF_SIZE_TX, MAC_TX_DEFER, MAC_RX_DEFER, \
	MAC_TX_TWO_DEFER_1, MAC_TX_TWO_DEFER_2, MAC_SLOT_TIME, \
	MAC_ADDR_MASK +0, MAC_ADDR_MASK +1, MAC_ADDR_MASK +2, \
	MAC_ADDR_MASK +3, MAC_ADDR_MASK +4, MAC_ADDR_MASK +5, \
	MAC_ADDR +0, MAC_ADDR +1, MAC_ADDR +2, \
	MAC_ADDR +3, MAC_ADDR +4, MAC_ADDR +5, \
	MAC_INT_CLK_THRESH, \
	MAC_UNI_ADDR +0, MAC_UNI_ADDR +1, MAC_UNI_ADDR +2, \
	MAC_UNI_ADDR +3, MAC_UNI_ADDR +4, MAC_UNI_ADDR +5, \
	MAC_CORE_CNTRL \
}

#define NPE_STAT_NUM            34
#define NPE_STAT_NUM_BASE       22
#define NPE_Q_STAT_NUM           4

#define NPE_Q_STAT_STRINGS \
	{"RX ready to use queue len     "}, \
	{"RX received queue len         "}, \
	{"TX to be send queue len       "}, \
	{"TX done queue len             "},

#define NPE_STAT_STRINGS \
	{"StatsAlignmentErrors          "}, \
	{"StatsFCSErrors                "}, \
	{"StatsInternalMacReceiveErrors "}, \
	{"RxOverrunDiscards             "}, \
	{"RxLearnedEntryDiscards        "}, \
	{"RxLargeFramesDiscards         "}, \
	{"RxSTPBlockedDiscards          "}, \
	{"RxVLANTypeFilterDiscards      "}, \
	{"RxVLANIdFilterDiscards        "}, \
	{"RxInvalidSourceDiscards       "}, \
	{"RxBlackListDiscards           "}, \
	{"RxWhiteListDiscards           "}, \
	{"RxUnderflowEntryDiscards      "}, \
	{"StatsSingleCollisionFrames    "}, \
	{"StatsMultipleCollisionFrames  "}, \
	{"StatsDeferredTransmissions    "}, \
	{"StatsLateCollisions           "}, \
	{"StatsExcessiveCollsions       "}, \
	{"StatsInternalMacTransmitErrors"}, \
	{"StatsCarrierSenseErrors       "}, \
	{"TxLargeFrameDiscards          "}, \
	{"TxVLANIdFilterDiscards        "}, \
\
	{"RxValidFramesTotalOctets      "}, \
	{"RxUcastPkts                   "}, \
	{"RxBcastPkts                   "}, \
	{"RxMcastPkts                   "}, \
	{"RxPkts64Octets                "}, \
	{"RxPkts65to127Octets           "}, \
	{"RxPkts128to255Octets          "}, \
	{"RxPkts256to511Octets          "}, \
	{"RxPkts512to1023Octets         "}, \
	{"RxPkts1024to1518Octets        "}, \
	{"RxInternalNPEReceiveErrors    "}, \
	{"TxInternalNPETransmitErrors   "}

