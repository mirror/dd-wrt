#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/completion.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/semaphore.h>
#include <asm/arch-sl2312/irqs.h>
#include <asm/arch/it8712.h>
#include <asm/arch/sl2312.h>
#include <linux/mtd/kvctl.h>
#include <linux/sysctl_storlink.h>

#define BIG_ENDIAN  	0

#define GMAC_DEBUG      0

#define GMAC_PHY_IF     2

/* define PHY address */
#define HPHY_ADDR   0x01
#define GPHY_ADDR   0x02

#define CONFIG_ADM_6999 1
/* define chip information */
#define DRV_NAME			"SL2312"
#define DRV_VERSION			"0.1.1"
#define SL2312_DRIVER_NAME  DRV_NAME " Fast Ethernet driver " DRV_VERSION

/* define TX/RX descriptor parameter */
#define MAX_ETH_FRAME_SIZE	1920
#define TX_BUF_SIZE			MAX_ETH_FRAME_SIZE
#define TX_DESC_NUM			128
#define TX_BUF_TOT_LEN		(TX_BUF_SIZE * TX_DESC_NUM)
#define RX_BUF_SIZE			MAX_ETH_FRAME_SIZE
#define RX_DESC_NUM			256
#define RX_BUF_TOT_LEN		(RX_BUF_SIZE * RX_DESC_NUM)
#define MAX_ISR_WORK        20

unsigned int int_status = 0;

/* define GMAC base address */
#define GMAC_PHYSICAL_BASE_ADDR	    (SL2312_GMAC_BASE)
#define GMAC_BASE_ADDR			    (IO_ADDRESS(GMAC_PHYSICAL_BASE_ADDR))
#define GMAC_GLOBAL_BASE_ADDR       (IO_ADDRESS(SL2312_GLOBAL_BASE))

#define GMAC0_BASE                  (IO_ADDRESS(SL2312_GMAC0_BASE))
#define GMAC1_BASE                  (IO_ADDRESS(SL2312_GMAC1_BASE))

/* memory management utility */
#define DMA_MALLOC(size,handle)		pci_alloc_consistent(NULL,size,handle)
#define DMA_MFREE(mem,size,handle)	pci_free_consistent(NULL,size,mem,handle)

//#define gmac_read_reg(offset)       (readl(GMAC_BASE_ADDR + offset))
//#define gmac_write_reg(offset,data,mask)  writel( (gmac_read_reg(offset)&~mask) |(data&mask),(GMAC_BASE_ADDR+offset))

/* define owner bit */
#define CPU		0
#define DMA		1

#define ACTIVE      1
#define NONACTIVE   0

#define CONFIG_SL_NAPI

#ifndef CONFIG_SL2312_MPAGE
#define CONFIG_SL2312_MPAGE
#endif

#ifdef CONFIG_SL2312_MPAGE
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#endif

#ifndef CONFIG_TXINT_DISABLE
//#define CONFIG_TXINT_DISABLE
#endif

enum phy_state
{
    LINK_DOWN   = 0,
    LINK_UP     = 1
};


/* transmit timeout value */
#define TX_TIMEOUT  (6*HZ)

/***************************************/
/* the offset address of GMAC register */
/***************************************/
enum GMAC_REGISTER {
	GMAC_STA_ADD0 	= 0x0000,
	GMAC_STA_ADD1	= 0x0004,
	GMAC_STA_ADD2	= 0x0008,
	GMAC_RX_FLTR	= 0x000c,
	GMAC_MCAST_FIL0 = 0x0010,
	GMAC_MCAST_FIL1 = 0x0014,
	GMAC_CONFIG0	= 0x0018,
	GMAC_CONFIG1	= 0x001c,
	GMAC_CONFIG2	= 0x0020,
	GMAC_BNCR		= 0x0024,
	GMAC_RBNR		= 0x0028,
	GMAC_STATUS		= 0x002c,
	GMAC_IN_DISCARDS= 0x0030,
	GMAC_IN_ERRORS  = 0x0034,
	GMAC_IN_MCAST   = 0x0038,
	GMAC_IN_BCAST   = 0x003c,
	GMAC_IN_MAC1    = 0x0040,
	GMAC_IN_MAC2    = 0x0044
};

/*******************************************/
/* the offset address of GMAC DMA register */
/*******************************************/
enum GMAC_DMA_REGISTER {
	GMAC_DMA_DEVICE_ID		= 0xff00,
	GMAC_DMA_STATUS			= 0xff04,
	GMAC_TXDMA_CTRL 	 	= 0xff08,
	GMAC_TXDMA_FIRST_DESC 	= 0xff0c,
	GMAC_TXDMA_CURR_DESC	= 0xff10,
	GMAC_RXDMA_CTRL			= 0xff14,
	GMAC_RXDMA_FIRST_DESC	= 0xff18,
	GMAC_RXDMA_CURR_DESC	= 0xff1c,
};

/*******************************************/
/* the register structure of GMAC          */
/*******************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_0004
	{
#if (BIG_ENDIAN==1)
		unsigned int sta_add2_l16	: 16;	/* station MAC address2 bits 15 to 0 */
		unsigned int sta_add1_h16	: 16;	/* station MAC address1 bits 47 to 32 */
#else
		unsigned int sta_add1_h16	: 16;	/* station MAC address1 bits 47 to 32 */
		unsigned int sta_add2_l16	: 16;	/* station MAC address2 bits 15 to 0 */
#endif
	} bits;
} GMAC_STA_ADD1_T;

typedef union
{
	unsigned int bits32;
	struct bit1_000c
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 27;
		unsigned int error			:  1;	/* enable receive of all error frames */
		unsigned int promiscuous	:  1;   /* enable receive of all frames */
		unsigned int broadcast		:  1;	/* enable receive of broadcast frames */
		unsigned int multicast		:  1;	/* enable receive of multicast frames that pass multicast filter */
		unsigned int unicast		:  1;	/* enable receive of unicast frames that are sent to STA address */
#else
		unsigned int unicast		:  1;	/* enable receive of unicast frames that are sent to STA address */
		unsigned int multicast		:  1;	/* enable receive of multicast frames that pass multicast filter */
		unsigned int broadcast		:  1;	/* enable receive of broadcast frames */
		unsigned int promiscuous	:  1;   /* enable receive of all frames */
		unsigned int error			:  1;	/* enable receive of all error frames */
		unsigned int 				: 27;
#endif
	} bits;
} GMAC_RX_FLTR_T;

typedef union
{
	unsigned int bits32;
	struct bit1_0018
	{
#if (BIG_ENDIAN==1)
		unsigned int : 10;
		unsigned int inv_rx_clk     : 1;	/* Inverse RX Clock */
		unsigned int rising_latch   : 1;
        unsigned int rx_tag_remove  :  1;   /* Remove Rx VLAN tag */
        unsigned int ipv6_tss_rx_en :  1;   /* IPv6 TSS RX enable */
        unsigned int ipv4_tss_rx_en :  1;   /* IPv4 TSS RX enable */
        unsigned int rgmii_en       :  1;   /* RGMII in-band status enable */
		unsigned int tx_fc_en		:  1;	/* TX flow control enable */
		unsigned int rx_fc_en		:  1;	/* RX flow control enable */
		unsigned int sim_test		:  1;	/* speed up timers in simulation */
		unsigned int dis_col		:  1;	/* disable 16 collisions abort function */
		unsigned int dis_bkoff		:  1;	/* disable back-off function */
		unsigned int max_len		:  3;	/* maximum receive frame length allowed */
		unsigned int adj_ifg		:  4;	/* adjust IFG from 96+/-56 */
        unsigned int                :  1;   /* reserved */
		unsigned int loop_back		:  1;	/* transmit data loopback enable */
		unsigned int dis_rx			:  1;	/* disable receive */
		unsigned int dis_tx			:  1;	/* disable transmit */
#else
		unsigned int dis_tx			:  1;	/* disable transmit */
		unsigned int dis_rx			:  1;	/* disable receive */
		unsigned int loop_back		:  1;	/* transmit data loopback enable */
        unsigned int                :  1;   /* reserved */
		unsigned int adj_ifg		:  4;	/* adjust IFG from 96+/-56 */
		unsigned int max_len		:  3;	/* maximum receive frame length allowed */
		unsigned int dis_bkoff		:  1;	/* disable back-off function */
		unsigned int dis_col		:  1;	/* disable 16 collisions abort function */
		unsigned int sim_test		:  1;	/* speed up timers in simulation */
		unsigned int rx_fc_en		:  1;	/* RX flow control enable */
		unsigned int tx_fc_en		:  1;	/* TX flow control enable */
        unsigned int rgmii_en       :  1;   /* RGMII in-band status enable */
        unsigned int ipv4_tss_rx_en :  1;   /* IPv4 TSS RX enable */
        unsigned int ipv6_tss_rx_en :  1;   /* IPv6 TSS RX enable */
        unsigned int rx_tag_remove  :  1;   /* Remove Rx VLAN tag */
		unsigned int rising_latch   :  1;
		unsigned int inv_rx_clk : 1;	/* Inverse RX Clock */
		unsigned int : 10;
#endif
	} bits;
} GMAC_CONFIG0_T;

typedef union
{
	unsigned int bits32;
	struct bit1_001c
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 28;
		unsigned int buf_size		:  4; 	/* per packet buffer size */
#else
		unsigned int buf_size		:  4; 	/* per packet buffer size */
		unsigned int 				: 28;
#endif
	} bits;
} GMAC_CONFIG1_T;

typedef union
{
	unsigned int bits32;
	struct bit1_0020
	{
#if (BIG_ENDIAN==1)
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
#else
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
#endif
	} bits;
} GMAC_CONFIG2_T;

typedef union
{
	unsigned int bits32;
	struct bit1_0024
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 16;
		unsigned int buf_num		: 16; 	/* return buffer number from software */
#else
		unsigned int buf_num		: 16; 	/* return buffer number from software */
		unsigned int 				: 16;
#endif
	} bits;
} GMAC_BNCR_T;

typedef union
{
	unsigned int bits32;
	struct bit1_0028
	{
#if (BIG_ENDIAN==1)
		unsigned int				: 16;
		unsigned int buf_remain		: 16;	/* remaining buffer number */
#else
		unsigned int buf_remain		: 16;	/* remaining buffer number */
		unsigned int				: 16;
#endif
	} bits;
} GMAC_RBNR_T;

typedef union
{
	unsigned int bits32;
	struct bit1_002c
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 25;
		unsigned int mii_rmii		:  2;   /* PHY interface type */
		unsigned int phy_mode		:  1;	/* PHY interface mode in 10M-bps */
		unsigned int duplex			:  1;	/* duplex mode */
		unsigned int speed			:  2;	/* link speed(00->2.5M 01->25M 10->125M) */
		unsigned int link			:  1;	/* link status */
#else
		unsigned int link			:  1;	/* link status */
		unsigned int speed			:  2;	/* link speed(00->2.5M 01->25M 10->125M) */
		unsigned int duplex			:  1;	/* duplex mode */
		unsigned int phy_mode		:  1;	/* PHY interface mode in 10M-bps */
		unsigned int mii_rmii		:  2;   /* PHY interface type */
		unsigned int 				: 25;
#endif
	} bits;
} GMAC_STATUS_T;


typedef union
{
	unsigned int bits32;
	struct bit1_009
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 10;
		unsigned int tx_fail		:  1; 	/* Tx fail interrupt */
		unsigned int cnt_full		:  1;	/* MIB counters half full interrupt */
		unsigned int rx_pause_on	:  1;	/* received pause on frame interrupt */
		unsigned int tx_pause_on	:  1;	/* transmit pause on frame interrupt */
		unsigned int rx_pause_off   :  1;	/* received pause off frame interrupt */
		unsigned int tx_pause_off	:  1;	/* received pause off frame interrupt */
		unsigned int rx_overrun		:  1;   /* GMAC Rx FIFO overrun interrupt */
		unsigned int tx_underrun	:  1;	/* GMAC Tx FIFO underrun interrupt */
		unsigned int				:  6;
		unsigned int m_tx_fail 		:  1;	/* Tx fail interrupt mask */
		unsigned int m_cnt_full		:  1;	/* MIB counters half full interrupt mask */
		unsigned int m_rx_pause_on	:  1;	/* received pause on frame interrupt mask */
		unsigned int m_tx_pause_on  :  1;	/* transmit pause on frame interrupt mask */
		unsigned int m_rx_pause_off :  1;	/* received pause off frame interrupt mask */
		unsigned int m_tx_pause_off	:  1;	/* received pause off frame interrupt mask */
		unsigned int m_rx_overrun	:  1;   /* GMAC Rx FIFO overrun interrupt mask */
		unsigned int m_tx_underrun	:  1;	/* GMAC Tx FIFO underrun interrupt mask */
#else
		unsigned int m_tx_underrun	:  1;	/* GMAC Tx FIFO underrun interrupt mask */
		unsigned int m_rx_overrun	:  1;   /* GMAC Rx FIFO overrun interrupt mask */
		unsigned int m_tx_pause_off	:  1;	/* received pause off frame interrupt mask */
		unsigned int m_rx_pause_off :  1;	/* received pause off frame interrupt mask */
		unsigned int m_tx_pause_on  :  1;	/* transmit pause on frame interrupt mask */
		unsigned int m_rx_pause_on	:  1;	/* received pause on frame interrupt mask */
		unsigned int m_cnt_full		:  1;	/* MIB counters half full interrupt mask */
		unsigned int m_tx_fail 		:  1;	/* Tx fail interrupt mask */
		unsigned int				:  6;
		unsigned int tx_underrun	:  1;	/* GMAC Tx FIFO underrun interrupt */
		unsigned int rx_overrun		:  1;   /* GMAC Rx FIFO overrun interrupt */
		unsigned int tx_pause_off	:  1;	/* received pause off frame interrupt */
		unsigned int rx_pause_off   :  1;	/* received pause off frame interrupt */
		unsigned int tx_pause_on	:  1;	/* transmit pause on frame interrupt */
		unsigned int rx_pause_on	:  1;	/* received pause on frame interrupt */
		unsigned int cnt_full		:  1;	/* MIB counters half full interrupt */
		unsigned int tx_fail		:  1; 	/* Tx fail interrupt */
		unsigned int 				: 10;
#endif
	} bits;
} GMAC_INT_MASK_T;


/*******************************************/
/* the register structure of GMAC DMA      */
/*******************************************/
typedef union
{
	unsigned int bits32;
	struct bit2_ff00
	{
#if (BIG_ENDIAN==1)
		unsigned int                :  7;   /* reserved */
		unsigned int s_ahb_err		:  1;	/* Slave AHB bus error */
		unsigned int tx_err_code    :  4;   /* TxDMA error code */
		unsigned int rx_err_code  	:  4;   /* RxDMA error code */
		unsigned int device_id		: 12;
		unsigned int revision_id	:  4;
#else
		unsigned int revision_id	:  4;
		unsigned int device_id		: 12;
		unsigned int rx_err_code  	:  4;   /* RxDMA error code */
		unsigned int tx_err_code    :  4;   /* TxDMA error code */
		unsigned int s_ahb_err		:  1;	/* Slave AHB bus error */
		unsigned int                :  7;   /* reserved */
#endif
	} bits;
} GMAC_DMA_DEVICE_ID_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff04
	{
#if (BIG_ENDIAN==1)
		unsigned int ts_finish		:  1;	/* finished tx interrupt */
		unsigned int ts_derr		:  1;   /* AHB Bus Error while tx */
		unsigned int ts_perr		:  1;   /* Tx Descriptor protocol error */
		unsigned int ts_eodi		:  1;	/* TxDMA end of descriptor interrupt */
		unsigned int ts_eofi		:  1;   /* TxDMA end of frame interrupt */
		unsigned int rs_finish		:  1;   /* finished rx interrupt */
		unsigned int rs_derr		:  1;   /* AHB Bus Error while rx */
		unsigned int rs_perr		:  1;   /* Rx Descriptor protocol error */
		unsigned int rs_eodi		:  1;	/* RxDMA end of descriptor interrupt */
		unsigned int rs_eofi		:  1;	/* RxDMA end of frame interrupt */
		unsigned int        		:  1; 	/* Tx fail interrupt */
		unsigned int cnt_full		:  1;	/* MIB counters half full interrupt */
		unsigned int rx_pause_on	:  1;	/* received pause on frame interrupt */
		unsigned int tx_pause_on	:  1;	/* transmit pause on frame interrupt */
		unsigned int rx_pause_off   :  1;	/* received pause off frame interrupt */
		unsigned int tx_pause_off	:  1;	/* received pause off frame interrupt */
		unsigned int rx_overrun		:  1;   /* GMAC Rx FIFO overrun interrupt */
		unsigned int link_change	:  1;	/* GMAC link changed Interrupt for RGMII mode */
		unsigned int        		:  1;
		unsigned int            	:  1;
		unsigned int 				:  3;
		unsigned int loop_back		:  1;	/* loopback TxDMA to RxDMA */
		unsigned int        		:  1;	/* Tx fail interrupt mask */
		unsigned int m_cnt_full		:  1;	/* MIB counters half full interrupt mask */
		unsigned int m_rx_pause_on	:  1;	/* received pause on frame interrupt mask */
		unsigned int m_tx_pause_on  :  1;	/* transmit pause on frame interrupt mask */
		unsigned int m_rx_pause_off :  1;	/* received pause off frame interrupt mask */
		unsigned int m_tx_pause_off	:  1;	/* received pause off frame interrupt mask */
		unsigned int m_rx_overrun	:  1;   /* GMAC Rx FIFO overrun interrupt mask */
		unsigned int m_link_change	:  1;	/* GMAC link changed Interrupt mask for RGMII mode */
#else
		unsigned int m_link_change	:  1;	/* GMAC link changed Interrupt mask for RGMII mode */
		unsigned int m_rx_overrun	:  1;   /* GMAC Rx FIFO overrun interrupt mask */
		unsigned int m_tx_pause_off	:  1;	/* received pause off frame interrupt mask */
		unsigned int m_rx_pause_off :  1;	/* received pause off frame interrupt mask */
		unsigned int m_tx_pause_on  :  1;	/* transmit pause on frame interrupt mask */
		unsigned int m_rx_pause_on	:  1;	/* received pause on frame interrupt mask */
		unsigned int m_cnt_full		:  1;	/* MIB counters half full interrupt mask */
		unsigned int         		:  1;	/* Tx fail interrupt mask */
		unsigned int loop_back		:  1;	/* loopback TxDMA to RxDMA */
		unsigned int 				:  3;
		unsigned int            	:  1;
		unsigned int        		:  1;
		unsigned int link_change	:  1;	/* GMAC link changed Interrupt for RGMII mode */
		unsigned int rx_overrun		:  1;   /* GMAC Rx FIFO overrun interrupt */
		unsigned int tx_pause_off	:  1;	/* received pause off frame interrupt */
		unsigned int rx_pause_off   :  1;	/* received pause off frame interrupt */
		unsigned int tx_pause_on	:  1;	/* transmit pause on frame interrupt */
		unsigned int rx_pause_on	:  1;	/* received pause on frame interrupt */
		unsigned int cnt_full		:  1;	/* MIB counters half full interrupt */
		unsigned int        		:  1; 	/* Tx fail interrupt */
		unsigned int rs_eofi		:  1;	/* RxDMA end of frame interrupt */
		unsigned int rs_eodi		:  1;	/* RxDMA end of descriptor interrupt */
		unsigned int rs_perr		:  1;   /* Rx Descriptor protocol error */
		unsigned int rs_derr		:  1;   /* AHB Bus Error while rx */
		unsigned int rs_finish		:  1;   /* finished rx interrupt */
		unsigned int ts_eofi		:  1;   /* TxDMA end of frame interrupt */
		unsigned int ts_eodi		:  1;	/* TxDMA end of descriptor interrupt */
		unsigned int ts_perr		:  1;   /* Tx Descriptor protocol error */
		unsigned int ts_derr		:  1;   /* AHB Bus Error while tx */
		unsigned int ts_finish		:  1;	/* finished tx interrupt */
#endif
	} bits;
} GMAC_DMA_STATUS_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff08
	{
#if (BIG_ENDIAN==1)
		unsigned int td_start		:  1;	/* Start DMA transfer */
		unsigned int td_continue	:  1;   /* Continue DMA operation */
		unsigned int td_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int 				:  1;
		unsigned int td_prot		:  4;	/* TxDMA protection control */
		unsigned int td_burst_size  :  2;	/* TxDMA max burst size for every AHB request */
		unsigned int td_bus		    :  2;	/* peripheral bus width;0x->8 bits,10->16 bits,11->32 bits */
		unsigned int td_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int td_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int td_fail_en 	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int td_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int td_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int td_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int 				: 14;
#else
		unsigned int 				: 14;
		unsigned int td_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int td_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int td_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int td_fail_en 	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int td_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int td_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int td_bus		    :  2;	/* peripheral bus width;0x->8 bits,10->16 bits,11->32 bits */
		unsigned int td_burst_size  :  2;	/* TxDMA max burst size for every AHB request */
		unsigned int td_prot		:  4;	/* TxDMA protection control */
		unsigned int 				:  1;
		unsigned int td_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int td_continue	:  1;   /* Continue DMA operation */
		unsigned int td_start		:  1;	/* Start DMA transfer */
#endif
	} bits;
} GMAC_TXDMA_CTRL_T;


