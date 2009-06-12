#ifndef CYGONCE_HAL_IOP310_H
#define CYGONCE_HAL_IOP310_H

/*=============================================================================
//
//      hal_iop310.h
//
//      HAL Description of IOP310 (IQ80200/IQ80310) control registers, etc.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter, gthomas
// Date:         2000-10-10
// Purpose:      Intel IOP310 hardware description
// Description:
// Usage:        #include <cyg/hal/hal_iop310.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/hal_xscale.h>

#ifndef __ASSEMBLER__
// Platform specific initializations
extern void hal_plf_hardware_init(void);
#endif

// Serial ports
#define IOP310_SERIAL_PORT_A 0xFE800000
#define IOP310_SERIAL_PORT_B 0xFE810000

#define DCACHE_FLUSH_AREA 0xc0000000

// Addresses of the left and right 7-segment display
#define DISPLAY_LEFT    0xFE840000
#define DISPLAY_RIGHT   0xFE850000

// 7-segment encodings for the hex display
#define DISPLAY_0       0xc0
#define DISPLAY_1       0xf9
#define DISPLAY_2       0xa4
#define DISPLAY_3       0xb0
#define DISPLAY_4       0x99
#define DISPLAY_5       0x92
#define DISPLAY_6       0x82
#define DISPLAY_7       0xF8
#define DISPLAY_8       0x80
#define DISPLAY_9       0x90
#define DISPLAY_A       0x88
#define DISPLAY_B       0x83
#define DISPLAY_C       0xa7
#define DISPLAY_D       0xa1
#define DISPLAY_E       0x86
#define DISPLAY_F       0x8e

#ifndef iop310_is_host
/* Backplane Detect Register */
#define BACKPLANE_DET_REG       ((volatile unsigned char *)0xfe870000)
#  define BP_HOST_BIT           0x1
#define iop310_is_host()        (*BACKPLANE_DET_REG & BP_HOST_BIT)
#endif

// XINT3 mask register
#define X3ISR_REG   ((volatile unsigned char *)0xfe820000)
#define X3MASK_REG  ((volatile unsigned char *)0xfe860000)
#  define XINT3_TIMER    0x01
#  define XINT3_ETHERNET 0x02
#  define XINT3_UART_1   0x04
#  define XINT3_UART_2   0x08
#  define XINT3_PCI_INTD 0x10

/* PAL-based external timer definitions */
#define TIMER_LA0_REG_ADDR      ((volatile unsigned char *)0xfe880000)
#define TIMER_LA1_REG_ADDR      ((volatile unsigned char *)0xfe890000)
#define TIMER_LA2_REG_ADDR      ((volatile unsigned char *)0xfe8a0000)
#define TIMER_LA3_REG_ADDR      ((volatile unsigned char *)0xfe8b0000)
#define TIMER_ENABLE_REG_ADDR   ((volatile unsigned char *)0xfe8c0000)

#define TIMER_COUNT_MASK                0x5f    /* 6 bits of timer data with the MSB in bit 6 not bit 5 */
#define TIMER_CNT_ENAB                  0x1
#define TIMER_INT_ENAB                  0x2
#define EXT_TIMER_CLK_FREQ              33000000        /* external timer runs at 33 MHz */
#define TICKS_10MSEC                    100                     /* 10msec = 100 ticks/sec */
#define EXT_TIMER_10MSEC_COUNT  (EXT_TIMER_CLK_FREQ / TICKS_10MSEC)
#define TICKS_5MSEC                             200                     /* 5msec = 200 ticks/sec */
#define EXT_TIMER_5MSEC_COUNT   (EXT_TIMER_CLK_FREQ / TICKS_5MSEC)

#define EXT_TIMER_CNT_ENAB()            (*TIMER_ENABLE_REG_ADDR |= TIMER_CNT_ENAB)
#define EXT_TIMER_CNT_DISAB()           (*TIMER_ENABLE_REG_ADDR &= ~TIMER_CNT_ENAB)
#define EXT_TIMER_INT_ENAB()            (*TIMER_ENABLE_REG_ADDR |= TIMER_INT_ENAB)
#define EXT_TIMER_INT_DISAB()           (*TIMER_ENABLE_REG_ADDR &= ~TIMER_INT_ENAB)

