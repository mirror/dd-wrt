/******************************************************************************
 * MODULE NAME:    star_misc.h
 * PROJECT CODE:   Vega
 * DESCRIPTION:    
 * MAINTAINER:     Jacky Hou
 * DATE:           9 February 2009
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2008-2009 Cavium Networks Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/

#ifndef _CNS3XXX_MISC_H_
#define _CNS3XXX_MISC_H_
#include <mach/board.h>
#define MISC_MEM_MAP_VALUE(offset) (*((volatile unsigned int *)(CNS3XXX_MISC_BASE_VIRT + offset)))


/*
 * define access macros
 */
#define MISC_MEMORY_REMAP_REG                      MISC_MEM_MAP_VALUE(0x00)
#define MISC_CHIP_CONFIG_REG                       MISC_MEM_MAP_VALUE(0x04)
#define MISC_DEBUG_PROBE_DATA_REG                  MISC_MEM_MAP_VALUE(0x08)
#define MISC_DEBUG_PROBE_SELECTION_REG             MISC_MEM_MAP_VALUE(0x0C)
#define MISC_IO_PIN_FUNC_SELECTION_REG             MISC_MEM_MAP_VALUE(0x10)
#define MISC_GPIOA_PIN_ENABLE_REG                  MISC_MEM_MAP_VALUE(0x14)
#define MISC_GPIOB_PIN_ENABLE_REG                  MISC_MEM_MAP_VALUE(0x18)
#define MISC_IO_PAD_DRIVE_STRENGTH_CTRL_A          MISC_MEM_MAP_VALUE(0x1C)
#define MISC_IO_PAD_DRIVE_STRENGTH_CTRL_B          MISC_MEM_MAP_VALUE(0x20)
#define MISC_GPIOA_15_0_PULL_CTRL_REG			   MISC_MEM_MAP_VALUE(0x24)
#define MISC_GPIOA_16_31_PULL_CTRL_REG			   MISC_MEM_MAP_VALUE(0x28)
#define MISC_GPIOB_15_0_PULL_CTRL_REG              MISC_MEM_MAP_VALUE(0x2C)
#define MISC_GPIOB_16_31_PULL_CTRL_REG             MISC_MEM_MAP_VALUE(0x30)
#define MISC_IO_PULL_CTRL_REG                  	   MISC_MEM_MAP_VALUE(0x34)
#define MISC_E_FUSE_31_0_REG                       MISC_MEM_MAP_VALUE(0x40)
#define MISC_E_FUSE_63_32_REG                      MISC_MEM_MAP_VALUE(0x44)
#define MISC_E_FUSE_95_64_REG                      MISC_MEM_MAP_VALUE(0x48)
#define MISC_E_FUSE_127_96_REG                     MISC_MEM_MAP_VALUE(0x4C)
#define MISC_SOFTWARE_TEST_1_REG                   MISC_MEM_MAP_VALUE(0x50)
#define MISC_SOFTWARE_TEST_2_REG                   MISC_MEM_MAP_VALUE(0x54)



// USB MISC
#define  MISC_USB_CFG_REG			  MISC_MEM_MAP_VALUE(0x800)
#define  MISC_USB_STS_REG			  MISC_MEM_MAP_VALUE(0x804)
#define  MISC_USBPHY00_CFG_REG			  MISC_MEM_MAP_VALUE(0x808)
#define  MISC_USBPHY01_CFG_REG			  MISC_MEM_MAP_VALUE(0x80c)
#define  MISC_USBPHY10_CFG_REG			  MISC_MEM_MAP_VALUE(0x810)
#define  MISC_USBPHY11_CFG_REG			  MISC_MEM_MAP_VALUE(0x814)

#define MISC_PCIEPHY_CMCTL0_REG			   	MISC_MEM_MAP_VALUE(0x900)
#define MISC_PCIEPHY_CMCTL1_REG			   	MISC_MEM_MAP_VALUE(0x904)

