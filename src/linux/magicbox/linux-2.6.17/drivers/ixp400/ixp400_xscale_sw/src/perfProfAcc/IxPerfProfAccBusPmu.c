/**
 * @file IxPerfProfAccBusPmu.c
 *
 * @brief  Source file for the Xscale Bus PMU
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

#include "IxPerfProfAcc.h"
#include "IxFeatureCtrl.h"
#include "IxPerfProfAccBusPmu_p.h"
#include "IxPerfProfAcc_p.h"
#include "IxOsal.h"

#define IX_PERFPROF_ACC_BUS_PMU_MAX_EVENTS IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT

static BOOL startFlag = FALSE;

static UINT32
ixPerfProfAccBusPmuEventMap[IX_PERFPROF_ACC_BUS_PMU_MAX_EVENTS + 1] = {
    0,                  /* ENUM starts at 1, there is no entry for location [0] */
    PEC1_NORTH_NPEA_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEA_GRANT_SELECT*/
    PEC1_NORTH_NPEB_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEB_GRANT_SELECT*/
    PEC1_NORTH_NPEC_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEC_GRANT_SELECT*/
    PEC1_NORTH_BUS_IDLE,    /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_BUS_IDLE_SELECT*/
    PEC1_NORTH_NPEA_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEA_REQ_SELECT*/
    PEC1_NORTH_NPEB_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEB_REQ_SELECT*/
    PEC1_NORTH_NPEC_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEC_REQ_SELECT*/

    PEC1_SOUTH_GSKT_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_GSKT_GRANT_SELECT*/
    PEC1_SOUTH_ABB_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_ABB_GRANT_SELECT*/
    PEC1_SOUTH_PCI_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_PCI_GRANT_SELECT*/
    PEC1_SOUTH_APB_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_APB_GRANT_SELECT*/
    PEC1_SOUTH_GSKT_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_GSKT_REQ_SELECT*/
    PEC1_SOUTH_ABB_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_ABB_REQ_SELECT*/
    PEC1_SOUTH_PCI_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_PCI_REQ_SELECT*/
    PEC1_SOUTH_APB_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_APB_REQ_SELECT*/

    PEC1_SDR_0_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_0_HIT_SELECT*/
    PEC1_SDR_1_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_1_HIT_SELECT*/
    PEC1_SDR_2_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_2_HIT_SELECT*/
    PEC1_SDR_3_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_3_HIT_SELECT*/
    PEC1_SDR_4_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_4_MISS_SELECT*/
    PEC1_SDR_5_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_5_MISS_SELECT*/
    PEC1_SDR_6_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_6_MISS_SELECT*/
    PEC1_SDR_7_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_7_MISS_SELECT*/

    PEC2_NORTH_NPEA_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEA_XFER_SELECT*/
    PEC2_NORTH_NPEB_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEB_XFER_SELECT*/
    PEC2_NORTH_NPEC_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEC_XFER_SELECT*/
    PEC2_NORTH_BUS_WRITE,   /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_BUS_WRITE_SELECT*/
    PEC2_NORTH_NPEA_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEA_OWN_SELECT*/
    PEC2_NORTH_NPEB_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEB_OWN_SELECT*/
    PEC2_NORTH_NPEC_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEC_OWN_SELECT*/

    PEC2_SOUTH_GSKT_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_GSKT_XFER_SELECT*/
    PEC2_SOUTH_ABB_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_ABB_XFER_SELECT*/
    PEC2_SOUTH_PCI_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_PCI_XFER_SELECT*/
    PEC2_SOUTH_APB_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_APB_XFER_SELECT*/
    PEC2_SOUTH_GSKT_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_GSKT_OWN_SELECT*/
    PEC2_SOUTH_ABB_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_ABB_OWN_SELECT*/
    PEC2_SOUTH_PCI_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_PCI_OWN_SELECT*/
    PEC2_SOUTH_APB_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_APB_OWN_SELECT*/

    PEC2_SDR_1_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_1_HIT_SELECT*/
    PEC2_SDR_2_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_2_HIT_SELECT*/
    PEC2_SDR_3_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_3_HIT_SELECT*/
    PEC2_SDR_4_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_4_HIT_SELECT*/
    PEC2_SDR_5_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_5_MISS_SELECT*/
    PEC2_SDR_6_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_6_MISS_SELECT*/
    PEC2_SDR_7_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_7_MISS_SELECT*/
    PEC2_SDR_0_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_0_MISS_SELECT*/

    PEC3_NORTH_NPEA_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEA_RETRY_SELECT*/
    PEC3_NORTH_NPEB_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEB_RETRY_SELECT*/
    PEC3_NORTH_NPEC_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEC_RETRY_SELECT*/
    PEC3_NORTH_BUS_READ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_BUS_READ_SELECT*/
    PEC3_NORTH_NPEA_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEA_WRITE_SELECT*/
    PEC3_NORTH_NPEB_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEB_WRITE_SELECT*/
    PEC3_NORTH_NPEC_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEC_WRITE_SELECT*/

    PEC3_SOUTH_GSKT_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_GSKT_RETRY_SELECT*/
    PEC3_SOUTH_ABB_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_ABB_RETRY_SELECT*/
    PEC3_SOUTH_PCI_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_PCI_RETRY_SELECT*/
    PEC3_SOUTH_APB_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_APB_RETRY_SELECT*/
    PEC3_SOUTH_GSKT_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_GSKT_WRITE_SELECT*/
    PEC3_SOUTH_ABB_WRITE,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_ABB_WRITE_SELECT*/
    PEC3_SOUTH_PCI_WRITE,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_PCI_WRITE_SELECT*/
    PEC3_SOUTH_APB_WRITE,   /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_APB_WRITE_SELECT*/

    PEC3_SDR_2_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_2_HIT_SELECT*/
    PEC3_SDR_3_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_3_HIT_SELECT*/
    PEC3_SDR_4_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_4_HIT_SELECT*/
    PEC3_SDR_5_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_5_HIT_SELECT*/
    PEC3_SDR_6_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_6_MISS_SELECT*/
    PEC3_SDR_7_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_7_MISS_SELECT*/
    PEC3_SDR_0_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_0_MISS_SELECT*/
    PEC3_SDR_1_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_1_MISS_SELECT*/

    PEC4_SOUTH_PCI_SPLIT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_PCI_SPLIT_SELECT*/
    PEC4_SOUTH_EXP_SPLIT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_EXP_SPLIT_SELECT*/
    PEC4_SOUTH_APB_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_GRANT_SELECT*/
    PEC4_SOUTH_APB_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_XFER_SELECT*/
    PEC4_SOUTH_GSKT_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_GSKT_READ_SELECT*/
    PEC4_SOUTH_ABB_READ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_ABB_READ_SELECT*/
    PEC4_SOUTH_PCI_READ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_PCI_READ_SELECT*/
    PEC4_SOUTH_APB_READ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_READ_SELECT*/

    PEC4_NORTH_ABB_SPLIT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_ABB_SPLIT_SELECT*/
    PEC4_NORTH_NPEA_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_REQ_SELECT*/
    PEC4_NORTH_NPEA_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_READ_SELECT*/
    PEC4_NORTH_NPEB_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEB_READ_SELECT*/
    PEC4_NORTH_NPEC_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEC_READ_SELECT*/

    PEC4_SDR_3_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_3_HIT_SELECT*/
    PEC4_SDR_4_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_4_HIT_SELECT*/
    PEC4_SDR_5_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_5_HIT_SELECT*/
    PEC4_SDR_6_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_6_HIT_SELECT*/
    PEC4_SDR_7_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_7_MISS_SELECT*/
    PEC4_SDR_0_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_0_MISS_SELECT*/
    PEC4_SDR_1_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_1_MISS_SELECT*/
    PEC4_SDR_2_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_2_MISS_SELECT*/

    PEC5_SOUTH_ABB_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_GRANT_SELECT*/
    PEC5_SOUTH_ABB_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_XFER_SELECT*/
    PEC5_SOUTH_ABB_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_RETRY_SELECT*/
    PEC5_SOUTH_EXP_SPLIT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_EXP_SPLIT_SELECT*/
    PEC5_SOUTH_ABB_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_REQ_SELECT*/
    PEC5_SOUTH_ABB_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_OWN_SELECT*/
    PEC5_SOUTH_BUS_IDLE,    /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_BUS_IDLE_SELECT*/

    PEC5_NORTH_NPEB_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_GRANT_SELECT*/
    PEC5_NORTH_NPEB_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_XFER_SELECT*/
    PEC5_NORTH_NPEB_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_RETRY_SELECT*/
    PEC5_NORTH_NPEB_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_REQ_SELECT*/
    PEC5_NORTH_NPEB_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_OWN_SELECT*/
    PEC5_NORTH_NPEB_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_WRITE_SELECT*/
    PEC5_NORTH_NPEB_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_READ_SELECT*/

    PEC5_SDR_4_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_4_HIT_SELECT*/
    PEC5_SDR_5_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_5_HIT_SELECT*/
    PEC5_SDR_6_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_6_HIT_SELECT*/
    PEC5_SDR_7_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_7_HIT_SELECT*/
    PEC5_SDR_0_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_0_MISS_SELECT*/
    PEC5_SDR_1_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_1_MISS_SELECT*/
    PEC5_SDR_2_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_2_MISS_SELECT*/
    PEC5_SDR_3_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_3_MISS_SELECT*/

    PEC6_SOUTH_PCI_GRANT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_GRANT_SELECT*/
    PEC6_SOUTH_PCI_XFER,    /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_XFER_SELECT*/
    PEC6_SOUTH_PCI_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_RETRY_SELECT*/
    PEC6_SOUTH_PCI_SPLIT,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_SPLIT_SELECT*/
    PEC6_SOUTH_PCI_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_REQ_SELECT*/
    PEC6_SOUTH_PCI_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_OWN_SELECT*/
    PEC6_SOUTH_BUS_WRITE,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_BUS_WRITE_SELECT*/

    PEC6_NORTH_NPEC_GRANT,  /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_GRANT_SELECT*/
    PEC6_NORTH_NPEC_XFER,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_XFER_SELECT*/
    PEC6_NORTH_NPEC_RETRY,  /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_RETRY_SELECT*/
    PEC6_NORTH_NPEC_REQ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_REQ_SELECT*/
    PEC6_NORTH_NPEC_OWN,    /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_OWN_SELECT*/
    PEC6_NORTH_NPEB_WRITE,  /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEB_WRITE_SELECT*/
    PEC6_NORTH_NPEC_READ,   /*IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_READ_SELECT*/

    PEC6_SDR_5_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_5_HIT_SELECT*/
    PEC6_SDR_6_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_6_HIT_SELECT*/
    PEC6_SDR_7_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_7_HIT_SELECT*/
    PEC6_SDR_0_HIT,         /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_0_HIT_SELECT*/
    PEC6_SDR_1_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_1_MISS_SELECT*/
    PEC6_SDR_2_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_2_MISS_SELECT*/
    PEC6_SDR_3_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_3_MISS_SELECT*/
    PEC6_SDR_4_MISS,        /*IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_4_MISS_SELECT*/

    PEC7_SOUTH_APB_RETRY,   /*IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_RETRY_SELECT*/
    PEC7_SOUTH_APB_REQ,     /*IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_REQ_SELECT*/
    PEC7_SOUTH_APB_OWN,     /*IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_OWN_SELECT*/
    PEC7_SOUTH_BUS_READ,    /*IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_BUS_READ_SELECT*/
    PEC7_SOUTH_CYCLE_COUNT  /*IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT*/
};


