/* $Id: //depot/software/SDK/Triscend/a7hal/include/triscend_a7v.h#27 $ */
/*
 ***************************************************************************
 *  triscend_a7v.h
 *
 *  Copyright (c) 2002-2003 Triscend Corporation. All rights reserved.
 *
 ***************************************************************************
 */
#ifndef _TRISCEND_A7V_H
#define _TRISCEND_A7V_H


/*
 **********************************************************
 *  Field and Bit Manipulation Defines and Macros
 **********************************************************
 */
#define A7_REG(reg) \
    (*(volatile unsigned int*)(reg))

#define BIT_MASK(bitIdx) \
    (1 << (bitIdx))

#define FIELD_MASK(fieldIdx,nBits) \
    (((1 << (nBits)) - 1) << (fieldIdx))

#define GET_BIT(addr32,bitIdx) \
    (*(volatile unsigned int*)(addr32) & (1 << (bitIdx)) ? 1 : 0)

#define PUT_BIT(addr32,bitIdx,bitVal) \
    (*(volatile unsigned int*)(addr32) = (0 == (bitVal)) ? \
     *(volatile unsigned int*)(addr32) & ~(1 << (bitIdx)) : \
     *(volatile unsigned int*)(addr32) | (1 << (bitIdx)))

#define SET_BIT(addr32,bitIdx) \
    (*(volatile unsigned int*)(addr32) |= (1 << (bitIdx)))

#define CLR_BIT(addr32,bitIdx) \
    (*(volatile unsigned int*)(addr32) &= ~(1 << (bitIdx)))

#define GET_FIELD(addr32,fieldIdx,nBits) \
    ((*(volatile unsigned int*)(addr32) >> (fieldIdx)) & ((1 << (nBits)) - 1))

#define PUT_FIELD(addr32,fieldIdx,nBits,fieldValue) \
    (*(volatile unsigned int*)(addr32) = \
    (*(volatile unsigned int*)(addr32) & ~FIELD_MASK(fieldIdx,nBits)) | \
    (((fieldValue) << (fieldIdx)) & FIELD_MASK(fieldIdx,nBits)))

#define SET_FIELD(addr32,fieldIdx,nBits) \
    (*(volatile unsigned int*)(addr32) |= FIELD_MASK(fieldIdx,nBits))

#define CLR_FIELD(addr32,fieldIdx,nBits) \
    (*(volatile unsigned int*)(addr32) &= ~FIELD_MASK(fieldIdx,nBits))

/*
 **********************************************************
 *  Memory Map
 **********************************************************
 */
#define INTERNAL_ROM_BASE           0xd1000000UL
#define INTERNAL_ROM_SIZE           0x00010000UL
#define INTERNAL_RAM_BASE           0xd1030000UL
#define INTERNAL_RAM_SIZE           0x00008000UL
#define CONTROL_REG_BASE            0xd1010000UL
#define CONTROL_REG_SIZE            0x00010000UL
#define TEST_AREA_BASE              0xd1020000UL
#define TEST_AREA_SIZE              0x00010000UL
#define CONFIG_MEM_BASE             0xd1040000UL
#define CONFIG_MEM_SIZE             0x00040000UL
#define CSLDMA_BASE                 0xb0000000UL

#define EXTERNAL_SDRAM_BASE         0xc0000000UL
#define EXTERNAL_SDRAM_SIZE         0x10000000UL
#define EXTERNAL_FLASH_RAM_0_BASE   0xd0000000UL
#define EXTERNAL_FLASH_RAM_A_BASE   0xd2000000UL        // aliased FLASH_RAM_0
#define EXTERNAL_FLASH_RAM_1_BASE   0xd3000000UL
#define EXTERNAL_FLASH_RAM_SIZE     0x01000000UL

/*
 **********************************************************
 * For backward compatibility with A7S
 **********************************************************
 */
#define EXTERNAL_FLASH_BASE         EXTERNAL_FLASH_RAM_0_BASE
#define EXTERNAL_FLASH_SIZE         EXTERNAL_FLASH_RAM_SIZE

/*
 **********************************************************
 *  Control Register Base Address Definitions
 **********************************************************
 */
#define MSS_BASE                    0xd1010000UL
#define SYS_BASE                    0xd1010100UL
#define INT_BASE                    0xd1010200UL
#define REMAP_BASE                  0xd1010400UL
#define TIMER_BASE                  0xd1010500UL
#define WD_BASE                     0xd1010600UL
#define CFG_BASE                    0xd1010700UL
#define DMA_BASE                    0xd1010800UL
#define UART_BASE                   0xd1010900UL
#define BPU_BASE                    0xd1010a00UL
#define PU_BASE                     0xd1011100UL

#define DMAX_BASE                   0xd1010c00UL
#define TWSI0_BASE                  0xd1010d00UL
#define TWSI1_BASE                  0xd1010e00UL
#define ENET0_BASE                  0xd1011200UL
#define SSI_BASE                    0xd1011300UL
#define MFTA_BASE                   0xd1011400UL
#define ADC_BASE                    0xd1011500UL
#define CAN_BASE                    0xd1011800UL
#define ENET1_BASE                  0xd1011C00UL
#define USB_BASE                    0xd1012000UL


/*
 **********************************************************
 *  Register Access Attributes
 **********************************************************
 */
#define READ_ACCESS                 1
#define WRITE_ACCESS                2

/*
 **********************************************************
 * Memory Subsystem Unit Definition
 **********************************************************
 */

/*
 *  Memory Subsystem Configuration Register (read/write)
 *      Device Width encoding:
 *          00: Byte     ( 8bit)
 *          01: Halfword (16bit)
 *          10: Word     (32bit)
 *          11: Reserved
 */
#define MSS_CONFIG_REG              (MSS_BASE + 0x00)
#define MSS_CONFIG_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define MSS_CONFIG_RESET_VALUE      0x00000002UL
#define BUS_MODE_FIELD              0
#define   NBITS_BUS_MODE              4
#define MIU_DEV_WIDTH_FIELD         4
#define   NBITS_MIU_DEV_WIDTH         2
#define SDIU_DEV_WIDTH_FIELD        6
#define   NBITS_SDIU_DEV_WIDTH        2
#define R_MAP_FIELD                 8
#define   NBITS_R_MAP                 2
#define B_MAP_FIELD                 10
#define   NBITS_B_MAP                 3
#define N_BANK_BIT                  13
#define NE_BANK_BIT                 14
#define PIPE_BIT                    15
#define DMA_0_BUF_EN_BIT            16
#define DMA_1_BUF_EN_BIT            17
#define DMA_2_BUF_EN_BIT            18
#define DMA_3_BUF_EN_BIT            19
#define DMA_4_BUF_EN_BIT            20
#define DMA_5_BUF_EN_BIT            21
#define DMA_6_BUF_EN_BIT            22
#define DMA_BUF_EN_RESERVED_BIT     23
#define MSS_CONFIG_RESERVED_24      24
#define R_MAP_BIT_2                 26


/*
 *  CS1 Memory Subsystem Configuration Register (read/write)
 *      Device Width encoding:
 *          00: Byte     ( 8bit)
 *          01: Halfword (16bit)
 *          10: Word     (32bit)
 *          11: Reserved
 *      CS1 Size Field encoding:
 *          0xff:  64K
 *          0xfe: 128K
 *          0xfc: 256K
 *          0xf8: 512K
 *          0xf0:   1M
 *          0xe0:   2M
 *          0xc0:   4M
 *          0x80:   8M
 *          0x00:  16M
 */
#define MSS_CS1_CONFIG_REG          (MSS_BASE + 0x18)
#define MSS_CS1_CONFIG_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define MSS_CS1_CONFIG_RESET_VALUE  0x00000002UL
#define CS1_DEV_WIDTH_FIELD         0
#define   NBITS_CS1_DEV_WIDTH         2
#define CS1_SIZE_FIELD              8
#define   NBITS_CS1_SIZE              8


/*
 *  Static Memory Interface Timing Control Registers (read/write)
 */
#define MSS_CS0_TIM_CTRL_REG        (MSS_BASE + 0x04)
#define MSS_CS1_TIM_CTRL_REG        (MSS_BASE + 0x1c)
#define MSS_TIM_CTRL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define MSS_TIM_CTRL_RESET_VALUE    0x00077777UL
#define WC_SETUP_FIELD              0
#define   NBITS_WC_SETUP              4
#define WC_WIDTH_FIELD              4
#define   NBITS_WC_WIDTH              4
#define WC_HOLD_FIELD               8
#define   NBITS_WC_HOLD               4
#define RC_SETUP_FIELD              12
#define   NBITS_RC_SETUP              4
#define RC_WIDTH_FIELD              16
#define   NBITS_RC_WIDTH              4


/*
 *  SDRAM Mode Register (read/write)
 */
#define MSS_SDR_MODE_REG            (MSS_BASE + 0x08)
#define MSS_SDR_MODE_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define MSS_SDR_MODE_RESET_VALUE    0x02223222UL
#define TRP_FIELD                   0
#define   NBITS_TRP                   3
#define TRCD_FIELD                  4
#define   NBITS_TRCD                  3
#define TWR_FIELD                   8
#define   NBITS_TWR                   3
#define TRC_FIELD                   12
#define   NBITS_TRC                   3
#define MODE_REG_FIELD              16
#define   NBITS_MODE_REG              14


/*
 *  SDRAM Control Register (read/write)
 */
#define MSS_SDR_CTRL_REG            (MSS_BASE + 0x0c)
#define MSS_SDR_CTRL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define MSS_SDR_CTRL_RESET_VALUE    0x00000000UL
#define PWR_MAN_FIELD               0
#define   NBITS_PWR_MAN               3
#define REF_EN_BIT                  3
#define RFSH_RATE_FIELD             16
#define   NBITS_RFSH_RATE             12
#define RFSH_BURST_FIELD            28
#define   NBITS_RFSH_BURST            4


/*
 *  SDRAM Status Register (read only)
 *      SDRAM Status Field interpretation:
 *          000: SDRAM controller is in normal operation
 *          001: SDRAM controller is disabled
 *          010: SDRAM controller is in self-refresh mode
 *          100: SDRAM controller is in stand-by mode
 */
#define MSS_STATUS_REG              (MSS_BASE + 0x10)
#define MSS_STATUS_REG_ACCESS       (READ_ACCESS)
#define MSS_STATUS_RESET_VALUE      0x00000300UL
#define SD_STATUS_FIELD             0
#define   NBITS_SD_STATUS             3
#define RFSH_OVF_BIT                3


/*
 *  SDRAM Status Clear Register (write only)
 *      write 1 to clear the corresponding status reg bit
 */
#define MSS_STATUS_CLEAR_REG        (MSS_BASE + 0x14)
#define MSS_STATUS_CLEAR_REG_ACCESS (WRITE_ACCESS)
#define RFSH_OVF_CLR_BIT            3

/*
 **********************************************************
 *  System Control Registers
 **********************************************************
 */

/*
 *  Clock Control Register (read/write)
 */
#define SYS_CLOCK_CONTROL_REG       (SYS_BASE + 0x00)
#define SYS_CLOCK_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CLOCK_CONTROL_RESET_VALUE   0x00000200UL
#define CLK_SEL_BIT                 0
#define PLL_SEL_BIT                 1
#define CSL_SEL_BIT                 2
#define CK_EN_BIT                   3
#define XTAL_EN_BIT                 4
#define OSC_SMT_BIT                 8
#define OSC_FEB_BIT                 9
#define OSC_EB_BIT                  10
#define OSC_S_BIT                   11


/*
 *  PLL Control Register (read/write)
 *      PLL lock: wait 500 reference cycles after de-asserting PLL reset
 *      CLK_freq_out = CLK_reference * (CLKF+1) / ( (CLKR+1) * (CLKOD+1) )
 */
#define SYS_PLL_CONTROL_REG         (SYS_BASE + 0x14)
#define SYS_PLL_CONTROL_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define PLL_CONTROL_RESET_VALUE     0x00000003UL
#define PLL_RESET_BIT               0
#define PLL_POWERDOWN_BIT           1
#define PLL_BYPASS_BIT              2
#define PLL_TEST_BIT                3
#define PLL_REFSEL_BIT              4
#define PLL_CLKF_FIELD              8   // PLL Feedback Divider
#define   NBITS_PLL_CLKF              12
#define PLL_CLKOD_FIELD             20  // PLL Output Divider
#define   NBITS_PLL_CLKOD             3
#define PLL_CLKR_FIELD              24
#define   NBITS_PLL_CLKR              6 // PLL Reference divider


/*
 *  PLL Status Register (read only)
 */
#define SYS_PLL_STATUS_REG          (SYS_BASE + 0x04)
#define SYS_PLL_STATUS_REG_ACCESS   (READ_ACCESS)
#define PLL_STATUS_RESET_VALUE      0x00000001UL
#define PLL_NOT_LOCK_BIT            0


/*
 *  PLL Status Clear Register (write only)
 *      Write a '1' to clear the NotLock bit in the PLL status register
 */
#define SYS_PLL_STATUS_CLEAR_REG    (SYS_BASE + 0x08)
#define SYS_PLL_STATUS_CLEAR_REG_ACCESS (WRITE_ACCESS)
#define PLL_NOT_LOCK_CLEAR_BIT      0


/*
 *  Reset Control Register (read/write)
 *      Write a '1' to assert the System Reset.
 *      The System Reset bit is self-clearing.
 */
#define SYS_RESET_CONTROL_REG       (SYS_BASE + 0x0c)
#define SYS_RESET_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define RESET_CONTROL_RESET_VALUE   0x00000000UL
#define SLAVE_DIS_BIT               1
#define SYS_RESET_BIT               2


/*
 *  Power Down Control Register (read/write)
 *      User can select which portion of the device is
 *      turned off or kept on during power down mode.
 *      Setting a bit turns off the circuit in power down.
 *      A zero keeps the circuit running at all times.
 */
#define SYS_POWER_CONTROL_REG       (SYS_BASE + 0x10)
#define SYS_POWER_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define POWER_CONTROL_RESET_VALUE   0x00000000UL
#define PD_BCK_EN_BIT               0
#define PD_CSL_BCK_EN_BIT           1
#define PD_OSC_EN_BIT               2
#define PD_IO_EN_BIT                3
#define PD_BIT                      4
#define POR_DIS_BIT                 5


/*
 **********************************************************
 *  Remap Control Registers
 **********************************************************
 */

/*
 *  Pause Register (write only)
 *      Writing to this register puts the ARM core (only) in
 *      a low power state until receiving an interrupt.
 */
#define REMAP_PAUSE_REG             (REMAP_BASE + 0x00)
#define REMAP_PAUSE_ACCESS          (WRITE_ACCESS)


/*
 *  Identification Register (read only)
 */
#define REMAP_IDENTIFICATION_REG    (REMAP_BASE + 0x10)
#define REMAP_IDENTIFICATION_REG_ACCESS (READ_ACCESS)
#define TRISCEND_A7Vx05             0x08c9d2ffUL


/*
 *  Revision Register (read only)
 */
#define REMAP_REVISION_REG          (REMAP_BASE + 0x14)
#define REMAP_REVISION_REG_ACCESS   (READ_ACCESS)
#define SEMI_CUSTOM_REV_FIELD       0
#define   NBITS_SEMI_CUSTOM_REV       4
#define ROM_REV_FIELD               4
#define   NBITS_ROM_REV               4
#define CSL_REV_FIELD               8
#define   NBITS_CSL_REV               4
#define IO_REV_FIELD                12
#define   NBITS_IO_REV                4
#define MISC_REV_FIELD              16
#define   NBITS_MISC_REV              4


/*
 *  Clear Reset Map Register (write only)
 *      Writing to this address causes a system memory map switch from the
 *      user initial memory map to the one used during normal operation.
 *      It effectively clears the Flash alias bit in the Alias Enable Register.
 */
#define REMAP_CLEAR_RESET_MAP_REG   (REMAP_BASE + 0x20)
#define REMAP_CLEAR_RESET_MAP_REG_ACCESS (WRITE_ACCESS)


/*
 *  Reset Status Register (read only)
 *      Indicates the cause of the latest reset.
 *      Default reset value depends on reset cause.
 */
#define REMAP_RESET_STATUS_REG      (REMAP_BASE + 0x30)
#define REMAP_RESET_STATUS_REG_ACCESS (READ_ACCESS)
#define POR_BIT                     0
#define CFG_RST_BIT                 1
#define RST_PIN_BIT                 2
#define J_CFG_RST_BIT               3
#define CPU_RST_BIT                 4
#define J_CPU_RST_BIT               5
#define WD_RST_BIT                  6
#define APP_RST_BIT                 7
#define SYS_RST_BIT                 8
#define J_SYS_RST_BIT               9
#define SOFT_RST_BIT                10


/*
 *  Reset Status Clear Register (write only)
 *      Writing a "1" clears the corresponding bit in Reset Status Register.
 *      Writing a "0" has no effect.
 */
#define REMAP_RESET_STATUS_CLEAR_REG (REMAP_BASE + 0x34)
#define REMAP_RESET_STATUS_CLEAR_REG_ACCESS (WRITE_ACCESS)
#define POR_CLR_BIT                 0
#define CFG_RST_CLR_BIT             1
#define RST_PIN_CLR_BIT             2
#define J_CFG_RST_CLR_BIT           3
#define CPU_RST_CLR_BIT             4
#define J_CPU_RST_CLR_BIT           5
#define WD_RST_CLR_BIT              6
#define APP_RST_CLR_BIT             7
#define SYS_RST_CLR_BIT             8
#define J_SYS_RST_CLR_BIT           9
#define SOFT_RST_CLR_BIT            10


/*
 *  Pin Status Register (read only)
 *      This register enables software to get the status of
 *      some static pins of the device.
 */
#define REMAP_PIN_STATUS_REG        (REMAP_BASE + 0x38)
#define REMAP_PIN_STATUS_REG_ACCESS (READ_ACCESS)
#define VSYS_BIT                    0
#define RSTN_BIT                    1
#define SLAVEN_BIT                  2
#define VSYS_GOOD_BIT               3
#define VSYS_BAD_BIT                4


/*
 *  Pin Status Clear Register (write only)
 *      Writing a "1" clears the corresponding bit in the Pin Status Register.
 *      Writing a "0" has no effect.
 */
#define REMAP_PIN_STATUS_CLEAR_REG  (REMAP_BASE + 0x3c)
#define REMAP_PIN_STATUS_CLEAR_REG_ACCESS (WRITE_ACCESS)
#define VSYS_GOOD_CLR_BIT           3
#define VSYS_BAD_CLR_BIT            4


/*
 *  Alias Enable Register (read/write)
 *      This register defines which alias is enabled at the bottom of the
 *      memory starting at address 0. If more than one alias is enabled,
 *      they are overlaid over each other with the following priority
 *      (from highest to lowest priority):
 *      1. Internal ROM
 *      2. FLASH (CS0)
 *      3. SRAM
 *      4. FLASH (CS1)
 *      5. SDRAM
 */
#define REMAP_ALIAS_ENABLE_REG      (REMAP_BASE + 0x40)
#define REMAP_ALIAS_ENABLE_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define ALIAS_ENABLE_RESET_VALUE    0x0000001fUL
#define ROM_AEN_BIT                 0
#define FLASH_AEN_BIT               1
#define SRAM_AEN_BIT                2
#define SDRAM_AEN_BIT               3
#define CS1_AEN_BIT                 4


/*
 *  Scratchpad SRAM Config Register (read/write)
 *	    The SRAM size bit is set to 1 when tracing, thus blocking
 *      CPU & CSI access to upper 8K SRAM (for use as trace buffer).
 *          bit = 0 : SRAM size = 16K
 *		    bit = 1 : SRAM size =  8K (lower)
 *      Each SRAM protection bit protects a 4KB block from CSI accesses.
 */
#define REMAP_SRAM_CONFIG_REG       (REMAP_BASE + 0x44)
#define REMAP_SRAM_CONFIG_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define SRAM_CONFIG_RESET_VALUE     0x00000000UL
#define SRAM_PRIO_BIT               0
#define SRAM_TRACE_EN_BIT           1
#define SRAM_PROTECT_FIELD          2
#define   NBITS_SRAM_PROTECT          8
#define SRAM_ORG_BIT               10

/*
 *  Scratchpad SRAM Base Address Register (read/write)
 *      Used to relocate the internal SRAM (scratchpad).
 */
#define REMAP_SRAM_BASE_ADR_REG     (REMAP_BASE + 0x48)
#define REMAP_SRAM_BASE_ADR_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define SRAM_BASE_ADR_RESET_VALUE   0xd1030000UL
#define SRAM_BASE_ADR_FIELD         14
#define   NBITS_SRAM_BASE_ADR         18


/*
 *  Access Protect Register (read/write)
 *      Setting dmaDis bit disallows DMA writes into Control Register area.
 *      Setting testEn bit allows access to co-processor registers and Cache.
 */
#define REMAP_ACC_PROTECT_REG       (REMAP_BASE + 0x4c)
#define REMAP_ACC_PROTECT_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define ACC_PROTECT_RESET_VALUE     0x00000001UL
#define DMA_DIS_BIT                 0
#define TEST_EN_BIT                 1

/*
 * USB Configuration Register.
 */
#define USB_CONFIG_REG              (REMAP_BASE + 0x50)
#define USB_CONFIG_ACCESS           (READ_ACCESS | WRITE_ACCESS)
#define USB_CONFIG_RESET_VALUE      0x00000000UL
#define USB_RESET_BIT               0
#define USB_ENABLE_BIT              1
#define USB_INTERNAL_PHY_ENABLE_BIT 2
#define USB_BUS_CLOCK_SELECT_BIT    3

/*
 * USB PHY Test Register.
 */
#define USB_PHY_TEST_REG            (REMAP_BASE + 0x54)
#define USB_PHY_TEST_ACCESS         (READ_ACCESS | WRITE_ACCESS)
#define USB_PHY_TEST_RESET_VALUE    0x00000000UL
#define USB_RAM_TEST_ENABLE_BIT     0
#define USB_PHY_TEST_ENABLE_BIT     1
#define USB_PHY_VPO_BIT             2
#define USB_PHY_VMO_BIT             3
#define USB_PHY_OEB_BIT             4
#define USB_PHY_EB_BIT              5
#define USB_PHY_SP_BIT              6
#define USB_PHY_VP_BIT              8
#define USB_PHY_VM_BIT              9
#define USB_PHY_RCV_BIT             10

/*
 **********************************************************
 *  Configuration Unit Definition
 **********************************************************
 */

/*
 *  Configuration Control Register (read/write)
 *      I/O Enable    : used to disable PIO during configuration.
 *      Config Done   : used to blank signals coming from CSL during config.
 *      Decode Enable : used to force '0' on CSL row and column addresses.
 *      Config Stop   : used to gate off clocks used for config (saves power).
 */
