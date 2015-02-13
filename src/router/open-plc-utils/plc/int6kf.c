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
 *   int6kf.c - Atheros Powerline Device Flash Utility;
 *
 *   this program sends and receives raw ethernet frames and so needs
 *   root priviledges; if you install it using "chmod 555" and "chown
 *   root:root" then you must login as root to run it; otherwise, you
 *   can install it using "chmod 4555" and "chown root:root" so that
 *   anyone can run it; the program will refuse to run until you get
 *   things right;
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
#include <limits.h>
#include <string.h>
#include <ctype.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../ram/nvram.h"
#include "../ram/sdram.h"
#include "../nvm/nvm.h"
#include "../plc/plc.h"
#include "../pib/pib.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/chipset.c"
#include "../plc/Confirm.c"
#include "../plc/Devices.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/FlashNVM.c"
#include "../plc/ReadMME.c"
#include "../plc/Request.c"
#include "../plc/SendMME.c"
#include "../plc/StartDevice1.c"
#include "../plc/FlashDevice1.c"
#include "../plc/UpgradeDevice1.c"
#include "../plc/StartFirmware1.c"
#include "../plc/WriteFirmware1.c"
#include "../plc/WriteCFG.c"
#include "../plc/WriteMEM.c"
#include "../plc/WriteNVM.c"
#include "../plc/WritePIB.c"
#include "../plc/WaitForReset.c"
#include "../plc/WaitForStart.c"
#include "../plc/WaitForRestart.c"
#endif

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/checkfilename.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/hexdump.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/typename.c"
#include "../tools/strfbits.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../ram/sdramfile.c"
#include "../ram/sdrampeek.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvm.c"
#include "../nvm/nvmfile1.c"
#include "../nvm/nvmpeek1.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile1.c"
#include "../pib/pibpeek1.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/HomePlugHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#include "../mme/MMECode.c"
#endif

#ifndef MAKEFILE
#include "../key/keys.c"
#endif

/*====================================================================*
 *
 *   int main (int argc, const char * argv[]);
 *
 *   parse command line, populate int6k structure and perform selected
 *   operations; show help summary when asked; see getoptv and putoptv
 *   to understand command line parsing and help summary display; see
 *   int6k.h for the definition of struct int6k;
 *
 *   the default interface is eth1 because most people use eth0 as
 *   their principle network connection; you can specify another
 *   interface with -i or define environment string PLC to make
 *   that the default interface and save typing;
 *
 *--------------------------------------------------------------------*/

int main (int argc, const char * argv [])

{
	extern struct channel channel;
	static const char *optv [] =
	{
		"C:i:eFN:p:P:qt:vx",
		"-C file -P file -N file",
		"Atheros Powerline Device Flash Utility for INT6300",
		"C f\twrite CFG file to device using VS_SET_SDRAM",
		"e\tredirect stderr messages to stdout",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface number [2]",

#else

		"i s\thost interface name [" CHANNEL_ETHDEVICE "]",

#endif

		"F[F]\tflash [force] NVRAM after firmware start using VS_MOD_NVM",
		"N f\twrite NVM file to device using VS_WR_MEM",
		"P f\twrite PIB file to device using VS_WR_MEM",
		"q\tquiet mode",

#if defined (WINPCAP) || defined (LIBPCAP)

		"t n\tread capture time is (n) milliseconds [50]",

#else

		"t n\tread timeout is (n) milliseconds [50]",

#endif

		"v\tverbose mode",
		"x\texit on error",
		(const char *) (0)
	};

#include "../plc/plc.c"

	char firmware [PLC_VERSION_STRING];
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
	opterr = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'C':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.CFG.file = open (optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", optarg);
			}
			if (sdramfile (plc.CFG.file, optarg, plc.flags))
			{
				error (1, ECANCELED, "CFG file %s is corrupt", optarg);
			}
			_setbits (plc.flags, PLC_SDRAM_CONFIG);
			plc.CFG.name = optarg;
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
		case 'F':
			_setbits (plc.module, PLC_MODULE_NVM_PIB);
			if (_anyset (plc.flags, PLC_FLASH_DEVICE))
			{
				_setbits (plc.module, VS_MODULE_FORCE);
			}
			_setbits (plc.flags, PLC_FLASH_DEVICE);
			break;
		case 'N':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.NVM.file = open (optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", optarg);
			}
			plc.NVM.name = optarg;
			if (nvmfile1 (&plc.NVM))
			{
				error (1, errno, "Bad firmware file: %s", plc.NVM.name);
			}
			_setbits (plc.flags, PLC_WRITE_MAC);
			break;
		case 'P':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.PIB.file = open (optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", optarg);
			}
			plc.PIB.name = optarg;
			if (pibfile1 (&plc.PIB))
			{
				error (1, errno, "Bad parameter file: %s", plc.PIB.name);
			}
			_setbits (plc.flags, PLC_WRITE_PIB);
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 't':
			channel.timeout = (unsigned)(uintspec (optarg, 0, UINT_MAX));
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
	if (argc)
	{
		error (1, ECANCELED, "Too many arguments");
	}
	if (plc.CFG.file == -1)
	{
		error (1, ECANCELED, "No CFG file specified");
	}
	if (plc.PIB.file == -1)
	{
		error (1, ECANCELED, "No PIB file specified");
	}
	if (plc.NVM.file == -1)
	{
		error (1, ECANCELED, "No NVM file specified");
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (struct message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (WaitForStart (&plc, firmware, sizeof (firmware)))
	{
		Failure (&plc, "Device must be connected");
		return (-1);
	}
	if (plc.hardwareID > CHIPSET_INT6300)
	{
		Failure (&plc, "Device must be %s or earlier; try using int6kboot.", chipsetname (CHIPSET_INT6300));
		return (-1);
	}
	if (strcmp (firmware, "BootLoader"))
	{
		Failure (&plc, "Bootloader must be running");
		return (-1);
	}
	if (!StartDevice1 (&plc))
	{
		if (_anyset (plc.flags, PLC_FLASH_DEVICE))
		{
			UpgradeDevice1 (&plc);
		}
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