/* This function places all the input parameter into a structure,
 * and calls the setup function to set all the registers.
 **/
PUBLIC IxPerfProfAccStatus
ixPerfProfAccBusPmuStart (
    IxPerfProfAccBusPmuMode mode,
    IxPerfProfAccBusPmuEventCounters1 pecEvent1,
    IxPerfProfAccBusPmuEventCounters2 pecEvent2,
    IxPerfProfAccBusPmuEventCounters3 pecEvent3,
    IxPerfProfAccBusPmuEventCounters4 pecEvent4,
    IxPerfProfAccBusPmuEventCounters5 pecEvent5,
    IxPerfProfAccBusPmuEventCounters6 pecEvent6,
    IxPerfProfAccBusPmuEventCounters7 pecEvent7)
{

    UINT32 PecCounter; /* Variable to increment in the for loop */

    IxPerfProfAccBusPmuModeEvents eventsPerCounter;
    IxPerfProfAccStatus status;
    IxFeatureCtrlDeviceId deviceType=IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;

    deviceType = ixFeatureCtrlDeviceRead();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
	return IX_PERFPROF_ACC_STATUS_COMPONENT_NOT_SUPPORTED;
    }
       
    #ifdef __vxworks
    /* Initialize all virtual addresses for vxWorks */
    esrVirtualAddress =  (UINT32)IX_OSAL_MEM_MAP(PMU_ESR,IX_OSAL_IXP400_PMU_MAP_SIZE);
    srVirtualAddress  =  (UINT32)IX_OSAL_MEM_MAP(PMU_SR,IX_OSAL_IXP400_PMU_MAP_SIZE);
    #endif

    if (IX_PERFPROF_ACC_BUS_PMU_MODE_HALT == mode )
    {
        return (IX_PERFPROF_ACC_STATUS_BUS_PMU_MODE_ERROR);
    }
    status = ixPerfProfAccLock();
    if (IX_PERFPROF_ACC_STATUS_SUCCESS != status)
    {
        return IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS;
    }

    else
    {
        /* Store input parameters into a structure */
        eventsPerCounter.counterMode = mode;
        eventsPerCounter.counterEvent1 = pecEvent1;
        eventsPerCounter.counterEvent2 = pecEvent2;
        eventsPerCounter.counterEvent3 = pecEvent3;
        eventsPerCounter.counterEvent4 = pecEvent4;
        eventsPerCounter.counterEvent5 = pecEvent5;
        eventsPerCounter.counterEvent6 = pecEvent6;
        eventsPerCounter.counterEvent7 = pecEvent7;

        /* Initialise 59 bit supporting counters */
        for (PecCounter = 0; IX_PERFPROF_ACC_BUS_PMU_MAX_PECS > PecCounter; PecCounter++)
        {
            upper32BitCounter[PecCounter] = 0;
        } /* End of for loop */

        /* Bind interrupt handler to increment counter when an overflow occurs */
        if (IX_SUCCESS != ixOsalIrqBind (
	                      IX_OSAL_IXP400_AHB_PMU_IRQ_LVL,
                              (IxOsalVoidFnVoidPtr)ixPerfProfAccBusPmuPecOverflowHdlr,
                              NULL))
        {
            ixPerfProfAccUnlock();
            return IX_PERFPROF_ACC_STATUS_FAIL;
        }
        else
        {
            /* Call setup function to setup Event Status Register */
            status = ixPerfProfAccBusPmuSetup(eventsPerCounter);
            if (IX_SUCCESS != status)
            {
                /* Unbind the interrupt */
                ixOsalIrqUnbind (IX_OSAL_IXP400_AHB_PMU_IRQ_LVL);
#ifdef __vxworks
                IX_OSAL_MEM_UNMAP(esrVirtualAddress);
                IX_OSAL_MEM_UNMAP(srVirtualAddress);
#endif
                ixPerfProfAccUnlock();
                startFlag = FALSE;
            }
            else
            {
                startFlag = TRUE;
            }
        } /* End of if-else */

        return status;
    } /* End of if-else */
} /* End of function ixPerfProfAccBusPmuStart */

