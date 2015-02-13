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
 *   pibruin.c - Atheros Classification Rule Insert Utility;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
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
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/chars.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../plc/rules.h"
#include "../nvm/nvm.h"
#include "../pib/pib.h"

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
#include "../tools/checksum32.c"
#include "../tools/memout.c"
#include "../tools/assist.c"
#include "../tools/bytespec.c"
#include "../tools/basespec.c"
#include "../tools/ipv4spec.c"
#include "../tools/ipv6spec.c"
#include "../tools/emalloc.c"
#include "../tools/todigit.c"
#include "../tools/codelist.c"
#include "../tools/getargv.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile1.c"
#include "../pib/piblock.c"
#if defined (PRINT_RULES) || defined (PRINT_CLASSIFIERS)
#include "../pib/ruledump.c"
#endif
#endif

#ifndef MAKEFILE
#include "../nvm/nvmseek2.c"
#endif

#ifndef MAKEFILE
#include "../plc/ParseRule.c"
#include "../plc/rules.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define PIB_OFFSET1 0x0228
#define PIB_OFFSET2 0x0760

/*====================================================================*
 *
 *   signed pibimage1 (signed fd, char const * filename, unsigned offset, struct PIBClassifiers * classifiers);
 *
 *   read thunderbolt/lightning parameter block into memory, edit it
 *   and save it;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage1 (signed fd, char const * filename, unsigned offset, struct PIBClassifiers * classifiers)

{
	struct pib_header * pib_header;
	off_t extent;
	void * memory;
	if ((extent = lseek (fd, 0, SEEK_END)) == -1)
	{
		error (1, errno, FILE_CANTSIZE, filename);
	}
	if (!(memory = malloc (extent)))
	{
		error (1, errno, FILE_CANTLOAD, filename);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	if (read (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	memcpy ((byte *)(memory) + offset, classifiers, sizeof (*classifiers));
	pib_header = (struct pib_header *)(memory);
	pib_header->CHECKSUM = checksum32 (memory, extent, pib_header->CHECKSUM);
	if (write (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTSAVE, filename);
	}
	if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	free (memory);
	close (fd);
	return (0);
}


/*====================================================================*
 *
 *   signed pibimage2 (signed fd, char const * filename, struct nvm_header2 * nvm_header, unsigned offset, struct PIBClassifiers * classifiers);
 *
 *   read panther/lynx parameter block into memory, edit it and save
 *   it;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage2 (signed fd, char const * filename, struct nvm_header2 * nvm_header, unsigned offset, struct PIBClassifiers * classifiers)

{
	void * memory;
	off_t extent = LE32TOH (nvm_header->ImageLength);
	if (!(memory = malloc (extent)))
	{
		error (1, errno, FILE_CANTLOAD, filename);
	}
	if (read (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	memcpy ((byte *)(memory) + offset, classifiers, sizeof (*classifiers));
	nvm_header->ImageChecksum = checksum32 (memory, extent, 0);
	if (write (fd, memory, extent) != extent)
	{
		error (1, errno, FILE_CANTSAVE, filename);
	}
	if (lseek (fd, (off_t)(0) - extent, SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	nvm_header->HeaderChecksum = checksum32 (nvm_header, sizeof (* nvm_header), nvm_header->HeaderChecksum);
	if (lseek (fd, (off_t)(0) - sizeof (* nvm_header), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	if (write (fd, nvm_header, sizeof (* nvm_header)) != sizeof (* nvm_header))
	{
		error (1, errno, FILE_CANTSAVE, filename);
	}
	if (lseek (fd, (off_t)(0) - sizeof (* nvm_header), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	free (memory);
	return (0);
}


/*====================================================================*
 *
 *   void make_rule (int argc, char const * argv [], struct PIBClassifiers * classifiers);
 *
 *   read argv [] and populate struct MMEClassifiers with QCA PLC
 *   classifier rules; function ParseRule converts each rule to an
 *   MMERule; we then convert that into one of two formats used to
 *   store classifier rules in the PIB;
 *
 *
 *--------------------------------------------------------------------*/

static void make_rule (int argc, char const * argv [], struct PIBClassifiers * classifiers)

