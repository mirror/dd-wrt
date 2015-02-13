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
 *   plcfwd.c - Atheros PLC Forward Configuration Manager;
 *
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
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

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/Request.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/Devices.c"
#endif

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/hexdump.c"
#include "../tools/hexview.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexout.c"
#include "../tools/todigit.c"
#include "../tools/synonym.c"
#include "../tools/binout.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/MMECode.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PLCFWD_VERBOSE (1 << 0)
#define PLCFWD_SILENCE (1 << 1)

#define PLCFWD_LENGTH 0
#define PLCFWD_OFFSET 0

#define PLCFWD_GET 0
#define PLCFWD_ADD 1
#define PLCFWD_REM 2
#define PLCFWD_STO 3
#define PLCFWD_CTL 4
#define PLCFWD_SET 5
#define PLCFWD_FWD 6
#define PLCFWD_VER 0

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

/*
 *   this structure is only used in the VS_FORWARD_CONFIG message but
 *   it is common to several variations of the message and is used in
 *   arrays;
 */

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct item

{
	uint8_t MAC_ADDR [ETHER_ADDR_LEN];
	uint16_t NUM_VLANIDS;
	uint16_t VLANID [10];
}

item;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*
 *   synonym table for options -M and -S;
 */

#define STATES (sizeof (states) / sizeof (struct _term_))

static const struct _term_ states [] =

{
	{
		"disable",
		"0"
	},
	{
		"enable",
		"1"
	},
	{
		"off",
		"0"
	},
	{
		"on",
		"1"
	}
};


/*====================================================================*
 *
 *   void readitem (struct item * item, char const * string);
 *
 *   encode a slave structure with infomation specified by a string
 *   specification has the following production:
 *
 *   <spec> := <mac_addr>
 *   <spec> := <spec>,<vlan_id>
 *
 *   basically, encode slave->MAC_ADDR then encode slave->VLANID[]
 *   with hexadecimal VLANID values; we allow 10 VLANID values but
 *   only 8 are legal;
 *
 *   the idea is to read multiple input strings and call this function
 *   to initialize one or more slave structures; it is possible to fit
 *   up to 128 slave structures in one message frame;
 *
 *
 *--------------------------------------------------------------------*/

static void readitem (struct item * item, char const * string)

{
	register uint8_t * origin = (uint8_t *)(item->MAC_ADDR);
	register uint8_t * offset = (uint8_t *)(item->MAC_ADDR);
	size_t extent = sizeof (item->MAC_ADDR);
	memset (item, 0, sizeof (* item));
	while ((extent) && (*string))
	{
		unsigned radix = RADIX_HEX;
		unsigned field = sizeof (uint8_t) + sizeof (uint8_t);
		unsigned value = 0;
		unsigned digit = 0;
		if ((offset != origin) && (*string == HEX_EXTENDER))
		{
			string++;
		}
		while (field--)
		{
			if ((digit = todigit (*string)) < radix)
			{
				value *= radix;
				value += digit;
				string++;
				continue;
			}
			error (1, EINVAL, "bad MAC address: ...[%s] (1)", string);
		}
		*offset = value;
		offset++;
		extent--;
	}
	if (extent)
	{
		error (1, EINVAL, "bad MAC address: ...[%s] (2)", string);
	}
	while (isspace (*string))
	{
		string++;
	}
	if ((*string) && (*string != ','))
	{
		error (1, EINVAL, "bad MAC address: ...[%s] (3)", string);
	}
	while (*string == ',')
	{
		unsigned radix = RADIX_DEC;
		unsigned digit = 0;
		unsigned value = 0;
		do
		{
			string++;
		}
		while (isspace (*string));
		while ((digit = todigit (*string)) < radix)
		{
			value *= radix;
			value += digit;
			string++;
		}
		while (isspace (*string))
		{
			string++;
		}
		if (item->NUM_VLANIDS < (sizeof (item->VLANID) / sizeof (uint16_t)))
		{
			item->VLANID [item->NUM_VLANIDS++] = value;
		}
	}
	while (isspace (*string))
	{
		string++;
	}
	if (*string)
	{
		error (1, EINVAL, "bad VLAN ID: ...[%s]", string);
	}
	return;
}


/*====================================================================*
 *
 *   unsigned readlist (struct item list [], unsigned size);
 *
 *   read one or more items from stdin; discard comments; assume one
 *   item per line; permit multiple items on one line when separated
 *   by semicolon; items cannot straddle lines; readitem () controls
 *   what consitutes one item;
 *
 *--------------------------------------------------------------------*/

static unsigned readlist (struct item list [], unsigned size)

{
	struct item * item = list;
	char string [1024];
	char * sp = string;
	signed c;
	for (c = getc (stdin); c != EOF; c = getc (stdin))
	{
		if (isspace (c))
		{
			continue;
		}
		if (c == '#')
		{
			while ((c != '\n') && (c != EOF))
			{
				c = getc (stdin);
			}
			continue;
		}
		sp = string;
		while ((c != ';') && (c != '\n') && (c != EOF))
		{
			*sp++ = (char)(c);
			c = getc (stdin);
		}
		*sp = (char)(0);
		if (size)
		{
			readitem (item++, string);
			size--;
		}
	}
	return ((unsigned)(item - list));
}