// Companion chip MCU registers
#define    MMR_BASE         0x00001500
#define    SDIR_OFF         0x00000000
#define    SDCR_OFF         0x00000004
#define    SDBR_OFF         0x00000008
#define    SBR0_OFF         0x0000000C
#define    SBR1_OFF         0x00000010
#define    ECCR_OFF         0x00000034
#define    FEBR1_OFF        0x00000050
#define    FBSR1_OFF        0x00000058
#define    FWSR0_OFF        0x0000005C
#define    FWSR1_OFF        0x00000060
#define    RFR_OFF          0x00000068

// MCU Register Values
#define    MRS_CAS_LAT_2    0x00000000
#define    MRS_CAS_LAT_3    0x00000001
#define    MRS_PRECHRG      0x00000002       
#define    MRS_NO_OP        0x00000003
#define    MRS_AUTO_RFRSH   0x00000004
#define    MRS_NORM_OP      0x00000006
#define    MRS_NOP_DELAY    0x00004000
#define    SDCR_INIT_VAL    0x00000aa0  // was 0x14
#define    SBR0_INIT_VAL    0x00000008  // 32 Meg Boundary (64 mbit device)
#define    SBR1_INIT_VAL    0x00000008  // 32 Meg Boundary (64 mbit device)
#define    ECCR_INIT_VAL    0x0000000C  // ECC enabled, correction on and no reporting
#define    RFR_INIT_VAL     0x00000600  // Initial Refresh Rate
#define    FBSR1_INIT_VAL   0x00000040  // 8MB Bank Size
#define    FWSR0_INIT_VAL   0x00000001  // 1ws add-data (needed for PP state machine), 0ws recovery
#define    FWSR1_INIT_VAL   0x00000000  // 0ws add-data, 0ws recovery


/**************************
 * I2C Bus Interface Unit *
 **************************/

/* Processor I2C Device ID */
#define I2C_DEVID       0x02  /* I2C slave address to which the unit responds when in slave-receive mode */

/* Timeout limit for SDRAM EEPROM to respond */
#define I2C_TIMOUT      0x1000000  /* bumped this way up...used to be 0x100000*/ 

/* Control Register */
#define ICR_ADDR        0x00001680  /* Address */
#define ICR_START       0x0001  /* 1:send a Start condition to the I2C when in master mode */
#define ICR_STOP        0x0002  /* 1:send a Stop condition after next data byte transferred on I2C bus in master mode */
#define ICR_ACK         0x0004  /* Ack/Nack control: 1:Nack, 0:Ack (negative or positive pulse) */
#define ICR_TRANSFER    0x0008  /* 1:send/receive byte, 0:cleared by I2C unit when done */
#define ICR_ABORT       0x0010  /* 1:I2C sends STOP w/out data permission, 0:ICR bit used only */
#define ICR_SCLENB      0x0020  /* I2C clock output: 1:Enabled, 0:Disabled. ICCR configured before ! */
#define ICR_ENB         0x0040  /* I2C unit: 1:Enabled, 0:Disabled */
#define ICR_GCALL       0x0080  /* General Call: 1:Disabled, 0:Enabled */
#define ICR_IEMPTY      0x0100  /* 1: IDBR Transmit Empty Interrupt Enable */
#define ICR_IFULL       0x0200  /* 1: IDBR Receive Full Interrupt Enable */
#define ICR_IERR        0x0400  /* 1: Bus Error Interrupt Enable */
#define ICR_ISTOP       0x0800  /* 1: Slave Stop Detected Interrupt Enable */
#define ICR_IARB        0x1000  /* 1: Arbitration Loss Detected Interrupt Enable */
#define ICR_ISADDR      0x2000  /* 1: Slave Address Detected Interrupt Enable */
#define ICR_RESET       0x4000  /* 1: I2C unit reset */

