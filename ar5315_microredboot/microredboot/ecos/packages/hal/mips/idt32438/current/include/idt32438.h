/* idt79eb438.h - Integrated Device Technologies IDT79EB438 CPU Board header */

/* Copyright 1984-2002 Wind River Systems, Inc. */
/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
 
01a,14Oct02,krao  New file from IDT for IDT79EB438 Board.

*/

/*
DESCRIPTION

This file contains constants for initialization and configuration of
vxWorks on the IDT79EB438 board. The Evaluation Board Manuals and
IDT79RC32438 User Reference Manual contain full descriptions of the
following registers and their uses.

The register offsets and bit masks for the CPU are defined in a
separate file (rc32438.h) so that they can be shared with other BSPs.
This file contains constants that are specific to the board and BSP.

*/

#ifndef __INCidt79eb438h
#define __INCidt79eb438h

#ifdef __cplusplus
extern "C" {
#endif   /* __cplusplus */

/* includes */

//#include "rc32438.h"


/*defines*/

 

/* Rc32438 CPU Register Settings */


#define BUS                     0           /* no PCI, VME, or MULTI BUS      */


#define CPU_CLOCK_RATE          198000000   /* CPU clock crystal rate (Hertz) */
#define ERR_CNTL_STATUS         0xB8030030  /* Register to disable watchdog   */
#define ERR_CNTL_VALUE          0x00000000  /* Mask to disable watchdog       */
#define SYS_WB_FLUSH                        /* Enable calls to sysWbFlush ()  */
 
/* Rc32438 Device Controller Settings */
 
#define DEV_CTL_BASE            0xB8010000  /* Device controller Base         */

#define DEV0BASE_OFF            0x00        /* Device 0 Base offset           */
#define DEV0MASK_OFF            0x04        /* Device 0 Mask offset           */
#define DEV0C_OFF               0x08        /* Device 0 Control offset        */
#define DEV0TC_OFF              0x0C        /* Device 0 Timing Conf  offset   */

#define DEV1BASE_OFF            0x10        /* Device 1 Base offset           */
#define DEV1MASK_OFF            0x14        /* Device 1 Mask offset           */
#define DEV1C_OFF               0x18        /* Device 1 Control offset        */
#define DEV1TC_OFF              0x1C        /* Device 1 Timing Conf offset    */

#define DEV2BASE_OFF            0x20        /* Device 2 Base offset           */
#define DEV2MASK_OFF            0x24        /* Device 2 Mask offset           */
#define DEV2C_OFF               0x28        /* Device 2 Control offset        */
#define DEV2TC_OFF              0x2C        /* Device 2 Timing Conf offset    */

#define DEV3BASE_OFF            0x30        /* Device 3 Base offset           */
#define DEV3MASK_OFF            0x34        /* Device 3 Mask offset           */
#define DEV3C_OFF               0x38        /* Device 3 Control offset        */
#define DEV3TC_OFF              0x3C        /* Device 3 Timing Conf offset    */


#undef EIGHT_BIT_EPROM                     /* Select default Boot device CS0 */

/*Various chip select values */

#ifdef EIGHT_BIT_EPROM                      /* If boot device is 8 bit eprom  */  
	<<<BOMB>>>

#define MCR_CS0_BS              0x1FC00000  /* Device 0 Base Value            */
#define MCR_CS0_MASK            0xFFE00000  /* Device 0 Mask Value            */
#define MCR_CS0_CTRL            0x06183324  /* Device 0 Control Value         */
#define MCR_CS0_TC              0x00000922  /* Device 0 Timing Conf Value     */

#define MCR_CS1_BS              0x08000000  /* Device 1 Base Value            */
#define MCR_CS1_MASK            0xFFC00000  /* Device 1 Mask Value            */
#define MCR_CS1_CTRL            0x06183325  /* Device 1 Control Value         */
#define MCR_CS1_TC              0x00000922  /* Device 1 Timing Conf Value     */

#else 

/* If boot device is 16 bit flash */

#define MCR_CS0_BS              0x1F800000  /* Device 0 Base Value            */
#define MCR_CS0_MASK            0xFF800000  /* Device 0 Mask Value            */
#define MCR_CS0_CTRL            0x06183325  /* Device 0 Control Value         */
#define MCR_CS0_TC              0x00000922  /* Device 0 Timing Conf Value     */

#define MCR_CS1_BS              0x08000000  /* Device 1 Base Value            */
#define MCR_CS1_MASK            0xFFE00000  /* Device 1 Mask Value            */
#define MCR_CS1_CTRL            0x06183324  /* Device 1 Control  Value        */
#define MCR_CS1_TC              0x00000922  /* Device 1 Timing Conf Value     */

#endif

/* Eight bit port setting for CS2 */

#define MCR_CS2_BS              0x0C000000  /* Device 2 Base Value            */
#define MCR_CS2_MASK            0xFFF00000  /* Device 2 Mask Value            */
#define MCR_CS2_CTRL            0x06183324  /* Device 2 Control Value         */
#define MCR_CS2_TC              0x00000922  /* Device 2 Timing Conf Value     */

/* Sixteen bit port setting for CS3 */

#define MCR_CS3_BS              0x10000000  /* Device 3 Base Value            */
#define MCR_CS3_MASK            0xFC000000  /* Device 3 Mask Value            */
#define MCR_CS3_CTRL            0x06183325  /* Device 3 Control Value         */
#define MCR_CS3_TC              0x00000922  /* Device 3 Timing Conf Value     */
 
#define TIMER_BASE_ADDR         0xB8028000  /* Timer Base address             */
#define DISABLE_TIMER           0x0         /* Disable Timer value            */
#define ENABLE_TIMER            0x1         /* Enable Timer value             */
#define WTC_CTL_REG             0xB803003C  /* Watchdog Control Register      */
#define BT_TIMER_COMPARE        0xB8010064  /* Bus Transaction Timer compare  */
#define EB438_REG_BASE          0xB0000000  /* 32438 Register Base - for PCI  */
 
/* GPIO Registers */

#define GPIO_BASE_REG           0xB8048000              /* GPIO Base          */
#define GPIO_FUNC_REG           (GPIO_BASE_REG + 0x00)  /* GPIO Function      */
#define GPIO_CFG_REG            (GPIO_BASE_REG + 0x04)  /* GPIO Config        */
#define GPIO_DATA_REG           (GPIO_BASE_REG + 0x08)  /* GPIO Data          */
#define GPIO_INT_LVL            (GPIO_BASE_REG + 0x0C)  /* GPIO Interrupt lvl */
#define GPIO_INTR_STAT          (GPIO_BASE_REG + 0x10)  /* GPIO Interrupt Stat*/
#define GPIO_NMI_ENBL           (GPIO_BASE_REG + 0x14)  /* GPIO NMI Enable    */
#define IDT79_NUM_GPIO          8                       /* hack for now FIXME */ 

/* GPIO Status Mask for PCI */

#define GPIO_PCI_INTR_MASK       0x09000000


/* task default status register */

#define INITIAL_SR              (SR_CU0 | SR_BEV)
#define INITIAL_CFG             0x0002E880

 
#define INT_LVL_SR_IMASK        ( INT_LVL_GRP6 | INT_LVL_GRP5 | INT_LVL_GRP4 | \
                                INT_LVL_GRP3 |INT_LVL_GRP2 | INT_LVL_SW0 |     \
                                INT_LVL_SW1)