#define MISC_PCIEPHY0_CTL_REG			   	MISC_MEM_MAP_VALUE(0x940)
#define MISC_PCIE0_AXIS_AWMISC_REG			MISC_MEM_MAP_VALUE(0x944)
#define MISC_PCIE0_AXIS_ARMISC_REG			MISC_MEM_MAP_VALUE(0x948)
#define MISC_PCIE0_AXIS_RMISC_REG			MISC_MEM_MAP_VALUE(0x94C)
#define MISC_PCIE0_AXIS_BMISC_REG			MISC_MEM_MAP_VALUE(0x950)
#define MISC_PCIE0_AXIM_RMISC_REG			MISC_MEM_MAP_VALUE(0x954)
#define MISC_PCIE0_AXIM_BMISC_REG			MISC_MEM_MAP_VALUE(0x958)
#define MISC_PCIE0_CTRL_REG			   	MISC_MEM_MAP_VALUE(0x95C)
#define MISC_PCIE0_PM_DEBUG_REG			   	MISC_MEM_MAP_VALUE(0x960)
#define MISC_PCIE0_RFC_DEBUG_REG			MISC_MEM_MAP_VALUE(0x964)
#define MISC_PCIE0_CXPL_DEBUGL_REG			MISC_MEM_MAP_VALUE(0x968)
#define MISC_PCIE0_CXPL_DEBUGH_REG			MISC_MEM_MAP_VALUE(0x96C)
#define MISC_PCIE0_DIAG_DEBUGH_REG			MISC_MEM_MAP_VALUE(0x970)
#define MISC_PCIE0_W1CLR_REG			        MISC_MEM_MAP_VALUE(0x974)
#define MISC_PCIE0_INT_MASK_REG			        MISC_MEM_MAP_VALUE(0x978)
#define MISC_PCIE0_INT_STATUS_REG			MISC_MEM_MAP_VALUE(0x97C)

#define MISC_PCIEPHY1_CTL_REG                           MISC_MEM_MAP_VALUE(0xa40)
#define MISC_PCIE1_AXIS_AWMISC_REG                      MISC_MEM_MAP_VALUE(0xa44)
#define MISC_PCIE1_AXIS_ARMISC_REG                      MISC_MEM_MAP_VALUE(0xa48)
#define MISC_PCIE1_AXIS_RMISC_REG                       MISC_MEM_MAP_VALUE(0xa4C)
#define MISC_PCIE1_AXIS_BMISC_REG                       MISC_MEM_MAP_VALUE(0xa50)
#define MISC_PCIE1_AXIM_RMISC_REG                       MISC_MEM_MAP_VALUE(0xa54)
#define MISC_PCIE1_AXIM_BMISC_REG                       MISC_MEM_MAP_VALUE(0xa58)
#define MISC_PCIE1_CTRL_REG                             MISC_MEM_MAP_VALUE(0xa5C)
#define MISC_PCIE1_PM_DEBUG_REG                         MISC_MEM_MAP_VALUE(0xa60)
#define MISC_PCIE1_RFC_DEBUG_REG                        MISC_MEM_MAP_VALUE(0xa64)
#define MISC_PCIE1_CXPL_DEBUGL_REG                      MISC_MEM_MAP_VALUE(0xa68)
#define MISC_PCIE1_CXPL_DEBUGH_REG                      MISC_MEM_MAP_VALUE(0xa6C)
#define MISC_PCIE1_DIAG_DEBUGH_REG                      MISC_MEM_MAP_VALUE(0xa70)
#define MISC_PCIE1_W1CLR_REG                            MISC_MEM_MAP_VALUE(0xa74)
#define MISC_PCIE1_INT_MASK_REG                         MISC_MEM_MAP_VALUE(0xa78)
#define MISC_PCIE1_INT_STATUS_REG                       MISC_MEM_MAP_VALUE(0xa7C)






/*
 * define constants macros
 */
#define MISC_PARALLEL_FLASH_BOOT             (0x0)
#define MISC_SPI_SERIAL_FLASH_BOOT           (0x1)
#define MISC_NAND_FLASH_BOOT                 (0x2)

#define MISC_ALIGN_LITTLE_ENDIAN                   (0x0)
#define MISC_UNALIGN_LITTLE_ENDIAN                 (0x2)
#define MISC_UNALIGN_BIG_ENDIAN                    (0x3)