/**
 *
 * Function to check the validity of choices for PEC1 to PEC7 for North Mode
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to North
 *
 **/
IxPerfProfAccStatus
ixPerfProfAccBusPmuNorthCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents)
{
    /* Variable for event select register. Set to 0 initially */
    UINT32 esrValue = 0;

    /* Check if PEC1 selection is within range for North Mode */
    if((modeEvents.counterEvent1 < IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEA_GRANT_SELECT)||
        (modeEvents.counterEvent1 > IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEC_REQ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR;
    }
    /* Event select for PEC1 */
    SET_PMU_PEC(esrValue,PEC1,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent1]);


    /* Check if PEC2 selection is within range for North Mode */
    if((modeEvents.counterEvent2 < IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEA_XFER_SELECT)||
        (modeEvents.counterEvent2 > IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEC_OWN_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR;
    }
    /* Event select for PEC2 */
    SET_PMU_PEC(esrValue,PEC2,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent2]);


    /* Check if PEC3 selection is within range for North Mode */
    if((modeEvents.counterEvent3 < IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEA_RETRY_SELECT)||
        (modeEvents.counterEvent3 > IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEC_WRITE_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR;
    }
    /* Event select for PEC3 */
    SET_PMU_PEC(esrValue,PEC3,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent3]);


    /* Check if PEC4 selection is within range for North Mode */
    if((modeEvents.counterEvent4 < IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_ABB_SPLIT_SELECT)||
        (modeEvents.counterEvent4 > IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEC_READ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR;
    }
    /* Event select for PEC4 */
    SET_PMU_PEC(esrValue,PEC4,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent4]);


    /* Check if PEC5 selection is within range for North Mode */
    if((modeEvents.counterEvent5 < IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_GRANT_SELECT)||
        (modeEvents.counterEvent5 > IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_READ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR;
    }
    /* Event select for PEC5 */
    SET_PMU_PEC(esrValue,PEC5,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent5]);


    /* Check if PEC6 selection is within range for North Mode */
    if((modeEvents.counterEvent6 < IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_GRANT_SELECT)||
        (modeEvents.counterEvent6 > IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_READ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR;
    }
    /* Event select for PEC6 */
    SET_PMU_PEC(esrValue,PEC6,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent6]);


    /* Check if PEC7 selection is within range for North Mode */
    if(modeEvents.counterEvent7 != IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT)
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR;
    }
    /* Event select for PEC7 */
    SET_PMU_PEC(esrValue,PEC7,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent7]);

    SET_PMU_MODE(esrValue, IX_PERFPROF_ACC_BUS_PMU_MODE_NORTH);
    SET_PMU_ESR(esrValue);

    return IX_PERFPROF_ACC_STATUS_SUCCESS;
} /* end of ixPerfProfAccBusPmuNorthCheckAndSelect () */


