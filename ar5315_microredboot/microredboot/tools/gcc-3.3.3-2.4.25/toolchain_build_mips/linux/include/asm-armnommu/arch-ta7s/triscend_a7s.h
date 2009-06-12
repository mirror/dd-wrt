/*
 ***************************************************************************
 *  triscend_a7.h
 *
 *  Copyright (c) 2000, 2001 Triscend Corporation. All rights reserved.
 *
 ***************************************************************************
 */

#ifndef _TRISCEND_A7_H
#define _TRISCEND_A7_H

/*
 **********************************************************
 *  Memory Map
 **********************************************************
 */
#define EXTERNAL_SDRAM_BASE  0xc0000000
#define EXTERNAL_SDRAM_SIZE  0x10000000
#define EXTERNAL_FLASH_BASE  0xd0000000
#define EXTERNAL_FLASH_SIZE  0x01000000
#define CONTROL_REG_BASE     0xd1010000
#define CONTROL_REG_SIZE     0x00010000
#define TEST_AREA_BASE       0xd1020000
#define TEST_AREA_SIZE       0x00010000
#define INTERNAL_RAM_BASE    0xd1030000
#define INTERNAL_RAM_SIZE    0x00004000
#define CONFIG_MEM_BASE      0xd1040000
#define CONFIG_MEM_SIZE      0x00040000


/*
 **********************************************************
 *  Field and Bit Manipulation Defines and Macros
 **********************************************************
 */
#define A7_REG(reg) (*(volatile unsigned int*)(reg))

#define BIT_MASK(bitIdx) (1 << (bitIdx))

#define FIELD_MASK(fieldIdx,nBits) (((1 << (nBits)) - 1) << (fieldIdx))

#define GET_BIT(addr32,bitIdx) (*(volatile unsigned int*)(addr32) & (1 << (bitIdx)) ? 1 : 0)

#define PUT_BIT(addr32,bitIdx,bitVal) (*(volatile unsigned int*)(addr32) = (0 == (bitVal)) ? *(volatile unsigned int*)(addr32) & ~(1 << (bitIdx)) : *(volatile unsigned int*)(addr32) | (1 << (bitIdx)))
#define SET_BIT(addr32,bitIdx) (*(volatile unsigned int*)(addr32) |= (1 << (bitIdx)))

#define CLR_BIT(addr32,bitIdx) (*(volatile unsigned int*)(addr32) &= ~(1 << (bitIdx)))

#define GET_FIELD(addr32,fieldIdx,nBits) ((*(volatile unsigned int*)(addr32) >> (fieldIdx)) & ((1 << (nBits)) - 1))

#define PUT_FIELD(addr32,fieldIdx,nBits,fieldValue) (*(volatile unsigned int*)(addr32) = (*(volatile unsigned int*)(addr32) & ~FIELD_MASK(fieldIdx,nBits)) | (((fieldValue) << (fieldIdx)) & FIELD_MASK(fieldIdx,nBits)))
#define SET_FIELD(addr32,fieldIdx,nBits) (*(volatile unsigned int*)(addr32) |= FIELD_MASK(fieldIdx,nBits))

#define CLR_FIELD(addr32,fieldIdx,nBits) (*(volatile unsigned int*)(addr32) &= ~FIELD_MASK(fieldIdx,nBits))


/*
 **********************************************************
 *  Control Register Base Address Definitions
 **********************************************************
 */
#define MSS_BASE    0xd1010000
#define SYS_BASE    0xd1010100
#define INT_BASE    0xd1010200
#define REMAP_BASE  0xd1010400
#define TIMER_BASE  0xd1010500
#define WD_BASE     0xd1010600
#define CFG_BASE    0xd1010700
#define DMA_BASE    0xd1010800
#define UART_BASE   0xd1010900
#define BPU_BASE    0xd1010a00
#define PU_BASE     0xd1011100


/*
 **********************************************************
 * Memory Subsystem Unit Definition
 **********************************************************
 */

/*
 *  Memory Subsystem Configuration Register (read/write)
 */
#define MSS_CONFIG_REG          (MSS_BASE + 0x00)
#define MSS_CONFIG_RESET_VALUE  0x00004002
#define BUS_MODE_FIELD             0
#define   NBITS_BUS_MODE             4
#define MIU_DEV_WIDTH_FIELD        4
#define   NBITS_MIU_DEV_WIDTH        2
#define SDIU_DEV_WIDTH_FIELD       6
#define   NBITS_SDIU_DEV_WIDTH       2
#define R_MAP_FIELD                8
#define   NBITS_R_MAP                2
#define B_MAP_FIELD               10
#define   NBITS_B_MAP                3
#define N_BANK_BIT                13
#define NE_BANK_BIT               14
#define PIPE_BIT                  15
#define SDRAM_DMA_BUF_EN_FIELD    16
#define   NBITS_SDRAM_DMA_BUF_EN     4
#define FLASH_DMA_BUF_EN_FIELD    20
#define   NBITS_FLASH_DMA_BUF_EN     4
#define MSS_FLASH_X16             24
/* The following fields are now deleted
#define MSS_ARB_FIELD             24
#define   NBITS_MSS_ARB              2
*/


/*
 *  Static Memory Interface Timing Control Register (read/write)
 */
#define MSS_TIM_CTRL_REG          (MSS_BASE + 0x04)
#define MSS_TIM_CTRL_RESET_VALUE  0x00077777
#define WC_SETUP_FIELD      0
#define   NBITS_WC_SETUP      4
#define WC_WIDTH_FIELD      4
#define   NBITS_WC_WIDTH      4
#define WC_HOLD_FIELD       8
#define   NBITS_WC_HOLD       4
#define RC_SETUP_FIELD     12
#define   NBITS_RC_SETUP      4
#define RC_WIDTH_FIELD     16
#define   NBITS_RC_WIDTH      4


/*
 *  SDRAM Mode Register (read/write)
 */
#define MSS_SDR_MODE_REG          (MSS_BASE + 0x08)
#define MSS_SDR_MODE_RESET_VALUE  0x02223222
#define TRP_FIELD          0
#define   NBITS_TRP          3
#define TRCD_FIELD         4
#define   NBITS_TRCD         3
#define TWR_FIELD          8
#define   NBITS_TWR          3
#define TRC_FIELD         12
#define   NBITS_TRC          3
#define MODE_REG_FIELD    16
#define   NBITS_MODE_REG    14


