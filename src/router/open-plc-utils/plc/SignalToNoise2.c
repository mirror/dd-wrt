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
 *   signed SignalToNoise2 (struct plc * plc);
 *
 *   amp.h
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SIGNALTONOISE2_SOURCE
#define SIGNALTONOISE2_SOURCE

#include <stdio.h>
#include <stdint.h>

#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/endian.h"
#include "../plc/plc.h"
#include "../mme/mme.h"

#ifndef TONEMAPS2_SOURCE

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


#endif

#ifndef TONEMAPS2_SOURCE

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

signed SignalToNoise2 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	byte tonemap [PLC_SLOTS + 1][AMP_CARRIERS >> 1];
	uint16_t GIL [PLC_SLOTS];
	uint16_t AGC [PLC_SLOTS];
	double SNR [PLC_SLOTS];
	double BPC [PLC_SLOTS];
	double AvgSNR;
	double AvgBPC;
	uint16_t extent = 0;
	uint16_t active = 0;
	uint16_t carriers = AMP_CARRIERS;
	uint16_t carrier = 0;
	uint8_t slots = PLC_SLOTS;
	uint8_t slot = 0;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_rx_tone_map_char_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
		uint8_t MME_SUBVER;
		uint8_t Reserved1 [3];
		uint8_t MACADDRESS [ETHER_ADDR_LEN];
		uint8_t TMSLOT;
		uint8_t COUPLING;
	}
	* request = (struct vs_rx_tone_map_char_request *) (message);
	struct __packed vs_rx_tonemap_char_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_fmi qualcomm;
		uint8_t MSTATUS;
		uint8_t Reserved1;
		uint16_t MME_LEN;
		uint8_t MME_SUBVER;
		uint8_t Reserved2;
		uint8_t MACADDR [6];
		uint8_t TMSLOT;
		uint8_t COUPLING;
		uint8_t NUMTMS;
		uint8_t Reserved4;
		uint16_t TMNUMACTCARRIERS;
		uint32_t Reserved6;
		uint8_t GIL;
		uint8_t Reserved7;
		uint8_t AGC;
		uint8_t Reserved8;
		uint8_t MOD_CARRIER [1];
	}
	* confirm = (struct vs_rx_tonemap_char_confirm *) (message);
	struct __packed vs_rx_tonemap_char_fragment
	{
		struct ethernet_hdr ethernet;
		struct homeplug_fmi qualcomm;
		uint8_t MOD_CARRIER [1];
	}
	* fragment = (struct vs_rx_tonemap_char_fragment *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (tonemap, 0, sizeof (tonemap));
	for (carrier = slot = 0; slot < slots; carrier = 0, slot++)
	{
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader1 (&request->qualcomm, 1, (VS_RX_TONE_MAP_CHAR | MMTYPE_REQ));
		memcpy (request->MACADDRESS, plc->RDA, sizeof (request->MACADDRESS));
		request->TMSLOT = slot;
		request->COUPLING = plc->coupling;
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		if (ReadMME (plc, 1, (VS_RX_TONE_MAP_CHAR | MMTYPE_CNF)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (confirm->MSTATUS)
		{
			error (1, 0, "Device refused request for slot %d: %s", slot, MMECode (VS_RX_TONE_MAP_CHAR | MMTYPE_CNF, confirm->MSTATUS));
		}
		GIL [slot] = confirm->GIL;
		AGC [slot] = confirm->AGC;
		carriers = LE16TOH (confirm->TMNUMACTCARRIERS);
		slots = confirm->NUMTMS;
		extent = LE16TOH (confirm->MME_LEN) - 22;
		if (extent > (AMP_CARRIERS >> 1))
		{
			error (1, EOVERFLOW, "Too many carriers");
		}
		plc->packetsize -= sizeof (struct vs_rx_tonemap_char_confirm);
		plc->packetsize += sizeof (confirm->MOD_CARRIER);
		if (plc->packetsize > extent)
		{
			plc->packetsize = extent;
		}
		memcpy (&tonemap [slot] [carrier], &confirm->MOD_CARRIER, plc->packetsize);
		carrier += plc->packetsize;
		extent -= plc->packetsize;
		while (extent)
		{
			if (ReadMME (plc, 1, (VS_RX_TONE_MAP_CHAR | MMTYPE_CNF)) <= 0)
			{
				error (1, errno, CHANNEL_CANTREAD);
			}
			plc->packetsize -= sizeof (struct vs_rx_tonemap_char_fragment);
			plc->packetsize += sizeof (fragment->MOD_CARRIER);
			if (plc->packetsize > extent)
			{
				plc->packetsize = extent;
			}
			memcpy (&tonemap [slot] [carrier], fragment->MOD_CARRIER, plc->packetsize);
			carrier += plc->packetsize;
			extent -= plc->packetsize;
		}
	}
	carrier = 0;

/*
 *   LOW BANDS;
 */

	memset (BPC, 0, sizeof (BPC));
	memset (SNR, 0, sizeof (SNR));
	AvgBPC = 0;
	AvgSNR = 0;
	while (carrier < INT_CARRIERS)
	{
		unsigned value = 0;
		unsigned scale = 0;
		unsigned index = carrier >> 1;
		printf ("%04d", carrier);
		for (slot = 0; slot < slots; slot++)
		{
			value = tonemap [slot][index];
			if ((carrier & 1))
			{
				value >>= 4;
			}
			value &= 0x0F;
			if (value > (AMP_BITS-1))
			{
				error (0, EINVAL, "Index %d Slot %d Value %d", carrier, slot, value);
			}
			printf (",%02d", mod2bits [value]);
			BPC [slot] += mod2bits [value];
			SNR [slot] += mod2db [value];
			AvgBPC += mod2bits [value];
			AvgSNR += mod2db [value];
			value *= value;
			scale += value;
		}
		if (_anyset (plc->flags, PLC_GRAPH))
		{
			printf (" %03d ", scale);
			if (scale)
			{
				scale /= slots;
				while (scale--)
				{
					printf ("#");
				}
				active++;
			}
		}
		printf ("\n");
		carrier++;
	}
	AvgBPC /= active;
	AvgBPC /= slots;
	AvgSNR /= active;
	AvgSNR /= slots;
	printf (" SNR");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(SNR [slot]) / active);
	}
	printf (",%8.3f", AvgSNR);
	printf (" \n");
	printf (" ATN");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(SNR [slot]) / active - 60);
	}
	printf (",%8.3f", AvgSNR - 60);
	printf (" \n");
	printf (" BPC");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(BPC [slot]) / active);
	}
	printf (",%8.3f", AvgBPC);
	printf (" \n");