typedef union
{
	unsigned int bits32;
	struct bit2_ff0c
	{
#if (BIG_ENDIAN==1)
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int 					:  3;
#else
		unsigned int 					:  3;
		unsigned int td_busy			:  1;/* 1-TxDMA busy; 0-TxDMA idle */
		unsigned int td_first_des_ptr	: 28;/* first descriptor address */
#endif
	} bits;
} GMAC_TXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff10
	{
#if (BIG_ENDIAN==1)
		unsigned int ndar			: 28;	/* next descriptor address */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int    			:  1;
		unsigned int sof_eof		:  2;
#else
		unsigned int sof_eof		:  2;
		unsigned int    			:  1;
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int ndar			: 28;	/* next descriptor address */
#endif
	} bits;
} GMAC_TXDMA_CURR_DESC_T;


typedef union
{
	unsigned int bits32;
	struct bit2_ff14
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_start		:  1;	/* Start DMA transfer */
		unsigned int rd_continue	:  1;   /* Continue DMA operation */
		unsigned int rd_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int 				:  1;
		unsigned int rd_prot		:  4;	/* DMA protection control */
		unsigned int rd_burst_size  :  2;	/* DMA max burst size for every AHB request */
		unsigned int rd_bus		    :  2;	/* peripheral bus width;0x->8 bits,10->16 bits,11->32 bits */
		unsigned int rd_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int rd_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int rd_fail_en  	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int rd_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int rd_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int rd_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int 				: 14;
#else
		unsigned int 				: 14;
		unsigned int rd_eof_en      :  1;   /* End of frame interrupt Enable;1-enable;0-mask */
		unsigned int rd_eod_en  	:  1;	/* End of Descriptor interrupt Enable;1-enable;0-mask */
		unsigned int rd_perr_en 	:  1;	/* Protocol Failure Interrupt Enable;1-enable;0-mask */
		unsigned int rd_fail_en  	:  1;	/* DMA Fail Interrupt Enable;1-enable;0-mask */
		unsigned int rd_finish_en   :  1;	/* DMA Finish Event Interrupt Enable;1-enable;0-mask */
		unsigned int rd_endian		:  1;	/* AHB Endian. 0-little endian; 1-big endian */
		unsigned int rd_bus		    :  2;	/* peripheral bus width;0x->8 bits,10->16 bits,11->32 bits */
		unsigned int rd_burst_size  :  2;	/* DMA max burst size for every AHB request */
		unsigned int rd_prot		:  4;	/* DMA protection control */
		unsigned int 				:  1;
		unsigned int rd_chain_mode	:  1;	/* Descriptor Chain Mode;1-Descriptor Chain mode, 0-Direct DMA mode*/
		unsigned int rd_continue	:  1;   /* Continue DMA operation */
		unsigned int rd_start		:  1;	/* Start DMA transfer */
#endif
	} bits;
} GMAC_RXDMA_CTRL_T;


typedef union
{
	unsigned int bits32;
	struct bit2_ff18
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int 					:  3;
#else
		unsigned int 					:  3;
		unsigned int rd_busy			:  1;/* 1-RxDMA busy; 0-RxDMA idle */
		unsigned int rd_first_des_ptr	: 28;/* first descriptor address */
#endif
	} bits;
} GMAC_RXDMA_FIRST_DESC_T;

typedef union
{
	unsigned int bits32;
	struct bit2_ff1c
	{
#if (BIG_ENDIAN==1)
		unsigned int ndar			: 28;	/* next descriptor address */
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int    			:  1;
		unsigned int sof_eof		:  2;
#else
		unsigned int sof_eof		:  2;
		unsigned int    			:  1;
		unsigned int eofie			:  1;	/* end of frame interrupt enable */
		unsigned int ndar			: 28;	/* next descriptor address */
#endif
	} bits;
} GMAC_RXDMA_CURR_DESC_T;


/********************************************/
/*          Descriptor Format               */
/********************************************/

typedef struct descriptor_t
{
	union frame_control_t
	{
		unsigned int bits32;
		struct bits_0000
		{
#if (BIG_ENDIAN==1)
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int csum_state : 3;	/* checksum error status */
			unsigned int vlan_tag   : 1;    /* 802.1q vlan tag packet */
			unsigned int frame_state: 3;    /* reference Rx Status1 */
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
#else
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int frame_state: 3;    /* reference Rx Status1 */
			unsigned int vlan_tag   : 1;    /* 802.1q vlan tag packet */
			unsigned int csum_state : 3;	/* checksum error status */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
#endif
		} bits_rx;

		struct bits_0001
		{
#if (BIG_ENDIAN==1)
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int            : 6;
			unsigned int success_tx : 1;    /* successful transmitted */
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
#else
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int success_tx : 1;    /* successful transmitted */
			unsigned int            : 6;
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
#endif
        } bits_tx_in;

		struct bits_0002
		{
#if (BIG_ENDIAN==1)
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int            : 2;
			unsigned int udp_csum_en: 1;    /* TSS UDP checksum enable */
			unsigned int tcp_csum_en: 1;    /* TSS TCP checksum enable */
			unsigned int ipv6_tx_en : 1;    /* TSS IPv6 TX enable */
			unsigned int ip_csum_en : 1;    /* TSS IPv4 IP Header checksum enable */
			unsigned int vlan_enable: 1;    /* VLAN TIC insertion enable */
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
#else
			unsigned int buffer_size:16;	/* transfer buffer size associated with current description*/
			unsigned int desc_count : 6;	/* number of descriptors used for the current frame */
			unsigned int vlan_enable: 1;    /* VLAN TIC insertion enable */
			unsigned int ip_csum_en : 1;    /* TSS IPv4 IP Header checksum enable */
			unsigned int ipv6_tx_en : 1;    /* TSS IPv6 TX enable */
			unsigned int tcp_csum_en: 1;    /* TSS TCP checksum enable */
			unsigned int udp_csum_en: 1;    /* TSS UDP checksum enable */
			unsigned int            : 2;
			unsigned int perr		: 1;	/* protocol error during processing this descriptor */
			unsigned int derr		: 1;	/* data error during processing this descriptor */
			unsigned int own 		: 1;	/* owner bit. 0-CPU, 1-DMA */
#endif
        } bits_tx_out;

	} frame_ctrl;

	union flag_status_t
	{
		unsigned int bits32;
		struct bits_0004
		{
#if (BIG_ENDIAN==1)
            unsigned int priority   : 3;    /* user priority extracted from receiving frame*/
            unsigned int cfi        : 1;	/* cfi extracted from receiving frame*/
			unsigned int vlan_id    :12;	/* VLAN ID extracted from receiving frame */
			unsigned int frame_count:16;	/* received frame byte count,include CRC,not include VLAN TIC */
#else
			unsigned int frame_count:16;	/* received frame byte count,include CRC,not include VLAN TIC */
			unsigned int vlan_id    :12;	/* VLAN ID extracted from receiving frame */
            unsigned int cfi        : 1;	/* cfi extracted from receiving frame*/
            unsigned int priority   : 3;    /* user priority extracted from receiving frame*/
#endif
		} bits_rx_status;

		struct bits_0005
		{
#if (BIG_ENDIAN==1)
            unsigned int priority   : 3;    /* user priority to transmit*/
            unsigned int cfi        : 1;	/* cfi to transmit*/
			unsigned int vlan_id    :12;	/* VLAN ID to transmit */
			unsigned int frame_count:16;    /* total tx frame byte count */
#else
			unsigned int frame_count:16;    /* total tx frame byte count */
			unsigned int vlan_id    :12;	/* VLAN ID to transmit */
            unsigned int cfi        : 1;	/* cfi to transmit*/
            unsigned int priority   : 3;    /* user priority to transmit*/
#endif
		} bits_tx_flag;
	} flag_status;

	unsigned int buf_adr;	/* data buffer address */

	union next_desc_t
	{
		unsigned int next_descriptor;
		struct bits_000c
		{
#if (BIG_ENDIAN==1)
			unsigned int ndar		:28;	/* next descriptor address */
			unsigned int eofie		: 1;	/* end of frame interrupt enable */
			unsigned int    		: 1;
			unsigned int sof_eof	: 2;	/* 00-the linking descriptor   01-the last descriptor of a frame*/
			                                /* 10-the first descriptor of a frame    11-only one descriptor for a frame*/
#else
			unsigned int sof_eof	: 2;	/* 00-the linking descriptor   01-the last descriptor of a frame*/
			                                /* 10-the first descriptor of a frame    11-only one descriptor for a frame*/
			unsigned int    		: 1;
			unsigned int eofie		: 1;	/* end of frame interrupt enable */
			unsigned int ndar		:28;	/* next descriptor address */
#endif
		} bits;
	} next_desc;
} GMAC_DESCRIPTOR_T;

typedef struct gmac_conf {
	struct net_device *dev;
	int portmap;
	int vid;
	int flag;     /* 1: active  0: non-active */
} sys_gmac_conf;

struct gmac_private {
	unsigned char       *tx_bufs;	/* Tx bounce buffer region. */
	unsigned char       *rx_bufs;
	GMAC_DESCRIPTOR_T	*tx_desc;	/* point to virtual TX descriptor address*/
	GMAC_DESCRIPTOR_T	*rx_desc;	/* point to virtual RX descriptor address*/
	GMAC_DESCRIPTOR_T	*tx_cur_desc;	/* point to current TX descriptor */
	GMAC_DESCRIPTOR_T	*rx_cur_desc;	/* point to current RX descriptor */
	GMAC_DESCRIPTOR_T   *tx_finished_desc;
	GMAC_DESCRIPTOR_T   *rx_finished_desc;
	unsigned long       cur_tx;
	unsigned int        cur_rx;	/* Index into the Rx buffer of next Rx pkt. */
	unsigned int        tx_flag;
	unsigned long       dirty_tx;
	unsigned char       *tx_buf[TX_DESC_NUM];	/* Tx bounce buffers */
	dma_addr_t          tx_desc_dma; /* physical TX descriptor address */
	dma_addr_t          rx_desc_dma;	/* physical RX descriptor address */
	dma_addr_t          tx_bufs_dma; /* physical TX descriptor address */
	dma_addr_t          rx_bufs_dma; /* physical RX descriptor address */
    struct net_device_stats  stats;
	pid_t               thr_pid;
	wait_queue_head_t   thr_wait;
	struct completion   thr_exited;
    spinlock_t          lock;
    int                 time_to_die;
	unsigned int		tx_desc_hdr[GMAC_PHY_IF];	/* the descriptor which sw can fill */
	unsigned int		tx_desc_tail[GMAC_PHY_IF];	/* the descriptor which is not cleaned yet */
};


struct reg_ioctl_data {
    unsigned int    reg_addr;   /* the register address */
    unsigned int    val_in;     /* data write to the register */
    unsigned int    val_out;    /* data read from the register */
};

#ifdef CONFIG_SL2312_MPAGE
typedef struct tx_data_t {
	int	freeable; // 1 when it's skb. it can be freed in tx interrupt handler
	struct sk_buff*	skb; // skb
	int	desc_in_use; // 1 when the desc is in use. 0 when desc is available.
	long end_seq; // to find out packets are in seq.
	// so this value is the seq of next packet.
} tx_data;
#endif

/*************************************************************
 *         Global Variable
 *************************************************************/
struct semaphore        sem_gmac;   /* semaphore for share pins issue */

/*************************************************************
 *        Static Global Variable
 *************************************************************/
// static unsigned int     MAC_BASE_ADDR = GMAC0_BASE;
static unsigned int     gmac_base_addr[GMAC_PHY_IF] = {GMAC0_BASE,GMAC1_BASE};
static unsigned int     gmac_irq[GMAC_PHY_IF] = {IRQ_GMAC0,IRQ_GMAC1};
static struct net_device *gmac_dev[GMAC_PHY_IF];

static unsigned int     FLAG_SWITCH=0;	/* if 1-->switch chip presented. if 0-->switch chip unpresented */
static unsigned int     flow_control_enable[GMAC_PHY_IF] = {1,1};
static unsigned int     pre_phy_status[GMAC_PHY_IF] = {LINK_DOWN,LINK_DOWN};
static unsigned int     tx_desc_virtual_base[GMAC_PHY_IF];
static unsigned int     rx_desc_virtual_base[GMAC_PHY_IF];
static unsigned int     full_duplex = 1;
static unsigned int     speed = 1;
#ifdef CONFIG_SL2312_MPAGE
static tx_data		    tx_skb[GMAC_PHY_IF][TX_DESC_NUM];
#else
static struct sk_buff   *tx_skb[GMAC_PHY_IF][TX_DESC_NUM];
#endif
static struct sk_buff   *rx_skb[GMAC_PHY_IF][RX_DESC_NUM];
static unsigned int     tx_desc_start_adr[GMAC_PHY_IF];
static unsigned int     rx_desc_start_adr[GMAC_PHY_IF];
static unsigned char    eth0_mac[6]= {0x00,0x50,0xc2,0x2b,0xd3,0x25};
static unsigned char    eth1_mac[6]= {0x00,0x50,0xc2,0x2b,0xdf,0xfe};
static unsigned int     next_tick = 3 * HZ;

static unsigned int     phy_addr[GMAC_PHY_IF] = {0x01,0x02};  /* define PHY address */

DECLARE_WAIT_QUEUE_HEAD(gmac_queue);
//static 	wait_queue_t    wait;

struct gmac_conf VLAN_conf[] = {
#ifdef CONFIG_ADM_6999
	{ (struct net_device *)0,0x7F,1 },
	{ (struct net_device *)0,0x80,2 }
#endif
#ifdef CONFIG_ADM_6996
	{ (struct net_device *)0,0x0F,1 },
	{ (struct net_device *)0,0x10,2 }
#endif
};

#define NUM_VLAN_IF	(sizeof(VLAN_conf)/sizeof(struct gmac_conf))


/************************************************/
/*            GMAC function declare             */
/************************************************/

unsigned int mii_read(unsigned char phyad,unsigned char regad);
void mii_write(unsigned char phyad,unsigned char regad,unsigned int value);
static void gmac_set_phy_status(struct net_device *dev);
static void gmac_get_phy_status(struct net_device *dev);
static int gmac_phy_thread (void *data);
static int gmac_set_mac_address(struct net_device *dev, void *addr);
static void gmac_tx_timeout(struct net_device *dev);
static void gmac_tx_packet_complete(struct net_device *dev);
static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev);
static void gmac_set_rx_mode(struct net_device *dev);
static void gmac_rx_packet(struct net_device *dev);
static int gmac_open (struct net_device *dev);
static int gmac_netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);

static unsigned int gmac_get_dev_index(struct net_device *dev);
static unsigned int gmac_select_interface(struct net_device *dev);

#ifdef CONFIG_SL2312_MPAGE
int printk_all(int dev_index, struct gmac_private* tp);
#endif

/****************************************/
/*	SPI Function Declare		*/
/****************************************/
void SPI_write(unsigned char addr,unsigned int value);
unsigned int SPI_read(unsigned char table,unsigned char addr);
void SPI_write_bit(char bit_EEDO);
unsigned int SPI_read_bit(void);
void SPI_default(void);
void SPI_reset(unsigned char rstype,unsigned char port_cnt);
void SPI_pre_st(void);
void SPI_CS_enable(unsigned char enable);
void SPI_Set_VLAN(unsigned char LAN,unsigned int port_mask);
void SPI_Set_tag(unsigned int port,unsigned tag);
void SPI_Set_PVID(unsigned int PVID,unsigned int port_mask);
unsigned int SPI_Get_PVID(unsigned int port);
void SPI_mac_lock(unsigned int port, unsigned char lock);
void SPI_get_port_state(unsigned int port);
void SPI_port_enable(unsigned int port,unsigned char enable);
unsigned int SPI_get_identifier(void);
void SPI_get_status(unsigned int port);

/****************************************/
/*	VLAN Function Declare		        */
/****************************************/
int getVLANfromdev (struct net_device *dev );
struct net_device * getdevfromVLAN( int VID);



/************************************************/
/*                 function body                */
/************************************************/
#if 0
void hw_memcpy(void *to,const void *from,unsigned long n)
{
    writel(from,SL2312_DRAM_CTRL_BASE+0x20);  /* set source address */
    writel(to,SL2312_DRAM_CTRL_BASE+0x24);    /* set destination address */
    writel(n,SL2312_DRAM_CTRL_BASE+0x28);     /* set byte count */
    writel(0x00000001,SL2312_DRAM_CTRL_BASE+0x2c);
    while (readl(SL2312_DRAM_CTRL_BASE+0x2c));
}
#endif

static unsigned int gmac_read_reg(unsigned int addr)
{
    unsigned int    reg_val;
//    unsigned int    flags;
//    spinlock_t     lock;

//    spin_lock_irqsave(&lock, flags);
    reg_val = readl(addr);	// Gary Chen
//	spin_unlock_irqrestore(&lock, flags);
	return (reg_val);
}

static void gmac_write_reg(unsigned int addr,unsigned int data,unsigned int bit_mask)
{
	unsigned int    reg_val;
    //unsigned int    *addr;
//    unsigned int    flags;
//    spinlock_t     lock;

//    spin_lock_irqsave(&lock, flags);
	reg_val = ( gmac_read_reg(addr) & (~bit_mask) ) | (data & bit_mask);
    writel(reg_val,addr);
//	spin_unlock_irqrestore(&lock, flags);
	return;
}


static void gmac_sw_reset(struct net_device *dev)
{
    unsigned int    index;
    unsigned int    reg_val;

    index = gmac_get_dev_index(dev);
    if (index==0)
        reg_val = readl(GMAC_GLOBAL_BASE_ADDR+0x0c) | 0x00000020;   /* GMAC0 S/W reset */
    else
        reg_val = readl(GMAC_GLOBAL_BASE_ADDR+0x0c) | 0x00000040;   /* GMAC1 S/W reset */

    writel(reg_val,GMAC_GLOBAL_BASE_ADDR+0x0c);
    return;
}

static void gmac_get_mac_address(void)
{
#ifdef CONFIG_MTD
	extern int get_vlaninfo(vlaninfo* vlan);
    static vlaninfo    vlan[2];

    if (get_vlaninfo(&vlan[0]))
    {
        memcpy(eth0_mac,vlan[0].mac,6);
        VLAN_conf[0].vid = vlan[0].vlanid;
        VLAN_conf[0].portmap = vlan[0].vlanmap;
        memcpy(eth1_mac,vlan[1].mac,6);
        VLAN_conf[1].vid = vlan[1].vlanid;
        VLAN_conf[1].portmap = vlan[1].vlanmap;
    }
#else
    unsigned int reg_val;

    reg_val = readl(IO_ADDRESS(SL2312_SECURITY_BASE)+0xac);
    eth0_mac[4] = (reg_val & 0xff00) >> 8;
    eth0_mac[5] = reg_val & 0x00ff;
    reg_val = readl(IO_ADDRESS(SL2312_SECURITY_BASE)+0xac);
    eth1_mac[4] = (reg_val & 0xff00) >> 8;
    eth1_mac[5] = reg_val & 0x00ff;
#endif
    return;
}

static unsigned int gmac_get_dev_index(struct net_device *dev)
{
    unsigned int    i;

    /* get device index number */
    for (i=0;i<GMAC_PHY_IF;i++)
    {
        if (gmac_dev[i]==dev)
        {
            return(i);
        }
    }
    return (0xff);
}

static unsigned int gmac_select_interface(struct net_device *dev)
{
    unsigned int    index;

    index = gmac_get_dev_index(dev);
    // MAC_BASE_ADDR = gmac_base_addr[index];	// Gary Chen
    return (index);
}


static void gmac_dump_register(struct net_device *dev)
{
#if 0
    unsigned int   i,val,index;

    index = gmac_select_interface(dev);

    printk("========== GMAC%d ==========\n",index);
    for (i=0;i<=0x7c;i=i+4)
    {
        val = gmac_read_reg(gmac_base_addr[index] + i);
        printk("offset = %08x   value = %08x\n",i,val);
    }
    for (i=0xff00;i<=0xff7c;i=i+4)
    {
        val = gmac_read_reg(gmac_base_addr[index] + i);
        printk("offset = %08x   value = %08x\n",i,val);
    }
#endif
}