/* Status Register */
#define ISR_ADDR        0x00001684  /* Address */
#define ISR_RWMODE      0x0001  /* 1: I2C in master receive = slave transmit mode */
#define ISR_ACK         0x0002  /* 1: I2C received/sent a Nack, 0: Ack */
#define ISR_BUSY        0x0004  /* 1: Processor's I2C unit busy */
#define ISR_BUSBUSY     0x0008  /* 1: I2C bus busy. Processor's I2C unit not involved */
#define ISR_STOP        0x0010  /* 1: Slave Stop detected (when in slave mode: receive or transmit) */
#define ISR_ARB         0x0020  /* 1: Arbitration Loss Detected */
#define ISR_EMPTY       0x0040  /* 1: Transfer finished on I2C bus. If enabled in ICR, interrupt signaled */
#define ISR_FULL        0x0080  /* 1: IDBR received new byte from I2C bus. If ICR, interrupt signaled */
#define ISR_GCALL       0x0100  /* 1: I2C unit received a General Call address */
#define ISR_SADDR       0x0200  /* 1: I2C unit detected a 7-bit address matching the general call or ISAR */
#define ISR_ERROR       0x0400  /* Bit set by unit when a Bus Error detected */

#define ISAR_ADDR       0x00001688  /* Address of the I2C Slave Address Register */
#define IDBR_ADDR       0x0000168C  /* Address of the I2C Data Buffer Register */
#define IDBR_MASK       0x000000ff
#define IDBR_MODE       0x01
#define ICCR_ADDR       0x00001690  /* Address of the I2C Clock Control Register */
#define IBMR_ADDR       0x00001694  /* Address of the I2C Bus Monitor Register */

/* SDRAM configuration */

/* SDRAM bank size values (SPD << 2) */
#define         RAM_0MEG        
#define         RAM_4MEG        4
#define         RAM_8MEG        8
#define         RAM_16MEG       16
#define         RAM_32MEG       32
#define         RAM_64MEG       64
#define         RAM_128MEG      128
#define         RAM_256MEG      256

/* SBR register definitions (valid bits are [7:3])*/
#define         SBR_32MEG       0x08
#define         SBR_64MEG       0x10
#define         SBR_128MEG      0x20
#define         SBR_256MEG      0x40

/* Drive Strengths - assume single DIMM configuration */
#define    SDCR_1BANK_X16       0x0aa0
#define    SDCR_2BANK_X16       0x12c8
#define    SDCR_1BANK_X8        0x1520
#define    SDCR_2BANK_X8        0x1548

/* SDRAM PD bytes                               */
#define         BANKCNT_BYTE     0x06 /* Byte #5 of SPD: number of module banks */
#define         SDRAM_WIDTH_BYTE 0x0e /* Byte #13 of SPD: DRAM width */
#define         BANKSZ_BYTE      0x20 /* Byte #31 of SPD: module bank density */
#define         CHECKSUM_BYTE    0x40 /* Byte #63 of SPD: checksum for bytes 0-62 */
#define         CONFIG_BYTE     0x0C  /* Byte #11 of SPD: DIMM configuration type (Parity or not, EEC) */

#define SDRAM_DEVID     0xA2  /* SDRAM Device ID  */

// Yavapai PCI and Peripheral Interrupt Unit
/*** Yavapai Registers ***/

