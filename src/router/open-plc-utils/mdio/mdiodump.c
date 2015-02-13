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
 *   mdiodump.c - Atheros MDIO Custom Module Analyser
 *
 *   Contributor(s):
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Marc Bertola <mbertola@qti.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#define _GETOPT_H

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/endian.h"
#include "../tools/symbol.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/hexstring.c"
#include "../tools/hexdecode.c"
#include "../tools/codelist.c"
#include "../tools/error.c"
#include "../tools/lookup.c"
#include "../tools/assist.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define MDIODUMP_SUMMARY (1 << 0)
#define MDIODUMP_VERBOSE (1 << 1)

#define MDIO32_NORMAL 0x00
#define MDIO32_ACCESS_USING_HIGH 0x02
#define MDIO32_SET_HIGH 0x03

/*====================================================================*
 *   supported PHY types;
 *--------------------------------------------------------------------*/

#define PHY_GENERIC 0
#define PHY_AR8236  1

struct _code_ switches [] =

{
	{
		PHY_GENERIC,
		"generic"
	},
	{
		PHY_AR8236,
		"ar8236"
	}
};


/*====================================================================*
 *   command structure;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

struct __packed command

{
	uint16_t ctrl;
	uint16_t data;
	uint16_t mask;
};


#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   register & memmap struction
 *--------------------------------------------------------------------*/

#define PHY_REGISTER 0
#define GBL_REGISTER 1

struct reg

{
	uint32_t address;
	uint8_t phy;
	uint8_t reg;
	uint32_t content;
	uint8_t type;
};

struct memmap

{
	unsigned size;
	unsigned used;
	struct reg * reg;
};


/*====================================================================*
 *
 *   signed write_phy_reg (struct memmap * memmap, uint8_t phy, uint8_t reg, uint16_t data, uint16_t mask);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed write_phy_reg (struct memmap * memmap, uint8_t phy, uint8_t reg, uint16_t data, uint16_t mask)

{
	unsigned i;
	for (i = 0; i < memmap->used; ++i)
	{
		if (memmap->reg [i].type != PHY_REGISTER)
		{
			continue;
		}
		if (memmap->reg [i].phy != phy)
		{
			continue;
		}
		if (memmap->reg [i].reg == reg)
		{
			continue;
		}
		memmap->reg [i].content &= mask;
		memmap->reg [i].content |= mask & data;
		return (0);
	}
	if (memmap->used < memmap->size)
	{
		memmap->reg [i].phy = phy;
		memmap->reg [i].reg = reg;
		memmap->reg [i].content = mask & data;
		memmap->reg [i].type = PHY_REGISTER;
		memmap->used++;
		return (0);
	}
	error (1, 0, "not enough registers to run simulation");
	return (-1);
}


/*====================================================================*
 *
 *   signed write_gbl_reg (struct memmap *memmap, uint32_t address, uint8_t upper, uint16_t data, uint16_t mask);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed write_gbl_reg (struct memmap * memmap, uint32_t address, uint8_t upper, uint16_t data, uint16_t mask)

{
	unsigned i;
	for (i = 0; i < memmap->used; ++i)
	{
		if (memmap->reg [i].type != GBL_REGISTER)
		{
			continue;
		}
		if (memmap->reg [i].address != address)
		{
			continue;
		}
		if (upper)
		{
			memmap->reg [i].content &= (mask << 16) | 0x0000FFFF;
			memmap->reg [i].content |= (mask & data) << 16;
		}
		else
		{
			memmap->reg [i].content &= mask | 0xFFFF0000;
			memmap->reg [i].content |= mask & data;
		}
		return (0);
	}
	if (memmap->used < memmap->size)
	{
		memmap->reg [i].address = address;
		memmap->reg [i].content = mask & data;
		if (upper)
		{
			memmap->reg [i].content <<= 16;
		}
		memmap->reg [i].type = GBL_REGISTER;
		memmap->used++;
		return (0);
	}
	error (1, 0, "not enough registers to run simulation");
	return (-1);
}


#if 0

/*====================================================================*
 *
 *   signed read_phy_reg (struct memmap * memmap, uint8_t phy, uint8_t reg, uint32_t * data);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed read_phy_reg (struct memmap * memmap, uint8_t phy, uint8_t reg, uint32_t * data)

{
	unsigned i;
	for (i = 0; i < memmap->used; ++i)
	{
		if (memmap->reg [i].type != PHY_REGISTER)
		{
			continue;
		}
		if (memmap->reg [i].phy != phy)
		{
			continue;
		}
		if (memmap->reg [i].reg != reg)
		{
			continue;
		}
		*data = memmap->reg [i].content;
		return (0);
	}
	return (-1);
}


/*====================================================================*
 *
 *   signed read_gbl_reg (struct memmap * memmap, uint32_t address, uint32_t * content);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed read_gbl_reg (struct memmap * memmap, uint32_t address, uint32_t * content)

{
	unsigned i;
	for (i = 0; i < memmap->used; ++i)
	{
		if (memmap->reg [i].type != GBL_REGISTER)
		{
			continue;
		}
		if (memmap->reg [i].address != address)
		{
			continue;
		}
		* content = memmap->reg [i].content;
		return (0);
	}
	return (-1);
}


#endif

/*====================================================================*
 *
 *   void print_memmap (struct memmap *memmap);
 *
 *
 *
 *--------------------------------------------------------------------*/

