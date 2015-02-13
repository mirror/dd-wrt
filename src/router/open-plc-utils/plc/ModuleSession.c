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
 *   signed ModuleSession (struct plc * plc, unsigned modules, struct vs_module_spec * vs_module_spec);
 *
 *   Establish a module download session; the session expires after
 *   30 minutes; the session is lost if no modules are committed in
 *   that time; the timeout is set to 10 seconds so that the device
 *   has enouth time to reply;
 *
 *   array vs_module_spec contains information about each module in
 *   this session;
 *
 *
 *--------------------------------------------------------------------*/

#ifndef MODULESESSION_SOURCE
#define MODULESESSION_SOURCE

#include "../tools/error.h"
#include "../plc/plc.h"

signed ModuleSession (struct plc * plc, unsigned modules, struct vs_module_spec * vs_module_spec)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_module_operation_start_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t RESERVED1;
		uint8_t NUM_OP_DATA;
		struct __packed
		{
			uint16_t MOD_OP;
			uint16_t MOD_OP_DATA_LEN;
			uint32_t MOD_OP_RSVD;
			uint32_t MOD_OP_SESSION_ID;
			uint8_t NUM_MODULES;
		}
		MODULE_SPEC;
		struct vs_module_spec MOD_OP_SPEC [10];
	}
	* request = (struct vs_module_operation_start_request *)(message);
	struct __packed vs_module_operation_start_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint16_t MSTATUS;
		uint16_t ERR_REC_CODE;
		uint32_t RESERVED;
		uint8_t NUM_OP_DATA;
		struct __packed
		{
			uint16_t MOD_OP;
			uint16_t MOD_OP_DATA_LEN;
			uint32_t MOD_OP_RSVD;
			uint32_t MOD_OP_SESSION_ID;
			uint8_t NUM_MODULES;
		}
		MODULE_SPEC;
		struct __packed
		{
			uint16_t MOD_STATUS;
			uint16_t ERR_REC_CODE;
		}
		MOD_OP_DATA [1];
	}
	* confirm = (struct vs_module_operation_start_confirm *)(message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	unsigned timer = channel->timeout;
	struct vs_module_spec * request_spec = (struct vs_module_spec *)(&request->MOD_OP_SPEC);
	Request (plc, "Start Module Write Session");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_MODULE_OPERATION | MMTYPE_REQ));
	plc->packetsize = sizeof (* request);
	request->NUM_OP_DATA = 1;
	request->MODULE_SPEC.MOD_OP = HTOLE16 (PLC_MOD_OP_START_SESSION);
	request->MODULE_SPEC.MOD_OP_DATA_LEN = HTOLE16 ((uint16_t)(sizeof (request->MODULE_SPEC)) + modules * sizeof (struct vs_module_spec));
	request->MODULE_SPEC.MOD_OP_SESSION_ID = HTOLE32 (plc->cookie);
	request->MODULE_SPEC.NUM_MODULES = modules;
	while (modules--)
	{
		request_spec->MODULE_ID = HTOLE16 (vs_module_spec->MODULE_ID);
		request_spec->MODULE_SUB_ID = HTOLE16 (vs_module_spec->MODULE_SUB_ID);
		request_spec->MODULE_LENGTH = HTOLE32 (vs_module_spec->MODULE_LENGTH);
		request_spec->MODULE_CHKSUM = vs_module_spec->MODULE_CHKSUM;
		vs_module_spec++;
		request_spec++;
	}
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	channel->timeout = PLC_MODULE_REQUEST_TIMEOUT;
	if (ReadMME (plc, 0, (VS_MODULE_OPERATION | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		channel->timeout = timer;
		return (-1);
	}
	channel->timeout = timer;
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	return (0);
}


#endif

