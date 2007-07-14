/*
 * include/asm-arm/arch-ixp4xx/platform.h
 *
 * Constants and functions that are useful to IXP4xx platform-specific code
 * and device drivers.
 *
 * Copyright (C) 2004 MontaVista Software, Inc.
 */

#ifndef __ASM_ARCH_HARDWARE_H__
#error "Do not include this directly, instead #include <asm/hardware.h>"
#endif

#ifndef __ASSEMBLY__

#include <asm/types.h>

#ifndef	__ARMEB__
#define	REG_OFFSET	0
#else
#define	REG_OFFSET	3
#endif

/*
 * Expansion bus memory regions
 */
#define IXP4XX_EXP_BUS_BASE_PHYS	(0x50000000)

#define	IXP4XX_EXP_BUS_CSX_REGION_SIZE	(0x01000000)

#define IXP4XX_EXP_BUS_CS0_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x00000000)
#define IXP4XX_EXP_BUS_CS1_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x01000000)
#define IXP4XX_EXP_BUS_CS2_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x02000000)
#define IXP4XX_EXP_BUS_CS3_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x03000000)
#define IXP4XX_EXP_BUS_CS4_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x04000000)
#define IXP4XX_EXP_BUS_CS5_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x05000000)
#define IXP4XX_EXP_BUS_CS6_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x06000000)
#define IXP4XX_EXP_BUS_CS7_BASE_PHYS	(IXP4XX_EXP_BUS_BASE_PHYS + 0x07000000)

/*
 * The expansion bus on the IXP4xx can be configured for either 16 or
 * 32MB windows and the CS offset for each region changes based on the
 * current configuration. This means that we cannot simply hardcode
 * each offset. ixp4xx_sys_init() looks at the expansion bus configuration
 * as setup by the bootloader to determine our window size.
 */
extern unsigned long ixp4xx_exp_bus_size;

#define	IXP4XX_EXP_BUS_BASE(region)\
		(IXP4XX_EXP_BUS_BASE_PHYS + ((region) * ixp4xx_exp_bus_size))

#define IXP4XX_EXP_BUS_END(region)\
		(IXP4XX_EXP_BUS_BASE(region) + ixp4xx_exp_bus_size - 1)

/* Those macros can be used to adjust timing and configure
 * other features for each region.
 */

#define IXP4XX_EXP_BUS_RECOVERY_T(x)	(((x) & 0x0f) << 16)
#define IXP4XX_EXP_BUS_HOLD_T(x)	(((x) & 0x03) << 20)
#define IXP4XX_EXP_BUS_STROBE_T(x)	(((x) & 0x0f) << 22)
#define IXP4XX_EXP_BUS_SETUP_T(x)	(((x) & 0x03) << 26)
#define IXP4XX_EXP_BUS_ADDR_T(x)	(((x) & 0x03) << 28)
#define IXP4XX_EXP_BUS_SIZE(x)		(((x) & 0x0f) << 10)
#define IXP4XX_EXP_BUS_CYCLES(x)	(((x) & 0x03) << 14)

#define IXP4XX_EXP_BUS_CS_EN		(1L << 31)
#define IXP4XX_EXP_BUS_BYTE_RD16	(1L << 6)
#define IXP4XX_EXP_BUS_HRDY_POL		(1L << 5)
#define IXP4XX_EXP_BUS_MUX_EN		(1L << 4)
#define IXP4XX_EXP_BUS_SPLT_EN		(1L << 3)
#define IXP4XX_EXP_BUS_WR_EN		(1L << 1)
#define IXP4XX_EXP_BUS_BYTE_EN		(1L << 0)

#define IXP4XX_EXP_BUS_CYCLES_INTEL	0x00
#define IXP4XX_EXP_BUS_CYCLES_MOTOROLA	0x01
#define IXP4XX_EXP_BUS_CYCLES_HPI	0x02

#define IXP4XX_FLASH_WRITABLE	(0x2)
#define IXP4XX_FLASH_DEFAULT	(0xbcd23c40)
#define IXP4XX_FLASH_WRITE	(0xbcd23c42)

