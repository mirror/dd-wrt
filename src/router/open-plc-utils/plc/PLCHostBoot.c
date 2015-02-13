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

/*====================================================================*"
 *
 *   PLCHostBoot.c -
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PLCHOSTBOOT_SOURCE
#define PLCHOSTBOOT_SOURCE

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/flags.h"
#include "../plc/plc.h"

#define MESSAGE "Starting PLC Host Server\n"
#define SOCKETNAME "/tmp/socket"

static bool done = false;
void terminate (signo_t signal)

{
	done = true;
	return;
}

static signed opensocket (char const * socketname)

{
	signed fd;
	struct sockaddr_un sockaddr_un =
	{

#ifdef __OpenBSD__

		0,

#endif

		AF_UNIX,
		"/tmp/socket"
	};
	strncpy (sockaddr_un.sun_path, socketname, sizeof (sockaddr_un.sun_path));

#ifdef __OpenBSD__

	sockaddr_un.sun_len = SUN_LEN (&sockaddr_un);

#endif

	if (unlink (sockaddr_un.sun_path))
	{
		if (errno != ENOENT)
		{
			error (1, errno, "%s", sockaddr_un.sun_path);
		}
	}
	if ((fd = socket (AF_UNIX, SOCK_DGRAM, 0)) == -1)
	{
		error (1, errno, "%s", sockaddr_un.sun_path);
	}
	if (bind (fd, (struct sockaddr *)(&sockaddr_un), sizeof (sockaddr_un)) == -1)
	{
		error (1, errno, "%s", sockaddr_un.sun_path);
	}
	if (chmod (sockaddr_un.sun_path, 0666) == -1)
	{
		error (1, errno, "%s", sockaddr_un.sun_path);
	}
	return (fd);
}


/*====================================================================*
 *
 *   signed PLCHostBoot (struct plc * plc, char const * socket, signed timer);
 *
 *   int6k.h
 *
 *   wait indefinitely for VS_HOST_ACTION messages; service the device
 *   only; it will stop dead - like a bug! - on error;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed PLCHostBoot (struct plc * plc, char const * socket, unsigned timer)

{
	byte buffer [3000];
	struct plctopology * plctopology = (struct plctopology *)(buffer);
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_host_action_ind
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr intellon;
		uint8_t MACTION;
		uint8_t MAJOR_VERSION;
		uint8_t MINOR_VERSION;
	}
	* indicate = (struct vs_host_action_ind *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	signed fd = opensocket (socket);
	char const * NVM = plc->NVM.name;
	char const * PIB = plc->PIB.name;
	unsigned action;
	signed status;
	memset (buffer, 0, sizeof (buffer));
	write (fd, MESSAGE, strlen (MESSAGE));
	while (!done)
	{
		status = ReadMME (plc, 0, (VS_HOST_ACTION | MMTYPE_IND));
		if (status < 0)
		{
			break;
		}
		if (status < 1)
		{
			PLCTopology (channel, message, plctopology);
			PLCTopologyPrint (plctopology);
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
				FlashDevice1 (plc);
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
			if (ReadParameters (plc))
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
			if (ReadParameters (plc))
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
			if (InitDevice (plc))
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
			if (ReadParameters (plc))
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
	close (fd);
	return (0);
}


#endif

