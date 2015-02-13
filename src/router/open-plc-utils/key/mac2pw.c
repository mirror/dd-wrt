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

#define _GETOPT_H

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../key/keys.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/todigit.c"
#include "../tools/uintspec.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../key/MACPasswords.c"
#include "../key/RNDPasswords.c"
#include "../key/putpwd.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN 6 
#endif

/*====================================================================*
 *   program functions;
 *--------------------------------------------------------------------*/

void (* generate)(unsigned, unsigned, unsigned, unsigned, unsigned, char, flag_t) = RNDPasswords;

/*====================================================================*
 *
 *   void function (const char * string, unsigned range, unsigned alpha, unsigned bunch, unsigned space, flag_t flags)
 *
 *   parse an Ethernet hardware address string into vendor and device
 *   ID substrings; print a specified number of consecutive addresses
 *   and password strings having a defined letter count and grouping;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static void function (const char * string, unsigned range, unsigned alpha, unsigned bunch, unsigned space, flag_t flags)

{
	extern void (* generate)(unsigned, unsigned, unsigned, unsigned, unsigned, char, flag_t);
	const char * offset = string;
	unsigned vendor = 0;
	unsigned device = 0;
	unsigned radix = 0x10;
	unsigned width;
	unsigned digit;
	for (width = 0; width < ETHER_ADDR_LEN; width++)
	{
		if ((digit = todigit (*offset)) < radix)
		{
			vendor *= radix;
			vendor += digit;
			offset++;
			continue;
		}
		error (1, EINVAL, "Bad MAC Address: %s", string);
	}
	if (!vendor)
	{
		error (1, EPERM, "Vendor ID can't be zero");
	}
	for (width = 0; width < ETHER_ADDR_LEN; width++)
	{
		if ((digit = todigit (*offset)) < radix)
		{
			device *= radix;
			device += digit;
			offset++;
			continue;
		}
		error (1, EINVAL, "Bad MAC Address: %s", string);
	}
	if (!device)
	{
		error (1, EPERM, "Device ID can't be zero");
	}
	if (*offset)
	{
		error (1, EINVAL, "Bad MAC address: %s", string);
	}
	if (range > (0x00FFFFFF - device))
	{
		error (1, ERANGE, "Want %d passwords but only %d left in range", range, (0x00FFFFFF - device));
	}
	generate (vendor, device, range, alpha, bunch, space, flags);
	return;
}


/*====================================================================*
 *
 *   int main (int argc, const char * argv []);
 *
 *   generate unique password strings for a range of device hardware
 *   addresses; print paired addresses and passwords on stdout;
 *
 *   Many Atheros programs expect the user to enter a password to
 *   access a device; the password is encoded to product the 16-bit
 *   Device Access Key (DAK) stored in the PIB;
 *
 *   Vendors must publish the device password so that end users can
 *   reproduce the same 16-byte hexadecimal value later; a password
 *   is more user-friendly than a 16-byte hexadecimal value;
 *
 *   given a range of MAC address, this program will produce unique
 *   passwords so vendors can program devices and print labels that
 *   ship devices;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#define DEFAULT_RANGE 1
#define DEFAULT_ALPHA 25
#define DEFAULT_BUNCH 0

int main (int argc, const char * argv [])

{
	extern void (* generate)(unsigned, unsigned, unsigned, unsigned, unsigned, char, flag_t);
	static const char * optv [] =
	{
		"b:el:mn:qv",
		"address [address] [...]",
		"Atheros device password generator",
		"b n\tbunching factor [" LITERAL (DEFAULT_BUNCH) "]",
		"e\tbase passwords on host system entropy (more secure)",
		"l n\tpassword letters [" LITERAL (DEFAULT_ALPHA) "]",
		"m\tbase passwords on MAC addresses (less secure)",                     
		"n n\tgenerate n consecutive passwords [" LITERAL (DEFAULT_RANGE) "]",
		"q\tomit device address on output",
		"v\tprepend PTS flag on output",
		(const char *)(0)
	};
	unsigned range = DEFAULT_RANGE;
	unsigned alpha = DEFAULT_ALPHA;
	unsigned bunch = DEFAULT_BUNCH;
	unsigned space = '-';
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char)(c))
		{
		case 'b':
			bunch = uintspec (optarg, 0, UCHAR_MAX);
			break;
		case 'e':
			generate = RNDPasswords;
			break;
		case 'l':
			alpha = uintspec (optarg, 12, 64);
			break;
		case 'm':
			generate = MACPasswords;
			break;
		case 'n':
			range = uintspec (optarg, 0, 0x00FFFFFF);
			break;
		case 'q':
			_setbits (flags, PASSWORD_SILENCE);
			break;
		case 'v':
			_setbits (flags, PASSWORD_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argv) && (* argv))
	{
		function (* argv, range, alpha, bunch, space, flags);
		argc--;
		argv++;
	}
	return (0);
}

