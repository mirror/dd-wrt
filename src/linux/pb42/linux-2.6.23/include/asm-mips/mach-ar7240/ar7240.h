#ifndef _AR7240_H
#define _AR7240_H

#include <asm/addrspace.h>

typedef unsigned int ar7240_reg_t;

#define ar7240_reg_rd(_phys)    (*(volatile ar7240_reg_t *)KSEG1ADDR(_phys))
#define ar7240_reg_wr_nf(_phys, _val) \
                    ((*(volatile ar7240_reg_t *)KSEG1ADDR(_phys)) = (_val))

#define ar7240_reg_wr(_phys, _val) do {     \
         ar7240_reg_wr_nf(_phys, _val);     \
         ar7240_reg_rd(_phys);       \
}while(0);

#define ar7240_reg_rmw_set(_reg, _mask)  do {                        \
    ar7240_reg_wr((_reg), (ar7240_reg_rd((_reg)) | (_mask)));      \
    ar7240_reg_rd((_reg));                                           \
}while(0);

#define ar7240_reg_rmw_clear(_reg, _mask)  do {                        \
    ar7240_reg_wr((_reg), (ar7240_reg_rd((_reg)) & ~(_mask)));      \
    ar7240_reg_rd((_reg));                                           \
}while(0);

/*
 * Address map
 */
#define AR7240_PCI_MEM_BASE             0x10000000  /* 128M */
#define AR7240_APB_BASE                 0x18000000  /* 384M */
#define AR7240_GE0_BASE                 0x19000000  /* 16M */
#define AR7240_GE1_BASE                 0x1a000000  /* 16M */
#define AR7240_USB_OHCI_BASE            0x1b000000
#define AR7240_USB_EHCI_BASE            0x1b000000  
#define AR7240_SPI_BASE                 0x1f000000

/* 
 * Added the PCI LCL RESET register from u-boot
 * ar7240_soc.h so that we can query the PCI LCL RESET 
 * register for the presence of WLAN H/W.
 */
#define AR7240_PCI_LCL_BASE             (AR7240_APB_BASE+0x000f0000)
#define AR7240_PCI_LCL_APP              (AR7240_PCI_LCL_BASE+0x00)
#define AR7240_PCI_LCL_RESET            (AR7240_PCI_LCL_BASE+0x18)

/*
 * APB block
 */
#define AR7240_DDR_CTL_BASE             AR7240_APB_BASE+0x00000000
#define AR7240_CPU_BASE                 AR7240_APB_BASE+0x00010000
#define AR7240_UART_BASE                AR7240_APB_BASE+0x00020000
#define AR7240_USB_CONFIG_BASE          AR7240_APB_BASE+0x00030000
#define AR7240_GPIO_BASE                AR7240_APB_BASE+0x00040000
#define AR7240_PLL_BASE                 AR7240_APB_BASE+0x00050000
#define AR7240_RESET_BASE               AR7240_APB_BASE+0x00060000
#define AR7240_SLIC_BASE                AR7240_APB_BASE+0x00090000
#define AR7240_DMA_BASE                 AR7240_APB_BASE+0x000A0000
#define AR7240_STEREO_BASE              AR7240_APB_BASE+0x000B0000
#define AR7240_PCI_CTLR_BASE           AR7240_APB_BASE+0x000F0000

/*
 * DDR block
 */
#define AR7240_DDR_CONFIG               AR7240_DDR_CTL_BASE+0
#define AR7240_DDR_CONFIG2              AR7240_DDR_CTL_BASE+4
#define AR7240_DDR_MODE                 AR7240_DDR_CTL_BASE+0x08
#define AR7240_DDR_EXT_MODE             AR7240_DDR_CTL_BASE+0x0c
#define AR7240_DDR_CONTROL              AR7240_DDR_CTL_BASE+0x10
#define AR7240_DDR_REFRESH              AR7240_DDR_CTL_BASE+0x14
#define AR7240_DDR_RD_DATA_THIS_CYCLE   AR7240_DDR_CTL_BASE+0x18
#define AR7240_DDR_TAP_CONTROL0         AR7240_DDR_CTL_BASE+0x1c
#define AR7240_DDR_TAP_CONTROL1         AR7240_DDR_CTL_BASE+0x20
#define AR7240_DDR_TAP_CONTROL2         AR7240_DDR_CTL_BASE+0x24
#define AR7240_DDR_TAP_CONTROL3         AR7240_DDR_CTL_BASE+0x28

/*
 * DDR Config values
 */
#define AR7240_DDR_CONFIG_16BIT             (1 << 31)
#define AR7240_DDR_CONFIG_PAGE_OPEN         (1 << 30)
#define AR7240_DDR_CONFIG_CAS_LAT_SHIFT      27
#define AR7240_DDR_CONFIG_TMRD_SHIFT         23
#define AR7240_DDR_CONFIG_TRFC_SHIFT         17
#define AR7240_DDR_CONFIG_TRRD_SHIFT         13
#define AR7240_DDR_CONFIG_TRP_SHIFT          9
#define AR7240_DDR_CONFIG_TRCD_SHIFT         5
#define AR7240_DDR_CONFIG_TRAS_SHIFT         0