#define CFG_CONFIG_CONTROL_REG      (CFG_BASE + 0x00)
#define CFG_CONFIG_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CONFIG_CONTROL_RESET_VALUE  0x00000000UL
#define EN_ZIP_BIT                  0
#define EN_REG_BIT                  1
#define EN_IO_BIT                   2
#define CONFIG_DONE_BIT             4
#define DEC_EN_BIT                  5
#define CFGENS_EN_BIT               6
#define CFG_STOP_BIT                7
#define CFG_CRC_EN_BIT              8
#define CFG_CRC_RSTN_BIT            9
#define ADEC_TST_BIT                10


/*
 *  Configuration Timing Register (read/write)
 *      CSL Configuration Access Wait States:
 *          generated by config unit in response to CSL reads.
 *          number of wait states = cslWait
 *      PIO Phase Delay:
 *          length in clock cycles of PIO config read & write phases.
 *          length in clock cycles = (pioPhase - 1)
 */
#define CFG_CONFIG_TIMING_REG       (CFG_BASE + 0x04)
#define CFG_CONFIG_TIMING_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CONFIG_TIMING_RESET_VALUE   0x00000024UL
#define CSL_WAIT_FIELD              0
#define   NBITS_CSL_WAIT              3
#define PIO_PHASE_FIELD             3
#define   NBITS_PIO_PHASE             3


/*
 *  IO Recover Register (read/write)
 *      bit = 0 : I/O pin is reclaimed by User
 *      bit = 1 : I/O pin is reserved by System
 */
#define CFG_IO_RECOVER_REG          (CFG_BASE + 0x08)
#define CFG_IO_RECOVER_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define IO_RECOVER_RESET_VALUE      0x00008061UL
#define SD_SEL_BIT                  0
#define WEN1_SEL_BIT                1
#define WEN2_SEL_BIT                2
#define WEN3_SEL_BIT                3
#define XDONE_SEL_BIT               4
#define A18_SEL_BIT                 5
#define A19_SEL_BIT                 6
#define A20_SEL_BIT                 7
#define A21_SEL_BIT                 8
#define A22_SEL_BIT                 9
#define A23_SEL_BIT                 10
#define A24_TO_A31_SEL_BIT          11
#define D8_TO_D15_SEL_BIT           12
#define D16_TO_D31_SEL_BIT          13
#define ECLK_SEL_BIT                14
#define SDCEN1_SEL_BIT              15
#define ENET0_MII_SEL_BIT           18
#define ENET0_MD_SEL_BIT            19
#define ENET0_X_SEL_BIT             20
#define CEN1_SEL_BIT                21
#define CAN_SEL_BIT                 22
#define SSI_SEL_BIT                 23
#define ENET1_MII_SEL_BIT           24
#define ENET1_MD_SEL_BIT            25
#define ENET1_X_SEL_BIT             26
#define USB_EXT_SEL_BIT             27
#define USB_PDN_SEL_BIT             28
#define USB_CLK_SEL_BIT             29
#define TWSI0_SEL_BIT               30
#define TWSI1_SEL_BIT               31

/*
 *  Configuration Protection Register (read/write)
 *      When a protection bit is set, the configuration plane
 *      is protected from read or write accesses. (Read data = 0).
 *      Once this bit is set, it can only be cleared through a power cycle.
 */
#define CFG_CONFIG_PROTECT_REG      (CFG_BASE + 0x0c)
#define CFG_CONFIG_PROTECT_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CONFIG_PROTECT_RESET_VALUE  0x00000000UL
#define RD_SECURE_BIT               0
#define WR_SECURE_BIT               1


/*
 **********************************************************
 * Interrupt Unit Definition
 **********************************************************
 */

/*
 *  Interrupt Registers Definition
 */
#define INT_IRQ_STATUS_REG          (INT_BASE + 0x00)
#define INT_IRQ_STATUS_REG_ACCESS   (READ_ACCESS)
#define IRQ_STATUS_RESET_VALUE      0x00000000UL

#define INT_IRQ_RAW_STATUS_REG      (INT_BASE + 0x04)
#define INT_IRQ_RAW_STATUS_REG_ACCESS (READ_ACCESS)
#define IRQ_RAW_STATUS_RESET_VALUE  0x00000000UL

#define INT_IRQ_ENABLE_REG          (INT_BASE + 0x08)
#define INT_IRQ_ENABLE_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define IRQ_ENABLE_RESET_VALUE      0x00000000UL

#define INT_IRQ_ENABLE_CLEAR_REG    (INT_BASE + 0x0c)
#define INT_IRQ_ENABLE_CLEAR_REG_ACCESS (WRITE_ACCESS)

/* 
 *  IRQ Software Reg provides a software mechanism to generate interrupts.
 */
#define INT_IRQ_SOFT_REG            (INT_BASE + 0x10)
#define INT_IRQ_SOFT_REG_ACCESS     (WRITE_ACCESS)
#define SOFT_IRQ_BIT                1

/* 
 *  IRQ Vector Enable Reg is used to enable individual hardware vectors.
 */
#define INT_IRQ_VECTOR_EN_REG       (INT_BASE + 0x20)
#define INT_IRQ_VECTOR_EN_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR_EN_RESET_VALUE   0x00000000UL
#define IRQ_VEC0_EN_BIT             0
#define IRQ_VEC1_EN_BIT             1
#define IRQ_VEC2_EN_BIT             2
#define IRQ_VEC3_EN_BIT             3
#define IRQ_VEC4_EN_BIT             4
#define IRQ_VEC5_EN_BIT             5
#define IRQ_VEC6_EN_BIT             6
#define IRQ_VEC7_EN_BIT             7

/* 
 *  The 5-bit value in the IRQ Vector Source Reg is used to
 *      assign an interrupt source to an interrupt vector.
 */
#define INT_IRQ_VECTOR0_SRC_REG     (INT_BASE + 0x24)
#define INT_IRQ_VECTOR0_SRC__REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR0_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR1_SRC_REG     (INT_BASE + 0x28)
#define INT_IRQ_VECTOR1_SRC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR1_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR2_SRC_REG     (INT_BASE + 0x2c)
#define INT_IRQ_VECTOR2_SRC_REG_CCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR2_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR3_SRC_REG     (INT_BASE + 0x30)
#define INT_IRQ_VECTOR3_SRC__REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR3_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR4_SRC_REG     (INT_BASE + 0x34)
#define INT_IRQ_VECTOR4_SRC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR4_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR5_SRC_REG     (INT_BASE + 0x38)
#define INT_IRQ_VECTOR5_SRC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR5_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR6_SRC_REG     (INT_BASE + 0x3c)
#define INT_IRQ_VECTOR6_SRC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR6_SRC_RESET_VALUE 0x00000000UL

#define INT_IRQ_VECTOR7_SRC_REG     (INT_BASE + 0x40)
#define INT_IRQ_VECTOR7_SRC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define IRQ_VECTOR7_SRC_RESET_VALUE 0x00000000UL

#define IRQ_VEC_SRC_FIELD           0
#define   NBITS_IRQ_VEC_SRC           5


/* 
 *  The 32-bit value in the IRQ Vector Reg substitutes the instruction
 *      that is fetched for the corresponding IRQ vector.
 */
#define INT_IRQ_VECTOR_DEF_REG      (INT_BASE + 0x60)
#define INT_IRQ_VECTOR_DEF_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR0_REG         (INT_BASE + 0x64)
#define INT_IRQ_VECTOR0_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR1_REG         (INT_BASE + 0x68)
#define INT_IRQ_VECTOR1_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR2_REG         (INT_BASE + 0x6c)
#define INT_IRQ_VECTOR2_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR3_REG         (INT_BASE + 0x70)
#define INT_IRQ_VECTOR3_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR4_REG         (INT_BASE + 0x74)
#define INT_IRQ_VECTOR4_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR5_REG         (INT_BASE + 0x78)
#define INT_IRQ_VECTOR5_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR6_REG         (INT_BASE + 0x7c)
#define INT_IRQ_VECTOR6_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_IRQ_VECTOR7_REG         (INT_BASE + 0x80)
#define INT_IRQ_VECTOR7_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define INT_FIQ_STATUS_REG          (INT_BASE + 0x100)
#define INT_FIQ_STATUS_REG_ACCESS   (READ_ACCESS)
#define FIQ_STATUS_RESET_VALUE      0x00000000UL

#define INT_FIQ_RAW_STATUS_REG      (INT_BASE + 0x104)
#define INT_FIQ_RAW_STATUS_REG_ACCESS (READ_ACCESS)
#define FIQ_RAW_STATUS_RESET_VALUE  0x00000000UL

#define INT_FIQ_ENABLE_REG          (INT_BASE + 0x108)
#define INT_FIQ_ENABLE_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define FIQ_ENABLE_RESET_VALUE      0x00000000UL
#define FIQ_EN_BIT                  0

#define INT_FIQ_ENABLE_CLEAR_REG    (INT_BASE + 0x10c)
#define INT_FIQ_ENABLE_CLEAR_REG_ACCESS (WRITE_ACCESS)
#define FIQ_EN_CLR_BIT              0

#define INT_IRQ_STEER_REG           (INT_BASE + 0x110)
#define INT_IRQ_STEER_REG_ACCESS    (WRITE_ACCESS)
#define IRQ_STEER_RESET_VALUE       0x00000000UL


/*
 *  Interrupt Bits Definition
 */
#define FIQ_BIT                     0
#define IRQ_SOFTWARE_BIT            1
#define IRQ_SERIAL_0_BIT            2
#define IRQ_TIMER_0_BIT             3
#define IRQ_TIMER_1_BIT             4
#define IRQ_SERIAL_1_BIT            5
#define IRQ_WATCHDOG_BIT            6
#define IRQ_DMA_0_BIT               7
#define IRQ_DMA_1_BIT               8
#define IRQ_DMA_2_BIT               9
#define IRQ_DMA_3_BIT               10
#define IRQ_CSL_USER_0_BIT          11
#define IRQ_CSL_USER_1_BIT          12
#define IRQ_CSL_USER_2_BIT          13
#define IRQ_JTAG_BIT                14
#define IRQ_BREAKPOINT_BIT          15
#define IRQ_MFTA_BIT                16
#define IRQ_TWSI0_BIT               17
#define IRQ_TWSI1_BIT               18
#define IRQ_DMA_4_BIT               19
#define IRQ_DMA_5_BIT               20
#define IRQ_USB_BIT                 21
#define IRQ_MAC0_TX_BIT             22
#define IRQ_MAC0_RX_BIT             23
#define IRQ_DMA_6_BIT               24
#define IRQ_DMA_7_BIT               25
#define IRQ_PLL_BIT                 26
#define IRQ_MAC1_TX_BIT             27
#define IRQ_MAC1_RX_BIT             28
#define IRQ_CAN_BIT                 29
#define IRQ_ADC_BIT                 30


/*
 ***********************************************************
 * DMA Unit Definition
 ***********************************************************
 */

/*
 *  DMA Control Registers Definition (read/write)
 */
#define DMA0_CONTROL_REG            (DMA_BASE + 0x00)
#define DMA1_CONTROL_REG            (DMA_BASE + 0x40)
#define DMA2_CONTROL_REG            (DMA_BASE + 0x80)
#define DMA3_CONTROL_REG            (DMA_BASE + 0xc0)
#define DMA_CONTROL_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define DMA_CONTROL_RESET_VALUE     0x00000000UL
#define CLEAR_BIT                   0
#define DMA_ENABLE_BIT              1
#define DMA_INIT_BIT                2
#define CONT_BIT                    3
#define SFT_REQ_BIT                 4
#define BLOCK_EN_BIT                5
#define TRANS_DIR_FIELD             6
#define   NBITS_TRANS_DIR             2
#define SRC_ADDR_MODE_FIELD         8
#define   NBITS_SRC_ADDR_MODE         2
#define DEST_ADDR_MODE_FIELD        10
#define   NBITS_DEST_ADDR_MODE        2
#define TRANS_SIZE_FIELD            12
#define   NBITS_TRANS_SIZE            2
#define METHOD_BIT                  14
#define AUX_DIS_BIT                 15
#define CRC_EN_BIT                  16
#define BROADCAST_BIT               17
#define BUFFER_FULL_FREEZE_BIT      19


/*
 *  DMA Interrupt Registers Definitions
 */
#define DMA0_INT_ENABLE_REG         (DMA_BASE + 0x04)
#define DMA1_INT_ENABLE_REG         (DMA_BASE + 0x44)
#define DMA2_INT_ENABLE_REG         (DMA_BASE + 0x84)
#define DMA3_INT_ENABLE_REG         (DMA_BASE + 0xc4)
#define DMA_INT_ENABLE_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define DMA_INT_ENABLE_RESET_VALUE  0x00000000UL

#define DMA0_INT_REG                (DMA_BASE + 0x08)
#define DMA1_INT_REG                (DMA_BASE + 0x48)
#define DMA2_INT_REG                (DMA_BASE + 0x88)
#define DMA3_INT_REG                (DMA_BASE + 0xc8)
#define DMA_INT_REG_ACCESS          (READ_ACCESS)
#define DMA_INT_RESET_VALUE         0x00000000UL

#define DMA0_INT_CLEAR_REG          (DMA_BASE + 0x0c)
#define DMA1_INT_CLEAR_REG          (DMA_BASE + 0x4c)
#define DMA2_INT_CLEAR_REG          (DMA_BASE + 0x8c)
#define DMA3_INT_CLEAR_REG          (DMA_BASE + 0xcc)
#define DMA_INT_CLEAR_REG_ACCESS    (WRITE_ACCESS)

#define TC_BIT                      0
#define INIT_BIT                    1
#define OVF_BIT                     2
#define FULL_BIT                    3
#define EMPTY_BIT                   4
#define LAST_BIT                    5
#define RETRANS_BIT                 6
#define DESC_BIT                    7
#define BAD_RETR_BIT                8


/*
 *  DMA Transfer Count Registers Definition
 */
#define DMA0_TRANS_CNT_REG          (DMA_BASE + 0x1c)
#define DMA1_TRANS_CNT_REG          (DMA_BASE + 0x5c)
#define DMA2_TRANS_CNT_REG          (DMA_BASE + 0x9c)
#define DMA3_TRANS_CNT_REG          (DMA_BASE + 0xdc)
#define DMA_TRANS_CNT_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define TRANS_CNT_FIELD             0
#define   NBITS_TRANS_CNT             16

#define DMA0_CUR_TRANS_CNT_REG      (DMA_BASE + 0x2c)
#define DMA1_CUR_TRANS_CNT_REG      (DMA_BASE + 0x6c)
#define DMA2_CUR_TRANS_CNT_REG      (DMA_BASE + 0xac)
#define DMA3_CUR_TRANS_CNT_REG      (DMA_BASE + 0xec)
#define DMA_CUR_TRANS_CNT_REG_ACCESS (READ_ACCESS)
#define CUR_TRANS_CNT_FIELD         TRANS_CNT_FIELD
#define   NBITS_CUR_TRANS_CNT         NBITS_TRANS_CNT


/*
 *  DMA Pending Request Counter Register Definition (read only)
 */
#define DMA0_PEND_REQ_REG           (DMA_BASE + 0x30)
#define DMA1_PEND_REQ_REG           (DMA_BASE + 0x70)
#define DMA2_PEND_REQ_REG           (DMA_BASE + 0xb0)
#define DMA3_PEND_REQ_REG           (DMA_BASE + 0xf0)
#define DMA_PEND_REQ_REG_ACCESS     (READ_ACCESS)
#define PEND_REQ_CTRL_FIELD         0
#define   NBITS_PEND_REQ_CTRL         10
#define LAST_POST_FIELD             16
#define   NBITS_LAST_POST             10


/*
 *  DMA Descriptor Table
 */
#define DMA0_DES_TABLE_ADDR_REG     (DMA_BASE + 0x10)
#define DMA1_DES_TABLE_ADDR_REG     (DMA_BASE + 0x50)
#define DMA2_DES_TABLE_ADDR_REG     (DMA_BASE + 0x90)
#define DMA3_DES_TABLE_ADDR_REG     (DMA_BASE + 0xd0)
#define DMA_DES_TABLE_ADDR_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define DMA0_CUR_DESC_ADDR_REG      (DMA_BASE + 0x20)
#define DMA1_CUR_DESC_ADDR_REG      (DMA_BASE + 0x60)
#define DMA2_CUR_DESC_ADDR_REG      (DMA_BASE + 0xa0)
#define DMA3_CUR_DESC_ADDR_REG      (DMA_BASE + 0xe0)
#define DMA_CUR_DESC_ADDR_REG_ACCESS (READ_ACCESS)

#if !defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)
typedef struct
{
    unsigned int sourceAddress;
    unsigned int reserved;
    unsigned int destinationAddress;
    unsigned int controlStatus;
}
DMA_DESCRIPTOR;
#endif

// DMA_DESCRIPTOR -> controlStatus Definition
#define ACT_WHEN_COMPL_FIELD        0
#define   NBITS_ACT_WHEN_COMPL        2
#define DESC_INT_BIT                2
#define CRC_CLR_DIS_BIT             3
#define CONT_DESC_BIT               4
#define BUFF_STAT_BIT               5
#define DIS_BRCST_BIT               6
#define TRANS_LENGTH_FIELD          16
#define   NBITS_TRANS_LENGTH          16


/*
 *  Other DMA Registers Definition - 32-bit
 */
#define DMA0_SRC_ADDR_REG           (DMA_BASE + 0x14)
#define DMA1_SRC_ADDR_REG           (DMA_BASE + 0x54)
#define DMA2_SRC_ADDR_REG           (DMA_BASE + 0x94)
#define DMA3_SRC_ADDR_REG           (DMA_BASE + 0xd4)
#define DMA_SRC_ADDR_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)

#define DMA0_DST_ADDR_REG           (DMA_BASE + 0x18)
#define DMA1_DST_ADDR_REG           (DMA_BASE + 0x58)
#define DMA2_DST_ADDR_REG           (DMA_BASE + 0x98)
#define DMA3_DST_ADDR_REG           (DMA_BASE + 0xd8)
#define DMA_DST_ADDR_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)

#define DMA0_CUR_SRC_ADDR_REG       (DMA_BASE + 0x24)
#define DMA1_CUR_SRC_ADDR_REG       (DMA_BASE + 0x64)
#define DMA2_CUR_SRC_ADDR_REG       (DMA_BASE + 0xa4)
#define DMA3_CUR_SRC_ADDR_REG       (DMA_BASE + 0xe4)
#define DMA_CUR_SRC_ADDR_REG_ACCESS (READ_ACCESS)

#define DMA0_CUR_DEST_ADDR_REG      (DMA_BASE + 0x28)
#define DMA1_CUR_DEST_ADDR_REG      (DMA_BASE + 0x68)
#define DMA2_CUR_DEST_ADDR_REG      (DMA_BASE + 0xa8)
#define DMA3_CUR_DEST_ADDR_REG      (DMA_BASE + 0xe8)
#define DMA_CUR_DEST_ADDR_REG_ACCESS (READ_ACCESS)

#define DMA_CRC_REG                 (DMA_BASE + 0xfc)
#define DMA_CRC_REG_ACCESS          (READ_ACCESS)


/*
 ***********************************************************
 * DMAX Unit Definition
 ***********************************************************
 */

/*
 *  DMAX Control Registers Definition (read/write)
 */
#define DMAX0_CONTROL_REG           (DMAX_BASE + 0x00)
#define DMAX1_CONTROL_REG           (DMAX_BASE + 0x40)
#define DMAX2_CONTROL_REG           (DMAX_BASE + 0x80)
#define DMAX3_CONTROL_REG           (DMAX_BASE + 0xc0)
#define DMAX_CONTROL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define DMAX_CONTROL_RESET_VALUE    0x00000000UL
// for more bit definitions see: DMA Unit -> DMA Control Registers Definition
#define DMAX_PACKET_EN_BIT          18
#define BUFFER_FULL_FREEZE_BIT      19


/*
 *  DMAX Interrupt Registers Definitions
 *      The bitfields for these regs are defined in the DMA Unit.
 */
#define DMAX0_INT_ENABLE_REG        (DMAX_BASE + 0x04)
#define DMAX1_INT_ENABLE_REG        (DMAX_BASE + 0x44)
#define DMAX2_INT_ENABLE_REG        (DMAX_BASE + 0x84)
#define DMAX3_INT_ENABLE_REG        (DMAX_BASE + 0xc4)
#define DMAX_INT_ENABLE_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define DMAX_INT_ENABLE_RESET_VALUE 0x00000000UL

#define DMAX0_INT_STATUS_REG        (DMAX_BASE + 0x08)
#define DMAX1_INT_STATUS_REG        (DMAX_BASE + 0x48)
#define DMAX2_INT_STATUS_REG        (DMAX_BASE + 0x88)
#define DMAX3_INT_STATUS_REG        (DMAX_BASE + 0xc8)
#define DMAX_INT_STATUS_REG_ACCESS  (READ_ACCESS)
#define DMAX_INT_STATUS_RESET_VALUE 0x00000000UL

#define DMAX0_INT_CLEAR_REG         (DMAX_BASE + 0x0c)
#define DMAX1_INT_CLEAR_REG         (DMAX_BASE + 0x4c)
#define DMAX2_INT_CLEAR_REG         (DMAX_BASE + 0x8c)
#define DMAX3_INT_CLEAR_REG         (DMAX_BASE + 0xcc)
#define DMAX_INT_CLEAR_REG_ACCESS   (WRITE_ACCESS)


/*
 *  DMAX Descriptor Table Address Registers Definition
 */
#define DMAX0_DES_TABLE_ADDR_REG    (DMAX_BASE + 0x10)
#define DMAX1_DES_TABLE_ADDR_REG    (DMAX_BASE + 0x50)
#define DMAX2_DES_TABLE_ADDR_REG    (DMAX_BASE + 0x90)
#define DMAX3_DES_TABLE_ADDR_REG    (DMAX_BASE + 0xd0)
#define DMAX_DES_TABLE_ADDR_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define DMAX0_CUR_DESC_ADDR_REG     (DMAX_BASE + 0x20)
#define DMAX1_CUR_DESC_ADDR_REG     (DMAX_BASE + 0x60)
#define DMAX2_CUR_DESC_ADDR_REG     (DMAX_BASE + 0xa0)
#define DMAX3_CUR_DESC_ADDR_REG     (DMAX_BASE + 0xe0)
#define DMAX_CUR_DESC_ADDR_REG_ACCESS (READ_ACCESS)

/*
 *  Descriptor typedefs
 *      For A7-Mode, see typedef for DMA_DESCRIPTOR in the DMA Unit.
 *      The controlStatus bitfields are also defined in the DMA Unit.
 */
#if !defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)
typedef struct
{
    unsigned int packetControlStatus;
    unsigned int reserved;
    unsigned int startAddress;
    unsigned int controlStatus;
}
DMAX_PACKET_DESCRIPTOR;
#endif

