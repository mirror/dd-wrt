/**
 * @file IxPerfProfAccBusPmu_p.h
 *
 * @date April 9 2003
 *
 * @brief Private header file for the  BUS PMU portion of the IxPerfProfAcc
 * software component
 *
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IXPERFPROFACCBUSPMU_P_H
#define IXPERFPROFACCBUSPMU_P_H

#include "IxOsal.h"

#ifdef __vxworks
/* Variable to be used in Macros below indicating the virtually mapped addresses */
UINT32 esrVirtualAddress;
UINT32 srVirtualAddress;
UINT32 pmsrVirtualAddress;
UINT32 pecVirtualAddress;
#endif /* __vxworks */

/* PMU registers memory mapping */

#define ESR_MODE_BITS_OFFSET            2 /* Offsets the mode bits in the Event Status 
                                             Register */
#define NUM_OF_PEC_BITS                 3 /* Number od bits to select an event for a
                                             particular PEC */
#define ESR_MODE_BITS_MASK              0xfffffffc /* Mask out the mode bits in the ESR */
#define STATUS_REGISTER_RESET           0xffffffff /* Value to write to the  status register 
                                                      with to reset the register */
#define PEC_REGISTER_27BIT_MASK         0x7ffffff  /* Value to mask the PEC and obtain the
                                                      counter value */
#define PEC_REGISTER_OFFSET             0x4        /* Value to offset before accessing next
                                                      PEC register address */
#define PEC_INCREMENT                   1          /* Increments the PEC value when calling 
                                                      from function */
#define ESR_MODE_MASK                   0x3        /* Mask for the mode field in the esr */
#define STATUS_REGISTER_BIT1_MASK       0x1

/* Registers off set value from base address */
#define STATUS_REGISTER_ADDR_OFFSET     0x4
#define PEC1_ADDR_OFFSET                0x8
#define PEC2_ADDR_OFFSET                0xc
#define PEC3_ADDR_OFFSET                0x10
#define PEC4_ADDR_OFFSET                0x14
#define PEC5_ADDR_OFFSET                0x18
#define PEC6_ADDR_OFFSET                0x1c
#define PEC7_ADDR_OFFSET                0x20
#define PMSR_ADDR_OFFSET                0x24
 
/* Addresses for all the available registers. Event Status Register, Status register
   Programmable Event Counter, and Previous Master Slave Register */
#ifdef __vxworks
#define IX_PERFPROF_IXP400_PMU_BASE (IX_OSAL_IXP400_PERIPHERAL_PHYS_BASE + 0x00002000)
#elif defined(__linux)
#define IX_PERFPROF_IXP400_PMU_BASE (IX_OSAL_IXP400_PERIPHERAL_VIRT_BASE + 0x2000)
#endif

#define PMU_ESR               (IX_PERFPROF_IXP400_PMU_BASE)
#define PMU_SR                (IX_PERFPROF_IXP400_PMU_BASE+STATUS_REGISTER_ADDR_OFFSET)
#define PMU_PEC1              (IX_PERFPROF_IXP400_PMU_BASE+PEC1_ADDR_OFFSET)
#define PMU_PEC2              (IX_PERFPROF_IXP400_PMU_BASE+PEC2_ADDR_OFFSET)
#define PMU_PEC3              (IX_PERFPROF_IXP400_PMU_BASE+PEC3_ADDR_OFFSET)
#define PMU_PEC4              (IX_PERFPROF_IXP400_PMU_BASE+PEC4_ADDR_OFFSET)
#define PMU_PEC5              (IX_PERFPROF_IXP400_PMU_BASE+PEC5_ADDR_OFFSET)
#define PMU_PEC6              (IX_PERFPROF_IXP400_PMU_BASE+PEC6_ADDR_OFFSET)
#define PMU_PEC7              (IX_PERFPROF_IXP400_PMU_BASE+PEC7_ADDR_OFFSET)
#define PMU_PMSR              (IX_PERFPROF_IXP400_PMU_BASE+PMSR_ADDR_OFFSET)

