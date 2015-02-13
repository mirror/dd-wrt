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
 *   plcmdio32.c - Qualcomm Atheros 32-bit MDIO Register Editor
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../plc/plc.h"
#include "../ram/nvram.h"
#include "../ram/sdram.h"
#include "../nvm/nvm.h"
#include "../pib/pib.h"
#include "../mme/mme.h"
#include "../mdio/mdio.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Devices.c"
#include "../plc/Display.c"
#include "../mme/UnwantedMessage.c"
#endif

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/todigit.c"
#include "../tools/hexdump.c"
#include "../tools/hexview.c"
#include "../tools/regview32.c"
#include "../tools/synonym.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/MMECode.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define MDIO_FLAG_REVERSE (1 << 0)

#define MDIO_MODE_READ 0
#define MDIO_MODE_WRITE 1

#define CODE_SHIFT 3
#define CODE_MASK (0x03 << CODE_SHIFT)
#define CODE_HIGH_ADDR 0x03
#define CODE_LOW_ADDR 0x02

#define HIGH_ADDR_SHIFT 9
#define LOW_ADDR_SHIFT 1

#define HIGH_ADDR_MASK (0x000003FF << HIGH_ADDR_SHIFT)
#define LOW_ADDR_MASK 0x000001FC

/*====================================================================*
 *
 *   signed mdio (struct channel * channel, uint8_t mode, uint8_t phy, uint16_t * data);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed mdio (struct channel * channel, uint8_t mode, uint8_t phy, uint8_t reg, uint16_t * data)

{
	struct message message;
	signed packetsize;

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_mdio_command_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t OPERATION;
		uint8_t PHY;
		uint8_t REG;
		uint16_t DATA;
	}
	* request = (struct vs_mdio_command_request *)(&message);
	struct __packed vs_mdio_command_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t MSTATUS;
		uint16_t DATA;
		uint8_t PHY;
		uint8_t REG;
	}
	* confirm = (struct vs_mdio_command_confirm *)(&message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (&message, 0, sizeof (message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_MDIO_COMMAND | MMTYPE_REQ));
	request->OPERATION = mode;
	request->PHY = phy;
	request->REG = reg;
	request->DATA = HTOLE16 (*data);

#if 1

	printf (" phy 0x%02X", phy);
	printf (" reg 0x%02X", reg);
	printf (" data 0x%04X", * data);
	printf ("\n");

#endif

	if (sendpacket (channel, &message, (ETHER_MIN_LEN - ETHER_CRC_LEN)) == -1)
	{
		error (1, errno, CHANNEL_CANTSEND);
	}
	while ((packetsize = readpacket (channel, &message, sizeof (message))) > 0)
	{
		if (UnwantedMessage (&message, packetsize, 0, (VS_MDIO_COMMAND | MMTYPE_CNF)))
		{
			continue;
		}
		if (confirm->MSTATUS)
		{
			error (0, 0, "%s (%0X): %s", MMECode (confirm->qualcomm.MMTYPE, confirm->MSTATUS), confirm->MSTATUS, PLC_WONTDOIT);
			continue;
		}
		*data = confirm->DATA;
		return (0);
	}
	return (-1);
}


/*====================================================================*
 *
 *   void function (struct channel * channel, uint8_t mode, uint32_t address, uint32_t content, flag_t flags);
 *
 *
 *
 *--------------------------------------------------------------------*/

static void function (struct channel * channel, uint8_t mode, uint32_t address, uint32_t content, flag_t flags)