/*====================================================================*
 *
 *   void showlist (struct item list [], unsigned items)
 *
 *   print item list on stdout in a format suitable for input using
 *   readlist (); this function may be commented out if it not used;
 *
 *
 *--------------------------------------------------------------------*/

#if 0

static void showlist (struct item list [], unsigned items)

{
	while (items--)
	{
		uint16_t fields = list->NUM_VLANIDS;
		uint16_t * field = list->VLANID;
		hexout (list->MAC_ADDR, sizeof (list->MAC_ADDR), 0, 0, stdout);
		while (fields--)
		{
			printf (", %d", *field);
			field++;
		}
		printf ("\n");
		list++;
	}
	return;
}


#endif

/*====================================================================*
 *
 *   signed ReadVLANIDs (struct plc * plc, uint32_t offset, uint32_t length);
 *
 *
 *--------------------------------------------------------------------*/

static signed ReadVLANIDs (struct plc * plc, uint32_t offset, uint32_t length)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint32_t DATA_LENGTH;
		uint32_t DATA_OFFSET;
		uint16_t RESERVED3;
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint32_t DATA_LENGTH;
		uint32_t DATA_OFFSET;
		uint8_t DATA [PLC_MODULE_SIZE];
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_GET;
	request->MVERSION = PLCFWD_VER;
	request->DATA_OFFSET = HTOLE32 (offset);
	request->DATA_LENGTH = HTOLE32 (length);
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
		hexview (confirm->DATA, LE32TOH (confirm->DATA_OFFSET), LE32TOH (confirm->DATA_LENGTH), stdout);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed AddVLANIDs (struct plc * plc, struct item list [], unsigned items);
 *
 *
 *--------------------------------------------------------------------*/

static signed AddVLANIDs (struct plc * plc, struct item list [], unsigned items)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint16_t ITEMS;
		struct item LIST [1];
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	struct item * item = request->LIST;
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_ADD;
	request->MVERSION = PLCFWD_VER;
	request->ITEMS = HTOLE16 (items);
	while (items--)
	{
		unsigned count;
		memcpy (item->MAC_ADDR, list->MAC_ADDR, sizeof (item->MAC_ADDR));
		item->NUM_VLANIDS = HTOLE16 (list->NUM_VLANIDS);
		for (count = 0; count < list->NUM_VLANIDS; count++)
		{
			item->VLANID [count] = HTOLE16 (list->VLANID [count]);
		}

// item++;

		item = (struct item *)(&item->VLANID [count]);
		list++;
	}
	plc->packetsize = (signed)((uint8_t *)(item) - (uint8_t *)(request));
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   signed RemoveVLANIDs (struct plc * plc, struct item list [], unsigned items);
 *
 *
 *--------------------------------------------------------------------*/

static signed RemoveVLANIDs (struct plc * plc, struct item list [], unsigned items)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint16_t ITEMS;
		struct item LIST [1];
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	struct item * item = request->LIST;
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_REM;
	request->MVERSION = PLCFWD_VER;
	request->ITEMS = HTOLE16 (items);
	while (items--)
	{
		unsigned count;
		memcpy (item->MAC_ADDR, list->MAC_ADDR, sizeof (item->MAC_ADDR));
		item->NUM_VLANIDS = HTOLE16 (list->NUM_VLANIDS);
		for (count = 0; count < list->NUM_VLANIDS; count++)
		{
			item->VLANID [count] = HTOLE16 (list->VLANID [count]);
		}

// item++;

		item = (struct item *)(&item->VLANID [count]);
		list++;
	}
	plc->packetsize = (signed)((uint8_t *)(item) - (uint8_t *)(request));
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   signed CommitVLANIDs (struct plc * plc);
 *
 *
 *--------------------------------------------------------------------*/

static signed CommitVLANIDs (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_STO;
	request->MVERSION = PLCFWD_VER;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   signed ControlVLANIDs (struct plc * plc);
 *
 *
 *--------------------------------------------------------------------*/

static signed ControlVLANIDs (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint8_t ENABLE;
		uint8_t UPSTREAMCHECK;
		uint8_t RESERVED3;
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_CTL;
	request->MVERSION = PLCFWD_VER;
	request->ENABLE = plc->module;
	request->UPSTREAMCHECK = plc->pushbutton;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   signed DefaultVLANIDs (struct plc * plc);
 *
 *
 *--------------------------------------------------------------------*/

static signed DefaultVLANIDs (struct plc * plc, struct item list [], unsigned items)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint16_t VLANID;
		uint16_t RESERVED3;
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_SET;
	request->MVERSION = PLCFWD_VER;
	request->VLANID = HTOLE16 (list [0].VLANID [0]);
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   signed ForwardVLANIDs (struct plc * plc);
 *
 *
 *--------------------------------------------------------------------*/

static signed ForwardVLANIDs (struct plc * plc)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_forward_config_request
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t MREQUEST;
		uint8_t MVERSION;
		uint32_t RESERVED2;
		uint8_t ENABLED;
		uint16_t VLANID;
		uint16_t RESERVED3;
		struct item ITEM;
	}
	* request = (struct vs_forward_config_request *) (message);
	struct __packed vs_forward_config_confirm
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint8_t RESERVED1;
		uint8_t RESULTCODE;
		uint8_t OPERATION;
		uint8_t MVERSION;
		uint32_t RESERVED2;
	}
	* confirm = (struct vs_forward_config_confirm *) (message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_FORWARD_CONFIG | MMTYPE_REQ));
	request->MREQUEST = PLCFWD_FWD;
	request->MVERSION = PLCFWD_VER;
	plc->packetsize = (ETHER_MIN_LEN - ETHER_CRC_LEN);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_FORWARD_CONFIG | MMTYPE_CNF)) > 0)
	{
		if (confirm->RESULTCODE)
		{
			Failure (plc, PLC_WONTDOIT);
			continue;
		}
	}
	return (0);
}