#define AR7240_DDR_CONFIG2_BL2          (2 << 0)
#define AR7240_DDR_CONFIG2_BL4          (4 << 0)
#define AR7240_DDR_CONFIG2_BL8          (8 << 0)

#define AR7240_DDR_CONFIG2_BT_IL        (1 << 4)       
#define AR7240_DDR_CONFIG2_CNTL_OE_EN   (1 << 5)
#define AR7240_DDR_CONFIG2_PHASE_SEL    (1 << 6)
#define AR7240_DDR_CONFIG2_DRAM_CKE     (1 << 7)
#define AR7240_DDR_CONFIG2_TWR_SHIFT    8
#define AR7240_DDR_CONFIG2_TRTW_SHIFT   12
#define AR7240_DDR_CONFIG2_TRTP_SHIFT   17
#define AR7240_DDR_CONFIG2_TWTR_SHIFT   21
#define AR7240_DDR_CONFIG2_HALF_WIDTH_L (1 << 31)   

#define AR7240_DDR_TAP_DEFAULT          0x18

/*
 * DDR block, gmac flushing 
 */
#define AR7240_DDR_GE0_FLUSH            AR7240_DDR_CTL_BASE+0x7c
#define AR7240_DDR_GE1_FLUSH            AR7240_DDR_CTL_BASE+0x80
#define AR7240_DDR_USB_FLUSH            AR7240_DDR_CTL_BASE+0xa4
#define AR7240_DDR_PCIE_FLUSH           AR7240_DDR_CTL_BASE+0x88

#define AR7240_EEPROM_GE0_MAC_ADDR      0xbfff1000
#define AR7240_EEPROM_GE1_MAC_ADDR      0xbfff1006

/*
 * PLL block/CPU
 */

#define AR7240_PLL_CONFIG               AR7240_PLL_BASE+0x0


#define PLL_DIV_SHIFT   0
#define PLL_DIV_MASK    0x3ff
#define REF_DIV_SHIFT   10
#define REF_DIV_MASK    0xf
#define AHB_DIV_SHIFT   19
#define AHB_DIV_MASK    0x1
#define DDR_DIV_SHIFT   22
#define DDR_DIV_MASK    0x1
#define AR7240_ETH_PLL_CONFIG           AR7240_PLL_BASE+0x4

#define AR7240_ETH_INT0_CLK             AR7240_PLL_BASE+0x14
#define AR7240_ETH_INT1_CLK             AR7240_PLL_BASE+0x18


/*
 * USB block
 */
#define AR7240_USB_FLADJ_VAL            AR7240_USB_CONFIG_BASE
#define AR7240_USB_CONFIG               AR7240_USB_CONFIG_BASE+0x4
#define AR7240_USB_WINDOW               0x1000000
#define AR7240_USB_MODE			AR7240_USB_EHCI_BASE+0x1a8

/*
 * PCI block
 */
#define AR7240_PCI_WINDOW           0x8000000       /* 128MB */
#define AR7240_PCI_WINDOW0_OFFSET   AR7240_DDR_CTL_BASE+0x7c
#define AR7240_PCI_WINDOW1_OFFSET   AR7240_DDR_CTL_BASE+0x80
#define AR7240_PCI_WINDOW2_OFFSET   AR7240_DDR_CTL_BASE+0x84
#define AR7240_PCI_WINDOW3_OFFSET   AR7240_DDR_CTL_BASE+0x88
#define AR7240_PCI_WINDOW4_OFFSET   AR7240_DDR_CTL_BASE+0x8c
#define AR7240_PCI_WINDOW5_OFFSET   AR7240_DDR_CTL_BASE+0x90
#define AR7240_PCI_WINDOW6_OFFSET   AR7240_DDR_CTL_BASE+0x94
#define AR7240_PCI_WINDOW7_OFFSET   AR7240_DDR_CTL_BASE+0x98

#define AR7240_PCI_WINDOW0_VAL      0x10000000
#define AR7240_PCI_WINDOW1_VAL      0x11000000
#define AR7240_PCI_WINDOW2_VAL      0x12000000
#define AR7240_PCI_WINDOW3_VAL      0x13000000
#define AR7240_PCI_WINDOW4_VAL      0x14000000
#define AR7240_PCI_WINDOW5_VAL      0x15000000
#define AR7240_PCI_WINDOW6_VAL      0x16000000
#define AR7240_PCI_WINDOW7_VAL      0x07000000