/* PCI-to-PCI Bridge Unit 0000 1000H through 0000 10FFH */
#define VIDR_ADDR       0x00001000
#define DIDR_ADDR       0x00001002
#define PCR_ADDR        0x00001004
#define PSR_ADDR        0x00001006
#define RIDR_ADDR       0x00001008
#define CCR_ADDR        0x00001009
#define CLSR_ADDR       0x0000100C
#define PLTR_ADDR       0x0000100D
#define HTR_ADDR        0x0000100E
/* Reserved 0x0000100F through  0x00001017 */
#define PBNR_ADDR       0x00001018
#define SBNR_ADDR       0x00001019
#define SUBBNR_ADDR     0x0000101A
#define SLTR_ADDR       0x0000101B
#define IOBR_ADDR       0x0000101C
#define IOLR_ADDR       0x0000101D
#define SSR_ADDR        0x0000101E
#define MBR_ADDR        0x00001020
#define MLR_ADDR        0x00001022
#define PMBR_ADDR       0x00001024
#define PMLR_ADDR       0x00001026
/* Reserved 0x00001028 through 0x00001033 */
#define BSVIR_ADDR      0x00001034
#define BSIR_ADDR       0x00001036
/* Reserved 0x00001038 through 0x0000103D */
#define BCR_ADDR        0x0000103E
#define EBCR_ADDR       0x00001040
#define SISR_ADDR       0x00001042
#define PBISR_ADDR      0x00001044
#define SBISR_ADDR      0x00001048
#define SACR_ADDR       0x0000104C
#define PIRSR_ADDR      0x00001050
#define SIOBR_ADDR      0x00001054
#define SIOLR_ADDR      0x00001055
#define SCCR_ADDR       0x00001056              /* EAS inconsistent */
#define SMBR_ADDR       0x00001058
#define SMLR_ADDR       0x0000105A
#define SDER_ADDR       0x0000105C
#define QCR_ADDR        0x0000105E
#define CDTR_ADDR       0x00001060              /* EAS inconsistent */
/* Reserved 0x00001064 through 0x000010FFH */

/* Performance Monitoring Unit 0000 1100H through 0000 11FFH */
#define GTMR_ADDR       0x00001100
#define ESR_ADDR        0x00001104
#define EMISR_ADDR      0x00001108
/* Reserved 0x0000110C */                       /* EAS inconsistent */
#define GTSR_ADDR       0x00001110              /* EAS inconsistent */
#define PECR1_ADDR      0x00001114              /* EAS inconsistent */
#define PECR2_ADDR      0x00001118              /* EAS inconsistent */
#define PECR3_ADDR      0x0000111C              /* EAS inconsistent */
#define PECR4_ADDR      0x00001120              /* EAS inconsistent */
#define PECR5_ADDR      0x00001124              /* EAS inconsistent */
#define PECR6_ADDR      0x00001128              /* EAS inconsistent */
#define PECR7_ADDR      0x0000112C              /* EAS inconsistent */
#define PECR8_ADDR      0x00001130              /* EAS inconsistent */
#define PECR9_ADDR      0x00001134              /* EAS inconsistent */
#define PECR10_ADDR     0x00001138              /* EAS inconsistent */
#define PECR11_ADDR     0x0000113C              /* EAS inconsistent */
#define PECR12_ADDR     0x00001140              /* EAS inconsistent */
#define PECR13_ADDR     0x00001144              /* EAS inconsistent */
#define PECR14_ADDR     0x00001148              /* EAS inconsistent */
/* Reserved 0x0000104C through 0x000011FFH */   /* EAS inconsistent */