/*
 *  SDRAM Control Register (read/write)
 */
#define MSS_SDR_CTRL_REG          (MSS_BASE + 0x0c)
#define MSS_SDR_CTRL_RESET_VALUE  0x00000000
#define PWR_MAN_FIELD         0
#define   NBITS_PWR_MAN         3
#define RFSH_RATE_FIELD      16
#define   NBITS_RFSH_RATE      12
#define RFSH_BURST_FIELD     28
#define   NBITS_RFSH_BURST      4


/*
 *  SDRAM Status Register (read only)
 */
#define MSS_STATUS_REG          (MSS_BASE + 0x10)
#define MSS_STATUS_RESET_VALUE  0x00000001
#define SD_STATUS_FIELD    0
#define   NBITS_SD_STATUS    3
#define RFSH_OVF_BIT       3


/*
 *  SDRAM Status Clear Register (write only)
 *      write 1 to clear the corresponding status reg bit
 */
#define MSS_STATUS_CLEAR_REG  (MSS_BASE + 0x14)
#define RFSH_OVF_CLR_BIT   3


/*
 **********************************************************
 *  System Control Registers
 **********************************************************
 */

/*
 *  Clock Control Register (read/write)
 *      Use this register to select which portion of the
 *      device is turned off or kept on in power down mode.
 *      For any of the bits set, the corresponding circuit
 *      is be turned off in power down.
 *      A "0" will keep the circuit running at all times.
 *      The PLL output frequency = PLL_DIV x 32KHz
 *      PLL_SCALE:   PLL Pre-scale Value:
 *       0             1
 *       1             2
 *       2             4
 *       4             8
 *       8            16
 *      16            32
 */
#define SYS_CLOCK_CONTROL_REG      (SYS_BASE + 0x00)
#define CLOCK_CONTROL_RESET_VALUE  0x00040000
#define CLK_SEL_BIT        0
#define PLL_SEL_BIT        1
#define CSL_SEL_BIT        2
#define CK_EN_BIT          3
#define XTAL_EN_BIT        4
#define PLL_EN_BIT         5
#define PLL_TEST_BIT       6
#define REF_SEL_BIT        7
#define PLL_DIV_FIELD      8
#define   NBITS_PLL_DIV    12
#define PLL_SCALE_FIELD    24
#define   NBITS_PLL_SCALE     5


/*
 *  PLL Status Register (read only)
 */
#define SYS_PLL_STATUS_REG      (SYS_BASE + 0x04)
#define PLL_STATUS_RESET_VALUE  0x00000001
#define PLL_NOT_LOCK_BIT   0
#define PLL_LOCK_BIT       1


/*
 *  PLL Status Clear Register (write only)
 */
#define SYS_PLL_STATUS_CLEAR_REG  (SYS_BASE + 0x08)
#define PLL_NOT_LOCK_CLEAR_BIT   0
#define PLL_LOCK_CLEAR_BIT       1


/*
 *  Reset Control Register (read/write)
 *      The SYS_RESET bit is used by software to reset the
 *      system as seen by the user. It resets the CPU, the
 *      bus and all the peripherals without affecting the
 *      configuration of the device.
 *      The bit is self-clearing.
 */
#define SYS_RESET_CONTROL_REG      (SYS_BASE + 0x0c)
#define RESET_CONTROL_RESET_VALUE  0x00000000
#define SLAVE_DIS_BIT    1
#define SYS_RESET_BIT    2


/*
 *  Power Down Control Register (read/write)
 *      User can select which portion of the device is
 *      turned off or kept on during power down mode.
 *      Setting a bit turns off the circuit in power down.
 *      A zero keeps the circuit running at all times.
 */
#define SYS_POWER_CONTROL_REG      (SYS_BASE + 0x10)
#define POWER_CONTROL_RESET_VALUE  0x00000000
#define PD_BCK_EN_BIT      0
#define PD_CSL_BCK_EN_BIT  1
#define PD_OSC_EN_BIT      2
#define PD_IO_EN_BIT       3
#define PD_BIT             4
#define POR_DIS_BIT        5


/*
 *  Pause Register (write only)
 *      Writing to this register puts the ARM core (only) in
 *      a low power state until receiving an interrupt.
 */
#define REMAP_PAUSE_REG  (REMAP_BASE + 0x00)


/*
 *  Identification Register (read only)
 */
#define REMAP_IDENTIFICATION_REG  (REMAP_BASE + 0x10)
#define TRISCEND_TA7S20  0x1803d2ff


/*
 *  Revision Register (read only)
 *      Revision = 0 for first device
 */
#define REMAP_REVISION_REG  (REMAP_BASE + 0x14)
#define TRISCEND_TA7S20_REV113  0x00000113


/*
 *  Clear Reset Map Register (write only)
 *      Writing to this address causes a system memory map switch from the
 *      user initial memory map to the one used during normal operation.
 *      It effectively clears the Flash alias bit in the Alias Enable Register.
 */
#define REMAP_CLEAR_RESET_MAP_REG  (REMAP_BASE + 0x20)


/*
 *  Reset Status Register (read only)
 *      Indicates the cause of the latest reset.
 *      Default reset value depends on reset cause.
 */
#define REMAP_RESET_STATUS_REG  (REMAP_BASE + 0x30)
#define POR_BIT         0
#define CFG_RST_BIT     1
#define RST_PIN_BIT     2
#define J_CFG_RST_BIT   3
#define CPU_RST_BIT     4
#define J_CPU_RST_BIT   5
#define WD_RST_BIT      6
#define APP_RST_BIT     7
#define SYS_RST_BIT     8
#define J_SYS_RST_BIT   9
#define SOFT_RST_BIT   10


/*
 *  Reset Status Clear Register (write only)
 *      Writing a "1" clears the corresponding bit in Reset Status Register.
 *      Writing a "0" has no effect.
 */