static void print_memmap (struct memmap * memmap)

{
	unsigned i;
	for (i = 0; i < memmap->used; ++i)
	{
		if (memmap->reg [i].type == PHY_REGISTER)
		{
			printf ("phy 0x%02x, reg 0x%02x: 0x%04x\n", memmap->reg [i].phy, memmap->reg [i].reg, memmap->reg [i].content);
		}
		if (memmap->reg [i].type == GBL_REGISTER)
		{
			printf ("0x%08x: 0x%08x\n", memmap->reg [i].address, memmap->reg [i].content);
		}
	}
	return;
}


/*====================================================================*
 *
 *   void print_command (struct command *command);
 *
 *
 *
 *--------------------------------------------------------------------*/

static void print_command (struct command * command)

{
	union __packed
	{
		uint16_t data;
		struct __packed
		{
			uint16_t start: 2;
			uint16_t operation: 2;
			uint16_t phy_address: 5;
			uint16_t reg_address: 5;
			uint16_t turnaround: 2;
		}
		bits;
	}
	ctrl;
	ctrl.data = command->ctrl;
	printf ("%02x %02x %04x %04x;\n", ctrl.bits.phy_address, ctrl.bits.reg_address, command->data, command->mask);
	return;
}


/*====================================================================*
 *
 *   signed init_memmap (unsigned count, struct memmap * memmap);
 *
 *
 *
 *--------------------------------------------------------------------*/

static signed init_memmap (unsigned count, struct memmap * memmap)

{
	memmap->reg = calloc (count, sizeof (struct reg));
	if (memmap->reg == NULL)
	{
		error (1, errno, "could not allocate reg memory");
	}
	memmap->size = count;
	memmap->used = 0;
	return (0);
}


/*====================================================================*
 *
 *   void free_memmap (struct memmap * memmap);
 *
 *
 *
 *--------------------------------------------------------------------*/

static void free_memmap (struct memmap * memmap)

{
	free (memmap->reg);
	return;
}


/*====================================================================*
 *
 *   signed phy_ar8236 (char const * filename, unsigned commands, flag_t flags);
 *
 *
 *--------------------------------------------------------------------*/

static signed phy_ar8236 (char const * filename, unsigned commands, flag_t flags)