#ifdef __vxworks
/* Macro to write selected value to the Event Status Register */
#define SET_PMU_ESR(value)    (IX_OSAL_WRITE_LONG(esrVirtualAddress, value))
/* Macro to get ESR value */
#define GET_PMU_ESR           (IX_OSAL_READ_LONG(esrVirtualAddress))
/* Macro to reset status register */
#define SR_RESET()            (IX_OSAL_WRITE_LONG(srVirtualAddress,STATUS_REGISTER_RESET))
/* Macro to read from status register */
#define GET_SR()              (IX_OSAL_READ_LONG(srVirtualAddress))
/* macro to select PEC and read its value */
#define PMU_CNT_ADDR(pec)     (IX_PERFPROF_IXP400_PMU_BASE + PEC_REGISTER_OFFSET + (PEC_REGISTER_OFFSET * (pec)))
#define PMU_CNT_PMSR()        (IX_OSAL_READ_LONG(pmsrVirtualAddress))
#define PMU_CNT_GET()         (IX_OSAL_READ_LONG(pecVirtualAddress)& PEC_REGISTER_27BIT_MASK)

#elif defined(__linux)
/* Macro to write selected value to the Event Status Register */
#define SET_PMU_ESR(value)    (IX_OSAL_WRITE_LONG(PMU_ESR, value))
/* Macro to get ESR value */
#define GET_PMU_ESR           (IX_OSAL_READ_LONG(PMU_ESR))
/* Macro to reset status register */
#define SR_RESET()            (IX_OSAL_WRITE_LONG(PMU_SR, STATUS_REGISTER_RESET))
/* Macro to read from status register */
#define GET_SR()              (IX_OSAL_READ_LONG(PMU_SR))
/* macro to select PEC and read its value */
#define PMU_CNT_ADDR(pec)     (IX_PERFPROF_IXP400_PMU_BASE + PEC_REGISTER_OFFSET + (PEC_REGISTER_OFFSET * (pec)))
#define PMU_CNT_PMSR()        (IX_OSAL_READ_LONG(PMU_PMSR))
#define PMU_CNT_GET(pec)      (IX_OSAL_READ_LONG(PMU_CNT_ADDR(pec))& PEC_REGISTER_27BIT_MASK)

#endif /* __vxworks */

/* Macros for register manipulation */
/* Value by which to offset the ESR in order for a particular PEC to be selected. */ 
#define PEC_SELECT_VALUE(counter)       (ESR_MODE_BITS_OFFSET + (NUM_OF_PEC_BITS * (IX_PERFPROF_ACC_BUS_PMU_MAX_PECS - (counter + PEC_INCREMENT))))
/* Macro to select event for a particular PEC */
#define SET_PMU_PEC(value,pec,event)    (value) = (value) | ((event) << PEC_SELECT_VALUE(pec))
/* Macro set the mode required */
#define SET_PMU_MODE(value,mode)        (value) = ((value) & ESR_MODE_BITS_MASK) | (mode)
/* Macro to reset event status register */ 
#define ESR_RESET()                     (SET_PMU_ESR(0)) 
#define PMU_HALT()                      (SET_PMU_MODE((GET_PMU_ESR), IX_PERFPROF_ACC_BUS_PMU_MODE_HALT))


/* Event for all counters. Memory mapped values */
#define PEC1_NORTH_NPEA_GRANT           0x0
#define PEC1_NORTH_NPEB_GRANT           0x1
#define PEC1_NORTH_NPEC_GRANT           0x2
#define PEC1_NORTH_BUS_IDLE             0x4
#define PEC1_NORTH_NPEA_REQ             0x5
#define PEC1_NORTH_NPEB_REQ             0x6
#define PEC1_NORTH_NPEC_REQ             0x7

#define PEC1_SOUTH_GSKT_GRANT           0x0
#define PEC1_SOUTH_ABB_GRANT            0x1
#define PEC1_SOUTH_PCI_GRANT            0x2
#define PEC1_SOUTH_APB_GRANT            0x3
#define PEC1_SOUTH_GSKT_REQ             0x4
#define PEC1_SOUTH_ABB_REQ              0x5
#define PEC1_SOUTH_PCI_REQ              0x6
#define PEC1_SOUTH_APB_REQ              0x7

#define PEC1_SDR_0_HIT                  0x0
#define PEC1_SDR_1_HIT                  0x1
#define PEC1_SDR_2_HIT                  0x2
#define PEC1_SDR_3_HIT                  0x3
#define PEC1_SDR_4_MISS                 0x4
#define PEC1_SDR_5_MISS                 0x5
#define PEC1_SDR_6_MISS                 0x6
#define PEC1_SDR_7_MISS                 0x7

#define PEC2_NORTH_NPEA_XFER            0x0
#define PEC2_NORTH_NPEB_XFER            0x1
#define PEC2_NORTH_NPEC_XFER            0x2
#define PEC2_NORTH_BUS_WRITE            0x4
#define PEC2_NORTH_NPEA_OWN             0x5
#define PEC2_NORTH_NPEB_OWN             0x6
#define PEC2_NORTH_NPEC_OWN             0x7