#define REMAP_RESET_STATUS_CLEAR_REG  (REMAP_BASE + 0x34)
#define POR_CLR_BIT         0
#define CFG_RST_CLR_BIT     1
#define RST_PIN_CLR_BIT     2
#define J_CFG_RST_CLR_BIT   3
#define CPU_RST_CLR_BIT     4
#define J_CPU_RST_CLR_BIT   5
#define WD_RST_CLR_BIT      6
#define APP_RST_CLR_BIT     7
#define SYS_RST_CLR_BIT     8
#define J_SYS_RST_CLR_BIT   9
#define SOFT_RST_CLR_BIT   10


/*
 *  Pin Status Register (read only)
 *      This register enables software to get the status of
 *      some static pins of the device.
 */
#define REMAP_PIN_STATUS_REG  (REMAP_BASE + 0x38)
#define VSYS_BIT       0
#define RSTN_BIT       1
#define SLAVEN_BIT     2
#define VSYS_GOOD_BIT  3
#define VSYS_BAD_BIT   4


/*
 *  Pin Status Clear Register (write only)
 *      Writing a "1" clears the corresponding bit in the Pin Status Register.
 *      Writing a "0" has no effect.
 */
#define REMAP_PIN_STATUS_CLEAR_REG  (REMAP_BASE + 0x3c)
#define VSYS_GOOD_CLR_BIT  3
#define VSYS_BAD_CLR_BIT   4


/*
 *  Alias Enable Register (read/write)
 *      This register defines which alias is enabled at the bottom of the
 *      memory starting at address 0. If more than one alias is enabled,
 *      they are overlaid over each other with the following priority
 *      (from highest to lowest priority): ROM, FLASH, SRAM, SDRAM.
 */
#define REMAP_ALIAS_ENABLE_REG    (REMAP_BASE + 0x40)
#define ALIAS_ENABLE_RESET_VALUE  0x0000000f
#define ROM_AEN_BIT     0
#define FLASH_AEN_BIT   1
#define SRAM_AEN_BIT    2
#define SDRAM_AEN_BIT   3


/*
 *  SRAM Config Register (read/write)
 */
#define REMAP_SRAM_CONFIG_REG    (REMAP_BASE + 0x44)
#define SRAM_CONFIG_RESET_VALUE  0x00000000
#define SRAM_PRIO_BIT             0
#define SRAM_SIZE_BIT             1
#define SRAM_PROTECT_FIELD        2
#define   NBITS_SRAM_PROTECT        4


/*
 *  SRAM Base Address Register (read/write)
 *      The internal SRAM can be relocated using this register
 */
#define REMAP_SRAM_BASE_ADR_REG    (REMAP_BASE + 0x48)
#define SRAM_BASE_ADR_RESET_VALUE  0x0d1030000
#define SRAM_BASE_ADR_FIELD   14
#define   NBITS_SRAM_BASE_ADR    18


/*
 *  Access Protect Register (read/write)
 *      When the dmaDis bit is set, DMA writes are not
 *      allowed into the control register area.
 */
#define REMAP_ACC_PROTECT_REG    (REMAP_BASE + 0x4c)
#define ACC_PROTECT_RESET_VALUE  0x00000001
#define DMA_DIS_BIT  0
#define TEST_EN_BIT  1


/*
 **********************************************************
 *  Configuration Unit Definition
 **********************************************************
 */

/*
 *  Configuration Control Register (read/write)
 */
#define CFG_CONFIG_CONTROL_REG      (CFG_BASE + 0x00)
#define CONFIG_CONTROL_RESET_VALUE  0x00000000
#define EN_ZIP_BIT       0
#define EN_REG_BIT       1
#define EN_IO_BIT        2
#define CONFIG_DONE_BIT  4
#define DEC_EN_BIT       5
#define CFGENS_EN_BIT    6
#define CFG_STOP_BIT     7


/*
 *  Configuration Timing Register (read/write)
 *      CSL Configuration Access Wait States:
 *          generated by config unit in response to CSL reads.
 *          number of wait states = cslWait
 *      PIO Phase Delay:
 *          length in clock cycles of PIO config read & write phases.
 *          length in clock cycles = (pioPhase - 1)
 */
#define CFG_CONFIG_TIMING_REG      (CFG_BASE + 0x04)
#define CONFIG_TIMING_RESET_VALUE  0x00000024
#define CSL_WAIT_FIELD       0
#define   NBITS_CSL_WAIT       3
#define PIO_PHASE_FIELD      3
#define   NBITS_PIO_PHASE      3


/*
 *  IO Recover Register (read/write)
 */
#define CFG_IO_RECOVER_REG      (CFG_BASE + 0x08)
#define IO_RECOVER_RESET_VALUE  0x00008061
#define SD_SEL_BIT           0
#define CEN1_SEL_BIT         1
#define CEN2_SEL_BIT         2
#define CEN3_SEL_BIT         3
#define XDONE_SEL_BIT        4
#define A18_SEL_BIT          5
#define A19_SEL_BIT          6
#define A20_SEL_BIT          7
#define A21_SEL_BIT          8
#define A22_SEL_BIT          9
#define A23_SEL_BIT         10
#define A24_TO_A31_SEL_BIT  11
#define D8_TO_D15_SEL_BIT   12
#define D16_TO_D31_SEL_BIT  13
#define ECLK_SEL_BIT        14
#define SDCEN1_SEL_BIT      15


/*
 *  Configuration Protection Register (read/write)
 *      When a protection bit is set, the configuration plane
 *      is protected from read or write accesses. (Read data = 0).
 *      Once this bit is set, it can only be cleared through a power cycle.
 */
#define CFG_CONFIG_PROTECT_REG      (CFG_BASE + 0x0c)
#define CONFIG_PROTECT_RESET_VALUE  0x00000000
#define RD_SECURE_BIT   0
#define WR_SECURE_BIT   1


/*
 **********************************************************
 * Interrupt Unit Definition
 **********************************************************
 */

/*
 *  Interrupt Registers Definition
 */