#define RC32438_SR              (SR_CU0 | INT_LVL_SR_IMASK |\
                                INT_LVL_TIMER | SR_IE)
/* interrupt priority */

#define INT_PRIO_MSB            TRUE        /* interrupt priority msb highest */


/* interrupt levels */

#define INT_LVL_TIMER           SR_IBIT8    /* RC32438 timer (fixed)          */
#define INT_LVL_GRP6            SR_IBIT7    /* IORQ 4                         */
#define INT_LVL_GRP5            SR_IBIT6    /* IORQ 3                         */
#define INT_LVL_GRP4            SR_IBIT5    /* IORQ 2                         */
#define INT_LVL_GRP3            SR_IBIT4    /* IORQ 1                         */
#define INT_LVL_GRP2            SR_IBIT3    /* IORQ 0                         */
#define INT_LVL_SW1             SR_IBIT2    /* sw interrupt 1 (fixed)         */
#define INT_LVL_SW0             SR_IBIT1    /* sw interrupt 0 (fixed)         */


/* interrupt indexes */

#define INT_INDX_TIMER          7           /* RC32438 timer (fixed)          */
#define INT_INDX_IORQ4          6           /* IORQ 4                         */
#define INT_INDX_IORQ3          5           /* IORQ 3                         */
#define INT_INDX_IORQ2          4           /* IORQ 2                         */
#define INT_INDX_IORQ1          3           /* IORQ 1                         */
#define INT_INDX_IORQ0          2           /* IORQ 0                         */
#define INT_INDX_SW1            1           /* sw interrupt 1                 */
#define INT_INDX_SW0            0           /* sw interrupt 0                 */
 