/* Address Translation Unit 0000 1200H through 0000 12FFH */
#define ATUVID_ADDR             0x00001200
#define ATUDID_ADDR             0x00001202
#define PATUCMD_ADDR            0x00001204
#define PATUSR_ADDR             0x00001206
#define ATURID_ADDR             0x00001208
#define ATUCCR_ADDR             0x00001209
#define ATUCLSR_ADDR            0x0000120C
#define ATULT_ADDR              0x0000120D
#define ATUHTR_ADDR             0x0000120E
#define ATUBISTR_ADDR           0x0000120F
#define PIABAR_ADDR             0x00001210
/* Reserved 0x00001214 through 0x0000122B */
#define ASVIR_ADDR              0x0000122C
#define ASIR_ADDR               0x0000122E
#define ERBAR_ADDR              0x00001230
/* Reserved 0x00001234 */
/* Reserved 0x00001238 */
#define ATUILR_ADDR             0x0000123C
#define ATUIPR_ADDR             0x0000123D
#define ATUMGNT_ADDR            0x0000123E
#define ATUMLAT_ADDR            0x0000123F
#define PIALR_ADDR              0x00001240
#define PIATVR_ADDR             0x00001244
#define SIABAR_ADDR             0x00001248
#define SIALR_ADDR              0x0000124C
#define SIATVR_ADDR             0x00001250
#define POMWVR_ADDR             0x00001254
/* Reserved 0x00001258 */
#define POIOWVR_ADDR            0x0000125C
#define PODWVR_ADDR             0x00001260
#define POUDR_ADDR              0x00001264
#define SOMWVR_ADDR             0x00001268
#define SOIOWVR_ADDR            0x0000126C
/* Reserved 0x00001270 */
#define ERLR_ADDR               0x00001274
#define ERTVR_ADDR              0x00001278
/* Reserved 0x0000127C */
/* Reserved 0x00001280 */
/* Reserved 0x00001284 */
#define ATUCR_ADDR              0x00001288
/* Reserved 0x0000128C */
#define PATUISR_ADDR            0x00001290
#define SATUISR_ADDR            0x00001294
#define SATUCMD_ADDR            0x00001298
#define SATUSR_ADDR             0x0000129A
#define SODWVR_ADDR             0x0000129C
#define SOUDR_ADDR              0x000012A0
#define POCCAR_ADDR             0x000012A4
#define SOCCAR_ADDR             0x000012A8
#define POCCDR_ADDR             0x000012AC
#define SOCCDR_ADDR             0x000012B0
#define PAQCR_ADDR              0x000012B4
#define SAQCR_ADDR              0x000012B8
#define PAIMR_ADDR              0x000012BC
#define SAIMR_ADDR              0x000012C0
/* Reserved 0x000012C4 through 0x000012FF */

/* Messaging Unit 0000 1300H through 0000 130FH */
#define IMR0_ADDR               0x00001310
#define IMR1_ADDR               0x00001314
#define OMR0_ADDR               0x00001318
#define OMR1_ADDR               0x0000131C
#define IDR_ADDR                0x00001320
#define IISR_ADDR               0x00001324
#define IIMR_ADDR               0x00001328
#define ODR_ADDR                0x0000132C
#define OISR_ADDR               0x00001330
#define OIMR_ADDR               0x00001334
/* Reserved 0x00001338 through 0x0000134F */
#define MUCR_ADDR               0x00001350
#define QBAR_ADDR               0x00001354
/* Reserved 0x00001358 */
/* Reserved 0x0000135C */
#define IFHPR_ADDR              0x00001360
#define IFTPR_ADDR              0x00001364
#define IPHPR_ADDR              0x00001368
#define IPTPR_ADDR              0x0000136C
#define OFHPR_ADDR              0x00001370
#define OFTPR_ADDR              0x00001374
#define OPHPR_ADDR              0x00001378
#define OPTPR_ADDR              0x0000137C
#define IAR_ADDR                0x00001380
/* Reserved 0x00001384 through 0x000013FF */

/* DMA Controller 0000 1400H through 0000 14FFH */
#define CCR0_ADDR               0x00001400
#define CSR0_ADDR               0x00001404
/* Reserved 0x00001408 */
#define DAR0_ADDR               0x0000140C
#define NDAR0_ADDR              0x00001410
#define PADR0_ADDR              0x00001414
#define PUADR0_ADDR             0x00001418
#define LADR0_ADDR              0x0000141C
#define BCR0_ADDR               0x00001420
#define DCR0_ADDR               0x00001424
/* Reserved 0x00001428 through 0x0000143F */
#define CCR1_ADDR               0x00001440
#define CSR1_ADDR               0x00001444
/* Reserved 0x00001448 */
#define DAR1_ADDR               0x0000144C
#define NDAR1_ADDR              0x00001450
#define PADR1_ADDR              0x00001454
#define PUADR1_ADDR             0x00001458
#define LADR1_ADDR              0x0000145C
#define BCR1_ADDR               0x00001460
#define DCR1_ADDR               0x00001464
/* Reserved 0x00001468 through 0x0000147F */
#define CCR2_ADDR               0x00001480
#define CSR2_ADDR               0x00001484
/* Reserved 0x00001488 */
#define DAR2_ADDR               0x0000148C
#define NDAR2_ADDR              0x00001490
#define PADR2_ADDR              0x00001494
#define PUADR2_ADDR             0x00001498
#define LADR2_ADDR              0x0000149C
#define BCR2_ADDR               0x000014A0
#define DCR2_ADDR               0x000014A4
/* Reserved 0x000014A8 through 0x000014FF */