#define INT_IRQ_STATUS_REG          (INT_BASE + 0x00)	// read only
#define INT_IRQ_RAW_STATUS_REG      (INT_BASE + 0x04)	// read only
#define INT_IRQ_ENABLE_REG          (INT_BASE + 0x08)	// read/write
#define INT_IRQ_ENABLE_CLEAR_REG    (INT_BASE + 0x0c)	// write only
#define INT_IRQ_SOFT_REG            (INT_BASE + 0x10)	// write only
#define INT_FIQ_STATUS_REG          (INT_BASE + 0x100)	// read only
#define INT_FIQ_RAW_STATUS_REG      (INT_BASE + 0x104)	// read only
#define INT_FIQ_ENABLE_REG          (INT_BASE + 0x108)	// read/write
#define INT_FIQ_ENABLE_CLEAR_REG    (INT_BASE + 0x10c)	// write only
#define INT_IRQ_STEER_REG           (INT_BASE + 0x110)	// write only
#define IRQ_STATUS_RESET_VALUE      0x00000000
#define IRQ_RAW_STATUS_RESET_VALUE  0x00000000
#define IRQ_ENABLE_RESET_VALUE      0x00000000
#define FIQ_STATUS_RESET_VALUE      0x00000000
#define FIQ_RAW_STATUS_RESET_VALUE  0x00000000
#define FIQ_ENABLE_RESET_VALUE      0x00000000
#define FIQ_BIT              0
#define IRQ_SOFTWARE_BIT     1
#define IRQ_SERIAL_0_BIT     2
#define IRQ_TIMER_0_BIT      3
#define IRQ_TIMER_1_BIT      4
#define IRQ_SERIAL_1_BIT     5
#define IRQ_WATCHDOG_BIT     6
#define IRQ_DMA_0_BIT        7
#define IRQ_DMA_1_BIT        8
#define IRQ_DMA_2_BIT        9
#define IRQ_DMA_3_BIT       10
#define IRQ_CSL_USER_0_BIT  11
#define IRQ_CSL_USER_1_BIT  12
#define IRQ_CSL_USER_2_BIT  13
#define IRQ_JTAG_BIT        14
#define IRQ_BREAKPOINT_BIT  15


/*
 ***********************************************************
 * DMA Unit Definition
 ***********************************************************
 */

/*
 *  DMA Control Registers Definition (read/write)
 */
#define DMA0_CONTROL_REG         (DMA_BASE + 0x00)
#define DMA1_CONTROL_REG         (DMA_BASE + 0x40)
#define DMA2_CONTROL_REG         (DMA_BASE + 0x80)
#define DMA3_CONTROL_REG         (DMA_BASE + 0xc0)
#define DMA_CONTROL_RESET_VALUE  0x00000000
#define CLEAR_BIT                      0
#define DMA_ENABLE_BIT                 1
#define DMA_INIT_BIT                   2
#define CONT_BIT                       3
#define SFT_REQ_BIT                    4
#define BLOCK_EN_BIT                   5
#define TRANS_DIR_FIELD                6
#define   NBITS_TRANS_DIR                2
#define SRC_ADDR_MODE_FIELD            8
#define   NBITS_SRC_ADDR_MODE            2
#define DEST_ADDR_MODE_FIELD          10
#define   NBITS_DEST_ADDR_MODE           2
#define TRANS_SIZE_FIELD              12
#define   NBITS_TRANS_SIZE               2
#define METHOD_BIT                    14
#define AUX_DIS_BIT                   15
#define CRC_EN_BIT                    16
#define BROADCAST_BIT                 17


/*
 *  DMA Interrupt Registers Definitions
 */
#define DMA0_INT_ENABLE_REG         (DMA_BASE + 0x04)	// read/write
#define DMA1_INT_ENABLE_REG         (DMA_BASE + 0x44)	// read/write
#define DMA2_INT_ENABLE_REG         (DMA_BASE + 0x84)	// read/write
#define DMA3_INT_ENABLE_REG         (DMA_BASE + 0xc4)	// read/write
#define DMA0_INT_REG                (DMA_BASE + 0x08)	// read only
#define DMA1_INT_REG                (DMA_BASE + 0x48)	// read only
#define DMA2_INT_REG                (DMA_BASE + 0x88)	// read only
#define DMA3_INT_REG                (DMA_BASE + 0xc8)	// read only
#define DMA0_INT_CLEAR_REG          (DMA_BASE + 0x0c)	// write only
#define DMA1_INT_CLEAR_REG          (DMA_BASE + 0x4c)	// write only
#define DMA2_INT_CLEAR_REG          (DMA_BASE + 0x8c)	// write only
#define DMA3_INT_CLEAR_REG          (DMA_BASE + 0xcc)	// write only
#define DMA_INT_ENABLE_RESET_VALUE  0x00000000
#define DMA_INT_RESET_VALUE         0x00000000
#define TC_BIT        0
#define INIT_BIT      1
#define OVF_BIT       2
#define FULL_BIT      3
#define EMPTY_BIT     4
#define LAST_BIT      5
#define RETRANS_BIT   6
#define DESC_BIT      7
#define BAD_RETR_BIT  8


/*
 *  DMA Transfer Count Registers Definition
 */
#define DMA0_TRANS_CNT_REG  (DMA_BASE + 0x1c)	// read/write
#define DMA1_TRANS_CNT_REG  (DMA_BASE + 0x5c)	// read/write
#define DMA2_TRANS_CNT_REG  (DMA_BASE + 0x9c)	// read/write
#define DMA3_TRANS_CNT_REG  (DMA_BASE + 0xdc)	// read/write
#define TRANS_CNT_FIELD    0
#define   NBITS_TRANS_CNT    16

#define DMA0_CUR_TRANS_CNT_REG  (DMA_BASE + 0x2c)	// read only
#define DMA1_CUR_TRANS_CNT_REG  (DMA_BASE + 0x6c)	// read only
#define DMA2_CUR_TRANS_CNT_REG  (DMA_BASE + 0xac)	// read only
#define DMA3_CUR_TRANS_CNT_REG  (DMA_BASE + 0xec)	// read only
#define CUR_TRANS_CNT_FIELD   TRANS_CNT_FIELD
#define   NBITS_CUR_TRANS_CNT   NBITS_TRANS_CNT


/*
 *  DMA Pending Request Counter Register Definition (read only)
 */
