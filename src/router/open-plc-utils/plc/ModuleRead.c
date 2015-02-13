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
 *   signed ModuleRead (struct plc * plc, struct _file_ * file, uint16_t source, uint16_t module, uint16_t submodule);
 *
 *   plc.h
 *
 *   Read a module from volatile or non-volatile memory and write it
 *   to file;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef MODULEREAD_SOURCE
#define MODULEREAD_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>

#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/endian.h"
#include "../tools/memory.h"
#include "../plc/plc.h"

signed ModuleRead (struct plc * plc, struct _file_ * file, uint16_t source, uint16_t module, uint16_t submodule)

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

	unsigned timer = channel->timeout;
	uint16_t length = PLC_MODULE_SIZE;
	uint32_t offset = 0;
	Request (plc, "Read Module from %s", source? "Flash": "Memory");
	if (lseek (file->file, 0, SEEK_SET) == -1)
	{
		error (PLC_EXIT (plc), errno, FILE_CANTHOME, file->name);
		return (-1);
	}
	while (length == PLC_MODULE_SIZE)
	{
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_MODULE_OPERATION | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		request->NUM_OP_DATA = 1;
		request->MODULE_SPEC.MOD_OP = HTOLE16 (source);
		request->MODULE_SPEC.MOD_OP_DATA_LEN = HTOLE16 (sizeof (request->MODULE_SPEC));
		request->MODULE_SPEC.MOD_OP_RSVD = HTOLE32 (0);
		request->MODULE_SPEC.MODULE_ID = HTOLE16 (module);
		request->MODULE_SPEC.MODULE_SUB_ID = HTOLE16 (submodule);
		request->MODULE_SPEC.MODULE_LENGTH = HTOLE16 (length);
		request->MODULE_SPEC.MODULE_OFFSET = HTOLE32 (offset);

#if 0
#if defined (__GNUC__)
#warning "Debug code active in module ModuleRead"
#endif

		fprintf (stderr, "----- \n");
		fprintf (stderr, "RESERVED 0x%08X\n", LE32TOH (request->RESERVED));
		fprintf (stderr, "NUM_OP_DATA %d\n", request->NUM_OP_DATA);
		fprintf (stderr, "MOD_OP 0x%02X\n", LE16TOH (request->MODULE_SPEC.MOD_OP));
		fprintf (stderr, "MOD_OP_DATA_LEN %d\n", LE16TOH (request->MODULE_SPEC.MOD_OP_DATA_LEN));
		fprintf (stderr, "RESERVED 0x%08X\n", LE32TOH (request->MODULE_SPEC.MOD_OP_RSVD));
		fprintf (stderr, "MODULE_ID 0x%04X\n", LE16TOH (request->MODULE_SPEC.MODULE_ID));
		fprintf (stderr, "MODULE_SUB_ID 0x%04X\n", LE16TOH (request->MODULE_SPEC.MODULE_SUB_ID));
		fprintf (stderr, "MODULE_LENGTH %d\n", LE16TOH (request->MODULE_SPEC.MODULE_LENGTH));
		fprintf (stderr, "MODULE_OFFSET 0x%08X\n", LE32TOH (request->MODULE_SPEC.MODULE_OFFSET));
		fprintf (stderr, "\n");

#endif

		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		channel->timeout = PLC_MODULE_READ_TIMEOUT;
		if (ReadMME (plc, 0, (VS_MODULE_OPERATION | MMTYPE_CNF)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		channel->timeout = timer;

#if 0
#if defined (__GNUC__)
#warning "Debug code active in module ModuleRead"
#endif

		fprintf (stderr, "MSTATUS 0x%04X\n", LE16TOH (confirm->MSTATUS));
		fprintf (stderr, "ERROR_REC_CODE %d\n", LE16TOH (confirm->ERR_REC_CODE));
		fprintf (stderr, "RESERVED 0x%08X\n", LE32TOH (confirm->RESERVED));
		fprintf (stderr, "NUM_OP_DATA %d\n", confirm->NUM_OP_DATA);
		fprintf (stderr, "MOD_OP 0x%02X\n", LE16TOH (confirm->MODULE_SPEC.MOD_OP));
		fprintf (stderr, "MOD_OP_DATA_LEN %d\n", LE16TOH (confirm->MODULE_SPEC.MOD_OP_DATA_LEN));
		fprintf (stderr, "RESERVED 0x%08X\n", LE32TOH (confirm->MODULE_SPEC.MOD_OP_RSVD));
		fprintf (stderr, "MODULE_ID 0x%04X\n", LE16TOH (confirm->MODULE_SPEC.MODULE_ID));
		fprintf (stderr, "MODULE_SUB_ID 0x%04X\n", LE16TOH (confirm->MODULE_SPEC.MODULE_SUB_ID));
		fprintf (stderr, "MODULE_LENGTH %d\n", LE16TOH (confirm->MODULE_SPEC.MODULE_LENGTH));
		fprintf (stderr, "MODULE_OFFSET 0x%08X\n", LE32TOH (confirm->MODULE_SPEC.MODULE_OFFSET));
		fprintf (stderr, "\n");

#endif

		if (LE16TOH (confirm->MSTATUS))
		{
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		length = LE16TOH (confirm->MODULE_SPEC.MODULE_LENGTH);
		offset = LE32TOH (confirm->MODULE_SPEC.MODULE_OFFSET);
		if (write (file->file, confirm->MODULE_DATA, length) != (signed)(length))
		{
			error (PLC_EXIT (plc), errno, FILE_CANTSAVE, file->name);
			return (-1);
		}
		offset += length;
	}
	return (0);
}


#endif