#define MISC_CPU_CLOCK_333_MHZ               (0)
#define MISC_CPU_CLOCK_366_MHZ               (1)
#define MISC_CPU_CLOCK_400_MHZ               (2)
#define MISC_CPU_CLOCK_433_MHZ               (3)
#define MISC_CPU_CLOCK_466_MHZ               (4)
#define MISC_CPU_CLOCK_500_MHZ               (5)
#define MISC_CPU_CLOCK_533_MHZ               (6)
#define MISC_CPU_CLOCK_566_MHZ               (7)
#define MISC_CPU_CLOCK_600_MHZ               (8)
#define MISC_CPU_CLOCK_633_MHZ               (9)
#define MISC_CPU_CLOCK_666_MHZ               (10)
#define MISC_CPU_CLOCK_700_MHZ               (11)

/*
 * Macro-defines for shared pins with GPIO_A
 */
#if 0
#define MISC_LCD_PWR_PIN                     ((0x1 << 0))
#define MISC_CIM_OE_PIN                      ((0x1 << 1))

#define MISC_SMC_PINS                        ((0x1 << 2) | (0x1 << 3) | (0x1 << 4) | (0x1 << 5)| (0x1 << 6))
#define MISC_SMC_CS3_PIN                     ((0x1 << 2))
#define MISC_SMC_CS2_PIN                     ((0x1 << 3))
#define MISC_SMC_CLK_PIN                     ((0x1 << 4))
#define MISC_SMC_ADV_PIN                     ((0x1 << 5))
#define MISC_SMC_CRE_PIN                     ((0x1 << 6))


#define MISC_NFI_PINS                        ((0x1 << 7) | (0x1 << 8) | (0x1 << 9) | (0x1 << 10)| (0x1 << 11))
#define MISC_NFI_BUSY_PIN                    ((0x1 << 7))
#define MISC_NFI_CS3_PIN                     ((0x1 << 8))
#define MISC_NFI_CS2_PIN                     ((0x1 << 9))
#define MISC_NFI_CE1_PIN                     ((0x1 << 10))
#define MISC_NFI_CE0_PIN                     ((0x1 << 11))

#define MISC_EXT_INT2_PIN                    ((0x1 << 12))
#define MISC_EXT_INT1_PIN                    ((0x1 << 13))
#define MISC_EXT_INT0_PIN                    ((0x1 << 14))


#define MISC_UART0_PINS                      ((0x1 << 15) | (0x1 << 16) | (0x1 << 17) | (0x1 << 18))
#define MISC_UART0_RTS_PIN                   ((0x1 << 15))
#define MISC_UART0_CTS_PIN                   ((0x1 << 16))
#define MISC_UART0_TXD_PIN                   ((0x1 << 17))
#define MISC_UART0_RXD_PIN                   ((0x1 << 18))

#define MISC_UART1_PINS                      ((0x1 << 19) | (0x1 << 20) | (0x1 << 21) | (0x1 << 22))
#define MISC_UART1_RTS_PIN                   ((0x1 << 19))
#define MISC_UART1_CTS_PIN                   ((0x1 << 20))
#define MISC_UART1_RXD_PIN                   ((0x1 << 21))
#define MISC_UART1_TXD_PIN                   ((0x1 << 22))

#define MISC_UART2_PINS                      ((0x1 << 23) | (0x1 << 24))
#define MISC_UART2_RXD_PIN                   ((0x1 << 23))
#define MISC_UART2_TXD_PIN                   ((0x1 << 24))

#define MISC_PCM_PINS                        ((0x1 << 25) | (0x1 << 26) | (0x1 << 27) | (0x1 << 28))
#define MISC_PCM_CLK_PIN                     ((0x1 << 25))
#define MISC_PCM_FS_PIN                      ((0x1 << 26))
#define MISC_PCM_DT_PIN                      ((0x1 << 27))
#define MISC_PCM_DR_PIN                      ((0x1 << 28))

#define MISC_SPI_CS1_PIN                     ((0x1 << 29))
#define MISC_SPI_CS0_PIN                     ((0x1 << 30))
#define MISC_SPI_CLK_PIN                     ((0x1 << 31))
#else

#define MISC_UART0_PINS                      ((0x1 << 15) | (0x1 << 16) | (0x1 << 17) | (0x1 << 18))
#define MISC_UART0_RTS_PIN                   ((0x1 << 15))
#define MISC_UART0_CTS_PIN                   ((0x1 << 16))
#define MISC_UART0_TXD_PIN                   ((0x1 << 17))
#define MISC_UART0_RXD_PIN                   ((0x1 << 18))