#define DMA0_PEND_REQ_REG  (DMA_BASE + 0x30)
#define DMA1_PEND_REQ_REG  (DMA_BASE + 0x70)
#define DMA2_PEND_REQ_REG  (DMA_BASE + 0xb0)
#define DMA3_PEND_REQ_REG  (DMA_BASE + 0xf0)
#define PEND_REQ_CTRL_FIELD      0
#define   NBITS_PEND_REQ_CTRL     10
#define LAST_POST_FIELD         16
#define   NBITS_LAST_POST         10


/*
 *  DMA Descriptor Table
 */
#define DMA0_DES_TABLE_ADDR_REG  (DMA_BASE + 0x10)	// read/write
#define DMA1_DES_TABLE_ADDR_REG  (DMA_BASE + 0x50)	// read/write
#define DMA2_DES_TABLE_ADDR_REG  (DMA_BASE + 0x90)	// read/write
#define DMA3_DES_TABLE_ADDR_REG  (DMA_BASE + 0xd0)	// read/write
#define DMA0_CUR_DESC_ADDR_REG   (DMA_BASE + 0x20)	// read only
#define DMA1_CUR_DESC_ADDR_REG   (DMA_BASE + 0x60)	// read only
#define DMA2_CUR_DESC_ADDR_REG   (DMA_BASE + 0xa0)	// read only
#define DMA3_CUR_DESC_ADDR_REG   (DMA_BASE + 0xe0)	// read only

#if !defined( _ASMLANGUAGE ) && !defined( __ASSEMBLER__ )
typedef struct
{
    unsigned int sourceAddress;
    unsigned int reserved;
    unsigned int destinationAddress;
    unsigned int controlStatus;
}
DMA_DESCRIPTOR;
#endif

/*
 * DMA_DESCRIPTOR controlStatus Definition
 */
#define ACT_WHEN_COMPL_FIELD          0
#define   NBITS_ACT_WHEN_COMPL          2
#define DESC_INT_BIT                  2
#define CRC_CLR_DIS_BIT               3
#define CONT_DESC_BIT                 4
#define BUFF_STAT_BIT                 5
#define TRANS_LENGTH_FIELD           16
#define   NBITS_TRANS_LENGTH           16


/*
 *  Other DMA Registers Definition - 32-bit registers
 */
#define DMA0_SRC_ADDR_REG       (DMA_BASE + 0x14)	// read/write
#define DMA1_SRC_ADDR_REG       (DMA_BASE + 0x54)	// read/write
#define DMA2_SRC_ADDR_REG       (DMA_BASE + 0x94)	// read/write
#define DMA3_SRC_ADDR_REG       (DMA_BASE + 0xd4)	// read/write
#define DMA0_CUR_SRC_ADDR_REG   (DMA_BASE + 0x24)	// read only
#define DMA1_CUR_SRC_ADDR_REG   (DMA_BASE + 0x64)	// read only
#define DMA2_CUR_SRC_ADDR_REG   (DMA_BASE + 0xa4)	// read only
#define DMA3_CUR_SRC_ADDR_REG   (DMA_BASE + 0xe4)	// read only


#define DMA0_DST_ADDR_REG       (DMA_BASE + 0x18)	// read/write
#define DMA1_DST_ADDR_REG       (DMA_BASE + 0x58)	// read/write
#define DMA2_DST_ADDR_REG       (DMA_BASE + 0x98)	// read/write
#define DMA3_DST_ADDR_REG       (DMA_BASE + 0xd8)	// read/write
#define DMA0_CUR_DEST_ADDR_REG  (DMA_BASE + 0x28)	// read only
#define DMA1_CUR_DEST_ADDR_REG  (DMA_BASE + 0x68)	// read only
#define DMA2_CUR_DEST_ADDR_REG  (DMA_BASE + 0xa8)	// read only
#define DMA3_CUR_DEST_ADDR_REG  (DMA_BASE + 0xe8)	// read only


#define DMA_CRC_REG  (DMA_BASE + 0xfc)	// read only


/*
 **********************************************************
 * UART Definition
 **********************************************************
 */

/*
 *  Uart Control Register Structure (read/write)
 */
#define UART0_CONTROL_REG         (UART_BASE + 0x00)
#define UART1_CONTROL_REG         (UART_BASE + 0x40)
#define UART_CONTROL_RESET_VALUE  0x00000000
#define UART_PRESCALE_FIELD     0
#define   NBITS_UART_PRESCALE     8
#define PRESCALE_EN_BIT         8
#define MODEM_EN_BIT            9
#define TX_DMA_EN_BIT          16
#define TX_DMA_SEL_FIELD       17
#define   NBITS_TX_DMA_SEL        2
/*
 * reserved bit 19
 */
#define RX_DMA_EN_BIT          20
#define RX_DMA_SEL_FIELD       21
#define   NBITS_RX_DMA_SEL        2


/*
 *  DLAB=0: Uart Rx Tx Register Structure       (read/write)
 *  DLAB=1: Uart Divisor LSB Register Structure (read/write)
 */
#define UART0_RX_TX_REG  (UART_BASE + 0x20)
#define UART1_RX_TX_REG  (UART_BASE + 0x60)
#define DATA_FIELD    0
#define   NBITS_DATA    8

#define UART0_DIVISOR_LSB_REG  (UART_BASE + 0x20)
#define UART1_DIVISOR_LSB_REG  (UART_BASE + 0x60)
#define DIVISOR_LSB_FIELD    0
#define   NBITS_DIVISOR_LSB    8


/*
 *  DLAB=0: Uart Interrupt Enable Register Structure (read/write)
 *  DLAB=1: Uart Divisor MSB Register Structure      (read/write)
 */
#define UART0_INT_ENABLE_REG         (UART_BASE + 0x24)
#define UART1_INT_ENABLE_REG         (UART_BASE + 0x64)
#define UART_INT_ENABLE_RESET_VALUE  0x00000000
#define RDRE_BIT   0
#define THREE_BIT  1
#define RLSE_BIT   2
#define MSE_BIT    3

