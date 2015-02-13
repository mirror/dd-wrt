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
 *   signed SignalToNoise1 (struct plc * plc);
 *
 *   plc.h
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SIGNALTONOISE1_SOURCE
#define SIGNALTONOISE1_SOURCE

#include <stdio.h>
#include <stdint.h>

#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/endian.h"
#include "../plc/plc.h"

#ifndef TONEMAPS1_SOURCE

static uint8_t const mod2bits [AMP_BITS] =

{
	0,
	1,
	2,
	3,
	4,
	6,
	8,
	10,
	12
};

static uint8_t const mod2db [AMP_BITS] =

{
	0,
	2,
	4,
	7,
	10,
	16,
	22,
	28,
	36
};


#endif

signed SignalToNoise1 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	byte tonemap [PLC_SLOTS + 1][AMP_CARRIERS >> 1];
	byte GIL [PLC_SLOTS];
	byte AGC [PLC_SLOTS];
	double SNR [PLC_SLOTS] =
	{
		0.0
	};
	double BPC [PLC_SLOTS] =
	{
		0.0
	};
	double AvgSNR = 0;
	double AvgBPC = 0;
	unsigned carriers = 0;
	unsigned carrier = 0;
	unsigned slots = 0;
	unsigned slot = 0;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_tone_slot_char_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MACADDRESS [ETHER_ADDR_LEN];
		uint8_t TMSLOT;
	}
	* request = (struct vs_tone_slot_char_request *) (message);
	struct __packed vs_tone_slot_char_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t TMSLOT;
		uint8_t NUMTMS;
		uint16_t TMNUMACTCARRIERS;
		uint8_t MOD_CARRIER [(INT_CARRIERS + 1) >> 1];
		uint8_t GIL;
		uint8_t AGC;
	}
	* confirm = (struct vs_tone_slot_char_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_RX_TONE_MAP_CHAR | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	memcpy (request->MACADDRESS, plc->RDA, sizeof (request->MACADDRESS));
	request->TMSLOT = 0;
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMME (plc, 0, (VS_RX_TONE_MAP_CHAR | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	carriers = LE16TOH (confirm->TMNUMACTCARRIERS);
	slots = confirm->NUMTMS;
	if (!slots)
	{
		error (PLC_EXIT (plc), ECANCELED, "No Tone Maps Available");
		return (-1);
	}
	memset (tonemap, 0, sizeof (tonemap));
	for (slot = 0; slot < slots; slot++)
	{
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_RX_TONE_MAP_CHAR | MMTYPE_REQ));
		memcpy (request->MACADDRESS, plc->RDA, sizeof (request->MACADDRESS));
		request->TMSLOT = slot;
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		if (ReadMME (plc, 0, (VS_RX_TONE_MAP_CHAR | MMTYPE_CNF)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		memcpy (&tonemap [slot][0], confirm->MOD_CARRIER, sizeof (confirm->MOD_CARRIER));
		GIL [slot] = confirm->GIL;
		AGC [slot] = confirm->AGC;
	}
	while (carrier < carriers)
	{
		unsigned value = 0;
		unsigned index = carrier >> 1;
		for (slot = 0; slot < PLC_SLOTS; slot++)
		{
			value = tonemap [slot][index];
			if ((carrier & 1))
			{
				value >>= 4;
			}
			value &= 0x0F;
			BPC [slot] += mod2bits [value];
			SNR [slot] += mod2db [value];
			AvgBPC += mod2bits [value];
			AvgSNR += mod2db [value];
		}
		carrier++;
	}
	AvgBPC /= carriers;
	AvgBPC /= slots;
	AvgSNR /= carriers;
	AvgSNR /= slots;
	printf (" SNR");
	for (slot = 0; slot < slots; slot++)
	{
		printf (" %8.3f", SNR [slot] / carriers);
	}
	printf (" %8.3f", AvgSNR);
	printf (" \n");
	printf (" ATN");
	for (slot = 0; slot < slots; slot++)
	{
		printf (" %8.3f", SNR [slot] / carriers - 60);
	}
	printf (" %8.3f", AvgSNR - 60);
	printf (" \n");
	printf (" BPC");
	for (slot = 0; slot < slots; slot++)
	{
		printf (" %8.3f", BPC [slot] / carriers);
	}
	printf (" %8.3f", AvgBPC);
	printf (" \n");
	printf (" AGC");
	for (slot = 0; slot < slots; slot++)
	{
		printf (" %02d", AGC [slot]);
	}
	printf (" \n");
	printf (" GIL");
	for (slot = 0; slot < slots; slot++)
	{
		printf (" %02d", GIL [slot]);
	}
	printf (" \n");
	return (0);
}


#endif