/*
 *  DMAX Source/Destination/Count Registers Definition - 32-bit
 */
#define DMAX0_TRANS_SRC_ADDR_REG    (DMAX_BASE + 0x14)
#define DMAX1_TRANS_SRC_ADDR_REG    (DMAX_BASE + 0x54)
#define DMAX2_TRANS_SRC_ADDR_REG    (DMAX_BASE + 0x94)
#define DMAX3_TRANS_SRC_ADDR_REG    (DMAX_BASE + 0xd4)
#define DMAX_TRANS_SRC_ADDR_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define DMAX0_TRANS_DEST_ADDR_REG   (DMAX_BASE + 0x18)
#define DMAX1_TRANS_DEST_ADDR_REG   (DMAX_BASE + 0x58)
#define DMAX2_TRANS_DEST_ADDR_REG   (DMAX_BASE + 0x98)
#define DMAX3_TRANS_DEST_ADDR_REG   (DMAX_BASE + 0xd8)
#define DMAX_TRANS_DEST_ADDR_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define DMAX0_TRANS_COUNT_REG       (DMAX_BASE + 0x1c)
#define DMAX1_TRANS_COUNT_REG       (DMAX_BASE + 0x5c)
#define DMAX2_TRANS_COUNT_REG       (DMAX_BASE + 0x9c)
#define DMAX3_TRANS_COUNT_REG       (DMAX_BASE + 0xdc)
#define DMAX_TRANS_COUNT_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)

#define DMAX0_CUR_SRC_ADDR_REG      (DMAX_BASE + 0x24)
#define DMAX1_CUR_SRC_ADDR_REG      (DMAX_BASE + 0x64)
#define DMAX2_CUR_SRC_ADDR_REG      (DMAX_BASE + 0xa4)
#define DMAX3_CUR_SRC_ADDR_REG      (DMAX_BASE + 0xe4)
#define DMAX_CUR_SRC_ADDR_REG_ACCESS (READ_ACCESS)

#define DMAX0_CUR_DEST_ADDR_REG     (DMAX_BASE + 0x28)
#define DMAX1_CUR_DEST_ADDR_REG     (DMAX_BASE + 0x68)
#define DMAX2_CUR_DEST_ADDR_REG     (DMAX_BASE + 0xa8)
#define DMAX3_CUR_DEST_ADDR_REG     (DMAX_BASE + 0xe8)
#define DMAX_CUR_DEST_ADDR_REG_ACCESS (READ_ACCESS)

#define DMAX0_CUR_TRANS_COUNT_REG   (DMAX_BASE + 0x2c)
#define DMAX1_CUR_TRANS_COUNT_REG   (DMAX_BASE + 0x6c)
#define DMAX2_CUR_TRANS_COUNT_REG   (DMAX_BASE + 0xac)
#define DMAX3_CUR_TRANS_COUNT_REG   (DMAX_BASE + 0xec)
#define DMAX_CUR_TRANS_COUNT_REG_ACCESS (READ_ACCESS)

/*
 *  DMAX Pending Request Counter Register Definition (read only)
 *      The bitfields for these regs are defined in the DMA Unit.
 */
#define DMAX0_PENDING_REQ_REG       (DMAX_BASE + 0x30)
#define DMAX1_PENDING_REQ_REG       (DMAX_BASE + 0x70)
#define DMAX2_PENDING_REQ_REG       (DMAX_BASE + 0xb0)
#define DMAX3_PENDING_REQ_REG       (DMAX_BASE + 0xf0)
#define DMAX_PENDING_REQ_REG_ACCESS (READ_ACCESS)


/*
 *  DMAX Packet Mode Control Status Register Definition - 32-bit (read only)
 *      If DMA is configured for D2M (Ethernet Rx), this field is the
 *          "Receive_Descriptor_Status", and is written from the Ethernet
 *          device to the DMA after the last piece of data is transferred
 *          for that particular Descriptor entry.
 *      If the DMA is configured for M2D (Ethernet Tx), this field is the
 *          "Transmit_Descriptor_Control", and is written from the DMA to
 *          the Ethernet device before the first piece of data is transferred
 *          for that particular Descriptor entry.
 */
#define DMAX0_PACK_CONT_STAT_REG    (DMAX_BASE + 0x34)
#define DMAX1_PACK_CONT_STAT_REG    (DMAX_BASE + 0x74)
#define DMAX2_PACK_CONT_STAT_REG    (DMAX_BASE + 0xb4)
#define DMAX3_PACK_CONT_STAT_REG    (DMAX_BASE + 0xf4)
#define DMAX_PACK_CONT_STAT_REG_ACCESS (READ_ACCESS)

#define DMAX_PACK_TX_LAST_DESC      0
#define DMAX_PACK_TX_OPTIONS_FIELD  4
#define   NBITS_DMAX_PACK_TX_OPTIONS  3
#define DMAX_PACK_TX_BVALID_FIELD   30
#define   NBITS_DMAX_PACK_TX_BVALID   2

#define DMAX_PACK_RX_LAST_DESC_BIT  0
#define DMAX_PACK_RX_LEN_ERR_BIT    4
#define DMAX_PACK_RX_CTL_RECD_BIT   5
#define DMAX_PACK_RX_INT_RX_BIT     6
#define DMAX_PACK_RX_ALIGN_ERR_BIT  8
#define DMAX_PACK_RX_CRC_ERR_BIT    9
#define DMAX_PACK_RX_OVERFLOW_BIT   10
#define DMAX_PACK_RX_LONG_ERR_BIT   11
#define DMAX_PACK_RX_PAR_BIT        13
#define DMAX_PACK_RX_GOOD_BIT       14
#define DMAX_PACK_RX_HALTED_BIT     15
#define DMAX_PACK_RX_MCAST_BIT      17
#define DMAX_PACK_RX_BCAST_BIT      18
#define DMAX_PACK_RX_VLAN_BIT       19
#define DMAX_PACK_RX_PAUSE_BIT      20
#define DMAX_PACK_RX_ARC_STATUS_FIELD 21
#define   NBITS_DMAX_PACK_RX_ARC_STATUS 4
#define DMAX_PACK_RX_ARC_ENT_FIELD  25
#define   NBITS_DMAX_PACK_RX_ARC_ENT  2
#define DMAX_PACK_RX_LAST_BVALID_FIELD 30
#define   NBITS_DMAX_PACK_RX_LAST_BVALID 2

/*
 * DMAX Descriptor IRQ Count Register Definition
 */
#define DMAX0_IRQ_COUNT_REG         (DMAX_BASE + 0x38)
#define DMAX1_IRQ_COUNT_REG         (DMAX_BASE + 0x78)
#define DMAX2_IRQ_COUNT_REG         (DMAX_BASE + 0xb8)
#define DMAX3_IRQ_COUNT_REG         (DMAX_BASE + 0xf8)
#define DMAX_PACK_CONT_STAT_REG_ACCESS (READ_ACCESS)

/*
 *  DMAX CRC Register Definition - 32-bit (read only)
 */
#define DMAX_CRC_REG                (DMAX_BASE + 0xfc)
#define DMAX_CRC_REG_ACCESS         (READ_ACCESS)


/*
 **********************************************************
 * UART Definition
 **********************************************************
 */

/*
 *  Uart Control Register Structure (read/write)
 */
#define UART0_CONTROL_REG           (UART_BASE + 0x00)
#define UART1_CONTROL_REG           (UART_BASE + 0x40)
#define UART_CONTROL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define UART_CONTROL_RESET_VALUE    0x00000000UL
#define UART_PRESCALE_FIELD         0
#define   NBITS_UART_PRESCALE         8
#define PRESCALE_EN_BIT             8
#define MODEM_EN_BIT                9
#define TX_DMA_EN_BIT               16
#define TX_DMA_SEL_FIELD            17
#define   NBITS_TX_DMA_SEL            2
//reserved bit 19
#define RX_DMA_EN_BIT               20
#define RX_DMA_SEL_FIELD            21
#define   NBITS_RX_DMA_SEL            2


/*
 *  DLAB=0: Uart Rx Tx Register Structure       (read/write)
 *  DLAB=1: Uart Divisor LSB Register Structure (read/write)
 */
#define UART0_RX_TX_REG             (UART_BASE + 0x20)
#define UART1_RX_TX_REG             (UART_BASE + 0x60)
#define UART_RX_REG_ACCESS          (READ_ACCESS)
#define UART_TX_REG_ACCESS          (WRITE_ACCESS)
#define DATA_FIELD                  0
#define   NBITS_DATA                  8

#define UART0_DIVISOR_LSB_REG       (UART_BASE + 0x20)
#define UART1_DIVISOR_LSB_REG       (UART_BASE + 0x60)
#define UART_DIVISOR_LSB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define DIVISOR_LSB_FIELD           0
#define   NBITS_DIVISOR_LSB           8


/*
 *  DLAB=0: Uart Interrupt Enable Register Structure (read/write)
 *  DLAB=1: Uart Divisor MSB Register Structure      (read/write)
 */
#define UART0_INT_ENABLE_REG        (UART_BASE + 0x24)
#define UART1_INT_ENABLE_REG        (UART_BASE + 0x64)
#define UART_INT_ENABLE_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define UART_INT_ENABLE_RESET_VALUE 0x00000000UL
#define RDRE_BIT                    0
#define THREE_BIT                   1
#define RLSE_BIT                    2
#define MSE_BIT                     3

#define UART0_DIVISOR_MSB_REG       (UART_BASE + 0x24)
#define UART1_DIVISOR_MSB_REG       (UART_BASE + 0x64)
#define UART_DIVISOR_MSB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define DIVISOR_MSB_FIELD           0
#define   NBITS_DIVISOR_MSB           8


/*
 *  Uart Interrupt ID Register Structure (read only)
 *  Uart Fifo Control Register Structure (write only)
 */
#define UART0_INT_ID_REG            (UART_BASE + 0x28)
#define UART1_INT_ID_REG            (UART_BASE + 0x68)
#define UART_INT_ID_REG_ACCESS      (READ_ACCESS)
#define UART_INT_ID_RESET_VALUE     0x00000001UL
#define INT_ID_FIELD                0
#define   NBITS_INT_ID                4
#define FIFO_MODE_FIELD             6
#define   NBITS_FIFO_MODE             2

#define UART0_FIFO_CTRL_REG         (UART_BASE + 0x28)
#define UART1_FIFO_CTRL_REG         (UART_BASE + 0x68)
#define UART_FIFO_CTRL_REG_ACCESS   (WRITE_READ_ACCESS)
#define UART_FIFO_CTRL_RESET_VALUE  0x00000000UL
#define FIFO_MODE_ENABLE_BIT        0
#define RX_FIFO_CLR_BIT             1
#define TX_FIFO_CLR_BIT             2
#define FIFO_TRIG_LEVEL_FIELD       6
#define   NBITS_FIFO_TRIG_LEVEL       2


/*
 *  Uart Line Control Register Structure (read/write)
 */
#define UART0_LINE_CONTROL_REG      (UART_BASE + 0x2c)
#define UART1_LINE_CONTROL_REG      (UART_BASE + 0x6c)
#define UART_LINE_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define UART_LINE_CONTROL_RESET_VALUE 0x00000000UL
#define STB_WLS_FIELD               0
#define   NBITS_STB_WLS               3
#define PEN_BIT                     3
#define EPS_BIT                     4
#define STICK_PARITY_BIT            5
#define BREAK_BIT                   6
#define DLAB_BIT                    7


/*
 *  Uart Modem Control Register Structure (read/write)
 */
#define UART0_MODEM_CONTROL_REG     (UART_BASE + 0x30)
#define UART1_MODEM_CONTROL_REG     (UART_BASE + 0x70)
#define UART_MODEM_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define UART_MODEM_CONTROL_RESET_VALUE 0x00000000UL
#define DTR_BIT                     0
#define RTS_BIT                     1
#define LOOP_BIT                    3


/*
 *  Uart Line Status Register Structure (read only)
 */
#define UART0_LINE_STATUS_REG       (UART_BASE + 0x34)
#define UART1_LINE_STATUS_REG       (UART_BASE + 0x74)
#define UART_LINE_STATUS_REG_ACCESS (READ_ACCESS)
#define UART_LINE_STATUS_RESET_VALUE 0x00000060UL
#define DR_BIT                      0
#define OE_BIT                      1
#define PE_BIT                      2
#define FE_BIT                      3
#define BI_BIT                      4
#define THRE_BIT                    5
#define TEMT_BIT                    6
#define ERROR_BIT                   7


/*
 *  Uart Modem Status Register Structure (read only)
 */
#define UART0_MODEM_STATUS_REG      (UART_BASE + 0x38)
#define UART1_MODEM_STATUS_REG      (UART_BASE + 0x78)
#define UART_MODEM_STATUS_REG_ACCESS (READ_ACCESS)
#define UART_MODEM_STATUS_RESET_VALUE 0x00000000UL
#define DELTA_CTS_BIT               0
#define DELTA_DSR_BIT               1
#define TERI_BIT                    2
#define DELTA_DCD_BIT               3
#define CTS_BIT                     4
#define DSR_BIT                     5
#define RI_BIT                      6
#define DCD_BIT                     7


/*
 *  Uart Scratchpad Register Structure (read/write)
 */
#define UART0_SCRATCHPAD_REG        (UART_BASE + 0x3c)
#define UART1_SCRATCHPAD_REG        (UART_BASE + 0x7c)
#define UART_SCRATCHPAD_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)



/*
 **********************************************************
 * Timer Unit Definition
 **********************************************************
 */
/*
 *  Timer Load Register Structure (read/write)
 */
#define TIMER0_LOAD_REG             (TIMER_BASE + 0x00)
#define TIMER1_LOAD_REG             (TIMER_BASE + 0x20)
#define TIMER_LOAD_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define LOAD_FIELD                  0
#define   NBITS_LOAD                  16


/*
 *  Timer Value Register Structure (read only)
 */
#define TIMER0_VALUE_REG            (TIMER_BASE + 0x04)
#define TIMER1_VALUE_REG            (TIMER_BASE + 0x24)
#define TIMER_VALUE_REG_ACCESS      (READ_ACCESS)
#define VALUE_FIELD                 0
#define   NBITS_VALUE                 16


/*
 *  Timer Control Register Structure (read/write)
 */
#define TIMER0_CONTROL_REG          (TIMER_BASE + 0x08)
#define TIMER1_CONTROL_REG          (TIMER_BASE + 0x28)
#define TIMER_CONTROL_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define TIMER_CONTROL_RESET_VALUE   0x00000000UL
#define PRESCALE_FIELD              2
#define   NBITS_PRESCALE              2
#define TIM_MODE_BIT                6
#define TIM_ENABLE_BIT              7


/*
 *  Timer Clear Register Structure (write only)
 *      Write a "1" to clear the timer interrupt.
 *      Writing a "0" has no effect.
 */
#define TIMER0_CLEAR_REG            (TIMER_BASE + 0x0c)
#define TIMER1_CLEAR_REG            (TIMER_BASE + 0x2c)
#define TIMER_CLEAR_REG_ACCESS      (WRITE_ACCESS)
#define TIM_INT_CLEAR_BIT           0



/*
 **********************************************************
 * Watchdog Unit Definition
 **********************************************************
 */

/*
 *  Watchdog Control Register Structure (read/write)
 *      Writing a '1' to the WD_RESET bit will reset the watchdog timer,
 *      including its interrupt and reset logic. This bit is self-clearing.
 */
#define WATCHDOG_CONTROL_REG        (WD_BASE + 0x00)
#define WATCHDOG_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define WATCHDOG_CONTROL_RESET_VALUE 0x00000000UL
#define WD_ENABLE_BIT               0
#define WD_RESET_BIT                1
#define EN_WD_RST_BIT               2


/*
 *  Watchdog Timeout Value Register Structure (read/write)
 */
#define WATCHDOG_TIMEOUT_VAL_REG    (WD_BASE + 0x04)
#define WATCHDOG_TIMEOUT_VAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)


/*
 *  Watchdog Current Value Register Structure (read only)
 */
#define WATCHDOG_CURRENT_VAL_REG    (WD_BASE + 0x08)
#define WATCHDOG_CURRENT_VAL_REG_ACCESS (READ_ACCESS)


/*
 *  Watchdog Clear Register Structure (write only)
 *      Writing a '1' to the WD_INT_CLR bit will clear the watchdog interrupt.
 *      Writing a "0" has no effect.
 */
#define WATCHDOG_CLEAR_REG          (WD_BASE + 0x0c)
#define WATCHDOG_CLEAR_REG_ACCESS   (WRITE_ACCESS)
#define WD_INT_CLR_BIT              0



/*
 ******************************************************************************
 * Breakpoint Unit Definition
 ******************************************************************************
 */

/*
 * Breakpoint Control Register definition (read/write)
 */
#define BPU_BRK_CONTROL_REG         (BPU_BASE + 0x00)
#define BPU_BRK_CONTROL_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define BRK_CONTROL_RESET_VALUE     0x00000000UL
#define CPU_STIM_FIELD          0
#define   NBITS_CPU_STIM          2
//#define CLR_DBG_REQ_BIT         2  // hardware not tested
#define CSL_FREEZE_BIT          3
#define CSL_MATCH_BIT           4
#define BP_TYPE_FIELD           5
#define   NBITS_BP_TYPE           3
#define RESTART_BIT             8
#define BP_ENABLE_FIELD         9
#define   NBITS_BP_ENABLE         2
//#define DBG_BRK_EN_BIT         11  // hardware not tested
//#define DBG_REQ_EN_BIT         12  // hardware not tested
#define BP0_NOT_MATCH_BIT      13
#define BP1_NOT_MATCH_BIT      14
#define ARB_FREEZE_BIT         15
#define BUS_SEL_BIT            16
#define BP0_DATA_SEL_BIT       17
#define BP1_DATA_SEL_BIT       18
#define TRACE_EN_BIT           19
#define TRACE_FORMAT_FIELD     20
#define   NBITS_TRACE_FORMAT      4
#define CSL_CAPT_EN_BIT        24
#define BCLK_FREEZE_BIT        25
#define CPU_DBG_EN_BIT         26
#define CAPT_PULSE_BIT         27
//#define DBG_EXT0_EN_BIT        29  // hardware not tested
//#define DBG_EXT1_EN_BIT        30  // hardware not tested

// CSI bus Trace Format enumeration
#define CSI_TRACE_ALL        0x0
#define CSI_TRACE_VALID      0x1
#define CSI_TRACE_DMA0       0x2
#define CSI_TRACE_DMA1       0x3
#define CSI_TRACE_DMA2       0x4
#define CSI_TRACE_DMA3       0x5
#define CSI_TRACE_ARM        0x6
#define CSI_TRACE_DMAX0      0x7
#define CSI_TRACE_DMAX1      0x8
#define CSI_TRACE_DMAX2      0x9
#define CSI_TRACE_ENET_TXRX  0xa

// CPU bus Trace Format enumeration
#define CPU_TRACE_ALL          0x0
#define CPU_TRACE_VALID        0x1
#define CPU_TRACE_INSTRUCTION  0x2
#define CPU_TRACE_DATA_ACCESS  0x3


/*
 * Breakpoint Bus Mask/Compare Registers Definition (read/write)
 */
#define BPU_BRK_MASK_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define BPU_BRK_COMP_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

/*
 * Breakpoint Bus Mask0/Compare0 Registers Definition (read/write)
 */
#define BPU_BRK0_BUS_MASK0_REG  (BPU_BASE + 0x04)
#define BPU_BRK0_BUS_COMP0_REG  (BPU_BASE + 0x24)
#define BPU_BRK1_BUS_MASK0_REG  (BPU_BASE + 0x14)
#define BPU_BRK1_BUS_COMP0_REG  (BPU_BASE + 0x34)

/*
 * Breakpoint Bus Mask1/Compare1 Registers Definition (read/write)
 */
#define BPU_BRK0_BUS_MASK1_REG  (BPU_BASE + 0x08)
#define BPU_BRK0_BUS_COMP1_REG  (BPU_BASE + 0x28)
#define BPU_BRK1_BUS_MASK1_REG  (BPU_BASE + 0x18)
#define BPU_BRK1_BUS_COMP1_REG  (BPU_BASE + 0x38)

/*
 * Breakpoint Bus Mask2/Compare2 Registers Definition (read/write)
 */
#define BPU_BRK0_BUS_MASK2_REG  (BPU_BASE + 0x0c)
#define BPU_BRK0_BUS_COMP2_REG  (BPU_BASE + 0x2c)
#define BPU_BRK1_BUS_MASK2_REG  (BPU_BASE + 0x1c)
#define BPU_BRK1_BUS_COMP2_REG  (BPU_BASE + 0x3c)

// BUS MASK2 for CSI BUS
#define SW_DMA_ACK_FIELD       0
#define   NBITS_SW_DMA_ACK       4
#define SW_MODE_FIELD          4
#define   NBITS_SW_MODE          6
#define SW_DMA_CTRL_FIELD     10
#define   NBITS_SW_DMA_CTRL      4
#define SW_SIZE_FIELD         14
#define   NBITS_SW_SIZE          2
#define SW_RD_EN_BIT          16
#define SW_WR_EN_BIT          17
#define SW_BP_CTRL_BIT        18
#define RD_OR_WR_BIT          19
#define SW_DMAX_ACK_FIELD     20
#define   NBITS_SW_DMAX_ACK      3
#define SW_DMAX_CTRL_FIELD    23
#define   NBITS_SW_DMAX_CTRL     3
#define OR_DMA_ACK_BIT        26
#define RD_WR_ACK_BIT         27

// BUS MASK 2 for CPU BUS
#define WRITE_BIT              0
#define SIZE_FIELD             1
#define   NBITS_SIZE             2
#define LOCK_BIT               3
#define TRANS_FIELD            4
#define   NBITS_TRANS            2
#define ABORT_BIT              6
#define PROT_FIELD             7
#define   NBITS_PROT             2
#define DBGACK_BIT            17

/*
 * Breakpoint Bus Mask3 Registers Definition (read/write)
 */
#define BPU_BRK0_BUS_MASK3_REG  (BPU_BASE + 0x10)
#define BPU_BRK1_BUS_MASK3_REG  (BPU_BASE + 0x20)
#define FR_FIELD    0
#define   NBITS_FR   18


/*
 * Breakpoint Counter Compare/Out Registers Definition
 */
#define BPU_BRK_CNT_COMP_REG         (BPU_BASE + 0x44)
#define BPU_BRK_CNT_COMP_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)

#define BPU_BRK_CNT_OUT_REG          (BPU_BASE + 0x48)
#define BPU_BRK_CNT_OUT_REG_ACCESS   (READ_ACCESS)

#define BRK_CNT0_FIELD     0
#define   NBITS_BRK_CNT0    16
#define BRK_CNT1_FIELD    16
#define   NBITS_BRK_CNT1    16