#define MISC_UART1_PINS                      ((0x1 << 19) | (0x1 << 20) | (0x1 << 21) | (0x1 << 22))
#define MISC_UART1_RTS_PIN                   ((0x1 << 19))
#define MISC_UART1_CTS_PIN                   ((0x1 << 20))
#define MISC_UART1_RXD_PIN                   ((0x1 << 21))
#define MISC_UART1_TXD_PIN                   ((0x1 << 22))

#define MISC_UART2_PINS                      ((0x1 << 23) | (0x1 << 24))
#define MISC_UART2_RXD_PIN                   ((0x1 << 23))
#define MISC_UART2_TXD_PIN                   ((0x1 << 24))




#define MISC_SD_PWR_ON_PIN                   ((0x1 << 2))
#define MISC_OTG_DRVVBUS_PIN                 ((0x1 << 3))
#define MISC_CIM_OE_PIN                      ((0x1 << 8))
#define MISC_LCD_PWR_PIN                     ((0x1 << 9))
#define MISC_SMC_CS3_PIN                     ((0x1 << 10))
#define MISC_SMC_CS2_PIN                     ((0x1 << 11))
#define MISC_SMC_CLK_PIN                     ((0x1 << 12))
#define MISC_SMC_ADV_PIN                     ((0x1 << 13))
#define MISC_SMC_CRE_PIN                     ((0x1 << 14))
#define MISC_SMC_ADDR_26_PIN                 ((0x1 << 15))

#define MISC_SD_nCD_PIN                     ((0x1 << 16))
#define MISC_SD_nWP_PIN                     ((0x1 << 17))
#define MISC_SD_CLK_PIN                     ((0x1 << 18))
#define MISC_SD_CMD_PIN                     ((0x1 << 19))
#define MISC_SD_DT7_PIN                     ((0x1 << 20))
#define MISC_SD_DT6_PIN                     ((0x1 << 21))
#define MISC_SD_DT5_PIN                     ((0x1 << 22))
#define MISC_SD_DT4_PIN                     ((0x1 << 23))
#define MISC_SD_DT3_PIN                     ((0x1 << 24))
#define MISC_SD_DT2_PIN                     ((0x1 << 25))
#define MISC_SD_DT1_PIN                     ((0x1 << 26))
#define MISC_SD_DT0_PIN                     ((0x1 << 27))
#define MISC_SD_LED_PIN                     ((0x1 << 28))

#define MISC_UR_RXD1_PIN                    ((0x1 << 29))
#define MISC_UR_TXD1_PIN                    ((0x1 << 30))
#define MISC_UR_RTS2_PIN                    ((0x1 << 31))

#endif


/*
 * Macro-defines for shared pins with GPIO_B
 */
#if 0
#define MISC_SPI_DT_PIN                     ((0x1 << 0))
#define MISC_SPI_DR_PIN                     ((0x1 << 1))

#define MISC_SD_CD_PIN                      ((0x1 << 2))
#define MISC_SD_WP_PIN                      ((0x1 << 3))
#define MISC_SD_CLK_PIN                     ((0x1 << 4))
#define MISC_SD_CMD_PIN                     ((0x1 << 5))
#define MISC_SD_DT7_PIN                     ((0x1 << 6))
#define MISC_SD_DT6_PIN                     ((0x1 << 7))
#define MISC_SD_DT5_PIN                     ((0x1 << 8))
#define MISC_SD_DT4_PIN                     ((0x1 << 9))
#define MISC_SD_DT3_PIN                     ((0x1 << 10))
#define MISC_SD_DT2_PIN                     ((0x1 << 11))
#define MISC_SD_DT1_PIN                     ((0x1 << 12))
#define MISC_SD_DT0_PIN                     ((0x1 << 13))
#define MISC_SD_LED_PIN                     ((0x1 << 14))


#define MISC_I2S_CLK_PIN                     ((0x1 << 15))
#define MISC_I2S_FS_PIN                      ((0x1 << 16))
#define MISC_I2S_DT_PIN                      ((0x1 << 17))
#define MISC_I2S_DR_PIN                      ((0x1 << 18))

