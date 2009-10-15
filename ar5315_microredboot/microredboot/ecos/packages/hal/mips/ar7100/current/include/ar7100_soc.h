#ifndef _AR7100_SOC_H
#define _AR7100_SOC_H

/*
 * Address map
 */
#define AR7100_PCI_MEM_BASE             0x10000000  /* 128M */
#define AR7100_APB_BASE                 0x18000000  /* 384M */
#define AR7100_GE0_BASE                 0x19000000  /* 16M */
#define AR7100_GE1_BASE                 0x1a000000  /* 16M */
#define AR7100_USB_EHCI_BASE            0x1b000000  
#define AR7100_USB_OHCI_BASE            0x1c000000
#define AR7100_SPI_BASE                 0x1f000000

/*
 * APB block
 */
#define AR7100_DDR_CTL_BASE             AR7100_APB_BASE+0x00000000
#define AR7100_CPU_BASE                 AR7100_APB_BASE+0x00010000
#define AR7100_UART_BASE                AR7100_APB_BASE+0x00020000
#define AR7100_USB_CONFIG_BASE          AR7100_APB_BASE+0x00030000
#define AR7100_GPIO_BASE                AR7100_APB_BASE+0x00040000
#define AR7100_PLL_BASE                 AR7100_APB_BASE+0x00050000
#define AR7100_RESET_BASE               AR7100_APB_BASE+0x00060000

/*
 * DDR block
 */

#define AR7100_DDR_CONFIG               AR7100_DDR_CTL_BASE+0
#define AR7100_DDR_CONFIG2              AR7100_DDR_CTL_BASE+4
#define AR7100_DDR_MODE                 AR7100_DDR_CTL_BASE+0x08
#define AR7100_DDR_EXT_MODE             AR7100_DDR_CTL_BASE+0x0c
#define AR7100_DDR_CONTROL              AR7100_DDR_CTL_BASE+0x10
#define AR7100_DDR_REFRESH              AR7100_DDR_CTL_BASE+0x14
#define AR7100_DDR_RD_DATA_THIS_CYCLE   AR7100_DDR_CTL_BASE+0x18
#define AR7100_DDR_TAP_CONTROL0         AR7100_DDR_CTL_BASE+0x1c
#define AR7100_DDR_TAP_CONTROL1         AR7100_DDR_CTL_BASE+0x20
#define AR7100_DDR_TAP_CONTROL2         AR7100_DDR_CTL_BASE+0x24
#define AR7100_DDR_TAP_CONTROL3         AR7100_DDR_CTL_BASE+0x28

#define AR7100_DDR_CONFIG_16BIT             (1 << 31)
#define AR7100_DDR_CONFIG_PAGE_OPEN         (1 << 30)
#define AR7100_DDR_CONFIG_CAS_LAT_SHIFT      27
#define AR7100_DDR_CONFIG_TMRD_SHIFT         23
#define AR7100_DDR_CONFIG_TRFC_SHIFT         17
#define AR7100_DDR_CONFIG_TRRD_SHIFT         13
#define AR7100_DDR_CONFIG_TRP_SHIFT          9
#define AR7100_DDR_CONFIG_TRCD_SHIFT         5
#define AR7100_DDR_CONFIG_TRAS_SHIFT         0

#define AR7100_DDR_CONFIG2_BL2          (2 << 0)
#define AR7100_DDR_CONFIG2_BL4          (4 << 0)
#define AR7100_DDR_CONFIG2_BL8          (8 << 0)

#define AR7100_DDR_CONFIG2_BT_IL        (1 << 4)       
#define AR7100_DDR_CONFIG2_CNTL_OE_EN   (1 << 5)
#define AR7100_DDR_CONFIG2_PHASE_SEL    (1 << 6)
#define AR7100_DDR_CONFIG2_DRAM_CKE     (1 << 7)
#define AR7100_DDR_CONFIG2_TWR_SHIFT    8
#define AR7100_DDR_CONFIG2_TRTW_SHIFT   12
#define AR7100_DDR_CONFIG2_TRTP_SHIFT   17
#define AR7100_DDR_CONFIG2_TWTR_SHIFT   21
#define AR7100_DDR_CONFIG2_HALF_WIDTH_L (1 << 31)   

#define AR7100_DDR_TAP_DEFAULT          0x18

/*
 * PLL
 */
#define AR7100_CPU_PLL_CONFIG           AR7100_PLL_BASE    

