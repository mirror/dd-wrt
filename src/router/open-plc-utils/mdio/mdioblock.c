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
 *   mdioblock.c
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/endian.h"
#include "../tools/number.h"
#include "../tools/chars.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../mdio/mdio.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/todigit.c"
#endif

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static signed c;
static unsigned row = 1;
static unsigned col = 1;
static uint16_t count = 0;
static uint16_t instr = 0;
static uint16_t addr;
static uint16_t data;
static uint16_t mask;
static uint16_t phy;
static uint16_t reg;

/*====================================================================*
 *
 *   signed mygetc ();
 *
 *   fetch next character from stdin and update the file cursor;
 *
 *--------------------------------------------------------------------*/

static signed mygetc ()

{
	signed c = getc (stdin);
	if (c == '\n')
	{
		row++;
		col = 0;
	}
	else if (~c)
	{
		col++;
	}
	return (c);
}


/*====================================================================*
 *
 *   uint16_t integer (unsigned radix);
 *
 *
 *--------------------------------------------------------------------*/

static uint16_t integer (unsigned radix)

{
	extern signed c;
	uint16_t value = 0;
	unsigned digit = 0;
	while ((digit = todigit (c)) < radix)
	{
		value *= radix;
		value += digit;
		c = mygetc ();
	}
	while (isspace (c))
	{
		c = mygetc ();
	}
	return (value);
}


/*====================================================================*
 *
 *   void assemble (flag_t flags);
 *
 *   read stdin and write stdout; convert hexadecimal input to binary
 *   MDIO register instructions;
 *
 *   the input file consists of zero or more hexadecimal instructions
 *   consisting of a phy address, register address, register data and
 *   register mask; instructions are terminated with semicolon; fields
 *   are separated by white space; scriptstyle comments are permitted
 *   between instructions but not between instruction fields;
 *
 *   the output file will consist of one 16-bit program header plus
 *   one 16-bit MDIO instructions for each input instruction; the
 *   output is padded to the nearest multiple of 32-bits;
 *
 *--------------------------------------------------------------------*/

static void assemble (flag_t flags)

{
	extern signed c;
	c = mygetc ();
	while (c != EOF)
	{
		if (isspace (c))
		{
			do
			{
				c = mygetc ();
			}
			while (isspace (c));
			continue;
		}
		if ((c == '#') || (c == ';'))
		{
			do
			{
				c = mygetc ();
			}
			while (nobreak (c));
			continue;
		}
		phy = integer (16);
		reg = integer (16);
		data = integer (16);
		mask = integer (16);
		instr = MDIO16_INSTR (1, 1, phy, reg, 2);
		write (STDOUT_FILENO, &instr, sizeof (instr));
		data = HTOLE16 (data & 0xFFFF);
		write (STDOUT_FILENO, &data, sizeof (data));
		mask = HTOLE16 (mask & 0xFFFF);
		write (STDOUT_FILENO, &mask, sizeof (mask));
		count++;
		if (_anyset (flags, MDIO_VERBOSE))
		{
			fprintf (stderr, "INSTR=0x%04X DATA=0x%04X MASK=0x%04X\n", instr, data, mask);
		}
		if ((c == ';') || (c == EOF))
		{
			c = mygetc ();
			continue;
		}
		if (_allclr (flags, MDIO_SILENCE))
		{
			error (1, 0, "Illegal character or missing terminator: line %d col %d", row, col);
		}
	}
	return;
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
	static const char * optv [] =
	{
		"qv",
		"file [ file ] [ ...] > file",
		"Atheros Clause 22 MDIO Instruction Block Assembler",
		"q\tquiet mode",
		"v\tverbose mode",
		(const char *) (0)
	};
	flag_t flags = (flag_t)(0);
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'q':
			_setbits (flags, MDIO_SILENCE);
			break;
		case 'v':
			_setbits (flags, MDIO_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (isatty (STDOUT_FILENO))
	{
		error (1, ECANCELED, "stdout must be a file or pipe");
	}

#if defined (WIN32)

	setmode (STDOUT_FILENO, O_BINARY);

#endif

	instr = MDIO16_START (1, 0, count);
	write (STDOUT_FILENO, &instr, sizeof (instr));
	if (!argc)
	{
		assemble (flags);
	}
	while ((argc) && (* argv))
	{
		if (!freopen (* argv, "rb", stdin))
		{
			error (1, errno, "%s", * argv);
		}
		assemble (flags);
		argc--;
		argv++;
	}
	instr = MDIO16_START (1, 0, count);
	addr = count * sizeof (instr) + sizeof (instr);
	if ((addr % sizeof (uint32_t)))
	{
		uint32_t pad = 0;
		write (STDOUT_FILENO, &pad, sizeof (pad) - addr % sizeof (pad));
	}
	if (!lseek (STDOUT_FILENO, 0, SEEK_SET))
	{
		write (STDOUT_FILENO, &instr, sizeof (instr));
	}
	if (_anyset (flags, MDIO_VERBOSE))
	{
		fprintf (stderr, "%d instructions\n", count);
	}
	return (0);
}