//Tim.Liao modify
#define MISC_I2C_SCL_PIN                     ((0x1 << 19))
#define MISC_I2C_SDA_PIN                     ((0x1 << 20))

#define MISC_GSW_P2_CRS_PIN                  ((0x1 << 21))
#define MISC_GSW_P2_COL_PIN                  ((0x1 << 22))
#define MISC_GSW_P1_CRS_PIN                  ((0x1 << 23))
#define MISC_GSW_P1_COL_PIN                  ((0x1 << 24))
#define MISC_GSW_P0_CRS_PIN                  ((0x1 << 25))
#define MISC_GSW_P0_COL_PIN                  ((0x1 << 26))

#define MISC_GSW_MDC_PIN                     ((0x1 << 27))
#define MISC_GSW_MDIO_PIN                    ((0x1 << 28))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))
#else
#define MISC_UR_CTS2_PIN                    ((0x1 << 0))
#define MISC_UR_RXD2_PIN                    ((0x1 << 1))
#define MISC_UR_TXD2_PIN                    ((0x1 << 2))
#define MISC_PCMCLK_PIN                     ((0x1 << 3))
#define MISC_PCMFS_PIN                     ((0x1 << 4))
#define MISC_PCMDT_PIN                     ((0x1 << 5))
#define MISC_PCMDR_PIN                     ((0x1 << 6))
#define MISC_PCM_PINS						(MISC_PCMCLK_PIN|MISC_PCMFS_PIN|MISC_PCMDT_PIN|MISC_PCMDR_PIN)

#define MISC_SPInCS1_PIN                     ((0x1 << 7))
#define MISC_SPInCS0_PIN                     ((0x1 << 8))
#define MISC_SPICLK_PIN                      ((0x1 << 9))
#define MISC_SPIDT_PIN                      ((0x1 << 10))
#define MISC_SPIDR_PIN                      ((0x1 << 11))

#define MISC_I2C_SCL_PIN                     ((0x1 << 12))
#define MISC_I2C_SDA_PIN                     ((0x1 << 13))

#define MISC_GSW_P2_CRS_PIN                  ((0x1 << 14))
#define MISC_GSW_P2_COL_PIN                  ((0x1 << 15))
#define MISC_GSW_P1_CRS_PIN                  ((0x1 << 16))
#define MISC_GSW_P1_COL_PIN                  ((0x1 << 17))
#define MISC_GSW_P0_CRS_PIN                  ((0x1 << 18))
#define MISC_GSW_P0_COL_PIN                  ((0x1 << 19))

#define MISC_GSW_MDC_PIN                     ((0x1 << 20))
#define MISC_GSW_MDIO_PIN                    ((0x1 << 21))

#define MISC_I2S_CLK_PIN                     (0x1 << 22)
#define MISC_I2S_FS_PIN                      (0x1 << 23)
#define MISC_I2S_DT_PIN                      (0x1 << 24)
#define MISC_I2S_DR_PIN                      (0x1 << 25)

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#define MISC_CLOCK_OUTPUT_PIN                ((0x1 << 26))

#define MISC_EXT_INT2_PIN                    ((0x1 << 27))
#define MISC_EXT_INT1_PIN                    ((0x1 << 28))
#define MISC_EXT_INT0_PIN                    ((0x1 << 29))

#define MISC_SATA_LED1_PIN                   ((0x1 << 30))
#define MISC_SATA_LED0_PIN                   ((0x1 << 31))

#endif
/*
 * Other defines
 */
#define MISC_GPIOA_PIN_0                     (0)
#define MISC_GPIOA_PIN_1                     (1)
#define MISC_GPIOA_PIN_2                     (2)
#define MISC_GPIOA_PIN_3                     (3)
#define MISC_GPIOA_PIN_4                     (4)
#define MISC_GPIOA_PIN_5                     (5)
#define MISC_GPIOA_PIN_6                     (6)
#define MISC_GPIOA_PIN_7                     (7)
#define MISC_GPIOA_PIN_8                     (8)
#define MISC_GPIOA_PIN_9                     (9)
#define MISC_GPIOA_PIN_10                    (10)
#define MISC_GPIOA_PIN_11                    (11)
#define MISC_GPIOA_PIN_12                    (12)
#define MISC_GPIOA_PIN_13                    (13)
#define MISC_GPIOA_PIN_14                    (14)
#define MISC_GPIOA_PIN_15                    (15)