/* Memory Controller 0000 1500H through 0000 15FFH */
#define SDIR_ADDR               0x00001500
#define SDCR_ADDR               0x00001504
#define SDBR_ADDR               0x00001508
#define SBR0_ADDR               0x0000150C
#define SBR1_ADDR               0x00001510
#define SDPR0_ADDR              0x00001514
#define SDPR1_ADDR              0x00001518
#define SDPR2_ADDR              0x0000151C
#define SDPR3_ADDR              0x00001520
#define SDPR4_ADDR              0x00001524
#define SDPR5_ADDR              0x00001528
#define SDPR6_ADDR              0x0000152C
#define SDPR7_ADDR              0x00001530
#define ECCR_ADDR               0x00001534
#define ELOG0_ADDR              0x00001538
#define ELOG1_ADDR              0x0000153C
#define ECAR0_ADDR              0x00001540
#define ECAR1_ADDR              0x00001544
#define ECTST_ADDR              0x00001548
#define FEBR0_ADDR              0x0000154C
#define FEBR1_ADDR              0x00001550
#define FBSR0_ADDR              0x00001554
#define FBSR1_ADDR              0x00001558
#define FWSR0_ADDR              0x0000155C
#define FWSR1_ADDR              0x00001560
#define MCISR_ADDR              0x00001564
#define RFR_ADDR                0x00001568
/* Reserved 0x0000156C through 0x000015FF */

/* Arbitration Control Unit 0000 1600H through 0000 167FH */
#define IACR_ADDR               0x00001600
#define MLTR_ADDR               0x00001604
#define MTTR_ADDR               0x00001608
/* Reserved 0x0000160C through 0x0000163F */

/* Bus Interface Control Unit 0000 1640H through 0000 167FH */
#define BIUCR_ADDR              0x00001640
#define BIUISR_ADDR             0x00001644
/* Reserved 0x00001648 through 0x0000167F */

/* I2C Bus Interface Unit 0000 1680H through 0000 16FFH */
#define ICR_ADDR                0x00001680
#define ISR_ADDR                0x00001684
#define ISAR_ADDR               0x00001688
#define IDBR_ADDR               0x0000168C
#define ICCR_ADDR               0x00001690
#define IBMR_ADDR               0x00001694
/* Reserved 0x00001698 through 0x000016FF */

/* PCI And Peripheral Interrupt Controller 0000 1700H through 0000 17FFH */
#define NISR_ADDR               0x00001700
#define X7ISR_ADDR              0x00001704
#define X6ISR_ADDR              0x00001708
#define PDIDR_ADDR              0x00001710              /* EAS inconsistent */
/* Reserved 0x00001714 through 0x0000177F */

/* Application Accelerator Unit 0000 1800H through 0000 18FFH */
#define ACR_ADDR                0x00001800
#define ASR_ADDR                0x00001804
#define ADAR_ADDR               0x00001808
#define ANDAR_ADDR              0x0000180C
#define SAR1_ADDR               0x00001810
#define SAR2_ADDR               0x00001814
#define SAR3_ADDR               0x00001818
#define SAR4_ADDR               0x0000181C
#define DAR_ADDR                0x00001820
#define ABCR_ADDR               0x00001824
#define ADCR_ADDR               0x00001828
#define SAR5_ADDR               0x0000182C
#define SAR6_ADDR               0x00001830
#define SAR7_ADDR               0x00001834
#define SAR8_ADDR               0x00001838

/* Reserved 0x0000183C through 0x000018FF */

#define X6ISR_REG ((volatile cyg_uint32 *)X6ISR_ADDR)
#  define X6ISR_DIP0 0x01
#  define X6ISR_DIP1 0x02
#  define X6ISR_DIP2 0x04
#  define X6ISR_EMIP 0x10
#  define X6ISR_AAIP 0x20