/*
 * Trace Buffer Address Pointer Definition
 * Trace Counter Definition
 */
#define BPU_BRK_TR_ADDR_REG              (BPU_BASE + 0x4c)
#define BPU_BRK_TRACE_ADDR_FIELD_ACCESS  (READ_ACCESS)
#define BPU_BRK_TR_ADDR_FLIP_BIT_ACCESS  (READ_ACCESS)
#define BPU_BRK_TRACE_CNT_FIELD_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define TRACE_ADDR_FIELD      0 // read only
#define   NBITS_TRACE_ADDR      9
#define TR_ADDR_FLIP_BIT      9 // read only
#define TRACE_CNT_FIELD      16 // read/write
#define   NBITS_TRACE_CNT       9


/*
 * Breakpoint Interrupt Clear Register Definition (write only)
 */
#define BPU_BRK_INT_CLEAR_REG         (BPU_BASE + 0x50)
#define BPU_BRK_INT_CLEAR_REG_ACCESS  (WRITE_ACCESS)
#define BRK_INT_CLEAR_BIT   0


/*
 * Breakpoint Status Register Definition (read only)
 */
#define BPU_BRK_STATUS_REG         (BPU_BASE + 0x54)
#define BPU_BRK_STATUS_REG_ACCESS  (READ_ACCESS)
#define STAT_CPU_FREEZE_BIT   0
#define STAT_CPU_IRQ_BIT      1
//#define STAT_DBG_REQ_BIT      2  // hardware not tested
//#define STAT_DBG_BREAK_BIT    3  // hardware not tested
//#define STAT_DBG_EXT0_BIT     4  // hardware not tested
//#define STAT_DBG_EXT1_BIT     5  // hardware not tested
#define STAT_ARB_FREEZE_BIT   6
#define STAT_CSL_MATCH_BIT    7
#define STAT_CSL_FREEZE_BIT   8
#define STAT_CSL_STEP_BIT     9
#define STAT_TRACE_FIN_BIT   10
#define STAT_TRACE_EN_BIT    11
#define STAT_BP0_CNT_TC_BIT  12
#define STAT_BP1_CNT_TC_BIT  13
#define STAT_BP_EVENT_BIT    14
#define STAT_DBG_ACK_BIT     15


/*
 * Breakpoint Start Count Register Definition (read only)
 *     Retains initial value written to Trace Counter before Trace was enabled
 */
#define BPU_BRK_START_CNT_REG         (BPU_BASE + 0x58)
#define BPU_BRK_START_CNT_REG_ACCESS  (READ_ACCESS)
#define TRACE_START_CNT_FIELD    0
#define   NBITS_TRACE_START_CNT    9


/*
 * Trace Buffer Definition
 *     The Trace Buffer is implemented in the Scratchpad RAM.
 *     The buffer is a data array of 128-bit wide and 512 deep.
 *     The trace can be moved relative to a programmed breakpoint event.
 */
/*
 * CSI Trace Capture Description
 */
#if !defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)
typedef struct
{
    unsigned int CsiSlaveWriteBusAddr;
    unsigned int CsiSlaveWriteBusData;
    unsigned int CsiControl;    // see definition below
    unsigned int CsiMasterReadData;
}
CSI_TRACE;
#endif

// CsiControl trace definition
#define CSI_SW_DMAACK_FIELD        0
#define   NBITS_CSI_SW_DMAACK        4
#define CSI_SW_MODE_FIELD          4
#define   NBITS_CSI_SW_MODE          6
#define CSI_SW_DMA_CTRL_FIELD     10
#define   NBITS_CSI_SW_DMA_CTRL      4
#define CSI_SW_SIZE_FIELD         14
#define   NBITS_CSI_SW_SIZE          2
#define CSI_SW_RD_EN_BIT          16
#define CSI_SW_WR_EN_BIT          17
#define CSI_SR_BP_CTRL_BIT        18
#define CSI_RD_OR_WR_BIT          19
#define CSI_SW_DMAX_ACK_FIELD     20
#define   NBITS_CSI_SW_DMAX_ACK      3
#define CSI_SW_DMAX_CTRL_FIELD    23
#define   NBITS_CSI_SW_DMAX_CTRL     3
#define CSI_FR_FIELD              26    // see enumeration below
#define   NBITS_CSI_FR               5
#define CSI_SW_WAIT_NOW_BIT       31

// CSI_FR_FIELD enumeration
#define FR_DMA0_IO      1
#define FR_DMA0_MEM     2
#define FR_DMA1_IO      3
#define FR_DMA1_MEM     4
#define FR_DMA2_IO      5
#define FR_DMA2_MEM     6
#define FR_DMA3_IO      7
#define FR_DMA3_MEM     8
#define FR_JTAG         9
#define FR_MIU         10
#define FR_CPU         11
#define FR_DMAX4_IO    13
#define FR_DMAX4_MEM   14
#define FR_DMAX5_IO    15
#define FR_DMAX5_MEM   16
#define FR_DMAX6_IO    17
#define FR_DMAX6_MEM   18


/*
 * CPU Trace Capture Description
 */
#if !defined(_ASMLANGUAGE) && !defined(__ASSEMBLER__)
typedef struct
{
    unsigned int CpuAddress;
    unsigned int CpuData;
    unsigned int CpuControl;    // see definition below
    unsigned int Reserved;
}
CPU_TRACE;
#endif

// CpuControl trace definition
#define ARM_WRITE_BIT         0
#define ARM_SIZE_FIELD        1
#define   NBITS_ARM_SIZE        2
#define ARM_LOCK_BIT          3
#define ARM_LOCK_FIELD        4
#define   NBITS_LOCK            2
#define ARM_ABORT_BIT         6
#define ARM_PROT_FIELD        7
#define   NBITS_ARM_PROT        2
#define ARM_DBG_ACK_BIT      17
#define ARM_DBG_BREAK_BIT    19
#define ARM_DBG_EXT0_BIT     20
#define ARM_DBG_EXT1_BIT     21
#define ARM_DBG_REQ_BIT      22
#define ARM_DBG_COMM_RX_BIT  23
#define ARM_DBG_COMM_TX_BIT  24
#define ARM_CACHELINE_BIT    25
#define ARM_THUMB_BIT        26
#define ARM_DBG_NEXEC_BIT    27
#define ARM_DBG_EN_BIT       28
#define ARM_DBG_RNG0_BIT     29
#define ARM_DBG_RNG1_BIT     30
#define ARM_CLK_EN_BIT       31



/*
 ******************************************************************************
 * Protection Unit Registers (PU_BASE)
 ******************************************************************************
 */

/*
 * Protection Unit Control Register definition (read/write)
 */
#define PU_CONTROL_REG          (PU_BASE + 0x04)
#define PU_CONTROL_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define PU_CONTROL_RESET_VALUE  0x00000000UL
#define PROT_EN_BIT    0
#define CACHE_EN_BIT   2


/*
 * Cacheable Area Register definition (read/write)
 */
#define PU_CACHEABLE_AREA_REG         (PU_BASE + 0x08)
#define PU_CACHEABLE_AREA_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define C_0_BIT  0
#define C_1_BIT  1
#define C_2_BIT  2
#define C_3_BIT  3
#define C_4_BIT  4
#define C_5_BIT  5
#define C_6_BIT  6
#define C_7_BIT  7


/*
 * Protection Area Register definition (read/write)
 */
#define PU_PROTECTION_AREA_REG         (PU_BASE + 0x14)
#define PU_PROTECTION_AREA_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define AP_0_FIELD      0
#define AP_1_FIELD      2
#define AP_2_FIELD      4
#define AP_3_FIELD      6
#define AP_4_FIELD      8
#define AP_5_FIELD     10
#define AP_6_FIELD     12
#define AP_7_FIELD     14
#define   NBITS_AP        2

// Access definitions
#define SUPER_NONE_USER_NONE   0
#define SUPER_FULL_USER_NONE   1
#define SUPER_FULL_USER_READ   2
#define SUPER_FULL_USER_FULL   3


/*
 * Memory Area Definition Registers definition (read/write)
 *     These regsiters define 8 programmable regions in memory.
 *     AREA_SIZE :  0 to 10 : reserved
 *     AREA_SIZE : 11 to 31 : size = 2 power (AREA_SIZE + 1)
 */
#define PU_AREA_DEF0_REG         (PU_BASE + 0x20)
#define PU_AREA_DEF1_REG         (PU_BASE + 0x24)
#define PU_AREA_DEF2_REG         (PU_BASE + 0x28)
#define PU_AREA_DEF3_REG         (PU_BASE + 0x2c)
#define PU_AREA_DEF4_REG         (PU_BASE + 0x30)
#define PU_AREA_DEF5_REG         (PU_BASE + 0x34)
#define PU_AREA_DEF6_REG         (PU_BASE + 0x38)
#define PU_AREA_DEF7_REG         (PU_BASE + 0x3c)
#define PU_AREA_DEF0_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define AREA_EN_BIT           0
#define AREA_SIZE_FIELD       1
#define   NBITS_AREA_SIZE       5
#define AREA_BASE_ADR_FIELD  12
#define   NBITS_AREA_BASE      20


/*
 * Cache Invalidate Register definition (write only)
 *     Writing to this register marks all lines of the cache as invalid.
 */
#define PU_CACHE_INVALIDATE_REG         (PU_BASE + 0x1c)
#define PU_CACHE_INVALIDATE_REG_ACCESS  (WRITE_ACCESS)


/*
 **********************************************************
 *  TWSI (Two-Wire Serial Interface) Register Definitions
 **********************************************************
 */

/*
 * TWSI Control Register definition (read/write)
 */
#define TWSI0_CONTROL_REG         (TWSI0_BASE + 0x00)
#define TWSI1_CONTROL_REG         (TWSI1_BASE + 0x00)
#define TWSI_CONTROL_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define TWSI_CONTROL_RESET_VALUE  0x00000000UL
#define TWSI_SLV_EN_BIT        0
#define TWSI_MSTR_EN_BIT       1
#define TWSI_SCALE_FIELD       3
#define   NBITS_TWSI_SCALE       2
#define TWSI_GC_EN_BIT         5


/*
 * TWSI Master Command Register definition (read/write)
 */
#define TWSI0_MASTER_CMD_REG         (TWSI0_BASE + 0x04)
#define TWSI1_MASTER_CMD_REG         (TWSI1_BASE + 0x04)
#define TWSI_MASTER_CMD_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define TWSI_MASTER_CMD_RESET_VALUE  0x00000000UL
#define TWSI_WR_REQ_BIT      0
#define TWSI_RD_REQ_BIT      1
#define TWSI_STOP_REQ_BIT    2
#define TWSI_START_REQ_BIT   3
#define TWSI_ABORT_CMD_BIT   6


/*
 * TWSI Timing Register definition (read/write)
 *     Clock Low  Period = (TLOW +1)*(System clock period)*(prescale factor)
 *     Clock High Period = (THIGH+1)*(System clock period)*(prescale factor)
 *     Bus Free Time     = (TBUF +1)*(System clock period)*(prescale factor)
 *     Data Hold Time    = (THOLD+1)*(System clock period)*(prescale factor)
 */
#define TWSI0_TIMING_REG         (TWSI0_BASE + 0x08)
#define TWSI1_TIMING_REG         (TWSI1_BASE + 0x08)
#define TWSI_TIMING_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define TWSI_TLOW_FIELD      0
#define   NBITS_TWSI_TLOW      8
#define TWSI_THIGH_FIELD     8
#define   NBITS_TWSI_THIGH     8
#define TWSI_TBUF_FIELD     16
#define   NBITS_TWSI_TBUF      8
#define TWSI_THOLD_FIELD    24
#define   NBITS_TWSI_THOLD     1


/*
 * TWSI Timeout Register definition (read/write)
 *     Clock Low Timeout Period =
 *         (Timeout+1)*(System clock period)*(prescale factor)
 */
#define TWSI0_TIMEOUT_REG        (TWSI0_BASE + 0x0c)
#define TWSI1_TIMEOUT_REG        (TWSI1_BASE + 0x0c)
#define TWSI_TIMEOUT_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define TWSI_TIMEOUT_FIELD     0
#define   NBITS_TWSI_TIMEOUT    16
#define TWSI_TOUT_EN_BIT      16


/*
 * TWSI Slave Address Register definition (read/write)
 */
#define TWSI0_SLAVE_ADDRESS_REG        (TWSI0_BASE + 0x14)
#define TWSI1_SLAVE_ADDRESS_REG        (TWSI1_BASE + 0x14)
#define TWSI_SLAVE_ADDRESS_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define TWSI_SLV_ADDR_FIELD     0
#define   NBITS_TWSI_SLV_ADDR     10
#define TWSI_TEN_ADDR_EN_BIT    10


/*
 * TWSI Slave Data Register definition (read/write)
 */
#define TWSI0_SLAVE_DATA_REG        (TWSI0_BASE + 0x18)
#define TWSI1_SLAVE_DATA_REG        (TWSI1_BASE + 0x18)
#define TWSI_SLAVE_DATA_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define TWSI_SLV_DATA_FIELD     0
#define   NBITS_TWSI_SLV_DATA     8


/*
 * TWSI Address Capture Register definition (read only)
 */
#define TWSI0_ADDRESS_CAPTURE_REG        (TWSI0_BASE + 0x1c)
#define TWSI1_ADDRESS_CAPTURE_REG        (TWSI1_BASE + 0x1c)
#define TWSI_ADDRESS_CAPTURE_REG_ACCESS  (READ_ACCESS)
#define TWSI_TRANS_ADDR_FIELD   0
#define   NBITS_TWSI_TRANS_ADDR   14


/*
 * TWSI Master Target Address Register definition (read/write)
 */
#define TWSI0_MSTR_TARGET_ADDRESS_REG       (TWSI0_BASE + 0x20)
#define TWSI1_MSTR_TARGET_ADDRESS_REG       (TWSI1_BASE + 0x20)
#define TWSI_MSTR_TARGET_ADDRESS_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define TWSI_MSTR_ADDR_FIELD     0
#define   NBITS_TWSI_MSTR_ADDR     7


/*
 * TWSI Master Data Register definition (read/write)
 */
#define TWSI0_MSTR_DATA_REG       (TWSI0_BASE + 0x24)
#define TWSI1_MSTR_DATA_REG       (TWSI1_BASE + 0x24)
#define TWSI_MSTR_DATA_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define TWSI_MSTR_DATA_FIELD    0
#define   NBITS_TWSI_MSTR_DATA    8


/*
 * TWSI Status Register definition (read only)
 */
#define TWSI0_STATUS_REG        (TWSI0_BASE + 0x28)
#define TWSI1_STATUS_REG        (TWSI1_BASE + 0x28)
#define TWSI_STATUS_REG_ACCESS  (READ_ACCESS)
#define TWSI_STATUS_RESET_VALUE  0x00400000UL
#define TWSI_SLV_RD_BIT        0
#define TWSI_SLV_WR_BIT        1
#define TWSI_GEN_CALL_BIT      2
#define TWSI_MSTR_WR_BIT       3
#define TWSI_MSTR_RD_BIT       4
#define TWSI_ANACK_BIT         6
#define TWSI_DNACK_BIT         7
#define TWSI_SLV_TIMEOUT_BIT   8
#define TWSI_MSTR_TIMEOUT_BIT  9
#define TWSI_SLV_WBEND_BIT     10
#define TWSI_SLV_RBEND_BIT     11
#define TWSI_ARB_LOSS_BIT      12
#define TWSI_EXT_CTIMEOUT_BIT  13


/*
 * TWSI Status Enable Register definition (read/write)
 *     Write a '1' to enable the corresponding status bit.
 */
#define TWSI0_STATUS_EN_REG         (TWSI0_BASE + 0x2c)
#define TWSI1_STATUS_EN_REG         (TWSI1_BASE + 0x2c)
#define TWSI_STATUS_EN_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define TWSI_STATUS_EN_RESET_VALUE  0x00000000UL
#define TWSI_SLV_RD_EN_BIT          0
#define TWSI_SLV_WR_EN_BIT          1
#define TWSI_GEN_CALL_EN_BIT        2
#define TWSI_MSTR_WR_EN_BIT         3
#define TWSI_MSTR_RD_EN_BIT         4
#define TWSI_ANACK_EN_BIT           6
#define TWSI_DNACK_EN_BIT           7
#define TWSI_SLV_TIMEOUT_EN_BIT     8
#define TWSI_MSTR_TIMEOUT_EN_BIT    9
#define TWSI_SLV_WBEND_EN_BIT       10
#define TWSI_SLV_RBEND_EN_BIT       11
#define TWSI_ARB_LOSS_EN_BIT        12
#define TWSI_EXT_CTIMEOUT_EN_BIT    13

/*
 * TWSI Status Clear Register definition (write only)
 *     Write a '1' to clear the corresponding status bit.
 */
#define TWSI0_STATUS_CLEAR_REG        (TWSI0_BASE + 0x30)
#define TWSI1_STATUS_CLEAR_REG        (TWSI1_BASE + 0x30)
#define TWSI_STATUS_CLEAR_REG_ACCESS  (WRITE_ACCESS)
#define TWSI_SLV_RD_CLR_BIT         0
#define TWSI_SLV_WR_CLR_BIT         1
#define TWSI_GEN_CALL_CLR_BIT       2
#define TWSI_MSTR_WR_CLR_BIT        3
#define TWSI_MSTR_RD_CLR_BIT        4
#define TWSI_ANACK_CLR_BIT          6
#define TWSI_DNACK_CLR_BIT          7
#define TWSI_SLV_TIMEOUT_CLR_BIT    8
#define TWSI_MSTR_TIMEOUT_CLR_BIT   9
#define TWSI_SLV_WBEND_CLR_BIT      10
#define TWSI_SLV_RBEND_CLR_BIT      11
#define TWSI_ARB_LOSS_CLR_BIT       12
#define TWSI_EXT_CTIMEOUT_CLR_BIT   13



/*
 **********************************************************
 *  Ethernet Registers Definitions
 **********************************************************
 */

/*
 * Ethernet Module Control Register definition
 *     Byte Order: 0 = LSB first
 *                 1 = MSB first
 *     Clock Divisor: 00 = Sysclock/14
 *                    01 = Sysclock/28
 *                    10 = Sysclock/34
 *                    11 = Sysclock/40
 */
#define ENET0_CONTROL_REG             (ENET0_BASE + 0x00)
#define ENET1_CONTROL_REG             (ENET1_BASE + 0x00)
#define ENET_CONTROL_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define ENET_CONTROL_RESET_VALUE      0x00000000
#define ENET_RESET_BIT                0
#define ENET_TX_DMA_BIT               1
#define ENET_RX_DMA_BIT               2
#define ENET_BYTE_ORDER_BIT           3
#define ENET_MDC_CLK_DIV_FIELD        4
#define   NBITS_ENET_MDC_CLK_DIV        5
#define ENET_CAMHIT_EN_BIT            11


/*
 * Ethernet Tx Write Data Port Register definition
 *     Provides transmit packet data in non-DMA mode.
 *     Don't use when Tx DMA mode is enabled.
 */
#define ENET0_TX_WRITE_DATA_REG        (ENET0_BASE + 0x04)
#define ENET1_TX_WRITE_DATA_REG        (ENET1_BASE + 0x04)
#define ENET_TX_WRITE_DATA_REG_ACCESS  (WRITE_ACCESS)


/*
 * Ethernet Tx Packet Boundary/Options Control Register definition
 *     Provides control/status info for transmit packets in non-DMA mode.
 *     Sets per-packet transmit options for transmit packets in non-DMA mode.
 *     Don't use when Tx DMA mode is enabled.
 *     Writing '1' to TxLastWord bit indicates that next word written to the
 *         Tx Packet Write Data Port will contain the last byte of the packet.
 *     LastByteValid  ByteOrder=0 ByteOrder=1
 *             Field  Valid Bits  Valid Bits
 *                00        0-31        0-31
 *                01        0--7       24-31
 *                10        0-15       16-31
 *                11        0-23        8-31
 *     The Tx Options bits[18:16] set the options for the next packet,
 *         and should be written after TxLastWord is written
 *         and before the first word of the next packet is written.
 */
#define ENET0_TX_PACKET_CTL_REG             (ENET0_BASE + 0x08)
#define ENET1_TX_PACKET_CTL_REG             (ENET1_BASE + 0x08)
#define ENET_TX_PACKET_CTL_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define ENET_TX_PACKET_CTL_RESET_VALUE      0x00000000
#define ENET_TX_LAST_WORD_BIT               0
#define ENET_TX_LAST_BVALID_FIELD           1
#define   NBITS_ENET_TX_LAST_BVALID           2
#define ENET_TX_OPTIONS_FIELD               16
#define   NBITS_ENET_TX_OPTIONS               3
// Tx Options Bits
#define   ENET_TX_DONT_PAD_SPKTS_BIT        16
#define   ENET_TX_NO_CRC_APPEND_BIT         17
#define   ENET_TX_INT_AFTER_TX_BIT          18


/*
 * Ethernet Tx FIFO Status Register definition
 *     Provides transmit write buffer and FIFO status.
 * Ethernet Tx Interrupt Enable Register definition
 *     Setting a bit position to '1' will enable the corresponding interrupt.
 * Ethernet Tx Interrupt Status Register definition
 *     Shows a '1' in each bit position where an interrupt has occurred.
 * Ethernet Tx Interrupt Clear Register definition
 *     Writing a '1' to a bit position will clear the corresponding interrupt.
 */
#define ENET0_TX_FIFO_STATUS_REG            (ENET0_BASE + 0x0c)
#define ENET1_TX_FIFO_STATUS_REG            (ENET1_BASE + 0x0c)
#define ENET_TX_FIFO_STATUS_REG_ACCESS      (READ_ACCESS)
#define ENET_TX_FIFO_STATUS_RESET_VALUE     0x00000014
#define ENET_WB_EMPTY_BIT                   2
#define ENET_WB_FULL_BIT                    3
#define ENET_WB_OVERFLOW_BIT                4
#define ENET_TX_FIFO_EMPTY_BIT              5
#define ENET_TX_FIFO_FULL_BIT               6

/*
 * Ethernet Tx Transmit Status Register definition
 *     Contains the Transmit Status from the MAC of last transmitted packet.
 *     Updated when packet is transmitted, not when its delivered into FIFO.
 */