#define MISC_GPIOA_RESISTOR_PULL_DOWN        (1)
#define MISC_GPIOA_RESISTOR_PULL_UP          (1)



/*
 * function declarations
 */


/*
 * macro declarations
 */
#define HAL_MISC_GET_SYSTEM_ALIGN_ENDIAN_MODE(mode) \
{ \
    (mode) = (MISC_CHIP_CONFIG_REG) & 0x3; \
}


#define HAL_MISC_GET_SYSTEM_CPU_CLOCK(cpu_clock) \
{ \
    (cpu_clock) = (MISC_CHIP_CONFIG_REG >> 5) & 0xF; \
}


#define HAL_MISC_ENABLE_SPI_SERIAL_FLASH_BANK_ACCESS() \
{ \
    (MISC_CHIP_CONFIG_REG) |= (0x1 << 16); \
}

#define HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS() \
{ \
    (MISC_CHIP_CONFIG_REG) &= ~(0x1 << 16); \
}


/*
 * Macro defines for GPIOA and GPIOB Pin Enable Register
 */
#define HAL_MISC_ENABLE_EXT_INT0_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_EXT_INT0_PIN); \
}

#define HAL_MISC_DISABLE_EXT_INT1_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_EXT_INT1_PIN); \
}

#define HAL_MISC_ENABLE_EXT_INT2_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_EXT_INT2_PIN); \
}

#define HAL_MISC_DISABLE_EXT_INT2_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_EXT_INT2_PIN); \
}

#define HAL_MISC_ENABLE_EXT_INT1_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_EXT_INT1_PIN); \
}

#define HAL_MISC_DISABLE_EXT_INT0_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_EXT_INT0_PIN); \
}


#define HAL_MISC_ENABLE_PCM_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_PCM_PINS); \
}

#define HAL_MISC_DISABLE_PCM_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_PCM_PINS); \
}


#define HAL_MISC_ENABLE_CIM_OE_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_CIM_OE_PIN); \
}

#define HAL_MISC_DISABLE_CIM_OE_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_CIM_OE_PIN); \
}


#define HAL_MISC_ENABLE_LCD_PWR_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_LCD_PWR_PIN); \
}

#define HAL_MISC_DISABLE_LCD_PWR_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_LCD_PWR_PIN); \
}


#define HAL_MISC_ENABLE_NFI_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_NFI_PINS); \
}

#define HAL_MISC_DISABLE_NFI_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_NFI_PINS); \
}



#define HAL_MISC_ENABLE_SMC_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_SMC_PINS); \
}

#define HAL_MISC_DISABLE_SMC_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_SMC_PINS); \
}

#define HAL_MISC_ENABLE_UART0_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_UART0_PINS); \
}

#define HAL_MISC_DISABLE_UART0_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_UART0_PINS); \
}

#define HAL_MISC_ENABLE_UART1_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_UART1_PINS); \
}

#define HAL_MISC_DISABLE_UART1_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_UART1_PINS); \
}

#define HAL_MISC_ENABLE_UART2_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_UART2_PINS); \
}

#define HAL_MISC_DISABLE_UART2_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_UART2_PINS); \
}





/*
 * Macro-defines for GPIO_B
 */
#define HAL_MISC_ENABLE_SPI_PINS() \
{ \
	(MISC_GPIOB_PIN_ENABLE_REG) |= \
							(MISC_SPInCS1_PIN | MISC_SPInCS0_PIN | \
							MISC_SPICLK_PIN | MISC_SPIDT_PIN | MISC_SPIDR_PIN); \
}

#define HAL_MISC_DISABLE_SPI_PINS() \
{ \
	(MISC_GPIOB_PIN_ENABLE_REG) &= \
							~(MISC_SPInCS1_PIN | MISC_SPInCS0_PIN | \
							MISC_SPICLK_PIN | MISC_SPIDT_PIN | MISC_SPIDR_PIN); \
}