/*
 * Clock Speed Definitions.
 */
#define IXP4XX_PERIPHERAL_BUS_CLOCK 	(66) /* 66Mhzi APB BUS   */ 
#define IXP4XX_UART_XTAL        	14745600

struct npe_plat_data {
	const char *name;
	int data_size;
	int inst_size;
	int id;		/* Node ID */
};

struct mac_plat_info {
	int npe_id;	/* Node ID of the NPE for this port */
	int port_id;	/* Port ID for NPE-B @ ixp465 */
	int eth_id;	/* Physical ID */
	int phy_id;	/* ID of the connected PHY (PCB/platform dependent) */
	int rxq_id;	/* Queue ID of the RX-free q */
	int rxdoneq_id; /* where incoming packets are returned */
	int txq_id;	/* Where to push the outgoing packets */
	unsigned char hwaddr[6]; /* Desired hardware address */

};

/*
 * The IXP4xx chips do not have an I2C unit, so GPIO lines are just
 * used to 
 * Used as platform_data to provide GPIO pin information to the ixp42x
 * I2C driver.
 */
struct ixp4xx_i2c_pins {
	unsigned long sda_pin;
	unsigned long scl_pin;
};

#define IXDP425_KSSPI_SELECT	4
#define IXDP425_KSSPI_TXD	3
#define IXDP425_KSSPI_CLOCK	2
#define IXDP425_KSSPI_RXD	0

/*
 * This structure provide a means for the board setup code
 * to give information to th pata_ixp4xx driver. It is
 * passed as platform_data.
 */
struct ixp4xx_spi_pins {
  unsigned long spis_pin;
  unsigned long spic_pin;
  unsigned long spid_pin;
  unsigned long spiq_pin;
};


struct ixp4xx_pata_data {
	volatile u32	*cs0_cfg;
	volatile u32	*cs1_cfg;
	unsigned long	cs0_bits;
	unsigned long	cs1_bits;
	void __iomem	*cs0;
	void __iomem	*cs1;
};

struct sys_timer;

/*
 * Frequency of clock used for primary clocksource
 */
extern unsigned long ixp4xx_timer_freq;

/*
 * Functions used by platform-level setup code
 */
extern void ixp4xx_map_io(void);
extern void ixp4xx_init_irq(void);
extern void ixp4xx_sys_init(void);
extern void ixp4xx_timer_init(void);
extern struct sys_timer ixp4xx_timer;
extern void ixp4xx_pci_preinit(void);
struct pci_sys_data;
extern int ixp4xx_setup(int nr, struct pci_sys_data *sys);
extern struct pci_bus *ixp4xx_scan_bus(int nr, struct pci_sys_data *sys);

/*
 * GPIO-functions
 */
/*
 * The following converted to the real HW bits the gpio_line_config
 */
/* GPIO pin types */
#define IXP4XX_GPIO_OUT 		0x1
#define IXP4XX_GPIO_IN  		0x2

/* GPIO signal types */
#define IXP4XX_GPIO_LOW			0
#define IXP4XX_GPIO_HIGH		1

/* GPIO Clocks */
#define IXP4XX_GPIO_CLK_0		14
#define IXP4XX_GPIO_CLK_1		15

static inline void gpio_line_config(u8 line, u32 direction)
{
	if (direction == IXP4XX_GPIO_IN)
		*IXP4XX_GPIO_GPOER |= (1 << line);
	else
		*IXP4XX_GPIO_GPOER &= ~(1 << line);
}

static inline void gpio_line_get(u8 line, int *value)
{
	*value = (*IXP4XX_GPIO_GPINR >> line) & 0x1;
}

static inline void gpio_line_set(u8 line, int value)
{
	if (value == IXP4XX_GPIO_HIGH)
	    *IXP4XX_GPIO_GPOUTR |= (1 << line);
	else if (value == IXP4XX_GPIO_LOW)
	    *IXP4XX_GPIO_GPOUTR &= ~(1 << line);
}
static inline void gpio_line_isr_clear(u8 line)
{
	*IXP4XX_GPIO_GPISR = (1 << line);
}

#endif // __ASSEMBLY__

