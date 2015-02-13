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
 *   int EmulateHost64 (struct plc * plc);
 *
 *   plc.h
 *
 *   wait indefinitely for VS_HOST_ACTION messages; service the device
 *   only; it will stop dead - like a bug! - on error;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef EMULATEHOST64_SOURCE
#define EMULATEHOST64_SOURCE

#include <unistd.h>
#include <memory.h>

#include "../plc/plc.h"
#include "../ether/channel.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/files.h"

#include "../nvm/nvm.h"
#include "../pib/pib.h"

signed EmulateHost64 (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);
	static char const * actions [] =
	{
		"start device",
		"store firmware",
		"store parameters",
		"update host",
		"config memory",
		"restore defaults",
		"unknown"
	};

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_host_action_ind
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MACTION;
		uint8_t MAJOR_VERSION;
		uint8_t MINOR_VERSION;
	}
	* indicate = (struct vs_host_action_ind *) (message);

#if 0

	struct __packed vs_host_action_rsp
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
	}
	* response = (struct vs_host_action_rsp *) (message);

#endif

#ifndef __GNUC__
#pragma pack (pop)
#endif

	char const * NVM = plc->NVM.name;
	char const * PIB = plc->PIB.name;
	signed status;
	signed action;
	Request (plc, "Waiting for Host Action");
	while (1)
	{
		status = ReadMME (plc, 0, (VS_HOST_ACTION | MMTYPE_IND));
		if (status < 0)
		{
			break;
		}
		if (status > 0)
		{
			printf ("\n");
			if (indicate->MACTION < (sizeof (actions) / sizeof (char const *)))
			{
				Confirm (plc, "Host Action Request is (%d) %s.", indicate->MACTION, actions [indicate->MACTION]);
			}
			else
			{
				error (0, ENOTSUP, "Host Action 0x%0X", indicate->MACTION);
				continue;
			}
			action = indicate->MACTION;
			memcpy (channel->peer, indicate->ethernet.OSA, sizeof (channel->peer));
			if (HostActionResponse (plc))
			{
				return (-1);
			}
			if (action == 0x00)
			{
				if (BootDevice1 (plc))
				{
					return (-1);
				}
				if (_anyset (plc->flags, PLC_FLASH_DEVICE))
				{
					if (WriteNVM (plc))
					{
						return (-1);
					}
					if (WritePIB (plc))
					{
						return (-1);
					}
					if (FlashNVM (plc))
					{
						return (-1);
					}
				}
				continue;
			}
			if (action == 0x01)
			{
				close (plc->NVM.file);
				if (ReadFirmware1 (plc))
				{
					return (-1);
				}
				if ((plc->NVM.file = open (plc->NVM.name = plc->nvm.name, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->NVM.name);
				}
				if (ResetDevice (plc))
				{
					return (-1);
				}
				continue;
			}
			if (action == 0x02)
			{
				close (plc->PIB.file);
				if (ReadParameters1 (plc))
				{
					return (-1);
				}
				if ((plc->PIB.file = open (plc->PIB.name = plc->pib.name, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->PIB.name);
				}
				if (ResetDevice (plc))
				{
					return (-1);
				}
				continue;
			}
			if (action == 0x03)
			{
				close (plc->PIB.file);
				if (ReadParameters1 (plc))
				{
					return (-1);
				}
				if ((plc->PIB.file = open (plc->PIB.name = plc->pib.name, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->PIB.name);
				}
				close (plc->NVM.file);
				if (ReadFirmware1 (plc))
				{
					return (-1);
				}
				if ((plc->NVM.file = open (plc->NVM.name = plc->nvm.name, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->NVM.name);
				}
				if (ResetDevice (plc))
				{
					return (-1);
				}
				continue;
			}
			if (action == 0x04)
			{
				if (InitDevice1 (plc))
				{
					return (-1);
				}
				continue;
			}
			if (action == 0x05)
			{
				close (plc->NVM.file);
				if ((plc->NVM.file = open (plc->NVM.name = NVM, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->NVM.name);
				}
				close (plc->PIB.file);
				if ((plc->PIB.file = open (plc->PIB.name = PIB, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->PIB.name);
				}
				if (ResetDevice (plc))
				{
					return (-1);
				}
				continue;
			}
			if (action == 0x06)
			{
				close (plc->PIB.file);
				if (ReadParameters1 (plc))
				{
					return (-1);
				}
				if ((plc->PIB.file = open (plc->PIB.name = plc->pib.name, O_BINARY|O_RDONLY)) == -1)
				{
					error (1, errno, "%s", plc->PIB.name);
				}
				continue;
			}
			error (0, ENOSYS, "Host Action 0x%02X", action);
		}
	}
	return (0);
}


#endif