/* interrupt vectors */

#define INT_NUM_BASE            64 /* Base for all BSP-defined int nums */

#define IV_TIMER_VEC            72          /* RC32438 timer (fixed)          */
#define IV_IORQ2_VEC            71          /*               - IORQ 4         */
#define COM1_INT_NUM            70          /* Serial port 1                  */
#define COM2_INT_NUM            69          /* Serial port 2                  */
#define ETH_OVR_NUM0            68          /* Ethernet over run              */
#define ETH_OVR_NUM1            60          /* Ethernet over run              */

#define INT_NUM_WLAN_BASE       73
#define INT_NUM_WLAN_0          73
#define INT_NUM_WLAN_1          74
                                            /*               - IORQ 2         */
#define IV_IORQ1_VEC            66          /*               - IORQ 1         */
#define IV_IORQ0_VEC            65          /* Timer0        - IORQ 0         */
#define TMR0_INT_NUM		64

#define INT_VEC_GRP2            IV_IORQ0_VEC
#define INT_VEC_GRP3            IV_IORQ1_VEC
#define INT_VEC_GRP4            IV_IORQ2_VEC
#define INT_VEC_GRP5            IV_IORQ3_VEC
#define INT_VEC_GRP7            IV_TIMER_VEC

#define INT_NUM_IRQ0            (INT_NUM_BASE + 2)

/* misc hardware defines */

#if CPU==RC32438
    #define MAX_PHYS_MEMORY     0x04000000  /* 64 MByte */
#endif

/* Miscellaneous */

#ifdef _ASMLANGUAGE
    #define CAST(c)
#else
    #define CAST(c) (c)
#endif


#define N_SIO_CHANNELS          1           /* Number of serial I/O channels */
#define N_UART_CHANNELS         1           /* Number of serial I/O channels */

/* Baud rate Oscillator clock */

#define NS16550_XTAL_FREQ       99000000     /* UART input frequencey */
#define UART_DEFAULT_BAUD_RATE  9600               /* default baud rate     */

/* The UARTs are inherently little endian devices. This offset is added to
 * the UART base addresses. It is necessary because the code is big-endian.
 * If this BSP were to support little endian code, this #define would
 * have to be conditional on the endiannes of the code.
 */

#define UART_ENDIAN_OFFSET      3

/* Rc32438 Internal Uart definitions */


#define UART0_BASE              0xB8050000      /* Base address of UART0      */
#define UART1_BASE              0xB8050020      /* Base address of UART1      */
#define UART_REG_ADDR_INTERVAL  4
#define UART_DEFAULT_BAUD_RATE  9600


/* PIO definition for Internal Uart   */

#define PIO_FUNCSEL_REG         0xB8048000
#define PIO_FUNCSEL_VAL         0x00f00303

/* Rc32438 Interrupt controller settings for Uart */

#define UART0_GENERAL_INTR_MASK 0x00000001
#define UART1_GENERAL_INTR_MASK 0x00000008


/* Rc32438 Timer0(used as Auxiliary clock)interrupts */


#define AUX_TIMER_INTR_PEND     0xB8038000      /* Aux timer interrupt pend   */
#define AUX_TIMER_INTR_TEST     0xB8038004      /* Aux timer interrupt test   */
#define AUX_TIMER_INTR_MASK     0xB8038008      /* Aux timer interrupt mask   */
#define AUX_TIMER_CNTL_REG      0xB8028014      /* Aux timer control reg      */
#define AUX_TIMER_CNT_REG       0xB802800c      /* Aux timer count reg        */
#define AUX_TIMER_CMP_REG       0xB8028010      /* Aux timer compare reg      */