{
	struct command command;
	struct memmap memmap;
	signed ar8236_code;
	signed set_high_addr = 0;
	uint16_t high_addr = 0;
	uint32_t address;
	uint16_t low_address;
	if (init_memmap (commands, &memmap))
	{
		error (1, 0, "could not allocate memory for simulation");
	}
	while (commands--)
	{
		if (read (STDIN_FILENO, &command, sizeof (struct command)) != sizeof (struct command))
		{
			error (0, errno, FILE_CANTREAD, filename);
			return (-1);
		}
		command.ctrl = LE16TOH (command.ctrl);
		command.data = LE16TOH (command.data);
		command.mask = LE16TOH (command.mask);
		ar8236_code = (command.ctrl & 0x180) >> 7;
		switch (ar8236_code)
		{
		case MDIO32_NORMAL:
			if (_anyset (flags, MDIODUMP_VERBOSE))
			{
				printf ("Normal MDIO Operation:\n");
				printf ("\tPhy Address: 0x%02x\n", (command.ctrl & 0x1F0) >> 4);
				printf ("\tRegister Address: 0x%02x\n", (command.ctrl & 0x3E00) >> 9);
			}
			if ((command.ctrl & 0x0C) >> 2 == 0x01)
			{
				write_phy_reg (&memmap, (command.ctrl & 0x1F0) >> 4, (command.ctrl & 0x3E00) >> 9, command.data, command.mask);
			}
			break;
		case MDIO32_SET_HIGH:
			set_high_addr = 1;
			high_addr = command.data & 0x3FF & command.mask;
			if ((command.ctrl & 0x0C) >> 2 == 0x01)
			{
				if (_anyset (flags, MDIODUMP_VERBOSE))
				{
					printf ("Set High Address to 0x%03x:\n", high_addr);
				}
			}
			else
			{
				if (_anyset (flags, MDIODUMP_VERBOSE))
				{
					printf ("Read High Address:\n");
				}
			}
			break;
		case MDIO32_ACCESS_USING_HIGH:
			if (!set_high_addr)
			{
				error (0, 0, "warning: high address bits not set when attempting to do a 32 bit read, assuming high address bits are 0");
				high_addr = 0;
			}
			low_address = (command.ctrl & 0x3E00) >> 9;
			low_address |= (command.ctrl & 0x070) << 1;
			address = high_addr << 9;
			address |= (low_address << 1) & 0xFFFFFFFC;
			if (low_address & 0x01)
			{
				if (_anyset (flags, MDIODUMP_VERBOSE))
				{
					printf ("Access bits 31:16 using address 0x%08x:\n", address);
				}
				write_gbl_reg (&memmap, address, 1, command.data, command.mask);
			}
			else
			{
				if (_anyset (flags, MDIODUMP_VERBOSE))
				{
					printf ("Access bits 15:0 using address 0x%08x:\n", address);
				}
				write_gbl_reg (&memmap, address, 0, command.data, command.mask);
			}
			break;
		}
		if ((command.ctrl & 0x03) != 0x01)
		{
			error (1, ECANCELED, "start command must be 0x01");
		}
		if (_anyset (flags, MDIODUMP_VERBOSE))
		{
			printf ("\tStart: 0x%02x\n", command.ctrl & 0x03);
			printf ("\tOperation: 0x%02x (%s)\n", (command.ctrl & 0x0C) >> 2, ((command.ctrl & 0x0C) >> 2 == 0x01)? "write": "read");
			printf ("\tTurnaround: 0x%02x\n", (command.ctrl & 0xC000) >> 14);
			printf ("\tData: 0x%04x\n", command.data);
			printf ("\tMask: 0x%04x\n", command.mask);
			printf ("\n");
		}
	}
	if (_anyset (flags, MDIODUMP_SUMMARY))
	{
		printf ("Memory after execution:\n");
		print_memmap (&memmap);
	}
	free_memmap (&memmap);
	return (0);
}


/*====================================================================*
 *
 *   signed phy_generic (char const * filename, unsigned commands, flag_t flags);
 *
 *   assume instructions are 16-bit and display them in human readable
 *   format on stdout;
 *
 *
 *--------------------------------------------------------------------*/

static signed phy_generic (char const * filename, unsigned commands, flag_t flags)