#define UART0_DIVISOR_MSB_REG  (UART_BASE + 0x24)
#define UART1_DIVISOR_MSB_REG  (UART_BASE + 0x64)
#define DIVISOR_MSB_FIELD     0
#define   NBITS_DIVISOR_MSB     8


/*
 *  Uart Interrupt ID Register Structure (read only)
 *  Uart Fifo Control Register Structure (write only)
 */
#define UART0_INT_ID_REG         (UART_BASE + 0x28)
#define UART1_INT_ID_REG         (UART_BASE + 0x68)
#define UART_INT_ID_RESET_VALUE  0x00000001
#define INT_ID_FIELD            0
#define   NBITS_INT_ID            4
#define FIFO_MODE_FIELD         6
#define   NBITS_FIFO_MODE         2

#define UART0_FIFO_CTRL_REG         (UART_BASE + 0x28)
#define UART1_FIFO_CTRL_REG         (UART_BASE + 0x68)
#define UART_FIFO_CTRL_RESET_VALUE  0x00000000
#define FIFO_MODE_ENABLE_BIT      0
#define RX_FIFO_CLR_BIT           1
#define TX_FIFO_CLR_BIT           2
#define FIFO_TRIG_LEVEL_FIELD     6
#define   NBITS_FIFO_TRIG_LEVEL     2


/*
 *  Uart Line Control Register Structure (read/write)
 */
#define UART0_LINE_CONTROL_REG         (UART_BASE + 0x2c)
#define UART1_LINE_CONTROL_REG         (UART_BASE + 0x6c)
#define UART_LINE_CONTROL_RESET_VALUE  0x00000000
#define STB_WLS_FIELD     0
#define   NBITS_STB_WLS     3
#define PEN_BIT           3
#define EPS_BIT           4
#define STICK_PARITY_BIT  5
#define BREAK_BIT         6
#define DLAB_BIT          7


/*
 *  Uart Modem Control Register Structure (read/write)
 */
#define UART0_MODEM_CONTROL_REG         (UART_BASE + 0x30)
#define UART1_MODEM_CONTROL_REG         (UART_BASE + 0x70)
#define UART_MODEM_CONTROL_RESET_VALUE  0x00000000
#define DTR_BIT   0
#define RTS_BIT   1
#define LOOP_BIT  3


/*
 *  Uart Line Status Register Structure (read only)
 */
#define UART0_LINE_STATUS_REG         (UART_BASE + 0x34)
#define UART1_LINE_STATUS_REG         (UART_BASE + 0x74)
#define UART_LINE_STATUS_RESET_VALUE  0x00000060
#define DR_BIT     0
#define OE_BIT     1
#define PE_BIT     2
#define FE_BIT     3
#define BI_BIT     4
#define THRE_BIT   5
#define TEMT_BIT   6
#define ERROR_BIT  7


/*
 *  Uart Modem Status Register Structure (read only)
 */
#define UART0_MODEM_STATUS_REG         (UART_BASE + 0x38)
#define UART1_MODEM_STATUS_REG         (UART_BASE + 0x78)
#define UART_MODEM_STATUS_RESET_VALUE  0x00000000
#define DELTA_CTS_BIT   0
#define DELTA_DSR_BIT   1
#define TERI_BIT        2
#define DELTA_DCD_BIT   3
#define CTS_BIT         4
#define DSR_BIT         5
#define RI_BIT          6
#define DCD_BIT         7


/*
 *  Uart Scratchpad Register Structure (read/write)
 */
#define UART0_SCRATCHPAD_REG  (UART_BASE + 0x3c)
#define UART1_SCRATCHPAD_REG  (UART_BASE + 0x7c)


/*
 **********************************************************
 * Timer Unit Definition
 **********************************************************
 */
/*
 *  Timer Load Register Structure (read/write)
 */
#define TIMER0_LOAD_REG  (TIMER_BASE + 0x00)
#define TIMER1_LOAD_REG  (TIMER_BASE + 0x20)
#define LOAD_FIELD       0
#define   NBITS_LOAD      16


/*
 *  Timer Value Register Structure (read only)
 */
#define TIMER0_VALUE_REG  (TIMER_BASE + 0x04)
#define TIMER1_VALUE_REG  (TIMER_BASE + 0x24)
#define VALUE_FIELD      0
#define   NBITS_VALUE     16


/*
 *  Timer Control Register Structure (read/write)
 */
#define TIMER0_CONTROL_REG         (TIMER_BASE + 0x08)
#define TIMER1_CONTROL_REG         (TIMER_BASE + 0x28)
#define TIMER_CONTROL_RESET_VALUE  0x00000000
#define PRESCALE_FIELD       2
#define   NBITS_PRESCALE       2
#define TIM_MODE_BIT         6
#define TIM_ENABLE_BIT       7


/*
 *  Timer Clear Register Structure (write only)
 *      Write a "1" to clear the timer interrupt.
 *      Writing a "0" has no effect.
 */
#define TIMER0_CLEAR_REG  (TIMER_BASE + 0x0c)
#define TIMER1_CLEAR_REG  (TIMER_BASE + 0x2c)
#define TIM_INT_CLEAR_BIT   0


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
#define WATCHDOG_CONTROL_REG          (WD_BASE + 0x00)
#define WATCHDOG_CONTROL_RESET_VALUE  0x00000000
#define WD_ENABLE_BIT    0
#define WD_RESET_BIT     1
#define EN_WD_RST_BIT    2


/*
 *  Watchdog Timeout Value Register Structure (read/write)
 */
#define WATCHDOG_TIMEOUT_VAL_REG  (WD_BASE + 0x04)


/*
 *  Watchdog Current Value Register Structure (read only)
 */
#define WATCHDOG_CURRENT_VAL_REG  (WD_BASE + 0x08)


/*
 *  Watchdog Clear Register Structure (write only)
 *      Writing a '1' to the WD_INT_CLR bit will clear the watchdog interrupt.
 *      Writing a "0" has no effect.
 */
#define WATCHDOG_CLEAR_REG  (WD_BASE + 0x0c)
#define WD_INT_CLR_BIT   0


/*
 ******************************************************************************
 * Breakpoint Unit Definition
 ******************************************************************************
 */

/*
 * Breakpoint Control Register definition (read/write)
 */
