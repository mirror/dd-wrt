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
 *   signed ReadFirmware1 (struct plc * plc);
 *
 *   plc.h
 *
 *   Read the firmware image from the device using as many VS_RD_MOD
 *   messages as needed; Write image blocks to file as as they are
 *   read;
 *
 *   the object here here is to read NVM from RAM in 1024 byte blocks
 *   until MOFFSET exceeds the image length;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef READFIRMWARE1_SOURCE
#define READFIRMWARE1_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/endian.h"
#include "../tools/memory.h"
#include "../nvm/nvm.h"

signed ReadFirmware1 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_rd_mod_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MODULEID;
		uint8_t MACCESS;
		uint16_t MLENGTH;
		uint32_t MOFFSET;
		uint8_t MSECRET [16];
	}
	* request = (struct vs_rd_mod_request *) (message);
	struct __packed vs_rd_mod_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint8_t RES [3];
		uint8_t MODULEID;
		uint8_t RESERVED;
		uint16_t MLENGTH;
		uint32_t MOFFSET;
		uint32_t CHKSUM;
		uint8_t BUFFER [PLC_RECORD_SIZE];
	}
	* confirm = (struct vs_rd_mod_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	uint32_t header = 0;
	uint32_t extent = 0;
	uint32_t offset = 0;
	uint16_t length = PLC_RECORD_SIZE;
	Request (plc, "Read Firmware from Device");
	if (lseek (plc->nvm.file, 0, SEEK_SET))
	{
		error (PLC_EXIT (plc), errno, FILE_CANTHOME, plc->nvm.name);
		return (1);
	}
	do
	{
		memset (message, 0, sizeof (* message));
		EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
		QualcommHeader (&request->qualcomm, 0, (VS_RD_MOD | MMTYPE_REQ));
		plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
		request->MODULEID = VS_MODULE_MAC;
		request->MLENGTH = HTOLE16 (length);
		request->MOFFSET = HTOLE32 (offset);
		if (SendMME (plc) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
			return (-1);
		}
		if (ReadMME (plc, 0, (VS_RD_MOD | MMTYPE_CNF)) <= 0)
		{
			error (PLC_EXIT (plc), errno, CHANNEL_CANTREAD);
			return (-1);
		}
		if (confirm->MSTATUS)
		{
			Failure (plc, PLC_WONTDOIT);
			return (-1);
		}
		if (LE16TOH (confirm->MLENGTH) != length)
		{
			error (PLC_EXIT (plc), 0, PLC_ERR_LENGTH);
			return (-1);
		}
		if (LE32TOH (confirm->MOFFSET) != offset)
		{
			error (PLC_EXIT (plc), 0, PLC_ERR_OFFSET);
			return (-1);
		}
		length = LE16TOH (confirm->MLENGTH);
		offset = LE32TOH (confirm->MOFFSET);
		if (checksum32 (confirm->BUFFER, length, confirm->CHKSUM))
		{
			error (PLC_EXIT (plc), ECANCELED, "Bad Packet Checksum");
			return (-1);
		}
		if (offset == extent)
		{
			struct nvm_header1 * nvm_header = (struct nvm_header1 *)(confirm->BUFFER);
			if (checksum32 (nvm_header, sizeof (* nvm_header), 0))
			{
				error (PLC_EXIT (plc), ECANCELED, "Bad Header Checksum");
				return (-1);
			}
			if (LE32TOH (nvm_header->HEADERVERSION) != 0x60000000)
			{
				error (PLC_EXIT (plc), ECANCELED, "Bad Header Version");
				return (-1);
			}
			extent += sizeof (* nvm_header);
			extent += LE32TOH (nvm_header->IMAGELENGTH);
			header = LE32TOH (nvm_header->NEXTHEADER);
		}
		if ((offset + length) > extent)
		{
			length = extent - offset;
		}
		if (lseek (plc->nvm.file, offset, SEEK_SET) != (off_t)(offset))
		{
			error (PLC_EXIT (plc), errno, FILE_CANTSEEK, plc->nvm.name);
			return (-1);
		}
		if (write (plc->nvm.file, confirm->BUFFER, length) != (signed)(length))
		{
			error (PLC_EXIT (plc), errno, FILE_CANTSEEK, plc->nvm.name);
			return (-1);
		}
		offset += length;
		length = 1024;
	}
	while ((header) || (offset < extent));
	Confirm (plc, "Read %s", plc->nvm.name);
	return (0);
}


#endif

