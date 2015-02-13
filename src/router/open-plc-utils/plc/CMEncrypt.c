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
 *   CMEncrypt - Send Encrypted Payload;
 *
 *   this program sends and receives raw ethernet frames and so needs
 *   root privileges; if you install it using "chmod 555" and "chown
 *   root:root" then you must login as root to run it; otherwise, you
 *   can install it using "chmod 4555" and "chown root:root" so that
 *   anyone can run it; the program will refuse to run until you get
 *   things right;
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

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
#include "../key/SHA256.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexdump.c"
#include "../tools/todigit.c"
#include "../tools/error.c"
#include "../tools/synonym.c"
#endif

#ifndef MAKEFILE
#include "../plc/Confirm.c"
#include "../plc/Failure.c"
#include "../plc/Request.c"
#include "../plc/Devices.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

#ifndef MAKEFILE
#include "../key/SHA256Reset.c"
#include "../key/SHA256Write.c"
#include "../key/SHA256Block.c"
#include "../key/SHA256Fetch.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/HomePlugHeader.c"
#include "../mme/MMECode.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define CMENCRYPT_PEKS 0x0F
#define CMENCRYPT_AVLN 0x00
#define CMENCRYPT_PID 0x04

/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"A:f:i:K:P:qv",
		"device [device] [...]",
		"Send an encrypted payload using CM_ENCRYPTED_PAYLOAD",
		"A n\tAVLN Status [" LITERAL (CMENCRYPT_AVLN) "]",
		"K n\tPayload Encryption Key Select (PEKS) [" LITERAL (CMENCRYPT_PEKS) "]",
		"P n\tProtocol Identifier (PID) [" LITERAL (CMENCRYPT_PID) "]",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"f f\tpayload file",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};

#ifndef __GNUC__
#pragma pack(push,1)
#endif

	struct __packed cm_encrypted_payload
	{
		uint8_t PEKS;
		uint8_t AVLN;
		uint8_t PID;
		uint16_t PRN;
		uint8_t PMN;
		uint8_t UUID [16];
		uint16_t LEN;
	}
	template =
	{
		CMENCRYPT_PEKS,
		CMENCRYPT_AVLN,
		CMENCRYPT_PID,
		0x0000,
		0x00,
		{
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00
		},
		0x0000
	};

#ifndef __GNUC__
#pragma pack (pop)
#endif

	struct _file_ file =
	{
		-1,
		(char const *) (0)
	};
	struct sha256 sha256;
	uint8_t digest [SHA256_DIGEST_LENGTH];
	time_t timer = time ((time_t *)(0));
	uint8_t packet [ETHER_MAX_LEN];
	uint8_t * buffer;
	signed extent;
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
		case 'f':
			if ((file.file = open (file.name = optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", file.name);
			}
			break;
		case 'P':
			template.PID = (byte)(uintspec (optarg, 0x00, 0x0F));
			break;
		case 'A':
			template.AVLN = (byte)(uintspec (optarg, 0x00, 0x08));
			break;
		case 'K':
			template.PEKS = (byte)(uintspec (optarg, 0x00, 0xFF));
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
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;

/*
 *	load entire file into memory;
 */

	if (file.file == -1)
	{
		error (1, ECANCELED, "No payload file given: Use -f <file>");
	}
	if ((extent = lseek (file.file, 0, SEEK_END)) == -1)
	{
		error (1, errno, FILE_CANTSIZE, file.name);
	}
	if (!(buffer = malloc (extent)))
	{
		error (1, errno, FILE_CANTLOAD, file.name);
	}
	if (lseek (file.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, file.name);
	}
	if (read (file.file, buffer, extent) != extent)
	{
		error (1, errno, FILE_CANTREAD, file.name);
	}
	close (file.file);
	SHA256Reset (&sha256);
	SHA256Write (&sha256, buffer, extent);
	SHA256Fetch (&sha256, digest);
	if (!argc)
	{
		error (1, ECANCELED, "No destination given");
	}
	openchannel (&channel);
	while ((argc) && (* argv))
	{
		signed offset = 0;
		signed remain = extent;

#if 0

		signed length = sizeof (struct packet_ms) - sizeof (template);

#else

		signed length = 502 - sizeof (template);

#endif

		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		template.PRN = (uint16_t)(timer);
		template.PMN = 0;
		memcpy (template.UUID, digest, sizeof (template.UUID));
		while (remain)
		{
			uint8_t * memory = packet;
			if (length > remain)
			{
				length = remain;
			}
			template.PMN++;
			template.LEN = HTOLE16 (length);
			memset (memory, 0, sizeof (struct message));
			memory += EthernetHeader ((struct ethernet_hdr *)(memory), channel.peer, channel.host, channel.type);
			memory += HomePlugHeader ((struct homeplug_hdr *)(memory), 0, (CM_ENCRYPTED_PAYLOAD | MMTYPE_IND));
			memcpy (memory, &template, sizeof (template));
			memory += sizeof (template);
			memcpy (memory, buffer + offset, length);
			memory += length;
			extent = (signed)(memory - packet);
			if (extent < (ETHER_MIN_LEN - ETHER_CRC_LEN))
			{
				extent = (ETHER_MIN_LEN - ETHER_CRC_LEN);
			}
			if (sendpacket (&channel, packet, extent) < extent)
			{
				error (1, errno, CHANNEL_CANTSEND);
			}
			offset += length;
			remain -= length;
		}
		argc--;
		argv++;
	}
	closechannel (&channel);
	free (buffer);
	exit (0);
}