#define ENET0_TX_TRANSMIT_STATUS_REG        (ENET0_BASE + 0x10)
#define ENET1_TX_TRANSMIT_STATUS_REG        (ENET1_BASE + 0x10)
#define ENET_TX_TRANSMIT_STATUS_REG_ACCESS      (READ_ACCESS)
#define ENET_TX_TRANSMIT_STATUS_RESET_VALUE 0x00000000
#define ENET_TX_COL_FIELD                   0
#define   NBITS_ENET_TX_COL                   4
#define ENET_EX_COL_BIT                     4
#define ENET_TX_DEFER_BIT                   5
#define ENET_PAUSED_BIT                     6
#define ENET_INT_TX_BIT                     7
#define ENET_UNDER_BIT                      8
#define ENET_EX_DEFER_BIT                   9
#define ENET_LCARR_BIT                      10
#define ENET_LATE_COL_BIT                   12
#define ENET_TX_PAR_BIT                     13
#define ENET_TX_COMP_BIT                    14
#define ENET_TX_HALTED_BIT                  15
#define ENET_SQ_ERR_BIT                     16
#define ENET_TX_MCAST_BIT                   17
#define ENET_TX_BCAST_BIT                   18
#define ENET_TX_VLAN_BIT                    19
#define ENET_TX_MACC_BIT                    20
#define ENET_TX_PAUSE_BIT                   21
#define ENET_TX_HNR_BIT                     22
#define ENET_TX_COUNT_LSB_FIELD             24
#define   NBITS_ENET_TX_COUNT_LSB             8


/*
 * Ethernet Tx Interrupt Enable Register definition
 *     Setting a bit position to '1' will enable the corresponding interrupt.
 * Ethernet Tx Interrupt Status Register definition
 *     Shows a '1' in each bit position where an interrupt has occurred.
 * Ethernet Tx Interrupt Clear Register definition
 *     Writing a '1' to a bit position will clear the corresponding interrupt.
 */
#define ENET0_TX_INT_ENABLE_REG             (ENET0_BASE + 0x14)
#define ENET1_TX_INT_ENABLE_REG             (ENET1_BASE + 0x14)
#define ENET_TX_INT_ENABLE_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define ENET_TX_INT_ENABLE_RESET_VALUE      0x00000000

#define ENET0_TX_INT_STATUS_REG             (ENET0_BASE + 0x18)
#define ENET1_TX_INT_STATUS_REG             (ENET1_BASE + 0x18)
#define ENET_TX_INT_STATUS_REG_ACCESS       (READ_ACCESS)
#define ENET_TX_INT_STATUS_RESET_VALUE      0x00000000

#define ENET0_TX_INT_CLEAR_REG              (ENET0_BASE + 0x1c)
#define ENET1_TX_INT_CLEAR_REG              (ENET1_BASE + 0x1c)
#define ENET_TX_INT_CLEAR_REG_ACCESS        (WRITE_ACCESS)

#define ENET_INT_WB_EMPTY_BIT               2
#define ENET_INT_WB_FULL_BIT                3
#define ENET_INT_WB_OVERFLOW_BIT            4
#define ENET_INT_TX_FIFO_EMPTY_BIT          5
#define ENET_INT_TX_FIFO_FULL_BIT           6
#define ENET_INT_MAC_TX_BIT                 7
#define ENET_INT_MAC_TX_LINK_BIT            16


/*
 * Ethernet Tx Packet Counter Register definition
 *     If any byte is written to, all other bytes are cleared to '0'.
 *    Count = number of packets transmitted by MAC since last counter reset.
 */
#define ENET0_TX_PACKET_COUNTER_REG             (ENET0_BASE + 0x20)
#define ENET1_TX_PACKET_COUNTER_REG             (ENET1_BASE + 0x20)
#define ENET_TX_PACKET_COUNTER_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define ENET_TX_PACKET_COUNTER_RESET_VALUE      0x00000000
#define ENET_TX_COUNT_FIELD                     0
#define   NBITS_ENET_TX_COUNT                     16


/*
 * Ethernet Rx Read Data Port Register definition
 *     Used to read received packet data in non-DMA mode.
 *     Reads from this register return garbage while Rx DMA mode is enabled.
 *     Reads from this register empties the Rx FIFO.
 *     Packet boundaries can be determined using the Rx Read Control register.
 *     Reads from this register when there's no data in Rx FIFO, return garbage
 *         and set the Rb_underflow bit in the Rx Read Status register, and
 *         may generate Interrupt depending on value of corresponding bit in
 *         the Interrupt mask register.
 */
#define ENET0_RX_READ_DATA_REG              (ENET0_BASE + 0x24)
#define ENET1_RX_READ_DATA_REG              (ENET1_BASE + 0x24)
#define ENET_RX_READ_DATA_REG_ACCESS        (READ_ACCESS)


/*
 * Ethernet Rx FIFO Status Register definition
 *     Provides control and status info for receive packets in non-DMA mode.
 *     This register should not be used when Rx DMA mode is enabled.
 *     LastByteValid  ByteOrder=0 ByteOrder=1
 *             Field  Valid Bits  Valid Bits
 *                00        0-31        0-31
 *                01        0--7       24-31
 *                10        0-15       16-31
 *                11        0-23        8-31
 */
#define ENET0_RX_FIFO_STATUS_REG            (ENET0_BASE + 0x28)
#define ENET1_RX_FIFO_STATUS_REG            (ENET1_BASE + 0x28)
#define ENET_RX_FIFO_STATUS_REG_ACCESS      (READ_ACCESS)
#define ENET_RX_FIFO_STATUS_RESET_VALUE     0x00000000
#define ENET_RX_LAST_WORD_BIT               0
#define ENET_RB_LAST_PRESENT_BIT            1
#define ENET_RB_EMPTY_BIT                   2
#define ENET_RB_FULL_BIT                    3
#define ENET_RB_UNDERFLOW_BIT               4
#define ENET_RD_FIFO_EMPTY_BIT              5
#define ENET_RD_FIFO_FULL_BIT               6
#define ENET_RX_LAST_BVALID_FIELD           8
#define   NBITS_ENET_RX_LAST_BVALID           2


/*
 * Ethernet Rx Receive Status Register definition
 *     Provides control and status info for receive packets in non-DMA mode.
 *     This register should not be used when Rx DMA mode is enabled.
 *     ArcStatus  Action    Reason
 *          0000  Toss      MAC control received and PassCtl==0
 *          0001  Toss      6 <= length < minLen, and was keep otherwise
 *          0010  Toss      ARC match, negative filtering
 *          0011  Reserved
 *          0100  Reserved
 *          0101  Reserved
 *          0110  Toss      no match, no enable, positive filtering
 *          0111  Toss      length < 6 and shortEn not set
 *          1000  Keep      broadcast, multicast, unicast enabled and matched
 *          1001  Reserved
 *          1010  Keep      ARC match positive filtering
 *          1011  Reserved
 *          1100  Reserved
 *          1101  Reserved
 *          1110  Keep      no match, negative filtering
 *          1111  Reserved
 */
#define ENET0_RX_RECEIVE_STATUS_REG             (ENET0_BASE + 0x2c)
#define ENET1_RX_RECEIVE_STATUS_REG             (ENET1_BASE + 0x2c)
#define ENET_RX_RECEIVE_STATUS_REG_ACCESS       (READ_ACCESS)
#define ENET_RX_RECEIVE_STATUS_RESET_VALUE      0x00000000
#define ENET_LEN_ERR_BIT                        4
#define ENET_CTL_RECD_BIT                       5
#define ENET_INT_RX_BIT                         6
#define ENET_ALIGN_ERR_BIT                      8
#define ENET_CRC_ERR_BIT                        9
#define ENET_OVERFLOW_BIT                       10
#define ENET_LONG_ERR_BIT                       11
#define ENET_RX_PARR_BIT                        13
#define ENET_GOOD_BIT                           14
#define ENET_RX_HALTED_BIT                      15
#define ENET_RX_MCAST_BIT                       17
#define ENET_RX_BCAST_BIT                       18
#define ENET_RX_VLAN_BIT                        19
#define ENET_RX_PAUSE_BIT                       20
#define ENET_ARC_STATUS_FIELD                   21
#define   NBITS_ENET_ARC_STATUS                   4
#define ENET_ARC_ENTRY_FIELD                    25
#define   NBITS_ENET_ARC_ENTRY                    2


/*
 * Ethernet Rx Interrupt Enable Register definition
 *     Setting a bit position to '1' will enable the corresponding interrupt.
 * Ethernet Rx Interrupt Status Register definition
 *     Shows a '1' in each bit position where an interrupt has occurred.
 * Ethernet Rx Interrupt Clear Register definition
 *     Writing a '1' to a bit position will clear the corresponding interrupt.
 */
#define ENET0_RX_INT_ENABLE_REG             (ENET0_BASE + 0x30)
#define ENET1_RX_INT_ENABLE_REG             (ENET1_BASE + 0x30)
#define ENET_RX_INT_ENABLE_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define ENET_RX_INT_ENABLE_RESET_VALUE      0x00000000

#define ENET0_RX_INT_STATUS_REG             (ENET0_BASE + 0x34)
#define ENET1_RX_INT_STATUS_REG             (ENET1_BASE + 0x34)
#define ENET_RX_INT_STATUS_REG_ACCESS       (READ_ACCESS)
#define ENET_RX_INT_STATUS_RESET_VALUE      0x00000000

#define ENET0_RX_INT_CLEAR_REG              (ENET0_BASE + 0x38)
#define ENET1_RX_INT_CLEAR_REG              (ENET1_BASE + 0x38)
#define ENET_RX_INT_CLEAR_REG_ACCESS        (WRITE_ACCESS)

#define ENET_INT_RX_LAST_WORD_BIT           0
#define ENET_INT_RB_LAST_PRESENT_BIT        1
#define ENET_INT_RB_EMPTY_BIT               2
#define ENET_INT_RB_FULL_BIT                3
#define ENET_INT_RB_UNDERFLOW_BIT           4
#define ENET_INT_RX_FIFO_EMPTY_BIT          5
#define ENET_INT_RX_FIFO_FULL_BIT           6
#define ENET_INT_MAC_RX_BIT                 7
#define ENET_INT_MAC_RX_LINK_BIT            16


/*
 * Ethernet Rx Packet Keep/Toss Counters Register definition
 *     If any byte is written to, all other bytes are cleared to '0'.
 */
#define ENET0_RX_PACKET_COUNTER_REG         (ENET0_BASE + 0x3c)
#define ENET1_RX_PACKET_COUNTER_REG         (ENET1_BASE + 0x3c)
#define ENET_RX_PACKET_COUNTER_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define ENET_RX_PACKET_COUNTER_RESET_VALUE  0x00000000
#define ENET_RX_KEEP_COUNT_FIELD            0
#define   NBITS_ENET_RX_KEEP_COUNT            16
#define ENET_RX_TOSS_COUNT_FIELD            16
#define   NBITS_ENET_RX_TOSS_COUNT            16


/*
 * Ethernet ARC Registers definition
 *     These should not be written to while MAC is enabled to receive packets.
 */
#define ENET0_ARC_40_REG    (ENET0_BASE + 0x40)
#define ENET1_ARC_40_REG    (ENET1_BASE + 0x40)
#define ENET0_ARC_44_REG    (ENET0_BASE + 0x44)
#define ENET1_ARC_44_REG    (ENET1_BASE + 0x44)
#define ENET0_ARC_48_REG    (ENET0_BASE + 0x48)
#define ENET1_ARC_48_REG    (ENET1_BASE + 0x48)
#define ENET0_ARC_4C_REG    (ENET0_BASE + 0x4c)
#define ENET1_ARC_4C_REG    (ENET1_BASE + 0x4c)
#define ENET0_ARC_50_REG    (ENET0_BASE + 0x50)
#define ENET1_ARC_50_REG    (ENET1_BASE + 0x50)
#define ENET_ARC_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)


/*
 * Ethernet MAC Pause Count Register definition
 * Ethernet MAC Remote Pause Count Register definition
 */
#define ENET0_MAC_PAUSE_COUNT_REG           (ENET0_BASE + 0xb0)
#define ENET1_MAC_PAUSE_COUNT_REG           (ENET1_BASE + 0xb0)
#define ENET0_MAC_REMOTE_PAUSE_COUNT_REG    (ENET0_BASE + 0xb4)
#define ENET1_MAC_REMOTE_PAUSE_COUNT_REG    (ENET1_BASE + 0xb4)
#define ENET_MAC_PAUSE_COUNT_REG_ACCESS     (READ_ACCESS)
#define ENET_MAC_PAUSE_COUNT_RESET_VALUE    0x00000000
#define MAC_PAUSE_COUNT_FIELD               0
#define   NBITS_MAC_PAUSE_COUNT               16


/*
 * Ethernet MAC Control Register definition
 */
#define ENET0_MAC_CONTROL_REG           (ENET0_BASE + 0xc0)
#define ENET1_MAC_CONTROL_REG           (ENET1_BASE + 0xc0)
#define ENET_MAC_CONTROL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_CONTROL_RESET_VALUE    0x0000
#define MAC_HALT_REQ_BIT                0
#define MAC_HALT_IMM_BIT                1
#define MAC_RESET_BIT                   2
#define MAC_FULL_DUP_BIT                3
#define MAC_LOOP_BIT                    4
#define MAC_CONN_FIELD                  5
#define   NBITS_MAC_CONN                  2
#define MAC_LOOP10_BIT                  7
#define MAC_LNK_CHG_BIT                 8
#define MAC_MISS_ROLL_BIT               10      // read-only
#define MAC_EN_MISS_ROLL_BIT            13
#define MAC_LINK10_BIT                  15      // read-only

// MAC_CONN_FIELD definitions
#define MAC_CONN_AUTO_BIT               0
#define MAC_CONN_FORCE_10_BIT           1
#define MAC_CONN_FORCE_MII_BIT          2


/*
 * Ethernet MAC ARC Control Register definition
 */
#define ENET0_MAC_ARC_CONTROL_REG           (ENET0_BASE + 0xc4)
#define ENET1_MAC_ARC_CONTROL_REG           (ENET1_BASE + 0xc4)
#define ENET_MAC_ARC_CONTROL_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_ARC_CONTROL_RESET_VALUE    0x0000
#define MAC_STATION_ACC_BIT                 0
#define MAC_GROUP_ACC_BIT                   1
#define MAC_BROAD_ACC_BIT                   2
#define MAC_NEG_ARC_BIT                     3
#define MAC_COMP_EN_BIT                     4


/*
 * Ethernet MAC Transmit Control Register definition
 */
#define ENET0_MAC_TX_CONTROL_REG            (ENET0_BASE + 0xc8)
#define ENET1_MAC_TX_CONTROL_REG            (ENET1_BASE + 0xc8)
#define ENET_MAC_TX_CONTROL_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_TX_CONTROL_RESET_VALUE     0x0000
#define MAC_TX_EN_BIT                       0
#define MAC_TX_HALT_BIT                     1
#define MAC_NO_PAD_BIT                      2
#define MAC_NO_CRC_BIT                      3
#define MAC_NO_EX_DEF_BIT                   5
#define MAC_SD_PAUSE_BIT                    6
#define MAC_EN_UNDER_BIT                    8
#define MAC_EN_EX_DEFER_BIT                 9
#define MAC_EN_LCARR_BIT                    10
#define MAC_EN_EX_COLL_BIT                  11
#define MAC_EN_LATE_COLL_BIT                12
#define MAC_EN_TX_PAR_BIT                   13
#define MAC_EN_COMPL_BIT                    14


/*
 * Ethernet MAC Transmit Status Register definition
 */
#define ENET0_MAC_TX_STATUS_REG             (ENET0_BASE + 0xcc)
#define ENET1_MAC_TX_STATUS_REG             (ENET1_BASE + 0xcc)
#define ENET_MAC_TX_STATUS_REG_ACCESS       (READ_ACCESS)
#define ENET_MAC_TX_STATUS_RESET_VALUE      0x000000
#define MAC_TX_COLL_FIELD                   0
#define   NBITS_MAC_TX_COLL                   4
#define MAC_EX_COLL_BIT                     4
#define MAC_TX_DEFER_BIT                    5
#define MAC_PAUSED_BIT                      6
#define MAC_INT_TX_BIT                      7
#define MAC_UNDER_BIT                       8
#define MAC_EX_DEFER_BIT                    9
#define MAC_LCARR_BIT                       10
#define MAC_LATE_COLL_BIT                   12
#define MAC_TX_PARR_BIT                     13
#define MAC_COMP_BIT                        14
#define MAC_TX_HALTED_BIT                   15
#define MAC_SQ_ERR_BIT                      16
#define MAC_TX_MCAST_BIT                    17
#define MAC_TX_BCAST_BIT                    18
#define MAC_TX_VLAN_BIT                     19
#define MAC_TX_MACC_BIT                     20
#define MAC_TX_PAUSE_BIT                    21
#define MAC_TX_HNR_BIT                      22


/*
 * Ethernet MAC Receive Control Register definition
 */
#define ENET0_MAC_RX_CONTROL_REG            (ENET0_BASE + 0xd0)
#define ENET1_MAC_RX_CONTROL_REG            (ENET1_BASE + 0xd0)
#define ENET_MAC_RX_CONTROL_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_RX_CONTROL_RESET_VALUE     0x0000
#define MAC_RX_EN_BIT                       0
#define MAC_RX_HALT_BIT                     1
#define MAC_LONG_EN_BIT                     2
#define MAC_SHORT_EN_BIT                    3
#define MAC_STRIP_CRC_BIT                   4
#define MAC_PASS_CTL_BIT                    5
#define MAC_IGNORE_CRC_BIT                  6
#define MAC_EN_ALIGN_BIT                    8
#define MAC_EN_CRC_ERR_BIT                  9
#define MAC_EN_OVER_BIT                     10
#define MAC_EN_LONG_ERR_BIT                 11
#define MAC_EN_RX_PARR_BIT                  13
#define MAC_EN_GOOD_BIT                     14


/*
 * Ethernet MAC Receive Status Register definition
 */
#define ENET0_MAC_RX_STATUS_REG             (ENET0_BASE + 0xd4)
#define ENET1_MAC_RX_STATUS_REG             (ENET1_BASE + 0xd4)
#define ENET_MAC_RX_STATUS_REG_ACCESS       (READ_ACCESS)
#define ENET_MAC_RX_STATUS_RESET_VALUE      0x00000000
#define MAC_LEN_ERR_BIT                     4
#define MAC_CTL_RECD_BIT                    5
#define MAC_INT_RX_BIT                      6
#define MAC_ALIGN_ERR_BIT                   8
#define MAC_CRC_ERR_BIT                     9
#define MAC_OVERFLOW_BIT                    10
#define MAC_LONG_ERR_BIT                    11
#define MAC_RX_PARR_BIT                     13
#define MAC_GOOD_BIT                        14
#define MAC_RX_HALTED_BIT                   15
#define MAC_RX_MCAST_BIT                    17
#define MAC_RX_BCAST_BIT                    18
#define MAC_RX_VLAN_BIT                     19
#define MAC_RX_PAUSE_BIT                    20
#define MAC_ARC_STATUS_FIELD                21
#define   NBITS_MAC_ARC_STATUS                4
#define MAC_ARC_ENT_FIELD                   25
#define   NBITS_MAC_ARC_ENT                   5


/*
 * Ethernet MAC SM Data Register definition
 */
#define ENET0_MAC_SM_DATA_REG               (ENET0_BASE + 0xd8)
#define ENET1_MAC_SM_DATA_REG               (ENET1_BASE + 0xd8)
#define ENET_MAC_SM_DATA_REG_ACCESS         (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_SM_DATA_RESET_VALUE        0x0000
#define MAC_MD_DATA_FIELD    0
#define   NBITS_MAC_MD_DATA   16


/*
 * Ethernet MAC SM Control/Address Register definition
 *     Before accessing PHY regs, software must ensure BUSY bit is not set.
 */
#define ENET0_MAC_SM_CTRL_ADDR_REG         (ENET0_BASE + 0xdc)
#define ENET1_MAC_SM_CTRL_ADDR_REG         (ENET1_BASE + 0xdc)
#define ENET_MAC_SM_CTRL_ADDR_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_SM_CTRL_ADDR_RESET_VALUE   0x0000
#define MAC_ADDR_FIELD                      0
#define   NBITS_MAC_ADDR                      5
#define MAC_PHY_FIELD                       5
#define   NBITS_MAC_PHY                       5
#define MAC_WRITE_BIT                       10
#define MAC_BUSY_BIT                        11
#define MAC_PRE_SUP_BIT                     12


/*
 * Ethernet MAC ARC Enable Register definition
 */
#define ENET0_MAC_ARC_ENABLE_REG            (ENET0_BASE + 0xe8)
#define ENET1_MAC_ARC_ENABLE_REG            (ENET1_BASE + 0xe8)
#define ENET_MAC_ARC_ENABLE_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_ARC_ENABLE_RESET_VALUE     0x000000
#define MAC_ARC_ENABLE_FIELD                0
#define   NBITS_MAC_ARC_ENABLE                3


/*
 * Ethernet MAC Missed Error Count Register definition
 */
#define ENET0_MAC_MISSED_ERR_CNT_REG        (ENET0_BASE + 0xec)
#define ENET1_MAC_MISSED_ERR_CNT_REG        (ENET1_BASE + 0xec)
#define ENET_MAC_MISSED_ERR_CNT_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define ENET_MAC_MISSED_ERR_CNT_RESET_VALUE 0x00000000
#define MAC_MISS_CNT_FIELD                  0
#define   NBITS_MAC_MISS_CNT                  16



/*
 **********************************************************
 *  SSI/SPI PROM Registers Definitions
 **********************************************************
 */

/*
 * SSI/SPI Control Register definition
 *     Shift applied = 1 << (shift value + 1)
 */
#define SSI_CONTROL_REG                 (SSI_BASE + 0x00)
#define SSI_CONTROL_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define SSI_CONTROL_RESET_VALUE         0x00000000
#define SSI_EN_BIT                      0
#define SSI_RAW_MODE_EN_BIT             1
#define SSI_XFER_MODE_BIT               2
#define SSI_PRESCALER_FIELD             4
#define   NBITS_SSI_PRESCALER             2
#define SSI_FORCE_CS_BIT                6
#define SSI_CS_CTRL_BIT                 7
#define SSI_TX_SIZE_FIELD               12
#define   NBITS_TX_SIZE                   5
#define SSI_RX_SIZE_FIELD               20
#define   NBITS_SSI_RX_SIZE               5


/*
 * SSI/SPI Master Execute Register definition
 *     Setting the bit initiates a Master read or write transaction.
 *     Hardware clears bit when transaction is processed by master logic.
 *     Bit can be polled instead of the Ready/Busy status bit.
 */
#define SSI_XFER_INIT_REG               (SSI_BASE + 0x04)
#define SSI_XFER_INIT_REG_ACCESS        (READ_ACCESS | WRITE_ACCESS)
#define SSI_XFER_INIT_RESET_VALUE       0x00000000
#define SSI_XFER_REQ_BIT                0


/*
 * SSI/SPI Timing Register definition
 *    Serial eeprom clock frequency range is from 0 to about 250 kHz.
 *    (Tsck+1) * 2 * (System clock period) * (prescale factor)
 */
#define SSI_TIMING_CTRL_REG             (SSI_BASE + 0x08)
#define SSI_TIMING_CTRL_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define SSI_TIMING_CTRL_RESET_VALUE     0x00000000
#define SSI_TSCK_FIELD                  0
#define   NBITS_SSI_TSCK                  8