/**
 *
 * Function to check the validity of choices for PEC1 to PEC7 for South Mode
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to South
 *
 **/
IxPerfProfAccStatus
ixPerfProfAccBusPmuSouthCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents)
{
    /* Variable for event select register. Set to 0 initially */
    UINT32 esrValue = 0;

    /* Check if PEC1 selection is within range for South Mode */
    if((modeEvents.counterEvent1 < IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_GSKT_GRANT_SELECT)||
        (modeEvents.counterEvent1 > IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_APB_REQ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR;
    }
    /* Event select for PEC1 */
    SET_PMU_PEC(esrValue,PEC1,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent1]);


    /* Check if PEC2 selection is within range for South Mode */
    if((modeEvents.counterEvent2 < IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_GSKT_XFER_SELECT)||
        (modeEvents.counterEvent2 > IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_APB_OWN_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR;
    }
    /* Event select for PEC2 */
    SET_PMU_PEC(esrValue,PEC2,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent2]);


    /* Check if PEC3 selection is within range for South Mode */
    if((modeEvents.counterEvent3 < IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_GSKT_RETRY_SELECT)||
        (modeEvents.counterEvent3 > IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_APB_WRITE_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR;
    }
    /* Event select for PEC3 */
    SET_PMU_PEC(esrValue,PEC3,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent3]);


    /* Check if PEC4 selection is within range for South Mode */
    if((modeEvents.counterEvent4 < IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_PCI_SPLIT_SELECT)||
        (modeEvents.counterEvent4 > IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_READ_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR;
    }
    /* Event select for PEC4 */
    SET_PMU_PEC(esrValue,PEC4,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent4]);


    /* Check if PEC5 selection is within range for South Mode */
    if((modeEvents.counterEvent5 < IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_GRANT_SELECT)||
        (modeEvents.counterEvent5 > IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_BUS_IDLE_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR;
    }
    /* Event select for PEC5 */
    SET_PMU_PEC(esrValue,PEC5,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent5]);


    /* Check if PEC6 selection is within range for South Mode */
    if((modeEvents.counterEvent6 < IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_GRANT_SELECT)||
        (modeEvents.counterEvent6 > IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_BUS_WRITE_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR;
    }
    /* Event select for PEC6 */
    SET_PMU_PEC(esrValue,PEC6,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent6]);


    /* Check if PEC7 selection is within range for South Mode */
    if((modeEvents.counterEvent7 < IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_RETRY_SELECT)||
        (modeEvents.counterEvent7 > IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR;
    }
    /* Event select for PEC7 */
    SET_PMU_PEC(esrValue,PEC7,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent7]);

    SET_PMU_MODE(esrValue, IX_PERFPROF_ACC_BUS_PMU_MODE_SOUTH);
    SET_PMU_ESR(esrValue);

    return IX_PERFPROF_ACC_STATUS_SUCCESS;
} /* end of ixPerfProfAccBusPmuSouthCheckAndSelect () */


