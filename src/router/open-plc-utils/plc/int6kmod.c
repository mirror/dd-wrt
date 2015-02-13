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
 *    int6kmod.c -
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
#include "../ether/channel.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Devices.c"
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/Request.c"
#include "../plc/Failure.c"
#include "../plc/ReadMME.c"
#include "../plc/SendMME.c"
#include "../plc/ModuleCommit.c"
#include "../plc/ModuleRead.c"
#include "../plc/ModuleDump.c"
#include "../plc/ModuleSpec.c"
#include "../plc/ModuleSession.c"
#include "../plc/ModuleWrite.c"
#endif

#ifndef MAKEFILE
#include "../tools/basespec.c"
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/hexdump.c"
#include "../tools/hexview.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/checkfilename.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/strfbits.c"
#include "../tools/synonym.c"
#include "../tools/typename.c"
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
 *
 *   void Manager (struct plc * plc, uint16_t ModuleID, uint16_t ModuleSubID);
 *
 *   read a custom modul from flash memory or write a suceom module
 *   to flash memory using the VS_MODULE_OPERATION message; read and
 *   save the old module before writing and commiting the new module;
 *
 *   ModuleDump () and ModuleRead () are called with argument source
 *   set to VS_MODULE_FLASH because this function operates on modules
 *   in flash memory only;
 *
 *
 *--------------------------------------------------------------------*/

static void Manager (struct plc * plc, uint16_t ModuleID, uint16_t ModuleSubID)

{
	if (_anyset (plc->flags, PLC_DUMP_MODULE))
	{
		ModuleDump (plc, PLC_MOD_OP_READ_FLASH, ModuleID, ModuleSubID);
		return;
	}
	if (_anyset (plc->flags, PLC_READ_MODULE))
	{
		ModuleRead (plc, &plc->nvm, PLC_MOD_OP_READ_FLASH, ModuleID, ModuleSubID);
	}
	if (_anyset (plc->flags, PLC_WRITE_MODULE))
	{
		struct vs_module_spec vs_module_spec;
		vs_module_spec.MODULE_ID = ModuleID;
		vs_module_spec.MODULE_SUB_ID = ModuleSubID;
		ModuleSpec (&plc->NVM, &vs_module_spec);
		ModuleSession (plc, 1, &vs_module_spec);
		ModuleWrite (plc, &plc->NVM, 0, &vs_module_spec);
		ModuleCommit (plc, (PLC_COMMIT_FORCE));
		return;
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *   parse command line, populate plc structure and perform selected
 *   operations; show help summary if asked; see getoptv and putoptv
 *   to understand command line parsing and help summary display; see
 *   plc.h for the definition of struct plc;
 *
 *   the command line accepts multiple MAC addresses and the program
 *   performs the specified operations on each address, in turn; the
 *   address order is significant but the option order is not; the
 *   default address is a local broadcast that causes all devices on
 *   the local H1 interface to respond but not those at the remote
 *   end of the powerline;
 *
 *   the default address is 00:B0:52:00:00:01; omitting the address
 *   will automatically address the local device; some options will
 *   cancel themselves if this makes no sense;
 *
 *   the default interface is eth1 because most people use eth0 as
 *   their principle network connection; you can specify another
 *   interface with -i or define environment string PLC to make
 *   that the default interface and save typing;
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"dei:m:M:qs:S:t:vx",
		"device [device] [...]",
		"Qualcomm Atheros Powerline Device Module Manager",
		"d\tdump module in hexadecimal format",
		"e\tredirect stderr to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"m f\tread module from NVRAM to file (f)",
		"M f\twrite module file (f) to flash memory",
		"s n\tmodule sub-ID [" LITERAL (PLC_SUBMODULEID) "]",
		"S n\tsession ID [" LITERAL (PLCSESSION) "]",
		"t n\tmodule ID [" LITERAL (PLC_MODULEID) "]",
		"q\tquiet mode",
		"v\tverbose mode",
		(char const *) (0)
	};

#include "../plc/plc.c"

	struct vs_module_spec vs_module_spec;
	uint16_t ModuleID = 0;
	uint16_t ModuleSubID = 0;
	signed c;
	memset (&vs_module_spec, 0, sizeof (vs_module_spec));
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
		case 'C':
			_setbits (plc.flags, PLC_COMMIT_MODULE);
			break;
		case 'd':
			if (_anyset (plc.flags, PLC_READ_MODULE))
			{
				error (1, EINVAL, "Options -d and -m are mutually exclusive");
			}
			_setbits (plc.flags, PLC_DUMP_MODULE);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'M':
			_setbits (plc.flags, PLC_WRITE_MODULE);
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.NVM.file = open (plc.NVM.name = optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", plc.NVM.name);
			}
			if (ModuleSpec (&plc.NVM, &vs_module_spec) == -1)
			{
				error (1, errno, "%s", optarg);
			}
			break;
		case 'm':
			if (_anyset (plc.flags, PLC_DUMP_MODULE))
			{
				error (1, EINVAL, "Options -d and -m are mutually exclusive");
			}
			_setbits (plc.flags, PLC_READ_MODULE);
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.nvm.file = open (plc.nvm.name = optarg, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "%s", plc.nvm.name);
			}

#ifndef WIN32

			chown (optarg, getuid (), getgid ());

#endif

			break;
		case 't':
			ModuleID = (uint16_t)(basespec (optarg, 16, sizeof (ModuleID)));
			break;
		case 's':
			ModuleSubID = (uint16_t)(basespec (optarg, 16, sizeof (ModuleSubID)));
			break;
		case 'S':
			plc.cookie = (uint32_t)(basespec (optarg, 16, sizeof (plc.cookie)));
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'x':
			_setbits (plc.flags, PLC_BAILOUT);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
	{
		if (plc.nvm.file != -1)
		{
			error (1, ECANCELED, PLC_NODEVICE);
		}
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		Manager (&plc, ModuleID, ModuleSubID);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		Manager (&plc, ModuleID, ModuleSubID);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

