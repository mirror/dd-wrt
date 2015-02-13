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

#ifndef IDENTITY2_SOURCE
#define IDENTITY2_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>

#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/endian.h"
#include "../tools/memory.h"
#include "../tools/flags.h"
#include "../nvm/nvm.h"
#include "../plc/plc.h"

/*====================================================================*
 *
 *   signed pibchain2 (void const * memory);
 *
 *   search panther/lynx image chain for the next PIB image; print
 *   information on stdout and return;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibchain2 (void const * memory, char const * filename, flag_t flags)

{
	struct nvm_header2 * nvm_header;
	uint32_t origin = ~0;
	uint32_t offset = 0;
	signed module = 0;
	do
	{
		nvm_header = (struct nvm_header2 *)((char *)(memory) + offset);
		if (LE16TOH (nvm_header->MajorVersion) != 1)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header->MinorVersion) != 1)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header->PrevHeader) != origin)
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_HDR_LINK, filename, module);
			}
			return (-1);
		}
		if (checksum32 (nvm_header, sizeof (* nvm_header), 0))
		{
			error (0, 0, NVM_HDR_CHECKSUM, filename, module);
			return (-1);
		}
		origin = offset;
		offset += sizeof (* nvm_header);
		if (LE32TOH (nvm_header->ImageType) == NVM_IMAGE_PIB)
		{
			struct pib_header * pib_header = (struct pib_header *)((char *)(memory) + offset);
			pib_header->PIBLENGTH = HTOLE16((uint16_t)(LE32TOH(nvm_header->ImageLength)));
			pibpeek2 ((char *)(memory) + offset);
			pib_header->PIBLENGTH = 0;
			break;
		}
		if (checksum32 ((char *)(memory) + offset, LE32TOH (nvm_header->ImageLength), nvm_header->ImageChecksum))
		{
			if (_allclr (flags, NVM_SILENCE))
			{
				error (0, errno, NVM_IMG_CHECKSUM, filename, module);
			}
			return (-1);
		}
		offset += LE32TOH (nvm_header->ImageLength);
		module++;
	}
	while (~nvm_header->NextHeader);
	return (0);
}


/*====================================================================*
 *
 *   signed Identity2 (struct plc * plc);
 *
 *   plc.h
 *
 *   read start of parameter chain from flash memory using single
 *   VS_MODULE_OPERATION message and print identity information on
 *   stdout;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed Identity2 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_module_operation_read_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t RESERVED;
		uint8_t NUM_OP_DATA;
		struct __packed
		{
			uint16_t MOD_OP;
			uint16_t MOD_OP_DATA_LEN;
			uint32_t MOD_OP_RSVD;
			uint16_t MODULE_ID;
			uint16_t MODULE_SUB_ID;
			uint16_t MODULE_LENGTH;
			uint32_t MODULE_OFFSET;
		}
		MODULE_SPEC;
	}
	* request = (struct vs_module_operation_read_request *)(message);
	struct __packed vs_module_operation_read_confirm
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
			uint16_t MODULE_ID;
			uint16_t MODULE_SUB_ID;
			uint16_t MODULE_LENGTH;
			uint32_t MODULE_OFFSET;
		}
		MODULE_SPEC;
		uint8_t MODULE_DATA [PLC_MODULE_SIZE];
	}
	* confirm = (struct vs_module_operation_read_confirm *)(message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_MODULE_OPERATION | MMTYPE_REQ));
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	request->NUM_OP_DATA = 1;
	request->MODULE_SPEC.MOD_OP = HTOLE16 (0);
	request->MODULE_SPEC.MOD_OP_DATA_LEN = HTOLE16 (sizeof (request->MODULE_SPEC));
	request->MODULE_SPEC.MOD_OP_RSVD = HTOLE32 (0);
	request->MODULE_SPEC.MODULE_ID = HTOLE16 (PLC_MODULEID_PARAMETERS);
	request->MODULE_SPEC.MODULE_SUB_ID = HTOLE16 (0);
	request->MODULE_SPEC.MODULE_LENGTH = HTOLE16 (PLC_MODULE_SIZE);
	request->MODULE_SPEC.MODULE_OFFSET = HTOLE32 (0);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	if (ReadMME (plc, 0, (VS_MODULE_OPERATION | MMTYPE_CNF)) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
		return (-1);
	}
	if (confirm->MSTATUS)
	{
		Failure (plc, PLC_WONTDOIT);
		return (-1);
	}
	pibchain2 (confirm->MODULE_DATA, "device", plc->flags);
	return (0);
}


#endif