/*
 *   HIGH BANDS;
 */

	memset (BPC, 0, sizeof (BPC));
	memset (SNR, 0, sizeof (SNR));
	AvgBPC = 0;
	AvgSNR = 0;
	while (carrier < carriers)
	{
		unsigned value = 0;
		unsigned scale = 0;
		unsigned index = carrier >> 1;
		printf ("%04d", carrier);
		for (slot = 0; slot < slots; slot++)
		{
			value = tonemap [slot][index];
			if ((carrier & 1))
			{
				value >>= 4;
			}
			value &= 0x0F;
			if (value > (AMP_BITS-1))
			{
				error (0, EINVAL, "Index %d Slot %d Value %d", carrier, slot, value);
			}
			printf (",%02d", mod2bits [value]);
			BPC [slot] += mod2bits [value];
			SNR [slot] += mod2db [value];
			AvgBPC += mod2bits [value];
			AvgSNR += mod2db [value];
			value *= value;
			scale += value;
		}
		if (_anyset (plc->flags, PLC_GRAPH))
		{
			printf (" %03d ", scale);
			if (scale)
			{
				scale /= slots;
				while (scale--)
				{
					printf ("#");
				}
			}
		}
		printf ("\n");
		carrier++;
		active++;
	}
	AvgBPC /= active;
	AvgBPC /= slots;
	AvgSNR /= active;
	AvgSNR /= slots;
	printf (" SNR");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(SNR [slot]) / active);
	}
	printf (",%8.3f", AvgSNR);
	printf (" \n");
	printf (" ATN");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(SNR [slot]) / active - 60);
	}
	printf (",%8.3f", AvgSNR - 60);
	printf (" \n");
	printf (" BPC");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%8.3f", (float)(BPC [slot]) / active);
	}
	printf (",%8.3f", AvgBPC);
	printf (" \n");
	printf (" AGC");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%02d", AGC [slot]);
	}
	printf (" \n");
	printf (" GIL");
	for (slot = 0; slot < slots; slot++)
	{
		printf (",%02d", GIL [slot]);
	}
	printf (" \n");
	return (0);
}


#endif

