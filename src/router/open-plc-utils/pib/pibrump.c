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
 *   pibrump.c - Atheros Classification Rule Dump Utility;
 *
 *   This program read classification rules stored in a PIB file and
 *   prints them on stdout in a format similar to int6krule commands;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/memory.h"
#include "../tools/symbol.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../pib/pib.h"
#include "../plc/rules.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/lookup.c"
#include "../tools/reword.c"
#include "../tools/hexstring.c"
#include "../tools/hexdecode.c"
#include "../tools/fdchecksum32.c"
#include "../tools/memout.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile1.c"
#endif

#ifndef MAKEFILE
#include "../plc/rules.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PIB_MAX_AUTOCONN 16
#define PIB_MAX_PRIORITY_MAPS 8

#define PIB_AUTOCONN_COUNT_OFFSET 0x22C
#define PIB_PRIORITY_COUNT_OFFSET 0x228

#define PIB_AUTOCONN_OFFSET 0x310
#define PIB_PRIORITY_MAPS_OFFSET 0x230

/*====================================================================*
 *
 *   signed Classifier (struct PIBClassifier * classifier);
 *
 *   plc.h
 *
 *   This function for program pibrump displays a single classifier;
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed Classifier (struct PIBClassifier * classifier)

{
	char buffer [sizeof (classifier->CR_VALUE) * 3 + 1];
	uint32_t val32;
	uint16_t val16;
	uint8_t val8;
	printf ("%s %s", reword (classifier->CR_PID, fields, SIZEOF (fields)), reword (classifier->CR_OPERAND, operators, SIZEOF (operators)));
	switch (classifier->CR_PID)
	{
	case FIELD_ETH_SA:
	case FIELD_ETH_DA:
		printf (" %s", hexstring (buffer, sizeof (buffer), classifier->CR_VALUE, ETHER_ADDR_LEN));
		break;
	case FIELD_IPV4_SA:
	case FIELD_IPV4_DA:
		putchar (' ');
		memout (classifier->CR_VALUE, IPv4_LEN, "%d", 1, '.', 0, stdout);
		break;
	case FIELD_IPV6_SA:
	case FIELD_IPV6_DA:
		putchar (' ');
		memout (classifier->CR_VALUE, IPv6_LEN, "%02x", 2, ':', 0, stdout);
		break;
	case FIELD_VLAN_UP:
	case FIELD_IPV6_TC:
	case FIELD_IPV4_TOS:
	case FIELD_IPV4_PROT:
		memcpy (&val8, classifier->CR_VALUE, sizeof (val8));
		printf (" 0x%02X", val8);
		break;
	case FIELD_VLAN_ID:
	case FIELD_TCP_SP:
	case FIELD_TCP_DP:
	case FIELD_UDP_SP:
	case FIELD_UDP_DP:
	case FIELD_IP_SP:
	case FIELD_IP_DP:
		memcpy (&val16, classifier->CR_VALUE, sizeof (val16));
		val16 = ntohs (val16);
		printf (" %d", val16);
		break;
	case FIELD_ETH_TYPE:
		memcpy (&val16, classifier->CR_VALUE, sizeof (val16));
		printf (" 0x%04X", val16);
		break;
	case FIELD_IPV6_FL:
		memcpy (&val32, &classifier->CR_VALUE [0], sizeof (val32));
		val32 = ntohl (val32);
		printf (" 0x%08X", val32);
		break;
	case FIELD_HPAV_MME:
		memcpy (&val8, &classifier->CR_VALUE [0], sizeof (val8));
		memcpy (&val16, &classifier->CR_VALUE [1], sizeof (val16));
		printf (" %02x:%04x", val8, val16);
		break;
	case FIELD_TCP_ACK:
		{
			code_t val;
			memcpy (&val, classifier->CR_VALUE, sizeof (val));
			printf (" %s", reword (val, states, SIZEOF (states)));
		}
		break;
	case FIELD_VLAN_TAG:
		{
			code_t val;
			memcpy (&val, classifier->CR_VALUE, sizeof (val));
			printf (" %s", reword (val, states, SIZEOF (states)));
		}
		break;
	default:
		printf (" *****UNSUPPORTED CODE*****");
		break;
	}
	return (0);
}


/*====================================================================*
 *
 *   signed AutoConnection (struct auto_connection * auto_connection, flag_t flags)
 *
 *   plc.h
 *
 *   This function for program pibrump displays a single AutoConnection
 *   structure;
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed AutoConnection (struct auto_connection * auto_connection)

{
	int i;
	if (auto_connection->MACTION == ACTION_TAGTX)
	{
		printf ("-T 0x%08X -V %d ", ntohl (auto_connection->cspec.VLAN_TAG), LE16TOH (auto_connection->cspec.CSPEC_VERSION));
	}
	printf ("%s", reword (auto_connection->MACTION, actions, SIZEOF (actions)));
	printf (" %s ", reword (auto_connection->MOPERAND, operands, SIZEOF (operands)));
	for (i = 0; i < LE16TOH (auto_connection->NUM_CLASSIFIERS); ++i)
	{
		Classifier (&auto_connection->CLASSIFIER [i]);
		putchar (' ');
	}
	printf ("add perm\n");
	return (0);
}


/*====================================================================*
 *
 *   signed ClassifierPriorityMap (struct classifier_priority_map * map)
 *
 *   plc.h
 *
 *   This function for program pibrump displays a single
 *   ClassifierPriorityMap structure;
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

signed ClassifierPriorityMap (struct classifier_priority_map * map)

{
	printf ("%s Any ", reword (LE32TOH (map->Priority), actions, SIZEOF (actions)));
	Classifier (&map->CLASSIFIER);
	printf (" add perm\n");
	return (0);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"eqv",
		"pibfile",
		"Atheros Classification Rule Dump Utility",
		"e\tredirect stderr messages to stdout",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	struct auto_connection auto_connection [PIB_MAX_AUTOCONN];
	struct classifier_priority_map classifier_priority_map [PIB_MAX_PRIORITY_MAPS];
	uint32_t AutoConnection_count;
	uint32_t PriorityMaps_count;
	unsigned i;
	flag_t flags = (flag_t)(0);
	struct _file_ pib;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'q':
			_setbits (flags, PIB_SILENCE);
			break;
		case 'v':
			_setbits (flags, PIB_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argc) && (* argv))
	{
		pib.name = * argv;
		if ((pib.file = open (pib.name, O_BINARY|O_RDONLY)) == -1)
		{
			error (1, errno, "%s", pib.name);
		}
		if (pibfile1 (&pib))
		{
			error (1, errno, "Bad PIB file: %s", pib.name);
		}
		if (lseek (pib.file, PIB_AUTOCONN_COUNT_OFFSET, SEEK_SET) != PIB_AUTOCONN_COUNT_OFFSET)
		{
			error (1, errno, "could not seek to AutoConnection count");
		}
		if (read (pib.file, &AutoConnection_count, sizeof (AutoConnection_count)) != sizeof (AutoConnection_count))
		{
			error (1, errno, "could not read AutoConnection count");
		}
		if (lseek (pib.file, PIB_AUTOCONN_OFFSET, SEEK_SET) != PIB_AUTOCONN_OFFSET)
		{
			error (1, errno, "could not seek to AutoConnections");
		}
		if (read (pib.file, &auto_connection, sizeof (auto_connection)) != sizeof (auto_connection))
		{
			error (1, errno, "could not read AutoConnections");
		}
		if (lseek (pib.file, PIB_PRIORITY_COUNT_OFFSET, SEEK_SET) != PIB_PRIORITY_COUNT_OFFSET)
		{
			error (1, errno, "could not seek to PriorityMaps count");
		}
		if (read (pib.file, &PriorityMaps_count, sizeof (PriorityMaps_count)) != sizeof (PriorityMaps_count))
		{
			error (1, errno, "could not read PriorityMaps count");
		}
		if (lseek (pib.file, PIB_PRIORITY_MAPS_OFFSET, SEEK_SET) != PIB_PRIORITY_MAPS_OFFSET)
		{
			error (1, errno, "could not seek to Priority Map");
		}
		if (read (pib.file, &classifier_priority_map, sizeof (classifier_priority_map)) != sizeof (classifier_priority_map))
		{
			error (1, errno, "could not read Priority Map");
		}
		close (pib.file);
		if (_allclr (flags, PIB_SILENCE))
		{
			printf ("# auto connection rules:\n");
		}
		for (i = 0; i < AutoConnection_count; ++i)
		{
			AutoConnection (&auto_connection [i]);
		}
		if (_allclr (flags, PIB_SILENCE))
		{
			printf ("# priority mapping rules:\n");
		}
		for (i = 0; i < PriorityMaps_count; ++i)
		{
			ClassifierPriorityMap (&classifier_priority_map [i]);
		}
		argv++;
		argc--;
	}
	exit (0);
}