/* Legacy */
#define AR7100_USB_PLL_CONFIG           AR7100_PLL_BASE+0x4
#define AR7100_USB_PLL_GE0_OFFSET       AR7100_PLL_BASE+0x10
#define AR7100_USB_PLL_GE1_OFFSET       AR7100_PLL_BASE+0x14

/* From Data Sheet */
#define AR7100_CONFIG_SEC_PLL           AR7100_PLL_BASE+0x4
#define AR7100_CONFIG_ETH_INT0_CLOCK    AR7100_PLL_BASE+0x10
#define AR7100_CONFIG_ETH_INT1_CLOCK    AR7100_PLL_BASE+0x14
#define AR7100_CONFIG_ETH_EXT_CLOCK     AR7100_PLL_BASE+0x18
#define AR7100_CONFIG_PCI_CLOCK         AR7100_PLL_BASE+0x1C

#define AR7100_PCI_CLOCK_33MHZ          0x000000ee
#define AR7100_PCI_CLOCK_66MHZ          0x00000067

#define AR7100_USB

#define PLL_CONFIG_PLL_POWER_DOWN_MASK  (1 << 0)
#define PLL_CONFIG_PLL_BYPASS_MASK      (1 << 1)
#define PLL_CONFIG_PLL_FB_SHIFT         3
#define PLL_CONFIG_PLL_FB_MASK          (0x1f << PLL_CONFIG_PLL_FB_SHIFT)
#define PLL_CONFIG_PLL_DIVOUT_SHIFT     8
#define PLL_CONFIG_PLL_DIVOUT_MASK      (0x03 << PLL_CONFIG_PLL_DIVOUT_SHIFT)
#define PLL_CONFIG_PLL_LOOP_BW_SHIFT    12
#define PLL_CONFIG_PLL_LOOP_BW_MASK     (0xf << PLL_CONFIG_PLL_LOOP_BW_SHIFT)
#define PLL_CONFIG_CPU_DIV_SHIFT        16
#define PLL_CONFIG_CPU_DIV_MASK         (3 << PLL_CONFIG_CPU_DIV_SHIFT)
#define PLL_CONFIG_DDR_DIV_SHIFT        18
#define PLL_CONFIG_DDR_DIV_MASK         (3 << PLL_CONFIG_DDR_DIV_SHIFT)
#define PLL_CONFIG_AHB_DIV_SHIFT        20
#define PLL_CONFIG_AHB_DIV_MASK         (7 << PLL_CONFIG_AHB_DIV_SHIFT)
#define PLL_CONFIG_LOCKED_SHIFT         30
#define PLL_CONFIG_LOCKED_MASK          (1 << PLL_CONFIG_LOCKED_SHIFT)
#define PLL_CONFIG_SW_UPDATE_SHIFT      31
#define PLL_CONFIG_SW_UPDATE_MASK       (1 << 31)


/* These are values used in platform.inc to select PLL settings */

#define AR7100_REV_ID           (AR7100_RESET_BASE + 0x90)
#define AR7100_REV_ID_MASK      0xff
#define AR7100_REV_ID_AR7130    0xa0
#define AR7100_REV_ID_AR7141    0xa1
#define AR7100_REV_ID_AR7161    0xa2

#define AR7100_PLL_USE_REV_ID    0
#define AR7100_PLL_200_200_100   1
#define AR7100_PLL_300_300_150   2
#define AR7100_PLL_333_333_166   3
#define AR7100_PLL_266_266_133   4
#define AR7100_PLL_266_266_66    5
#define AR7100_PLL_400_400_200   6
#define AR7100_PLL_600_300_150   7
#define AR7100_PLL_680_340_170   8
/* "Secret values", for debug only */
#define AR7100_PLL_720_360_180   30
#define AR7100_PLL_800_400_200   31
/*
 * PLL block
 */
#define AR7100_PLL_CONFIG               AR7100_PLL_BASE+0x0

#define PLL_DIV_SHIFT   3
#define PLL_DIV_MASK    0x1f
#define CPU_DIV_SHIFT   16
#define CPU_DIV_MASK    0x3
#define DDR_DIV_SHIFT   18
#define DDR_DIV_MASK    0x3
#define AHB_DIV_SHIFT   20
#define AHB_DIV_MASK    0x7

/*
 * CLOCK
 */
#define AR7100_CPU_CLOCK_CONTROL        AR7100_PLL_BASE+8
#define CLOCK_CONTROL_CLOCK_SWITCH_SHIFT  0
#define CLOCK_CONTROL_CLOCK_SWITCH_MASK  (1 << CLOCK_CONTROL_CLOCK_SWITCH_SHIFT)
#define CLOCK_CONTROL_RST_SWITCH_SHIFT    1
#define CLOCK_CONTROL_RST_SWITCH_MASK    (1 << CLOCK_CONTROL_RST_SWITCH_SHIFT)

