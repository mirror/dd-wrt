#ifndef RA2882ETHEND_H
#define RA2882ETHEND_H

#ifdef DSP_VIA_NONCACHEABLE
#define ESRAM_BASE	0xa0800000	/* 0x0080-0000  ~ 0x00807FFF */
#else
#define ESRAM_BASE	0x80800000	/* 0x0080-0000  ~ 0x00807FFF */
#endif

#define RX_RING_BASE	((int)(ESRAM_BASE + 0x7000))
#define TX_RING_BASE	((int)(ESRAM_BASE + 0x7800))

#if defined(CONFIG_RALINK_RT2880)
#define NUM_TX_RINGS 	1
#else
#define NUM_TX_RINGS 	4
#endif
#ifdef MEMORY_OPTIMIZATION
#ifdef CONFIG_RAETH_ROUTER
#define NUM_RX_DESC     128
#define NUM_TX_DESC    	128
#elif CONFIG_RT_3052_ESW
#define NUM_RX_DESC     64
#define NUM_TX_DESC     64
#else
#define NUM_RX_DESC     128
#define NUM_TX_DESC     128
#endif
//#define NUM_RX_MAX_PROCESS 32
#define NUM_RX_MAX_PROCESS 64
#else
#ifdef CONFIG_RAETH_ROUTER
#define NUM_RX_DESC     256
#define NUM_TX_DESC    	256
#elif CONFIG_RT_3052_ESW
#define NUM_RX_DESC     256
#define NUM_TX_DESC     256
#else
#define NUM_RX_DESC     256
#define NUM_TX_DESC     256
#endif
#define NUM_RX_MAX_PROCESS 16
#endif

#define DEV_NAME        "eth2"
#define RA2882_ENET0	RA2882ETH_BASE

#if defined (CONFIG_RT2880_FLASH_32M) && defined (CONFIG_RALINK_RT3052_MP2)
#define RT_EEPROM_BASE  0xBBFE0000
#else
#define RT_EEPROM_BASE  (CONFIG_MTD_PHYSMAP_START + 0x40000)
#endif

#define GMAC2_ADDR      (RT_EEPROM_BASE + 0x22)
#define GMAC0_ADDR      (RT_EEPROM_BASE + 0x28) 
#define GMAC1_ADDR      (RT_EEPROM_BASE + 0x2E)

#define RT2880_IRQ_ENET0	3 	/* hardware interrupt #3, defined in RT2880 Soc Design Spec Rev 0.03, pp43 */

#define RT2880_FE_INT_STATUS_REG (*(volatile unsigned long *)(FE_INT_STATUS))
#define RT2880_FE_INT_STATUS_CLEAN(reg) (*(volatile unsigned long *)(FE_INT_STATUS)) = reg

//#define RAETH_DEBUG
#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

struct net_device_stats *ra_get_stats(struct net_device *dev);

void ei_tx_timeout(struct net_device *dev);
int rather_probe(struct net_device *dev);
int ei_open(struct net_device *dev);
int ei_close(struct net_device *dev);

int ra2882eth_init(void);
void ra2882eth_cleanup_module(void);

void ei_xmit_housekeeping(unsigned long data);

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

#endif