#define ar7240_write_pci_window(_no)             \
  ar7240_reg_wr(AR7240_PCI_WINDOW##_no##_OFFSET, AR7240_PCI_WINDOW##_no##_VAL);
  
/*
 * CRP. To access the host controller config and status registers
 */
#define AR7240_PCI_CRP   0x180c0000
#define AR7240_PCI_DEV_CFGBASE     0x14000000 
#define AR7240_PCI_CRP_AD_CBE            AR7240_PCI_CRP
#define AR7240_PCI_CRP_WRDATA            AR7240_PCI_CRP+0x4
#define AR7240_PCI_CRP_RDDATA            AR7240_PCI_CRP+0x8
#define AR7240_PCI_ERROR                 AR7240_PCI_CRP+0x1c
#define AR7240_PCI_ERROR_ADDRESS         AR7240_PCI_CRP+0x20
#define AR7240_PCI_AHB_ERROR             AR7240_PCI_CRP+0x24
#define AR7240_PCI_AHB_ERROR_ADDRESS     AR7240_PCI_CRP+0x28

#define AR7240_CRP_CMD_WRITE             0x00010000
#define AR7240_CRP_CMD_READ              0x00000000

/*
 * PCI CFG. To generate config cycles
 */
#define AR7240_PCI_CFG_AD           AR7240_PCI_CRP+0xc
#define AR7240_PCI_CFG_CBE          AR7240_PCI_CRP+0x10
#define AR7240_PCI_CFG_WRDATA       AR7240_PCI_CRP+0x14
#define AR7240_PCI_CFG_RDDATA       AR7240_PCI_CRP+0x18
#define AR7240_CFG_CMD_READ         0x0000000a
#define AR7240_CFG_CMD_WRITE        0x0000000b

#define AR7240_PCI_IDSEL_ADLINE_START           17


/*
 * gpio configs
 */
#define AR7240_GPIO_OE                  AR7240_GPIO_BASE+0x0
#define AR7240_GPIO_IN                  AR7240_GPIO_BASE+0x4
#define AR7240_GPIO_OUT                 AR7240_GPIO_BASE+0x8
#define AR7240_GPIO_SET                 AR7240_GPIO_BASE+0xc
#define AR7240_GPIO_CLEAR               AR7240_GPIO_BASE+0x10
#define AR7240_GPIO_INT_ENABLE          AR7240_GPIO_BASE+0x14
#define AR7240_GPIO_INT_TYPE            AR7240_GPIO_BASE+0x18
#define AR7240_GPIO_INT_POLARITY        AR7240_GPIO_BASE+0x1c
#define AR7240_GPIO_INT_PENDING         AR7240_GPIO_BASE+0x20
#define AR7240_GPIO_INT_MASK            AR7240_GPIO_BASE+0x24
#define AR7240_GPIO_FUNCTIONS           AR7240_GPIO_BASE+0x28

/*
 * IRQ Map.
 * There are 4 conceptual ICs in the system. We generally give a block of 16
 * irqs to each IC.
 * CPU:                     0    - 0xf
 *      MISC:               0x10 - 0x1f
 *          GPIO:           0x20 - 0x2f
 *      PCI :               0x30 - 0x40
 * 
 */
#define AR7240_CPU_IRQ_BASE         0x00
#define AR7240_MISC_IRQ_BASE        0x10
#define AR7240_GPIO_IRQ_BASE        0x20
#define AR7240_PCI_IRQ_BASE         0x30

/*
 * The IPs. Connected to CPU (hardware IP's; the first two are software)
 */
#define AR7240_CPU_IRQ_PCI                  AR7240_CPU_IRQ_BASE+2
#define AR7240_CPU_IRQ_USB                  AR7240_CPU_IRQ_BASE+3
#define AR7240_CPU_IRQ_GE0                  AR7240_CPU_IRQ_BASE+4
#define AR7240_CPU_IRQ_GE1                  AR7240_CPU_IRQ_BASE+5
#define AR7240_CPU_IRQ_MISC                 AR7240_CPU_IRQ_BASE+6
#define AR7240_CPU_IRQ_TIMER                AR7240_CPU_IRQ_BASE+7

/*
 * Interrupts connected to the CPU->Misc line.
 */
#define AR7240_MISC_IRQ_TIMER               AR7240_MISC_IRQ_BASE+0
#define AR7240_MISC_IRQ_ERROR               AR7240_MISC_IRQ_BASE+1
#define AR7240_MISC_IRQ_GPIO                AR7240_MISC_IRQ_BASE+2
#define AR7240_MISC_IRQ_UART                AR7240_MISC_IRQ_BASE+3
#define AR7240_MISC_IRQ_WATCHDOG            AR7240_MISC_IRQ_BASE+4
#define AR7240_MISC_IRQ_PERF_COUNTER        AR7240_MISC_IRQ_BASE+5
#define AR7240_MISC_IRQ_USB_OHCI            AR7240_MISC_IRQ_BASE+6
#define AR7240_MISC_IRQ_DMA                 AR7240_MISC_IRQ_BASE+7
#define AR7240_MISC_IRQ_ENET_LINK           AR7240_MISC_IRQ_BASE+12

#define AR7240_MISC_IRQ_COUNT                 13

#define MIMR_TIMER                          0x01
#define MIMR_ERROR                          0x02
#define MIMR_GPIO                           0x04
#define MIMR_UART                           0x08
#define MIMR_WATCHDOG                       0x10
#define MIMR_PERF_COUNTER                   0x20
#define MIMR_OHCI_USB                       0x40
#define MIMR_DMA                            0x80
#define MIMR_ENET_LINK                      0x1000

#define MISR_TIMER                          MIMR_TIMER
#define MISR_ERROR                          MIMR_ERROR
#define MISR_GPIO                           MIMR_GPIO
#define MISR_UART                           MIMR_UART
#define MISR_WATCHDOG                       MIMR_WATCHDOG
#define MISR_PERF_COUNTER                   MIMR_PERF_COUNTER
#define MISR_OHCI_USB                       MIMR_OHCI_USB
#define MISR_DMA                            MIMR_DMA

/*
 * Interrupts connected to the Misc->GPIO line
 */
#define AR7240_GPIO_IRQn(_gpio)             AR7240_GPIO_IRQ_BASE+(_gpio)
#define AR7240_GPIO_IRQ_COUNT                 16

void ar7240_gpio_irq_init(int irq_base);

void ar7240_misc_enable_irq (unsigned int mask);
void ar7240_misc_disable_irq (unsigned int mask);

unsigned int ar7240_misc_get_irq_mask (void);
unsigned int ar7240_misc_get_irq_status (void);


/* Interrupts connected to CPU->PCI */
#ifdef CONFIG_PERICOM
#	define AR7240_PRI_BUS_NO	0u
#	define AR7240_PORT0_BUS_NO	1u
#	define AR7240_PORT1_BUS_NO	2u
#	define AR7240_PCI_IRQ_DEV0	(AR7240_PCI_IRQ_BASE + 0)
#	define AR7240_PCI_IRQ_DEV1	(AR7240_PCI_IRQ_BASE + 1)
#	define AR7240_PCI_IRQ_COUNT	2
#else
#	define AR7240_PCI_IRQ_DEV0	AR7240_PCI_IRQ_BASE+0
#	define AR7240_PCI_IRQ_COUNT	1
#endif /* CONFIG_PERICOM */

/*
 * PCI interrupt mask and status
 */
#define PIMR_DEV0                           0x01
#define PIMR_DEV1                           0x02
#define PIMR_DEV2                           0x04
#define PIMR_CORE                           0x10

#define PISR_DEV0                           PIMR_DEV0
#define PISR_DEV1                           PIMR_DEV1
#define PISR_DEV2                           PIMR_DEV2
#define PISR_CORE                           PIMR_CORE

void ar7240_pci_irq_init(int irq_base); /* ??? */


#define AR7240_GPIO_COUNT                   16

/*
 * GPIO Function Enables
 */
#define AR7240_GPIO_FUNCTION_STEREO_EN       (1<<17)
#define AR7240_GPIO_FUNCTION_SLIC_EN         (1<<16)
#define AR7240_GPIO_FUNCTION_SPI_CS_1_EN     (1<<15)
#define AR7240_GPIO_FUNCTION_SPI_CS_0_EN     (1<<14)
#define AR7240_GPIO_FUNCTION_UART_EN         (1<< 8)
#define AR7240_GPIO_FUNCTION_OVERCURRENT_EN  (1<< 4)
#define AR7240_GPIO_FUNCTION_USB_CLK_CORE_EN (1<< 0)
#define AR7240_GPIO_FUNCTION_WMAC_LED        (1<<22)

/*
 * GPIO Access & Control
 */
void ar7240_gpio_init(void);
void ar7240_gpio_down(void);
void ar7240_gpio_up(void);

/*
 * GPIO Helper Functions
 */
void ar7240_gpio_enable_slic(void);

/* enable UART block, takes away GPIO 10 and 9 */
void ar7240_gpio_enable_uart(void);

/* enable STEREO block, takes away GPIO 11,8,7, and 6 */
void ar7240_gpio_enable_stereo(void);

/* allow CS0/CS1 to be controlled via SPI register, takes away GPIO0/GPIO1 */
void ar7240_gpio_enable_spi_cs1_cs0(void);

/* allow GPIO0/GPIO1 to be used as SCL/SDA for software based i2c */
void ar7240_gpio_enable_i2c_on_gpio_0_1(void);

/*
 * GPIO General Functions
 */
void ar7240_gpio_drive_low(unsigned int mask);
void ar7240_gpio_drive_high(unsigned int mask);

unsigned int ar7240_gpio_float_high_test(unsigned int mask);

/*
 * Software support of i2c on gpio 0/1
 */
int ar7240_i2c_raw_write_bytes_to_addr(int addr, unsigned char *buffer, int count);
int ar7240_i2c_raw_read_bytes_from_addr(int addr, unsigned char *buffer, int count);

/* SPI, SLIC and GPIO are all multiplexed on gpio pins */
#define AR7240_SPI_FS           AR7240_SPI_BASE
#define AR7240_SPI_READ         AR7240_SPI_BASE
#define AR7240_SPI_CLOCK        AR7240_SPI_BASE+4
#define AR7240_SPI_WRITE        AR7240_SPI_BASE+8
#define AR7240_SPI_RD_STATUS    AR7240_SPI_BASE+12
#define AR7240_SPI_D0_HIGH      (1<<0)             /* Pin spi_do   */
#define AR7240_SPI_CLK_HIGH     (1<<8)             /* Pin spi_clk  */

#define AR7240_SPI_CS_ENABLE_0  (6<<16)            /* Pin gpio/cs0 (active low) */
#define AR7240_SPI_CS_ENABLE_1  (5<<16)            /* Pin gpio/cs1 (active low) */
#define AR7240_SPI_CS_ENABLE_2  (3<<16)            /* Pin gpio/cs2 (active low) */
//#define AR7240_SPI_CS_DIS       (AR7240_SPI_CS_ENABLE_0|AR7240_SPI_CS_ENABLE_1|AR7240_SPI_CS_ENABLE_2)
#define AR7240_SPI_CS_DIS       0x70000


#define AR7240_SPI_RD_STATUS     AR7240_SPI_BASE+12 /* spi_di is clocked into register pos 0 every clock */
/* 
 * SOC 
 */
#define AR7240_SPI_CMD_WREN         0x06
#define AR7240_SPI_CMD_RD_STATUS    0x05
#define AR7240_SPI_CMD_FAST_READ    0x0b
#define AR7240_SPI_CMD_PAGE_PROG    0x02
#define AR7240_SPI_CMD_SECTOR_ERASE 0xd8

/* Functions to access SPI through software. Example:
 *
 * ar7240_spi_down(); ---------------------- disable others from accessing SPI bus taking semaphore
 * ar7240_spi_enable_soft_access(); -------- disable HW control of SPI
 *
 * <board specific chip select routine>
 *
 * <read/write SPI using using custom routine or general purposeflash routines
 * Custom routine may use:
 *
 *  ar7240_spi_raw_output_u8(unsigned char)
 *  ar7240_spi_raw_output_u32(unsigned int)
 *  ar7240_spi_raw_input_u32()
 *
 * General purpose flash routines:
 *  ar7240_spi_flash_read_page(unsigned int addr, unsigned char *data, int len);
 *  ar7240_spi_flash_write_page(unsigned int addr, unsigned char *data, int len);
 *  ar7240_spi_flash_sector_erase(unsigned int addr);
 * >
 *
 * <board specific chip deselect routine>
 *
 * ar7240_spi_disable_soft_acess(); ------- enable HW control of SPI bus
 * ar7240_spi_up(); ----------------------- enable others to access SPI bus releasing semaphore
 */
void ar7240_spi_init(void);
void ar7240_spi_down(void);
void ar7240_spi_up(void);

static inline void 
ar7240_spi_enable_soft_access(void)
{
  ar7240_reg_wr_nf(AR7240_SPI_FS, 1); 
}

static inline void 
ar7240_spi_disable_soft_access(void)
{
  ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
  ar7240_reg_wr_nf(AR7240_SPI_FS, 0);
}

void ar7240_spi_raw_output_u8(unsigned char val);
void ar7240_spi_raw_output_u32(unsigned int val);
unsigned int ar7240_spi_raw_input_u32(void);

#define AR7240_SPI_SECTOR_SIZE      (1024*64)

void ar7240_spi_flash_read_page(unsigned int addr, unsigned char *data, int len);
void ar7240_spi_flash_write_page(unsigned int addr, unsigned char *data, int len);
void ar7240_spi_flash_sector_erase(unsigned int addr);

/* 
 * Allow access to cs0-2 when GPIO Function enables cs0-2 through SPI register.
 */
static inline void ar7240_spi_enable_cs0(void)
{
  unsigned int cs;
  ar7240_spi_down();
  ar7240_spi_enable_soft_access();
  cs = ar7240_reg_rd(AR7240_SPI_WRITE) & ~AR7240_SPI_CS_DIS;
  ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_ENABLE_0 | cs);
}