#define PEC2_SOUTH_GSKT_XFER            0x0
#define PEC2_SOUTH_ABB_XFER             0x1
#define PEC2_SOUTH_PCI_XFER             0x2
#define PEC2_SOUTH_APB_XFER             0x3
#define PEC2_SOUTH_GSKT_OWN             0x4
#define PEC2_SOUTH_ABB_OWN              0x5
#define PEC2_SOUTH_PCI_OWN              0x6
#define PEC2_SOUTH_APB_OWN              0x7

#define PEC2_SDR_1_HIT                  0x0
#define PEC2_SDR_2_HIT                  0x1
#define PEC2_SDR_3_HIT                  0x2
#define PEC2_SDR_4_HIT                  0x3
#define PEC2_SDR_5_MISS                 0x4
#define PEC2_SDR_6_MISS                 0x5
#define PEC2_SDR_7_MISS                 0x6
#define PEC2_SDR_0_MISS                 0x7

#define PEC3_NORTH_NPEA_RETRY           0x0
#define PEC3_NORTH_NPEB_RETRY           0x1
#define PEC3_NORTH_NPEC_RETRY           0x2
#define PEC3_NORTH_BUS_READ             0x4
#define PEC3_NORTH_NPEA_WRITE           0x5
#define PEC3_NORTH_NPEB_WRITE           0x6
#define PEC3_NORTH_NPEC_WRITE           0x7

#define PEC3_SOUTH_GSKT_RETRY           0x0
#define PEC3_SOUTH_ABB_RETRY            0x1
#define PEC3_SOUTH_PCI_RETRY            0x2
#define PEC3_SOUTH_APB_RETRY            0x3
#define PEC3_SOUTH_GSKT_WRITE           0x4
#define PEC3_SOUTH_ABB_WRITE            0x5
#define PEC3_SOUTH_PCI_WRITE            0x6
#define PEC3_SOUTH_APB_WRITE            0x7

#define PEC3_SDR_2_HIT                  0x0
#define PEC3_SDR_3_HIT                  0x1
#define PEC3_SDR_4_HIT                  0x2
#define PEC3_SDR_5_HIT                  0x3
#define PEC3_SDR_6_MISS                 0x4
#define PEC3_SDR_7_MISS                 0x5
#define PEC3_SDR_0_MISS                 0x6
#define PEC3_SDR_1_MISS                 0x7

#define PEC4_SOUTH_PCI_SPLIT            0x0
#define PEC4_SOUTH_EXP_SPLIT            0x1
#define PEC4_SOUTH_APB_GRANT            0x2
#define PEC4_SOUTH_APB_XFER             0x3
#define PEC4_SOUTH_GSKT_READ            0x4
#define PEC4_SOUTH_ABB_READ             0x5
#define PEC4_SOUTH_PCI_READ             0x6
#define PEC4_SOUTH_APB_READ             0x7

#define PEC4_NORTH_ABB_SPLIT            0x0
#define PEC4_NORTH_NPEA_REQ             0x4
#define PEC4_NORTH_NPEA_READ            0x5
#define PEC4_NORTH_NPEB_READ            0x6
#define PEC4_NORTH_NPEC_READ            0x7

#define PEC4_SDR_3_HIT                  0x0
#define PEC4_SDR_4_HIT                  0x1
#define PEC4_SDR_5_HIT                  0x2
#define PEC4_SDR_6_HIT                  0x3
#define PEC4_SDR_7_MISS                 0x4
#define PEC4_SDR_0_MISS                 0x5
#define PEC4_SDR_1_MISS                 0x6
#define PEC4_SDR_2_MISS                 0x7

#define PEC5_SOUTH_ABB_GRANT            0x0
#define PEC5_SOUTH_ABB_XFER             0x1
#define PEC5_SOUTH_ABB_RETRY            0x2
#define PEC5_SOUTH_EXP_SPLIT            0x3
#define PEC5_SOUTH_ABB_REQ              0x4
#define PEC5_SOUTH_ABB_OWN              0x5
#define PEC5_SOUTH_BUS_IDLE             0x6

#define PEC5_NORTH_NPEB_GRANT           0x0
#define PEC5_NORTH_NPEB_XFER            0x1
#define PEC5_NORTH_NPEB_RETRY           0x2
#define PEC5_NORTH_NPEB_REQ             0x4
#define PEC5_NORTH_NPEB_OWN             0x5
#define PEC5_NORTH_NPEB_WRITE           0x6
#define PEC5_NORTH_NPEB_READ            0x7

