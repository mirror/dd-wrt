/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   signed LinkStatistics (struct plc * plc);
 *
 *   plc.h
 *
 *   this plugin for plcstat reads device link statistics using a
 *   VS_LNK_STATS message and displays information on stdout; since
 *   the output may not suite all requirements, users are encouraged
 *   to modify as needed; there is way too much data displayed here;
 *
 *   some code is duplicated for the sake of modularity;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef LINKSTATISTICS_SOURCE
#define LINKSTATISTICS_SOURCE

#include <stdint.h>
#include <memory.h>
#include <inttypes.h>

#include "../plc/plc.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define PAD ' '
#define DIGITS 16

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct __packed transmit

{
	uint64_t NUMTXMPDU_ACKD;
	uint64_t NUMTXMPDU_COLL;
	uint64_t NUMTXMPDU_FAIL;
	uint64_t NUMTXPBS_PASS;
	uint64_t NUMTXPBS_FAIL;
}

transmit;
typedef struct __packed receive

{
	uint64_t NUMRXMPDU_ACKD;
	uint64_t NUMRXMPDU_FAIL;
	uint64_t NUMRXPBS_PASS;
	uint64_t NUMRXPBS_FAIL;
	uint64_t SUMTURBOBER_PASS;
	uint64_t SUMTURBOBER_FAIL;
	uint8_t NUMRXINTERVALS;
	uint8_t RXINTERVALSTATS [1];
}

receive;
typedef struct __packed interval

{
	uint8_t RXPHYRATE_MBPS_0;
	uint64_t NUMRXPBS_PASS;
	uint64_t NUMRXPBS_FAIL;
	uint64_t SUMTURBOBER_PASS;
	uint64_t SUMTURBOBER_FAIL;
}

interval;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *
 *   unsigned error_rate (uint64_t passed, uint64_t failed);
 *
 *   compute error rate for a given quantity; the error rate is the
 *   ratio of failures to attempts;
 *
 *--------------------------------------------------------------------*/

static float error_rate (uint64_t passed, uint64_t failed)

{
	if ((passed) || (failed))
	{
		return ((float)(failed * 100) / (float)(passed + failed));
	}
	return (0);
}


/*====================================================================*
 *
 *   float fec_bit_error_rate (struct receive * receive);
 *
 *   compute the FEC-BER from the VS_LNK_STATS when DIRECTION=1 and
 *   LID=0xF8;
 *
 *--------------------------------------------------------------------*/

static float fec_bit_error_rate (struct receive * receive)

{
	float FECBitErrorRate = 0;
	if (receive->SUMTURBOBER_PASS || receive->SUMTURBOBER_FAIL)
	{
		float TotalSumOfBitError = 100 * (float)(receive->SUMTURBOBER_PASS + receive->SUMTURBOBER_FAIL);
		float TotalSumOfBits = 8 * 520 * (float)(receive->NUMRXPBS_PASS + receive->NUMRXPBS_FAIL);
		FECBitErrorRate = TotalSumOfBitError / TotalSumOfBits;
	}
	return (FECBitErrorRate);
}


/*====================================================================*
 *
 *   void transmit (struct transmit * transmit);
 *
 *   display transmit statistics in fixed field format;
 *
 *--------------------------------------------------------------------*/

static void TransmitStatistics (struct transmit * transmit)