static inline void ar7240_spi_enable_cs1(void)
{
  unsigned int cs;
  ar7240_spi_down();
  ar7240_spi_enable_soft_access();
  cs = ar7240_reg_rd(AR7240_SPI_WRITE) & ~AR7240_SPI_CS_DIS;
  ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_ENABLE_1 | cs);
}

static inline void ar7240_spi_disable_cs(void)
{
  unsigned int cs = ar7240_reg_rd(AR7240_SPI_WRITE) | AR7240_SPI_CS_DIS;
  ar7240_reg_wr_nf(AR7240_SPI_WRITE, cs);
  ar7240_spi_disable_soft_access();
  ar7240_spi_up();
}

/* 
 * Example usage to access BOOT flash
 */
static inline void ar7240_spi_flash_cs0_sector_erase(unsigned int addr) 
{
  ar7240_spi_enable_cs0();
  ar7240_spi_flash_sector_erase(addr);
  ar7240_spi_disable_cs();
}

static inline void ar7240_spi_flash_cs0_write_page(unsigned int addr, unsigned char *data, int len)
{
  ar7240_spi_enable_cs0();
  ar7240_spi_flash_write_page(addr, data, len);
  ar7240_spi_disable_cs();
}

/*
 * Reset block
 */
#define AR7240_GENERAL_TMR            AR7240_RESET_BASE+0
#define AR7240_GENERAL_TMR_RELOAD     AR7240_RESET_BASE+4
#define AR7240_WATCHDOG_TMR_CONTROL   AR7240_RESET_BASE+8
#define AR7240_WATCHDOG_TMR           AR7240_RESET_BASE+0xc
#define AR7240_MISC_INT_STATUS        AR7240_RESET_BASE+0x10
#define AR7240_MISC_INT_MASK          AR7240_RESET_BASE+0x14