#define HAL_MISC_ENABLE_SD_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_SD_CD_PIN | MISC_SD_WP_PIN | MISC_SD_CLK_PIN |MISC_SD_CMD_PIN |MISC_SD_DT7_PIN|MISC_SD_DT6_PIN | \
                                    MISC_SD_DT5_PIN | MISC_SD_DT4_PIN |MISC_SD_DT3_PIN | MISC_SD_DT2_PIN| MISC_SD_DT1_PIN | MISC_SD_DT0_PIN | MISC_SD_LED_PIN); \
}

#define HAL_MISC_DISABLE_SD_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_SD_CD_PIN | MISC_SD_WP_PIN | MISC_SD_CLK_PIN |MISC_SD_CMD_PIN |MISC_SD_DT7_PIN|MISC_SD_DT6_PIN |\
                                    MISC_SD_DT5_PIN | MISC_SD_DT4_PIN |MISC_SD_DT3_PIN | MISC_SD_DT2_PIN| MISC_SD_DT1_PIN | MISC_SD_DT0_PIN | MISC_SD_LED_PIN); \
}


#define HAL_MISC_ENABLE_I2S_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_I2S_CLK_PIN | MISC_I2S_FS_PIN | MISC_I2S_DT_PIN |MISC_I2S_DR_PIN |MISC_I2S_DR_PIN); \
}

#define HAL_MISC_DISABLE_I2S_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_I2S_CLK_PIN | MISC_I2S_FS_PIN | MISC_I2S_DT_PIN |MISC_I2S_DR_PIN |MISC_I2S_DR_PIN); \
}

//Tim.Liao modify I2C pin
#define HAL_MISC_ENABLE_I2C_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) |= (MISC_I2C_SCL_PIN | MISC_I2C_SDA_PIN); \
}

#define HAL_MISC_DISABLE_I2C_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) &= ~(MISC_I2C_SCL_PIN | MISC_I2C_SDA_PIN); \
}

#define HAL_MISC_ENABLE_GSW_P2_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_GSW_P2_CRS_PIN | MISC_GSW_P2_COL_PIN); \
}

#define HAL_MISC_DISABLE_GSW_P2_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_GSW_P2_CRS_PIN | MISC_GSW_P2_COL_PIN); \
}


#define HAL_MISC_ENABLE_GSW_P1_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_GSW_P1_CRS_PIN | MISC_GSW_P1_COL_PIN); \
}

#define HAL_MISC_DISABLE_GSW_P1_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_GSW_P1_CRS_PIN | MISC_GSW_P1_COL_PIN); \
}



#define HAL_MISC_ENABLE_GSW_P0_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_GSW_P0_CRS_PIN | MISC_GSW_P0_COL_PIN); \
}

#define HAL_MISC_DISABLE_GSW_P0_CRS_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_GSW_P0_CRS_PIN | MISC_GSW_P0_COL_PIN); \
}


#define HAL_MISC_ENABLE_MDC_MDIO_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_GSW_MDC_PIN | MISC_GSW_MDIO_PIN); \
}

#define HAL_MISC_DISABLE_MDC_MDIO_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_GSW_MDC_PIN | MISC_GSW_MDIO_PIN); \
}



#define HAL_MISC_ENABLE_SATA_LED_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_SATA_LED1_PIN | MISC_SATA_LED0_PIN); \
}

#define HAL_MISC_DISABLE_SATA_LED_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_SATA_LED1_PIN | MISC_SATA_LED0_PIN); \
}



#define HAL_MISC_ENABLE_CLOCK_OUTPUT_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) |= (MISC_CLOCK_OUTPUT_PIN); \
}

#define HAL_MISC_DISABLE_CLOCK_OUTPUT_PIN() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG) &= ~(MISC_CLOCK_OUTPUT_PIN); \
}


#define HAL_MISC_ENABLE_ALL_SHARED_GPIO_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) = (0x0); \
    (MISC_GPIOB_PIN_ENABLE_REG) = (0x0); \
}

#define HAL_MISC_DISABLE_ALL_SHARED_GPIO_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG) = (0xFFFFFFFF); \
    (MISC_GPIOB_PIN_ENABLE_REG) = (0xFFFFFFFF); \
}



#endif  // end of #ifndef _STAR_MISC_H_