{
	static char const * optv [] =
	{
		"T:V:",
		"",
		"Atheros Classification Rule Insert Utility",
		"T\ttag",
		"V\tversion",
		(char const *) (0)
	};
	struct MMERule rule;
	struct cspec cspec;
	signed c;
	optind = 1;
	memset (&rule, 0, sizeof (rule));
	memset (&cspec, 0, sizeof (cspec));
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'T':
			cspec.VLAN_TAG = (uint32_t)(basespec (optarg, 16, sizeof (cspec.VLAN_TAG)));
			cspec.VLAN_TAG = htonl (cspec.VLAN_TAG);
			break;
		case 'V':
			cspec.CSPEC_VERSION = (uint16_t)(basespec (optarg, 10, sizeof (cspec.CSPEC_VERSION)));
			cspec.CSPEC_VERSION = HTOLE16 (cspec.CSPEC_VERSION);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;

#if defined (DUMP_RULE_ARGS)

/*
 *   print arguments on stdout;
 */

	for (c = 0; c < argc; c++)
	{
		printf ("argv [%d] = [%s]\n", c, argv [c]);
	}

#endif

	if ((argc) && (*argv))
	{
		ParseRule (&argc, &argv, &rule, &cspec);

#if defined (PRINT_RULES)

/*
 *   print MMERule set on stdout in human readable format; these rules are populated by ParseRule but members
 *   are not PIB ready; they are not properly grouped, byte-aligned or endian-ized;
 */

		MMERuleDump (&rule);

#endif

		if ((rule.NUM_CLASSIFIERS > 1) || (rule.MACTION == ACTION_STRIPTX) || (rule.MACTION == ACTION_STRIPRX) || (rule.MACTION == ACTION_TAGTX) || (classifiers->priority_count >= RULE_MAX_PRIORITY_MAPS))
		{
			struct auto_connection * auto_connection;
			if (classifiers->autoconn_count >= RULE_MAX_AUTOCONN)
			{
				error (1, ENOTSUP, "Too many auto connection rules");
			}
			auto_connection = &classifiers->auto_connection [classifiers->autoconn_count];
			auto_connection->MACTION = rule.MACTION;
			auto_connection->MOPERAND = rule.MOPERAND;
			auto_connection->NUM_CLASSIFIERS = HTOLE16 ((uint16_t)(rule.NUM_CLASSIFIERS));
			for (c = 0; c < RULE_MAX_CLASSIFIERS; c++)
			{
				auto_connection->CLASSIFIER [c].CR_PID = HTOLE32 ((uint32_t)(0xFF));
				auto_connection->CLASSIFIER [c].CR_OPERAND = HTOLE32 ((uint32_t)(0xFF));
			}
			for (c = 0; c < rule.NUM_CLASSIFIERS; c++)
			{
				auto_connection->CLASSIFIER [c].CR_PID = HTOLE32 ((uint32_t)(rule.CLASSIFIER [c].CR_PID));
				auto_connection->CLASSIFIER [c].CR_OPERAND = HTOLE32 ((uint32_t)(rule.CLASSIFIER [c].CR_OPERAND));
				memcpy (auto_connection->CLASSIFIER [c].CR_VALUE, rule.CLASSIFIER [c].CR_VALUE, sizeof (auto_connection->CLASSIFIER [c].CR_VALUE));
			}
			memcpy (&auto_connection->cspec, &rule.cspec, sizeof (auto_connection->cspec));
			classifiers->autoconn_count++;
		}
		else
		{
			struct classifier_priority_map * classifier_priority_map;
			if (classifiers->priority_count >= RULE_MAX_PRIORITY_MAPS)
			{
				error (1, ENOTSUP, "Too many priority map rules");
			}
			classifier_priority_map = &classifiers->classifier_priority_map [classifiers->priority_count];
			classifier_priority_map->Priority = HTOLE32 ((uint32_t)(rule.MACTION));
			classifier_priority_map->CLASSIFIER.CR_PID = HTOLE32 ((uint32_t)(rule.CLASSIFIER [0].CR_PID));
			classifier_priority_map->CLASSIFIER.CR_OPERAND = HTOLE32 ((uint32_t)(rule.CLASSIFIER [0].CR_OPERAND));
			memcpy (classifier_priority_map->CLASSIFIER.CR_VALUE, rule.CLASSIFIER [0].CR_VALUE, sizeof (classifier_priority_map->CLASSIFIER.CR_VALUE));
			classifiers->priority_count++;
		}
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
	static char const * optv [] =
	{
		"eo:qv",
		"pibfile < rules",
		"Atheros Classification Rule Insert Utility",
		"e\tredirect stderr messages to stdout",
		"o x\talternate offset is (x)",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};
	struct PIBClassifiers classifiers;
	char const * vector [24];
	uint32_t offset = 0;
	uint32_t version;
	signed fd;
	signed count;
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'o':
			offset = (unsigned) (basespec (optarg, 16, sizeof (offset)));
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
	memset (&classifiers, 0, sizeof (classifiers));
	while ((count = getargv (SIZEOF (vector), vector)))
	{
		make_rule (count, vector, &classifiers);
	}
	classifiers.priority_count = HTOLE32 (classifiers.priority_count);
	classifiers.autoconn_count = HTOLE32 (classifiers.autoconn_count);

#if defined (PRINT_CLASSIFIERS)

/*
 *   print PIB-ready classification rules on stdout in human readable format; these rules are written
 *   directly into the appropriate location in the PIB as a single block;
 */

	PIBClassifiersDump (&classifiers);

#endif

#if defined (WRITE_CLASSIFIERS)

	write (STDOUT_FILENO, &classifiers, sizeof (classifiers));

#endif

	while ((argc) && (*argv))
	{
		if ((fd = open (*argv, O_BINARY|O_RDWR)) == -1)
		{
			error (0, errno, "%s", *argv);
		}
		else if (read (fd, &version, sizeof (version)) != sizeof (version))
		{
			error (0, errno, FILE_CANTREAD, *argv);
		}
		else if (lseek (fd, 0, SEEK_SET))
		{
			error (0, errno, FILE_CANTHOME, *argv);
		}
		else if (LE32TOH (version) == 0x00010001)
		{
			struct nvm_header2 nvm_header;
			if (!nvmseek2 (fd, *argv, &nvm_header, NVM_IMAGE_PIB))
			{
				pibimage2 (fd, *argv, &nvm_header, offset? offset: PIB_OFFSET2, &classifiers);
			}
		}
		else
		{
			pibimage1 (fd, *argv, offset? offset: PIB_OFFSET1, &classifiers);
		}
		close (fd);
		argc--;
		argv++;
	}
	exit (0);
}