#define AR7240_PCI_INT_STATUS         AR7240_PCI_CTLR_BASE+0x4c
#define AR7240_PCI_INT_MASK           AR7240_PCI_CTLR_BASE+0x50
#define AR7240_PCI_INT_A_L		(1 << 14) /* INTA Level Trigger */
#define AR7240_PCI_INT_B_L		(1 << 15) /* INTB Level Trigger */
#define AR7240_PCI_INT_C_L		(1 << 16) /* INTC Level Trigger */
#define AR7240_GLOBAL_INT_STATUS      AR7240_RESET_BASE+0x20
#define AR7240_RESET                  AR7240_RESET_BASE+0x1c
#define AR7240_OBSERVATION_ENABLE     AR7240_RESET_BASE+0x28


#define AR7240_WD_ACT_MASK      3u
#define AR7240_WD_ACT_NONE      0u /* No Action */
#define AR7240_WD_ACT_GP_INTR   1u /* General purpose intr */
#define AR7240_WD_ACT_NMI       2u /* NMI */
#define AR7240_WD_ACT_RESET     3u /* Full Chip Reset */

#define AR7240_WD_LAST_SHIFT    31  
#define AR7240_WD_LAST_MASK     ((uint32_t)(1 << AR7240_WD_LAST_SHIFT))