#define BPU_BRK_CONTROL_REG      (BPU_BASE + 0x00)
#define BRK_CONTROL_RESET_VALUE  0x00000000
#define CPU_STIM_FIELD                 0
#define   NBITS_CPU_STIM                 3
#define CSL_FREEZE_BIT                 3
#define CSL_MATCH_BIT                  4
#define BP_TYPE_FIELD                  5
#define   NBITS_BP_TYPE                  3
#define RESTART_BIT                    8
#define BP_ENABLE_FIELD                9
#define   NBITS_BP_ENABLE                2
#define BP0_NOT_MATCH_BIT             13
#define BP1_NOT_MATCH_BIT             14
#define ARB_FREEZE_BIT                15
#define BUS_SEL_BIT                   16
#define BP0_DATA_SEL_BIT              17
#define BP1_DATA_SEL_BIT              18
#define TRACE_EN_BIT                  20
#define TRACE_FORMAT_FIELD            21
#define   NBITS_TRACE_FORMAT             3
#define CSL_CAPT_EN_BIT               24
#define BCLK_FREEZE_BIT               25
#define CPU_DBG_EN_BIT                26


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
/*
 * BUS MASK 2 for CPU BUS
 */
#define NRW_BIT               0
#define MAS_FIELD             1
#define   NBITS_MAS             2
#define LOCK_BIT              3
#define SEQ_BIT               4
#define ABORT_BIT             5
#define NTRANS_BIT            6
#define NMREQ_BIT             7
#define OPC_BIT               8
#define DBGACK_BIT           17
/*
 * BUS MASK2 for CSI BUS
 */
#define SW_DMA_ACK_FIELD      0
#define   NBITS_SW_DMA_ACK      4
#define SW_MODE_FIELD         4
#define   NBITS_SW_MODE         4
#define SW_DMA_CTRL_FIELD     8
#define   NBITS_SW_DMA_CTRL     4
#define SW_SIZE_FIELD        12
#define   NBITS_SW_SIZE         2
#define SW_RD_EN_BIT         14
#define SW_WR_EN_BIT         15
#define SW_BP_CTRL_BIT       16
#define RD_OR_WR_BIT         18
#define OR_DMA_ACK_BIT       19
#define RD_WR_ACK_BIT        20


/*
 * Breakpoint Bus Mask3 Registers Definition (read/write)
 */
#define BPU_BRK0_BUS_MASK3_REG  (BPU_BASE + 0x10)
#define BPU_BRK1_BUS_MASK3_REG  (BPU_BASE + 0x20)
#define FR_FIELD    0
#define   NBITS_FR   11


/*
 * Breakpoint Counter Compare/Out Registers Definition
 */
#define BPU_BRK_CNT_COMP_REG  (BPU_BASE + 0x44)	// read/write
#define BPU_BRK_CNT_OUT_REG   (BPU_BASE + 0x48)	// read only
#define BRK_CNT0_FIELD     0
#define   NBITS_BRK_CNT0    16
#define BRK_CNT1_FIELD    16
#define   NBITS_BRK_CNT1    16


/*
 * Trace Buffer Address Pointer Register Definition (read only)
 * Trace Counter Register Definition (read/write)
 */
#define BPU_BRK_TR_ADDR_REG  (BPU_BASE + 0x4c)
#define TRACE_ADDR_FIELD      0	// read only
#define   NBITS_TRACE_ADDR      9	// read only
#define TR_ADDR_FLIP_BIT      9	// read only
#define TRACE_CNT_FIELD      16	// read/write
#define   NBITS_TRACE_CNT       9	// read/write


/*
 * Breakpoint Interrupt Clear Register Definition (write only)
 */
#define BPU_BRK_INT_CLEAR_REG  (BPU_BASE + 0x50)
#define BRK_INT_CLEAR_BIT   0


/*
 * Breakpoint Status Register Definition (read only)
 */
#define BPU_BRK_STATUS_REG  (BPU_BASE + 0x54)
#define STAT_CPU_FREEZE_BIT   0
#define STAT_CPU_IRQ_BIT      1
#define STAT_ARB_FREEZE_BIT   6
#define STAT_CSL_MATCH_BIT    7
#define STAT_CSL_FREEZE_BIT   8
#define STAT_CSL_STEP_BIT     9
#define STAT_TRACE_FIN_BIT   10
#define STAT_TRACE_EN_BIT    11
#define STAT_BP0_CNT_TC_BIT  12
#define STAT_BP1_CNT_TC_BIT  13
#define STAT_BP_EVENT_BIT    14


/*
 * Trace Buffer Definition
 *     The Trace Buffer is implemented in the Scratchpad RAM.
 *     The buffer is a data array of 128-bit wide and 512 deep.
 *     The trace can be moved relative to a programmed breakpoint event.
 */
/*
 * CSI Trace Capture Description
 */
#if !defined( _ASMLANGUAGE ) && !defined( __ASSEMBLER__ )
typedef struct
{
    unsigned int CsiSlaveWriteBusAddr;
    unsigned int CsiSlaveWriteBusData;
    unsigned int CsiControl;
    unsigned int CsiMasterReadData;
}
CSI_TRACE;
#endif

/*
 * CsiControl trace definition
 */
#define CSI_SW_DMAACK_FIELD      0
#define   NBITS_CSI_SW_DMAACK      4
#define CSI_SW_MODE_FIELD        4
#define   NBITS_CSI_SW_MODE        4
#define CSI_SW_DMA_CTRL_FIELD    8
#define   NBITS_CSI_SW_DMA_CTRL    4
#define CSI_SW_SIZE_FIELD       12
#define   NBITS_CSI_SW_SIZE        2
#define CSI_SW_RD_EN_BIT        14
#define CSI_SW_WR_EN_BIT        15
#define CSI_SR_BP_CTRL_BIT      16
#define CSI_RD_OR_WR_BIT        18
#define CSI_OR_DMA_ACK_BIT      19
#define CSI_FR_FIELD            20
#define   NBITS_CSI_FR            11
#define CSI_SW_WAIT_NOW_BIT     31

/*
 * CPU Trace Capture Description
 */
#if !defined( _ASMLANGUAGE ) && !defined( __ASSEMBLER__ )