static int gmac_init_chip(struct net_device *dev)
{
	GMAC_RBNR_T		rbnr_val,rbnr_mask;
	GMAC_CONFIG2_T	config2_val;
	GMAC_CONFIG0_T	config0,config0_mask;
	GMAC_CONFIG1_T	config1;
	struct sockaddr sock;
	unsigned int    status;
	unsigned int    phy_mode;
	unsigned int    index;

    index = gmac_get_dev_index(dev);

    /* set GMAC RMII mode */
    if (index==0)
        phy_mode = 0;   /* 0->MII 1->GMII 2->RGMII(10/100) 3->RGMII(1000) */
    else
        phy_mode = 2;   /* 0->MII 1->GMII 2->RGMII(10/100) 3->RGMII(1000) */

    /* set PHY operation mode */
    status = (phy_mode<<5) | 0x11 | (full_duplex<<3) | (speed<<1);
    gmac_write_reg(gmac_base_addr[index] + GMAC_STATUS,status ,0x0000007f);

	/* set station MAC address1 and address2 */
	if (index==0)
	    memcpy(&sock.sa_data[0],&eth0_mac[0],6);
    else
	    memcpy(&sock.sa_data[0],&eth1_mac[0],6);
    gmac_set_mac_address(dev,(void *)&sock);

    /* set RX_FLTR register to receive all multicast packet */
    gmac_write_reg(gmac_base_addr[index] + GMAC_RX_FLTR,0x0000001F,0x0000001f);
    //gmac_write_reg(gmac_base_addr[index] + GMAC_RX_FLTR,0x00000007,0x0000001f);

	/* set per packet buffer size */
	config1.bits32 = 0;
    config1.bits.buf_size = 11; /* buffer size = 2048-byte */
    gmac_write_reg(gmac_base_addr[index] + GMAC_CONFIG1,config1.bits32,0x0000000f);

	/* set flow control threshold */
	config2_val.bits32 = 0;
	config2_val.bits.set_threshold = RX_DESC_NUM/4;
	config2_val.bits.rel_threshold = RX_DESC_NUM*3/4;
	gmac_write_reg(gmac_base_addr[index] + GMAC_CONFIG2,config2_val.bits32,0xffffffff);

	/* init remaining buffer number register */
	rbnr_val.bits32 = 0;
	rbnr_val.bits.buf_remain = RX_DESC_NUM;
	rbnr_mask.bits32 = 0;
	rbnr_mask.bits.buf_remain = 0xffff;
	gmac_write_reg(gmac_base_addr[index] + GMAC_RBNR,rbnr_val.bits32,rbnr_mask.bits32);

    /* disable TX/RX and disable internal loop back */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.max_len = 2;
    if (flow_control_enable[index]==1)
    {
        config0.bits.tx_fc_en = 1; /* enable tx flow control */
        config0.bits.rx_fc_en = 1; /* enable rx flow control */
        printk("Enable MAC Flow Control...\n");
    }
    else
    {
        config0.bits.tx_fc_en = 0; /* disable tx flow control */
        config0.bits.rx_fc_en = 0; /* disable rx flow control */
        printk("Disable MAC Flow Control...\n");
    }
    config0.bits.dis_rx = 1;  /* disable rx */
    config0.bits.dis_tx = 1;  /* disable tx */
    config0.bits.loop_back = 0; /* enable/disable GMAC loopback */
	config0.bits.inv_rx_clk = 0;
	config0.bits.rising_latch = 1;
	config0.bits.ipv4_tss_rx_en = 1;  /* enable H/W to check ip checksum */
	config0.bits.ipv6_tss_rx_en = 1;  /* enable H/W to check ip checksum */

    config0_mask.bits.max_len = 7;
    config0_mask.bits.tx_fc_en = 1;
    config0_mask.bits.rx_fc_en = 1;
    config0_mask.bits.dis_rx = 1;
    config0_mask.bits.dis_tx = 1;
    config0_mask.bits.loop_back = 1;
    config0_mask.bits.inv_rx_clk = 1;
	config0_mask.bits.rising_latch = 1;
	config0_mask.bits.ipv4_tss_rx_en = 1;
	config0_mask.bits.ipv6_tss_rx_en = 1;
    gmac_write_reg(gmac_base_addr[index] + GMAC_CONFIG0,config0.bits32,config0_mask.bits32);

	return (0);
}

static void gmac_enable_tx_rx(struct net_device *dev)
{
	GMAC_CONFIG0_T	config0,config0_mask;
	int				dev_index;

    dev_index = gmac_select_interface(dev);

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 0;  /* enable rx */
    config0.bits.dis_tx = 0;  /* enable tx */
    config0_mask.bits.dis_rx = 1;
    config0_mask.bits.dis_tx = 1;
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
}

static void gmac_disable_tx_rx(struct net_device *dev)
{
	GMAC_CONFIG0_T	config0,config0_mask;
	int				dev_index;

    dev_index = gmac_select_interface(dev);

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 1;  /* disable rx */
    config0.bits.dis_tx = 1;  /* disable tx */
    config0_mask.bits.dis_rx = 1;
    config0_mask.bits.dis_tx = 1;
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
}