/*
 * SSI/SPI Data Input Register definition
 */
#define SSI_RX_DATA_REG                 (SSI_BASE + 0x0c)
#define SSI_RX_DATA_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define SSI_RX_DATA_RESET_VALUE         0x00000000
#define SSI_RX_DATA_FIELD               0
#define   NBITS_SSI_RX_DATA               16


/*
 * SSI/SPI Command/Address I/O Register definition
 */
#define SSI_TX_DATA_REG                 (SSI_BASE + 0x10)
#define SSI_TX_DATA_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define SSI_TX_DATA_RESET_VALUE         0x00000000
#define SSI_TX_DATA_FIELD               0
#define   NBITS_SSI_TX_DATA               32


/*
 * SSI/SPI Raw Mode Register definition
 */
#define SSI_RAW_MODE_REG                (SSI_BASE + 0x14)
#define SSI_RAW_MODE_REG_ACCESS         (READ_ACCESS | WRITE_ACCESS)
#define SSI_RAW_MODE_RESET_VALUE        0x00000000
#define SSI_MOSI_OUT_BIT                0
#define SSI_MISO_OUT_BIT                1
#define SSI_SCK_OUT_BIT                 2
#define SSI_SSN_OUT_BIT                 3
#define SSI_MOSI_IN_BIT                 4
#define SSI_MISO_IN_BIT                 5
#define SSI_SCK_IN_BIT                  6
#define SSI_SSN_IN_BIT                  7
#define SSI_MOSI_DIR_BIT                8
#define SSI_MISO_DIR_BIT                9
#define SSI_SCK_DIR_BIT                 10
#define SSI_SSN_DIR_BIT                 11


/*
 * SSI/SPI Status Register definition
 */
#define SSI_STATUS_REG                  (SSI_BASE + 0x1c)
#define SSI_STATUS_REG_ACCESS           (READ_ACCESS | WRITE_ACCESS)
#define SSI_STATUS_RESET_VALUE          0x00000000
#define SSI_STAT_READY_BUSY_BIT         0


/*
 **********************************************************
 *  MFTA Registers Definitions
 **********************************************************
 */
/*
 * MFTA Control Register definition
 */
#define MFTA_TIMER_CTRL_REG             (MFTA_BASE + 0x00)
#define MFTA_TIMER_CTRL_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER_CTRL_RESET_VALUE     0x03030303

#define MFTA_TW0_FIELD                  0
#define   NBITS_MFTA_TW0                  2
#define MFTA_TEN0_BIT                   2
#define MFTA_CLKSEL0_BIT                3
#define MFTA_UDCTRL0_BIT                4
#define MFTA_SBDCEN0_BIT                5
#define MFTA_TCCMA0_BIT                 6

#define MFTA_TW1_FIELD                  8
#define   NBITS_MFTA_TW1                  2
#define MFTA_TEN1_BIT                   10
#define MFTA_CLKSEL1_BIT                11
#define MFTA_UDCTRL1_BIT                12
#define MFTA_SBDCEN1_BIT                13
#define MFTA_TCCMA1_BIT                 14

#define MFTA_TW2_FIELD                  16
#define   NBITS_MFTA_TW2                  2
#define MFTA_TEN2_BIT                   18
#define MFTA_CLKSEL2_BIT                19
#define MFTA_UDCTRL2_BIT                20
#define MFTA_SBDCEN2_BIT                21
#define MFTA_TCCMA2_BIT                 22

#define MFTA_TW3_FIELD                  24
#define   NBITS_MFTA_TW3                  2
#define MFTA_TEN3_BIT                   26
#define MFTA_CLKSEL3_BIT                27
#define MFTA_UDCTRL3_BIT                28
#define MFTA_SBDCEN3_BIT                29
#define MFTA_TCCMA3_BIT                 30

/*
 * MFTA Interrupt Enable Register definition
 */
#define MFTA_INT_ENABLE_REG             (MFTA_BASE + 0x04)
#define MFTA_INT_ENABLE_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define MFTA_INT_ENABLE_RESET_VALUE     0x00000000

#define MFTA_TIMER0COMPA_IEN_BIT        0
#define MFTA_TIMER0COMPB_IEN_BIT        1
#define MFTA_TIMER0UDF_IEN_BIT          2
#define MFTA_TIMER0OVF_IEN_BIT          3
#define MFTA_TIMER0CAPTA_IEN_BIT        4
#define MFTA_TIMER0CAPTB_IEN_BIT        5

#define MFTA_TIMER1COMPA_IEN_BIT        8
#define MFTA_TIMER1COMPB_IEN_BIT        9
#define MFTA_TIMER1UDF_IEN_BIT          10
#define MFTA_TIMER1OVF_IEN_BIT          11
#define MFTA_TIMER1CAPTA_IEN_BIT        12
#define MFTA_TIMER1CAPTB_IEN_BIT        13

#define MFTA_TIMER2COMPA_IEN_BIT        16
#define MFTA_TIMER2COMPB_IEN_BIT        17
#define MFTA_TIMER2UDF_IEN_BIT          18
#define MFTA_TIMER2OVF_IEN_BIT          19
#define MFTA_TIMER2CAPTA_IEN_BIT        20
#define MFTA_TIMER2CAPTB_IEN_BIT        21

#define MFTA_TIMER3COMPA_IEN_BIT        24
#define MFTA_TIMER3COMPB_IEN_BIT        25
#define MFTA_TIMER3UDF_IEN_BIT          26
#define MFTA_TIMER3OVF_IEN_BIT          27
#define MFTA_TIMER3CAPTA_IEN_BIT        28
#define MFTA_TIMER3CAPTB_IEN_BIT        29

/*
 * MFTA Interrupt Status Register definition
 */
#define MFTA_INT_STATUS_REG             (MFTA_BASE + 0x08)
#define MFTA_INT_STATUS_REG_ACCESS      (READ_ACCESS)
#define MFTA_INT_STATUS_RESET_VALUE     0x00000000

#define MFTA_TIMER0COMPA_ISF_BIT        0
#define MFTA_TIMER0COMPB_ISF_BIT        1
#define MFTA_TIMER0UDF_ISF_BIT          2
#define MFTA_TIMER0OVF_ISF_BIT          3
#define MFTA_TIMER0CAPTA_ISF_BIT        4
#define MFTA_TIMER0CAPTB_ISF_BIT        5

#define MFTA_TIMER1COMPA_ISF_BIT        8
#define MFTA_TIMER1COMPB_ISF_BIT        9
#define MFTA_TIMER1UDF_ISF_BIT          10
#define MFTA_TIMER1OVF_ISF_BIT          11
#define MFTA_TIMER1CAPTA_ISF_BIT        12
#define MFTA_TIMER1CAPTB_ISF_BIT        13

#define MFTA_TIMER2COMPA_ISF_BIT        16
#define MFTA_TIMER2COMPB_ISF_BIT        17
#define MFTA_TIMER2UDF_ISF_BIT          18
#define MFTA_TIMER2OVF_ISF_BIT          19
#define MFTA_TIMER2CAPTA_ISF_BIT        20
#define MFTA_TIMER2CAPTB_ISF_BIT        21

#define MFTA_TIMER3COMPA_ISF_BIT        24
#define MFTA_TIMER3COMPB_ISF_BIT        25
#define MFTA_TIMER3UDF_ISF_BIT          26
#define MFTA_TIMER3OVF_ISF_BIT          27
#define MFTA_TIMER3CAPTA_ISF_BIT        28
#define MFTA_TIMER3CAPTB_ISF_BIT        29

/*
 * MFTA Interrupt Clear Register definition
 */
#define MFTA_INT_CLEAR_REG              (MFTA_BASE + 0x0c)
#define MFTA_INT_CLEAR_REG_ACCESS       (WRITE_ACCESS)

#define MFTA_TIMER0COMPA_CLR_BIT        0
#define MFTA_TIMER0COMPB_CLR_BIT        1
#define MFTA_TIMER0UDF_CLR_BIT          2
#define MFTA_TIMER0OVF_CLR_BIT          3
#define MFTA_TIMER0CAPTA_CLR_BIT        4
#define MFTA_TIMER0CAPTB_CLR_BIT        5

#define MFTA_TIMER1COMPA_CLR_BIT        8
#define MFTA_TIMER1COMPB_CLR_BIT        9
#define MFTA_TIMER1UDF_CLR_BIT          10
#define MFTA_TIMER1OVF_CLR_BIT          11
#define MFTA_TIMER1CAPTA_CLR_BIT        12
#define MFTA_TIMER1CAPTB_CLR_BIT        13

#define MFTA_TIMER2COMPA_CLR_BIT        16
#define MFTA_TIMER2COMPB_CLR_BIT        17
#define MFTA_TIMER2UDF_CLR_BIT          18
#define MFTA_TIMER2OVF_CLR_BIT          19
#define MFTA_TIMER2CAPTA_CLR_BIT        20
#define MFTA_TIMER2CAPTB_CLR_BIT        21

#define MFTA_TIMER3COMPA_CLR_BIT        24
#define MFTA_TIMER3COMPB_CLR_BIT        25
#define MFTA_TIMER3UDF_CLR_BIT          26
#define MFTA_TIMER3OVF_CLR_BIT          27
#define MFTA_TIMER3CAPTA_CLR_BIT        28
#define MFTA_TIMER3CAPTB_CLR_BIT        29

/*
 * MFTA Timer0 Value Register definition
 */
#define MFTA_TIMER0_VALUE_REG           (MFTA_BASE + 0x20)
#define MFTA_TIMER0_VALUE_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER0_VALUE_RESET_VALUE   0x00000000

/*
 * MFTA Timer0 CaptureA Register definition
 */
#define MFTA_TIMER0_CAPTUREA_REG        (MFTA_BASE + 0x24)
#define MFTA_TIMER0_CAPTUREA_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER0_CAPTUREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer0 CaptureB Register definition
 */
#define MFTA_TIMER0_CAPTUREB_REG        (MFTA_BASE + 0x28)
#define MFTA_TIMER0_CAPTUREB_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER0_CAPTUREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer0 CompareA Register definition
 */
#define MFTA_TIMER0_COMPAREA_REG        (MFTA_BASE + 0x2c)
#define MFTA_TIMER0_COMPAREA_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER0_COMPAREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer0 CompareB Register definition
 */
#define MFTA_TIMER0_COMPAREB_REG        (MFTA_BASE + 0x30)
#define MFTA_TIMER0_COMPAREB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER0_COMPAREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer1 Value Register definition
 */
#define MFTA_TIMER1_VALUE_REG           (MFTA_BASE + 0x40)
#define MFTA_TIMER1_VALUE_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER1_VALUE_RESET_VALUE   0x00000000

/*
 * MFTA Timer1 CaptureA Register definition
 */
#define MFTA_TIMER1_CAPTUREA_REG        (MFTA_BASE + 0x44)
#define MFTA_TIMER1_CAPTUREA_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER1_CAPTUREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer1 CaptureB Register definition
 */
#define MFTA_TIMER1_CAPTUREB_REG        (MFTA_BASE + 0x48)
#define MFTA_TIMER1_CAPTUREB_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER1_CAPTUREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer1 CompareA Register definition
 */
#define MFTA_TIMER1_COMPAREA_REG        (MFTA_BASE + 0x4c)
#define MFTA_TIMER1_COMPAREA_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER1_COMPAREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer1 CompareB Register definition
 */
#define MFTA_TIMER1_COMPAREB_REG        (MFTA_BASE + 0x50)
#define MFTA_TIMER1_COMPAREB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER1_COMPAREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer2 Value Register definition
 */
#define MFTA_TIMER2_VALUE_REG           (MFTA_BASE + 0x60)
#define MFTA_TIMER2_VALUE_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER2_VALUE_RESET_VALUE   0x00000000

/*
 * MFTA Timer2 CaptureA Register definition
 */
#define MFTA_TIMER2_CAPTUREA_REG        (MFTA_BASE + 0x64)
#define MFTA_TIMER2_CAPTUREA_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER2_CAPTUREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer2 CaptureB Register definition
 */
#define MFTA_TIMER2_CAPTUREB_REG        (MFTA_BASE + 0x68)
#define MFTA_TIMER2_CAPTUREB_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER2_CAPTUREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer2 CompareA Register definition
 */
#define MFTA_TIMER2_COMPAREA_REG        (MFTA_BASE + 0x6c)
#define MFTA_TIMER2_COMPAREA_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER2_COMPAREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer2 CompareB Register definition
 */
#define MFTA_TIMER2_COMPAREB_REG        (MFTA_BASE + 0x70)
#define MFTA_TIMER2_COMPAREB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER2_COMPAREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer3 Value Register definition
 */
#define MFTA_TIMER3_VALUE_REG           (MFTA_BASE + 0x80)
#define MFTA_TIMER3_VALUE_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER3_VALUE_RESET_VALUE   0x00000000

/*
 * MFTA Timer3 CaptureA Register definition
 */
#define MFTA_TIMER3_CAPTUREA_REG        (MFTA_BASE + 0x84)
#define MFTA_TIMER3_CAPTUREA_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER3_CAPTUREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer3 CaptureB Register definition
 */
#define MFTA_TIMER3_CAPTUREB_REG        (MFTA_BASE + 0x88)
#define MFTA_TIMER3_CAPTUREB_REG_ACCESS (READ_ACCESS)
#define MFTA_TIMER3_CAPTUREB_RESET_VALUE 0x00000000

/*
 * MFTA Timer3 CompareA Register definition
 */
#define MFTA_TIMER3_COMPAREA_REG        (MFTA_BASE + 0x8c)
#define MFTA_TIMER3_COMPAREA_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER3_COMPAREA_RESET_VALUE 0x00000000

/*
 * MFTA Timer3 CompareB Register definition
 */
#define MFTA_TIMER3_COMPAREB_REG        (MFTA_BASE + 0x90)
#define MFTA_TIMER3_COMPAREB_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define MFTA_TIMER3_COMPAREB_RESET_VALUE 0x00000000

/*
 **********************************************************
 *  CAN Registers Definitions
 **********************************************************
 */

/*
 * CAN Interrupt Status Register Definitions.
 */
#define CAN_INTERRUPT_STATUS_REG        (CAN_BASE + 0x00)
#define CAN_INTERRUPT_STATUS_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_INTERRUPT_STATUS_RESET_VALUE 0x00000000

#define CAN_ARB_LOSS_ISF_BIT            2
#define CAN_OVR_LOAD_ISF_BIT            3
#define CAN_BIT_ERR_ISF_BIT             4
#define CAN_STUFF_ERR_ISF_BIT           5
#define CAN_ACK_ERR_ISF_BIT             6
#define CAN_FORM_ERR_ISF_BIT            7
#define CAN_CRC_ERR_ISF_BIT             8
#define CAN_BUS_OFF_ISF_BIT             9
#define CAN_RX_MSG_LOSS_ISF_BIT         10
#define CAN_TX_MSG_ISF_BIT              11
#define CAN_RX_MSG_ISF_BIT              12

/*
 * CAN Interrupt Enable Register Definitions.
 */
#define CAN_INTERRUPT_ENABLE_REG        (CAN_BASE + 0x04)
#define CAN_INTERRUPT_ENABLE_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_INTERRUPT_ENABLE_RESET_VALUE 0x00000000

#define CAN_ARB_LOSS_IEN_BIT            2
#define CAN_OVR_LOAD_IEN_BIT            3
#define CAN_BIT_ERR_IEN_BIT             4
#define CAN_STUFF_ERR_IEN_BIT           5
#define CAN_ACK_ERR_IEN_BIT             6
#define CAN_FORM_ERR_IEN_BIT            7
#define CAN_CRC_ERR_IEN_BIT             8
#define CAN_BUS_OFF_IEN_BIT             9
#define CAN_RX_MSG_LOSS_IEN_BIT         10
#define CAN_TX_MSG_IEN_BIT              11
#define CAN_RX_MSG_IEN_BIT              12

/*
 * CAN Buffer Status Register Definitions.
 */
#define CAN_BUFFER_STATUS_REG           (CAN_BASE + 0x08)
#define CAN_BUFFER_STATUS_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_BUFFER_STATUS_RESET_VALUE   0x00000000

#define CAN_TX_PEND7_MTR_FIELD          16
#define   NBITS_CAN_TX_PEND7_MTR          8

#define CAN_RX_AV7_MTR_FIELD            0
#define   NBITS_CAN_RX_AV7_MTR            16

/*
 * CAN Error Status Register Definitions.
 */
#define CAN_ERROR_STATUS_REG            (CAN_BASE + 0x0c)
#define CAN_ERROR_STATUS_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_ERROR_STATUS_RESET_VALUE    0x00000000

#define CAN_RXGTE96_BIT                 19
#define CAN_TXGTE96_BIT                 18

#define CAN_ERROR_STAT_FIELD            16
#define   NBITS_CAN_ERROR_STAT            2

#define CAN_RX_ERR_CNT_FIELD            8
#define   NBITS_CAN_RX_ERR_CNT            8

#define CAN_TX_ERR_CNT_FIELD            0
#define   NBITS_CAN_TX_ERR_CNT            8

#define CAN_OPERATING_MODE_REG          (CAN_BASE + 0x10)
#define CAN_OPERATING_MODE_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_OPERATING_MODE_RESET_VALUE  0x00000000

#define CAN_LO_MODE_SEL_BIT             1
#define CAN_RUN_STOP_SEL_BIT            0

/*
 * CAN Configuration Register Definitions.
 */
#define CAN_CONFIG_REG                  (CAN_BASE + 0x14)
#define CAN_CONFIG_REG_ACCESS           (READ_ACCESS | WRITE_ACCESS)
#define CAN_CONFIG_RESET_VALUE          0x00000000

#define CAN_BITRATE_CFG_FIELD           16
#define   NBITS_CAN_BITRATE_CFG           15

#define CAN_ARB_CTRL_BIT                12

#define CAN_TSEG1_CTRL_FIELD            8
#define   NBITS_CAN_TSEG1_CTRL            4

#define CAN_TSEG2_CTRL_FIELD            5
#define   NBITS_CAN_TSEG2_CTRL            3

#define CAN_ARESTART_BIT                4

#define CAN_SYNC_JMP_WIDTH_FIELD        2
#define   NBITS_CAN_SYNC_JMP_WIDTH        2

#define CAN_SMODE_SEL_BIT               1
#define CAN_EMODE_SEL_BIT               0

/*
 * CAN TX Message Control Register Bit Definitions.
 * (CAN_TX_MESSAGEx_CTRL)
 */
#define CAN_TX_RID_WPN_BIT              23
#define CAN_TX_RTR_BIT                  21
#define CAN_TX_IDE_BIT                  20

#define CAN_TX_DLC_FIELD                16
#define   NBITS_CAN_TX_DLC                4

#define CAN_TX_IEN_WPN_BIT              3
#define CAN_TX_IE_BIT                   2
#define CAN_TX_TAR_BIT                  1
#define CAN_TX_REQ_BIT                  0

/*
 * CAN TX Message ID Register Bit Definitions.
 * (CAN_TX_MESSAGEx_ID)
 */
#define CAN_TX_MESSAGE_ID_FIELD         3
#define   NBITS_CAN_TX_MESSAGE_ID         29

/*
 * CAN RX Message Control Register Bit Definitions.
 * (CAN_RX_MESSAGEx_CTRL)
 */
#define CAN_RX_RID_WPN_BIT              23
#define CAN_RX_RTR_BIT                  21
#define CAN_RX_IDE_BIT                  20

#define CAN_RX_DLC_FIELD                16
#define   NBITS_CAN_RX_DLC                4

#define CAN_RX_LIRB_WPN_BIT             7
#define CAN_RX_LINKFLAG_BIT             6
#define CAN_RX_IE_BIT                   5
#define CAN_RX_RTR_AUTO_BIT             4
#define CAN_RX_ENABLE_BIT               3
#define CAN_RX_RAR_BIT                  2
#define CAN_RX_RTR_PEND_BIT             1
#define CAN_RX_MSGAV_BIT                0

/*
 * CAN RX Message ID Register Bit Definitions.
 * (CAN_RX_MESSAGEx_ID)
 */
#define CAN_RX_MESSAGE_ID_FIELD         3
#define   NBITS_CAN_RX_MESSAGE_ID         29

/*
 * CAN RX Message Arbitration Acceptance Mask Register Bit Definitions.
 * (CAN_RX_ARB_AMRx)
 * and
 * CAN RX Message Arbitration Acceptance Code Register Bit Definitions.
 * (CAN_RX_ARB_ACRx)
 */
#define CAN_RX_ID_MASK_FIELD            3
#define   NBITS_CAN_RX_ID_MASK            29

#define CAN_RX_IDE_MASK_BIT             2
#define CAN_RX_RTR_MASK_BIT             1

/*
 * CAN RX Message Data Accpetance Mask Register Bit Definitions.
 * (CAN_RX_DATA_ARMx)
 * and
 * CAN RX Message Data Accpetance Code Register Bit Definitions.
 * (CAN_RX_DATA_ACRx)
 */
#define CAN_RX_DATA_MASK_FIELD          0
#define   NBITS_CAN_RX_DATA_MASK          16


/*
 * CAN TX Message 0
 */
#define CAN_TX_MESSAGE0_CTRL_REG        (CAN_BASE + 0x20)
#define CAN_TX_MESSAGE0_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE0_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE0_ID_REG          (CAN_BASE + 0x24)
#define CAN_TX_MESSAGE0_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE0_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE0_DATAH_REG       (CAN_BASE + 0x28)
#define CAN_TX_MESSAGE0_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE0_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE0_DATAL_REG       (CAN_BASE + 0x2c)
#define CAN_TX_MESSAGE0_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE0_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 1
 */
#define CAN_TX_MESSAGE1_CTRL_REG        (CAN_BASE + 0x30)
#define CAN_TX_MESSAGE1_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE1_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE1_ID_REG          (CAN_BASE + 0x34)
#define CAN_TX_MESSAGE1_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE1_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE1_DATAH_REG       (CAN_BASE + 0x38)
#define CAN_TX_MESSAGE1_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE1_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE1_DATAL_REG       (CAN_BASE + 0x3c)
#define CAN_TX_MESSAGE1_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE1_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 2
 */
#define CAN_TX_MESSAGE2_CTRL_REG        (CAN_BASE + 0x40)
#define CAN_TX_MESSAGE2_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE2_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE2_ID_REG          (CAN_BASE + 0x44)
#define CAN_TX_MESSAGE2_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE2_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE2_DATAH_REG       (CAN_BASE + 0x48)
#define CAN_TX_MESSAGE2_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE2_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE2_DATAL_REG       (CAN_BASE + 0x4c)
#define CAN_TX_MESSAGE2_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE2_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 3
 */