/*====================================================================*
 *
 *   void function (struct plc * plc, struct item list [], unsigned items);
 *
 *   perform the VLANID action specified by the action member
 *   in struct plc as set in the main program; only one action
 *   is performed;
 *
 *
 *--------------------------------------------------------------------*/

static void function (struct plc * plc, uint32_t offset, uint32_t length, struct item * list, unsigned items)

{
	if (plc->action == PLCFWD_GET)
	{
		ReadVLANIDs (plc, offset, length);
		return;
	}
	if (plc->action == PLCFWD_ADD)
	{
		AddVLANIDs (plc, list, items);
		return;
	}
	if (plc->action == PLCFWD_REM)
	{
		RemoveVLANIDs (plc, list, items);
		return;
	}
	if (plc->action == PLCFWD_STO)
	{
		CommitVLANIDs (plc);
		return;
	}
	if (plc->action == PLCFWD_CTL)
	{
		ControlVLANIDs (plc);
		return;
	}
	if (plc->action == PLCFWD_SET)
	{
		DefaultVLANIDs (plc, list, items);
		return;
	}
	if (plc->action == PLCFWD_FWD)
	{
		ForwardVLANIDs (plc);
		return;
	}
	return;
}


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
		"ACD:ef:i:l:M:o:qRS:t:vxz:",
		"device [device] [...] [> stdout]",
		"Qualcomm Atheros VLANID Forward Configuration Manager",
		"A\tadd VLAN ID of multiple slaves to memory",
		"C\tcommit configuration to flash memory",
		"D x\tset default VLAN ID",
		"e\tredirect stderr to stdout",
		"f s\tread VLANIDS from file (s)",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"l n\tdata length in bytes [" LITERAL (PLCFWD_LENGTH) "]",
		"M n\tenable VLANID forwarding on the master",
		"o x\tdata offset in bytes [" LITERAL (PLCFWD_OFFSET) "]",
		"q\tquiet mode",
		"R\tremove VLAN ID of multiple slaves from memory",
		"S n\tenable VLANID forwarding on all slaves",
		"t n\ttimeout is (n) millisecond [" LITERAL (CHANNEL_TIMEOUT) "]",
		"v\tverbose mode",
		"x\texit on error",
		"z s\tslavespec",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct item list [128];
	unsigned size = sizeof (list) / sizeof (struct item);
	unsigned items = 0;
	uint32_t offset = 0;
	uint32_t length = 0;
	signed c;
	memset (&list, 0, sizeof (list));
	if (getenv (PLCDEVICE))
	{
		channel.ifname = strdup (getenv (PLCDEVICE));
	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'A':
			plc.action = PLCFWD_ADD;
			break;
		case 'C':
			plc.action = PLCFWD_STO;
			break;
		case 'D':
			plc.action = PLCFWD_SET;
			list [0].VLANID [0] = (uint16_t)(basespec (optarg, 10, sizeof (uint16_t)));
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'f':
			if (!freopen (optarg, "rb", stdin))
			{
				error (1, errno, "%s", optarg);
			}
			items += readlist (&list [items], size - items);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'M':
			plc.action = PLCFWD_CTL;
			plc.module = (uint8_t)(uintspec (synonym (optarg, states, STATES), 0, UCHAR_MAX));
			break;
		case 'l':
			length = (uint32_t) (basespec (optarg, 10, sizeof (length)));
			break;
		case 'o':
			offset = (uint32_t) (basespec (optarg, 10, sizeof (offset)));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'R':
			plc.action = PLCFWD_REM;
			break;
		case 'S':
			plc.action = PLCFWD_CTL;
			plc.pushbutton = (uint8_t)(uintspec (synonym (optarg, states, STATES), 0, UCHAR_MAX));
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'x':
			_setbits (plc.flags, PLC_BAILOUT);
			break;
		case 'z':
			readitem (&list [items++], optarg);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;

#if 0

	showlist (list, items);

#endif

	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		function (&plc, offset, length, list, items);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		function (&plc, offset, length, list, items);
		argv++;
		argc--;
	}
	free (plc.message);
	closechannel (&channel);
	return (0);
}