/*
 * FIFO flushes
 */
#define AR7100_DDR_GE0_FLUSH            AR7100_DDR_CTL_BASE+0x9c
#define AR7100_DDR_GE1_FLUSH            AR7100_DDR_CTL_BASE+0xa0
#define AR7100_DDR_PCI_FLUSH            AR7100_DDR_CTL_BASE+0xa8

/*
 * USB block
 */
#define AR7100_USB_FLADJ_VAL            AR7100_USB_CONFIG_BASE
#define AR7100_USB_CONFIG               AR7100_USB_CONFIG_BASE+0x4
#define AR7100_USB_WINDOW               0x1000000

/*
 * PCI block
 */
#define AR7100_PCI_WINDOW           0x8000000       /* 128MB */
#define AR7100_PCI_WINDOW0_OFFSET   AR7100_DDR_CTL_BASE+0x7c
#define AR7100_PCI_WINDOW1_OFFSET   AR7100_DDR_CTL_BASE+0x80
#define AR7100_PCI_WINDOW2_OFFSET   AR7100_DDR_CTL_BASE+0x84
#define AR7100_PCI_WINDOW3_OFFSET   AR7100_DDR_CTL_BASE+0x88
#define AR7100_PCI_WINDOW4_OFFSET   AR7100_DDR_CTL_BASE+0x8c
#define AR7100_PCI_WINDOW5_OFFSET   AR7100_DDR_CTL_BASE+0x90
#define AR7100_PCI_WINDOW6_OFFSET   AR7100_DDR_CTL_BASE+0x94
#define AR7100_PCI_WINDOW7_OFFSET   AR7100_DDR_CTL_BASE+0x98

#define AR7100_PCI_WINDOW0_VAL      0x10000000
#define AR7100_PCI_WINDOW1_VAL      0x11000000
#define AR7100_PCI_WINDOW2_VAL      0x12000000
#define AR7100_PCI_WINDOW3_VAL      0x13000000
#define AR7100_PCI_WINDOW4_VAL      0x14000000
#define AR7100_PCI_WINDOW5_VAL      0x15000000
#define AR7100_PCI_WINDOW6_VAL      0x16000000
#define AR7100_PCI_WINDOW7_VAL      0x07000000


/*
 * CRP. To access the host controller config and status registers
 */
#define AR7100_PCI_CRP   (AR7100_PCI_MEM_BASE|(AR7100_PCI_WINDOW7_VAL+0x10000))

#define AR7100_PCI_CRP_AD_CBE               AR7100_PCI_CRP
#define AR7100_PCI_CRP_WRDATA               AR7100_PCI_CRP+0x4
#define AR7100_PCI_CRP_RDDATA               AR7100_PCI_CRP+0x8
#define AR7100_PCI_ERROR            AR7100_PCI_CRP+0x1c
#define AR7100_PCI_ERROR_ADDRESS    AR7100_PCI_CRP+0x20
#define AR7100_PCI_AHB_ERROR            AR7100_PCI_CRP+0x24
#define AR7100_PCI_AHB_ERROR_ADDRESS    AR7100_PCI_CRP+0x28

#define AR7100_CRP_CMD_WRITE             0x00010000
#define AR7100_CRP_CMD_READ              0x00000000

/*
 * PCI CFG. To generate config cycles
 */
#define AR7100_PCI_CFG_AD           AR7100_PCI_CRP+0xc
#define AR7100_PCI_CFG_CBE          AR7100_PCI_CRP+0x10
#define AR7100_PCI_CFG_WRDATA       AR7100_PCI_CRP+0x14
#define AR7100_PCI_CFG_RDDATA       AR7100_PCI_CRP+0x18
#define AR7100_CFG_CMD_READ         0x0000000a
#define AR7100_CFG_CMD_WRITE        0x0000000b

#define AR7100_PCI_IDSEL_ADLINE_START           17

/*
 * gpio configs
 */
#define AR7100_GPIO_OE                  AR7100_GPIO_BASE+0x0
#define AR7100_GPIO_IN                  AR7100_GPIO_BASE+0x4
#define AR7100_GPIO_OUT                 AR7100_GPIO_BASE+0x8
#define AR7100_GPIO_SET                 AR7100_GPIO_BASE+0xc
#define AR7100_GPIO_CLEAR               AR7100_GPIO_BASE+0x10
#define AR7100_GPIO_INT_ENABLE          AR7100_GPIO_BASE+0x14
#define AR7100_GPIO_INT_TYPE            AR7100_GPIO_BASE+0x18
#define AR7100_GPIO_INT_POLARITY        AR7100_GPIO_BASE+0x1c
#define AR7100_GPIO_INT_PENDING         AR7100_GPIO_BASE+0x20
#define AR7100_GPIO_INT_MASK            AR7100_GPIO_BASE+0x24