#define CAN_TX_MESSAGE3_CTRL_REG        (CAN_BASE + 0x50)
#define CAN_TX_MESSAGE3_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE3_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE3_ID_REG          (CAN_BASE + 0x54)
#define CAN_TX_MESSAGE3_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE3_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE3_DATAH_REG       (CAN_BASE + 0x58)
#define CAN_TX_MESSAGE3_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE3_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE3_DATAL_REG       (CAN_BASE + 0x5c)
#define CAN_TX_MESSAGE3_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE3_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 4
 */
#define CAN_TX_MESSAGE4_CTRL_REG        (CAN_BASE + 0x60)
#define CAN_TX_MESSAGE4_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE4_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE4_ID_REG          (CAN_BASE + 0x64)
#define CAN_TX_MESSAGE4_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE4_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE4_DATAH_REG       (CAN_BASE + 0x68)
#define CAN_TX_MESSAGE4_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE4_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE4_DATAL_REG       (CAN_BASE + 0x6c)
#define CAN_TX_MESSAGE4_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE4_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 5
 */
#define CAN_TX_MESSAGE5_CTRL_REG        (CAN_BASE + 0x70)
#define CAN_TX_MESSAGE5_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE5_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE5_ID_REG          (CAN_BASE + 0x74)
#define CAN_TX_MESSAGE5_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE5_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE5_DATAH_REG       (CAN_BASE + 0x78)
#define CAN_TX_MESSAGE5_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE5_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE5_DATAL_REG       (CAN_BASE + 0x7c)
#define CAN_TX_MESSAGE5_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE5_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 6
 */
#define CAN_TX_MESSAGE6_CTRL_REG        (CAN_BASE + 0x80)
#define CAN_TX_MESSAGE6_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE6_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE6_ID_REG          (CAN_BASE + 0x84)
#define CAN_TX_MESSAGE6_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE6_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE6_DATAH_REG       (CAN_BASE + 0x88)
#define CAN_TX_MESSAGE6_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE6_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE6_DATAL_REG       (CAN_BASE + 0x8c)
#define CAN_TX_MESSAGE6_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE6_DATAL_RESET_VALUE 0x00000000

/*
 * CAN TX Message 7
 */
#define CAN_TX_MESSAGE7_CTRL_REG        (CAN_BASE + 0x90)
#define CAN_TX_MESSAGE7_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE7_CTRL_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE7_ID_REG          (CAN_BASE + 0x94)
#define CAN_TX_MESSAGE7_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE7_ID_RESET_VALUE  0x00000000

#define CAN_TX_MESSAGE7_DATAH_REG       (CAN_BASE + 0x98)
#define CAN_TX_MESSAGE7_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE7_DATAH_RESET_VALUE 0x00000000

#define CAN_TX_MESSAGE7_DATAL_REG       (CAN_BASE + 0x9c)
#define CAN_TX_MESSAGE7_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_TX_MESSAGE7_DATAL_RESET_VALUE 0x00000000

/*
 * CAN RX Message 0
 */
#define CAN_RX_MESSAGE0_CTRL_REG        (CAN_BASE + 0xa0)
#define CAN_RX_MESSAGE0_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE0_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE0_ID_REG          (CAN_BASE + 0xa4)
#define CAN_RX_MESSAGE0_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE0_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE0_DATAH_REG       (CAN_BASE + 0xa8)
#define CAN_RX_MESSAGE0_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE0_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE0_DATAL_REG       (CAN_BASE + 0xac)
#define CAN_RX_MESSAGE0_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE0_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR0_REG             (CAN_BASE + 0xb0)
#define CAN_RX_ARB_AMR0_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR0_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR0_REG             (CAN_BASE + 0xb4)
#define CAN_RX_ARB_ACR0_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR0_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR0_REG            (CAN_BASE + 0xb8)
#define CAN_RX_DATA_AMR0_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR0_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR0_REG            (CAN_BASE + 0xbc)
#define CAN_RX_DATA_ACR0_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR0_RESET_VALUE    0x00000000

/*
 * CAN RX Message 1
 */
#define CAN_RX_MESSAGE1_CTRL_REG        (CAN_BASE + 0xc0)
#define CAN_RX_MESSAGE1_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE1_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE1_ID_REG          (CAN_BASE + 0xc4)
#define CAN_RX_MESSAGE1_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE1_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE1_DATAH_REG       (CAN_BASE + 0xc8)
#define CAN_RX_MESSAGE1_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE1_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE1_DATAL_REG       (CAN_BASE + 0xcc)
#define CAN_RX_MESSAGE1_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE1_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR1_REG             (CAN_BASE + 0xd0)
#define CAN_RX_ARB_AMR1_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR1_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR1_REG             (CAN_BASE + 0xd4)
#define CAN_RX_ARB_ACR1_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR1_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR1_REG            (CAN_BASE + 0xd8)
#define CAN_RX_DATA_AMR1_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR1_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR1_REG            (CAN_BASE + 0xdc)
#define CAN_RX_DATA_ACR1_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR1_RESET_VALUE    0x00000000

/*
 * CAN RX Message 2
 */
#define CAN_RX_MESSAGE2_CTRL_REG        (CAN_BASE + 0xe0)
#define CAN_RX_MESSAGE2_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE2_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE2_ID_REG          (CAN_BASE + 0xe4)
#define CAN_RX_MESSAGE2_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE2_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE2_DATAH_REG       (CAN_BASE + 0xe8)
#define CAN_RX_MESSAGE2_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE2_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE2_DATAL_REG       (CAN_BASE + 0xec)
#define CAN_RX_MESSAGE2_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE2_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR2_REG             (CAN_BASE + 0xf0)
#define CAN_RX_ARB_AMR2_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR2_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR2_REG             (CAN_BASE + 0xf4)
#define CAN_RX_ARB_ACR2_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR2_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR2_REG            (CAN_BASE + 0xf8)
#define CAN_RX_DATA_AMR2_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR2_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR2_REG            (CAN_BASE + 0xfc)
#define CAN_RX_DATA_ACR2_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR2_RESET_VALUE    0x00000000

/*
 * CAN RX Message 3
 */
#define CAN_RX_MESSAGE3_CTRL_REG        (CAN_BASE + 0x100)
#define CAN_RX_MESSAGE3_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE3_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE3_ID_REG          (CAN_BASE + 0x104)
#define CAN_RX_MESSAGE3_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE3_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE3_DATAH_REG       (CAN_BASE + 0x108)
#define CAN_RX_MESSAGE3_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE3_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE3_DATAL_REG       (CAN_BASE + 0x10c)
#define CAN_RX_MESSAGE3_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE3_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR3_REG             (CAN_BASE + 0x110)
#define CAN_RX_ARB_AMR3_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR3_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR3_REG             (CAN_BASE + 0x114)
#define CAN_RX_ARB_ACR3_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR3_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR3_REG            (CAN_BASE + 0x118)
#define CAN_RX_DATA_AMR3_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR3_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR3_REG            (CAN_BASE + 0x11c)
#define CAN_RX_DATA_ACR3_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR3_RESET_VALUE    0x00000000

/*
 * CAN RX Message 4
 */
#define CAN_RX_MESSAGE4_CTRL_REG        (CAN_BASE + 0x120)
#define CAN_RX_MESSAGE4_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE4_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE4_ID_REG          (CAN_BASE + 0x124)
#define CAN_RX_MESSAGE4_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE4_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE4_DATAH_REG       (CAN_BASE + 0x128)
#define CAN_RX_MESSAGE4_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE4_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE4_DATAL_REG       (CAN_BASE + 0x12c)
#define CAN_RX_MESSAGE4_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE4_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR4_REG             (CAN_BASE + 0x130)
#define CAN_RX_ARB_AMR4_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR4_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR4_REG             (CAN_BASE + 0x134)
#define CAN_RX_ARB_ACR4_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR4_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR4_REG            (CAN_BASE + 0x138)
#define CAN_RX_DATA_AMR4_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR4_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR4_REG            (CAN_BASE + 0x13c)
#define CAN_RX_DATA_ACR4_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR4_RESET_VALUE    0x00000000

/*
 * CAN RX Message 5
 */
#define CAN_RX_MESSAGE5_CTRL_REG        (CAN_BASE + 0x140)
#define CAN_RX_MESSAGE5_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE5_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE5_ID_REG          (CAN_BASE + 0x144)
#define CAN_RX_MESSAGE5_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE5_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE5_DATAH_REG       (CAN_BASE + 0x148)
#define CAN_RX_MESSAGE5_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE5_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE5_DATAL_REG       (CAN_BASE + 0x14c)
#define CAN_RX_MESSAGE5_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE5_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR5_REG             (CAN_BASE + 0x150)
#define CAN_RX_ARB_AMR5_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR5_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR5_REG             (CAN_BASE + 0x154)
#define CAN_RX_ARB_ACR5_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR5_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR5_REG            (CAN_BASE + 0x158)
#define CAN_RX_DATA_AMR5_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR5_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR5_REG            (CAN_BASE + 0x15c)
#define CAN_RX_DATA_ACR5_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR5_RESET_VALUE    0x00000000

/*
 * CAN RX Message 6
 */
#define CAN_RX_MESSAGE6_CTRL_REG        (CAN_BASE + 0x160)
#define CAN_RX_MESSAGE6_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE6_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE6_ID_REG          (CAN_BASE + 0x164)
#define CAN_RX_MESSAGE6_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE6_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE6_DATAH_REG       (CAN_BASE + 0x168)
#define CAN_RX_MESSAGE6_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE6_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE6_DATAL_REG       (CAN_BASE + 0x16c)
#define CAN_RX_MESSAGE6_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE6_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR6_REG             (CAN_BASE + 0x170)
#define CAN_RX_ARB_AMR6_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR6_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR6_REG             (CAN_BASE + 0x174)
#define CAN_RX_ARB_ACR6_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR6_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR6_REG            (CAN_BASE + 0x178)
#define CAN_RX_DATA_AMR6_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR6_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR6_REG            (CAN_BASE + 0x17c)
#define CAN_RX_DATA_ACR6_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR6_RESET_VALUE    0x00000000

/*
 * CAN RX Message 7
 */
#define CAN_RX_MESSAGE7_CTRL_REG        (CAN_BASE + 0x180)
#define CAN_RX_MESSAGE7_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE7_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE7_ID_REG          (CAN_BASE + 0x184)
#define CAN_RX_MESSAGE7_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE7_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE7_DATAH_REG       (CAN_BASE + 0x188)
#define CAN_RX_MESSAGE7_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE7_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE7_DATAL_REG       (CAN_BASE + 0x18c)
#define CAN_RX_MESSAGE7_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE7_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR7_REG             (CAN_BASE + 0x190)
#define CAN_RX_ARB_AMR7_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR7_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR7_REG             (CAN_BASE + 0x194)
#define CAN_RX_ARB_ACR7_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR7_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR7_REG            (CAN_BASE + 0x198)
#define CAN_RX_DATA_AMR7_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR7_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR7_REG            (CAN_BASE + 0x19c)
#define CAN_RX_DATA_ACR7_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR7_RESET_VALUE    0x00000000

/*
 * CAN RX Message 8
 */
#define CAN_RX_MESSAGE8_CTRL_REG        (CAN_BASE + 0x1a0)
#define CAN_RX_MESSAGE8_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE8_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE8_ID_REG          (CAN_BASE + 0x1a4)
#define CAN_RX_MESSAGE8_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE8_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE8_DATAH_REG       (CAN_BASE + 0x1a8)
#define CAN_RX_MESSAGE8_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE8_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE8_DATAL_REG       (CAN_BASE + 0x1ac)
#define CAN_RX_MESSAGE8_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE8_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR8_REG             (CAN_BASE + 0x1b0)
#define CAN_RX_ARB_AMR8_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR8_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR8_REG             (CAN_BASE + 0x1b4)
#define CAN_RX_ARB_ACR8_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR8_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR8_REG            (CAN_BASE + 0x1b8)
#define CAN_RX_DATA_AMR8_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR8_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR8_REG            (CAN_BASE + 0x1bc)
#define CAN_RX_DATA_ACR8_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR8_RESET_VALUE    0x00000000

/*
 * CAN RX Message 9
 */
#define CAN_RX_MESSAGE9_CTRL_REG        (CAN_BASE + 0x1c0)
#define CAN_RX_MESSAGE9_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE9_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE9_ID_REG          (CAN_BASE + 0x1c4)
#define CAN_RX_MESSAGE9_ID_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE9_ID_RESET_VALUE  0x00000000

#define CAN_RX_MESSAGE9_DATAH_REG       (CAN_BASE + 0x1c8)
#define CAN_RX_MESSAGE9_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE9_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE9_DATAL_REG       (CAN_BASE + 0x1cc)
#define CAN_RX_MESSAGE9_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE9_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR9_REG             (CAN_BASE + 0x1d0)
#define CAN_RX_ARB_AMR9_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR9_RESET_VALUE     0x00000000

#define CAN_RX_ARB_ACR9_REG             (CAN_BASE + 0x1d4)
#define CAN_RX_ARB_ACR9_REG_ACCESS      (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR9_RESET_VALUE     0x00000000

#define CAN_RX_DATA_AMR9_REG            (CAN_BASE + 0x1d8)
#define CAN_RX_DATA_AMR9_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR9_RESET_VALUE    0x00000000

#define CAN_RX_DATA_ACR9_REG            (CAN_BASE + 0x1dc)
#define CAN_RX_DATA_ACR9_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR9_RESET_VALUE    0x00000000

/*
 * CAN RX Message 10
 */
#define CAN_RX_MESSAGE10_CTRL_REG       (CAN_BASE + 0x1e0)
#define CAN_RX_MESSAGE10_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE10_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE10_ID_REG         (CAN_BASE + 0x1e4)
#define CAN_RX_MESSAGE10_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE10_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE10_DATAH_REG      (CAN_BASE + 0x1e8)
#define CAN_RX_MESSAGE10_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE10_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE10_DATAL_REG      (CAN_BASE + 0x1ec)
#define CAN_RX_MESSAGE10_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE10_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR10_REG            (CAN_BASE + 0x1f0)
#define CAN_RX_ARB_AMR10_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR10_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR10_REG            (CAN_BASE + 0x1f4)
#define CAN_RX_ARB_ACR10_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR10_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR10_REG           (CAN_BASE + 0x1f8)
#define CAN_RX_DATA_AMR10_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR10_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR10_REG           (CAN_BASE + 0x1fc)
#define CAN_RX_DATA_ACR10_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR10_RESET_VALUE   0x00000000

/*
 * CAN RX Message 11
 */
#define CAN_RX_MESSAGE11_CTRL_REG       (CAN_BASE + 0x200)
#define CAN_RX_MESSAGE11_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE11_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE11_ID_REG         (CAN_BASE + 0x204)
#define CAN_RX_MESSAGE11_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE11_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE11_DATAH_REG      (CAN_BASE + 0x208)
#define CAN_RX_MESSAGE11_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE11_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE11_DATAL_REG      (CAN_BASE + 0x20c)
#define CAN_RX_MESSAGE11_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE11_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR11_REG            (CAN_BASE + 0x210)
#define CAN_RX_ARB_AMR11_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR11_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR11_REG            (CAN_BASE + 0x214)
#define CAN_RX_ARB_ACR11_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR11_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR11_REG           (CAN_BASE + 0x218)
#define CAN_RX_DATA_AMR11_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR11_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR11_REG           (CAN_BASE + 0x21c)
#define CAN_RX_DATA_ACR11_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR11_RESET_VALUE   0x00000000

/*
 * CAN RX Message 12
 */
#define CAN_RX_MESSAGE12_CTRL_REG       (CAN_BASE + 0x220)
#define CAN_RX_MESSAGE12_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE12_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE12_ID_REG         (CAN_BASE + 0x224)
#define CAN_RX_MESSAGE12_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE12_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE12_DATAH_REG      (CAN_BASE + 0x228)
#define CAN_RX_MESSAGE12_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE12_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE12_DATAL_REG      (CAN_BASE + 0x23c)
#define CAN_RX_MESSAGE12_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE12_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR12_REG            (CAN_BASE + 0x230)
#define CAN_RX_ARB_AMR12_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR12_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR12_REG            (CAN_BASE + 0x234)
#define CAN_RX_ARB_ACR12_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR12_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR12_REG           (CAN_BASE + 0x238)
#define CAN_RX_DATA_AMR12_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR12_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR12_REG           (CAN_BASE + 0x23c)
#define CAN_RX_DATA_ACR12_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR12_RESET_VALUE   0x00000000

/*
 * CAN RX Message 13
 */
#define CAN_RX_MESSAGE13_CTRL_REG       (CAN_BASE + 0x240)
#define CAN_RX_MESSAGE13_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE13_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE13_ID_REG         (CAN_BASE + 0x244)
#define CAN_RX_MESSAGE13_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE13_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE13_DATAH_REG      (CAN_BASE + 0x248)
#define CAN_RX_MESSAGE13_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE13_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE13_DATAL_REG      (CAN_BASE + 0x24c)
#define CAN_RX_MESSAGE13_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE13_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR13_REG            (CAN_BASE + 0x250)
#define CAN_RX_ARB_AMR13_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR13_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR13_REG            (CAN_BASE + 0x254)
#define CAN_RX_ARB_ACR13_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR13_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR13_REG           (CAN_BASE + 0x258)
#define CAN_RX_DATA_AMR13_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR13_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR13_REG           (CAN_BASE + 0x25c)
#define CAN_RX_DATA_ACR13_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR13_RESET_VALUE   0x00000000

/*
 * CAN RX Message 14
 */
#define CAN_RX_MESSAGE14_CTRL_REG       (CAN_BASE + 0x260)
#define CAN_RX_MESSAGE14_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE14_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE14_ID_REG         (CAN_BASE + 0x264)
#define CAN_RX_MESSAGE14_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE14_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE14_DATAH_REG       (CAN_BASE + 0x268)
#define CAN_RX_MESSAGE14_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE14_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE14_DATAL_REG      (CAN_BASE + 0x26c)
#define CAN_RX_MESSAGE14_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE14_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR14_REG            (CAN_BASE + 0x270)
#define CAN_RX_ARB_AMR14_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR14_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR14_REG            (CAN_BASE + 0x274)
#define CAN_RX_ARB_ACR14_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR14_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR14_REG           (CAN_BASE + 0x278)
#define CAN_RX_DATA_AMR14_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR14_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR14_REG           (CAN_BASE + 0x27c)
#define CAN_RX_DATA_ACR14_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR14_RESET_VALUE   0x00000000

/*
 * CAN RX Message 15
 */
#define CAN_RX_MESSAGE15_CTRL_REG       (CAN_BASE + 0x280)
#define CAN_RX_MESSAGE15_CTRL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE15_CTRL_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE15_ID_REG         (CAN_BASE + 0x284)
#define CAN_RX_MESSAGE15_ID_REG_ACCESS  (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE15_ID_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE15_DATAH_REG      (CAN_BASE + 0x288)
#define CAN_RX_MESSAGE15_DATAH_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE15_DATAH_RESET_VALUE 0x00000000

#define CAN_RX_MESSAGE15_DATAL_REG      (CAN_BASE + 0x28c)
#define CAN_RX_MESSAGE15_DATAL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_MESSAGE15_DATAL_RESET_VALUE 0x00000000

#define CAN_RX_ARB_AMR15_REG            (CAN_BASE + 0x290)
#define CAN_RX_ARB_AMR15_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_AMR15_RESET_VALUE    0x00000000

#define CAN_RX_ARB_ACR15_REG            (CAN_BASE + 0x294)
#define CAN_RX_ARB_ACR15_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_ARB_ACR15_RESET_VALUE    0x00000000

#define CAN_RX_DATA_AMR15_REG           (CAN_BASE + 0x298)
#define CAN_RX_DATA_AMR15_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_AMR15_RESET_VALUE   0x00000000

#define CAN_RX_DATA_ACR15_REG           (CAN_BASE + 0x29c)
#define CAN_RX_DATA_ACR15_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define CAN_RX_DATA_ACR15_RESET_VALUE   0x00000000

/*
 **********************************************************
 * ADC Interface Definition
 **********************************************************
 */
/*
 * ADC Control Register Definitions.
 */
#define ADC_CONTROL_REG                 (ADC_BASE + 0x00)
#define ADC_CONTROL_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define ADC_CONTROL_RESET_VALUE         0x00000000

/*
 * ADC Control Register Bit Definitions.
 */
#define ADC_WAIT_FIELD                  24
#define   NBITS_ADC_WAIT_FIELD            8

#define ADC_WATER_MARK_FIELD            16
#define   NBITS_ADC_WATER_MARK            5

#define ADC_PRESCALER_FIELD             8
#define   NBITS_ADC_PRESCALER             5

#define ADC_INT_REF_BIT                 3
#define ADC_POWER_DOWN_BIT              2
#define ADC_RESET_BIT                   0

/*
 * ADC Scan Sequencer Control Register Definitions.
 */
#define ADC_SCAN_SEQ_CONTROL_REG        (ADC_BASE + 0x04)
#define ADC_SCAN_SEQ_CONTROL_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define ADC_SCAN_SEQ_CONTROL_RESET_VALUE 0x00000000

/*
 * ADC Scan Sequencer Control Register Bit Definitions.
 */
#define ADC_SCAN_SEQ_ADDR_PTR_FIELD     8
#define   NBITS_ADC_SCAN_SEQ_ADDR_PTR     5

#define ADC_SCAN_SEQ_RUN_BIT            1
#define ADC_SCAN_SEQ_RST_BIT            0

/*
 * ADC Interrupt Enable Register Definitions.
 */
#define ADC_INTERRUPT_ENABLE_REG        (ADC_BASE + 0x10)
#define ADC_INTERRUPT_ENABLE_REG_ACCESS (READ_ACCESS | WRITE_ACCESS)
#define ADC_INTERRUPT_ENABLE_RESET_VALUE 0x00000000

/*
 * ADC Interrupt Enable Register Bit Definitions.
 */
#define ADC_SAMPLE_IRQ_IE_BIT           7
#define ADC_OVERWATER_IE_BIT            6
#define ADC_OVF_IE_BIT                  5
#define ADC_UDF_IE_BIT                  4
#define ADC_FULL_IE_BIT                 3
#define ADC_NOT_EMPTY_IE_BIT            2
#define ADC_STOPPED_IE_BIT              1
#define ADC_CONV_COMP_IE_BIT            0

/*
 * ADC Interrupt Status Register Definitions.
 */
#define ADC_INTERRUPT_STATUS_REG        (ADC_BASE + 0x14)
#define ADC_INTERRUPT_STATUS_REG_ACCESS (READ_ACCESS)
#define ADC_INTERRUPT_STATUS_RESET_VALUE 0x00000000

/*
 * ADC Interrupt Status Register Bit Definitions.
 */