/**
 *
 * Function to check the validity of choices for PEC1 to PEC7 for Sdram Mode
 * Return errors if choice/s are not valid, otherwise set Event Select Register
 * and mode to Sdram
 *
 **/
IxPerfProfAccStatus
ixPerfProfAccBusPmuSdramCheckAndSelect (IxPerfProfAccBusPmuModeEvents modeEvents)
{
    /* Variable for event select register. Set to 0 initially */
    UINT32 esrValue = 0;

    /* Check if PEC1 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent1 < IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_0_HIT_SELECT)||
        (modeEvents.counterEvent1 > IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_7_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR;
    }
    /* Event select for PEC1 */
    SET_PMU_PEC(esrValue,PEC1,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent1]);


    /* Check if PEC2 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent2 < IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_1_HIT_SELECT)||
        (modeEvents.counterEvent2 > IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_0_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR;
    }
    /* Event select for PEC2 */
    SET_PMU_PEC(esrValue,PEC2,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent2]);


    /* Check if PEC3 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent3 < IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_2_HIT_SELECT)||
        (modeEvents.counterEvent3 > IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_1_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR;
    }
    /* Event select for PEC3 */
    SET_PMU_PEC(esrValue,PEC3,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent3]);


    /* Check if PEC4 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent4 < IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_3_HIT_SELECT)||
        (modeEvents.counterEvent4 > IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_2_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR;
    }
    /* Event select for PEC4 */
    SET_PMU_PEC(esrValue,PEC4,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent4]);


    /* Check if PEC5 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent5 < IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_4_HIT_SELECT)||
        (modeEvents.counterEvent5 > IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_3_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR;
    }
    /* Event select for PEC5 */
    SET_PMU_PEC(esrValue,PEC5,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent5]);


    /* Check if PEC6 selection is within range for Sdram Mode */
    if((modeEvents.counterEvent6 < IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_5_HIT_SELECT)||
        (modeEvents.counterEvent6 > IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_4_MISS_SELECT))
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR;
    }
    /* Event select for PEC6 */
    SET_PMU_PEC(esrValue,PEC6,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent6]);


    /* Check if PEC7 selection is within range for Sdram Mode */
    if(modeEvents.counterEvent7 != IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT)
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR;
    }
    /* Event select for PEC7 */
    SET_PMU_PEC(esrValue,PEC7,ixPerfProfAccBusPmuEventMap[modeEvents.counterEvent7]);

    SET_PMU_MODE(esrValue, IX_PERFPROF_ACC_BUS_PMU_MODE_SDRAM);
    SET_PMU_ESR(esrValue);

    return IX_PERFPROF_ACC_STATUS_SUCCESS;
} /* end of ixPerfProfAccBusPmuSdramCheckAndSelect () */