#define AUX_CLOCK_FREQ          NS16550_XTAL_FREQ  

/* defines used for Timer Interrupts */ 

#define CNTR_TMR0_MASK          0x01            /* Counter timer 0 mask       */
#define CNTR_TMR1_MASK          0x02            /* Counter timer 1 mask       */
#define CNTR_TMR2_MASK          0x04            /* Counter timer 2 mask       */
#define REFRESH_TIMER_MASK      0x08            /* Refresh timer mask         */
#define WDOG_TMR_TO_MASK        0x10            /* Watchdog timer mask        */
#define UNDEC_CPU_WR_MASK       0x20            /* Undecoded CPU write mask   */
#define CNTR_TMR0_ENBL          0x01            /* Counter timer 0 enable     */
#define INTR_MASK_TIMER0        0x00000001      /* Interrupt mask timer 0     */

/* counter timer 0 registers */

#define CNTR_TMR0_COUNTREG      0xB8028000      /* Cntr timer 0 count reg     */
#define CNTR_TMR0_COMPREG       0xB8028004      /* Cntr timer 0 compare reg   */
#define CNTR_TMR0_CTRLREG       0xB8028008      /* Cntr timer 0 control reg   */

#define TIMEOUT_COUNT           0x0000FFFF
 
/* RTC-NVRAM Base Address */

#define NVRAM_BASE_ADDR         0xAC080000      /* NVRAM Base address         */
 
#define MAX_DMA_INTRS           13              /* Max num of DMA interrupt   */
#define ETH_INTFC0              0xB8058000      /* Ethernet interface 0 Base  */
#define ETH_INTFC1              0xB8060000      /* Ethernet interface 1 Base  */
#define INTR_MASK_ETH1_OVR      0x00008000      /* Ethernet 1 mask overrun    */
#define INTR_MASK_ETH0_OVR      0x00001000      /* Ethernet 0 mask overrun    */


/* I2C Interface related defines */


#define PSL_75CLK_100KBPS       94              /* Prescalar clock value      */
#define I2C_MASTER_INT_MASK     0xF             /* I2C Master interrupt mask  */
#define I2C_SLAVE_INT_MASK      0x7F            /* I2C Slave interrupt mask   */
#define I2C_24LC64_CNTL_BYTE    0xA0            /* I2C Ctrl byte for eeprom   */
#define I2C_24LC64_RD_EN        0x01            /* I2C Read enable            */
#define I2C_MEM_SIZE            8192            /* 8K Bytes of I2C Space      */
#define I2C_READ_WRITE_ADRS     0x0             /* beginning address of I2C   */


/* "NVRAM" related defines  (I2C serial EEPROM is used for NVRAM) */

#define NV_RAM_SIZE             512  /* 512 bytes in I2C are used for Nvram */
#define NV_RAM_READ_WRITE_ADRS  0x0  /* beginning address of I2C for "NVRAM" */
 
/* Mask Generation Macro */

#ifndef MSK
#define MSK(n)                  ((1 << (n)) - 1)
#endif

/* REFRESH Counter registers */

#define RCOUNT                  0xB8028024

/* Cache related definitions */
 
#define IDT32438_CONFIG1_DA_SHF   7
#define IDT32438_CONFIG1_DA_MSK   (MSK(3) << IDT32438_CONFIG1_DA_SHF)
#define IDT32438_CONFIG1_DL_SHF   10
#define IDT32438_CONFIG1_DL_MSK   (MSK(3) << IDT32438_CONFIG1_DL_SHF)
#define IDT32438_CONFIG1_DS_SHF   13
#define IDT32438_CONFIG1_DS_MSK   (MSK(3) << IDT32438_CONFIG1_DS_SHF)
#define IDT32438_CONFIG1_IA_SHF   16
#define IDT32438_CONFIG1_IA_MSK   (MSK(3) << IDT32438_CONFIG1_IA_SHF)
#define IDT32438_CONFIG1_IL_SHF   19
#define IDT32438_CONFIG1_IL_MSK   (MSK(3) << IDT32438_CONFIG1_IL_SHF)
#define IDT32438_CONFIG1_IS_SHF   22
#define IDT32438_CONFIG1_IS_MSK   (MSK(3) << IDT32438_CONFIG1_IS_SHF)

/* DDR Registers */