typedef struct
{
    unsigned int CpuAddress;
    unsigned int CpuData;
    unsigned int CpuControl;
    unsigned int Reserved;
}
CPU_TRACE;
#endif

/*
 * CpuControl trace definition
 */
#define ARM_NRW_BIT         0
#define ARM_MAS_FIELD       1
#define   NBITS_ARM_MAS       2
#define ARM_LOCK_BIT        3
#define ARM_SEQ_BIT         4
#define ARM_ABORT_BIT       5
#define ARM_NTRANS_BIT      6
#define ARM_NMREQ_BIT       7
#define ARM_NOPC_BIT        8
#define ARM_DBGACK_BIT     17
#define ARM_CACHELINE_BIT  18
#define ARM_TBIT       19
#define ARM_NEXEC_BIT      20
#define ARM_NM_FIELD       22
#define   NBITS_ARM_NM        5
#define ARM_NWAIT_BIT      27


/*
 ******************************************************************************
 * Protection Unit Registers (PU_BASE)
 ******************************************************************************
 */

/*
 * Protection Unit Control Register definition (read/write)
 */
#define PU_CONTROL_REG         (PU_BASE + 0x04)
#define PU_CONTROL_RESET_VALUE 0x00000000
#define PROT_EN_BIT    0
#define CACHE_EN_BIT   2


/*
 * Cacheable Area Register definition (read/write)
 */
#define PU_CACHEABLE_AREA_REG  (PU_BASE + 0x08)
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
#define PU_PROTECTION_AREA_REG  (PU_BASE + 0x14)
#define AP_0_FIELD      0
#define   NBITS_AP_0      2
#define AP_1_FIELD      2
#define   NBITS_AP_1      2
#define AP_2_FIELD      4
#define   NBITS_AP_2      2
#define AP_3_FIELD      6
#define   NBITS_AP_3      2
#define AP_4_FIELD      8
#define   NBITS_AP_4      2
#define AP_5_FIELD     10
#define   NBITS_AP_5      2
#define AP_6_FIELD     12
#define   NBITS_AP_6      2
#define AP_7_FIELD     14
#define   NBITS_AP_7      2

/*
 * Memory Area Definition Registers definition (read/write)
 *     These regsiters define 8 programmable regions in memory.
 */
#define PU_AREA_DEF0_REG  (PU_BASE + 0x20)
#define PU_AREA_DEF1_REG  (PU_BASE + 0x24)
#define PU_AREA_DEF2_REG  (PU_BASE + 0x28)
#define PU_AREA_DEF3_REG  (PU_BASE + 0x2c)
#define PU_AREA_DEF4_REG  (PU_BASE + 0x30)
#define PU_AREA_DEF5_REG  (PU_BASE + 0x34)
#define PU_AREA_DEF6_REG  (PU_BASE + 0x38)
#define PU_AREA_DEF7_REG  (PU_BASE + 0x3c)
#define AREA_EN_BIT           0
#define AREA_SIZE_FIELD       1
#define   NBITS_AREA_SIZE       5
#define AREA_BASE_ADR_FIELD  12
#define   NBITS_AREA_BASE      20


/*
 * Cache Invalidate Register definition (write only)
 *     Reserved.
 */
#define PU_CACHE_INVALIDATE_REG  (PU_BASE + 0x1c)



/*
 **********************************************************
 * Debug Unit Definition
 **********************************************************
 */

/*
 * Triscend A7 JTAG Instructions
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
 * Triscend A7 JTAG IDs
 */
#define JTAG_TA7S20     0x1843d2ff
#define JTAG_TA7S20_AB  0x1f0f0f0f


 /*
    * Triscend A7 JTAG Scan Chains
    *     The DEBUG chain is scanned with the msb first.
    *     All other chains are scanned with the lsb first.
  */
#define SCAN_TEST_CHAIN        0
#define DEBUG_CHAIN            1
#define ICEBREAKER_CHAIN       2
#define BOUNDARY_SCAN_CHAIN    3	// for Triscend internal use only
#define INTERNAL_SCAN_CHAIN    5	// for Triscend internal use only
#define BUSTAT_CHAIN           6
#define BUSCON_CHAIN           7
#define MEMORY_RW_CHAIN        9
#define MEMORY_RW_INCR_CHAIN  10
#define MEMORY_FILL_CHAIN     11
#define CRC_CHANNEL_CHAIN     12

/*
 * Bitlengths of Triscend A7 JTAG registers/chains
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
#define BUSCON_CPU_RESET_BIT        0
#define BUSCON_CPU_FREEZE_BIT       1
#define BUSCON_CPU_INT_BIT          2
#define BUSCON_FORCE_CFGRST_BIT     3
#define BUSCON_FORCE_NOCFGRST_BIT   4
#define BUSCON_OSC_BREAK_BIT        5
#define BUSCON_FORCE_BRST_BIT       6
#define BUSCON_FORCE_NOBRST_BIT     7
#define BUSCON_JTAG_BP_EVT_BIT      8

/*
 * BUSTAT chain bit definition
 */
#define BUSTAT_CPU_EXECUTING_BIT    0
#define BUSTAT_DBG_ACK_BIT          1
#define BUSTAT_BP_EVT_BIT           2
#define BUSTAT_CPU_IN_RESET_BIT     3
#define BUSTAT_BRST_STATUS_BIT      4
#define BUSTAT_BCLK_STATUS_BIT      5
#define BUSTAT_CFGRST_STATUS_BIT    6
#define BUSTAT_END_OF_TRACE_BIT     7
#define BUSTAT_CPU_RESET_BIT        8
#define BUSTAT_CPU_FREEZE_BIT       9
#define BUSTAT_CPU_INT_BIT         10
#define BUSTAT_FORCE_CFGRST_BIT    11
#define BUSTAT_FORCE_NOCFGRST_BIT  12
#define BUSTAT_OSC_BREAK_BIT       13
#define BUSTAT_FORCE_BRST_BIT      14
#define BUSTAT_FORCE_NOBRST_BIT    15
#define BUSTAT_JTAG_BP_EVT_BIT     16


#endif /* _TRISCEND_A7_H */