{
	printf ("    TX");
	printf (" %20" PRId64, transmit->NUMTXPBS_PASS);
	printf (" %20" PRId64, transmit->NUMTXPBS_FAIL);
	printf (" %6.2f%%", error_rate (transmit->NUMTXPBS_PASS, transmit->NUMTXPBS_FAIL));
	printf (" %20" PRId64, transmit->NUMTXMPDU_ACKD);
	printf (" %20" PRId64, transmit->NUMTXMPDU_FAIL);
	printf (" %20" PRId64, transmit->NUMTXMPDU_COLL);
	printf (" %6.2f%%", error_rate (transmit->NUMTXMPDU_ACKD, transmit->NUMTXMPDU_FAIL));
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *   void Receive (struct receive * receive);
 *
 *   display receive statistics in fixed field format;
 *
 *--------------------------------------------------------------------*/

static void ReceiveStatistics (struct receive * receive)

{
	printf ("    RX");
	printf (" %20" PRId64, receive->NUMRXPBS_PASS);
	printf (" %20" PRId64, receive->NUMRXPBS_FAIL);
	printf (" %6.2f%%", error_rate (receive->NUMRXPBS_PASS, receive->NUMRXPBS_FAIL));
	printf (" %20" PRId64, receive->NUMRXMPDU_ACKD);
	printf (" %20" PRId64, receive->NUMRXMPDU_FAIL);
	printf (" %6.2f%%", error_rate (receive->NUMRXMPDU_ACKD, receive->NUMRXMPDU_FAIL));
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *   void Receive (struct receive * receive);
 *
 *   display receive statistics in fixed field format for each slot;
 *   the last line sumarizes results for all slots;
 *
 *--------------------------------------------------------------------*/

static void Receive2 (struct receive * receive)

{
	struct interval * interval = (struct interval *)(receive->RXINTERVALSTATS);
	uint8_t slot = 0;
	while (slot < receive->NUMRXINTERVALS)
	{
		printf (" %1d", slot);
		printf (" %3d", interval->RXPHYRATE_MBPS_0);
		printf (" %20" PRId64, interval->NUMRXPBS_PASS);
		printf (" %20" PRId64, interval->NUMRXPBS_FAIL);
		printf (" %6.2f%%", error_rate (interval->NUMRXPBS_PASS, interval->NUMRXPBS_FAIL));
		printf (" %20" PRId64, interval->SUMTURBOBER_PASS);
		printf (" %20" PRId64, interval->SUMTURBOBER_FAIL);
		printf (" %6.2f%%", error_rate (interval->SUMTURBOBER_PASS, interval->SUMTURBOBER_FAIL));
		printf ("\n");
		interval++;
		slot++;
	}
	printf ("   ALL");
	printf (" %20" PRId64, receive->NUMRXPBS_PASS);
	printf (" %20" PRId64, receive->NUMRXPBS_FAIL);
	printf (" %6.2f%%", error_rate (receive->NUMRXPBS_PASS, receive->NUMRXPBS_FAIL));
	printf (" %20" PRId64, receive->SUMTURBOBER_PASS);
	printf (" %20" PRId64, receive->SUMTURBOBER_FAIL);
	printf (" %6.2f%%", error_rate (receive->SUMTURBOBER_PASS, receive->SUMTURBOBER_FAIL));
	printf (" %6.2f%%", fec_bit_error_rate (receive));
	printf ("\n");
	return;
}


/*====================================================================*
 *
 *   signed LinkStatistics (struct plc * plc);
 *
 *
 *--------------------------------------------------------------------*/

signed LinkStatistics (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_lnk_stats_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MCONTROL;
		uint8_t DIRECTION;
		uint8_t LID;
		uint8_t MACADDRESS [ETHER_ADDR_LEN];
	}
	* request = (struct vs_lnk_stats_request *) (message);
	struct __packed vs_lnk_stats_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t DIRECTION;
		uint8_t LID;
		uint8_t TEI;
		uint8_t LSTATS [1];
	}
	* confirm = (struct vs_lnk_stats_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_LNK_STATS | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->MCONTROL = plc->pushbutton;
	request->DIRECTION = plc->module;
	request->LID = plc->action;
	memcpy (request->MACADDRESS, plc->RDA, sizeof (request->MACADDRESS));
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMME (plc, 0, (VS_LNK_STATS | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	if (confirm->DIRECTION == 0)
	{
		printf ("   DIR");
		printf (" ----------- PBs PASS");
		printf (" ----------- PBs FAIL");
		printf (" PBs ERR");
		printf (" ---------- MPDU ACKD");
		printf (" ---------- MPDU FAIL");
		printf ("\n");
		TransmitStatistics ((struct transmit *)(confirm->LSTATS));
		printf ("\n");
	}
	if (confirm->DIRECTION == 1)
	{
		printf ("   DIR");
		printf (" ----------- PBs PASS");
		printf (" ----------- PBs FAIL");
		printf (" PBs ERR");
		printf (" ---------- MPDU ACKD");
		printf (" ---------- MPDU FAIL");
		printf ("\n");
		ReceiveStatistics ((struct receive *)(confirm->LSTATS));
		printf ("\n");
		printf ("   PHY");
		printf (" ----------- PBs PASS");
		printf (" ----------- PBs FAIL");
		printf (" PBs ERR");
		printf (" ----------- BER PASS");
		printf (" ----------- BER FAIL");
		printf (" BER ERR");
		printf ("\n");
		Receive2 ((struct receive *)(confirm->LSTATS));
		printf ("\n");
	}
	if (confirm->DIRECTION == 2)
	{
		printf ("   DIR");
		printf (" ----------- PBs PASS");
		printf (" ----------- PBs FAIL");
		printf (" PBs ERR");
		printf (" ---------- MPDU ACKD");
		printf (" ---------- MPDU FAIL");
		printf ("\n");
		TransmitStatistics ((struct transmit *)(confirm->LSTATS));
		ReceiveStatistics ((struct receive *)(confirm->LSTATS + sizeof (struct transmit)));
		printf ("\n");
		printf ("   PHY");
		printf (" ----------- PBs PASS");
		printf (" ----------- PBs FAIL");
		printf (" PBs ERR");
		printf (" ----------- BER PASS");
		printf (" ----------- BER FAIL");
		printf (" BER ERR");
		printf ("\n");
		Receive2 ((struct receive *)(confirm->LSTATS + sizeof (struct transmit)));
		printf ("\n");
	}
	return (0);
}


#endif