/*
 * Performace counters
 */
#define AR7240_PERF0_COUNTER          AR7240_GE0_BASE+0xa0
#define AR7240_PERF1_COUNTER          AR7240_GE1_BASE+0xa0

/*
 * SLIC/STEREO DMA Size Configurations 
 */
#define AR7240_DMA_BUF_SIZE_4X2      0x00
#define AR7240_DMA_BUF_SIZE_8X2      0x01
#define AR7240_DMA_BUF_SIZE_16X2     0x02
#define AR7240_DMA_BUF_SIZE_32X2     0x03
#define AR7240_DMA_BUF_SIZE_64X2     0x04
#define AR7240_DMA_BUF_SIZE_128X2    0x05
#define AR7240_DMA_BUF_SIZE_256X2    0x06
#define AR7240_DMA_BUF_SIZE_512X2    0x07

/*
 * SLIC/STEREO DMA Assignments
 */
#define AR7240_DMA_CHAN_SLIC0_RX     0
#define AR7240_DMA_CHAN_SLIC1_RX     1
#define AR7240_DMA_CHAN_STEREO_RX    2
#define AR7240_DMA_CHAN_SLIC0_TX     3
#define AR7240_DMA_CHAN_SLIC1_TX     4
#define AR7240_DMA_CHAN_STEREO_TX    5

/* Low-level routines */
void ar7240_dma_addr_wr  (int chan, unsigned int val);
void ar7240_dma_config_wr(int chan, unsigned int val);
void ar7240_dma_update_wr(int chan, unsigned int val);

unsigned int ar7240_dma_addr_rd  (int chan);
unsigned int ar7240_dma_config_rd(int chan);

/* Use this routine to configure DMA access. Example:
 *
 * ar7240_dma_config_buffer( AR7240_DMA_CHAN_SLIC0_TX, 
 *                           < address of buffer >,
 *                           AR7240_DMA_BUF_SIZE_512X2
 */
void ar7240_dma_config_buffer(int chan, void *buffer, int sizeCfg);

/*
 * SLIC register definitions
 */
#define AR7240_SLIC_STATUS                   (AR7240_SLIC_BASE+0x00)
#define AR7240_SLIC_CNTRL                    (AR7240_SLIC_BASE+0x04)
#define AR7240_SLIC_SLOT0_NUM                (AR7240_SLIC_BASE+0x08)
#define AR7240_SLIC_SLOT1_NUM                (AR7240_SLIC_BASE+0x0c)
#define AR7240_SLIC_SAM_POS                  (AR7240_SLIC_BASE+0x2c)
#define AR7240_SLIC_FREQ_DIV                 (AR7240_SLIC_BASE+0x30)

/*
 * SLIC Control bits
 */
#define AR7240_SLIC_CNTRL_ENABLE             (1<<0)
#define AR7240_SLIC_CNTRL_SLOT0_ENABLE       (1<<1)
#define AR7240_SLIC_CNTRL_SLOT1_ENABLE       (1<<2)
#define AR7240_SLIC_CNTRL_IRQ_ENABLE         (1<<3)

/*
 * SLIC Helper Functions
 */
unsigned int ar7240_slic_status_rd(void);
unsigned int ar7240_slic_cntrl_rd(void);

void ar7240_slic_cntrl_wr(unsigned int val); 
void ar7240_slic_0_slot_pos_wr(unsigned int val);
void ar7240_slic_1_slot_pos_wr(unsigned int val);
void ar7240_slic_freq_div_wr(unsigned int val);
void ar7240_slic_sample_pos_wr(unsigned int val);

void ar7240_slic_setup(int _sam, int _s0n, int _s1n);

/*
 * STEREO register definitions
 */
#define AR7240_STEREO_CONFIG                 (AR7240_STEREO_BASE+0x00)
#define AR7240_STEREO_VOLUME                 (AR7240_STEREO_BASE+0x04)

/*
 * Stereo Configuration Bits
 */