#ifdef CONFIG_SL_NAPI
static int gmac_rx_poll_ga(struct net_device *dev, int *budget)
{
	struct gmac_private *tp = dev->priv;
	struct sk_buff 		*skb;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	GMAC_RXDMA_FIRST_DESC_T	rxdma_busy;
    GMAC_DESCRIPTOR_T   *rx_desc;
	unsigned int 		pkt_size;
	unsigned int        desc_count;
    unsigned int        vid;
//    unsigned int        priority;
	unsigned int        own;
	unsigned int        good_frame = 0;
	unsigned int        index;
	unsigned int        dev_index;
	int                 work = 0;
	int                 work_done = 0;
	int                 quota = min(dev->quota, *budget);

    dev_index = gmac_select_interface(dev);

	for (;;)
	{
        own = tp->rx_cur_desc->frame_ctrl.bits32 >> 31;
        if (own == CPU) /* check owner bit */
        {
	        rx_desc = tp->rx_cur_desc;
#if (GMAC_DEBUG==1)
	        /* check error interrupt */
	        if ( (rx_desc->frame_ctrl.bits_rx.derr==1)||(rx_desc->frame_ctrl.bits_rx.perr==1) )
	        {
    	        printk("%s::Rx Descriptor Processing Error !!!\n",__func__);
    	    }
#endif
    	    /* get frame information from the first descriptor of the frame */
	        pkt_size = rx_desc->flag_status.bits_rx_status.frame_count - 4;  /*total byte count in a frame*/
#if (GMAC_DEBUG==1)
            priority = rx_desc->flag_status.bits_rx_status.priority;    /* 802.1p priority */
#endif
            vid = rx_desc->flag_status.bits_rx_status.vlan_id;          /* 802.1q vlan id */
            if (vid == 0)
            {
                vid = 1;    /* default vlan */
            }
	        desc_count = rx_desc->frame_ctrl.bits_rx.desc_count; /* get descriptor count per frame */

    		if (rx_desc->frame_ctrl.bits_rx.frame_state == 0x000) /* good frame */
    		{
    			tp->stats.rx_bytes += pkt_size;
    			tp->stats.rx_packets++;
    			good_frame = 1;
    		}
    		else
    		{
    			tp->stats.rx_errors++;
    			good_frame = 0;
    			printk("RX status: 0x%x\n",rx_desc->frame_ctrl.bits_rx.frame_state);
    		}
    	}
    	else
    	{
    	    work_done = 1;
    	    break;  /* Rx process is completed */
    	}

        if (good_frame == 1)
        {
            /* get rx skb buffer index */
            index = ((unsigned int)tp->rx_cur_desc - rx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
            if (rx_skb[dev_index][index])
            {
                skb_reserve (rx_skb[dev_index][index], 2);	/* 16 byte align the IP fields. */
                rx_skb[dev_index][index]->dev = dev;
                rx_skb[dev_index][index]->ip_summed = CHECKSUM_UNNECESSARY;
 			    skb_put(rx_skb[dev_index][index],pkt_size);
 			    rx_skb[dev_index][index]->protocol = eth_type_trans(rx_skb[dev_index][index],dev); /* set skb protocol */
 			    netif_rx(rx_skb[dev_index][index]);  /* socket rx */
 			    dev->last_rx = jiffies;

 			    /* allocate rx skb buffer */
                if ( (skb = dev_alloc_skb(RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
                {
                    printk("%s::skb buffer allocation fail !\n",__func__);
                }
                rx_skb[dev_index][index] = skb;
                tp->rx_cur_desc->buf_adr = (unsigned int)__pa(skb->data) | 0x02;    /* insert two bytes in the beginning of rx data */
            }
            else
            {
                printk("%s::rx skb index error !\n",__func__);
            }
        }

	    tp->rx_cur_desc->frame_ctrl.bits_rx.own = DMA; /* release rx descriptor to DMA */
        /* point to next rx descriptor */
        tp->rx_cur_desc = (GMAC_DESCRIPTOR_T *)((tp->rx_cur_desc->next_desc.next_descriptor & 0xfffffff0)+rx_desc_virtual_base[dev_index]);

        /* release buffer to Remaining Buffer Number Register */
        if (flow_control_enable[dev_index] ==1)
        {
//            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_BNCR,desc_count,0x0000ffff);
            writel(desc_count,(unsigned int *)(gmac_base_addr[dev_index] + GMAC_BNCR));
        }

		if (work++ >= quota )
		{
			break;
		}
    }

    /* if RX DMA process is stoped , restart it */
	rxdma_busy.bits.rd_first_des_ptr = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_FIRST_DESC);
	if (rxdma_busy.bits.rd_busy == 0)
	{
	    rxdma_ctrl.bits32 = 0;
    	rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
	    rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
	    rxdma_ctrl_mask.bits32 = 0;
    	rxdma_ctrl_mask.bits.rd_start = 1;
	    rxdma_ctrl_mask.bits.rd_continue = 1;
	    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
    }

	dev->quota -= work;
	*budget -= work;
	if (work_done==1)
	{
	    /* Receive descriptor is empty now */
        netif_rx_complete(dev);
        /* enable receive interrupt */
        gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,0x0007c000,0x0007c000);   /* enable rx interrupt */
        return 0;
    }
    else
    {
        return -1;
    }
}

static int gmac_rx_poll_gb(struct net_device *dev, int *budget)
{
	struct gmac_private *tp = dev->priv;
	struct sk_buff 		*skb;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	GMAC_RXDMA_FIRST_DESC_T	rxdma_busy;
    GMAC_DESCRIPTOR_T   *rx_desc;
	unsigned int 		pkt_size;
	unsigned int        desc_count;
    unsigned int        vid;
//    unsigned int        priority;
	unsigned int        own;
	unsigned int        good_frame = 0;
	unsigned int        index;
	unsigned int        dev_index;
	int                 work = 0;
	int                 work_done = 0;
	int                 quota = min(dev->quota, *budget);

    dev_index = gmac_select_interface(dev);

	for (;;)
	{
        own = tp->rx_cur_desc->frame_ctrl.bits32 >> 31;
        if (own == CPU) /* check owner bit */
        {
	        rx_desc = tp->rx_cur_desc;
#if (GMAC_DEBUG==1)
	        /* check error interrupt */
	        if ( (rx_desc->frame_ctrl.bits_rx.derr==1)||(rx_desc->frame_ctrl.bits_rx.perr==1) )
	        {
    	        printk("%s::Rx Descriptor Processing Error !!!\n",__func__);
    	    }
#endif
    	    /* get frame information from the first descriptor of the frame */
	        pkt_size = rx_desc->flag_status.bits_rx_status.frame_count - 4;  /*total byte count in a frame*/
#if (GMAC_DEBUG==1)
            priority = rx_desc->flag_status.bits_rx_status.priority;    /* 802.1p priority */
#endif
            vid = rx_desc->flag_status.bits_rx_status.vlan_id;          /* 802.1q vlan id */
            if (vid == 0)
            {
                vid = 1;    /* default vlan */
            }
	        desc_count = rx_desc->frame_ctrl.bits_rx.desc_count; /* get descriptor count per frame */

    		if (rx_desc->frame_ctrl.bits_rx.frame_state == 0x000) /* good frame */
    		{
    			tp->stats.rx_bytes += pkt_size;
    			tp->stats.rx_packets++;
    			good_frame = 1;
    		}
    		else
    		{
    			tp->stats.rx_errors++;
    			good_frame = 0;
    			printk("RX status: 0x%x\n",rx_desc->frame_ctrl.bits_rx.frame_state);
    		}
    	}
    	else
    	{
    	    work_done = 1;
    	    break;  /* Rx process is completed */
    	}

        if (good_frame == 1)
        {
            /* get rx skb buffer index */
            index = ((unsigned int)tp->rx_cur_desc - rx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
            if (rx_skb[dev_index][index])
            {
                skb_reserve (rx_skb[dev_index][index], 2);	/* 16 byte align the IP fields. */
                rx_skb[dev_index][index]->dev = dev;
                rx_skb[dev_index][index]->ip_summed = CHECKSUM_UNNECESSARY;
 			    skb_put(rx_skb[dev_index][index],pkt_size);
 			    rx_skb[dev_index][index]->protocol = eth_type_trans(rx_skb[dev_index][index],dev); /* set skb protocol */
 			    netif_rx(rx_skb[dev_index][index]);  /* socket rx */
 			    dev->last_rx = jiffies;

 			    /* allocate rx skb buffer */
                if ( (skb = dev_alloc_skb(RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
                {
                    printk("%s::skb buffer allocation fail !\n",__func__);
                }
                rx_skb[dev_index][index] = skb;
                tp->rx_cur_desc->buf_adr = (unsigned int)__pa(skb->data) | 0x02;    /* insert two bytes in the beginning of rx data */
            }
            else
            {
                printk("%s::rx skb index error !\n",__func__);
            }
        }

	    tp->rx_cur_desc->frame_ctrl.bits_rx.own = DMA; /* release rx descriptor to DMA */
        /* point to next rx descriptor */
        tp->rx_cur_desc = (GMAC_DESCRIPTOR_T *)((tp->rx_cur_desc->next_desc.next_descriptor & 0xfffffff0)+rx_desc_virtual_base[dev_index]);

        /* release buffer to Remaining Buffer Number Register */
        if (flow_control_enable[dev_index] ==1)
        {
//            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_BNCR,desc_count,0x0000ffff);
            writel(desc_count,(unsigned int *)(gmac_base_addr[dev_index] + GMAC_BNCR));
        }

		if (work++ >= quota )
		{
			break;
		}
    }

    /* if RX DMA process is stoped , restart it */
	rxdma_busy.bits.rd_first_des_ptr = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_FIRST_DESC);
	if (rxdma_busy.bits.rd_busy == 0)
	{
	    rxdma_ctrl.bits32 = 0;
    	rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
	    rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
	    rxdma_ctrl_mask.bits32 = 0;
    	rxdma_ctrl_mask.bits.rd_start = 1;
	    rxdma_ctrl_mask.bits.rd_continue = 1;
	    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
    }

	dev->quota -= work;
	*budget -= work;
	if (work_done==1)
	{
	    /* Receive descriptor is empty now */
        netif_rx_complete(dev);
        /* enable receive interrupt */
        gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,0x0007c000,0x0007c000);   /* enable rx interrupt */
        return 0;
    }
    else
    {
        return -1;
    }
}

#endif

static void gmac_rx_packet(struct net_device *dev)
{
	struct gmac_private *tp = dev->priv;
	struct sk_buff 		*skb;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	GMAC_RXDMA_FIRST_DESC_T	rxdma_busy;
    GMAC_DESCRIPTOR_T   *rx_desc;
	unsigned int 		pkt_size;
	unsigned int        desc_count;
    unsigned int        vid;
//    unsigned int        priority;
	unsigned int        own;
	unsigned int        good_frame = 0;
	unsigned int        i,index;
	unsigned int        dev_index;

    dev_index = gmac_select_interface(dev);

	for (i=0;i<256;i++)
	{
        own = tp->rx_cur_desc->frame_ctrl.bits32 >> 31;
        if (own == CPU) /* check owner bit */
        {
	        rx_desc = tp->rx_cur_desc;
#if (GMAC_DEBUG==1)
	        /* check error interrupt */
	        if ( (rx_desc->frame_ctrl.bits_rx.derr==1)||(rx_desc->frame_ctrl.bits_rx.perr==1) )
	        {
    	        printk("%s::Rx Descriptor Processing Error !!!\n",__func__);
    	    }
#endif
    	    /* get frame information from the first descriptor of the frame */
	        pkt_size = rx_desc->flag_status.bits_rx_status.frame_count - 4;  /*total byte count in a frame*/
#if (GMAC_DEBUG==1)
            priority = rx_desc->flag_status.bits_rx_status.priority;    /* 802.1p priority */
#endif
            vid = rx_desc->flag_status.bits_rx_status.vlan_id;          /* 802.1q vlan id */
            if (vid == 0)
            {
                vid = 1;    /* default vlan */
            }
	        desc_count = rx_desc->frame_ctrl.bits_rx.desc_count; /* get descriptor count per frame */

    		if (rx_desc->frame_ctrl.bits_rx.frame_state == 0x000) /* good frame */
    		{
    			tp->stats.rx_bytes += pkt_size;
    			tp->stats.rx_packets++;
    			good_frame = 1;
    		}
    		else
    		{
    			tp->stats.rx_errors++;
    			good_frame = 0;
    			printk("RX status: 0x%x\n",rx_desc->frame_ctrl.bits_rx.frame_state);
    		}
    	}
    	else
    	{
    	    break;  /* Rx process is completed */
    	}

        if (good_frame == 1)
        {
            /* get rx skb buffer index */
            index = ((unsigned int)tp->rx_cur_desc - rx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
            if (rx_skb[dev_index][index])
            {
                skb_reserve (rx_skb[dev_index][index], 2);	/* 16 byte align the IP fields. */
                rx_skb[dev_index][index]->dev = dev;
                rx_skb[dev_index][index]->ip_summed = CHECKSUM_UNNECESSARY;
 			    skb_put(rx_skb[dev_index][index],pkt_size);
 			    rx_skb[dev_index][index]->protocol = eth_type_trans(rx_skb[dev_index][index],dev); /* set skb protocol */
 			    netif_rx(rx_skb[dev_index][index]);  /* socket rx */
 			    dev->last_rx = jiffies;

 			    /* allocate rx skb buffer */
                if ( (skb = dev_alloc_skb(RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
                {
                    printk("%s::skb buffer allocation fail !\n",__func__);
                }
                rx_skb[dev_index][index] = skb;
                tp->rx_cur_desc->buf_adr = (unsigned int)__pa(skb->data) | 0x02;    /* insert two bytes in the beginning of rx data */
            }
            else
            {
                printk("%s::rx skb index error !\n",__func__);
            }
        }

	    tp->rx_cur_desc->frame_ctrl.bits_rx.own = DMA; /* release rx descriptor to DMA */
        /* point to next rx descriptor */
        tp->rx_cur_desc = (GMAC_DESCRIPTOR_T *)((tp->rx_cur_desc->next_desc.next_descriptor & 0xfffffff0)+rx_desc_virtual_base[dev_index]);

        /* release buffer to Remaining Buffer Number Register */
        if (flow_control_enable[dev_index] ==1)
        {
            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_BNCR,desc_count,0x0000ffff);
        }
    }

    /* if RX DMA process is stoped , restart it */
	rxdma_busy.bits.rd_first_des_ptr = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_FIRST_DESC);
	if (rxdma_busy.bits.rd_busy == 0)
	{
	    rxdma_ctrl.bits32 = 0;
    	rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
	    rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
	    rxdma_ctrl_mask.bits32 = 0;
    	rxdma_ctrl_mask.bits.rd_start = 1;
	    rxdma_ctrl_mask.bits.rd_continue = 1;
	    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
    }
}

#ifdef CONFIG_SL2312_MPAGE
static inline void free_tx_buf(int dev_index, int desc_index)
{
	if (tx_skb[dev_index][desc_index].freeable &&
	    tx_skb[dev_index][desc_index].skb) {
		struct sk_buff* skb = tx_skb[dev_index][desc_index].skb;
		//printk("free_skb %x, len %d\n", skb, skb->len);
#ifdef CONFIG_TXINT_DISABLE
		dev_kfree_skb(skb);
#else
		dev_kfree_skb_irq(skb);
#endif
		tx_skb[dev_index][desc_index].skb = 0;
	}
}

#ifdef CONFIG_TXINT_DISABLE
static void gmac_tx_packet_complete(struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
    GMAC_DESCRIPTOR_T	    *tx_hw_complete_desc, *next_desc;
    unsigned int desc_cnt=0;
    unsigned int i,index,dev_index;
    unsigned int tx_current_descriptor = 0;
	// int own_dma = 0;

    dev_index = gmac_select_interface(dev);

	index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
	if (tx_skb[dev_index][index].desc_in_use && tp->tx_finished_desc->frame_ctrl.bits_tx_in.own == CPU) {
		free_tx_buf(dev_index, index);
		tx_skb[dev_index][index].desc_in_use = 0;
	}
	next_desc = (GMAC_DESCRIPTOR_T*)((tp->tx_finished_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);

	for (;;) {
		tx_hw_complete_desc = (GMAC_DESCRIPTOR_T *)((gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC) & 0xfffffff0)+ tx_desc_virtual_base[dev_index]);
		if (next_desc == tx_hw_complete_desc)
			break;
		if (next_desc->frame_ctrl.bits_tx_in.own == CPU) {
			if (next_desc->frame_ctrl.bits_tx_in.success_tx == 1) {
				tp->stats.tx_bytes += next_desc->flag_status.bits_tx_flag.frame_count;
				tp->stats.tx_packets ++;
			} else {
				tp->stats.tx_errors++;
			}
			desc_cnt = next_desc->frame_ctrl.bits_tx_in.desc_count;
			for (i=1; i<desc_cnt; i++) {
				/* get tx skb buffer index */
				index = ((unsigned int)next_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
				next_desc->frame_ctrl.bits_tx_in.own = CPU;
				free_tx_buf(dev_index, index);
				tx_skb[dev_index][index].desc_in_use = 0;
				tp->tx_desc_tail[dev_index] = (tp->tx_desc_tail[dev_index] +1) & (TX_DESC_NUM-1);
				/* release Tx descriptor to CPU */
				next_desc = (GMAC_DESCRIPTOR_T *)((next_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
			}
			/* get tx skb buffer index */
			index = ((unsigned int)next_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
			/* free skb buffer */
			next_desc->frame_ctrl.bits_tx_in.own = CPU;
			free_tx_buf(dev_index, index);
			tx_skb[dev_index][index].desc_in_use = 0;
			tp->tx_desc_tail[dev_index] = (tp->tx_desc_tail[dev_index] +1) & (TX_DESC_NUM-1);
			tp->tx_finished_desc = next_desc;
//			printk("finish tx_desc index %d\n", index);
			next_desc = (GMAC_DESCRIPTOR_T *)((next_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
		}
		else
			break;
	}
	if (netif_queue_stopped(dev))
	{
		netif_wake_queue(dev);
	}

}
#else
static void gmac_tx_packet_complete(struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
	GMAC_DESCRIPTOR_T	    *tx_hw_complete_desc;
	unsigned int desc_cnt=0;
	unsigned int i,index,dev_index;
	unsigned int tx_current_descriptor = 0;
	// int own_dma = 0;

	dev_index = gmac_select_interface(dev);

	index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);

	/* check tx status and accumulate tx statistics */
	for (;;)
	{

        for (i=0;i<1000;i++)
        {
            tx_current_descriptor = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC);
            if ( ((tx_current_descriptor & 0x00000003)==0x00000003) ||  /* only one descriptor */
                 ((tx_current_descriptor & 0x00000003)==0x00000001) )   /* the last descriptor */
            {
                break;
            }
            udelay(1);
        }
        if (i==1000)
        {
//            gmac_dump_register(dev);
//            printk("%s: tx current descriptor = %x \n",__func__,tx_current_descriptor);
//            printk_all(dev_index, tp);
            continue;
        }

	    /* get tx H/W completed descriptor virtual address */
    	tx_hw_complete_desc = (GMAC_DESCRIPTOR_T *)((tx_current_descriptor & 0xfffffff0)+ tx_desc_virtual_base[dev_index]);
//    	tx_hw_complete_desc = (GMAC_DESCRIPTOR_T *)((gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC) & 0xfffffff0)+ tx_desc_virtual_base[dev_index]);
	    if (tp->tx_finished_desc == tx_hw_complete_desc ) // ||
		    //tx_skb[dev_index][index].desc_in_use )   /* complete tx processing */
		{
			break;
		}

        for (;;)
        {
    		if (tp->tx_finished_desc->frame_ctrl.bits_tx_in.own == CPU)
    		{
    #if (GMAC_DEBUG==1)
    			if ( (tp->tx_finished_desc->frame_ctrl.bits_tx_in.derr) ||
    			   (tp->tx_finished_desc->frame_ctrl.bits_tx_in.perr) )
    			{
    				printk("%s::Descriptor Processing Error !!!\n",__func__);
    			}
    #endif
    			if (tp->tx_finished_desc->frame_ctrl.bits_tx_in.success_tx == 1)
    			{
    				tp->stats.tx_bytes += tp->tx_finished_desc->flag_status.bits_tx_flag.frame_count;
    				tp->stats.tx_packets ++;
    			}
    			else
    			{
    				tp->stats.tx_errors++;
    			}
    			desc_cnt = tp->tx_finished_desc->frame_ctrl.bits_tx_in.desc_count;
    			for (i=1; i<desc_cnt; i++)  /* multi-descriptor in one packet */
    			{
    				/* get tx skb buffer index */
    				index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
    				tp->tx_finished_desc->frame_ctrl.bits_tx_in.own = CPU;
    				free_tx_buf(dev_index, index);
    				tx_skb[dev_index][index].desc_in_use = 0;
    				/* release Tx descriptor to CPU */
    				tp->tx_finished_desc = (GMAC_DESCRIPTOR_T *)((tp->tx_finished_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
    			}
    			/* get tx skb buffer index */
    			index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
    			/* free skb buffer */
    			tp->tx_finished_desc->frame_ctrl.bits_tx_in.own = CPU;
    			free_tx_buf(dev_index, index);
    			tx_skb[dev_index][index].desc_in_use = 0;
    			tp->tx_finished_desc = (GMAC_DESCRIPTOR_T *)((tp->tx_finished_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);

        	    if (tp->tx_finished_desc == tx_hw_complete_desc )
        		{
        			break;
        		}
            }
    		else
    		{
    			break;
       		}
		}
	}

	if (netif_queue_stopped(dev))
	{
		netif_wake_queue(dev);
	}

}
#endif
#else

static void gmac_tx_packet_complete(struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
    GMAC_DESCRIPTOR_T	    *tx_hw_complete_desc;
    unsigned int desc_cnt=0;
    unsigned int i,index,dev_index;

    dev_index = gmac_select_interface(dev);

	/* get tx H/W completed descriptor virtual address */
	tx_hw_complete_desc = (GMAC_DESCRIPTOR_T *)((gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC) & 0xfffffff0)+ tx_desc_virtual_base[dev_index]);
	/* check tx status and accumulate tx statistics */
    for (;;)
    {
        if (tp->tx_finished_desc == tx_hw_complete_desc)   /* complete tx processing */
        {
            break;
        }
    	if (tp->tx_finished_desc->frame_ctrl.bits_tx_in.own == CPU)
    	{
#if (GMAC_DEBUG==1)
    	    if ( (tp->tx_finished_desc->frame_ctrl.bits_tx_in.derr) ||
    	         (tp->tx_finished_desc->frame_ctrl.bits_tx_in.perr) )
    	    {
    	        printk("%s::Descriptor Processing Error !!!\n",__func__);
    	    }
#endif
            if (tp->tx_finished_desc->frame_ctrl.bits_tx_in.success_tx == 1)
            {
                tp->stats.tx_bytes += tp->tx_finished_desc->flag_status.bits_tx_flag.frame_count;
                tp->stats.tx_packets ++;
            }
            else
            {
                tp->stats.tx_errors++;
            }
            desc_cnt = tp->tx_finished_desc->frame_ctrl.bits_tx_in.desc_count;
        	for (i=1; i<desc_cnt; i++)  /* multi-descriptor in one packet */
        	{
                /* get tx skb buffer index */
                index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
                /* free skb buffer */
                if (tx_skb[dev_index][index])
                {
		            dev_kfree_skb_irq(tx_skb[dev_index][index]);
		        }
        	    /* release Tx descriptor to CPU */
                tp->tx_finished_desc = (GMAC_DESCRIPTOR_T *)((tp->tx_finished_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
                tp->tx_finished_desc->frame_ctrl.bits_tx_in.own = CPU;
         	}
            /* get tx skb buffer index */
            index = ((unsigned int)tp->tx_finished_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
            /* free skb buffer */
            if (tx_skb[dev_index][index])
            {
	            dev_kfree_skb_irq(tx_skb[dev_index][index]);
	        }
            tp->tx_finished_desc = (GMAC_DESCRIPTOR_T *)((tp->tx_finished_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
     	}
    }

	if (netif_queue_stopped(dev))
	{
	    netif_wake_queue(dev);
	}

}


#endif

#if 0
static void gmac_weird_interrupt(struct net_device *dev)
{
    gmac_dump_register(dev);
}
#endif

/* The interrupt handler does all of the Rx thread work and cleans up
   after the Tx thread. */
static irqreturn_t gmac_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device       *dev = (struct net_device *)dev_instance;
	GMAC_RXDMA_FIRST_DESC_T	rxdma_busy;
//	GMAC_TXDMA_FIRST_DESC_T	txdma_busy;
//    GMAC_TXDMA_CTRL_T       txdma_ctrl,txdma_ctrl_mask;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	GMAC_DMA_STATUS_T	    status;
    unsigned int            i,dev_index;
	int                     handled = 0;

    dev_index = gmac_select_interface(dev);

	handled = 1;

#ifdef CONFIG_SL_NAPI
	disable_irq(gmac_irq[dev_index]);   /* disable GMAC interrupt */

    status.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_DMA_STATUS);	 /* read DMA status */
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_DMA_STATUS,status.bits32,status.bits32);    /* clear DMA status */

    if (status.bits.rx_overrun == 1)
    {
		printk("%s::RX Overrun !!!%d\n",__func__,gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RBNR));
         gmac_dump_register(dev);
        /* if RX DMA process is stoped , restart it */
        rxdma_busy.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_FIRST_DESC) ;
        if (rxdma_busy.bits.rd_busy == 0)
        {
            /* restart Rx DMA process */
	        rxdma_ctrl.bits32 = 0;
	        rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
            rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
            rxdma_ctrl_mask.bits32 = 0;
	        rxdma_ctrl_mask.bits.rd_start = 1;
            rxdma_ctrl_mask.bits.rd_continue = 1;
            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
        }
    }

    /* process rx packet */
   	if (netif_running(dev) && ((status.bits.rs_eofi==1)||(status.bits.rs_finish==1)))
   	{
        if (likely(netif_rx_schedule_prep(dev)))
        {
            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,0,0x0007c000);   /* disable rx interrupt */
            __netif_rx_schedule(dev);
        }
    }
#ifndef CONFIG_TXINT_DISABLE
    /* process tx packet */
	if (netif_running(dev) && ((status.bits.ts_eofi==1)||(status.bits.ts_finish==1)))
	{
		gmac_tx_packet_complete(dev);
	}
#endif

	enable_irq(gmac_irq[dev_index]);    /* enable GMAC interrupt */
    return IRQ_RETVAL(handled);
#endif

   /* disable GMAC interrupt */
	disable_irq(gmac_irq[dev_index]);
    for (i=0;i<MAX_ISR_WORK;i++)
    {
        /* read DMA status */
	    status.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_DMA_STATUS);
int_status = status.bits32;
	    /* clear DMA status */
        gmac_write_reg(gmac_base_addr[dev_index] + GMAC_DMA_STATUS,status.bits32,status.bits32);

        if ((status.bits32 & 0xffffc000)==0)
        {
            break;
        }

	    if (status.bits.rx_overrun == 1)
	    {
			printk("%s::RX Overrun !!!%d\n",__func__,gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RBNR));
           	gmac_dump_register(dev);
            /* if RX DMA process is stoped , restart it */
	        rxdma_busy.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_FIRST_DESC) ;
	        if (rxdma_busy.bits.rd_busy == 0)
	        {
	            /* restart Rx DMA process */
    	        rxdma_ctrl.bits32 = 0;
    	        rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
	            rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
	            rxdma_ctrl_mask.bits32 = 0;
    	        rxdma_ctrl_mask.bits.rd_start = 1;
	            rxdma_ctrl_mask.bits.rd_continue = 1;
	            gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
            }
	    }

        /* receive rx interrupt */
    	if (netif_running(dev) && ((status.bits.rs_eofi==1)||(status.bits.rs_finish==1)))
    	{
    		gmac_rx_packet(dev);
//    		gmac_tx_packet_complete(dev);
        }

        /* receive tx interrupt */
    	// if (netif_running(dev) && (status.bits.ts_finish==1))
#ifndef CONFIG_TXINT_DISABLE
    	if (netif_running(dev) && ((status.bits.ts_eofi==1)||
			   (status.bits.ts_finish==1)))
    	{
    		gmac_tx_packet_complete(dev);
    	}
#endif
    	/* check uncommon events */
/*        if ((status.bits32 & 0x632fc000)!=0)
        {
            printk("%s::DMA Status = %08x \n",__func__,status.bits32);
            gmac_weird_interrupt(dev);
        }
*/
	}

    /* enable GMAC interrupt */
	enable_irq(gmac_irq[dev_index]);
	//printk("gmac_interrupt complete!\n\n");
	return IRQ_RETVAL(handled);
}

static void gmac_hw_start(struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
	GMAC_TXDMA_CURR_DESC_T	tx_desc;
	GMAC_RXDMA_CURR_DESC_T	rx_desc;
    GMAC_TXDMA_CTRL_T       txdma_ctrl,txdma_ctrl_mask;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	GMAC_DMA_STATUS_T       dma_status,dma_status_mask;
	int						dev_index;

    dev_index = gmac_select_interface(dev);

	/* program TxDMA Current Descriptor Address register for first descriptor */
	tx_desc.bits32 = (unsigned int)(tp->tx_desc_dma);
	tx_desc.bits.eofie = 1;
	tx_desc.bits.sof_eof = 0x03;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC,tx_desc.bits32,0xffffffff);
	gmac_write_reg(gmac_base_addr[dev_index] + 0xff2c,tx_desc.bits32,0xffffffff);   /* tx next descriptor address */

	/* program RxDMA Current Descriptor Address register for first descriptor */
	rx_desc.bits32 = (unsigned int)(tp->rx_desc_dma);
	rx_desc.bits.eofie = 1;
	rx_desc.bits.sof_eof = 0x03;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CURR_DESC,rx_desc.bits32,0xffffffff);
	gmac_write_reg(gmac_base_addr[dev_index] + 0xff3c,rx_desc.bits32,0xffffffff);   /* rx next descriptor address */

	/* enable GMAC interrupt & disable loopback */
	dma_status.bits32 = 0;
	dma_status.bits.loop_back = 0;  /* disable DMA loop-back mode */
//	dma_status.bits.m_tx_fail = 1;
	dma_status.bits.m_cnt_full = 1;
	dma_status.bits.m_rx_pause_on = 1;
	dma_status.bits.m_tx_pause_on = 1;
	dma_status.bits.m_rx_pause_off = 1;
	dma_status.bits.m_tx_pause_off = 1;
	dma_status.bits.m_rx_overrun = 1;
	dma_status.bits.m_link_change = 1;
	dma_status_mask.bits32 = 0;
	dma_status_mask.bits.loop_back = 1;
//	dma_status_mask.bits.m_tx_fail = 1;
	dma_status_mask.bits.m_cnt_full = 1;
	dma_status_mask.bits.m_rx_pause_on = 1;
	dma_status_mask.bits.m_tx_pause_on = 1;
	dma_status_mask.bits.m_rx_pause_off = 1;
	dma_status_mask.bits.m_tx_pause_off = 1;
	dma_status_mask.bits.m_rx_overrun = 1;
	dma_status_mask.bits.m_link_change = 1;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_DMA_STATUS,dma_status.bits32,dma_status_mask.bits32);

    /* program tx dma control register */
	txdma_ctrl.bits32 = 0;
	txdma_ctrl.bits.td_start = 0;    /* start TX DMA transfer */
	txdma_ctrl.bits.td_continue = 0; /* continue Tx DMA operation */
	txdma_ctrl.bits.td_chain_mode = 1;  /* chain mode */
	txdma_ctrl.bits.td_prot = 0;
	txdma_ctrl.bits.td_burst_size = 2;  /* DMA burst size for every AHB request */
	txdma_ctrl.bits.td_bus = 2;         /* peripheral bus width */
	txdma_ctrl.bits.td_endian = 0;      /* little endian */
#ifdef CONFIG_TXINT_DISABLE
	txdma_ctrl.bits.td_finish_en = 0;   /* DMA finish event interrupt disable */
#else
	txdma_ctrl.bits.td_finish_en = 1;   /* DMA finish event interrupt enable */
#endif
	txdma_ctrl.bits.td_fail_en = 1;     /* DMA fail interrupt enable */
	txdma_ctrl.bits.td_perr_en = 1;     /* protocol failure interrupt enable */
	txdma_ctrl.bits.td_eod_en = 0;      /* disable Tx End of Descriptor Interrupt */
	//txdma_ctrl.bits.td_eod_en = 0;      /* disable Tx End of Descriptor Interrupt */
#ifdef CONFIG_TXINT_DISABLE
	txdma_ctrl.bits.td_eof_en = 0;      /* end of frame interrupt disable */
#else
	txdma_ctrl.bits.td_eof_en = 1;      /* end of frame interrupt enable */
#endif
	txdma_ctrl_mask.bits32 = 0;
	txdma_ctrl_mask.bits.td_start = 1;
	txdma_ctrl_mask.bits.td_continue = 1;
	txdma_ctrl_mask.bits.td_chain_mode = 1;
	txdma_ctrl_mask.bits.td_prot = 15;
	txdma_ctrl_mask.bits.td_burst_size = 3;
	txdma_ctrl_mask.bits.td_bus = 3;
	txdma_ctrl_mask.bits.td_endian = 1;
	txdma_ctrl_mask.bits.td_finish_en = 1;
	txdma_ctrl_mask.bits.td_fail_en = 1;
	txdma_ctrl_mask.bits.td_perr_en = 1;
	txdma_ctrl_mask.bits.td_eod_en = 1;
	//txdma_ctrl_mask.bits.td_eod_en = 1;
	txdma_ctrl_mask.bits.td_eof_en = 1;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CTRL,txdma_ctrl.bits32,txdma_ctrl_mask.bits32);

    /* program rx dma control register */
	rxdma_ctrl.bits32 = 0;
	rxdma_ctrl.bits.rd_start = 1;    /* start RX DMA transfer */
	rxdma_ctrl.bits.rd_continue = 1; /* continue RX DMA operation */
	rxdma_ctrl.bits.rd_chain_mode = 1;  /* chain mode */
	rxdma_ctrl.bits.rd_prot = 0;
	rxdma_ctrl.bits.rd_burst_size = 2;  /* DMA burst size for every AHB request */
	rxdma_ctrl.bits.rd_bus = 2;         /* peripheral bus width */
	rxdma_ctrl.bits.rd_endian = 0;      /* little endian */
	rxdma_ctrl.bits.rd_finish_en = 1;   /* DMA finish event interrupt enable */
	rxdma_ctrl.bits.rd_fail_en = 1;     /* DMA fail interrupt enable */
	rxdma_ctrl.bits.rd_perr_en = 1;     /* protocol failure interrupt enable */
	rxdma_ctrl.bits.rd_eod_en = 0;      /* disable Rx End of Descriptor Interrupt */
	rxdma_ctrl.bits.rd_eof_en = 1;      /* end of frame interrupt enable */
	rxdma_ctrl_mask.bits32 = 0;
	rxdma_ctrl_mask.bits.rd_start = 1;
	rxdma_ctrl_mask.bits.rd_continue = 1;
	rxdma_ctrl_mask.bits.rd_chain_mode = 1;
	rxdma_ctrl_mask.bits.rd_prot = 15;
	rxdma_ctrl_mask.bits.rd_burst_size = 3;
	rxdma_ctrl_mask.bits.rd_bus = 3;
	rxdma_ctrl_mask.bits.rd_endian = 1;
	rxdma_ctrl_mask.bits.rd_finish_en = 1;
	rxdma_ctrl_mask.bits.rd_fail_en = 1;
	rxdma_ctrl_mask.bits.rd_perr_en = 1;
	rxdma_ctrl_mask.bits.rd_eod_en = 1;
	rxdma_ctrl_mask.bits.rd_eof_en = 1;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
    return;
}

static void gmac_hw_stop(struct net_device *dev)
{
    GMAC_TXDMA_CTRL_T       txdma_ctrl,txdma_ctrl_mask;
    GMAC_RXDMA_CTRL_T       rxdma_ctrl,rxdma_ctrl_mask;
	int 					dev_index;

    dev_index = gmac_select_interface(dev);

    /* program tx dma control register */
	txdma_ctrl.bits32 = 0;
	txdma_ctrl.bits.td_start = 0;
	txdma_ctrl.bits.td_continue = 0;
	txdma_ctrl_mask.bits32 = 0;
	txdma_ctrl_mask.bits.td_start = 1;
	txdma_ctrl_mask.bits.td_continue = 1;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CTRL,txdma_ctrl.bits32,txdma_ctrl_mask.bits32);
    /* program rx dma control register */
	rxdma_ctrl.bits32 = 0;
	rxdma_ctrl.bits.rd_start = 0;    /* stop RX DMA transfer */
	rxdma_ctrl.bits.rd_continue = 0; /* stop continue RX DMA operation */
	rxdma_ctrl_mask.bits32 = 0;
	rxdma_ctrl_mask.bits.rd_start = 1;
	rxdma_ctrl_mask.bits.rd_continue = 1;
	gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RXDMA_CTRL,rxdma_ctrl.bits32,rxdma_ctrl_mask.bits32);
}

static int gmac_init_desc_buf(struct net_device *dev)
{
	struct gmac_private *tp = dev->priv;
	struct sk_buff 		*skb;
	dma_addr_t          tx_first_desc_dma=0;
	dma_addr_t          rx_first_desc_dma=0;
	dma_addr_t          rx_first_buf_dma=0;
	unsigned int        i,index;

    printk("Descriptor buffer init......\n");

    /* get device index number */
    index = gmac_get_dev_index(dev);
#ifdef CONFIG_SL2312_MPAGE
	for (i=0; i<TX_DESC_NUM; i++) {
		tx_skb[index][i].freeable = 0;
		tx_skb[index][i].skb = 0;
		tx_skb[index][i].desc_in_use = 0;
		tx_skb[index][i].end_seq = 0;
	}
#else
    for (i=0;i<TX_DESC_NUM;i++)
    {
        tx_skb[index][i] = NULL;
    }
#endif
    for (i=0;i<RX_DESC_NUM;i++)
    {
        rx_skb[index][i] = NULL;
    }

	/* allocates TX/RX descriptors */
	tp->tx_desc = DMA_MALLOC(TX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),(dma_addr_t *)&tp->tx_desc_dma);
    tx_desc_virtual_base[index] = (unsigned int)tp->tx_desc - (unsigned int)tp->tx_desc_dma;
    memset(tp->tx_desc,0x00,TX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T));
	tp->rx_desc = DMA_MALLOC(RX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),(dma_addr_t *)&tp->rx_desc_dma);
    rx_desc_virtual_base[index] = (unsigned int)tp->rx_desc - (unsigned int)tp->rx_desc_dma;
    memset(tp->rx_desc,0x00,RX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T));
    tx_desc_start_adr[index] = (unsigned int)tp->tx_desc;   /* for tx skb index calculation */
    rx_desc_start_adr[index] = (unsigned int)tp->rx_desc;   /* for rx skb index calculation */
    printk("tx_desc = %08x\n",(unsigned int)tp->tx_desc);
    printk("rx_desc = %08x\n",(unsigned int)tp->rx_desc);
	printk("tx_desc_dma = %08x\n",tp->tx_desc_dma);
	printk("rx_desc_dma = %08x\n",tp->rx_desc_dma);

	if (tp->tx_desc==0x00 || tp->rx_desc==0x00)
	{
		free_irq(dev->irq, dev);

		if (tp->tx_desc)
			DMA_MFREE(tp->tx_desc, TX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),tp->tx_desc_dma);
		if (tp->rx_desc)
			DMA_MFREE(tp->rx_desc, RX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),tp->rx_desc_dma);
		return -ENOMEM;
	}

	/* TX descriptors initial */
	tp->tx_cur_desc = tp->tx_desc;  /* virtual address */
	tp->tx_finished_desc = tp->tx_desc; /* virtual address */
	tx_first_desc_dma = tp->tx_desc_dma; /* physical address */
	for (i = 1; i < TX_DESC_NUM; i++)
	{
		tp->tx_desc->frame_ctrl.bits_tx_out.own = CPU; /* set owner to CPU */
		tp->tx_desc->frame_ctrl.bits_tx_out.buffer_size = TX_BUF_SIZE;  /* set tx buffer size for descriptor */
		tp->tx_desc_dma = tp->tx_desc_dma + sizeof(GMAC_DESCRIPTOR_T); /* next tx descriptor DMA address */
		tp->tx_desc->next_desc.next_descriptor = tp->tx_desc_dma | 0x0000000b;
		tp->tx_desc = &tp->tx_desc[1] ; /* next tx descriptor virtual address */
	}
	/* the last descriptor will point back to first descriptor */
	tp->tx_desc->frame_ctrl.bits_tx_out.own = CPU;
	tp->tx_desc->frame_ctrl.bits_tx_out.buffer_size = TX_BUF_SIZE;
	tp->tx_desc->next_desc.next_descriptor = tx_first_desc_dma | 0x0000000b;
	tp->tx_desc = tp->tx_cur_desc;
	tp->tx_desc_dma = tx_first_desc_dma;

	/* RX descriptors initial */
	tp->rx_cur_desc = tp->rx_desc;  /* virtual address */
	rx_first_desc_dma = tp->rx_desc_dma; /* physical address */
	for (i = 1; i < RX_DESC_NUM; i++)
	{
        if ( (skb = dev_alloc_skb(RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
        {
            printk("%s::skb buffer allocation fail !\n",__func__);
        }
        rx_skb[index][i-1] = skb;
        tp->rx_desc->buf_adr = (unsigned int)__pa(skb->data) | 0x02;    /* insert two bytes in the beginning of rx data */
		tp->rx_desc->frame_ctrl.bits_rx.own = DMA;  /* set owner bit to DMA */
		tp->rx_desc->frame_ctrl.bits_rx.buffer_size = RX_BUF_SIZE; /* set rx buffer size for descriptor */
		tp->rx_bufs_dma = tp->rx_bufs_dma + RX_BUF_SIZE;    /* point to next buffer address */
		tp->rx_desc_dma = tp->rx_desc_dma + sizeof(GMAC_DESCRIPTOR_T); /* next rx descriptor DMA address */
		tp->rx_desc->next_desc.next_descriptor = tp->rx_desc_dma | 0x0000000b;
		tp->rx_desc = &tp->rx_desc[1]; /* next rx descriptor virtual address */
	}
	/* the last descriptor will point back to first descriptor */
    if ( (skb = dev_alloc_skb(RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
    {
        printk("%s::skb buffer allocation fail !\n",__func__);
    }
    rx_skb[index][i-1] = skb;
    tp->rx_desc->buf_adr = (unsigned int)__pa(skb->data) | 0x02;    /* insert two bytes in the beginning of rx data */
	tp->rx_desc->frame_ctrl.bits_rx.own = DMA;
	tp->rx_desc->frame_ctrl.bits_rx.buffer_size = RX_BUF_SIZE;
	tp->rx_desc->next_desc.next_descriptor = rx_first_desc_dma | 0x0000000b;
	tp->rx_desc = tp->rx_cur_desc;
	tp->rx_desc_dma = rx_first_desc_dma;
	tp->rx_bufs_dma = rx_first_buf_dma;

	for (i=0; i<GMAC_PHY_IF; i++) {
		tp->tx_desc_hdr[i] = 0;
		tp->tx_desc_tail[i] = 0;
	}
	return (0);
}

static int gmac_clear_counter (struct net_device *dev)
{
	struct gmac_private *tp = dev->priv;
	unsigned int    dev_index;

    dev_index = gmac_select_interface(dev);
//	tp = gmac_dev[index]->priv;
    /* clear counter */
    gmac_read_reg(gmac_base_addr[dev_index] + GMAC_IN_DISCARDS);
    gmac_read_reg(gmac_base_addr[dev_index] + GMAC_IN_ERRORS);
    tp->stats.tx_bytes = 0;
    tp->stats.tx_packets = 0;
	tp->stats.tx_errors = 0;
    tp->stats.rx_bytes = 0;
	tp->stats.rx_packets = 0;
	tp->stats.rx_errors = 0;
    tp->stats.rx_dropped = 0;
	return (0);
}

static int gmac_open (struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
	int    retval;

    gmac_select_interface(dev);

	/* chip reset */
	gmac_sw_reset(dev);

    /* allocates tx/rx descriptor and buffer memory */
    gmac_init_desc_buf(dev);

    /* get mac address from FLASH */
    gmac_get_mac_address();

    /* set PHY register to start autonegition process */
    gmac_set_phy_status(dev);

	/* GMAC initialization */
	if (gmac_init_chip(dev))
	{
		printk (KERN_ERR "GMAC init fail\n");
	}

    /* start DMA process */
	gmac_hw_start(dev);

    /* enable tx/rx register */
    gmac_enable_tx_rx(dev);

    /* clear statistic counter */
    gmac_clear_counter(dev);

   	netif_start_queue (dev);

    /* hook ISR */
	retval = request_irq (dev->irq, gmac_interrupt, SA_INTERRUPT, dev->name, dev);
	if (retval)
		return retval;

	if(!FLAG_SWITCH)
	{
    	init_waitqueue_head (&tp->thr_wait);
    	init_completion(&tp->thr_exited);

    	tp->time_to_die = 0;
    	tp->thr_pid = kernel_thread (gmac_phy_thread, dev, CLONE_FS | CLONE_FILES);
    	if (tp->thr_pid < 0)
    	{
    		printk (KERN_WARNING "%s: unable to start kernel thread\n",dev->name);
    	}
    }
	return (0);
}

static int gmac_close(struct net_device *dev)
{
    struct gmac_private *tp = dev->priv;
    unsigned int        i,dev_index;
    unsigned int        ret;

    dev_index = gmac_get_dev_index(dev);

    /* stop tx/rx packet */
    gmac_disable_tx_rx(dev);

    /* stop the chip's Tx and Rx DMA processes */
	gmac_hw_stop(dev);

    netif_stop_queue(dev);

    /* disable interrupts by clearing the interrupt mask */
    synchronize_irq();
    free_irq(dev->irq,dev);

	DMA_MFREE(tp->tx_desc, TX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),(unsigned int)tp->tx_desc_dma);
	DMA_MFREE(tp->rx_desc, RX_DESC_NUM*sizeof(GMAC_DESCRIPTOR_T),(unsigned int)tp->rx_desc_dma);

#ifdef CONFIG_SL2312_MPAGE
//	kfree(tx_skb);
#endif

    for (i=0;i<RX_DESC_NUM;i++)
    {
        if (rx_skb[dev_index][i])
        {
            dev_kfree_skb(rx_skb[dev_index][i]);
        }
    }
	if(!FLAG_SWITCH)
	{
    	if (tp->thr_pid >= 0)
    	{
		    tp->time_to_die = 1;
    		wmb();
    		ret = kill_proc (tp->thr_pid, SIGTERM, 1);
    		if (ret)
    		{
    			printk (KERN_ERR "%s: unable to signal thread\n", dev->name);
    			return ret;
    		}
//    		wait_for_completion (&tp->thr_exited);
    	}
    }

    return (0);
}

#ifdef CONFIG_SL2312_MPAGE
int printk_all(int dev_index, struct gmac_private* tp)
{
	int i=0;
    unsigned int tx_current_descriptor = 0;
    int hw_index;
    int fi;
    GMAC_DESCRIPTOR_T* tmp_desc;

	GMAC_DESCRIPTOR_T* cur_desc=tp->tx_cur_desc;
	fi = ((unsigned int)cur_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
	printk("tmp_desc %x, id %d\n", (int)cur_desc, fi);

	tmp_desc = (GMAC_DESCRIPTOR_T*)((gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC) & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
	hw_index = ((unsigned int)tmp_desc - tx_desc_start_adr[dev_index])/ sizeof(GMAC_DESCRIPTOR_T);
	printk("hd_desc %x, ind %d, fin desc %x\n",(int)tmp_desc, hw_index, (int)tp->tx_finished_desc);

	for (i=0; i<TX_DESC_NUM; i++) {
		printk("**id %4d, hw_index %4d ==> ", fi, hw_index);
		printk("fc %8x ", tmp_desc->frame_ctrl.bits32);
		printk("fs %8x ", tmp_desc->flag_status.bits32);
		printk("fb %8x ", tmp_desc->buf_adr);
		printk("fd %8x\n",  tmp_desc->next_desc.next_descriptor);
	  	tmp_desc = (GMAC_DESCRIPTOR_T*)((tmp_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
		fi = ((unsigned int)tmp_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
	}
    tx_current_descriptor = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CURR_DESC);
    printk("%s: tx current descriptor = %x \n",__func__,tx_current_descriptor);
    printk("%s: interrupt status = %x \n",__func__,int_status);
    return 0;
}

int cleanup_desc(int dev_index, struct gmac_private* tp)
{
	int i=0;
	int index = ((unsigned int)tp->tx_cur_desc - tx_desc_start_adr[dev_index])/sizeof(GMAC_DESCRIPTOR_T);
	GMAC_DESCRIPTOR_T* fill_desc = tp->tx_cur_desc;

	for (i=0; i< TX_DESC_NUM; i++)
	{
		fill_desc->frame_ctrl.bits_tx_out.own = CPU;
		fill_desc->frame_ctrl.bits_tx_out.buffer_size = TX_BUF_SIZE;
		tx_skb[dev_index][index].desc_in_use = 0;
		free_tx_buf(dev_index, index);
		printk("cleanup di %d\n", index);
		fill_desc = (GMAC_DESCRIPTOR_T*)((fill_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
		index++;
		if (index > TX_DESC_NUM)
			index = 0;
	}
	return 1;
}

size_t get_available_tx_desc(struct net_device* dev, int dev_index)
{
	struct gmac_private *tp = dev->priv;
	unsigned int desc_hdr = tp->tx_desc_hdr[dev_index];
	unsigned int desc_tail = tp->tx_desc_tail[dev_index];
	int available_desc_num = (TX_DESC_NUM - desc_hdr + desc_tail) & (TX_DESC_NUM-1);
	if (!available_desc_num) {
		if (tx_skb[dev_index][desc_hdr].desc_in_use)
			return 0;
		else
			return TX_DESC_NUM;
	}
	return available_desc_num;
}

int check_free_tx_desc(int dev_index, int n, GMAC_DESCRIPTOR_T* desc)
{
	int i,index;
	GMAC_DESCRIPTOR_T* tmp_desc = desc;

	if (n > TX_DESC_NUM)
		return 0;

	index = ((unsigned int)tmp_desc - tx_desc_start_adr[dev_index])/sizeof(GMAC_DESCRIPTOR_T);
	for (i=0; i<n; i++)
	{
		if (tx_skb[dev_index][index].desc_in_use)
		{
			printk("sw desc %d is in use\n", index);
			/* cleanup all the descriptors to check if DMA still running */
			return 0;
		}
		index++;
		if (index == TX_DESC_NUM)
			index = 0;
	}
	return 1;
}

#define TCPHDRLEN(tcp_hdr)  ((ntohs(*((__u16 *)tcp_hdr + 6)) >> 12) & 0x000F)

inline int fill_in_desc(int dev_index, GMAC_DESCRIPTOR_T *desc, char* data, int len, int total_len, int sof, int freeable, int ownership, struct sk_buff* skb)
{
	int index = ((unsigned int)desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);

	if (desc->frame_ctrl.bits_tx_in.own == CPU)
	{
		tx_skb[dev_index][index].freeable = freeable;
		if ((sof & 0x01) && skb) {
			tx_skb[dev_index][index].skb = skb;
		}
		else
			tx_skb[dev_index][index].skb = 0;

		if (sof != 2)
			tx_skb[dev_index][index].desc_in_use = 1;
		else
			tx_skb[dev_index][index].desc_in_use = 0;

		consistent_sync(data, len, PCI_DMA_TODEVICE);
		desc->buf_adr = (unsigned int)__pa(data);
		desc->frame_ctrl.bits_tx_out.buffer_size = len;
		desc->flag_status.bits_tx_flag.frame_count = total_len;
		desc->next_desc.bits.eofie = 1;
		desc->next_desc.bits.sof_eof = sof;
		desc->frame_ctrl.bits_tx_out.vlan_enable = 0;
		desc->frame_ctrl.bits_tx_out.ip_csum_en = 1;     /* TSS IPv4 IP header checksum enable */
		desc->frame_ctrl.bits_tx_out.ipv6_tx_en = 1;    /* TSS IPv6 tx enable */
		desc->frame_ctrl.bits_tx_out.tcp_csum_en = 1;    /* TSS TCP checksum enable */
		desc->frame_ctrl.bits_tx_out.udp_csum_en = 1;    /* TSS UDP checksum enable */
        wmb();
		desc->frame_ctrl.bits_tx_out.own = ownership;
//		consistent_sync(desc, sizeof(GMAC_DESCRIPTOR_T), PCI_DMA_TODEVICE);
	}
	return 0;
}
#endif

static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct gmac_private     *tp = dev->priv;
	GMAC_TXDMA_CTRL_T		tx_ctrl,tx_ctrl_mask;
	GMAC_TXDMA_FIRST_DESC_T	txdma_busy;
	unsigned int            len = skb->len;
	unsigned int            dev_index;
	static unsigned int     pcount = 0;
#ifdef CONFIG_SL2312_MPAGE
    GMAC_DESCRIPTOR_T *fill_desc;
	int snd_pages = skb_shinfo(skb)->nr_frags;  /* get number of descriptor */
	int desc_needed = 1; // for jumbo packet, one descriptor is enough.
	int header_len = skb->len;
    struct iphdr	*ip_hdr;
    struct tcphdr	*tcp_hdr;
    int             tcp_hdr_len;
    int             data_len;
    int             prv_index;
    long            seq_num;
    int             first_desc_index;
    int             ownership, freeable;
    int             eof;
	int             i=0;
#endif
#ifdef CONFIG_TXINT_DISABLE
	int				available_desc_cnt = 0;
#endif

    dev_index = gmac_select_interface(dev);

#ifdef CONFIG_TXINT_DISABLE
	available_desc_cnt = get_available_tx_desc(dev, dev_index);

	if (available_desc_cnt < (TX_DESC_NUM >> 2)) {
		gmac_tx_packet_complete(dev);
	}
#endif

#ifdef CONFIG_SL2312_MPAGE

	fill_desc = tp->tx_cur_desc;
	if(!fill_desc) {
		printk("cur_desc is NULL!\n");
		return -1;
	}

	if (storlink_ctl.recvfile==2)
	{
	    printk("snd_pages=%d skb->len=%d\n",snd_pages,skb->len);
	}

	if (snd_pages)
		desc_needed += snd_pages;   /* decriptors needed for this large packet */

	if (!check_free_tx_desc(dev_index, desc_needed, fill_desc)) {
		printk("no available desc!\n");
        gmac_dump_register(dev);
		printk_all(dev_index, tp);
		tp->stats.tx_dropped++;
		if (pcount++ > 10)
		{
		    for (;;);
		}
		return -1;
	}

	first_desc_index = ((unsigned int)fill_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);

	/* check if the tcp packet is in order*/
	ip_hdr = (struct iphdr*) &(skb->data[14]);
	tcp_hdr = (struct tcphdr*) &(skb->data[14+ip_hdr->ihl * 4]);
	tcp_hdr_len = TCPHDRLEN(tcp_hdr) * 4;
	data_len = skb->len - 14 - ip_hdr->ihl *4 - tcp_hdr_len;

	prv_index = first_desc_index-1;
	if (prv_index <0)
	    prv_index += TX_DESC_NUM;
	seq_num = ntohl(tcp_hdr->seq);

	if (snd_pages)
	{
		// calculate header length
		// check fragment total length and header len = skb len - frag len
		// or parse the header.
		for (i=0; i<snd_pages; i++) {
			skb_frag_t* frag = &skb_shinfo(skb)->frags[i];
			header_len -= frag->size;
		}
		ownership = CPU;
		freeable = 0;
		/* fill header into first descriptor */
		fill_in_desc(dev_index, fill_desc, skb->data, header_len, len, 2, freeable, ownership, 0);
		fill_desc = (GMAC_DESCRIPTOR_T*)((fill_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
		tx_skb[dev_index][first_desc_index].end_seq = seq_num + data_len;

		eof = 0;
		ownership = DMA;
		for (i=0; i<snd_pages; i++)
		{
			skb_frag_t* frag = &skb_shinfo(skb)->frags[i];
			int start_pos = frag->page_offset;
			char* data_buf = page_address(frag->page);
			int data_size = frag->size;
			int cur_index;

			if (i == snd_pages-1)
			{
				eof=1;
				freeable = 1;
			}
			fill_in_desc(dev_index, fill_desc, data_buf+(start_pos), data_size,
			             len, eof, freeable, ownership, skb);
			cur_index = ((unsigned int)fill_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);

			fill_desc = (GMAC_DESCRIPTOR_T*)((fill_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
		}
		/* pass the ownership of the first descriptor to hardware */
//	    disable_irq(gmac_irq[dev_index]);
		tx_skb[dev_index][first_desc_index].desc_in_use = 1;
        wmb();
		tp->tx_cur_desc->frame_ctrl.bits_tx_out.own = DMA;
//		consistent_sync(tp->tx_cur_desc, sizeof(GMAC_DESCRIPTOR_T), PCI_DMA_TODEVICE);
		tp->tx_cur_desc = fill_desc;
		dev->trans_start = jiffies;
//	    enable_irq(gmac_irq[dev_index]);
	}
	else if ( tp->tx_cur_desc->frame_ctrl.bits_tx_out.own == CPU )
	{
//		tx_skb[dev_index][first_desc_index].end_seq = seq_num + data_len;
//	    disable_irq(gmac_irq[dev_index]);
		fill_in_desc(dev_index, tp->tx_cur_desc, skb->data, skb->len, skb->len, 3, 1, DMA, skb);
//	    enable_irq(gmac_irq[dev_index]);
		//consistent_sync(tp->tx_cur_desc, sizeof(GMAC_DESCRIPTOR_T), PCI_DMA_TODEVICE);
		tp->tx_cur_desc = (GMAC_DESCRIPTOR_T*)((tp->tx_cur_desc->next_desc.next_descriptor & 0xfffffff0) + tx_desc_virtual_base[dev_index]);
		dev->trans_start = jiffies;
	}
	else
	{
		printk("gmac tx drop!\n");
		tp->stats.tx_dropped++;
		return -1;
	}

#ifdef CONFIG_TXINT_DISABLE
	tp->tx_desc_hdr[dev_index] = (tp->tx_desc_hdr[dev_index] + desc_needed) & (TX_DESC_NUM-1);
#endif

#else
    if ((tp->tx_cur_desc->frame_ctrl.bits_tx_out.own == CPU) && (len < TX_BUF_SIZE))
	{
        index = ((unsigned int)tp->tx_cur_desc - tx_desc_start_adr[dev_index]) / sizeof(GMAC_DESCRIPTOR_T);
        tx_skb[dev_index][index] = skb;
        consistent_sync(skb->data,skb->len,PCI_DMA_TODEVICE);
        tp->tx_cur_desc->buf_adr = (unsigned int)__pa(skb->data);
    	tp->tx_cur_desc->flag_status.bits_tx_flag.frame_count = len;    /* total frame byte count */
    	tp->tx_cur_desc->next_desc.bits.sof_eof = 0x03;                 /*only one descriptor*/
		tp->tx_cur_desc->frame_ctrl.bits_tx_out.buffer_size = len;      /* descriptor byte count */
        tp->tx_cur_desc->frame_ctrl.bits_tx_out.vlan_enable = 0;
        tp->tx_cur_desc->frame_ctrl.bits_tx_out.ip_csum_en = 0;     /* TSS IPv4 IP header checksum enable */
        tp->tx_cur_desc->frame_ctrl.bits_tx_out.ipv6_tx_en = 0 ;    /* TSS IPv6 tx enable */
        tp->tx_cur_desc->frame_ctrl.bits_tx_out.tcp_csum_en = 0;    /* TSS TCP checksum enable */
        tp->tx_cur_desc->frame_ctrl.bits_tx_out.udp_csum_en = 0;    /* TSS UDP checksum enable */
        wmb();
    	tp->tx_cur_desc->frame_ctrl.bits_tx_out.own = DMA;	        /* set owner bit */
    	tp->tx_cur_desc = (GMAC_DESCRIPTOR_T *)((tp->tx_cur_desc->next_desc.next_descriptor & 0xfffffff0)+tx_desc_virtual_base[dev_index]);
    	dev->trans_start = jiffies;
	}
	else
	{
	 	/* no free tx descriptor */
		dev_kfree_skb(skb);
	    netif_stop_queue(dev);
		tp->stats.tx_dropped++;
		return (-1);
	}
#endif
 	/* if TX DMA process is stoped , restart it */
	txdma_busy.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_FIRST_DESC);
	if (txdma_busy.bits.td_busy == 0)
	{
		/* restart DMA process */
		tx_ctrl.bits32 = 0;
		tx_ctrl.bits.td_start = 1;
		tx_ctrl.bits.td_continue = 1;
		tx_ctrl_mask.bits32 = 0;
		tx_ctrl_mask.bits.td_start = 1;
		tx_ctrl_mask.bits.td_continue = 1;
		gmac_write_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CTRL,tx_ctrl.bits32,tx_ctrl_mask.bits32);
	}
	return (0);
}


struct net_device_stats * gmac_get_stats(struct net_device *dev)
{
    struct gmac_private *tp = dev->priv;
    unsigned long       flags;
    unsigned int        pkt_drop;
    unsigned int        pkt_error;
    unsigned int        dev_index;

    dev_index = gmac_select_interface(dev);

//	if (storlink_ctl.recvfile==3)
//	{
//        printk("GMAC_GLOBAL_BASE_ADDR=%x\n", readl(GMAC_GLOBAL_BASE_ADDR+0x30));
//        gmac_dump_register(dev);
//        printk_all(0, dev);
//    }

    if (netif_running(dev))
    {
        /* read H/W counter */
        spin_lock_irqsave(&tp->lock,flags);
        pkt_drop = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_IN_DISCARDS);
        pkt_error = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_IN_ERRORS);
        tp->stats.rx_dropped = tp->stats.rx_dropped + pkt_drop;
        tp->stats.rx_errors = tp->stats.rx_errors + pkt_error;
        spin_unlock_irqrestore(&tp->lock,flags);
    }
    return &tp->stats;
}

static unsigned const ethernet_polynomial = 0x04c11db7U;
static inline u32 ether_crc (int length, unsigned char *data)
{
	int crc = -1;
	unsigned int i;
	unsigned int crc_val=0;

	while (--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 0; bit < 8; bit++, current_octet >>= 1)
			crc = (crc << 1) ^ ((crc < 0) ^ (current_octet & 1) ?
			     ethernet_polynomial : 0);
	}
	crc = ~crc;
	for (i=0;i<32;i++)
	{
		crc_val = crc_val + (((crc << i) & 0x80000000) >> (31-i));
	}
	return crc_val;
}

static void gmac_set_rx_mode(struct net_device *dev)
{
    GMAC_RX_FLTR_T      filter;
	unsigned int        mc_filter[2];	/* Multicast hash filter */
    int                 bit_nr;
	unsigned int        i, dev_index;

    dev_index = gmac_select_interface(dev);

//    printk("%s : dev->flags = %x \n",__func__,dev->flags);
//    dev->flags |= IFF_ALLMULTI;  /* temp */
    filter.bits32 = 0;
    filter.bits.error = 0;
	if (dev->flags & IFF_PROMISC)
	{
	    filter.bits.error = 1;
        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	}
	else if (dev->flags & IFF_ALLMULTI)
	{
        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	}
	else
	{
		struct dev_mc_list *mclist;

        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;i++, mclist = mclist->next)
		{
            bit_nr = ether_crc(ETH_ALEN,mclist->dmi_addr) & 0x0000003f;
            if (bit_nr < 32)
            {
                mc_filter[0] = mc_filter[0] | (1<<bit_nr);
            }
            else
            {
                mc_filter[1] = mc_filter[1] | (1<<(bit_nr-32));
            }
		}
	}
    filter.bits32 = 0x1f;
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_RX_FLTR,filter.bits32,0xffffffff);

    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_MCAST_FIL0,mc_filter[0],0xffffffff);
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_MCAST_FIL1,mc_filter[1],0xffffffff);
    return;
}

static int gmac_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sock;
	unsigned int    reg_val;
	unsigned int    dev_index;
    unsigned int    i;

    dev_index = gmac_select_interface(dev);

	sock = (struct sockaddr *) addr;
	for (i = 0; i < 6; i++)
	{
		dev->dev_addr[i] = sock->sa_data[i];
	}

    reg_val = dev->dev_addr[0] + (dev->dev_addr[1]<<8) + (dev->dev_addr[2]<<16) + (dev->dev_addr[3]<<24);
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_STA_ADD0,reg_val,0xffffffff);
    reg_val = dev->dev_addr[4] + (dev->dev_addr[5]<<8) ;
    gmac_write_reg(gmac_base_addr[dev_index] + GMAC_STA_ADD1,reg_val,0x0000ffff);
    memcpy(&eth0_mac[0],&dev->dev_addr[0],6);
    printk("Storlink %s address = ",dev->name);
    printk("%02x",dev->dev_addr[0]);
    printk("%02x",dev->dev_addr[1]);
    printk("%02x",dev->dev_addr[2]);
    printk("%02x",dev->dev_addr[3]);
    printk("%02x",dev->dev_addr[4]);
    printk("%02x\n",dev->dev_addr[5]);

    return (0);
}

static void gmac_tx_timeout(struct net_device *dev)
{
	GMAC_TXDMA_CTRL_T	        tx_ctrl,tx_ctrl_mask;
    GMAC_TXDMA_FIRST_DESC_T     txdma_busy;
    int							dev_index;

    dev_index = gmac_select_interface(dev);

    /* if TX DMA process is stoped , restart it */
	txdma_busy.bits32 = gmac_read_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_FIRST_DESC);
	if (txdma_busy.bits.td_busy == 0)
	{
		/* restart DMA process */
		tx_ctrl.bits32 = 0;
		tx_ctrl.bits.td_start = 1;
		tx_ctrl.bits.td_continue = 1;
		tx_ctrl_mask.bits32 = 0;
		tx_ctrl_mask.bits.td_start = 1;
		tx_ctrl_mask.bits.td_continue = 1;
		gmac_write_reg(gmac_base_addr[dev_index] + GMAC_TXDMA_CTRL,tx_ctrl.bits32,tx_ctrl_mask.bits32);
	}
	netif_wake_queue(dev);
    return;
}

static int gmac_netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int rc = 0;
    unsigned char *hwa = rq->ifr_ifru.ifru_hwaddr.sa_data;

	if (!netif_running(dev))
	{
	    printk("Before changing the H/W address,please down the device.\n");
		return -EINVAL;
    }

	switch (cmd) {
	case SIOCETHTOOL:
        break;

    case SIOCSIFHWADDR:
        gmac_set_mac_address(dev,hwa);
        break;

	case SIOCGMIIPHY:	/* Get the address of the PHY in use. */
	case SIOCDEVPRIVATE:	/* binary compat, remove in 2.5 */
        break;

	case SIOCGMIIREG:	/* Read the specified MII register. */
	case SIOCDEVPRIVATE+1:
		break;

	case SIOCSMIIREG:	/* Write the specified MII register */
	case SIOCDEVPRIVATE+2:
		break;

	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

static void gmac_cleanup_module(void)
{
    int i;

    for (i=0;i<GMAC_PHY_IF;i++)
    {
        unregister_netdev(gmac_dev[i]);
    }
	return ;
}

static int __init gmac_init_module(void)
{
	struct gmac_private *tp;
	struct net_device *dev[GMAC_PHY_IF];
	unsigned int i;

#ifdef MODULE
	printk (KERN_INFO RTL8139_DRIVER_NAME "\n");
#endif
//	init_waitqueue_entry(&wait, current);

	printk("GMAC Init......\n");
	for(i = 0; i<GMAC_PHY_IF; i++)
	{
		dev[i] = alloc_etherdev(sizeof(struct gmac_private));
		if (dev[i] == NULL)
		{
			printk (KERN_ERR "Can't allocate ethernet device #%d .\n",i);
			return -ENOMEM;
		}
        gmac_dev[i] = dev[i];

		SET_MODULE_OWNER(dev[i]);

		tp = dev[i]->priv;

		dev[i]->base_addr = gmac_base_addr[i];
		dev[i]->irq = gmac_irq[i];
	    dev[i]->open = gmac_open;
	    dev[i]->stop = gmac_close;
		dev[i]->hard_start_xmit = gmac_start_xmit;
		dev[i]->get_stats = gmac_get_stats;
		dev[i]->set_multicast_list = gmac_set_rx_mode;
		dev[i]->set_mac_address = gmac_set_mac_address;
		dev[i]->do_ioctl = gmac_netdev_ioctl;
		dev[i]->tx_timeout = gmac_tx_timeout;
		dev[i]->watchdog_timeo = TX_TIMEOUT;
		dev[i]->features |= NETIF_F_SG|NETIF_F_HW_CSUM|NETIF_F_TSO;
#ifdef CONFIG_SL_NAPI
        printk("NAPI driver is enabled.\n");
        if (i==0)
        {
	        dev[i]->poll = gmac_rx_poll_ga;
	        dev[i]->weight = 64;
	    }
	    else
	    {
	        dev[i]->poll = gmac_rx_poll_gb;
	        dev[i]->weight = 64;
	    }
#endif

		if (register_netdev(dev[i]))
		{
			gmac_cleanup_module();
			return(-1);
		}
	}

#ifdef CONFIG_SL3516_ASIC
{
    unsigned int    val;

    /* set GMAC global register */
    val = readl(GMAC_GLOBAL_BASE_ADDR+0x10);
    val = val | 0x005a0000;
    writel(val,GMAC_GLOBAL_BASE_ADDR+0x10);
    writel(0x07f007f0,GMAC_GLOBAL_BASE_ADDR+0x1c);
    writel(0x77770000,GMAC_GLOBAL_BASE_ADDR+0x20);
    writel(0x77770000,GMAC_GLOBAL_BASE_ADDR+0x24);
	val = readl(GMAC_GLOBAL_BASE_ADDR+0x04);
	if((val&(1<<20))==0){           // GMAC1 enable
 		val = readl(GMAC_GLOBAL_BASE_ADDR+0x30);
		val = (val & 0xe7ffffff) | 0x08000000;
		writel(val,GMAC_GLOBAL_BASE_ADDR+0x30);
	}

}
#endif

//	printk("%s: dev0=%x  dev1=%x \n",__func__,dev[0],dev[1]);
//	FLAG_SWITCH = 0 ;
//	FLAG_SWITCH = SPI_get_identifier();
//	if(FLAG_SWITCH)
//	{
//		printk("Configure ADM699X...\n");
//		SPI_default();	//Add by jason for ADM699X configuration
//	}
	return (0);
}


module_init(gmac_init_module);
module_exit(gmac_cleanup_module);

static int gmac_phy_thread (void *data)
{
	struct net_device   *dev = data;
	struct gmac_private *tp = dev->priv;
	unsigned long       timeout;

    daemonize("%s", dev->name);
	allow_signal(SIGTERM);
//	reparent_to_init();
//	spin_lock_irq(&current->sigmask_lock);
//	sigemptyset(&current->blocked);
//	recalc_sigpending(current);
//	spin_unlock_irq(&current->sigmask_lock);
//	strncpy (current->comm, dev->name, sizeof(current->comm) - 1);
//	current->comm[sizeof(current->comm) - 1] = '\0';

	while (1)
	{
	    timeout = next_tick;
		do
		{
			timeout = interruptible_sleep_on_timeout (&tp->thr_wait, timeout);
		} while (!signal_pending (current) && (timeout > 0));

		if (signal_pending (current))
		{
//			spin_lock_irq(&current->sigmask_lock);
			flush_signals(current);
//			spin_unlock_irq(&current->sigmask_lock);
		}

		if (tp->time_to_die)
			break;

//        printk("%s : Polling PHY Status...%x\n",__func__,dev);
		rtnl_lock ();
        gmac_get_phy_status(dev);
		rtnl_unlock ();
	}
	complete_and_exit (&tp->thr_exited, 0);
}

static void gmac_set_phy_status(struct net_device *dev)
{
    GMAC_STATUS_T   status;
    unsigned int    reg_val;
    unsigned int    i = 0;
    unsigned int    index;

    if (FLAG_SWITCH==1)
    {
        return; /* GMAC connects to a switch chip, not PHY */
    }

    index = gmac_get_dev_index(dev);

    if (index == 0)
    {
//    	mii_write(phy_addr[index],0x04,0x0461); /* advertisement 10M full duplex, pause capable on */
//    	mii_write(phy_addr[index],0x04,0x0421); /* advertisement 10M half duplex, pause capable on */
    	mii_write(phy_addr[index],0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
//    	mii_write(phy_addr[index],0x04,0x04a1); /* advertisement 100M half duplex, pause capable on */
#ifdef CONFIG_SL3516_ASIC
    	mii_write(phy_addr[index],0x09,0x0300); /* advertisement 1000M full duplex, pause capable on */
//    	mii_write(phy_addr[index],0x09,0x0000); /* advertisement 1000M full duplex, pause capable on */
#endif
    }
    else
    {
//    	mii_write(phy_addr[index],0x04,0x0461); /* advertisement 10M full duplex, pause capable on */
//    	mii_write(phy_addr[index],0x04,0x0421); /* advertisement 10M half duplex, pause capable on */
    	mii_write(phy_addr[index],0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
//    	mii_write(phy_addr[index],0x04,0x04a1); /* advertisement 100M half duplex, pause capable on */
#ifdef CONFIG_SL3516_ASIC
//    	mii_write(phy_addr[index],0x09,0x0000); /* advertisement no 1000M */
    	mii_write(phy_addr[index],0x09,0x0300); /* advertisement 1000M full duplex, pause capable on */
#endif
	}

    mii_write(phy_addr[index],0x00,0x1200); /* Enable and Restart Auto-Negotiation */
    mii_write(phy_addr[index],0x18,0x0041); /* Enable Active led */
    while (((reg_val=mii_read(phy_addr[index],0x01)) & 0x00000004)!=0x04)
    {
        i++;
        if (i > 30)
        {
            break;
        }
	msleep(100);
    }
    if (i>30)
    {
        pre_phy_status[index] = LINK_DOWN;
		clear_bit(__LINK_STATE_START, &dev->state);
        netif_stop_queue(dev);
        storlink_ctl.link = 0;
        printk("Link Down (%04x) ",reg_val);
    }
    else
    {
        pre_phy_status[index] = LINK_UP;
		set_bit(__LINK_STATE_START, &dev->state);
        netif_wake_queue(dev);
        storlink_ctl.link = 1;
        printk("Link Up (%04x) ",reg_val);
    }

    status.bits32 = 0;
    reg_val = mii_read(phy_addr[index],10);
    printk("reg_val0 = %x \n",reg_val);
    if ((reg_val & 0x0800) == 0x0800)
    {
        status.bits.duplex = 1;
        status.bits.speed = 2;
        printk(" 1000M/Full \n");
    }
    else if ((reg_val & 0x0400) == 0x0400)
    {
        status.bits.duplex = 0;
        status.bits.speed = 2;
        printk(" 1000M/Half \n");
    }
    else
    {
        reg_val = (mii_read(phy_addr[index],0x05) & 0x05E0) >> 5;
        printk("reg_val1 = %x \n",reg_val);
        if ((reg_val & 0x08)==0x08) /* 100M full duplex */
        {
                status.bits.duplex = 1;
                status.bits.speed = 1;
                printk(" 100M/Full \n");
        }
        else if ((reg_val & 0x04)==0x04) /* 100M half duplex */
        {
                status.bits.duplex = 0;
                status.bits.speed = 1;
                printk(" 100M/Half \n");
        }
        else if ((reg_val & 0x02)==0x02) /* 10M full duplex */
        {
                status.bits.duplex = 1;
                status.bits.speed = 0;
                printk(" 10M/Full \n");
        }
        else if ((reg_val & 0x01)==0x01) /* 10M half duplex */
        {
                status.bits.duplex = 0;
                status.bits.speed = 0;
                printk(" 100M/Half \n");
        }
    }

    reg_val = (mii_read(phy_addr[index],0x05) & 0x05E0) >> 5;
    if ((reg_val & 0x20)==0x20)
    {
        flow_control_enable[index] = 1;
        printk("Flow Control Enable. \n");
    }
    else
    {
        flow_control_enable[index] = 0;
        printk("Flow Control Disable. \n");
    }
    full_duplex = status.bits.duplex;
    speed = status.bits.speed;
}

static void gmac_get_phy_status(struct net_device *dev)
{
	GMAC_CONFIG0_T	config0,config0_mask;
    GMAC_STATUS_T   status;
    unsigned int    reg_val;
    unsigned int    index;

    index = gmac_select_interface(dev);

    status.bits32 = 0;
    status.bits.phy_mode = 1;

#ifdef CONFIG_SL3516_ASIC
    status.bits.mii_rmii = 2;   /* default value for ASIC version */
//    status.bits.speed = 1;
#else
    if (index==0)
        status.bits.mii_rmii = 0;
    else
        status.bits.mii_rmii = 2;
#endif

    /* read PHY status register */
    reg_val = mii_read(phy_addr[index],0x01);
    if ((reg_val & 0x0024) == 0x0024) /* link is established and auto_negotiate process completed */
    {
        /* read PHY Auto-Negotiation Link Partner Ability Register */
        reg_val = mii_read(phy_addr[index],10);
        if ((reg_val & 0x0800) == 0x0800)
        {
            status.bits.mii_rmii = 3;  /* RGMII 1000Mbps mode */
            status.bits.duplex = 1;
            status.bits.speed = 2;
        }
        else if ((reg_val & 0x0400) == 0x0400)
        {
            status.bits.mii_rmii = 3;  /* RGMII 1000Mbps mode */
            status.bits.duplex = 0;
            status.bits.speed = 2;
        }
        else
        {
            reg_val = (mii_read(phy_addr[index],0x05) & 0x05E0) >> 5;
            if ((reg_val & 0x08)==0x08) /* 100M full duplex */
            {
                    status.bits.mii_rmii = 2;  /* RGMII 10/100Mbps mode */
                    status.bits.duplex = 1;
                    status.bits.speed = 1;
            }
            else if ((reg_val & 0x04)==0x04) /* 100M half duplex */
            {
                    status.bits.mii_rmii = 2;  /* RGMII 10/100Mbps mode */
                    status.bits.duplex = 0;
                    status.bits.speed = 1;
            }
            else if ((reg_val & 0x02)==0x02) /* 10M full duplex */
            {
                    status.bits.mii_rmii = 2;  /* RGMII 10/100Mbps mode */
                    status.bits.duplex = 1;
                    status.bits.speed = 0;
            }
            else if ((reg_val & 0x01)==0x01) /* 10M half duplex */
            {
                    status.bits.mii_rmii = 2;  /* RGMII 10/100Mbps mode */
                    status.bits.duplex = 0;
                    status.bits.speed = 0;
            }
        }
        status.bits.link = LINK_UP; /* link up */
        netif_wake_queue(dev);

        reg_val = (mii_read(phy_addr[index],0x05) & 0x05E0) >> 5;
        if ((reg_val & 0x20)==0x20)
        {
            if (flow_control_enable[index] == 0)
            {
                config0.bits32 = 0;
                config0_mask.bits32 = 0;
                config0.bits.tx_fc_en = 1; /* enable tx flow control */
                config0.bits.rx_fc_en = 1; /* enable rx flow control */
                config0_mask.bits.tx_fc_en = 1;
                config0_mask.bits.rx_fc_en = 1;
                gmac_write_reg(gmac_base_addr[index] + GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
//                printk("eth%d Flow Control Enable. \n",index);
            }
            flow_control_enable[index] = 1;
        }
        else
        {
            if (flow_control_enable[index] == 1)
            {
                config0.bits32 = 0;
                config0_mask.bits32 = 0;
                config0.bits.tx_fc_en = 0; /* disable tx flow control */
                config0.bits.rx_fc_en = 0; /* disable rx flow control */
                config0_mask.bits.tx_fc_en = 1;
                config0_mask.bits.rx_fc_en = 1;
                gmac_write_reg(gmac_base_addr[index] + GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
//                printk("eth%d Flow Control Disable. \n",index);
            }
            flow_control_enable[index] = 0;
        }

        if (pre_phy_status[index] == LINK_DOWN)
        {
            gmac_enable_tx_rx(dev);
            pre_phy_status[index] = LINK_UP;
			set_bit(__LINK_STATE_START, &dev->state);
			storlink_ctl.link = 1;
//			printk("eth%d Link Up ...\n",index);
        }
    }
    else
    {
        status.bits.link = LINK_DOWN; /* link down */
        netif_stop_queue(dev);
        flow_control_enable[index] = 0;
        storlink_ctl.link = 0;
        if (pre_phy_status[index] == LINK_UP)
        {
            gmac_disable_tx_rx(dev);
            pre_phy_status[index] = LINK_DOWN;
			clear_bit(__LINK_STATE_START, &dev->state);
//			printk("eth%d Link Down ...\n",index);
    	}

    }

    reg_val = gmac_read_reg(gmac_base_addr[index] + GMAC_STATUS);
    if (reg_val != status.bits32)
    {
        gmac_write_reg(gmac_base_addr[index] + GMAC_STATUS,status.bits32,0x0000007f);
    }
}

/***************************************/
/* define GPIO module base address     */
/***************************************/
#define GPIO_BASE_ADDR  (IO_ADDRESS(SL2312_GPIO_BASE))

/* define GPIO pin for MDC/MDIO */

// for gemini ASIC
#ifdef CONFIG_SL3516_ASIC
#define H_MDC_PIN           22
#define H_MDIO_PIN          21
#define G_MDC_PIN           22
#define G_MDIO_PIN          21
#else
#define H_MDC_PIN           3
#define H_MDIO_PIN          2
#define G_MDC_PIN           0
#define G_MDIO_PIN          1
#endif

//#define GPIO_MDC             0x80000000
//#define GPIO_MDIO            0x00400000

static unsigned int GPIO_MDC = 0;
static unsigned int GPIO_MDIO = 0;
static unsigned int GPIO_MDC_PIN = 0;
static unsigned int GPIO_MDIO_PIN = 0;

// For PHY test definition!!
#define LPC_EECK		0x02
#define LPC_EDIO		0x04
#define LPC_GPIO_SET		3
#define LPC_BASE_ADDR		IO_ADDRESS(IT8712_IO_BASE)
#define inb_gpio(x)		inb(LPC_BASE_ADDR + IT8712_GPIO_BASE + x)
#define outb_gpio(x, y)		outb(y, LPC_BASE_ADDR + IT8712_GPIO_BASE + x)

enum GPIO_REG
{
    GPIO_DATA_OUT   = 0x00,
    GPIO_DATA_IN    = 0x04,
    GPIO_PIN_DIR    = 0x08,
    GPIO_BY_PASS    = 0x0c,
    GPIO_DATA_SET   = 0x10,
    GPIO_DATA_CLEAR = 0x14,
};
/***********************/
/*    MDC : GPIO[31]   */
/*    MDIO: GPIO[22]   */
/***********************/

/***************************************************
* All the commands should have the frame structure:
*<PRE><ST><OP><PHYAD><REGAD><TA><DATA><IDLE>
****************************************************/

/*****************************************************************
* Inject a bit to NWay register through CSR9_MDC,MDIO
*******************************************************************/
void mii_serial_write(char bit_MDO) // write data into mii PHY
{
#if 0 //def CONFIG_SL2312_LPC_IT8712
	unsigned char iomode,status;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode |= (LPC_EECK|LPC_EDIO) ;				// Set EECK,EDIO,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	if(bit_MDO)
	{
		status = inb_gpio( LPC_GPIO_SET);
		status |= LPC_EDIO ;		//EDIO high
		outb_gpio(LPC_GPIO_SET, status);
	}
	else
	{
		status = inb_gpio( LPC_GPIO_SET);
		status &= ~(LPC_EDIO) ;		//EDIO low
		outb_gpio(LPC_GPIO_SET, status);
	}

	status |= LPC_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(LPC_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

#else
    unsigned int addr;
    unsigned int value;

    addr = GPIO_BASE_ADDR + GPIO_PIN_DIR;
    value = readl(addr) | GPIO_MDC | GPIO_MDIO; /* set MDC/MDIO Pin to output */
    writel(value,addr);
    if(bit_MDO)
    {
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDIO,addr); /* set MDIO to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDC,addr); /* set MDC to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDC,addr); /* set MDC to 0 */
    }
    else
    {
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDIO,addr); /* set MDIO to 0 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDC,addr); /* set MDC to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDC,addr); /* set MDC to 0 */
    }

#endif
}

/**********************************************************************
* read a bit from NWay register through CSR9_MDC,MDIO
***********************************************************************/
unsigned int mii_serial_read(void) // read data from mii PHY
{
#if 0 //def CONFIG_SL2312_LPC_IT8712
  	unsigned char iomode,status;
	unsigned int value ;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode &= ~(LPC_EDIO) ;		// Set EDIO input
	iomode |= (LPC_EECK) ;		// Set EECK,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	status = inb_gpio( LPC_GPIO_SET);
	status |= LPC_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(LPC_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

	value = inb_gpio( LPC_GPIO_SET);

	value = value>>2 ;
	value &= 0x01;

	return value ;

#else
    unsigned int *addr;
    unsigned int value;

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_PIN_DIR);
    value = readl(addr) & ~GPIO_MDIO; //0xffbfffff;   /* set MDC to output and MDIO to input */
    writel(value,addr);

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_SET);
    writel(GPIO_MDC,addr); /* set MDC to 1 */
    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
    writel(GPIO_MDC,addr); /* set MDC to 0 */

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_IN);
    value = readl(addr);
    value = (value & (1<<GPIO_MDIO_PIN)) >> GPIO_MDIO_PIN;
    return(value);

#endif
}

/***************************************
* preamble + ST
***************************************/
void mii_pre_st(void)
{
    unsigned char i;

    for(i=0;i<32;i++) // PREAMBLE
        mii_serial_write(1);
    mii_serial_write(0); // ST
    mii_serial_write(1);
}


/******************************************
* Read MII register
* phyad -> physical address
* regad -> register address
***************************************** */
unsigned int mii_read(unsigned char phyad,unsigned char regad)
{
    unsigned int i,value;
    unsigned int bit;

    if (phyad == GPHY_ADDR)
    {
        GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
        GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
    }
    else
    {
        GPIO_MDC_PIN = H_MDC_PIN;   /* assigned MDC pin for 10/100 PHY */
        GPIO_MDIO_PIN = H_MDIO_PIN; /* assigned MDIO pin for 10/100 PHY */
    }
    GPIO_MDC = (1<<GPIO_MDC_PIN);
    GPIO_MDIO = (1<<GPIO_MDIO_PIN);

    mii_pre_st(); // PRE+ST
    mii_serial_write(1); // OP
    mii_serial_write(0);

    for (i=0;i<5;i++) { // PHYAD
        bit= ((phyad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    for (i=0;i<5;i++) { // REGAD
        bit= ((regad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    mii_serial_read(); // TA_Z
//    if((bit=mii_serial_read()) !=0 ) // TA_0
//    {
//        return(0);
//    }
    value=0;
    for (i=0;i<16;i++) { // READ DATA
        bit=mii_serial_read();
        value += (bit<<(15-i)) ;
    }

    mii_serial_write(0); // dumy clock
    mii_serial_write(0); // dumy clock
//printk("%s: phy_addr=%x reg_addr=%x value=%x \n",__func__,phyad,regad,value);
    return(value);
}

/******************************************
* Write MII register
* phyad -> physical address
* regad -> register address
* value -> value to be write
***************************************** */
void mii_write(unsigned char phyad,unsigned char regad,unsigned int value)
{
    unsigned int i;
    char bit;

printk("%s: phy_addr=%x reg_addr=%x value=%x \n",__func__,phyad,regad,value);
    if (phyad == GPHY_ADDR)
    {
        GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
        GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
    }
    else
    {
        GPIO_MDC_PIN = H_MDC_PIN;   /* assigned MDC pin for 10/100 PHY */
        GPIO_MDIO_PIN = H_MDIO_PIN; /* assigned MDIO pin for 10/100 PHY */
    }
    GPIO_MDC = (1<<GPIO_MDC_PIN);
    GPIO_MDIO = (1<<GPIO_MDIO_PIN);

    mii_pre_st(); // PRE+ST
    mii_serial_write(0); // OP
    mii_serial_write(1);
    for (i=0;i<5;i++) { // PHYAD
        bit= ((phyad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    for (i=0;i<5;i++) { // REGAD
        bit= ((regad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }
    mii_serial_write(1); // TA_1
    mii_serial_write(0); // TA_0

    for (i=0;i<16;i++) { // OUT DATA
        bit= ((value>>(15-i)) & 0x01) ? 1 : 0 ;
        mii_serial_write(bit);
    }
    mii_serial_write(0); // dumy clock
    mii_serial_write(0); // dumy clock
}









/*				NOTES
 *   The instruction set of the 93C66/56/46/26/06 chips are as follows:
 *
 *               Start  OP	    *
 *     Function   Bit  Code  Address**  Data     Description
 *     -------------------------------------------------------------------
 *     READ        1    10   A7 - A0             Reads data stored in memory,
 *                                               starting at specified address
 *     EWEN        1    00   11XXXXXX            Write enable must precede
 *                                               all programming modes
 *     ERASE       1    11   A7 - A0             Erase register A7A6A5A4A3A2A1A0
 *     WRITE       1    01   A7 - A0   D15 - D0  Writes register
 *     ERAL        1    00   10XXXXXX            Erase all registers
 *     WRAL        1    00   01XXXXXX  D15 - D0  Writes to all registers
 *     EWDS        1    00   00XXXXXX            Disables all programming
 *                                               instructions
 *    *Note: A value of X for address is a don't care condition.
 *    **Note: There are 8 address bits for the 93C56/66 chips unlike
 *	      the 93C46/26/06 chips which have 6 address bits.
 *
 *   The 93Cx6 has a four wire interface: clock, chip select, data in, and
 *   data out.While the ADM6996 uning three interface: clock, chip select,and data line.
 *   The input and output are the same pin. ADM6996 can only recognize the write cmd.
 *   In order to perform above functions, you need
 *   1. to enable the chip select .
 *   2. send one clock of dummy clock
 *   3. send start bit and opcode
 *   4. send 8 bits address and 16 bits data
 *   5. to disable the chip select.
 *							Jason Lee 2003/07/30
 */

/***************************************/
/* define GPIO module base address     */
/***************************************/
#define GPIO_EECS	     0x00400000		/*   EECS: GPIO[22]   */
//#define GPIO_MOSI	     0x20000000         /*   EEDO: GPIO[29]   send to 6996*/
#define GPIO_MISO	     0x40000000         /*   EEDI: GPIO[30]   receive from 6996*/
#define GPIO_EECK	     0x80000000         /*   EECK: GPIO[31]   */

#define ADM_EECS		0x01
#define ADM_EECK		0x02
#define ADM_EDIO		0x04
/*************************************************************
* SPI protocol for ADM6996 control
**************************************************************/
#define SPI_OP_LEN	     0x03		// the length of start bit and opcode
#define SPI_OPWRITE	     0X05		// write
#define SPI_OPREAD	     0X06		// read
#define SPI_OPERASE	     0X07		// erase
#define SPI_OPWTEN	     0X04		// write enable
#define SPI_OPWTDIS	     0X04		// write disable
#define SPI_OPERSALL	     0X04		// erase all
#define SPI_OPWTALL	     0X04		// write all

#define SPI_ADD_LEN	     8			// bits of Address
#define SPI_DAT_LEN	     16			// bits of Data
#define ADM6996_PORT_NO	     6			// the port number of ADM6996
#define ADM6999_PORT_NO	     9			// the port number of ADM6999
#ifdef CONFIG_ADM_6996
	#define ADM699X_PORT_NO		ADM6996_PORT_NO
#endif
#ifdef CONFIG_ADM_6999
	#define ADM699X_PORT_NO		ADM6999_PORT_NO
#endif
#define LPC_GPIO_SET		3
#define LPC_BASE_ADDR			IO_ADDRESS(IT8712_IO_BASE)

extern int it8712_exist;

#define inb_gpio(x)			inb(LPC_BASE_ADDR + IT8712_GPIO_BASE + x)
#define outb_gpio(x, y)		outb(y, LPC_BASE_ADDR + IT8712_GPIO_BASE + x)

/****************************************/
/*	Function Declare		*/
/****************************************/
/*
void SPI_write(unsigned char addr,unsigned int value);
unsigned int SPI_read(unsigned char table,unsigned char addr);
void SPI_write_bit(char bit_EEDO);
unsigned int SPI_read_bit(void);
void SPI_default(void);
void SPI_reset(unsigned char rstype,unsigned char port_cnt);
void SPI_pre_st(void);
void SPI_CS_enable(unsigned char enable);
void SPI_Set_VLAN(unsigned char LAN,unsigned int port_mask);
void SPI_Set_tag(unsigned int port,unsigned tag);
void SPI_Set_PVID(unsigned int PVID,unsigned int port_mask);
void SPI_mac_lock(unsigned int port, unsigned char lock);
void SPI_get_port_state(unsigned int port);
void SPI_port_enable(unsigned int port,unsigned char enable);

void SPI_get_status(unsigned int port);
*/

struct PORT_CONFIG
{
	unsigned char auto_negotiation;	// 0:Disable	1:Enable
	unsigned char speed;		// 0:10M	1:100M
	unsigned char duplex;		// 0:Half	1:Full duplex
	unsigned char Tag;		// 0:Untag	1:Tag
	unsigned char port_disable;	// 0:port enable	1:disable
	unsigned char pvid;		// port VLAN ID 0001
	unsigned char mdix;		// Crossover judgement. 0:Disable 1:Enable
	unsigned char mac_lock;		// MAC address Lock 0:Disable 1:Enable
};

struct PORT_STATUS
{
	unsigned char link;		// 0:not link	1:link established
	unsigned char speed;		// 0:10M	1:100M
	unsigned char duplex;		// 0:Half	1:Full duplex
	unsigned char flow_ctl;		// 0:flow control disable 1:enable
	unsigned char mac_lock;		// MAC address Lock 0:Disable 1:Enable
	unsigned char port_disable;	// 0:port enable	1:disable

	// Serial Management
	unsigned long rx_pac_count;		//receive packet count
	unsigned long rx_pac_byte;		//receive packet byte count
	unsigned long tx_pac_count;		//transmit packet count
	unsigned long tx_pac_byte;		//transmit packet byte count
	unsigned long collision_count;		//error count
	unsigned long error_count ;

	unsigned long rx_pac_count_overflow;		//overflow flag
	unsigned long rx_pac_byte_overflow;
	unsigned long tx_pac_count_overflow;
	unsigned long tx_pac_byte_overflow;
	unsigned long collision_count_overflow;
	unsigned long error_count_overflow;
};

struct PORT_CONFIG port_config[ADM699X_PORT_NO];	// 0~3:LAN , 4:WAN , 5:MII
static struct PORT_STATUS port_state[ADM699X_PORT_NO];

/******************************************
* SPI_write
* addr -> Write Address
* value -> value to be write
***************************************** */
void SPI_write(unsigned char addr,unsigned int value)
{
	int     i;
	char    bit;
#ifdef CONFIG_IT8712_GPIO
	char    status;
#else
    int     ad1;
#endif

#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif
	SPI_CS_enable(1);

	SPI_write_bit(0);       //dummy clock

	//send write command (0x05)
	for(i=SPI_OP_LEN-1;i>=0;i--)
	{
		bit = (SPI_OPWRITE>>i)& 0x01;
		SPI_write_bit(bit);
	}
	// send 8 bits address (MSB first, LSB last)
	for(i=SPI_ADD_LEN-1;i>=0;i--)
	{
		bit = (addr>>i)& 0x01;
		SPI_write_bit(bit);
	}
	// send 16 bits data (MSB first, LSB last)
	for(i=SPI_DAT_LEN-1;i>=0;i--)
	{
		bit = (value>>i)& 0x01;
		SPI_write_bit(bit);
	}

	SPI_CS_enable(0);	// CS low

	for(i=0;i<0xFFF;i++) ;
#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif
}


/************************************
* SPI_write_bit
* bit_EEDO -> 1 or 0 to be written
************************************/
void SPI_write_bit(char bit_EEDO)
{
#ifdef CONFIG_IT8712_GPIO
	unsigned char iomode,status;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode |= (ADM_EECK|ADM_EDIO|ADM_EECS) ;				// Set EECK,EDIO,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	if(bit_EEDO)
	{
		status = inb_gpio( LPC_GPIO_SET);
		status |= ADM_EDIO ;		//EDIO high
		outb_gpio(LPC_GPIO_SET, status);
	}
	else
	{
		status = inb_gpio( LPC_GPIO_SET);
		status &= ~(ADM_EDIO) ;		//EDIO low
		outb_gpio(LPC_GPIO_SET, status);
	}

	status |= ADM_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(ADM_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

#else
	unsigned int addr;
	unsigned int value;

	addr = (GPIO_BASE_ADDR + GPIO_PIN_DIR);
	value = readl(addr) |GPIO_EECK |GPIO_MISO ;   /* set EECK/MISO Pin to output */
	writel(value,addr);
	if(bit_EEDO)
	{
		addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
		writel(GPIO_MISO,addr); /* set MISO to 1 */
		writel(GPIO_EECK,addr); /* set EECK to 1 */
		addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
		writel(GPIO_EECK,addr); /* set EECK to 0 */
	}
	else
	{
		addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
		writel(GPIO_MISO,addr); /* set MISO to 0 */
		addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
		writel(GPIO_EECK,addr); /* set EECK to 1 */
		addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
		writel(GPIO_EECK,addr); /* set EECK to 0 */
	}

	return ;
#endif
}

/**********************************************************************
* read a bit from ADM6996 register
***********************************************************************/
unsigned int SPI_read_bit(void) // read data from
{
#ifdef CONFIG_IT8712_GPIO
	unsigned char iomode,status;
	unsigned int value ;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode &= ~(ADM_EDIO) ;		// Set EDIO input
	iomode |= (ADM_EECS|ADM_EECK) ;		// Set EECK,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	status = inb_gpio( LPC_GPIO_SET);
	status |= ADM_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(ADM_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

	value = inb_gpio( LPC_GPIO_SET);

	value = value>>2 ;
	value &= 0x01;

	return value ;
#else
	unsigned int addr;
	unsigned int value;

	addr = (GPIO_BASE_ADDR + GPIO_PIN_DIR);
	value = readl(addr) & (~GPIO_MISO);   // set EECK to output and MISO to input
	writel(value,addr);

	addr =(GPIO_BASE_ADDR + GPIO_DATA_SET);
	writel(GPIO_EECK,addr); // set EECK to 1
	addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_EECK,addr); // set EECK to 0

	addr = (GPIO_BASE_ADDR + GPIO_DATA_IN);
	value = readl(addr) ;
	value = value >> 30;
	return value ;
#endif
}

/******************************************
* SPI_default
* EEPROM content default value
*******************************************/
void SPI_default(void)
{
	int i;
#ifdef CONFIG_ADM_6999
	SPI_write(0x11,0xFF30);
	for(i=1;i<8;i++)
		SPI_write(i,0x840F);

	SPI_write(0x08,0x880F);			//port 8 Untag, PVID=2
	SPI_write(0x09,0x881D);			//port 9 Tag, PVID=2 ,10M
	SPI_write(0x14,0x017F);			//Group 0~6,8 as VLAN 1
	SPI_write(0x15,0x0180);			//Group 7,8 as VLAN 2
#endif

#ifdef CONFIG_ADM_6996
	SPI_write(0x11,0xFF30);
	SPI_write(0x01,0x840F);			//port 0~3 Untag ,PVID=1 ,100M ,duplex
	SPI_write(0x03,0x840F);
	SPI_write(0x05,0x840F);
	SPI_write(0x07,0x840F);
	SPI_write(0x08,0x880F);			//port 4 Untag, PVID=2
	SPI_write(0x09,0x881D);			//port 5 Tag, PVID=2 ,10M
	SPI_write(0x14,0x0155);			//Group 0~3,5 as VLAN 1
	SPI_write(0x15,0x0180);			//Group 4,5 as VLAN 2

#endif

	for(i=0x16;i<=0x22;i++)
		SPI_write((unsigned char)i,0x0000);		// clean VLAN¡@map 3~15

	for (i=0;i<NUM_VLAN_IF;i++) 				// Set VLAN ID map 1,2
		SPI_Set_PVID( VLAN_conf[i].vid,  VLAN_conf[i].portmap);

	for(i=0;i<ADM699X_PORT_NO;i++)				// reset count
		SPI_reset(0,i);
}

/*************************************************
* SPI_reset
* rstype -> reset type
*	    0:reset all count for 'port_cnt' port
*	    1:reset specified count 'port_cnt'
* port_cnt   ->  port number or counter index
***************************************************/
void SPI_reset(unsigned char rstype,unsigned char port_cnt)
{

	int i;
#ifdef CONFIG_IT8712_GPIO
    char status;
#else
	int ad1;
#endif
	char bit;

#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif

	SPI_CS_enable(0);	// CS low

	SPI_pre_st(); // PRE+ST
	SPI_write_bit(0); // OP
	SPI_write_bit(1);

	SPI_write_bit(1);		// Table select, must be 1 -> reset Counter

	SPI_write_bit(0);		// Device Address
	SPI_write_bit(0);

	rstype &= 0x01;
	SPI_write_bit(rstype);		// Reset type 0:clear dedicate port's all counters 1:clear dedicate counter

	for (i=5;i>=0;i--) 		// port or cnt index
	{
		bit = port_cnt >> i ;
		bit &= 0x01 ;
		SPI_write_bit(bit);
	}

	SPI_write_bit(0); 		// dumy clock
	SPI_write_bit(0); 		// dumy clock

#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif
}

/*****************************************************
* SPI_pre_st
* preambler: 32 bits '1'   start bit: '01'
*****************************************************/
void SPI_pre_st(void)
{
	int i;

	for(i=0;i<32;i++) // PREAMBLE
		SPI_write_bit(1);
	SPI_write_bit(0); // ST
	SPI_write_bit(1);
}


/***********************************************************
* SPI_CS_enable
* before access ,you have to enable Chip Select. (pull high)
* When fisish, you should pull low !!
*************************************************************/
void SPI_CS_enable(unsigned char enable)
{
#ifdef CONFIG_IT8712_GPIO

	unsigned char iomode,status;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode |= (ADM_EECK|ADM_EDIO|ADM_EECS) ;				// Set EECK,EDIO,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);


	status = inb_gpio( LPC_GPIO_SET);
	if(enable)
		status |= ADM_EECS ;		//EECS high
	else
		status &= ~(ADM_EECS) ;	//EECS low

	outb_gpio(LPC_GPIO_SET, status);


	status |= ADM_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(ADM_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

#else
	unsigned int addr,value;

	addr = (GPIO_BASE_ADDR + GPIO_PIN_DIR);
	value = readl(addr) |GPIO_EECS |GPIO_EECK;   /* set EECS/EECK Pin to output */
	writel(value,addr);

	if(enable)
	{
		addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
		writel(GPIO_EECS,addr); /* set EECS to 1 */

	}
	else
	{
		addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
		writel(GPIO_EECS,addr); /* set EECS to 0 */
		addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
		writel(GPIO_EECK,addr); /* set EECK to 1 */	// at least one clock after CS low
		addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
		writel(GPIO_EECK,addr); /* set EECK to 0 */
	}
#endif
}

/*********************************************************
* SPI_Set_VLAN: group ports as VLAN
* LAN  -> VLAN number : 0~16
* port_mask -> ports which would group as LAN
* 	       ex. 0x03 = 0000 0011
*			port 0 and port 1
*********************************************************/
void SPI_Set_VLAN(unsigned char LAN,unsigned int port_mask)
{
	unsigned int i,value=0;
	unsigned reg_add = 0x13 + LAN ;

	for(i=0;i<ADM6996_PORT_NO;i++)
	{	if(port_mask&0x01)
		{
			switch(i)
			{
				case 0: value|=0x0001;	break;	//port0:bit[0]
				case 1: value|=0x0004;	break;  //port1:bit[2]
				case 2: value|=0x0010;	break;  //port2:bit[4]
				case 3: value|=0x0040;	break;  //port3:bit[6]
				case 4: value|=0x0080;	break;  //port4:bit[7]
				case 5: value|=0x0100;	break;  //port5:bit[8]
			}
		}
		port_mask >>= 1;
	}

	SPI_write(reg_add,value);
}


/*******************************************
* SPI_Set_tag
* port -> port number to set tag or untag
* tag  -> 0/set untag,  1/set tag
* In general, tag is for MII port. LAN and
* WAN port is configed as untag!!
********************************************/
void SPI_Set_tag(unsigned int port,unsigned tag)
{
	unsigned int regadd,value;

	// mapping port's register !! (0,1,2,3,4,5) ==> (1,3,5,7,8,9)
	if(port<=3)
		regadd=2*port+1;
	else if(port==4) regadd = 8 ;
	else regadd = 9 ;


	value = SPI_read(0,regadd);		//read original setting

	if(tag)
		value |= 0x0010 ;		// set tag
	else
		value &= 0xFFEF ;		// set untag

	SPI_write(regadd,value);		// write back!!
}

/************************************************
* SPI_Set_PVID
* PVID -> PVID number :
* port_mask -> ports which would group as LAN
* 	       ex. 0x0F = 0000 1111 ==> port 0~3
************************************************/
void SPI_Set_PVID(unsigned int PVID,unsigned int port_mask)
{
	unsigned int i,value=0;

	PVID &= 0x000F ;

	for(i=0;i<ADM699X_PORT_NO;i++)
	{	if(port_mask&0x01)
		{
#ifdef CONFIG_ADM_6996
			switch(i)
			{
				case 0:
					value = SPI_read(0,0x01);	// read original value
					value &= 0xC3FF ;			//set PVIC column as 0 first
					value |= PVID << 10 ;		//Set PVID column as PVID
					SPI_write(0x01,value);		//write back
					break;
				case 1:
					value = SPI_read(0,0x03);
					value &= 0xC3FF ;
					value |= PVID << 10 ;
					SPI_write(0x03,value);
					break;
				case 2:
					value = SPI_read(0,0x05);
					value &= 0xC3FF ;
					value |= PVID << 10 ;
					SPI_write(0x05,value);
					break;
				case 3:
					value = SPI_read(0,0x07);
					value &= 0xC3FF ;
					value |= PVID << 10 ;
					SPI_write(0x07,value);
					break;
				case 4:
					value = SPI_read(0,0x08);
					value &= 0xC3FF ;
					value |= PVID << 10 ;
					SPI_write(0x08,value);
					break;
				case 5:
					value = SPI_read(0,0x09);
					value &= 0xC3FF ;
					value |= PVID << 10 ;
					SPI_write(0x09,value);
					break;
			}
#endif
#ifdef CONFIG_ADM_6999
			value = SPI_read(0,(unsigned char)i+1);
			value &= 0xC3FF ;
			value |= PVID << 10 ;
			SPI_write((unsigned char)i+1,value);
#endif
		}
		port_mask >>= 1;
	}
}


/************************************************
* SPI_get_PVID
* port -> which ports to VID
************************************************/
unsigned int SPI_Get_PVID(unsigned int port)
{
	unsigned int value=0;

	if (port>=ADM6996_PORT_NO)
		return 0;

	switch(port)
	{
		case 0:
			value = SPI_read(0,0x01);	// read original value
			value &= 0x3C00 ;		// get VID
			value = value >> 10 ;		// Shift
			break;
		case 1:
			value = SPI_read(0,0x03);
			value &= 0x3C00 ;
			value = value >> 10 ;
			break;
		case 2:
			value = SPI_read(0,0x05);
			value &= 0x3C00 ;
			value = value >> 10 ;
			break;
		case 3:
			value = SPI_read(0,0x07);
			value &= 0x3C00 ;
			value = value >> 10 ;
			break;
		case 4:
			value = SPI_read(0,0x08);
			value &= 0x3C00 ;
			value = value >> 10 ;
			break;
		case 5:
			value = SPI_read(0,0x09);
			value &= 0x3C00 ;
			value = value >> 10 ;
			break;
	}
	return value ;
}


/**********************************************
* SPI_mac_clone
* port -> the port which will lock or unlock
* lock -> 0/the port will be unlock
*	  1/the port will be locked
**********************************************/
void SPI_mac_lock(unsigned int port, unsigned char lock)
{
	unsigned int i,value=0;

	value = SPI_read(0,0x12);		// read original

	for(i=0;i<ADM6996_PORT_NO;i++)
	{	if(lock)				// lock port
		{
			switch(port)
			{
				case 0: value|=0x0001;	break;	//port0:bit[0]
				case 1: value|=0x0004;	break;	//port1:bit[2]
				case 2: value|=0x0010;	break;	//port2:bit[4]
				case 3: value|=0x0040;	break;	//port3:bit[6]
				case 4: value|=0x0080;	break;	//port4:bit[7]
				case 5: value|=0x0100;	break;	//port5:bit[8]
			}
		}
		else
		{
			switch(i)			// unlock port
			{
				case 0: value&=0xFFFE;	break;
				case 1: value&=0xFFFB;	break;
				case 2: value&=0xFFEF;	break;
				case 3: value&=0xFFBF;	break;
				case 4: value&=0xFF7F;	break;
				case 5: value&=0xFEFF;	break;
			}
		}
	}

	SPI_write(0x12,value);
}


/***************************************************
* SPI_learn_pause
* pause = 01-80-c2-00-00-01
* DA=distination address
* forward -> 0: if DA == pause then drop and stop mac learning
*	     1: if DA == pause ,then forward it
***************************************************/
void SPI_pause_cmd_forward(unsigned char forward)
{
	unsigned int value=0;

	value = SPI_read(0,0x2C);		// read original setting
	if(forward)
		value |= 0x2000;		// set bit[13] '1'
	else
		value &= 0xDFFF;		// set bit[13] '0'

	SPI_write(0x2C,value);

}


/************************************************
* SPI_read
* table -> which table to be read: 1/count  0/EEPROM
* addr  -> Address to be read
* return : Value of the register
*************************************************/
unsigned int SPI_read(unsigned char table,unsigned char addr)
{
	int i ;
	unsigned int value=0;
	unsigned int bit;
#ifdef CONFIG_IT8712_GPIO
	unsigned char status;
#else
    unsigned int ad1;
#endif

#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif

	SPI_CS_enable(0);

	SPI_pre_st(); // PRE+ST
	SPI_write_bit(1); // OPCODE '10' for read
	SPI_write_bit(0);

	(table==1) ? SPI_write_bit(1) : SPI_write_bit(0) ;	// table select

	SPI_write_bit(0);		// Device Address
	SPI_write_bit(0);


	// send 7 bits address to be read
	for (i=6;i>=0;i--) {
		bit= ((addr>>i) & 0x01) ? 1 :0 ;
		SPI_write_bit(bit);
	}


	// turn around
	SPI_read_bit(); // TA_Z

	value=0;
	for (i=31;i>=0;i--) { // READ DATA
		bit=SPI_read_bit();
		value |= bit << i ;
	}

	SPI_read_bit(); // dumy clock
	SPI_read_bit(); // dumy clock

	if(!table)					// EEPROM, only fetch 16 bits data
	{
	    if(addr&0x01)				// odd number content (register,register-1)
		    value >>= 16 ;			// so we remove the rear 16bits
	    else					// even number content (register+1,register),
		    value &= 0x0000FFFF ;		// so we keep the rear 16 bits
	}


	SPI_CS_enable(0);

#ifdef CONFIG_IT8712_GPIO
	status = inb_gpio(LPC_GPIO_SET);
	status &= ~(ADM_EDIO) ;		//EDIO low
	outb_gpio(LPC_GPIO_SET, status);
#else
   	ad1 = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
	writel(GPIO_MISO,ad1); /* set MISO to 0 */
#endif

	return(value);

}



/**************************************************
* SPI_port_en
* port -> Number of port to config
* enable -> 1/ enable this port
*	    0/ disable this port
**************************************************/
void SPI_port_enable(unsigned int port,unsigned char enable)
{
	unsigned int reg_val ;
	unsigned char reg_add ;

	if(port<=3)
		reg_add=2*port+1;
	else if(port==4) reg_add = 8 ;
	else reg_add = 9 ;

	reg_val = SPI_read(0,reg_add);
	if(enable)
	{
		reg_val &= 0xFFDF ;
		SPI_write(reg_add,reg_val);
	}
	else
	{
		reg_val |= 0x0020 ;
		SPI_write(reg_add,reg_val);
	}
}

/********************************************************
* get port status
* port -> specify the port number to get configuration
*********************************************************/
void SPI_get_status(unsigned int port)
{
/*	unsigned int reg_val,add_offset[6];
	struct PORT_STATUS *status;
	status = &port_state[port];

	if(port>(ADM6996_PORT_NO-1))
		return ;

	// Link estabilish , speed, deplex, flow control ?
	if(port < 5 )
	{
		reg_val = SPI_read(1, 1) ;
		if(port < 4)
			reg_val >>= port*8 ;
		else
			reg_val >>=28 ;
		status->link = reg_val & 0x00000001 ;
		status->speed = reg_val  & 0x00000002 ;
		status->duplex = reg_val & 0x00000004 ;
		status->flow_ctl = reg_val & 0x00000008 ;
	}
	else if(port ==5 )
	{
		reg_val = SPI_read(1, 2) ;
		status->link = reg_val & 0x00000001 ;
		status->speed = reg_val  & 0x00000002 ;
		status->duplex = reg_val & 0x00000008 ;
		status->flow_ctl = reg_val & 0x00000010 ;
	}

	//   Mac Lock ?
	reg_val = SPI_read(0,0x12);
	switch(port)
	{
		case 0:	status->mac_lock = reg_val & 0x00000001;
		case 1:	status->mac_lock = reg_val & 0x00000004;
		case 2:	status->mac_lock = reg_val & 0x00000010;
		case 3:	status->mac_lock = reg_val & 0x00000040;
		case 4:	status->mac_lock = reg_val & 0x00000080;
		case 5:	status->mac_lock = reg_val & 0x00000100;
	}

	// port enable ?
	add_offset[0] = 0x01 ;		add_offset[1] = 0x03 ;
	add_offset[2] = 0x05 ;		add_offset[3] = 0x07 ;
	add_offset[4] = 0x08 ;		add_offset[5] = 0x09 ;
	reg_val = SPI_read(0,add_offset[port]);
	status->port_disable = reg_val & 0x0020;


	//  Packet Count ...
	add_offset[0] = 0x04 ;		add_offset[1] = 0x06 ;
	add_offset[2] = 0x08 ;		add_offset[3] = 0x0a ;
	add_offset[4] = 0x0b ;		add_offset[5] = 0x0c ;

	reg_val = SPI_read(1,add_offset[port]);
	status->rx_pac_count = reg_val ;
	reg_val = SPI_read(1,add_offset[port]+9);
	status->rx_pac_byte = reg_val ;
	reg_val = SPI_read(1,add_offset[port]+18);
	status->tx_pac_count = reg_val ;
	reg_val = SPI_read(1,add_offset[port]+27);
	status->tx_pac_byte = reg_val ;
	reg_val = SPI_read(1,add_offset[port]+36);
	status->collision_count = reg_val ;
	reg_val = SPI_read(1,add_offset[port]+45);
	status->error_count = reg_val ;
	reg_val = SPI_read(1, 0x3A);
	switch(port)
	{
		case 0:	status->rx_pac_count_overflow = reg_val & 0x00000001;
			status->rx_pac_byte_overflow = reg_val & 0x00000200 ;
		case 1:	status->rx_pac_count_overflow = reg_val & 0x00000004;
			status->rx_pac_byte_overflow = reg_val & 0x00000800 ;
		case 2:	status->rx_pac_count_overflow = reg_val & 0x00000010;
			status->rx_pac_byte_overflow = reg_val & 0x00002000 ;
		case 3:	status->rx_pac_count_overflow = reg_val & 0x00000040;;
			status->rx_pac_byte_overflow = reg_val & 0x00008000 ;
		case 4:	status->rx_pac_count_overflow = reg_val & 0x00000080;
			status->rx_pac_byte_overflow = reg_val & 0x00010000 ;
		case 5:	status->rx_pac_count_overflow = reg_val & 0x00000100;
			status->rx_pac_byte_overflow = reg_val & 0x00020000 ;
	}

	reg_val = SPI_read(1, 0x3B);
	switch(port)
	{
		case 0:	status->tx_pac_count_overflow = reg_val & 0x00000001;
			status->tx_pac_byte_overflow  = reg_val & 0x00000200 ;
		case 1:	status->tx_pac_count_overflow  = reg_val & 0x00000004;
			status->tx_pac_byte_overflow  = reg_val & 0x00000800 ;
		case 2:	status->tx_pac_count_overflow  = reg_val & 0x00000010;
			status->tx_pac_byte_overflow  = reg_val & 0x00002000 ;
		case 3:	status->tx_pac_count_overflow  = reg_val & 0x00000040;;
			status->tx_pac_byte_overflow  = reg_val & 0x00008000 ;
		case 4:	status->tx_pac_count_overflow  = reg_val & 0x00000080;
			status->tx_pac_byte_overflow  = reg_val & 0x00010000 ;
		case 5:	status->tx_pac_count_overflow  = reg_val & 0x00000100;
			status->tx_pac_byte_overflow  = reg_val & 0x00020000 ;
	}
*/

	unsigned int reg_val;
	struct PORT_STATUS *status;
	status = &port_state[port];

	if(port>=ADM6999_PORT_NO)
		return ;

	// Link estabilish , speed, deplex, flow control ?
	if(port < ADM6999_PORT_NO-1 )
	{
		reg_val = SPI_read(1, 0x01) ;
		reg_val = reg_val >> port*4 ;
		status->link = reg_val & 0x00000001 ;
		status->speed = reg_val  & 0x00000002 ;
		status->duplex = reg_val & 0x00000004 ;
		status->flow_ctl = reg_val & 0x00000008 ;
	}
	else if(port == (ADM6999_PORT_NO-1) )
	{
		reg_val = SPI_read(1, 0x02) ;
		status->link = reg_val & 0x00000001 ;
		status->speed = reg_val  & 0x00000002 ;
		status->duplex = reg_val & 0x00000008 ;
		status->flow_ctl = reg_val & 0x00000010 ;
	}

	// Mac Lock ?
	reg_val = SPI_read(0,0x12);
	reg_val = reg_val >> port ;
	reg_val = reg_val & 0x01 ;
	status->mac_lock = reg_val ? 0x01:0x00 ;

	// port enable ?
	reg_val = SPI_read(0,(unsigned char)port+1);
	status->port_disable = reg_val & 0x0020;

	//  Packet Count ...
	reg_val = SPI_read(1,(unsigned char)port+0x04);
	status->rx_pac_count = reg_val ;
	reg_val = SPI_read(1,(unsigned char)port+0x0D);
	status->rx_pac_byte = reg_val ;
	reg_val = SPI_read(1,(unsigned char)port+0x16);
	status->tx_pac_count = reg_val ;
	reg_val = SPI_read(1,(unsigned char)port+0x1F);
	status->tx_pac_byte = reg_val ;
	reg_val = SPI_read(1,(unsigned char)port+0x28);
	status->collision_count = reg_val ;
	reg_val = SPI_read(1,(unsigned char)port+0x31);
	status->error_count = reg_val ;
	reg_val = SPI_read(1, 0x3A);
	reg_val = reg_val >> port ;
	status->rx_pac_count_overflow = reg_val & 0x00000001;
	reg_val = reg_val >> 0x09 ;
	status->rx_pac_byte_overflow = reg_val & 0x00000001 ;

	reg_val = SPI_read(1, 0x3B);
	reg_val = reg_val >> port ;
	status->tx_pac_count_overflow = reg_val & 0x00000001;
	reg_val = reg_val >> 0x09 ;
	status->tx_pac_byte_overflow  = reg_val & 0x00000001 ;

	reg_val = SPI_read(1, 0x3C);
	reg_val = reg_val >> port ;
	status->collision_count_overflow = reg_val & 0x00000001;
	reg_val = reg_val >> 0x09 ;
	status->error_count_overflow  = reg_val & 0x00000001 ;

}

unsigned int SPI_get_identifier(void)
{
	unsigned int flag=0;

#ifdef CONFIG_IT8712_GPIO

	if (!it8712_exist) {
		return -ENODEV;
	}
	printk("it8712_gpio init\n");

	/* initialize registers */
	// switch all multi-function pins to GPIO
	LPCSetConfig(LDN_GPIO, 0x28, 0xff);

	// set simple I/O base address
	LPCSetConfig(LDN_GPIO, 0x62, IT8712_GPIO_BASE >> 8);
	LPCSetConfig(LDN_GPIO, 0x63, (unsigned char) IT8712_GPIO_BASE >> 8);

	// select GPIO to simple I/O
	LPCSetConfig(LDN_GPIO, 0xc3, 0xff);

	// enable internal pull-up
	LPCSetConfig(LDN_GPIO, 0xbb, 0xff);

#endif

	flag = SPI_read(1,0x00);
	printk("Get ADM identifier %6x\n",flag);
	if ((flag & 0xFFFF0) == 0x21120) {
		printk("ADM699X Found\n");
		return 1;
	}
	else {
		printk("ADM699X not Found\n");
		return 0;
	}
}