#define X7ISR_REG ((volatile cyg_uint32 *)X7ISR_ADDR)
#  define X7ISR_ISQC 0x02
#  define X7ISR_INDB 0x04
#  define X7ISR_BIST 0x08

#define NISR_REG ((volatile cyg_uint32 *)NISR_ADDR)
#  define NISR_MCU  0x01    
#  define NISR_PATU 0x02
#  define NISR_SATU 0x04
#  define NISR_PBDG 0x08
#  define NISR_SBDG 0x10
#  define NISR_DMA0 0x20
#  define NISR_DMA1 0x40
#  define NISR_DMA2 0x80
#  define NISR_MU   0x100
#  define NISR_AAU  0x400
#  define NISR_BIU  0x800

#define PIRSR_REG       REG32(0,PIRSR_ADDR)
#define IISR_REG        REG32(0,IISR_ADDR)
#define IIMR_REG        REG32(0,IIMR_ADDR)
#define OISR_REG        REG32(0,OISR_ADDR)
#define OIMR_REG        REG32(0,OIMR_ADDR)
#define EMISR_REG       REG32(0,EMISR_ADDR)
#define ISR_REG         REG32(0,ISR_ADDR)
#define GTMR_REG        REG32(0,GTMR_ADDR)
#define ESR_REG         REG32(0,ESR_ADDR)
#define ADCR_REG        REG32(0,ADCR_ADDR)
#define ICR_REG         REG32(0,ICR_ADDR)
#define ATUCR_REG       REG32(0,ATUCR_ADDR)

#define DCR0_REG        REG32(0,DCR0_ADDR)
#define DCR1_REG        REG32(0,DCR1_ADDR)
#define DCR2_REG        REG32(0,DCR2_ADDR)

#define ECCR_REG        REG32(0,ECCR_ADDR)
#define MCISR_REG       REG32(0,MCISR_ADDR)
#define ELOG0_REG       REG32(0,ELOG0_ADDR)
#define ELOG1_REG       REG32(0,ELOG1_ADDR)
#define ECAR0_REG       REG32(0,ECAR0_ADDR)
#define ECAR1_REG       REG32(0,ECAR1_ADDR)

#define PATUISR_REG     REG32(0,PATUISR_ADDR)
#define SATUISR_REG     REG32(0,SATUISR_ADDR)
#define PBISR_REG       REG32(0,PBISR_ADDR)
#define SBISR_REG       REG32(0,SBISR_ADDR)
#define CSR0_REG        REG32(0,CSR0_ADDR)
#define CSR1_REG        REG32(0,CSR1_ADDR)
#define CSR2_REG        REG32(0,CSR2_ADDR)
#define IISR_REG        REG32(0,IISR_ADDR)
#define ASR_REG         REG32(0,ASR_ADDR)
#define BIUISR_REG      REG32(0,BIUISR_ADDR)

#define PATUSR_REG      REG16(0,PATUSR_ADDR)
#define SATUSR_REG      REG16(0,SATUSR_ADDR)
#define PSR_REG         REG16(0,PSR_ADDR)
#define SSR_REG         REG16(0,SSR_ADDR)

#define PCR_REG         REG16(0,PCR_ADDR)
#define PBNR_REG        REG8(0,PBNR_ADDR)
#define SBNR_REG        REG8(0,SBNR_ADDR)
#define SUBBNR_REG      REG8(0,SUBBNR_ADDR)
#define BCR_REG         REG16(0,BCR_ADDR)
#define PATUCMD_REG     REG32(0,PATUCMD_ADDR)
#define PIABAR_REG      REG32(0,PIABAR_ADDR)
#define SIABAR_REG      REG32(0,SIABAR_ADDR)
#define SIALR_REG       REG32(0,SIALR_ADDR)
#define SIATVR_REG      REG32(0,SIATVR_ADDR)
#define POMWVR_REG      REG32(0,POMWVR_ADDR)
#define POIOWVR_REG     REG32(0,POIOWVR_ADDR)
#define PODWVR_REG      REG32(0,PODWVR_ADDR)
#define SOMWVR_REG      REG32(0,SOMWVR_ADDR)
#define SOIOWVR_REG     REG32(0,SOIOWVR_ADDR)
#define SATUCMD_REG     REG16(0,SATUCMD_ADDR)
#define SODWVR_REG      REG32(0,SODWVR_ADDR)