{
	struct command command;
	struct memmap memmap;
	if (init_memmap (commands, &memmap))
	{
		error (1, 0, "could not allocate memory for simulation");
	}
	while (commands--)
	{
		if (read (STDIN_FILENO, &command, sizeof (command)) != sizeof (command))
		{
			error (0, errno, FILE_CANTREAD, filename);
			return (-1);
		}
		command.ctrl = LE16TOH (command.ctrl);
		command.data = LE16TOH (command.data);
		command.mask = LE16TOH (command.mask);
		if ((command.ctrl & 0x03) != 0x01)
		{
			error (1, ECANCELED, "start command must be 0x01");
		}
		if (_anyset (flags, MDIODUMP_VERBOSE))
		{
			printf ("Start: 0x%02x\n", command.ctrl & 0x03);
			printf ("Operation: 0x%02x (%s)\n", (command.ctrl & 0x0C) >> 2, ((command.ctrl & 0x0C) >> 2 == 0x01)? "write": "read");
			printf ("Phy Address: 0x%02x\n", (command.ctrl & 0x1F0) >> 4);
			printf ("Register Address: 0x%02x\n", (command.ctrl & 0x3E00) >> 9);
			printf ("Turnaround: 0x%02x\n", (command.ctrl & 0xC000) >> 14);
			printf ("Data: 0x%04x\n", command.data);
			printf ("Mask: 0x%04x\n", command.mask);
			printf ("\n");
			continue;
		}
		if ((command.ctrl & 0x0C) >> 2 == 0x01)
		{
			if (_anyset (flags, MDIODUMP_SUMMARY))
			{
				write_phy_reg (&memmap, (command.ctrl & 0x1F0) >> 4, (command.ctrl & 0x3E00) >> 9, command.data, command.mask);
				continue;
			}
			print_command (&command);
			continue;
		}
	}
	if (_anyset (flags, MDIODUMP_SUMMARY))
	{
		printf ("Memory after execution:\n");
		print_memmap (&memmap);
	}
	free_memmap (&memmap);
	return (0);
}


/*====================================================================*
 *
 *   signed function (char const * filename, unsigned phy_code, flag_t flags);
 *
 *   read the MDIO block header to determine the number of MDIO
 *   instructions in the program block; call appropriate function
 *   to interpret instructions and display them in human readable
 *   format;
 *
 *
 *--------------------------------------------------------------------*/

static signed function (char const * filename, unsigned phy_code, flag_t flags)

{
	uint16_t mdio_header;
	unsigned commands;
	if (read (STDIN_FILENO, &mdio_header, sizeof (mdio_header)) != sizeof (mdio_header))
	{
		error (0, errno, FILE_CANTREAD, filename);
		return (-1);
	}
	mdio_header = LE16TOH (mdio_header);
	commands = (mdio_header & 0xFFC0) >> 6;
	printf ("# ------- %s -------\n", filename);
	if (_anyset (flags, MDIODUMP_SUMMARY))
	{
		printf ("Enabled: %s\n", (mdio_header & 0x0001)? "yes": "no");
		printf ("Number of Commands: %d\n", commands);
	}
	if (phy_code == PHY_GENERIC)
	{
		return (phy_generic (filename, commands, flags));
	}
	if (phy_code == PHY_AR8236)
	{
		return (phy_ar8236 (filename, commands, flags));
	}
	return (0);
}


/*====================================================================*
 *
 *   int main (int argc, const char * argv []);
 *
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, const char * argv [])

{
	static const char *optv [] =
	{
		"st:v",
		"file [file] [...]",
		"Atheros MDIO Custom Module Analyser",
		"s\tprint summary information",
		"t s\tinterpret MDIO commands for phy type (s) [generic]",
		"v\tprint complete module dump, not just the summary",
		(const char *) (0)
	};
	unsigned phy_code = PHY_GENERIC;
	flag_t flags = (flag_t)(0);
	signed state = 0;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 's':
			_setbits (flags, MDIODUMP_SUMMARY);
			break;
		case 't':
			if ((c = lookup (optarg, switches, SIZEOF (switches))) == -1)
			{
				assist (optarg, "type", switches, SIZEOF (switches));
			}
			_setbits (flags, MDIODUMP_SUMMARY);
			phy_code = (unsigned)(c);
			break;
		case 'b':
			_clrbits (flags, MDIODUMP_SUMMARY);
			break;
		case 'v':
			_setbits (flags, MDIODUMP_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (!argc)
	{
		function ("stdin", phy_code, flags);
	}
	while ((argc) && (* argv))
	{
		if (!freopen (* argv, "rb", stdin))
		{
			error (0, errno, "%s", * argv);
			state = 1;
			errno = 0;
		}
		else if (function (* argv, phy_code, flags))
		{
			state = 1;
		}
		argc--;
		argv++;
	}
	return (state);
}