#define AR7240_STEREO_CONFIG_ENABLE          (1<<24)
#define AR7240_STEREO_CONFIG_RESET           (1<<23)
#define AR7240_STEREO_CONFIG_DELAY           (1<<22)
#define AR7240_STEREO_CONFIG_MIC_WORD_SIZE   (1<<20)

#define AR7240_STEREO_CONFIG_MODE(x)           ((3&x)<<18)
#define AR7240_STEREO_MODE_STEREO             0
#define AR7240_STEREO_MODE_LEFT               1
#define AR7240_STEREO_MODE_RIGHT              2

#define AR7240_STEREO_CONFIG_DATA_WORD_SIZE(x) ((3&x)<<16)

#define AR7240_STEREO_CONFIG_I2S_32B_WORD    (1<<15)
#define AR7240_STEREO_CONFIG_MASTER          (1<<8)
#define AR7240_STEREO_CONFIG_PSEDGE(x)       (0xff&x)

/*
 * Word sizes to use with common configurations:
 */
#define AR7240_STEREO_WS_8B                     0
#define AR7240_STEREO_WS_16B                    1

/* 
 * Audio data is little endian so 16b values must be swapped in the DMA buffers.
 */
static inline int ar7240_stereo_sample_16b_cvt(unsigned int _v) { return (((_v<<8)&0xff00)|((_v>>8)&0xff)) & 0xffff; }

/* Low level read/write of configuration */
void ar7240_stereo_config_wr(unsigned int val);
void ar7240_stereo_volume_wr(unsigned int val);

unsigned int ar7240_stereo_config_rd(void);
unsigned int ar7240_stereo_volume_rd(void);

/*
 * Common configurations for stereo block
 */
#define AR7240_STEREO_CFG_MASTER_STEREO_FS32_48KHZ(ws) ( \
		       AR7240_STEREO_CONFIG_DELAY   | \
		       AR7240_STEREO_CONFIG_RESET   | \
		       AR7240_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
                       AR7240_STEREO_CONFIG_MODE(AR7240_STEREO_MODE_LEFT) | \
		       AR7240_STEREO_CONFIG_MASTER  | \
		       AR7240_STEREO_CONFIG_PSEDGE(26))
     
#define AR7240_STEREO_CFG_MASTER_STEREO_FS64_48KHZ(ws) ( \
		       AR7240_STEREO_CONFIG_DELAY   | \
		       AR7240_STEREO_CONFIG_RESET   | \
		       AR7240_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
                       AR7240_STEREO_CONFIG_MODE(AR7240_STEREO_MODE_STEREO) | \
		       AR7240_STEREO_CONFIG_I2S_32B_WORD | \
		       AR7240_STEREO_CONFIG_MASTER  | \
		       AR7240_STEREO_CONFIG_PSEDGE(13))

#define AR7240_STEREO_CFG_SLAVE_STEREO_FS32_48KHZ(ws) ( \
		       AR7240_STEREO_CONFIG_RESET   | \
		       AR7240_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
                       AR7240_STEREO_CONFIG_MODE(AR7240_STEREO_MODE_STEREO) | \
		       AR7240_STEREO_CONFIG_PSEDGE(26))

#define AR7240_STEREO_CFG_SLAVE_STEREO_FS64_48KHZ(ws) ( \
		       AR7240_STEREO_CONFIG_RESET   | \
		       AR7240_STEREO_CONFIG_I2S_32B_WORD | \
		       AR7240_STEREO_CONFIG_DATA_WORD_SIZE(ws) | \
                       AR7240_STEREO_CONFIG_MODE(AR7240_STEREO_MODE_STEREO) | \
		       AR7240_STEREO_CONFIG_PSEDGE(13))

/* Routine sets up STEREO block for use. Use one of the predefined
 * configurations. Example:
 * 
 * ar7240_stereo_config_setup(
 *   AR7240_STEREO_CFG_MASTER_STEREO_FS32_48KHZ(AR7240_STEREO_WS_16B))
 *
 */
void ar7240_stereo_config_setup(unsigned int cfg);

/* 48 kHz, 16 bit data & i2s 32fs */
static inline void ar7240_setup_for_stereo_master(int ws)
{ ar7240_stereo_config_setup(AR7240_STEREO_CFG_MASTER_STEREO_FS32_48KHZ(ws)); }

/* 48 kHz, 16 bit data & 32fs i2s */
static inline void ar7240_setup_for_stereo_slave(int ws) 
{ ar7240_stereo_config_setup(AR7240_STEREO_CFG_SLAVE_STEREO_FS32_48KHZ(ws)); }

/*
 * PERF CTL bits
 */