/*
 * SPI Block
 */
#define AR7100_SPI_FS           AR7100_SPI_BASE+0x0
#define AR7100_SPI_CLOCK        AR7100_SPI_BASE+0x4
#define AR7100_SPI_WRITE        AR7100_SPI_BASE+0x8
#define AR7100_SPI_READ         AR7100_SPI_BASE+0x0
#define AR7100_SPI_RD_STATUS    AR7100_SPI_BASE+0xc

#define AR7100_SPI_CS_DIS       0x70000
#define AR7100_SPI_CE_LOW       0x60000
#define AR7100_SPI_CE_HIGH      0x60100

#define AR7100_SPI_CMD_WREN         0x06
#define AR7100_SPI_CMD_RD_STATUS    0x05
#define AR7100_SPI_CMD_FAST_READ    0x0b
#define AR7100_SPI_CMD_PAGE_PROG    0x02
#define AR7100_SPI_CMD_SECTOR_ERASE 0xd8

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
#define AR7100_CPU_IRQ_BASE         0x00
#define AR7100_MISC_IRQ_BASE        0x10
#define AR7100_GPIO_IRQ_BASE        0x20
#define AR7100_PCI_IRQ_BASE         0x30

/*
 * The IPs. Connected to CPU (hardware IP's; the first two are software)
 */
#define AR7100_CPU_IRQ_PCI                  AR7100_CPU_IRQ_BASE+2
#define AR7100_CPU_IRQ_USB                  AR7100_CPU_IRQ_BASE+3
#define AR7100_CPU_IRQ_GE0                  AR7100_CPU_IRQ_BASE+4
#define AR7100_CPU_IRQ_GE1                  AR7100_CPU_IRQ_BASE+5
#define AR7100_CPU_IRQ_MISC                 AR7100_CPU_IRQ_BASE+6
#define AR7100_CPU_IRQ_TIMER                AR7100_CPU_IRQ_BASE+7

/*
 * Interrupts connected to the CPU->Misc line.
 */
#define AR7100_MISC_IRQ_TIMER               AR7100_MISC_IRQ_BASE+0
#define AR7100_MISC_IRQ_ERROR               AR7100_MISC_IRQ_BASE+1
#define AR7100_MISC_IRQ_GPIO                AR7100_MISC_IRQ_BASE+2
#define AR7100_MISC_IRQ_UART                AR7100_MISC_IRQ_BASE+3
#define AR7100_MISC_IRQ_WATCHDOG            AR7100_MISC_IRQ_BASE+4
#define AR7100_MISC_IRQ_COUNT                 5

#define MIMR_TIMER                          0x01
#define MIMR_ERROR                          0x02
#define MIMR_GPIO                           0x04
#define MIMR_UART                           0x08
#define MIMR_WATCHDOG                       0x10

#define MISR_TIMER                          MIMR_TIMER
#define MISR_ERROR                          MIMR_ERROR
#define MISR_GPIO                           MIMR_GPIO
#define MISR_UART                           MIMR_UART
#define MISR_WATCHDOG                       MIMR_WATCHDOG

/*
 * Interrupts connected to the Misc->GPIO line
 */
#define AR7100_GPIO_IRQn(_gpio)             AR7100_GPIO_IRQ_BASE+(_gpio)
#define AR7100_GPIO_IRQ_COUNT                 16

/*
 * Interrupts connected to CPU->PCI
 */
#define AR7100_PCI_IRQ_DEV0                  AR7100_PCI_IRQ_BASE+0
#define AR7100_PCI_IRQ_DEV1                  AR7100_PCI_IRQ_BASE+1
#define AR7100_PCI_IRQ_DEV2                  AR7100_PCI_IRQ_BASE+2
#define AR7100_PCI_IRQ_CORE                  AR7100_PCI_IRQ_BASE+3
#define AR7100_PCI_IRQ_COUNT                 4

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


#define AR7100_GPIO_COUNT                   16

/*
 * Reset block
 */