{
	uint8_t phy;
	uint8_t reg;
	uint16_t mdio_data;
	uint16_t high_addr = (address & HIGH_ADDR_MASK) >> HIGH_ADDR_SHIFT;
	uint16_t low_addr = (address & LOW_ADDR_MASK) >> LOW_ADDR_SHIFT;

/*
 *   supply chip with high address bytes
 */

	phy = CODE_HIGH_ADDR << CODE_SHIFT;
	reg = 0;
	mdio_data = high_addr;
	if (mdio (channel, MDIO_MODE_WRITE, phy, reg, &mdio_data))
	{
		error (1, 0, "could not set high address bits");
	}
	if (_allclr (flags, MDIO_FLAG_REVERSE))
	{

/*
 *   supply chip with first low address bytes and first data chunk
 */

		phy = CODE_LOW_ADDR << CODE_SHIFT;
		phy |= (low_addr & 0xE0) >> 5;
		reg = (low_addr & 0x1F);
		mdio_data = (content & 0x0000FFFF);
		if (mdio (channel, mode, phy, reg, &mdio_data))
		{
			error (1, 0, "could not read low 16bits");
		}
		if (mode == MDIO_MODE_READ)
		{
			content = mdio_data;
		}

/*
 *   supply chip with second low address bytes and second data chunk
 */

		phy = CODE_LOW_ADDR << CODE_SHIFT;
		phy |= (low_addr & 0xE0) >> 5;
		reg = (low_addr & 0x1F) | 0x01;
		mdio_data = (content & 0xFFFF0000) >> 16;
		if (mdio (channel, mode, phy, reg, &mdio_data))
		{
			error (1, 0, "could not read high 16bits");
		}
		if (mode == MDIO_MODE_READ)
		{
			content |= (mdio_data << 16);
			printf ("0x%08x: 0x%08x\n", address, content);
		}
	}
	else
	{

/*
 *   supply chip with second low address bytes and second data chunk
 */

		phy = CODE_LOW_ADDR << CODE_SHIFT;
		phy |= (low_addr & 0xE0) >> 5;
		reg = (low_addr & 0x1F) | 0x01;
		mdio_data = (content & 0xFFFF0000) >> 16;
		if (mdio (channel, mode, phy, reg, &mdio_data))
		{
			error (1, 0, "could not read high 16bits");
		}
		if (mode == MDIO_MODE_READ)
		{
			content = (mdio_data << 16);
		}

/*
 *   supply chip with first low address bytes and first data chunk
 */

		phy = CODE_LOW_ADDR << CODE_SHIFT;
		phy |= (low_addr & 0xE0) >> 5;
		reg = (low_addr & 0x1F);
		mdio_data = (content & 0x0000FFFF);
		if (mdio (channel, mode, phy, reg, &mdio_data))
		{
			error (1, 0, "could not read low 16bits");
		}
		if (mode == MDIO_MODE_READ)
		{
			content |= mdio_data;
			printf ("0x%08x: 0x%08x\n", address, content);
		}
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"a:d:ehi:qv",
		"[device] [...]",
		"Qualcomm Atheros 32-bit MDIO Register Editor",
		"a n\taddress is (n) [0x00000000]",
		"d n\tcontent is (n) [0x00000000]",
		"e\tredirect stderr to stdout",
		"h\tsend high-order data before low-order data",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	flag_t flags = (flag_t)(0);
	uint8_t mode = MDIO_MODE_READ;
	uint32_t address = 0;
	uint32_t content = 0;
	signed c;
	if (getenv (PLCDEVICE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE));

#else

		channel.ifname = strdup (getenv (PLCDEVICE));

#endif

	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'a':
			address = (uint32_t)(uintspec (optarg, 0, 0x0007FFFF));
			if (address & 0x03)
			{
				error (1, 0, "address must be on an even 4 byte boundary");
			}
			break;
		case 'd':
			mode = MDIO_MODE_WRITE;
			content = (uint32_t)(uintspec (optarg, 0, 0x0FFFFFFFF));
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			break;
		case 'r':
			_setbits (flags, MDIO_FLAG_REVERSE);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	openchannel (&channel);
	if (!argc)
	{
		function (&channel, mode, address, content, flags);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		function (&channel, mode, address, content, flags);
		argv++;
		argc--;
	}
	closechannel (&channel);
	return (0);
}

