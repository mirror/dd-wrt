/**
 * @file IxHssAccCodeletConfig.c
 *
 * @date 30 May 2002
 *
 * @brief This file contains the config implementation of the HSS Access
 * Codelet.
 *
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
 * @sa IxHssAccCodelet.h
 */

/*
 * Put the user defined include files required.
 */

#include "IxHssAcc.h"

#include "IxHssAccCodelet.h"
#include "IxHssAccCodeletCom.h"
#include "IxHssAccCodeletPkt.h"
#include "IxHssAccCodeletChan.h"

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/**************************************************/
/*               FRAMER CONFIGURATION             */
/**************************************************/

static IxHssAccPortConfig defaultHssPortConfig =
{
    /* Frame sync type = active low */
    IX_HSSACC_FRM_SYNC_ACTIVE_LOW,
    /* Frame sync output enable = output rising */
    IX_HSSACC_FRM_SYNC_OUTPUT_RISING,
    /* Frame sync clock edge = rising */
    IX_HSSACC_CLK_EDGE_RISING,
    /* Data clock edge = falling */
    IX_HSSACC_CLK_EDGE_FALLING,
    /* Clock direction = output */
    IX_HSSACC_SYNC_CLK_DIR_OUTPUT,
    /* Frame usage = enabled */
    IX_HSSACC_FRM_PULSE_ENABLED,
    /* Data rate = clock rate */
    IX_HSSACC_CLK_RATE,
    /* Data polarity = same */
    IX_HSSACC_DATA_POLARITY_SAME,
    /* Data endianness = lsb endian */
    IX_HSSACC_LSB_ENDIAN,
    /* Drain mode = normal */
    IX_HSSACC_TX_PINS_NORMAL,
    /* FBit usage = data */
    IX_HSSACC_SOF_DATA,
    /* Data enable = data */
    IX_HSSACC_DE_DATA,
    /* 56K type = low */
    IX_HSSACC_TXSIG_LOW,
    /* Unassigned type = low */
    IX_HSSACC_TXSIG_LOW,
    /* FBit type = fifo */
    IX_HSSACC_FB_FIFO,
    /* 56K endianness = bit 7 unused */
    IX_HSSACC_56KE_BIT_7_UNUSED,
    /* 56K selection = 32/8 data */
    IX_HSSACC_56KS_32_8_DATA,
    /* Frame offset = 0 */
    0,
    /* Frame size = 1024 (4 trunks * 32 timeslots * 8 bits) */
    1024,
};

/**************************************************/
/*              TIMESLOT CONFIGURATION            */
/**************************************************/

/*
 * redefinitions using shorter names to make following TDM Map more
 * readable
 */
#define TS_UNASSIGNED IX_HSSACC_TDMMAP_UNASSIGNED
#define TS_HDLC       IX_HSSACC_TDMMAP_HDLC
#define TS_VOICE64K   IX_HSSACC_TDMMAP_VOICE64K
#define TS_VOICE56K   IX_HSSACC_TDMMAP_VOICE56K

/* TDM Map indicates how each timeslot is used on the 4 E1 trunks */
static IxHssAccTdmSlotUsage tdmMap[IX_HSSACC_TSLOTS_PER_HSS_PORT] = 
{/* Trunk 0        Trunk 1        Trunk 2        Trunk 3     */
    TS_VOICE64K  , TS_VOICE64K  , TS_VOICE64K  , TS_VOICE64K  , /* TS 00 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 01 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 02 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 03 */
    TS_VOICE56K  , TS_VOICE56K  , TS_VOICE56K  , TS_VOICE56K  , /* TS 04 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 05 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 06 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 07 */
    TS_VOICE64K  , TS_VOICE64K  , TS_VOICE64K  , TS_VOICE64K  , /* TS 08 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 09 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 10 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 11 */
    TS_VOICE56K  , TS_VOICE56K  , TS_VOICE56K  , TS_VOICE56K  , /* TS 12 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 13 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 14 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 15 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 16 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 17 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 18 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 19 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 20 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 21 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 22 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 23 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 24 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 25 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 26 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 27 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, /* TS 28 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 29 */
    TS_HDLC      , TS_HDLC      , TS_HDLC      , TS_HDLC      , /* TS 30 */
    TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED, TS_UNASSIGNED  /* TS 31 */
};

/*
 * Function prototypes.
 */
IxHssAccTdmSlotUsage
ixHssAccCodeletChannelisedTimeslotGet (unsigned channelIndex);

/*
 * Function definition: ixHssAccCodeletChannelisedTimeslotGet
 */
IxHssAccTdmSlotUsage
ixHssAccCodeletChannelisedTimeslotGet (
    unsigned channelIndex)
{
    unsigned tsIndex;
    unsigned numVoiceChans = 0;

    for (tsIndex = 0; tsIndex < IX_HSSACC_TSLOTS_PER_HSS_PORT; tsIndex++)
    {
        if ((tdmMap[tsIndex] == IX_HSSACC_TDMMAP_VOICE64K) ||
            (tdmMap[tsIndex] == IX_HSSACC_TDMMAP_VOICE56K))
        {
            numVoiceChans++;
        }

        if (numVoiceChans == channelIndex + 1)
        {
            return tdmMap[tsIndex];
        }
    }

    return IX_HSSACC_TDMMAP_UNASSIGNED;
}

/*
 * Function definition: ixHssAccCodeletConfigure
 */

void
ixHssAccCodeletConfigure (
    IxHssAccHssPort hssPortId)
{
    IxHssAccConfigParams configParams;
    IX_STATUS status;

    /* Set the HSS tx port configuration */
    configParams.txPortConfig = defaultHssPortConfig;

    /* Set the HSS rx port configuration */
    configParams.rxPortConfig = defaultHssPortConfig;

    /* HssChannelized Number of Channels = 32 */
    configParams.numChannelised = IX_HSSACC_CODELET_CHAN_NUM_CHANS;

    /* HssPacketized Number of Packet-Pipes = 4 */
    configParams.hssPktChannelCount = 4;

    /* HssChannelized Idle Pattern = 0x7F */
    configParams.channelisedIdlePattern =
        IX_HSSACC_CODELET_CHAN_IDLE_PATTERN;

    /* Loopback = TRUE/FALSE (parameter) */
    configParams.loopback = ixHssAccCodeletHssLoopbackGet ();

    /* HssPacketized Pipes 0-3 Idle Patterns = 0x7F7F7F7F */
    configParams.packetizedIdlePattern =
        IX_HSSACC_CODELET_PKT_IDLE_PATTERN;

    /* Clock speed = 8192 */
    configParams.clkSpeed = IX_HSSACC_CLK_SPEED_8192KHZ;

    if(IX_HSSACC_HSS_PORT_0 == hssPortId)
    {
        /* configure the HSS port 0 */
        status = ixHssAccPortInit (
            hssPortId, &configParams, tdmMap,
           ixHssAccCodeletLastHssErrorHssPort0Callback);   
    }
    else 
    {
        /* configure the HSS port 1 */
        status = ixHssAccPortInit (
            hssPortId, &configParams, tdmMap,
            ixHssAccCodeletLastHssErrorHssPort1Callback);   
    }  

    /* if there was any problem then update stats */
    if (status != IX_SUCCESS)
    {
        stats[hssPortId].gen.portInitFails++;
    }
}