#define PERF_CTL_PCI_AHB_0           ( 0)
#define PERF_CTL_PCI_AHB_1           ( 1)
#define PERF_CTL_USB_0               ( 2)
#define PERF_CTL_USB_1               ( 3)
#define PERF_CTL_GE0_PKT_CNT         ( 4)
#define PERF_CTL_GEO_AHB_1           ( 5)
#define PERF_CTL_GE1_PKT_CNT         ( 6)
#define PERF_CTL_GE1_AHB_1           ( 7)
#define PERF_CTL_PCI_DEV_0_BUSY      ( 8)
#define PERF_CTL_PCI_DEV_1_BUSY      ( 9)
#define PERF_CTL_PCI_DEV_2_BUSY      (10)
#define PERF_CTL_PCI_HOST_BUSY       (11)
#define PERF_CTL_PCI_DEV_0_ARB       (12)
#define PERF_CTL_PCI_DEV_1_ARB       (13)
#define PERF_CTL_PCI_DEV_2_ARB       (14)
#define PERF_CTL_PCI_HOST_ARB        (15)
#define PERF_CTL_PCI_DEV_0_ACTIVE    (16)
#define PERF_CTL_PCI_DEV_1_ACTIVE    (17)
#define PERF_CTL_PCI_DEV_2_ACTIVE    (18)
#define PERF_CTL_HOST_ACTIVE         (19)

#define ar7240_perf0_ctl(_val) ar7240_reg_wr(AR7240_PERF_CTL, (_val))
#define ar7240_perf1_ctl(_val) ar7240_reg_rmw_set(AR7240_PERF_CTL, ((_val) << 8))


/* These are values used in platform.inc to select PLL settings */

#define AR7240_REV_ID           (AR7240_RESET_BASE + 0x90)
#define AR7240_REV_ID_MASK      0xffff
#define AR7240_REV_ID_AR7130    0xa0
#define AR7240_REV_ID_AR7141    0xa1
#define AR7240_REV_ID_AR7161    0xa2
#define AR7240_REV_1_0          0xc0
#define AR7240_REV_1_1          0xc1
#define AR7240_REV_1_2          0xc2
#define AR7241_REV_1_0          0x0100

#define AR7240_PLL_USE_REV_ID    0
#define AR7240_PLL_200_200_100   1
#define AR7240_PLL_300_300_150   2
#define AR7240_PLL_333_333_166   3
#define AR7240_PLL_266_266_133   4
#define AR7240_PLL_266_266_66    5
#define AR7240_PLL_400_400_200   6
#define AR7240_PLL_600_400_150   7


/*
 * AR7240_RESET bit defines
 */
#define AR7240_RESET_EXTERNAL               (1 << 28)
#define AR7240_RESET_FULL_CHIP              (1 << 24)
#define AR7240_RESET_CPU_NMI                (1 << 21)
#define AR7240_RESET_CPU_COLD_RESET_MASK    (1 << 20)
#define AR7240_RESET_DMA                    (1 << 19)
#define AR7240_RESET_SLIC                   (1 << 18)
#define AR7240_RESET_STEREO                 (1 << 17)
#define AR7240_RESET_DDR                    (1 << 16)
#define AR7240_RESET_GE1_MAC                (1 << 13)
#define AR7240_RESET_GE1_PHY                (1 << 12)
#define AR7240_RESET_GE0_MAC                (1 << 9)
#define AR7240_RESET_GE0_PHY                (1 << 8)
#define AR7240_RESET_PCIE_PHY_SHIFT	    (1 << 10)
#define AR7240_RESET_USBSUS_OVRIDE	    (1 << 3)
#define AR7240_RESET_USB_OHCI_DLL           (1 << 3)
#define AR7240_RESET_USB_HOST               (1 << 5)
#define AR7240_RESET_USB_PHY                (1 << 4)
#define AR7240_RESET_PCI_BUS                (1 << 1)
#define AR7240_RESET_PCI_CORE               (1 << 0)


void ar7240_reset(unsigned int mask);

/*
 * Mii block
 */
#define AR7240_MII0_CTRL                    0x18070000
#define AR7240_MII1_CTRL                    0x18070004

#define BIT(_x) (1 << (_x))

#define ar7240_get_bit(_reg, _bit)  (ar7240_reg_rd((_reg)) & (1 << (_bit)))

#define ar7240_flush_ge(_unit) do {                             \
    u32     reg = (_unit) ? AR7240_DDR_GE1_FLUSH : AR7240_DDR_GE0_FLUSH;   \
    ar7240_reg_wr(reg, 1);                 \
    while((ar7240_reg_rd(reg) & 0x1));   \
    ar7240_reg_wr(reg, 1);                 \
    while((ar7240_reg_rd(reg) & 0x1));   \
}while(0);

#define ar7240_flush_pcie() do {                             \
    ar7240_reg_wr(AR7240_DDR_PCIE_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_PCIE_FLUSH) & 0x1));   \
    ar7240_reg_wr(AR7240_DDR_PCIE_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_PCIE_FLUSH) & 0x1));   \
}while(0);

#define ar7240_flush_USB() do {                             \
    ar7240_reg_wr(AR7240_DDR_USB_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_USB_FLUSH) & 0x1));   \
    ar7240_reg_wr(AR7240_DDR_USB_FLUSH, 1);                 \
    while((ar7240_reg_rd(AR7240_DDR_USB_FLUSH) & 0x1));   \
}while(0);

int ar7240_local_read_config(int where, int size, u32 *value);
int ar7240_local_write_config(int where, int size, u32 value);
int ar7240_check_error(int verbose);
unsigned char __ar7240_readb(const volatile void __iomem *p);
unsigned short __ar7240_readw(const volatile void __iomem *p);

#endif
