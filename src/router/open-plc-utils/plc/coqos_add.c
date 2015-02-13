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
 *   coqos_add.c - Add MCMSA Stream
 *
 *
 *   Contributor(s):
 *	Bill Wike <bill.wike@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

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
#include "../plc/coqos.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/basespec.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/bytespec.c"
#include "../tools/ipv4spec.c"
#include "../tools/ipv6spec.c"
#include "../tools/hexdecode.c"
#include "../tools/todigit.c"
#include "../tools/typename.c"
#include "../tools/error.c"
#include "../tools/synonym.c"
#include "../tools/lookup.c"
#include "../tools/assist.c"
#include "../tools/codelist.c"
#endif

#ifndef MAKEFILE
#include "../plc/Request.c"
#include "../plc/Confirm.c"
#include "../plc/Failure.c"
#include "../plc/Display.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/Devices.c"
#include "../plc/rules.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

#ifndef MAKEFILE
#include "../mme/MMECode.c"
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#endif


/*====================================================================*
 *
 *   signed add_conn (struct plc * plc, struct connection * connection);
 *
 *
 *
 *   Contributor(s):
 *	Bill Wike <bill.wike@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed add_conn (struct plc * plc, struct connection * connection)

{
	struct channel * channel = (struct channel *)(plc->channel);
	struct message * message = (struct message *)(plc->message);

#ifndef __GNUC__
#pragma pack (push,1)
#endif

	struct __packed vs_add_conn_req
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		struct connection connection;
	}
	* request = (struct vs_add_conn_req *)(message);
	struct __packed vs_add_conn_cnf
	{
		struct ethernet_hdr ethernet;
		struct qualcomm_hdr qualcomm;
		uint32_t REQ_ID;
		uint8_t MSTATUS;
		uint16_t ERR_REC_CODE;
		uint32_t RSVD;
		uint16_t CID;
		uint8_t REJECT_MAC [ETHER_ADDR_LEN];
		uint16_t CSPEC_VERSION;
		uint8_t CONN_CAP;
		uint8_t CONN_COQOS_PRIO;
		uint16_t CONN_RATE;
		uint32_t CONN_TTL;
	}
	* confirm = (struct vs_add_conn_cnf *)(message);

#ifndef __GNUC__
#pragma pack (pop)
#endif

	Request (plc, "Add COQOS connection");
	memset (message, 0, sizeof (* message));
	EthernetHeader (&request->ethernet, channel->peer, channel->host, channel->type);
	QualcommHeader (&request->qualcomm, 0, (VS_CONN_ADD | MMTYPE_REQ));
	memcpy (&request->connection, connection, sizeof (struct connection));
	memcpy (&request->connection.rule.CLASSIFIERS [request->connection.rule.NUM_CLASSIFIERS], &request->connection.cspec, sizeof (request->connection.cspec));
	plc->packetsize = sizeof (struct vs_add_conn_req);
	if (SendMME (plc) <= 0)
	{
		error (PLC_EXIT (plc), errno, CHANNEL_CANTSEND);
		return (-1);
	}
	while (ReadMME (plc, 0, (VS_CONN_ADD | MMTYPE_CNF)) <= 0)
	{
		if (confirm->MSTATUS)
		{
			Failure (plc, "Device won't add connection");
			return (-1);
		}
		Confirm (plc, "Setting ...");
	}
	return (0);
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
		"ei:qv",
		"action priority destination rate ttl operand condition [...] [device] [...]\n\n          where condition is field operator value",
		"CoQos Stream Utility",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct connection connection;
	struct MMEClassifier * rule = (struct MMEClassifier *)(&connection.rule.CLASSIFIERS);
	uint16_t * word;
	uint8_t * byte;
	signed code;
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
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	memset (&connection, 0, sizeof (connection));
	if ((code = lookup (* argv++, actions, SIZEOF (actions))) == -1)
	{
		assist (*--argv, CLASSIFIER_ACTION_NAME, actions, SIZEOF (actions));
	}
	connection.cspec.CONN_CAP = (uint8_t)(code);
	argc--;
	if (!argc)
	{
		error (1, ECANCELED, "Expected Priority: 0-15");
	}
	connection.cspec.CONN_COQOS_PRIO = (uint8_t)(uintspec (* argv++, 0, 15));
	argc--;
	if (!argc)
	{
		error (1, ECANCELED, "Expected Destination MAC Address");
	}
	if (!hexencode (connection.APP_DA, sizeof (connection.APP_DA), synonym (* argv++, devices, SIZEOF (devices))))
	{
		error (1, errno, "Invalid MAC=[%s]", *--argv);
	}
	argc--;
	if (!argc)
	{
		error (1, ECANCELED, "Expected Data Rate: 10-9000 (kbps)");
	}
	connection.cspec.CONN_RATE = (uint16_t)(uintspec (* argv++, 1, 9000));
	argc--;
	if (!argc)
	{
		error (1, ECANCELED, "Expected TTL: 10000-2000000 (microseconds)");
	}
	connection.cspec.CONN_TTL = (uint32_t)(uintspec (* argv++, 10000, 2000000));
	argc--;
	if ((code = lookup (* argv++, operands, SIZEOF (operands))) == -1)
	{
		assist (*--argv, CLASSIFIER_OPERAND_NAME, operands, SIZEOF (operands));
	}
	connection.rule.MOPERAND = (uint8_t)(code);
	argc--;
	while ((* argv) && (lookup (* argv, controls, SIZEOF (controls)) == -1))
	{
		if ((code = lookup (* argv++, fields, SIZEOF (fields))) == -1)
		{
			assist (*--argv, CLASSIFIER_FIELD_NAME, fields, SIZEOF (fields));
		}
		rule->CR_PID = (uint8_t)(code);
		argc--;
		if ((code = lookup (* argv++, operators, SIZEOF (operators))) == -1)
		{
			assist (*--argv, CLASSIFIER_OPERATOR_NAME, operators, SIZEOF (operators));
		}
		rule->CR_OPERAND = (uint8_t)(code);
		argc--;
		if (!argc || !* argv)
		{
			error (1, ENOTSUP, "Have %s '%s' without any value", CLASSIFIER_OPERATOR_NAME, *--argv);
		}
		switch (rule->CR_PID)
		{
		case FIELD_ETH_SA:
		case FIELD_ETH_DA:
			bytespec (* argv++, rule->CR_VALUE, ETHER_ADDR_LEN);
			break;
		case FIELD_IPV4_SA:
		case FIELD_IPV4_DA:
			ipv4spec (* argv++, rule->CR_VALUE);
			break;
		case FIELD_IPV6_SA:
		case FIELD_IPV6_DA:
			ipv6spec (* argv++, rule->CR_VALUE);
			break;
		case FIELD_VLAN_UP:
		case FIELD_IPV4_TOS:
		case FIELD_IPV4_PROT:
			byte = (uint8_t *)(rule->CR_VALUE);
			*byte = (uint8_t)(basespec (* argv++, 0, sizeof (* byte)));
			break;
		case FIELD_VLAN_ID:
		case FIELD_TCP_SP:
		case FIELD_TCP_DP:
		case FIELD_UDP_SP:
		case FIELD_UDP_DP:
		case FIELD_IP_SP:
		case FIELD_IP_DP:
			word = (uint16_t *)(rule->CR_VALUE);
			*word = (uint16_t)(basespec (* argv++, 0, sizeof (* word)));
			*word = htons (*word);
			break;
		case FIELD_ETH_TYPE:
			word = (uint16_t *)(rule->CR_VALUE);
			*word = (uint16_t)(basespec (* argv++, 0, sizeof (* word)));
			*word = htons (*word);
			break;
		case FIELD_HPAV_MME:
			bytespec (* argv++, rule->CR_VALUE, sizeof (uint8_t) + sizeof (uint16_t));
			byte = (uint8_t *)(rule->CR_VALUE);
			HTOBE16 ((uint16_t)(* ++byte));
			break;
		case FIELD_IPV6_TC:
		case FIELD_IPV6_FL:
		case FIELD_TCP_ACK:
		default:
			error (1, ENOTSUP, "Field '%s' (0x%02X)", argv [-2], rule->CR_PID);
			break;
		}
		connection.rule.NUM_CLASSIFIERS++;
		if (connection.rule.NUM_CLASSIFIERS > RULE_MAX_CLASSIFIERS)
		{
			error (1, ENOTSUP, "More than %d classifiers in rule", RULE_MAX_CLASSIFIERS);
		}
		rule++;
		argc--;
	}
	connection.cspec.CSPEC_VERSION = 0x0001;
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		add_conn (&plc, &connection);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		add_conn (&plc, &connection);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