#define DDRC_VAL_AT_INIT        0x232A4980      /* DDR Control at init        */
/*#define DDRC_VAL_NORMAL         0xa32a4980*/      /* DDR Control value normal   */
#define DDRC_VAL_NORMAL         0x82984940      /* DDR Control value normal   */

/*DDR Base and Mask registers */

#define DDR_BASE                0xB8018000      /* DDR Base                   */
#define DDR0_BASE_VAL           0x00000000      /* DDR0 Base value            */
#define DDR0_MASK_VAL           0xFC000000      /* DDR0 Mask value            */
#define DDR1_BASE_VAL           0x04000000      /* DDR1 Base value            */
#define DDR1_MASK_VAL           0x00000000      /* DDR1 Mask value            */
#define DDR0_ABASE_VAL          0x08000000      /* DDR0 Alternate Base value  */
#define DDR0_AMASK_VAL          0x00000000      /* DDR0 Alternate Mask value  */

 

#define  DDR_REF_CMP_FAST       0x0100          /* DDR fast Refresh compare   */
#define  DDR_REF_CMP_VAL        0x0080          /* DDR Refresh Normal         */
#define  DDR_REF_CONTL_VAL      0x0001          /* Enable counter             */
#define  DDR_REF_COUNT_VAL      0x0000          /* DDR Refresh count value    */


/* Hidden register for RC32438 DDR */

#define DDRD_LLC_REG		0xB801C004
#define DDRD_LLC_VAL		0x00020183

/* DDR device specific  for MT46V16M16 commands */

#define DDR_CUST_NOP            0x003F          /* DDR Custom NOP             */
#define DDR_CUST_PRECHARGE      0x0033          /* DDR Custom Precharge       */
#define DDR_CUST_REFRESH        0x0027          /* DDR Custom refresh         */
#define DDR_LD_MODE_REG         0x0023          /* DDR load mode register     */
#define DDR_LD_EMODE_REG        0x0063          /* DDR load emode register    */
#define DDR_EMODE_VAL           0x0000          /* DDR Emode value            */
#define DDR_DLL_RES_MODE_VAL    0x0584          /* DDR DLL Reset mode value   */
#define DDR_DLL_MODE_VAL        0x0184          /* DDR DLL Mode value         */
#define DELAY_200USEC           25000           /* Delay value                */
#define DATA_PATTERN            0xAA556996      /* Data pattern               */

/* PCI Specific defines */

#define PCI_MAX_LATENCY         0xFFF           /* Max PCI latency            */

#define PCIM_SHFT               0x6             /* PCI Mode (PCIM) shift bit  */
#define PCIM_BIT_LEN            0x7             /* PCIM Bit Length            */
#define PCIM_H_EA               0x3             /* PCIM Host with Ext Arbiter */
#define PCIM_H_IA_FIX           0x4             /* ",Int Arb,fix priority     */
#define PCIM_H_IA_RR            0x5             /* ",Int Arb,round robin      */
#define PCI_ADDR_START          0x40000000      /* PCI Start address          */
#define CPUTOPCI_MEM_WIN        0x08000000      /* 32 MB Window               */
#define CPUTOPCI_IO_WIN         0x00100000      /* 1 MB I/O Window            */
#define SIZE_128MB              0x1B
#define SIZE_32MB               0x19            /* 2 to the power of 19 =32MB */
#define SIZE_1MB                0x14            /* 2 to the power of 14 =1MB  */
#define SIZE_256                0x08            /* 2 to the power of 08 =256B */

/* PCI Host Bridge Configuration Registers */