#define AR7100_GENERAL_TMR            AR7100_RESET_BASE+0
#define AR7100_GENERAL_TMR_RELOAD     AR7100_RESET_BASE+4
#define AR7100_WATCHDOG_TMR_CONTROL   AR7100_RESET_BASE+8
#define AR7100_WATCHDOG_TMR           AR7100_RESET_BASE+0xc
#define AR7100_MISC_INT_STATUS        AR7100_RESET_BASE+0x10
#define AR7100_MISC_INT_MASK          AR7100_RESET_BASE+0x14
#define AR7100_PCI_INT_STATUS         AR7100_RESET_BASE+0x18
#define AR7100_PCI_INT_MASK           AR7100_RESET_BASE+0x1c
#define AR7100_GLOBAL_INT_STATUS      AR7100_RESET_BASE+0x20
#define AR7100_RESET                  AR7100_RESET_BASE+0x24

/*
 * AR7100_RESET bit defines
 */
#define AR7100_RESET_EXTERNAL               (1 << 28)
#define AR7100_RESET_FULL_CHIP              (1 << 24)
#define AR7100_RESET_CPU_NMI                (1 << 21)
#define AR7100_RESET_CPU_COLD_RESET_MASK    (1 << 20)
#define AR7100_RESET_DDR                    (1 << 16)
#define AR7100_RESET_GE1_MAC                (1 << 13)
#define AR7100_RESET_GE1_PHY                (1 << 12)
#define AR7100_RESET_GE0_MAC                (1 << 9)
#define AR7100_RESET_GE0_PHY                (1 << 8)
#define AR7100_RESET_USB_HOST               (1 << 5)
#define AR7100_RESET_USB_PHY                (1 << 4)
#define AR7100_RESET_PCI_BUS                (1 << 1)
#define AR7100_RESET_PCI_CORE               (1 << 0)

#define AR7100_MII0_CTRL                    0x18070000
#define AR7100_MII1_CTRL                    0x18070004

#define K0BASE 0x80000000
#define K1BASE 0xa0000000

#ifndef __ASSEMBLER__
typedef enum {
    AR7100_DDR_16B_LOW,
    AR7100_DDR_16B_HIGH,
    AR7100_DDR_32B,
}ar7100_ddr_width_t;

#define KSEG1ADDR(_addr) (CYGARC_UNCACHED_ADDRESS(_addr))
#define ar7100_reg_rd(_phys)    (*(volatile unsigned int *)KSEG1ADDR(_phys))
#define ar7100_reg_wr_nf(_phys, _val) \
                    ((*(volatile unsigned int *)KSEG1ADDR(_phys)) = (_val))

#define ar7100_reg_wr(_phys, _val) do {     \
                    ar7100_reg_wr_nf(_phys, _val);  \
                    ar7100_reg_rd(_phys);       \
}while(0);

#define ar7100_write_pci_window(_no)             \
  ar7100_reg_wr(AR7100_PCI_WINDOW##_no##_OFFSET, AR7100_PCI_WINDOW##_no##_VAL);

#define BIT(_x) (1 << (_x))

#define ar7100_reg_rmw_set(_reg, _mask)  do {                        \
    ar7100_reg_wr((_reg), (ar7100_reg_rd((_reg)) | (_mask)));      \
    ar7100_reg_rd((_reg));                                           \
}while(0);

#define ar7100_reg_rmw_clear(_reg, _mask)  do {                        \
    ar7100_reg_wr((_reg), (ar7100_reg_rd((_reg)) & ~(_mask)));      \
    ar7100_reg_rd((_reg));                                           \
}while(0);

#define ar7100_get_bit(_reg, _bit)  (ar7100_reg_rd((_reg)) & (1 << (_bit)))

#define ar7100_flush_ge(_unit) do {                             \
    uint32_t     reg = (_unit) ? AR7100_DDR_GE1_FLUSH : AR7100_DDR_GE0_FLUSH;   \
    ar7100_reg_wr(reg, 1);                 \
    while((ar7100_reg_rd(reg) & 0x1));   \
    ar7100_reg_wr(reg, 1);                 \
    while((ar7100_reg_rd(reg) & 0x1));   \
}while(0);

#define ar7100_flush_pci() do {                             \
    ar7100_reg_wr(AR7100_DDR_PCI_FLUSH, 1);                 \
    while((ar7100_reg_rd(AR7100_DDR_PCI_FLUSH) & 0x1));   \
    ar7100_reg_wr(AR7100_DDR_PCI_FLUSH, 1);                 \
    while((ar7100_reg_rd(AR7100_DDR_PCI_FLUSH) & 0x1));   \
}while(0);

#endif  /*__ASSEMBLY*/

#endif