/* Function to setup events for each counter based on the mode */
IxPerfProfAccStatus
ixPerfProfAccBusPmuSetup (IxPerfProfAccBusPmuModeEvents modeEvents)
{

    ESR_RESET();  /* Reset overflow bits in Status Register. */

    /* Set the register values for north mode based on parameters input */
    if (modeEvents.counterMode == IX_PERFPROF_ACC_BUS_PMU_MODE_NORTH)
    {
        return (ixPerfProfAccBusPmuNorthCheckAndSelect(modeEvents));
    }

    /* Set the register values for South mode based on parameters input */
    else if (modeEvents.counterMode == IX_PERFPROF_ACC_BUS_PMU_MODE_SOUTH)
    {
        return (ixPerfProfAccBusPmuSouthCheckAndSelect(modeEvents));
    }

    /* Set the register values for Sdram mode based on parameters input */
    else if (modeEvents.counterMode == IX_PERFPROF_ACC_BUS_PMU_MODE_SDRAM)
    {
        return (ixPerfProfAccBusPmuSdramCheckAndSelect(modeEvents));
    } /* End if(modeEvents.counterMode == IX_PERFPROF_ACC_BUS_PMU_MODE_SDRAM)*/

    return IX_PERFPROF_ACC_STATUS_BUS_PMU_MODE_ERROR;
} /* End of function ixPerfProfAccBusPmuSetup*/

