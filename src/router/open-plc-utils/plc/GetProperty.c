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
 *   signed GetProperty (struct plc * plc, struct property * property);
 *
 *   plc.h
 *
 *   interrogte PLC devices for a property using the VS_GET_PROPERTY
 *   message and display the property value on stdout as a series of
 *   binary, decimal or hexadecimal bytes or an ASCII string;
 *
 *   the current implementation retrieves properties by number, not
 *   by name;
 *
 *--------------------------------------------------------------------*/

#ifndef GETPROPERTY_SOURCE
#define GETPROPERTY_SOURCE

#include <ctype.h>
#include <stdint.h>
#include <memory.h>

#include "../tools/error.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../plc/plc.h"

signed GetProperty (struct plc * plc, struct plcproperty * plcproperty)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_get_property_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t COOKIE;
		uint8_t DATA_FORMAT;
		uint8_t PROP_FORMAT;
		uint8_t RESERVED [2];
		uint32_t PROP_VERSION;
		uint32_t PROP_LENGTH;
		uint8_t PROP_NUMBER;
	}
	* request = (struct vs_get_property_request *) (message);
	struct __packed vs_get_property_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t MSTATUS;
		uint32_t COOKIE;
		uint8_t DATA_FORMAT;
		uint8_t RESERVED [3];
		uint32_t DATA_LENGTH;
		uint32_t DATA_BUFFER [1];
	}
	* confirm = (struct vs_get_property_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Get Property");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_GET_PROPERTY | MMTYPE_REQ));
	request->COOKIE = HTOLE32 (plc->cookie);
	request->DATA_FORMAT = plcproperty->DATA_FORMAT;
	request->PROP_FORMAT = plcproperty->PROP_FORMAT;
	request->PROP_VERSION = HTOLE32 (plcproperty->PROP_VERSION);
	request->PROP_LENGTH = HTOLE32 (plcproperty->PROP_LENGTH);
	request->PROP_NUMBER = HTOLE32 (plcproperty->PROP_NUMBER);
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_GET_PROPERTY | MMTYPE_CNF)) > 0)
	{
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		if (plcproperty->DATA_FORMAT == PLC_FORMAT_BIN)
		{
			binout (confirm->DATA_BUFFER, confirm->DATA_LENGTH, ' ', '\n', stdout);
			continue;
		}
		if (plcproperty->DATA_FORMAT == PLC_FORMAT_HEX)
		{
			hexout (confirm->DATA_BUFFER, confirm->DATA_LENGTH, ' ', '\n', stdout);
			continue;
		}
		if (plcproperty->DATA_FORMAT == PLC_FORMAT_DEC)
		{
			decout (confirm->DATA_BUFFER, confirm->DATA_LENGTH, ' ', '\n', stdout);
			continue;
		}
		if (plcproperty->DATA_FORMAT == PLC_FORMAT_ASC)
		{
			chrout (confirm->DATA_BUFFER, confirm->DATA_LENGTH, '.', '\n', stdout);
			continue;
		}
	}
	return (0);
}


#endif