#define PEC5_SDR_4_HIT                  0x0
#define PEC5_SDR_5_HIT                  0x1
#define PEC5_SDR_6_HIT                  0x2
#define PEC5_SDR_7_HIT                  0x3
#define PEC5_SDR_0_MISS                 0x4
#define PEC5_SDR_1_MISS                 0x5
#define PEC5_SDR_2_MISS                 0x6
#define PEC5_SDR_3_MISS                 0x7

#define PEC6_SOUTH_PCI_GRANT            0x0
#define PEC6_SOUTH_PCI_XFER             0x1
#define PEC6_SOUTH_PCI_RETRY            0x2
#define PEC6_SOUTH_PCI_SPLIT            0x3
#define PEC6_SOUTH_PCI_REQ              0x4
#define PEC6_SOUTH_PCI_OWN              0x5
#define PEC6_SOUTH_BUS_WRITE            0x6

#define PEC6_NORTH_NPEC_GRANT           0x0
#define PEC6_NORTH_NPEC_XFER            0x1
#define PEC6_NORTH_NPEC_RETRY           0x2
#define PEC6_NORTH_NPEC_REQ             0x4
#define PEC6_NORTH_NPEC_OWN             0x5
#define PEC6_NORTH_NPEB_WRITE           0x6
#define PEC6_NORTH_NPEC_READ            0x7

#define PEC6_SDR_5_HIT                  0x0
#define PEC6_SDR_6_HIT                  0x1
#define PEC6_SDR_7_HIT                  0x2
#define PEC6_SDR_0_HIT                  0x3
#define PEC6_SDR_1_MISS                 0x4
#define PEC6_SDR_2_MISS                 0x5
#define PEC6_SDR_3_MISS                 0x6
#define PEC6_SDR_4_MISS                 0x7

#define PEC7_SOUTH_APB_RETRY            0x0
#define PEC7_SOUTH_APB_REQ              0x4
#define PEC7_SOUTH_APB_OWN              0x5
#define PEC7_SOUTH_BUS_READ             0x6
#define PEC7_SOUTH_CYCLE_COUNT          0x7

/* define counter bits values */
#define NUM_PEC_BITS                    27 /* Number of bits in the PEC that carry the
                                              value of the counter */
#define UPPER_COUNTER_BITS              32 /* Extra bits to support 59 bit counting
                                              Incremented everytime an interrupt is 
                                              generated by a a overflow */

/* Value of counters */
typedef enum
{
    PEC1=0,
    PEC2,
    PEC3,
    PEC4,
    PEC5,
    PEC6,
    PEC7
} IxPerfProfBusPmuPEC;

/* Structure to store counter event selection */
typedef struct 
{
    IxPerfProfAccBusPmuMode counterMode;
    IxPerfProfAccBusPmuEventCounters1 counterEvent1;
    IxPerfProfAccBusPmuEventCounters2 counterEvent2;
    IxPerfProfAccBusPmuEventCounters3 counterEvent3;
    IxPerfProfAccBusPmuEventCounters4 counterEvent4;
    IxPerfProfAccBusPmuEventCounters5 counterEvent5;
    IxPerfProfAccBusPmuEventCounters6 counterEvent6;
    IxPerfProfAccBusPmuEventCounters7 counterEvent7;

} IxPerfProfAccBusPmuModeEvents;		


/* Variable declaration */

/* Variables to support 59 bit counters from the 27 bit PEC ones */
UINT32 upper32BitCounter[IX_PERFPROF_ACC_BUS_PMU_MAX_PECS];

/* Function declaration */

/* Setup events for each PMU */
IxPerfProfAccStatus 
ixPerfProfAccBusPmuSetup ( IxPerfProfAccBusPmuModeEvents modeEvents);

/**
 * 
 * Function to check the validity of choices for PEC1 to PEC7 for North Mode 
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to North 
 *
 **/ 
IxPerfProfAccStatus
ixPerfProfAccBusPmuNorthCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents); 

/**
 * 
 * Function to check the validity of choices for PEC1 to PEC7 for South Mode 
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to South 
 *
 **/ 
IxPerfProfAccStatus
ixPerfProfAccBusPmuSouthCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents); 

/**
 * 
 * Function to check the validity of choices for PEC1 to PEC7 for Sdram Mode 
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to Sdram 
 *
 **/ 
IxPerfProfAccStatus
ixPerfProfAccBusPmuSdramCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents);


/* Interrupt Handler */
void ixPerfProfAccBusPmuPecOverflowHdlr (void *);

#endif /* ifdef IXPERFPROFACCBUSPMU_P_H */