/* Function stops all collection of data */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccBusPmuStop (void)
{
    UINT32 esrValue;

    if (FALSE == startFlag)
    {
        return IX_PERFPROF_ACC_STATUS_BUS_PMU_START_NOT_CALLED;
    }

    else
    {
        /* Set mode to halt */
        esrValue = GET_PMU_ESR;
        SET_PMU_MODE(esrValue, IX_PERFPROF_ACC_BUS_PMU_MODE_HALT);
        SET_PMU_ESR(esrValue);
        /* Verify if the ESR has been set to halt mode */
        if (((GET_PMU_ESR) & ESR_MODE_MASK) != IX_PERFPROF_ACC_BUS_PMU_MODE_HALT)
        {
            ixPerfProfAccUnlock();
            return IX_PERFPROF_ACC_STATUS_FAIL;
        }

        else
        {
            /* Unbind the interrupt */
            ixOsalIrqUnbind(IX_OSAL_IXP400_AHB_PMU_IRQ_LVL);
            startFlag = FALSE ;
            ixPerfProfAccUnlock();
            #ifdef __vxworks
            IX_OSAL_MEM_UNMAP(esrVirtualAddress);
            IX_OSAL_MEM_UNMAP(srVirtualAddress);
            #endif
            return IX_PERFPROF_ACC_STATUS_SUCCESS;
        }
    } /* End of if-else */

}  /* End of function ixPerfProfAccBusPmuStop*/

/* Function that returns the values required by the calling function */
PUBLIC void
ixPerfProfAccBusPmuResultsGet (IxPerfProfAccBusPmuResults *BusPmuResults)
{
    int pecCounter;
    IxFeatureCtrlDeviceId deviceType=IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;
 
    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    /*error check the parameter*/
    if (NULL == BusPmuResults)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccBusPmuResultsGet - BusPmuResults is invalid\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    #ifdef __vxworks
    for (pecCounter = 0; IX_PERFPROF_ACC_BUS_PMU_MAX_PECS > pecCounter; pecCounter++)
    {
        pecVirtualAddress = (UINT32)IX_OSAL_MEM_MAP(PMU_CNT_ADDR(pecCounter+1), 
						    IX_OSAL_IXP400_PMU_MAP_SIZE);
        BusPmuResults->statsToGetLower27Bit[pecCounter] = PMU_CNT_GET();
        BusPmuResults->statsToGetUpper32Bit[pecCounter] = upper32BitCounter[pecCounter];
        IX_OSAL_MEM_UNMAP(pecVirtualAddress);
    }

    #elif defined(__linux)
    for (pecCounter = 0; IX_PERFPROF_ACC_BUS_PMU_MAX_PECS > pecCounter; pecCounter++)
    {
        BusPmuResults->statsToGetLower27Bit[pecCounter] = PMU_CNT_GET(pecCounter+1);
        BusPmuResults->statsToGetUpper32Bit[pecCounter] = upper32BitCounter[pecCounter];
    }
    
    #endif /* __vxworks */

} /* End of function ixPerfProfAccBusPmuResultsGet */


PUBLIC void
ixPerfProfAccBusPmuPMSRGet (UINT32 *pmsrValue)
{
    IxFeatureCtrlDeviceId deviceType=IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X;
 
    deviceType = ixFeatureCtrlDeviceRead ();
    if(IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X == deviceType)
    {
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAcc - This PPU component is not supported\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    /*error check the parameter*/
    if (NULL == pmsrValue)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccBusPmuPMSRGet - pmsrValue is invalid\n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    /* Get virtual address that is mapped depending on LE or BE */
    #ifdef __vxworks
    pmsrVirtualAddress = (UINT32)IX_OSAL_MEM_MAP(PMU_PMSR,IX_OSAL_IXP400_PMU_MAP_SIZE);
    #endif

    /* Assign PMSR value to pointer */
    *pmsrValue = PMU_CNT_PMSR();

    /* Release dynamic virtual address mapping */
    #ifdef __vxworks
    IX_OSAL_MEM_UNMAP(pmsrVirtualAddress);
    #endif
}


/* Interrupt handler routine. */
void
ixPerfProfAccBusPmuPecOverflowHdlr (void *voidParam)
{
    UINT32 statusRegisterValue;
    UINT32 maskingBit;
    int pecCounter;

    /* Read value of the status register */
    statusRegisterValue = GET_SR();
    SR_RESET();
    maskingBit = STATUS_REGISTER_BIT1_MASK;

    for (pecCounter = 0; IX_PERFPROF_ACC_BUS_PMU_MAX_PECS > pecCounter; pecCounter++)
    {
        if (statusRegisterValue & maskingBit)
        {
            upper32BitCounter[pecCounter] += 1;
        }

        maskingBit *= 2;
    }
}/* end of ixPerfProfAccBusPmuPecOverflowHdlr */