#define IDT32438_CONFIG0_ADDR   0x80000000      /* PCI Config 0 address       */
#define IDT32438_CONFIG1_ADDR   0x80000004      /* PCI Config 1 address       */
#define IDT32438_CONFIG2_ADDR   0x80000008      /* PCI Config 2 address       */
#define IDT32438_CONFIG3_ADDR   0x8000000C      /* PCI Config 3 address       */
#define IDT32438_CONFIG4_ADDR   0x80000010      /* PCI Config 4 address       */
#define IDT32438_CONFIG5_ADDR   0x80000014      /* PCI Config 5 address       */
#define IDT32438_CONFIG6_ADDR   0x80000018      /* PCI Config 6 address       */
#define IDT32438_CONFIG7_ADDR   0x8000001C      /* PCI Config 7 address       */
#define IDT32438_CONFIG8_ADDR   0x80000020      /* PCI Config 8 address       */
#define IDT32438_CONFIG9_ADDR   0x80000024      /* PCI Config 9 address       */
#define IDT32438_CONFIG10_ADDR  0x80000028      /* PCI Config 10 address      */
#define IDT32438_CONFIG11_ADDR  0x8000002C      /* PCI Config 11 address      */
#define IDT32438_CONFIG12_ADDR  0x80000030      /* PCI Config 12 address      */
#define IDT32438_CONFIG13_ADDR  0x80000034      /* PCI Config 13 address      */
#define IDT32438_CONFIG14_ADDR  0x80000038      /* PCI Config 14 address      */
#define IDT32438_CONFIG15_ADDR  0x8000003C      /* PCI Config 15 address      */
#define IDT32438_CONFIG16_ADDR  0x80000040      /* PCI Config 16 address      */
#define IDT32438_CONFIG17_ADDR  0x80000044      /* PCI Config 17 address      */
#define IDT32438_CONFIG18_ADDR  0x80000048      /* PCI Config 18 address      */
#define IDT32438_CONFIG19_ADDR  0x8000004C      /* PCI Config 19 address      */
#define IDT32438_CONFIG20_ADDR  0x80000050      /* PCI Config 20 address      */
#define IDT32438_CONFIG21_ADDR  0x80000054      /* PCI Config 21 address      */
#define IDT32438_CONFIG22_ADDR  0x80000058      /* PCI Config 22 address      */
#define IDT32438_CONFIG23_ADDR  0x8000005C      /* PCI Config 23 address      */
#define IDT32438_CONFIG24_ADDR  0x80000060      /* PCI Config 24 address      */
#define IDT32438_CONFIG25_ADDR  0x80000064      /* PCI Config 25 address      */

/* PCI Host Bridge Configuration Header values */

  
#define IDT32438_CMD            (COMMAND_IO  | \
                                 COMMAND_MEM | \
                                 COMMAND_BM  | \
                                 COMMAND_MWI | \
                                 COMMAND_PEN | \
                                 COMMAND_SEN)
 
#define IDT32438_STAT           (STATUS_MDPE | \
                                 STATUS_STA  | \
                                 STATUS_RTA  | \
                                 STATUS_RMA  | \
                                 STATUS_SSE  | \
                                 STATUS_PE)

#define IDT32438_DEV_VEND       0x0207111d      /* Device ID & Vendor ID      */
#define IDT32438_PCI_CONFIG0    IDT32438_DEV_VEND   
#define IDT32438_CNFG0          IDT32438_DEV_VEND 
                         
#define IDT32438_CNFG1          ((IDT32438_STAT<<16)|IDT32438_CMD)

#define IDT32438_REVID          0
#define IDT32438_CLASS_CODE     0
#define IDT32438_CNFG2          ((IDT32438_CLASS_CODE<<8) | \
                                IDT32438_REVID)


#define IDT32438_MASTER_LAT     0x3c
#define IDT32438_HEADER_TYPE    0
#define IDT32438_BIST           0
#define IDT32438_CACHE_LINE_SIZE   4

#define IDT32438_CNFG3          ((IDT32438_BIST<<24) | \
                                (IDT32438_HEADER_TYPE<<16) | \
                                (IDT32438_MASTER_LAT<<8) | \
                                IDT32438_CACHE_LINE_SIZE )

#define IDT32438_BAR0           0x00000008        /* 128 MB Memory            */
#define IDT32438_BAR1           0x08000001        /* 2 MB IO Window           */
#define IDT32438_BAR2           0x08200001        /* 2 MB IO window for 32438 */
                                                  /* internal Registers       */
#define IDT32438_BAR3           0x0               /* Spare 128 MB Memory      */
#define IDT32438_CNFG4          IDT32438_BAR0
#define IDT32438_CNFG5          IDT32438_BAR1
#define IDT32438_CNFG6          IDT32438_BAR2
#define IDT32438_CNFG7          IDT32438_BAR3


