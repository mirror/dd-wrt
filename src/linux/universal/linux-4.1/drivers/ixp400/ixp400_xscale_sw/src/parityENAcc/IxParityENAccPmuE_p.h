/**
 * @file IxParityENAccPmuE_p.h
 *
 * @author Intel Corporation
 * @date 26 July 2004
 *
 * @brief Private header file for Performance Monitoring Unit Enabler
 * sub-component of the IXP400 Parity Error Notifier access component.
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
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

#ifndef IXPARITYENACCPMUE_P_H
#define IXPARITYENACCPMUE_P_H

#include "IxOsal.h"

/*
 * #defines and macros used in this file.
 */

/*
 * Base Addresses for PMU Registers
 * 
 * The offset of 0x18 is for the PMSR register needed for PMU 
 * sub-component access 
 */
#define IXP400_PARITYENACC_PMU_BASEADDR     (IX_OSAL_IXP400_PMU_PHYS_BASE + 0x18)

/* Address Range for PMU register */
#define IXP400_PARITYENACC_PMU_MEMMAP_SIZE  (0x04)

/* Mask values to access PMU Previous Master/Slave Register */
#define IXP400_PARITYENACC_PMU_PMSR_SOUTH_AHB_ERR     (1 << 31)
#define IXP400_PARITYENACC_PMU_PMSR_NORTH_AHB_ERR     (1 << 30)
#define IXP400_PARITYENACC_PMU_PMSR_PREV_SLAVE_SOUTH  (0x0000F000)
#define IXP400_PARITYENACC_PMU_PMSR_PREV_SLAVE_NORTH  (0x00000F00)
#define IXP400_PARITYENACC_PMU_PMSR_PREV_MASTER_SOUTH (0x000000F0)
#define IXP400_PARITYENACC_PMU_PMSR_PREV_MASTER_NORTH (0x0000000F)

/*
 * Values as interpreted from PMU Previous Master/Slave Register
 *
 * NOTE: Y and N indicates whether that particular PMU device can
 * be a Master/Slave for a given AHB transactions respectively.
 */

/*
 * Master/Slave for a South AHB Bus Transaction
 * ixp46X/ixp43X  device
 * PMU Device # Name                Master  Slave
 * ------------ ---------------     ------  -----
 * 0            X-Scale BIU         Y       N
 * 1            PCI                 Y       Y
 * 2            AHB Bridge          Y       N
 * 3            Expansion Bus       Y       Y
 * 4            USBH0               Y       Y
 * 5            MCU                 N       Y
 * 6            APB Bridge          N       Y
 * 7            AQM                 N       Y
 * 8            RSA                 N       Y
 * 8            USBH1               Y       Y
 */

#define IXP400_PARITYENACC_PMU_PMSS_XSCALE_BIU        (0)
#define IXP400_PARITYENACC_PMU_PMSS_PBC               (1)
#define IXP400_PARITYENACC_PMU_PMSS_AHB_BRIDGE        (2)
#define IXP400_PARITYENACC_PMU_PMSS_EBC               (3)
#define IXP400_PARITYENACC_PMU_PMSS_USBH0             (4)
#define IXP400_PARITYENACC_PMU_PMSS_MCU               (5)
#define IXP400_PARITYENACC_PMU_PMSS_APB_BRIDGE        (6)
#define IXP400_PARITYENACC_PMU_PMSS_AQM               (7)
#define IXP400_PARITYENACC_PMU_PMSS_RSA               (8)
#define IXP400_PARITYENACC_PMU_PMSS_USBH1             (8)


/*
 * Master/Slave for a North AHB Bus Transaction
 *
 * PMU Device # Name                Master  Slave
 * ------------ ---------------     ------  -----
 * 0            NPE-A               Y       N
 * 1            NPE-B               Y       N (__ixp46X)
 * 1            NPE-B               N       N (__ixp43X)
 * 2            NPE-C               Y       N
 * 3            MCU                 N       Y
 * 4            AHB Bridge          N       Y
 */