#define MEMBASE_DRAM 0xa0000000

/* primary PCI bus definitions */ 
#define PRIMARY_BUS_NUM         0
#define PRIMARY_MEM_BASE        0x80000000
#define PRIMARY_DAC_BASE        0x84000000
#define PRIMARY_IO_BASE         0x90000000
#define PRIMARY_MEM_LIMIT       0x83ffffff
#define PRIMARY_DAC_LIMIT       0x87ffffff
#define PRIMARY_IO_LIMIT        0x9000ffff

/* secondary PCI bus definitions */
#define SECONDARY_BUS_NUM       1
#define SECONDARY_MEM_BASE      0x88000000
#define SECONDARY_DAC_BASE      0x8c000000
#define SECONDARY_IO_BASE       0x90010000
#define SECONDARY_MEM_LIMIT     0x8bffffff
#define SECONDARY_DAC_LIMIT     0x8fffffff
#define SECONDARY_IO_LIMIT      0x9001ffff

#ifndef __ASSEMBLER__
extern unsigned int _80312_EMISR;  // Only valid for PEC ISR
#endif

// ------------------------------------------------------------------------
#ifdef __ASSEMBLER__
// Macro to perform early setup (in hal_platform_setup.h) of some PCI
// registers. This should be used prior to the ECC scrup loop to prevent
// timeouts with some Plug 'n Play PC BIOSes. Before the 'retry' bit is
// cleared, this macro must setup the vendor/device ID and the primary
// inbound window BAR to allow primary side access to SDRAM.
//
//  Input: reg, reg1      -- scratch registers
//         reg_dram_size  -- register holding size of SDRAM
//         vendor_id      -- PCI vendor ID
//         device_id      -- PCI device ID

.macro IOP310_EARLY_PCI_SETUP reg0, reg1, reg_dram_size, vendor_id, device_id
        // Setup vendor/device ID for 80312
        ldr     \reg0, =ASVIR_ADDR
        ldr     \reg1, =\vendor_id
        strh    \reg1, [\reg0]
        ldr     \reg0, =ASIR_ADDR
        ldr     \reg1, =\device_id
        strh    \reg1, [\reg0]
        // Make all secondary bus devices private
        ldr     \reg0, =SISR_ADDR
        ldr     \reg1, =0x03FF
        strh    \reg1, [\reg0]
        // Setup primary inbound window into SDRAM
        ldr     \reg0, =PIATVR_ADDR
        mov     \reg1, #0xa0000000
        str     \reg1, [\reg0]
        ldr     \reg0, =PIALR_ADDR
        sub     \reg1, \reg_dram_size, #1  // dram_size - 1
        mvn     \reg1, \reg1               // 1s complement
        str     \reg1, [\reg0]
#ifdef CYGSEM_HAL_ARM_IOP310_CLEAR_PCI_RETRY
        ldr     \reg0, =EBCR_ADDR
        mov     \reg1, #0x0008
        strh    \reg1, [\reg0]
#endif  
.endm

#endif // __ASSEMBLER__

// ------------------------------------------------------------------------

// Override the default MMU off code. This is intended
// to be included in an inline asm statement.
#define CYGARC_HAL_MMU_OFF(__paddr__)        \
              "   mrc p15,0,r0,c1,c0,0\n"    \
              "   bic r0,r0,#0x05\n"         \
              "   b 99f\n"                   \
              "   .p2align 5\n"              \
              "99:\n"                        \
              "   mcr p15,0,r0,c1,c0,0\n"    \
              "   mrc p15,0,r0,c2,c0,0\n"    \
              "   mov r0,r0\n"               \
              "   sub pc,pc,#4\n"            \
              "   mov pc," #__paddr__ "\n"


/*---------------------------------------------------------------------------*/
/* end of hal_iop310.h                                                         */
#endif /* CYGONCE_HAL_IOP310_H */