#define IDT32438_SUBSYSTEM_ID   0
#define IDT32438_CNFG8          0
#define IDT32438_CNFG9          0
#define IDT32438_CNFG10         0
#define IDT32438_SUBSYS_VENDOR_ID  0
#define IDT32438_CNFG11         ((IDT32438_SUBSYS_VENDOR_ID<<16) | \
                                IDT32438_SUBSYSTEM_ID)
                                
#define IDT32438_INT_LINE       1
#define IDT32438_INT_PIN        1
#define IDT32438_MIN_GNT        8
#define IDT32438_MAX_LAT        0x38
#define IDT32438_CNFG12         0
#define IDT32438_CNFG13         0
#define IDT32438_CNFG14         0
#define IDT32438_CNFG15         ((IDT32438_MAX_LAT<<24) | \
                                (IDT32438_MIN_GNT<<16) | \
                                (IDT32438_INT_PIN<<8)  | \
                                IDT32438_INT_LINE)
#define IDT32438_RETRY_LIMIT    0x80
#define IDT32438_TRDY_LIMIT     0x80
#define IDT32438_CNFG16         ((IDT32438_RETRY_LIMIT<<8) | \
                                IDT32438_TRDY_LIMIT)
#define PCI_PBAXC_R             0x0
#define PCI_PBAXC_RL            0x1
#define PCI_PBAXC_RM            0x2
#define SIZE_128MB              0x1B
#define SIZE_2MB                0x15
#define SIZE_SHFT               2
#define PBAXC_MR_RL             1

#define IDT32438_PBA0C          (PBAXC_MRL | \
                                PBAXC_MR(PBAXC_MR_RL) | \
                                PBAXC_PP | \
                                PBAXC_SIZE(SIZE_128MB)| \
                                PBAXC_P | \
                                PBAXC_SB)
#define IDT32438_CNFG17         IDT32438_PBA0C
#define IDT32438_PBA0M          0x0
#define IDT32438_CNFG18         IDT32438_PBA0M
#define IDT32438_PBA1C          (PBAXC_SIZE(SIZE_1MB) | \
                                PBAXC_MSI| \
                                PBAXC_SB)
                        
#define IDT32438_CNFG19         IDT32438_PBA1C
#define IDT32438_PBA1M          0x0
#define IDT32438_CNFG20         IDT32438_PBA1M
#define IDT32438_PBA2C          (PBAXC_SIZE(SIZE_2MB) | \
                                PBAXC_MSI | \
                                PBAXC_SB)
                         
#define IDT32438_CNFG21         IDT32438_PBA2C
#define IDT32438_PBA2M          0x18000000
#define IDT32438_CNFG22         IDT32438_PBA2M
#define IDT32438_PBA3C          0
#define IDT32438_CNFG23         IDT32438_PBA3C
#define IDT32438_PBA3M          0
#define IDT32438_CNFG24         IDT32438_PBA3M 

/* define an unsigned array for the PCI registers */

#if !defined(_ASMLANGUAGE)

#endif
 

#define PCITC_DTIMER_VAL        8
#define PCITC_RTIMER_VAL        0x10

/*
 * BusErrCntReg is used to disable/Enable BusError thrown on PCI bus
 * on scanning
 */
 

/* RC32438 PCI host bridge register addresses */

#define EB438_PCI_BASE          0xB8080000                /* PCI BASE ADDRESS */
#define EB438_PCI_MEM_BAR0      (EB438_PCI_BASE + 0x1C)
#define EB438_PCI_MEM_BAR1      (EB438_PCI_BASE + 0x28)
#define EB438_PCI_MEM_BAR2      (EB438_PCI_BASE + 0x34)
#define EB438_PCI_MEM_BAR3      (EB438_PCI_BASE + 0x40)
#define EB438_PCI_CONFIG_ADDR   (EB438_PCI_BASE + 0x000C)
#define EB438_PCI_CONFIG_DATA   (EB438_PCI_BASE + 0x0010)

/*
 * PCI memory constants: Memory area 1 and 2 are the same size -
 * (twice the PCI_TLB_PAGE_SIZE). The definition of
 * CPU_TO_PCI_MEM_SIZE is coupled with the TLB setup routine
 * sysLib.c/sysTlbInit(), in that it assumes that 2 pages of size
 * PCI_TLB_PAGE_SIZE are set up in the TLB for each PCI memory space.
 */
 