#define ADC_SAMPLE_IRQ_ISF_BIT          7
#define ADC_OVERWATER_ISF_BIT           6
#define ADC_OVF_ISF_BIT                 5
#define ADC_UDF_ISF_BIT                 4
#define ADC_FULL_ISF_BIT                3
#define ADC_NOT_EMPTY_ISF_BIT           2
#define ADC_STOPPED_ISF_BIT             1
#define ADC_CONV_COMP_ISF_BIT           0

/*
 * ADC Interrupt Clear Register Definitions.
 */
#define ADC_INTERRUPT_CLEAR_REG         (ADC_BASE + 0x18)
#define ADC_INTERRUPT_CLEAR_REG_ACCESS  (WRITE_ACCESS)
#define ADC_INTERRUPT_CLEAR_RESET_VALUE 0x00000000

#define ADC_SAMPLE_IRQ_CLR_REG          7
#define ADC_OVERWATER_CRL_REG           6
#define ADC_OVF_CLR_REG                 5
#define ADC_UDF_CLR_REG                 4
#define ADC_FULL_CLR_REG                3
#define ADC_NOT_EMPTY_CLR_REG           2
#define ADC_STOPPED_CLR_REG             1
#define ADC_CONV_COMP_CLR_REG           0

/*
 * ADC Result FIFO Read Register Definitions.
 */
#define ADC_FIFO_READ_REG               (ADC_BASE + 0x20)
#define ADC_FIFO_READ_REG_ACCESS        (READ_ACCESS)
#define ADC_FIFO_READ_RESET_VALUE       0x00000000

/*
 * ADC Result FIFO Read Register Bit Definitions.
 */
#define ADC_RESULT_INVALID_BIT          25
#define ADC_RESULT_SAMPLE_IRQ_BIT       24

#define ADC_RESULT_CH_FIELD             16
#define   NBITS_ADC_RESULT_CH             4

#define ADC_RESULT_FIELD                0
#define  NBITS_ADC_RESULT                 10

/*
 * ADC Result FIFO Status Register Definitions.
 */
#define ADC_FIFO_STATUS_REG             (ADC_BASE + 0x24)
#define ADC_FIFO_STATUS_REG_ACCESS      (READ_ACCESS)
#define ADC_FIFO_STATUS_RESET_VALUE     0x00000000

/*
 * ADC Result FIFO Status Register Bit Definitions.
 */
#define ADC_RESULT_USED_FIELD           8
#define   NBITS_ADC_RESULT_USED           7

#define ADC_RESULT_OVERWATER_BIT        6
#define ADC_RESULT_FULL_BIT             3
#define ADC_RESULT_EMPTY_BIT            2

/*
 * ADC Scan Command Register Definitions.
 */
#define ADC_CMNDx_RESET_VALUE           0x00000000
#define ADC_CMNDx_REG_ACCESS            (READ_ACCESS | WRITE_ACCESS)

#define ADC_CMND0_REG                   (ADC_BASE + 0x80)
#define ADC_CMND1_REG                   (ADC_BASE + 0x84)
#define ADC_CMND2_REG                   (ADC_BASE + 0x88)
#define ADC_CMND3_REG                   (ADC_BASE + 0x8c)
#define ADC_CMND4_REG                   (ADC_BASE + 0x90)
#define ADC_CMND5_REG                   (ADC_BASE + 0x94)
#define ADC_CMND6_REG                   (ADC_BASE + 0x98)
#define ADC_CMND7_REG                   (ADC_BASE + 0x9c)
#define ADC_CMND8_REG                   (ADC_BASE + 0xa0)
#define ADC_CMND9_REG                   (ADC_BASE + 0xa4)
#define ADC_CMND10_REG                  (ADC_BASE + 0xa8)
#define ADC_CMND11_REG                  (ADC_BASE + 0xac)
#define ADC_CMND12_REG                  (ADC_BASE + 0xb0)
#define ADC_CMND13_REG                  (ADC_BASE + 0xb4)
#define ADC_CMND14_REG                  (ADC_BASE + 0xb8)
#define ADC_CMND15_REG                  (ADC_BASE + 0xbc)
#define ADC_CMND16_REG                  (ADC_BASE + 0xc0)
#define ADC_CMND17_REG                  (ADC_BASE + 0xc4)
#define ADC_CMND18_REG                  (ADC_BASE + 0xc8)
#define ADC_CMND19_REG                  (ADC_BASE + 0xcc)
#define ADC_CMND20_REG                  (ADC_BASE + 0xd0)
#define ADC_CMND21_REG                  (ADC_BASE + 0xd4)
#define ADC_CMND22_REG                  (ADC_BASE + 0xd8)
#define ADC_CMND23_REG                  (ADC_BASE + 0xdc)
#define ADC_CMND24_REG                  (ADC_BASE + 0xe0)
#define ADC_CMND25_REG                  (ADC_BASE + 0xe4)
#define ADC_CMND26_REG                  (ADC_BASE + 0xe8)
#define ADC_CMND27_REG                  (ADC_BASE + 0xec)
#define ADC_CMND28_REG                  (ADC_BASE + 0xf0)
#define ADC_CMND29_REG                  (ADC_BASE + 0xf4)
#define ADC_CMND30_REG                  (ADC_BASE + 0xf8)
#define ADC_CMND31_REG                  (ADC_BASE + 0xfc)

/*
 * ADC Scan Command Register Bit Definitions.
 */
#define ADC_SCANSEQ_CMND_FIELD          4
#define   NBITS_ADC_SCANSEQ_CMND          3
#define ADC_SCANSEQ_PARAM_FIELD         0
#define   NBITS_ADC_SCANSEQ_PARAM         4

/*
 **********************************************************
 * USB Interface Definition
 **********************************************************
 */

/*
 * USB Function Address Register Definitions.
 */
#define USB_FUNCTION_ADDR_REG           (USB_BASE + 0x00)
#define USB_FUNCTION_ADDR_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define USB_FUNCTION_ADDR_RESET_VALUE   0x00000000

/*
 * USB Function Address Register Bit Definitions.
 */
#define USB_FUNCTION_ADDR_FIELD         0
#define   NBITS_USB_FUNCTION_ADDR         6

#define USB_FUNCTION_ADDR_UPDATED_BIT   7

/*
 * USB Power Control Register Definitions.
 */
#define USB_POWER_CTRL_REG              (USB_BASE + 0x01)
#define USB_POWER_CTRL_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define USB_POWER_CTRL_RESET_VALUE      0x00000000

/*
 * USB Power Control Register Bit Definitions.
 */
#define USB_SUSPEND_ENA_BIT             0
#define USB_SUSPEND_MODE_BIT            1
#define USB_RESUME_BIT                  2
#define USB_RESET_ACTIVE_BIT            3
#define USB_ISO_UPDATE_BIT              7

/*
 * USB Interrupt IN1 Status Register Definitions.
 */
#define USB_INT_IN_STATUS_REG           (USB_BASE + 0x02)
#define USB_INT_IN_STATUS_REG_ACCESS    (READ_ACCESS)
#define USB_INT_IN_STATUS_RESET_VALUE   0x00000000

/*
 * USB Interrupt IN1 Status Register Bit Definitions.
 */
#define USB_ENDPOINT0_IN_ISF_BIT        0
#define USB_ENDPOINT1_IN_ISF_BIT        1
#define USB_ENDPOINT2_IN_ISF_BIT        2
#define USB_ENDPOINT3_IN_ISF_BIT        3
#define USB_ENDPOINT4_IN_ISF_BIT        4

/*
 * USB Interrupt OUT1 Status Register Definitions.
 */
#define USB_INT_OUT_STATUS_REG          (USB_BASE + 0x04)
#define USB_INT_OUT_STATUS_REG_ACCESS   (READ_ACCESS)
#define USB_INT_OUT_STATUS_RESET_VALUE  0x00000000

/*
 * USB Interrupt OUT1 Status Register Bit Definitions.
 */
#define USB_ENDPOINT1_OUT_ISF_BIT       1
#define USB_ENDPOINT2_OUT_ISF_BIT       2
#define USB_ENDPOINT3_OUT_ISF_BIT       3
#define USB_ENDPOINT4_OUT_ISF_BIT       4

/*
 * USB Interrupt Status Register Definitions.
 */
#define USB_INT_STATUS_REG              (USB_BASE + 0x06)
#define USB_INT_STATUS_REG_ACCESS       (READ_ACCESS)
#define USB_INT_STATUS_RESET_VALUE      0x00000000

/*
 * USB Interrupt Status Register Bit Definitions.
 */
#define USB_SUSPEND_ISF_BIT             0
#define USB_RESUME_ISF_BIT              1
#define USB_RESET_ISF_BIT               2
#define USB_SOF_ISF_BIT                 3

/*
 * USB Interrupt IN1 Enable Register Definitions.
 */
#define USB_INT_IN_ENABLE_REG           (USB_BASE + 0x07)
#define USB_INT_IN_ENABLE_REG_ACCESS    (READ_ACCESS | WRITE_ACCESS)
#define USB_INT_IN_ENABLE_RESET_VALUE   0x00000000

/*
 * USB Interrupt IN1 Enable Register Bit Definitions.
 */
#define USB_ENDPOINT0_IN_IEN_BIT        0
#define USB_ENDPOINT1_IN_IEN_BIT        1
#define USB_ENDPOINT2_IN_IEN_BIT        2
#define USB_ENDPOINT3_IN_IEN_BIT        3
#define USB_ENDPOINT4_IN_IEN_BIT        4

/*
 * USB Interrupt OUT1 Enable Register Definitions.
 */
#define USB_INT_OUT_ENABLE_REG          (USB_BASE + 0x09)
#define USB_INT_OUT_ENABLE_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_INT_OUT_ENABLE_RESET_VALUE  0x00000000

/*
 * USB Interrupt OUT1 Enable Register Bit Definitions.
 */
#define USB_ENDPOINT1_OUT_IEN_BIT       1
#define USB_ENDPOINT2_OUT_IEN_BIT       2
#define USB_ENDPOINT3_OUT_IEN_BIT       3
#define USB_ENDPOINT4_OUT_IEN_BIT       4

/*
 * USB Interrupt Enable Register Definitions.
 */
#define USB_INT_ENABLE_REG              (USB_BASE + 0x0b)
#define USB_INT_ENABLE_REG_ACCESS       (READ_ACCESS | WRITE_ACCESS)
#define USB_INT_ENABLE_RESET_VALUE      0x00000000

/*
 * USB Interrupt Enable Register Bit Definitions.
 */
#define USB_SUSPEND_IEN_BIT             0
#define USB_RESUME_IEN_BIT              1
#define USB_RESET_IEN_BIT               2
#define USB_SOF_IEN_BIT                 3

/*
 * USB Frame Number Register Definitions.
 */
#define USB_FRAME_NUM_REG               (USB_BASE + 0x0c)
#define USB_FRAME_NUM_REG_ACCESS        (READ_ACCESS)
#define USB_FRAME_NUM_RESET_VALUE       0x00000000

/*
 * USB Frame Number Register Bit Definitions.
 */
#define USB_FRAME_NUM_FIELD             0
#define   NBITS_USB_FRAME_NUM             11

/*
 * USB Endpoint Index Register Definitions.
 */
#define USB_ENDPOINT_INDEX_REG          (USB_BASE + 0x0e)
#define USB_ENDPOINT_INDEX_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT_INDEX_RESET_VALUE  0x00000000

/*
 * USB Endpoint Index Register Bit Definitions.
 */
#define USB_ENDPOINT_INDEX_FIELD        0
#define   NBITS_USB_ENDPOINT_INDEX        4

/*
 * USB Max IN Packet Register Definitions.
 */
#define USB_IN_MAX_PACKET_REG            (USB_BASE + 0x10)
#define USB_IN_MAX_PACKET_REG_ACCESS     (READ_ACCESS | WRITE_ACCESS)
#define USB_IN_MAX_PACKET_RESET_VALUE   0x00000000

/*
 * USB Max IN Packet Register Bit Definitions.
 */
#define USB_IN_MAX_PACKET_FIELD         0
#define   NBITS_USB_IN_MAX_PACKET         8

/*
 * USB CSR0 Register Definitions.
 */
#define USB_CSR0_REG                    (USB_BASE + 0x11)
#define USB_CSR0_REG_ACCESS             (READ_ACCESS | WRITE_ACCESS)
#define USB_CSR0_RESET_VALUE            0x00000000

/*
 * USB CSR0 Register Bit Definitions.
 */
#define USB_CSR0_OUT_PKT_RDY_BIT        0
#define USB_CSR0_IN_PKT_RDY_BIT         1
#define USB_CSR0_STALL_SENT_BIT         2
#define USB_CSR0_LAST_PKT_BIT           3
#define USB_CSR0_SETUP_END_BIT          4
#define USB_CSR0_STALL_BIT              5
#define USB_CSR0_OUT_PKT_RDY_CLR_BIT    6
#define USB_CSR0_SETUP_END_CLR_BIT      7

/*
 * USB IN CSR1 Register Definitions.
 */
#define USB_INCSR1_REG                  (USB_BASE + 0x11)
#define USB_INCSR1_REG_ACCESS           (READ_ACCESS | WRITE_ACCESS)
#define USB_INCSR1_RESET_VALUE          0x00000000

/*
 * USB IN CSR1 Register Bit Definitions.
 */
#define USB_INCSR1_IN_PKT_RDY_BIT       0
#define USB_INCSR1_FIFO_NOT_EMPTY_BIT   1
#define USB_INCSR1_UNDERRUN_BIT         2
#define USB_INCSR1_FIFO_FLUSH_BIT       3
#define USB_INCSR1_STALL_BIT            4
#define USB_INCSR1_STALL_SENT_BIT       5
#define USB_INCSR1_CLR_DATA_TOG_BIT     6

/*
 * USB IN CSR2 Register Definitions.
 */
#define USB_INCSR2_REG                  (USB_BASE + 0x12)
#define USB_INCSR2_REG_ACCESS           (READ_ACCESS | WRITE_ACCESS)
#define USB_INCSR2_RESET_VALUE          0x00000000

/*
 * USB IN CSR2 Register Bit Definitions.
 */
#define USB_INCSR2_DATA_TOG_FORCE_BIT   3
#define USB_INCSR2_DMA_ENA_BIT          4
#define USB_INCSR2_MODE_BIT             5
#define USB_INCSR2_ISO_BIT              6
#define USB_INCSR2_AUTO_SET_BIT         7

/*
 * USB Max OUT Packet Register Definitions.
 */
#define USB_OUT_MAX_PACKET_REG          (USB_BASE + 0x13)
#define USB_OUT_MAX_PACKET_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_OUT_MAX_PACKET_RESET_VALUE  0x00000000

/*
 * USB Max OUT Packet Register Bit Definitions.
 */
#define USB_OUT_MAX_PACKET_FIELD        0
#define   NBITS_USB_OUT_MAX_PACKET        8

/*
 * USB OUT CSR1 Register Definitions.
 */
#define USB_OUTCSR1_REG                 (USB_BASE + 0x14)
#define USB_OUTCSR1_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define USB_OUTCSR1_REG_RESET_VALUE     0x00000000

/*
 * USB OUT CSR1 Register Bit Definitions.
 */
#define USB_OUTCSR1_OUT_PKT_RDY_BIT     0
#define USB_OUTCSR1_FIFO_FULL_BIT       1
#define USB_OUTCSR1_OVERRUN_BIT         2
#define USB_OUTCSR1_DATA_ERR_BIT        3
#define USB_OUTCSR1_FIFO_FLUSH_BIT      4
#define USB_OUTCSR1_STALL_BIT           5
#define USB_OUTCSR1_STALL_SENT_BIT      6
#define USB_OUTCSR1_CLR_DATA_TOG_BIT    7

/*
 * USB OUT CSR2 Register Definitions.
 */
#define USB_OUTCSR2_REG                 (USB_BASE + 0x15)
#define USB_OUTCSR2_REG_ACCESS          (READ_ACCESS | WRITE_ACCESS)
#define USB_OUTCSR2_REG_RESET_VALUE     0x00000000

/*
 * USB OUT CSR2 Register Bit Definitions.
 */
#define USB_OUTCSR2_DMA_MODE_BIT        4
#define USB_OUTCSR2_DMA_ENABLE_BIT      5
#define USB_OUTCSR2_ISO_BIT             6
#define USB_OUTCSR2_AUTO_CLEAR_BIT      7

/*
 * USB Endpoint 0 Count Register Definitions.
 */
#define USB_ENDPOINT0_COUNT_REG         (USB_BASE + 0x16)
#define USB_ENDPOINT0_COUNT_REG_ACCESS  (READ_ACCESS)
#define USB_ENDPOINT0_COUNT_RESET_VALUE 0x00000000

/*
 * USB Endpoint 0 Count Register Bit Definitions.
 */
#define USB_ENDPOINT0_COUNT_FIELD       0
#define   NBITS_USB_ENDPOINT0_COUNT       7

/*
 * USB OUT Count Register Definitions.
 */
#define USB_OUT_COUNT_REG               (USB_BASE + 0x16)
#define USB_OUT_COUNT_REG_ACCESS        (READ_ACCESS)
#define USB_OUT_COUNT_RESET_VALUE       0x00000000

/*
 * USB OUT Count Register Bit Definitions.
 */
#define USB_OUT_COUNT_FIELD             0
#define   NBITS_USB_OUT_COUNT             11

/*
 * USB Endpoint 0 FIFO Register Definitions.
 */
#define USB_ENDPOINT0_FIFO_REG          (USB_BASE + 0x20)
#define USB_ENDPOINT0_FIFO_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT0_FIFO_RESET_VALUE  0x00000000

/*
 * USB Endpoint 1 FIFO Register Definitions.
 */
#define USB_ENDPOINT1_FIFO_REG          (USB_BASE + 0x21)
#define USB_ENDPOINT1_FIFO_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT1_FIFO_RESET_VALUE  0x00000000

/*
 * USB Endpoint 2 FIFO Register Definitions.
 */
#define USB_ENDPOINT2_FIFO_REG          (USB_BASE + 0x22)
#define USB_ENDPOINT2_FIFO_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT2_FIFO_RESET_VALUE  0x00000000

/*
 * USB Endpoint 3 FIFO Register Definitions.
 */
#define USB_ENDPOINT3_FIFO_REG          (USB_BASE + 0x23)
#define USB_ENDPOINT3_FIFO_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT3_FIFO_RESET_VALUE  0x00000000

/*
 * USB Endpoint 4 FIFO Register Definitions.
 */
#define USB_ENDPOINT4_FIFO_REG          (USB_BASE + 0x24)
#define USB_ENDPOINT4_FIFO_REG_ACCESS   (READ_ACCESS | WRITE_ACCESS)
#define USB_ENDPOINT4_FIFO_RESET_VALUE  0x00000000

/*
 **********************************************************
 * JTAG Interface Definition
 **********************************************************
 */
#define JTAG_A7Vx05  0x08c9d2ffUL


/*
 * Triscend A7V JTAG Instructions
 */
#define JTAG_EXTEST          0x00
#define JTAG_SCAN_N          0x02
#define JTAG_INTEST          0x0c
#define JTAG_IDCODE          0x0e
#define JTAG_BYPASS          0x0f
#define JTAG_CLAMP           0x05
#define JTAG_HIGHZ           0x07
#define JTAG_CLAMPZ          0x09
#define JTAG_SAMPLE_PRELOAD  0x03
#define JTAG_RESTART         0x04
#define JTAG_NONE            0xff


/*
 * Triscend A7V JTAG Scan Chains
 *     The DEBUG chain is scanned with the msb first.
 *     All other chains are scanned with the lsb first.
 */
#define SCAN_TEST_CHAIN        0
#define DEBUG_CHAIN            1
#define ICEBREAKER_CHAIN       2
#define BOUNDARY_SCAN_CHAIN    3        // for Triscend internal use only
#define INTERNAL_SCAN_CHAIN    5        // for Triscend internal use only
#define BUSTAT_CHAIN           6
#define BUSCON_CHAIN           7
#define MEMORY_RW_CHAIN        9
#define MEMORY_RW_INCR_CHAIN  10
#define MEMORY_FILL_CHAIN     11
#define CRC_CHANNEL_CHAIN     12

/*
 * Bitlengths of Triscend A7V JTAG registers/chains
 */
#define BITLEN_SCAN_N_REG             4
#define BITLEN_IDENTIFICATION_REG    32
#define BITLEN_BYPASS_REG             1
#define BITLEN_INSTRUCTION_REG        4
#define BITLEN_DEBUG_CHAIN           33
#define BITLEN_ICEBREAKER_CHAIN      38
#define BITLEN_BUSTAT_CHAIN          20
#define BITLEN_BUSCON_CHAIN          12
#define BITLEN_MEMORY_RW_CHAIN       68
#define BITLEN_MEMORY_RW_INCR_CHAIN  34
#define BITLEN_MEMORY_FILL_CHAIN     97
#define BITLEN_CRC_CHANNEL_CHAIN     32


/*
 * Buscon chain bit defintion
 */
#define BUSCON_CPU_RESET_BIT         0
#define BUSCON_CPU_FREEZE_BIT        1
#define BUSCON_CPU_INT_BIT           2
#define BUSCON_FORCE_CFGRST_BIT      3
#define BUSCON_FORCE_NOCFGRST_BIT    4
#define BUSCON_OSC_BREAK_BIT         5
#define BUSCON_FORCE_BRST_BIT        6
#define BUSCON_FORCE_NOBRST_BIT      7
#define BUSCON_JTAG_BP_EVT_BIT       8
#define BUSCON_A7S_TAP_RESET_BIT     9
#define BUSCON_A7S_TAP_DISABLE_BIT  10

/*
 * BUSTAT chain bit definition
 */
#define BUSTAT_CPU_EXECUTING_BIT     0
#define BUSTAT_DBG_ACK_BIT           1
#define BUSTAT_BP_EVT_BIT            2
#define BUSTAT_CPU_IN_RESET_BIT      3
#define BUSTAT_BRST_STATUS_BIT       4
#define BUSTAT_BCLK_STATUS_BIT       5
#define BUSTAT_CFGRST_STATUS_BIT     6
#define BUSTAT_END_OF_TRACE_BIT      7
#define BUSTAT_CPU_RESET_BIT         8
#define BUSTAT_CPU_FREEZE_BIT        9
#define BUSTAT_CPU_INT_BIT          10
#define BUSTAT_FORCE_CFGRST_BIT     11
#define BUSTAT_FORCE_NOCFGRST_BIT   12
#define BUSTAT_OSC_BREAK_BIT        13
#define BUSTAT_FORCE_BRST_BIT       14
#define BUSTAT_FORCE_NOBRST_BIT     15
#define BUSTAT_JTAG_BP_EVT_BIT      16
#define BUSTAT_A7S_TAP_RESET_BIT    17
#define BUSTAT_A7S_TAP_DISABLE_BIT  18


#endif /* _TRISCEND_A7V_H */

/* end triscend_a7v.h */