#define IXP400_PARITYENACC_PMU_PMSN_NPE_A             (0)
#define IXP400_PARITYENACC_PMU_PMSN_NPE_B             (1)
#define IXP400_PARITYENACC_PMU_PMSN_NPE_C             (2)
#define IXP400_PARITYENACC_PMU_PMSN_MCU               (3)
#define IXP400_PARITYENACC_PMU_PMSN_AHB_BRIDGE        (4)


/* Macros to adjust the Master/Slave values read from PMSR register */
#define IXP400_PARITYENACC_VAL_READ_PMU_SLAVE_SOUTH(value)      \
/* Shift the value into LSBs */                                 \
(((value) & IXP400_PARITYENACC_PMU_PMSR_PREV_SLAVE_SOUTH) >> 12)

#define IXP400_PARITYENACC_VAL_READ_PMU_SLAVE_NORTH(value)      \
/* Shift the value into LSB */                                  \
(((value) & IXP400_PARITYENACC_PMU_PMSR_PREV_SLAVE_NORTH) >>  8)

#define IXP400_PARITYENACC_VAL_READ_PMU_MASTER_SOUTH(value)     \
/* Shift the value into first nibble of LSB */                  \
(((value) & IXP400_PARITYENACC_PMU_PMSR_PREV_MASTER_SOUTH) >> 4)

#define IXP400_PARITYENACC_VAL_READ_PMU_MASTER_NORTH(value)     \
/* The value is already in first nibble LSB */                  \
((value) & IXP400_PARITYENACC_PMU_PMSR_PREV_MASTER_NORTH)


/*
 * Variable declarations
 */

/* Previous Master and Slave Register virtual address */
static UINT32 ixParityENAccPmuEPmsr = 0;

/* Previous Master and Slave Register contents */
static UINT32 ixParityENAccPmuEPmsrStatus = 0;

/*
 * These static tables are used to map onto the Master/Slaves as to be
 * observed by the public API with the help of the above enums  and by
 * using the subscript/index.
 *
 * Example: Consider PCI being the master of the AHB transaction, then
 * the enum value to be returned is selected as following:
 *
 * enumVariable = ixParityENAccPmuESouthAhbMaster[<PCI device Id from PMSR>]
 * enumVariable = ixParityENAccPmuESouthAhbMaster[1]
 *                (i.e., IXP400_PARITYENACC_PMUE_AHBS_MST_PBC)
 */

/* Master on North AHB Bus */
static UINT32 ixParityENAccPmuENorthAhbMaster[] =
{
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_A,       /* 0 */
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_B,       /* 1 */
    IXP400_PARITYENACC_PMUE_AHBN_MST_NPE_C        /* 2 */
};

/* Master on South AHB Bus */
static UINT32 ixParityENAccPmuESouthAhbMaster[] =
{
    IXP400_PARITYENACC_PMUE_AHBS_MST_XSCALE,      /* 0 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_PBC,         /* 1 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_AHB_BRIDGE,  /* 2 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_EBC,         /* 3 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_USBH0,       /* 4 */
    IXP400_PARITYENACC_PMUE_AHBS_MST_USBH1        /* 5 */
};

/* Slave on North AHB Bus */
static UINT32 ixParityENAccPmuENorthAhbSlave[] =
{
    IXP400_PARITYENACC_PMUE_AHBN_SLV_MCU,         /* 0 */
    IXP400_PARITYENACC_PMUE_AHBN_SLV_AHB_BRIDGE   /* 1 */
};

/* Slave on South AHB Bus */
static UINT32 ixParityENAccPmuESouthAhbSlave[] =
{
    IXP400_PARITYENACC_PMUE_AHBS_SLV_INVALID,     /* 0 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_PBC,         /* 1 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_INVALID,     /* 2 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_EBC,         /* 3 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_USBH0,       /* 4 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_MCU,         /* 5 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_APB_BRIDGE,  /* 6 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_AQM,         /* 7 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_RSA,         /* 8 */
    IXP400_PARITYENACC_PMUE_AHBS_SLV_USBH1        /* 8 */
};

#endif /* IXPARITYENACCPMUE_P_H */