#define CPU_TO_PCI_MEM_BASE1    0xE0000000
#define CPU_TO_PCI_MEM_SIZE1    (2*PCI_TLB_PAGE_SIZE)
#define CPU_TO_PCI_MEM_BASE2    0xF0000000
#define CPU_TO_PCI_MEM_SIZE2    (2*PCI_TLB_PAGE_SIZE)
#define CPU_TO_PCI_MEM_BASE3    0xB8C00000
#define CPU_TO_PCI_MEM_SIZE3    0x00400000
#define CPU_TO_PCI_IO_BASE      0xB8800000
#define CPU_TO_PCI_IO_SIZE      0x00100000      /* Total IO space             */
#define CPU_TO_PCI_IO16_SIZE    0x00010000      /* 16-bit Portion of IO space */
#define PCI_TO_CPU_MEM_BASE     0x00000000
#define PCI_TO_CPU_IO_BASE      0x00000000
#define PCI_BRIDGE_ENABLE       0x00000001      /* Rdy/internal arb/fixed pri */
#define PCI_BRIDGE_DISABLE      0x00000004      /* NOT Ready                  */

 

/* TLB attributes for PCI transactions */

#define PCI_MMU_PAGEMASK        0x00000FFF
#define MMU_PAGE_UNCACHED       0x00000010
#define MMU_PAGE_DIRTY          0x00000004
#define MMU_PAGE_VALID          0x00000002
#define MMU_PAGE_GLOBAL         0x00000001
#define PCI_MMU_PAGEATTRIB      (MMU_PAGE_UNCACHED|MMU_PAGE_DIRTY|\
                                 MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define PCI_MEMORY_SPACE1_VIRT  0xE0000000      /* Used for non-prefet  mem   */
#define PCI_MEMORY_SPACE1_PHYS  0x40000000
#define PCI_MEMORY_SPACE2_VIRT  0xF0000000      /* Used for prefetable memory */
#define PCI_MEMORY_SPACE2_PHYS  0x60000000
#define PCI_IO_SPACE1_PHYS	0x18800000
#define PCI_IO_SPACE1_VIRT	0xb8800000
#define PCI_TLB_PAGE_SIZE       0x01000000
#define TLB_HI_MASK             0xFFFFE000
#define TLB_LO_MASK             0x3FFFFFFF
#define PAGEMASK_SHIFT          13
#define TLB_LO_SHIFT            6
                                 


#if 0
/* 
 * The current PB32 board doesn't have any LEDs;
 * but we'll keep these handy definitions around
 * just in case we end up debugging on a base IDT
 * board, or if PB32 ever gets LEDs.
 */
/* LED addresses in IDT79EB438 Board. The LED Can accomodate 4 characters */

#define EB438_ALPHAN_ADRS       (CAST(volatile UCHAR *) 0xAC040000 )
#define EB438_ALPHAN_CHAR0      (CAST(volatile UINT *)(EB438_ALPHAN_ADRS + 0x3))
#define EB438_ALPHAN_CHAR1      (CAST(volatile UINT *)(EB438_ALPHAN_ADRS + 0x2))
#define EB438_ALPHAN_CHAR2      (CAST(volatile UINT *)(EB438_ALPHAN_ADRS + 0x1))
#define EB438_ALPHAN_CHAR3      (CAST(volatile UINT *)(EB438_ALPHAN_ADRS + 0x0))
#endif

/* Flash memory is mapped to KSEG 1 */
#define FLASH_CTL              0x1FC00000

#if 0
#if !defined(_ASMLANGUAGE)
extern unsigned long rom_base_adrs;
#endif
#endif

/*
 * Access macros for IDT KEG1 memory.
 */
#define sysRegRead(phys)        \
        (*(volatile unsigned int *)PHYS_TO_K1(phys))
#define sysRegWrite(phys,val) ((*(volatile unsigned int *)PHYS_TO_K1(phys)) = (val))

#define NFLC            3               /* number of flash config segments */
#define FLC_BOOTLINE    0
#define FLC_BOARDDATA   1
#define FLC_RADIOCFG    2


#define DELTA(a,b)      (((unsigned int)(a)) - ((unsigned int)(b)))

/* config reg value to be written at cold reset */

#define KSEG0_CACHE_MODE        3               /* cached                     */

#endif  /* __INCidteb438h */
